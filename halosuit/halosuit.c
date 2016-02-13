#ifndef MOCK_HARDWARE

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <time.h>

#include <halosuit/halosuit.h>
#include <halosuit/stateofcharge.h>
#include <halosuit/logger.h>
#include <halosuit/gpio.h>

// To add a new hardware device declare a static struct with:
// {device ID, pin number, starting charge (0 for low, 1 for high), 
// 0 for the file descriptor} 
// see gpio.h for details
static struct GPIO_device devices[] = {{"lights",LIGHTS_PIN,false,0},// id:1 pin:66, "high", fd
                                       {"lights auto",LIGHTS_AUTO_PIN,false,0},
                                       {"headlights white",HEADLIGHTS_WHITE_PIN,false,0},
                                       {"headlights red",HEADLIGHTS_RED_PIN,false,0},
                                       {"head fans",HEAD_FANS_PIN,false,0},
                                       {"water pump",WATER_PUMP_PIN,false,0},
                                       {"on button",ON_BUTTON_PIN,true,0}, 
                                       {"peltier",PELTIER_PIN,false,0},
                                       {"high current live",HIGH_CURRENT_LIVE_PIN,true,0},
                                       {"high current ground",HIGH_CURRENT_GROUND_PIN,true,0}
                                      };

static const int num_devices = (int)(sizeof(devices) / sizeof(devices[0]));

#define NUMBER_OF_TEMP_SENSORS 4

//analog in file descriptors
static int temperature[NUMBER_OF_TEMP_SENSORS - 1]; //water temperature is taken care of separately

static int current_draw = 0;  //TODO: need to calculate current value`

//FILE for pipe from readflow.py
static FILE* python_pipe;
static int flowrate = 0;
static double water_temp = 10.0;
// TODO: these defaults need to change when we get data on for them
static double voltage1 = 12.6;
static double voltage2 = 12.0;
static int heartrate = 90;

static char python_buffer[50];
static pthread_t python_thread_id;

//protects against using other functions early
static bool is_initialized = false;

static void enable_analog();

static double analog_to_temperature(char *string)  
{  
	int value = atoi(string); 
	double millivolts = (value / 4096.0) * 1800;  
	double temp = (millivolts - 500.0) / 10.0;  
	return temp;  
}

static void *python_thread()
{
	python_pipe = popen("python /usr/bin/readflow.py", "r");

	while (fgets(python_buffer, sizeof(python_buffer), python_pipe) != NULL) {
		sscanf(python_buffer, "%d %lf %lf %lf %d", &flowrate, &water_temp, &voltage1, &voltage2, &heartrate);
	}
    
    return NULL;
}

void halosuit_init()
{

    enable_analog();
    
	//export gpio pins
    for (int i = 0; i < num_devices; i++) {
        gpio_export(devices[i].pin);
        devices[i].fd = gpio_open_direction_file(devices[i].pin);
        gpio_write_direction_file(devices[i].fd,devices[i].high);
        close(devices[i].fd);
        devices[i].fd = gpio_open_value_file(devices[i].pin);
    }

    //open analog pins
    temperature[HEAD] = open("/sys/bus/iio/devices/iio:device0/in_voltage0_raw", O_RDONLY); //change
    temperature[ARMPITS] = open("/sys/bus/iio/devices/iio:device0/in_voltage1_raw", O_RDONLY);
    temperature[CROTCH] = open("/sys/bus/iio/devices/iio:device0/in_voltage2_raw", O_RDONLY);
    //temperature[WATER] = open("/sys/bus/iio/devices/iio:device0/in_voltage3_raw", O_RDONLY);

    pthread_create(&python_thread_id, NULL, &python_thread, NULL);

    is_initialized = true;
} 

void halosuit_exit()
{
	if (is_initialized) {
		is_initialized = false;

        for (int i = 0; i < num_devices; i++) {
            close(devices[i].fd);
            gpio_unexport(devices[i].pin);
        }

		close(temperature[HEAD]);
		close(temperature[ARMPITS]);
		close(temperature[CROTCH]);
		close(temperature[WATER]);
	}
}

int halosuit_relay_switch(unsigned int relay, int ps)
{
	if (is_initialized && relay < num_devices) {
		if (ps == HIGH) {
			write(devices[relay].fd, "1", 1);
			lseek(devices[relay].fd, 0, 0);
		} else if (ps == LOW) {
			write(devices[relay].fd, "0", 1);
			lseek(devices[relay].fd, 0, 0);
		} else {
			return -1;
		}
		return 0;
	}
	return -1;
}

int halosuit_relay_value(unsigned int relay, int *value)
{
	if (is_initialized && relay < num_devices) {
		char buf[2] = { 0 };
		read(devices[relay].fd, buf, 1);
		*value = atoi(buf);
		lseek(devices[relay].fd, 0, 0);
		return 0;
	}
	return -1;
}

int halosuit_temperature_value(unsigned int location, double *temp)
{
	if (is_initialized && location < NUMBER_OF_TEMP_SENSORS) {
		if (location == WATER) {
			*temp = water_temp;
		} else {
			char buf[5] = { 0 };
			read(temperature[location], buf, 4);
			*temp = analog_to_temperature(buf);
			lseek(temperature[location], 0, 0);
		}
		return 0;
	}
	return -1;
}

int halosuit_flowrate(int *flow) {
	if (is_initialized) {
		*flow = flowrate;
		return 0;
	}
	return -1;
}

int halosuit_voltage_value(unsigned int battery, int *value) 
{
	if (is_initialized) {
        if (battery == TURNIGY_8_AH) {
            *value = (int)(voltage1 * 1000);
        } 
        else if (battery == TURNIGY_2_AH) {
            *value = (int)(voltage2 * 1000);
        } 
        else {
            return -1;
        }
		return 0;
	}
	return -1;
}

int halosuit_current_draw_value(unsigned int batteryID, int *current)  //this is fairly complicated and should probably be simlified
{
    if (batteryID == TURNIGY_2_AH) {
        int value1 = 0;
        int value2 = 0;
        int current_draw = 0;
        if (halosuit_relay_value(ON_BUTTON, &value1)) {
            logger_log("WARNING: FAILURE TO READ ON_BUTTON FOR CURRENT DRAW");
        }
        else if (value1 == HIGH ){
            current_draw += LOW_AMP_DRAW;
        }
        value1 = 0;
        value2 = 0;
        if (halosuit_relay_value(HEADLIGHTS_WHITE, &value1) || halosuit_relay_value(HEADLIGHTS_WHITE, &value2)) {
            logger_log("WARNING: FAILURE TO READ HEADLIGHTS FOR CURRENT DRAW");
        }
        else if (value2 == HIGH) {
            current_draw += HEAD_LIGHTS_DRAW;
        }
        value1 = 0;
        value2 = 0;
        if (halosuit_relay_value(LIGHTS, &value1) || halosuit_relay_value(LIGHTS_AUTO, &value2)) {
            logger_log("WARNING: FAILURE TO READ LIGHTS FOR CURRENT DRAW");
        }
        else if (value1 == HIGH || value2 == HIGH) {
            current_draw += BODY_LIGHTS_DRAW;
        }

        *current = current_draw; 
    }
    else if (batteryID == TURNIGY_8_AH) {
        int current_draw = 0;
        int value = 0;
        if (halosuit_relay_value(PELTIER, &value)) {
            logger_log("WARNING: FAILURE TO READ PELTIER FOR CURRENT DRAW");
            return -1; // the peltier current draw is so much greater than the rest that if the current 
                       // can't be determined then there is no accuracy in the current draw
        }
        else if (value == HIGH) {
             current_draw += PELTIER_DRAW * 2; // multiplied by 2 since there are 2 peltier
        }
        value = 0;
        if (halosuit_relay_value(WATER_PUMP, &value)) {
            logger_log("WARNING: FAILURE TO READ WATER_PUMP FOR CURRENT DRAW");
        }
        else if (value == HIGH) {
            current_draw += WATER_PUMP_DRAW;
        }
        value = 0;
        if (halosuit_relay_value(HEAD_FANS, &value)) {
            logger_log("WARNING: FAILURE TO READ HEAD_FANS FOR CURRENT DRAW");
        }
        else if (value == HIGH) {
            current_draw += HEAD_FANS_DRAW;
        } 
        *current = current_draw;
    }

    return 0;
}


int halosuit_heartrate(int *heart) {
    if (is_initialized){
        *heart = heartrate;
        return 0;
    }
    return -1;
}


void enable_analog() { 
	int analog_fd = open("/sys/devices/bonecapemgr.*/slots", O_RDWR);
    bool analog_set = false;
    char buffer[1024];
    while (read(analog_fd,buffer,1024) != 0) {
        if ((strstr(buffer, "cape-bone-iio")) != NULL) {
            analog_set = true;
            break;
        }
    }
    close(analog_fd);
    if (!analog_set) {
        write(analog_fd, "cape-bone-iio", 13);
    }
}
#endif

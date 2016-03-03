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

#define NUMBER_OF_RELAYS 16
#define NUMBER_OF_TEMP_SENSORS 4

//file descriptors
static int relays[NUMBER_OF_RELAYS];

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
	int export_fd = open("/sys/class/gpio/export", O_WRONLY);
	//export gpio pins
	write(export_fd, "66", 2);
	write(export_fd, "67", 2);
	write(export_fd, "68", 2);
	write(export_fd, "69", 2);
	write(export_fd, "44", 2);
	write(export_fd, "45", 2);
	write(export_fd, "46", 2);
	write(export_fd, "26", 2);
    write(export_fd, "65", 2);
    write(export_fd, "47", 2);

	//flow sensor
	//write(export_fd, "65", 2);
	//close the export file descriptor for export
	close(export_fd);

	//open the files for the gpio pins direction
	relays[LIGHTS] = open("/sys/class/gpio/gpio66/direction", O_WRONLY); //change all
	relays[LIGHTS_AUTO] = open("/sys/class/gpio/gpio67/direction", O_WRONLY);
	relays[HEADLIGHTS_WHITE] = open("/sys/class/gpio/gpio68/direction", O_WRONLY);
	relays[HEADLIGHTS_RED] = open("/sys/class/gpio/gpio69/direction", O_WRONLY);
    relays[HEAD_FANS] = open("/sys/class/gpio/gpio44/direction", O_WRONLY);
    relays[WATER_PUMP] = open("/sys/class/gpio/gpio45/direction", O_WRONLY);
    relays[ON_BUTTON] = open("/sys/class/gpio/gpio26/direction", O_WRONLY);
    relays[PELTIER] = open("/sys/class/gpio/gpio46/direction", O_WRONLY);
    relays[HIGH_CURRENT_LIVE] = open("/sys/class/gpio/gpio65/direction", O_WRONLY);
    relays[HIGH_CURRENT_GROUND] = open("/sys/class/gpio/gpio47/direction", O_WRONLY);

    //initialize them to be output pins initialized to zero
    write(relays[LIGHTS], "low", 3);
    write(relays[LIGHTS_AUTO], "low", 3);
    write(relays[HEADLIGHTS_WHITE], "low", 3);
    write(relays[HEADLIGHTS_RED], "low", 3);
    write(relays[HEAD_FANS], "low", 3);
    write(relays[WATER_PUMP], "low", 3);
    write(relays[PELTIER], "low", 3);
    
    write(relays[ON_BUTTON], "high", 4); // must start at high 
    write(relays[HIGH_CURRENT_LIVE], "high", 4);
    write(relays[HIGH_CURRENT_GROUND], "high", 4);

    //we want open the value file so close this one
    close(relays[LIGHTS]);
    close(relays[LIGHTS_AUTO]);
    close(relays[HEADLIGHTS_WHITE]);
    close(relays[HEADLIGHTS_RED]);
    close(relays[HEAD_FANS]);
    close(relays[WATER_PUMP]);
    close(relays[ON_BUTTON]);
    close(relays[PELTIER]);
    close(relays[HIGH_CURRENT_LIVE]);
    close(relays[HIGH_CURRENT_GROUND]);

    //open the values on read/write
    relays[LIGHTS] = open("/sys/class/gpio/gpio66/value", O_RDWR); // change
    relays[LIGHTS_AUTO] = open("/sys/class/gpio/gpio67/value", O_RDWR);
    relays[HEADLIGHTS_WHITE] = open("/sys/class/gpio/gpio68/value", O_RDWR);
    relays[HEADLIGHTS_RED] = open("/sys/class/gpio/gpio69/value", O_RDWR);
    relays[HEAD_FANS] = open("/sys/class/gpio/gpio44/value", O_RDWR);
    relays[WATER_PUMP] = open("/sys/class/gpio/gpio45/value", O_RDWR);
    relays[ON_BUTTON] = open("/sys/class/gpio/gpio26/value", O_RDWR);
    relays[PELTIER] = open("/sys/class/gpio/gpio46/value", O_RDWR);
    relays[HIGH_CURRENT_LIVE] = open("/sys/class/gpio/gpio65/direction", O_RDWR);
    relays[HIGH_CURRENT_GROUND] = open("/sys/class/gpio/gpio47/direction", O_RDWR);



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

		close(relays[LIGHTS]);
    	close(relays[LIGHTS_AUTO]);
    	close(relays[HEADLIGHTS_WHITE]);
    	close(relays[HEADLIGHTS_RED]);
    	close(relays[HEAD_FANS]);
    	close(relays[WATER_PUMP]);
    	close(relays[ON_BUTTON]);
    	close(relays[PELTIER]);
        close(relays[HIGH_CURRENT_LIVE]);
        close(relays[HIGH_CURRENT_GROUND]);



    	int unexport_fd = open("/sys/class/gpio/unexport", O_WRONLY); //change
		//export gpio pins
		write(unexport_fd, "66", 2);
		write(unexport_fd, "67", 2);
		write(unexport_fd, "68", 2);
		write(unexport_fd, "69", 2);
		write(unexport_fd, "44", 2);
		write(unexport_fd, "45", 2);
		write(unexport_fd, "46", 2);
		write(unexport_fd, "26", 2);
		//close the export file descriptor for export
		close(unexport_fd);

		close(temperature[HEAD]);
		close(temperature[ARMPITS]);
		close(temperature[CROTCH]);
		close(temperature[WATER]);
	}
}

int halosuit_relay_switch(unsigned int relay, int ps)
{
	if (is_initialized && relay < NUMBER_OF_RELAYS) {
		if (ps == HIGH) {
			write(relays[relay], "1", 1);
			lseek(relays[relay], 0, 0);
		} else if (ps == LOW) {
			write(relays[relay], "0", 1);
			lseek(relays[relay], 0, 0);
		} else {
			return -1;
		}
		return 0;
	}
	return -1;
}

int halosuit_relay_value(unsigned int relay, int *value)
{
	if (is_initialized && relay < NUMBER_OF_RELAYS) {
		char buf[2] = { 0 };
		read(relays[relay], buf, 1);
		*value = atoi(buf);
		lseek(relays[relay], 0, 0);
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
	int analog_fd = open("/sys/devices/bone_capemgr.9/slots", O_RDWR);
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

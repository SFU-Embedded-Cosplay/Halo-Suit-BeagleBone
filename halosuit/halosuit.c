#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

#include <halosuit/halosuit.h>

#define NUMBER_OF_RELAYS 8
#define NUMBER_OF_TEMP_SENSORS 4

//file descriptors
static int relays[NUMBER_OF_RELAYS];

//analog in file descriptors
static int temperature[NUMBER_OF_TEMP_SENSORS - 1]; //water temperature is taken care of separately

//FILE for pipe from readflow.py
static FILE* python_pipe;
static int flowrate = 0;
static double water_temp = 0.0f;
static char python_buffer[512];
static pthread_t python_thread_id;

//protects against using other functions early
static bool is_initialized = false;

static double analog_to_temperature(char *string)  
{  
	int value = atoi(string); 
	double millivolts = (value / 4096.0) * 1800;  
	double temp = (millivolts - 500.0) / 10.0;  
	return temp;  
}

static void *python_thread()
{
	python_pipe = popen("python /root/readflow.py", "r");
}

void halosuit_init()
{
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

	//flow sensor
	//write(export_fd, "65", 2);
	//close the export file descriptor for export
	close(export_fd);

	//open the files for the gpio pins direction
	relays[LIGHTS] = open("/sys/class/gpio/gpio66/direction", O_WRONLY);
	relays[LIGHTS_AUTO] = open("/sys/class/gpio/gpio67/direction", O_WRONLY);
	relays[HEADLIGHTS_WHITE] = open("/sys/class/gpio/gpio68/direction", O_WRONLY);
	relays[HEADLIGHTS_RED] = open("/sys/class/gpio/gpio69/direction", O_WRONLY);
    relays[HEAD_FANS] = open("/sys/class/gpio/gpio44/direction", O_WRONLY);
    relays[WATER_PUMP] = open("/sys/class/gpio/gpio45/direction", O_WRONLY);
    relays[WATER_FAN] = open("/sys/class/gpio/gpio26/direction", O_WRONLY);
    relays[PELTIER] = open("/sys/class/gpio/gpio46/direction", O_WRONLY);

    //initialize them to be output pins initialized to zero
    write(relays[LIGHTS], "low", 3);
    write(relays[LIGHTS_AUTO], "low", 3);
    write(relays[HEADLIGHTS_WHITE], "low", 3);
    write(relays[HEADLIGHTS_RED], "low", 3);
    write(relays[HEAD_FANS], "low", 3);
    write(relays[WATER_PUMP], "low", 3);
    write(relays[WATER_FAN], "low", 3);
    write(relays[PELTIER], "low", 3);

    //we want open the value file so close this one
    close(relays[LIGHTS]);
    close(relays[LIGHTS_AUTO]);
    close(relays[HEADLIGHTS_WHITE]);
    close(relays[HEADLIGHTS_RED]);
    close(relays[HEAD_FANS]);
    close(relays[WATER_PUMP]);
    close(relays[WATER_FAN]);
    close(relays[PELTIER]);

    //open the values on read/write
    relays[LIGHTS] = open("/sys/class/gpio/gpio66/value", O_RDWR);
    relays[LIGHTS_AUTO] = open("/sys/class/gpio/gpio67/value", O_RDWR);
    relays[HEADLIGHTS_WHITE] = open("/sys/class/gpio/gpio68/value", O_RDWR);
    relays[HEADLIGHTS_RED] = open("/sys/class/gpio/gpio69/value", O_RDWR);
    relays[HEAD_FANS] = open("/sys/class/gpio/gpio44/value", O_RDWR);
    relays[WATER_PUMP] = open("/sys/class/gpio/gpio45/value", O_RDWR);
    relays[WATER_FAN] = open("/sys/class/gpio/gpio26/value", O_RDWR);
    relays[PELTIER] = open("/sys/class/gpio/gpio46/value", O_RDWR);

    //open analog pins
    temperature[HEAD] = open("/sys/bus/iio/devices/iio:device0/in_voltage0_raw", O_RDONLY);
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
    	close(relays[WATER_FAN]);
    	close(relays[PELTIER]);

    	int unexport_fd = open("/sys/class/gpio/unexport", O_WRONLY);
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
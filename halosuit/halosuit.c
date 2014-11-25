#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdlib.h>

#include <halosuit/halosuit.h>

#define NUMBER_OF_RELAYS 8
#define NUMBER_OF_TEMP_SENSORS 4

//file descriptors
static int relay[NUMBER_OF_RELAYS];

//analog in file descriptors
static int temperature[NUMBER_OF_TEMP_SENSORS];

//protects against using other functions early
static bool is_initialized = false;

static double analog_to_temperature(char *string)  
{  
	int value = atoi(string); 
	double millivolts = (value / 4096.0) * 1800;  
	double temp = (millivolts - 500.0) / 10.0;  
	return temp;  
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
	//close the export file descriptor for export
	close(export_fd);

	//open the files for the gpio pins direction
	relay[LIGHTS] = open("/sys/class/gpio/gpio66/direction", O_WRONLY);
	relay[LIGHTS_AUTO] = open("/sys/class/gpio/gpio67/direction", O_WRONLY);
	relay[HEADLIGHTS_WHITE] = open("/sys/class/gpio/gpio68/direction", O_WRONLY);
	relay[HEADLIGHTS_RED] = open("/sys/class/gpio/gpio69/direction", O_WRONLY);
    relay[HEAD_FANS] = open("/sys/class/gpio/gpio44/direction", O_WRONLY);
    relay[WATER_PUMP] = open("/sys/class/gpio/gpio45/direction", O_WRONLY);
    relay[WATER_FAN] = open("/sys/class/gpio/gpio26/direction", O_WRONLY);
    relay[PELTIER] = open("/sys/class/gpio/gpio46/direction", O_WRONLY);

    //initialize them to be output pins initialized to zero
    write(relay[LIGHTS], "low", 3);
    write(relay[LIGHTS_AUTO], "low", 3);
    write(relay[HEADLIGHTS_WHITE], "low", 3);
    write(relay[HEADLIGHTS_RED], "low", 3);
    write(relay[HEAD_FANS], "low", 3);
    write(relay[WATER_PUMP], "low", 3);
    write(relay[WATER_FAN], "low", 3);
    write(relay[PELTIER], "low", 3);

    //we want open the value file so close this one
    close(relay[LIGHTS]);
    close(relay[LIGHTS_AUTO]);
    close(relay[HEADLIGHTS_WHITE]);
    close(relay[HEADLIGHTS_RED]);
    close(relay[HEAD_FANS]);
    close(relay[WATER_PUMP]);
    close(relay[WATER_FAN]);
    close(relay[PELTIER]);

    //open the values on read/write
    relay[LIGHTS] = open("/sys/class/gpio/gpio66/value", O_RDWR);
    relay[LIGHTS_AUTO] = open("/sys/class/gpio/gpio67/value", O_RDWR);
    relay[HEADLIGHTS_WHITE] = open("/sys/class/gpio/gpio68/value", O_RDWR);
    relay[HEADLIGHTS_RED] = open("/sys/class/gpio/gpio69/value", O_RDWR);
    relay[HEAD_FANS] = open("/sys/class/gpio/gpio44/value", O_RDWR);
    relay[WATER_PUMP] = open("/sys/class/gpio/gpio45/value", O_RDWR);
    relay[WATER_FAN] = open("/sys/class/gpio/gpio26/value", O_RDWR);
    relay[PELTIER] = open("/sys/class/gpio/gpio46/value", O_RDWR);

    //open analog pins
    temperature[HEAD] = open("/sys/bus/iio/devices/iio:device0/in_voltage0_raw", O_RDONLY);
    temperature[ARMPITS] = open("/sys/bus/iio/devices/iio:device0/in_voltage1_raw", O_RDONLY);
    temperature[CROTCH] = open("/sys/bus/iio/devices/iio:device0/in_voltage2_raw", O_RDONLY);
    temperature[WATER] = open("/sys/bus/iio/devices/iio:device0/in_voltage3_raw", O_RDONLY);
    is_initialized = true;
} 

void halosuit_exit()
{
	if (is_initialized) {
		is_initialized = false;

		close(relay[LIGHTS]);
    	close(relay[LIGHTS_AUTO]);
    	close(relay[HEADLIGHTS_WHITE]);
    	close(relay[HEADLIGHTS_RED]);
    	close(relay[HEAD_FANS]);
    	close(relay[WATER_PUMP]);
    	close(relay[WATER_FAN]);
    	close(relay[PELTIER]);

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

int halosuit_relay_switch(int r, int ps)
{
	if (is_initialized) {
		if (ps == HIGH) {
			write(relay[r], "1", 1);
			lseek(relay[r], 0, 0);
		} else if (ps == LOW) {
			write(relay[r], "0", 1);
			lseek(relay[r], 0, 0);
		} else {
			return -1;
		}
		return 0;
	}
	return -1;
}

int halosuit_relay_value(int r, int *value)
{
	if (is_initialized) {
		char buf[2] = { 0 };
		read(relay[r], buf, 1);
		*value = atoi(buf);
		lseek(relay[r], 0, 0);
		return 0;
	}
	return -1;
}

int halosuit_temperature_value(int location, double *temp)
{
	if (is_initialized) {
		char buf[5] = { 0 };
		read(temperature[location], buf, 4);
		*temp = analog_to_temperature(buf);
		lseek(temperature[location], 0, 0);
		return 0;
	}
	return -1;
}
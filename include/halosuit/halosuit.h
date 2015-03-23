/*
    halosuit.h
*/
#ifndef HALOSUIT
#define HALOSUIT

#define	LIGHTS            	0	//GPIO 66
#define	LIGHTS_AUTO       	1 	//GPIO 67
#define	HEADLIGHTS_WHITE  	2   //GPIO 68  
#define	HEADLIGHTS_RED 	  	3	//GPIO 69
#define	HEAD_FANS 		  	4   //GPIO 44
#define	WATER_PUMP 			5   //GPIO 45
#define	ON_BUTTON 		    6	//GPIO 26
#define	PELTIER			    7	//GPIO 46

//current draws for each device
//plus a base current draw


#define HIGH 				1
#define LOW					0

//for temperatures
#define HEAD 				0
#define ARMPITS				1
#define CROTCH				2
#define WATER 				3

#define VOLTAGE_1 1
#define VOLTAGE_2 2

void halosuit_init(); //sets up the file descriptors
void halosuit_exit(); //closes the file descriptors

//on success returns 0, -1 on failure
int halosuit_relay_switch(unsigned int relay, int state);

//changes value to relays value and returns 0 on success and -1 on failure
int halosuit_relay_value(unsigned int relay, int *value);

int halosuit_temperature_value(unsigned int location, double *temp);

int halosuit_flowrate(int *flow);

int halosuit_voltage_value(int battery, double *value);

int halosuit_current_draw_value(int *value);

int halosuit_heartrate(int *heart);

#endif

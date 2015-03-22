/*
    automationtestdata.h
    used to test automation when running test comment out #include <halosuit/halosuit.h>
    and include testcode/automationtestdata.h. This module will replace the functions:
    halosuit_temperature_value
    halosuit_flowrate
    with functions that return dummy data.
*/
#ifndef AUTOMATION_TEST
#define AUTOMATION_TEST

#define	LIGHTS            	0	//GPIO 66
#define	LIGHTS_AUTO       	1 	//GPIO 67
#define	HEADLIGHTS_WHITE  	2   //GPIO 68  
#define	HEADLIGHTS_RED 	  	3	//GPIO 69
#define	HEAD_FANS 		  	4   //GPIO 44
#define	WATER_PUMP 			5   //GPIO 45
#define	ON_BUTTON 		    6	//GPIO 26
#define	PELTIER			    7	//GPIO 46


#define HIGH 				1
#define LOW					0

//for temperatures
#define HEAD 				0
#define ARMPITS				1
#define CROTCH				2
#define WATER 				3

// Test Constants
#define TIME_SPAN 60            // how long until it runs to the next value is used
#define FLOW_TEST_ON_VALUE 15   // value returned by halosuit_flowrate when pump is on
#define FLOW_TEST_OFF_VALUE 0   // value returned by halosuit_flowrate when pump is off

void halosuit_init(); //sets up the file descriptors
void halosuit_exit(); //closes the file descriptors

//on success returns 0, -1 on failure
int halosuit_relay_switch(unsigned int relay, int state);

//changes value to relays value and returns 0 on success and -1 on failure
int halosuit_relay_value(unsigned int relay, int *value);

int halosuit_temperature_value(unsigned int location, double *temp);

int halosuit_flowrate(int *flow);

#endif

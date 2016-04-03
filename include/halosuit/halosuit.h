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
#define HIGH_CURRENT_LIVE   8   //GPIO 65
#define HIGH_CURRENT_GROUND 9   //GPIO 47

//current draws for each device
//plus a base current draw
#define PELTIER_DRAW        -4300 // milliamps
#define WATER_PUMP_DRAW     -250
#define HEAD_FANS_DRAW      -50

#define LOW_AMP_DRAW        -450
#define HEAD_LIGHTS_DRAW    -100
#define BODY_LIGHTS_DRAW    -500 // we need to firm this number up

#define VOLTAGE_1           1
#define VOLTAGE_2           2


#define HIGH 				1
#define LOW					0

//for temperatures
#define HEAD 				0
#define ARMPITS				1
#define CROTCH				2
#define WATER 				3

#define FLOWRATE_INITIAL_VALUE      0
#define WATER_TEMP_INITIAL_VALUE    10.0
// TODO: these defaults need to change when we get data on for them
#define VOLTAGE_1_INITIAL_VALUE     12.6
#define VOLTAGE_2_INITIAL_VALUE     12.0
#define HEARTRATE_INITIAL_VALUE     90

#define TURNIGY_8AH_VOLTAGE     12600
#define TURNIGY_2AH_VOLTAGE     12000

#define MAX_VOLTAGE             13000
#define MIN_VOLTAGE             10000

void halosuit_init(); //sets up the file descriptors
void halosuit_exit(); //closes the file descriptors

//on success returns 0, -1 on failure
int halosuit_relay_switch(unsigned int relay, int state);

//changes value to relays value and returns 0 on success and -1 on failure
int halosuit_relay_value(unsigned int relay, int *value);

int halosuit_temperature_value(unsigned int location, double *temp);

int halosuit_flowrate(int *flow);

int halosuit_voltage_value(unsigned int battery, int *value);

int halosuit_current_draw_value(unsigned int battery, int *value);

int halosuit_heartrate(int *heart);

#endif

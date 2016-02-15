/*
    halosuit.h
*/
#pragma once

// index in array to access each relay
#define	LIGHTS                  0
#define	LIGHTS_AUTO             1
#define	HEADLIGHTS_WHITE        2
#define	HEADLIGHTS_RED          3
#define	HEAD_FANS               4
#define	WATER_PUMP              5
#define	ON_BUTTON               6
#define	PELTIER                 7
#define HIGH_CURRENT_LIVE       8
#define HIGH_CURRENT_GROUND     9

#define	LIGHTS_PIN              66
#define	LIGHTS_AUTO_PIN         67
#define	HEADLIGHTS_WHITE_PIN    68  
#define	HEADLIGHTS_RED_PIN      69
#define	HEAD_FANS_PIN           44
#define	WATER_PUMP_PIN          45
#define	ON_BUTTON_PIN           26
#define	PELTIER_PIN             46
#define HIGH_CURRENT_LIVE_PIN   65
#define HIGH_CURRENT_GROUND_PIN 47


//current draws for each device
//plus a base current draw
#define PELTIER_DRAW            -4300 // milliamps
#define WATER_PUMP_DRAW         -250
#define HEAD_FANS_DRAW          -50

#define LOW_AMP_DRAW            -450
#define HEAD_LIGHTS_DRAW        -100
#define BODY_LIGHTS_DRAW        -500 // we need to firm this number up      

#define VOLTAGE_1               1
#define VOLTAGE_2               2


#define HIGH                    1
#define LOW                     0

//for temperatures
#define HEAD                    0
#define ARMPITS                 1
#define CROTCH                  2
#define WATER                   3

#define HEAD_TEMP_APIN          0
#define ARMPIT_TEMP_APIN        1
#define CROTCH_TEMP_APIN        2

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

// this code is run can be used to simulate the gpio, analog and arduino code through sockets

#ifdef MOCK_HARDWARE

#include <stdio.h>
#include <pthread.h>
#include <stdbool.h>

#include <halosuit/halosuit.h>

typedef union {
int intVal;
double dblVal;
} MockHW_t;

// create socket and constantly read values from it and store values
void halosuit_init() 
{
    //return NULL;
}

// close socket and kill thread
void halosuit_exit() 
{
}

// 
int halosuit_relay_switch(unsigned int relay, int ps)
{
    return 0;
}

int halosuit_relay_value(unsigned int relay, int *value)
{
    return 0;
}

int halosuit_temperature_value(unsigned int location, double *value)
{
    return 0;
}

int halosuit_flowrate(int *flow)
{
    return 0;
}

int halosuit_voltage_value(unsigned int battery, int *value)
{
    return 0;
}

int halosuit_current_draw_value(unsigned int battery, int *value)
{
    return 0;
}

int halosuit_heartrate(int *heart)
{
    return 0;
}

#endif

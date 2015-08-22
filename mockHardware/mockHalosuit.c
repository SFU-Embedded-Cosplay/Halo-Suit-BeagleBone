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
 
enum SUIT_HW_PARAMS {
	// RELAYS
	E_RELAYS_FIRST,

	E_LIGHTS = E_RELAYS_FIRST,
	E_RELAYS_LIGHTS_AUTO,
	E_LIGHTS_HEADWHITE,
	E_HEADLIGHTS_RED,
	E_HEAD_FANS,
	E_WATER_PUMP,
	E_ON_BUTTON,
	E_PELTIER,
	E_HIGH_CURRENT_LIVE,
	E_HIGH_CURRENT_GROUND,

	E_RELAYS_LAST = E_HIGH_CURRENT_GROUND,

	// Temperatures
	E_TEMP_FIST,

	E_TEMP_HEAD = E_TEMP_FIST,
	E_TEMP_ARMPIT,
	E_TEMP_CROTCH,
	E_TEMP_WATER,

	E_TEMP_LAST = E_TEMP_WATER,

	// Voltage
	E_VOLTAGE_FIRST,

	E_VOLTAGE_1 = E_VOLTAGE_FIRST,
	E_VOLTAGE_2,

	E_VOLTAGE_LAST = E_VOLTAGE_2,

	// Flowrate
	E_FLOWRATE,

	// Count them:
	E_NUM_HW_PARAMS
};

// Store the mock vales for HW.
MockHW_t mock_data[E_NUM_HW_PARAMS];

static bool is_initialized = false;
static pthread_t json_reader_thread_id;

static void *read_JSON() 
{
	return NULL;

}

// create socket and constantly read values from it and store values
void halosuit_init() 
{
	pthread_create(&json_reader_thread_id, NULL, &read_JSON, NULL);
	is_initialized = true;
    //return NULL;
}

// close socket and kill thread
void halosuit_exit() 
{
	if (is_initialized) {
		is_initialized = false;
	}
}

// 
int halosuit_relay_switch(unsigned int relay, int ps)
{
	int mock_index = relay + E_RELAYS_FIRST;

	if (is_initialized && relay < E_NUM_HW_PARAMS) {
		if(ps != HIGH && ps != LOW) {
			return -1
		}

		mock_data[mock_index].intVal = ps;
		return 0;
	}
	return -1;
}

int halosuit_relay_value(unsigned int relay, int *value)
{
	int mock_index = relay + E_RELAYS_FIRST;
	value = &mock_data[mock_index].intVal;

	return 0;
}

int halosuit_temperature_value(unsigned int location, double *value)
{
	int mock_index = location + E_TEMP_FIST;
	value = &mock_data[mock_index].dblVal;

    return 0;
}

int halosuit_flowrate(int *flow)
{
	if(!is_initialized) {
		return -1;
	}

	flow = &mock_data[E_FLOWRATE].intVal;
    return 0;
}

int halosuit_voltage_value(unsigned int battery, int *value)
{
	if(!is_initialized) {
		return -1;
	}
	int mock_index = E_VOLTAGE_FIRST + battery - 1;
	value = &mock_data[mock_index].intVal;
    return 0;
}

int halosuit_current_draw_value(unsigned int battery, int *value)
{
	// requires calculations
    return 0;
}

int halosuit_heartrate(int *heart)
{
    return 0;
}

#endif

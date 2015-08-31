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
	E_NUMBER_OF_RELAYS = E_RELAYS_LAST,

	// Temperatures
	E_TEMP_FIST,

	E_TEMP_HEAD = E_TEMP_FIST,
	E_TEMP_ARMPIT,
	E_TEMP_CROTCH,
	E_TEMP_WATER,

	E_TEMP_LAST = E_TEMP_WATER,
	E_NUMBER_OF_TEMP_SENSORS = E_TEMP_LAST - E_NUMBER_OF_RELAYS,

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

// TODO: integrate the bellow values into the SUIT_HW_PARAMS enum, or some other data structure so that they can be accessed properly.
// the bellow 5 values are gained from the python thread.  I need to better understand these and then better deal with them
// I dont beleive these work properly at this point.
// From halosuit.h file.  we should probably put these in the header file.
static int flowrate = 0;
static double water_temp = 10.0;
// TODO: these defaults need to change when we get data on for them
static double voltage1 = 12.6;
static double voltage2 = 12.0;
static int heartrate = 90;

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
int halosuit_relay_switch(unsigned int relay, int ps) //done
{
	int mock_index = relay + E_RELAYS_FIRST;

	if (is_initialized && relay < E_NUMBER_OF_RELAYS) {
		if(ps != HIGH && ps != LOW) {
			return -1
		}

		mock_data[mock_index].intVal = ps;
		return 0;
	}
	return -1;
}

int halosuit_relay_value(unsigned int relay, int *value) //done
{
	if(is_initialized && relay < E_NUMBER_OF_RELAYS) {
		int mock_index = relay + E_RELAYS_FIRST;
		value = &mock_data[mock_index].intVal;
		return 0;
	}

	return -1;
}

int halosuit_temperature_value(unsigned int location, double *value) //half done
{
	if(is_initialized && location < NUMBER_OF_TEMP_SENSORS) {
		if(location == WATER) {
			value = E_TEMP_WATER;
			// some of the values like temp_water is inside the .c file and so what I have may not work.
		}
		int mock_index = location + E_TEMP_FIST;
		value = &mock_data[mock_index].dblVal;
	}	

    return 0;
}

int halosuit_flowrate(int *flow) //done
{
	if(!is_initialized) {
		return -1;
	}

	flow = &mock_data[E_FLOWRATE].intVal;
    return 0;
}

int halosuit_voltage_value(unsigned int battery, int *value) //done - I think - This assumes battery is not an index but is instead a value around 12000
{
	if (is_initialized) {
        if (battery == TURNIGY_8_AH) {
            *value = (int)(mock_data[E_VOLTAGE_1].intVal * 1000);
        } 
        else if (battery == TURNIGY_2_AH) {
            *value = (int)(mock_data[E_VOLTAGE_2].intVal * 1000);
        } 
        else {
            return -1;
        }
		return 0;
	}
	return -1;
}

int halosuit_current_draw_value(unsigned int battery, int *value)
{
	// requires calculations
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

static int getIntValue(MockHW_t hardware) 
{
	return hardware.intVal;
}

static void setIntValue(MockHW_t hardware, int value) 
{
	hardware.intVal = value;
}

static double getDoubleValue(MockHW_t hardware) 
{
	return hardware.dblVal;
}

static void setDoubleValue(MockHW_t hardware, double value) 
{ 
	hardware.dblVal = value
}

int halosuit_heartrate(int *heart)
{
	if (is_initialized){
        *heart = heartrate;
        return 0;
    }
    return -1;
}

#endif

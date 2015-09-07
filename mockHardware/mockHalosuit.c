// this code is run can be used to simulate the gpio, analog and arduino code through sockets

#ifdef MOCK_HARDWARE

#include <stdio.h>
#include <pthread.h>
#include <stdbool.h>

#include <json/json.h>

#include <halosuit/halosuit.h>
#include <halosuit/stateofcharge.h>
#include <halosuit/logger.h>


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
	E_NUMBER_OF_VOLTAGES = E_VOLTAGE_LAST - E_NUMBER_OF_TEMP_SENSORS,

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

const char[] SOCKET_PATH = "localhost";

static void get_int_value(MockHW_t hardware, int* storage) 
{
	storage = &hardware.intVal;
}

static void set_int_value(MockHW_t hardware, int value) 
{
	hardware.intVal = value;
}

static void get_double_value(MockHW_t hardware, double* storage) 
{
	storage = &hardware.dblVal;
}

static void set_double_value(MockHW_t hardware, double value) 
{ 
	hardware.dblVal = value;
}

static void *read_JSON() 
{

	int socket_descriptor;

	sockaddr_in server;

	const int PORT = 8080;
	const char[] INTERNET_ADDRESS = "127.0.0.1";

	const int INPUT_BUFFER_LENGTH = 1024;
	char socket_input_buffer[INPUT_BUFFER_LENGTH];

	//setup socket (connect to test server)
	if ((socket_descriptor = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(1);
    }

    printf("Trying to connect to test server...\n");

    server.sin_addr.s_addr = inet_addr(INTERNET_ADDRESS);
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);

    if (connect(socket_descriptor, (struct sockaddr *)&server, sizeof(server)) < 0) {
        perror("connect");
        exit(1);
    }

    printf("Connected.\n");


	//read in json from local socket.
	while(Connected(socket_descriptor) && is_initialized) {

        if (recv(socket_descriptor, socket_input_buffer, INPUT_BUFFER_LENGTH, MSG_DONTWAIT) != -1) {
        	// parse and store json values
        	parser_parse(socket_input_buffer);
        	sleep(1000);
        	
        } else {
            printf("Server closed connection\n");
            exit(1); //should this be here?
        }
    }

    close(socket_descriptor);

	return NULL;

}

// create socket and constantly read values from it and store values
void halosuit_init() 
{
	pthread_create(&json_reader_thread_id, NULL, &read_JSON, NULL);
	is_initialized = true;
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
			return -1;
		}

		set_int_value(mock_data[mock_index], ps);
		return 0;
	}
	return -1;
}

int halosuit_relay_value(unsigned int relay, int *value) //done
{
	if(is_initialized && relay < E_NUMBER_OF_RELAYS) {
		int mock_index = relay + E_RELAYS_FIRST;
		get_int_value(mock_data[mock_index], value);
		return 0;
	}

	return -1;
}

int halosuit_temperature_value(unsigned int location, double *value) //half done
{
	int mock_index = E_TEMP_FIST + location;

	if(is_initialized && mock_index < E_NUMBER_OF_TEMP_SENSORS) {
		if(mock_index == E_TEMP_WATER) {
			value = &water_temp; // TODO: PYTHON - deal with this in however we decide is best to deal with the python values.
			// some of the values like temp_water is inside the .c file and so what I have may not work.
		}
		int mock_index = location + E_TEMP_FIST;
		get_double_value(mock_data[mock_index], value);
	}	

    return 0;
}

int halosuit_flowrate(int *flow) //done
{
	if(!is_initialized) {
		return -1;
	}

	flow = &flowrate; // TODO: PYTHON - deal with this in however we decide is best to deal with the python values.
	// get_int_value(mock_data[E_FLOWRATE], flow);
    return 0;
}

int halosuit_voltage_value(unsigned int battery, int *value) //done - I think - This assumes battery is not an index but is instead a value around 12000
{
	if (is_initialized) {
        if (battery == TURNIGY_8_AH) {
        	*value = (int)(voltage1 * 1000); // TODO: PYTHON - deal with this in however we decide is best to deal with the python values.
            // get_double_value(mock_data[E_VOLTAGE_1], value) * 1000); //Im not sure if this should be getDoubleValue or get_int_value
        } 
        else if (battery == TURNIGY_2_AH) {
        	*value = (int)(voltage2 * 1000); // TODO: PYTHON - deal with this in however we decide is best to deal with the python values.
            // getDoubleValue(mock_data[E_VOLTAGE_2], value) * 1000); // still not sure if this should be getInt or getDouble
        } 
        else {
            return -1;
        }
		return 0;
	}
	return -1;
}

// TODO: this probably does not work and needs some work to go over it
int halosuit_current_draw_value(unsigned int batteryID, int *current)
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

// heart rate is normally gotten from python script and thus I need to look into how to properly do this.
int halosuit_heartrate(int *heart)
{
	if (is_initialized){
        *heart = heartrate;
        return 0;
    }
    return -1;
}

#endif

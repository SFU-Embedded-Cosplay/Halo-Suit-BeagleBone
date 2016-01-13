// this code is run can be used to simulate the gpio, analog and arduino code through sockets

#ifdef MOCK_HARDWARE

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <stdbool.h>
#include <assert.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>

#include <json/parser.h>

#include <halosuit/halosuit.h>
#include <halosuit/stateofcharge.h>
#include <halosuit/logger.h>


typedef union {
	int intValue;
	double doubleValue;
} MockHW_t;

static const char SUIT_HARDWARE_NAMES[18][25] = {
	"main lights",
	"main lights auto",
	"head lights white",
	"head lights red",
	"head fans",
	"water pump",
	"on button???", //TODO: what is this
	"peltier",
	"battery current live",
	"battery current ground",
	"head temperature",
	"armpit temperature",
	"crotch temperature",
	"water temperature",
	"voltage 1",
	"voltage 2",
	"flowrate",
	"heartrate"
};

enum SUIT_HARDWARE_PARAMS {
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

	// heartrate
	E_HEARTRATE,

	// Count them:
	E_NUM_HW_PARAMS
};

// Store the mock vales for HW.
MockHW_t mock_data[E_NUM_HW_PARAMS];
pthread_mutex_t hardwareLock;

static bool is_initialized = false;
static pthread_t json_reader_thread_id;

static void get_int_value(MockHW_t hardware, int* storage)
{
	pthread_mutex_lock(&hardwareLock);
	*storage = hardware.intValue;
	pthread_mutex_unlock(&hardwareLock);
}

static void set_int_value(MockHW_t* hardware, int value)
{
	pthread_mutex_lock(&hardwareLock);
	hardware->intValue = value;
	pthread_mutex_unlock(&hardwareLock);
}

static void get_double_value(MockHW_t hardware, double* storage)
{
	pthread_mutex_lock(&hardwareLock);
	*storage = hardware.doubleValue;
	pthread_mutex_unlock(&hardwareLock);
}

static void set_double_value(MockHW_t* hardware, double value)
{
	pthread_mutex_lock(&hardwareLock);
	hardware->doubleValue = value;
	pthread_mutex_unlock(&hardwareLock);
}

static void *read_JSON()
{
	int socket_descriptor;
	struct sockaddr_in server;

	const int PORT = 8080;
	const char* INTERNET_ADDRESS = "192.168.7.1";

	const int INPUT_BUFFER_LENGTH = 1024;
	char socket_input_buffer[INPUT_BUFFER_LENGTH];

	int sleep_time_in_seconds = 1;


	//setup socket (connect to test server)
	if ((socket_descriptor = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		perror("socket in mockHardware failed to get socket descriptor");
		exit(1);
	}

	printf("Trying to connect to test server through address %s on port %d...\n", INTERNET_ADDRESS, PORT);
	server.sin_addr.s_addr = inet_addr(INTERNET_ADDRESS);
	server.sin_family = AF_INET;
	server.sin_port = htons(PORT);

	if (connect(socket_descriptor, (struct sockaddr *)&server, sizeof(server)) != 0) {
		perror("\nsocket in mockHardware failed to get connect");
		exit(1);
	}

	printf("Connected.\n");


	//read in json from local socket.
	while(is_initialized) {
		// printf("looping through thread");
		if (recv(socket_descriptor, socket_input_buffer, INPUT_BUFFER_LENGTH, MSG_DONTWAIT) != -1) {
			printf("received socket message: |%s|\n", socket_input_buffer);
			// parse and store json values
			parser_parse(socket_input_buffer);

			printf("\n\nPRINTING HARDWARE STATISTICS %d: \n", E_NUM_HW_PARAMS);

			for(int i = 0; i < E_NUM_HW_PARAMS; i++) {
				int intValue = 0;
				get_int_value(mock_data[i], &intValue);

				double doubleValue = 0.0;
				get_double_value(mock_data[i], &doubleValue);

				printf("value for item: %s = %f %d\n", SUIT_HARDWARE_NAMES[i], doubleValue, intValue);


			}

			sleep(sleep_time_in_seconds);
		}

		sleep(sleep_time_in_seconds);
	}

	close(socket_descriptor);

	return NULL;
}

// create socket and constantly read values from it and store values
void halosuit_init()
{
	assert(!is_initialized);

	// TODO: place constatnts in header file.
	// initialize values so that they are the same in halosuit.c
	// static int flowrate = 0;
	printf("init called\n");
	set_double_value(&mock_data[E_TEMP_WATER], 10.0);
	set_double_value(&mock_data[E_VOLTAGE_1], 12.6);
	set_double_value(&mock_data[E_VOLTAGE_2], 12.0);
	set_int_value(&mock_data[E_HEARTRATE], 90);



	is_initialized = true;
	pthread_create(&json_reader_thread_id, NULL, &read_JSON, NULL);
}

// close socket and kill thread
void halosuit_exit()
{
	assert(is_initialized);

	if (is_initialized) {
		is_initialized = false;
		pthread_join(json_reader_thread_id, NULL);
	}
}

//
int halosuit_relay_switch(unsigned int relay, int ps) //done
{
	int mock_index = relay + E_RELAYS_FIRST;

	if (is_initialized && relay < (E_RELAYS_LAST - E_RELAYS_FIRST + 1)) {
		if(ps != HIGH && ps != LOW) {
			return -1;
		}

		set_int_value(&mock_data[mock_index], ps);
		return 0;
	}
	return -1;
}

int halosuit_relay_value(unsigned int relay, int *value) //done
{
	if(is_initialized) {
		int mock_index = relay + E_RELAYS_FIRST;
		get_int_value(mock_data[mock_index], value);
		return 0;
	}

	return -1;
}

int halosuit_temperature_value(unsigned int location, double *value) //half done
{
	int mock_index = E_TEMP_FIST + location;

	if(is_initialized) {
		if(mock_index == E_TEMP_WATER) {
			get_double_value(mock_data[E_TEMP_WATER], value);
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

	get_int_value(mock_data[E_FLOWRATE], flow);
    return 0;
}

int halosuit_voltage_value(unsigned int battery, int *value) //done - I think - This assumes battery is not an index but is instead a value around 12000
{
	double temp_voltage_value = 0;

	if (is_initialized) {
        if (battery == TURNIGY_8_AH) {
        	get_double_value(mock_data[E_VOLTAGE_1], &temp_voltage_value);
        	*value = (int)(temp_voltage_value * 1000);
            // get_double_value(mock_data[E_VOLTAGE_1], value) * 1000); //Im not sure if this should be getDoubleValue or get_int_value
        }
        else if (battery == TURNIGY_2_AH) {
        	get_double_value(mock_data[E_VOLTAGE_2], &temp_voltage_value);
        	*value = (int)(temp_voltage_value * 1000);
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
	assert(is_initialized);

	if (is_initialized){
        get_int_value(mock_data[E_HEARTRATE], heart);
        return 0;
    }
    return -1;
}

#endif

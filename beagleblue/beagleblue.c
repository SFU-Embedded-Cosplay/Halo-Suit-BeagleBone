/*
   beagleblue.c
*/
#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>
#include <pthread.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>

#include <beagleblue/beagleblue.h>
#include <beagleblue/bluetooth.h>
#include <config/config.h>
#include <halosuit/logger.h>
#include <halosuit/systemstatus.h>
//this is hard coded on both ends
#define ANDROID_PORT 3
#define GLASS_PORT 2

static connection_t android_connection = { 
	.server_socket = -1, 
	.client = -1, 
	.port = ANDROID_PORT,
	.is_client_connected = false, 
	.is_sending = false 
};

static connection_t glass_connection = { 
	.server_socket = -1, 
	.client = -1, 
	.port = GLASS_PORT,
	.is_client_connected = false, 
	.is_sending = false 
};

static bool beagleblue_is_done;

static pthread_t android_connect_thread_id;
static pthread_t android_recv_thread_id;
static pthread_t android_send_thread_id;
static pthread_t glass_connect_thread_id;
static pthread_t glass_recv_thread_id;
static pthread_t glass_send_thread_id;

static pthread_mutex_t android_send_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t glass_send_mutex = PTHREAD_MUTEX_INITIALIZER;

static char android_mac_addr[MAX_BUF_SIZE] = { 0 };
static char glass_mac_addr[MAX_BUF_SIZE] = { 0 };

static bool android_configured;
static bool glass_configured;

//if sockets are initialized close them before this
static void beagleblue_connect(connection_t *connection)
{
	char buf[20];

	bluetooth_bind_socket(connection);

	// put socket into listening mode
	if (connection->port == ANDROID_PORT && android_configured) {
		while (true) {
			bluetooth_connect_client(connection, buf);

			if (strcmp(buf, android_mac_addr) == 0) {
				break;
			} else {
				bluetooth_disconnect_client(connection);
				fprintf(stdout, "denied connection from %s\n", buf);
				fflush(stdout);

				logger_log("denied connection");
			}
		}
	} else if (connection->port == GLASS_PORT && glass_configured) {
		while (true) {
			bluetooth_connect_client(connection, buf);

			if (strcmp(buf, glass_mac_addr) == 0) {
				break;
			} else {
				bluetooth_disconnect_client(connection);
				fprintf(stdout, "denied connection from %s\n", buf);
				fflush(stdout);

				logger_log("denied connection");
			}
		}
	} else {
		bluetooth_connect_client(connection, buf);
	}

	fprintf(stdout, "accepted connection from %s\n", buf);
	fflush(stdout);

	if(connection->port == ANDROID_PORT) {
		logger_log("accepted connection from %s on Android port", buf);
	} else if(connection->port == GLASS_PORT) {
		logger_log("accepted connection from %s on Glass port", buf);
	} else {
		logger_log("accepted connection from %s on unknown port", buf);
	}
}

static void *android_connect_thread()
{
	beagleblue_connect(&android_connection);
	android_connection.is_client_connected = true;

	if (glass_connection.is_client_connected) {
		bluetooth_set_bluetooth_mode(SCAN_DISABLED, &beagleblue_is_done);
	}
	return NULL;
}

static void *glass_connect_thread()
{
	beagleblue_connect(&glass_connection);
	glass_connection.is_client_connected = true;

	if (android_connection.is_client_connected) {
		bluetooth_set_bluetooth_mode(SCAN_DISABLED, &beagleblue_is_done);
	}
	return NULL;
}

static void *android_recv_thread(void *callback)
{
	void (*on_receive)(char *) = callback;
	//init up here
	while(!beagleblue_is_done) {
		if (android_connection.is_client_connected) {

			if (bluetooth_receive_message(&android_connection) != -1) {
				on_receive(android_connection.receive_buffer);
			}
		}
	}
	
	logger_log("Android  receive thread ended");
	return NULL;
}

static void *android_send_thread()
{
	while(!beagleblue_is_done) {
		if (android_connection.is_client_connected) {
			if (android_connection.is_sending) {
				int start_time = time(NULL);
				int current_time = start_time;
				while (current_time - start_time < TIMEOUT) {
					if(bluetooth_send_message(android_connection) == -1) {
						current_time = time(NULL);
					} else {
						break;
					}
				}
				if (current_time - start_time >= TIMEOUT) {
					printf("Android Timed Out\n");
					fflush(stdout);
					logger_log("Android Timed Out because current_time(%d) - start_time(%d) >= TIMEOUT", current_time, start_time, TIMEOUT);
					android_connection.is_client_connected = false;
					//close(android_sock);
					bluetooth_disconnect_client(&android_connection);
					bluetooth_set_bluetooth_mode(SCAN_PAGE | SCAN_INQUIRY, &beagleblue_is_done);
					pthread_create(&android_connect_thread_id, NULL, &android_connect_thread, NULL);
				}
				android_connection.is_sending = false;
				pthread_mutex_unlock(&android_send_mutex);
			}
		}
	}
	
	logger_log("Android send thread ended");
	return NULL;
}

static void *glass_recv_thread(void *callback)
{
	void (*on_receive)(char *) = callback;
	//init up here
	while(!beagleblue_is_done) {
		if (glass_connection.is_client_connected) {

			if (bluetooth_receive_message(&glass_connection) != -1) {
				on_receive(glass_connection.receive_buffer);
			}
		}
	}
	
	logger_log("Glass receive thread ending");
	return NULL;
}

static void *glass_send_thread()
{
	while(!beagleblue_is_done) {
		if (glass_connection.is_client_connected) {
			if (glass_connection.is_sending) {

				int start_time = time(NULL);
				int current_time = start_time;

				while (current_time - start_time < TIMEOUT) {
					if(bluetooth_send_message(glass_connection) == -1) {
						current_time = time(NULL);
					} else {
						break;
					}
				}

				if (current_time - start_time >= TIMEOUT) {
					printf("Glass Timed Out\n");
					fflush(stdout);
					logger_log("Glass Timed Out because current_time(%d) - start_time(%d) >= TIMEOUT", current_time, start_time, TIMEOUT);
					glass_connection.is_client_connected = false;
					//close(glass_sock);
					bluetooth_disconnect_client(&glass_connection);
					bluetooth_set_bluetooth_mode(SCAN_PAGE | SCAN_INQUIRY, &beagleblue_is_done);
					pthread_create(&glass_connect_thread_id, NULL, &glass_connect_thread, NULL);
				}

				glass_connection.is_sending = false;
				pthread_mutex_unlock(&glass_send_mutex);
			}
		}
	}
	
	logger_log("Glass send thread ending");
	return NULL;
}

void beagleblue_init(void (*on_receive)(char *))
{
	beagleblue_is_done = false;
	bluetooth_set_bluetooth_mode(SCAN_INQUIRY | SCAN_PAGE, &beagleblue_is_done);
	//
	if (!beagleblue_is_done) {
		android_configured = config_get_string("Bluetooth", "android", android_mac_addr, MAX_BUF_SIZE) == 0;
		glass_configured = config_get_string("Bluetooth", "glass", glass_mac_addr, MAX_BUF_SIZE) == 0;

		printf("Bluetooth Discoverable\n");
		logger_log("Bluetooth Discoverable");
		pthread_create(&android_connect_thread_id, NULL, &android_connect_thread, NULL);
		pthread_create(&glass_connect_thread_id, NULL, &glass_connect_thread, NULL);
		pthread_create(&android_send_thread_id, NULL, &android_send_thread, NULL);
		pthread_create(&android_recv_thread_id, NULL, &android_recv_thread, on_receive);
		pthread_create(&glass_send_thread_id, NULL, &glass_send_thread, NULL);
		pthread_create(&glass_recv_thread_id, NULL, &glass_recv_thread, on_receive);

		systemstatus_set_status(BLUETOOTH_CONNECTED);
	}
	return;
}

void beagleblue_exit()
{
	beagleblue_is_done = true;
	beagleblue_join();
	bluetooth_disconnect_client(&android_connection);
	bluetooth_close_server_socket(&android_connection);

	bluetooth_disconnect_client(&glass_connection);
	bluetooth_close_server_socket(&glass_connection);
	return;
}

int beagleblue_glass_send(char *buf)
{
	if (glass_connection.is_client_connected) {
		//gets unlocked inside the glass send thread
		pthread_mutex_lock(&glass_send_mutex);
		memset(glass_connection.send_buffer, 0, BUFFER_SIZE);
		strncpy(glass_connection.send_buffer, buf, BUFFER_SIZE);
		glass_connection.is_sending = true;
		//needs to be thread safe

		return 0;
	}
	return -1;
}

int beagleblue_android_send(char *buf)
{
	if (android_connection.is_client_connected) {
		//gets unlock inside android send thread
		pthread_mutex_lock(&android_send_mutex);
		memset(android_connection.send_buffer, 0, BUFFER_SIZE);
		strncpy(android_connection.send_buffer, buf, BUFFER_SIZE);
		android_connection.is_sending = true; //modify to be thread safe
		return 0;
	}
	return -1;
}

void beagleblue_join()
{
	pthread_join(glass_recv_thread_id, NULL);
	pthread_join(glass_send_thread_id, NULL);
	pthread_join(android_recv_thread_id, NULL);
	pthread_join(android_send_thread_id, NULL);
}

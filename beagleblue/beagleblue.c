/*
   beagleblue.c
*/
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>
#include <bluetooth/rfcomm.h>
#include <sys/ioctl.h>
#include <stdbool.h>
#include <string.h>
#include <pthread.h>

#include <beagleblue/beagleblue.h>
#include <config/config.h>
#include <halosuit/logger.h>
//this is hard coded on both ends
#define ANDROID_PORT 3
#define GLASS_PORT 2

static char android_send_buffer[BUFFER_SIZE];
static char android_recv_buffer[BUFFER_SIZE];
static char glass_send_buffer[BUFFER_SIZE];
static char glass_recv_buffer[BUFFER_SIZE];


static bool android_is_sending = false;
static bool glass_is_sending = false;

static bool beagleblue_is_done;
static bool android_is_connected = false;
static bool glass_is_connected = false;

static pthread_t android_connect_thread_id;
static pthread_t android_recv_thread_id;
static pthread_t android_send_thread_id;
static pthread_t glass_connect_thread_id;
static pthread_t glass_recv_thread_id;
static pthread_t glass_send_thread_id;

static pthread_mutex_t android_send_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t glass_send_mutex = PTHREAD_MUTEX_INITIALIZER;

static int android_sock = -1, android_client = -1;
static int glass_sock = -1, glass_client = -1;

static char android_mac_addr[MAX_BUF_SIZE] = { 0 };
static char glass_mac_addr[MAX_BUF_SIZE] = { 0 };

static bool android_configured;
static bool glass_configured;

//consider converting to macro
static void set_bluetooth_mode(uint32_t mode)
{
	int sock = socket(AF_BLUETOOTH, SOCK_RAW, BTPROTO_HCI);
	int dev_id = hci_get_route(NULL);

	struct hci_dev_req dr;

	dr.dev_id  = dev_id;
	dr.dev_opt = mode;
	if (ioctl(sock, HCISETSCAN, (unsigned long) &dr) < 0) {
		beagleblue_is_done = true;
		fprintf(stderr, "Can't set scan mode on hci%d: %s (%d)\n", dev_id, strerror(errno), errno);
        char string[100];
        sprintf(string, "Can't set scan mode on hci%d: %s (%d)\n", dev_id, strerror(errno), errno);
        logger_log(string);
		logger_log("WARNING: This is a fatal error that will prevent bluetooth from working\n");
    }

    close(sock);
}

//if sockets are initialized close them before this
static void beagleblue_connect(int *sock, int *client, uint8_t channel)
{
	struct sockaddr_rc loc_addr = { 0 }, rem_addr = { 0 };
	socklen_t opt = sizeof(rem_addr);
	char buf[20];

	if (*sock == -1) {
		//initialize socket
		*sock = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);

		//bind socket to local bluetooth device
		loc_addr.rc_family = AF_BLUETOOTH;
		loc_addr.rc_bdaddr = *BDADDR_ANY;
		loc_addr.rc_channel = channel;
		bind(*sock, (struct sockaddr *)&loc_addr, sizeof(loc_addr));
	}
	// put socket into listening mode
    if (channel == ANDROID_PORT && android_configured) {
    	while (true) {
    		listen(*sock, 1);

			*client = accept(*sock, (struct sockaddr *)&rem_addr, &opt);
    		ba2str( &rem_addr.rc_bdaddr, buf );

    		if (strcmp(buf, android_mac_addr) == 0) {
    			break;
    		} else {
    			close(*client);
    			fprintf(stdout, "denied connection from %s\n", buf);
				fflush(stdout);

                logger_log("denied connection\n");
    		}
    	}
    } else if (channel == GLASS_PORT && glass_configured) {
    	while (true) {
    		listen(*sock, 1);

			*client = accept(*sock, (struct sockaddr *)&rem_addr, &opt);
    		ba2str( &rem_addr.rc_bdaddr, buf );

    		if (strcmp(buf, glass_mac_addr) == 0) {
    			break;
    		} else {
    			close(*client);
    			fprintf(stdout, "denied connection from %s\n", buf);
				fflush(stdout);

                logger_log("denied connection\n");
    		}
    	}
    } else {
    	listen(*sock, 1);

	*client = accept(*sock, (struct sockaddr *)&rem_addr, &opt);

    }
	ba2str( &rem_addr.rc_bdaddr, buf);
	fprintf(stdout, "accepted connection from %s\n", buf);
	fflush(stdout);

    logger_log("accepted connection\n");
}

static void *android_connect_thread()
{
	beagleblue_connect(&android_sock, &android_client, ANDROID_PORT);
	android_is_connected = true;
	if (glass_is_connected) {
		set_bluetooth_mode(SCAN_DISABLED);
	}
    return NULL;
}

static void *glass_connect_thread()
{
	beagleblue_connect(&glass_sock, &glass_client, GLASS_PORT);
	glass_is_connected = true;
	if (android_is_connected) {
		set_bluetooth_mode(SCAN_DISABLED);
	}
    return NULL;
}

static void *android_recv_thread(void *callback)
{
	void (*on_receive)(char *) = callback;
	//init up here
	while(!beagleblue_is_done) {
		if (android_is_connected) {
			memset(android_recv_buffer, 0, BUFFER_SIZE); //clear the buffer

			if (recv(android_client, android_recv_buffer, BUFFER_SIZE, MSG_DONTWAIT) != -1) {
				on_receive(android_recv_buffer);
			}
		}
	}
	return NULL;
}

static void *android_send_thread()
{
	while(!beagleblue_is_done) {
		if (android_is_connected) {
			if (android_is_sending) {
				int start_time = time(NULL);
				int current_time = start_time;
				while (current_time - start_time < TIMEOUT) {
					if(send(android_client, android_send_buffer, strlen(android_send_buffer), MSG_DONTWAIT) == -1) {
						current_time = time(NULL);
					} else {
						break;
					}
				}
				if (current_time - start_time == TIMEOUT) {
					printf("Android Timed Out\n");
					fflush(stdout);
                    logger_log("Android Timed Out\n");
					android_is_connected = false;
					//close(android_sock);
					close(android_client);
					set_bluetooth_mode(SCAN_PAGE | SCAN_INQUIRY);
					pthread_create(&android_connect_thread_id, NULL, &android_connect_thread, NULL);
				}
				android_is_sending = false;
				pthread_mutex_unlock(&android_send_mutex);
			}
		}
	}
	return NULL;
}

static void *glass_recv_thread(void *callback)
{
	void (*on_receive)(char *) = callback;
	//init up here
	while(!beagleblue_is_done) {
		if (glass_is_connected) {
			memset(glass_recv_buffer, 0, BUFFER_SIZE); //clear the buffer

			if (recv(glass_client, glass_recv_buffer, BUFFER_SIZE, MSG_DONTWAIT) != -1) {
				on_receive(glass_recv_buffer);
			}
		}
	}
	return NULL;
}

static void *glass_send_thread()
{
	while(!beagleblue_is_done) {
		if (glass_is_connected) {
			if (glass_is_sending) {

				int start_time = time(NULL);
				int current_time = start_time;

				while (current_time - start_time < TIMEOUT) {
					if(send(glass_client, glass_send_buffer, strlen(glass_send_buffer), MSG_DONTWAIT) == -1) {
						current_time = time(NULL);
					} else {
						break;
					}
				}

				if (current_time - start_time == TIMEOUT) {
					printf("Glass Timed Out\n");
					fflush(stdout);
                    logger_log("Glass Timed Out\n");
					glass_is_connected = false;
					//close(glass_sock);
					close(glass_client);
					set_bluetooth_mode(SCAN_PAGE | SCAN_INQUIRY);
					pthread_create(&glass_connect_thread_id, NULL, &glass_connect_thread, NULL);
				}

				glass_is_sending = false;
				pthread_mutex_unlock(&glass_send_mutex);
			}
		}
	}
	return NULL;
}

void beagleblue_init(void (*on_receive)(char *))
{
	beagleblue_is_done = false;
	set_bluetooth_mode(SCAN_INQUIRY | SCAN_PAGE);
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
	}
	return;
}

void beagleblue_exit()
{
	beagleblue_is_done = true;
	beagleblue_join();
	close(android_client);
	close(android_sock);
	close(glass_client);
	close(glass_sock);
	return;
}

int beagleblue_glass_send(char *buf)
{
	if (glass_is_connected) {
		//gets unlocked inside the glass send thread
		pthread_mutex_lock(&glass_send_mutex);
		memset(glass_send_buffer, 0, BUFFER_SIZE);
		strncpy(glass_send_buffer, buf, BUFFER_SIZE);
		glass_is_sending = true;
		//needs to be thread safe

		return 0;
	}
	return -1;
}

int beagleblue_android_send(char *buf)
{
	if (android_is_connected) {
		//gets unlock inside android send thread
		pthread_mutex_lock(&android_send_mutex);
		memset(android_send_buffer, 0, BUFFER_SIZE);
		strncpy(android_send_buffer, buf, BUFFER_SIZE);
		android_is_sending = true; //modify to be thread safe
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

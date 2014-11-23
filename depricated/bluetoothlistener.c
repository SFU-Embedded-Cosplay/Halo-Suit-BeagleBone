/*
    bluetoothlistener.c
*/

#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/sdp.h>
#include <bluetooth/sdp_lib.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>
#include <bluetooth/rfcomm.h>
#include <sys/ioctl.h>
#include <stdbool.h>
#include <string.h>

#include "bluetoothlistener.h"
#include "processor.h"

#define MAC_ADDRESS "A4:9A:58:83:49:DE"

static void setBluetoothMode(uint32_t mode){
    int sock = socket(AF_BLUETOOTH, SOCK_RAW, BTPROTO_HCI);
    int dev_id = hci_get_route(NULL);

    struct hci_dev_req dr;

    dr.dev_id  = dev_id;
    dr.dev_opt = mode;
    if (ioctl(sock, HCISETSCAN, (unsigned long) &dr) < 0) {
	fprintf(stderr, "Can't set scan mode on hci%d: %s (%d)\n", dev_id, strerror(errno), errno);
    }

    close(sock);
}

void* Listener_thread()
{
    bool done = false;
    struct sockaddr_rc loc_addr = { 0 };
    struct sockaddr_rc rem_addr = { 0 };
    char buffer[BUFFER_SIZE] = { 0 };
    int sock = 0;
    int client = 0;
    int bytes_read = 0;
    socklen_t opt = sizeof(rem_addr);

    // allocate socket
    sock = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);

    // bind socket to port 1 of the first available 
    // local bluetooth adapter
    loc_addr.rc_family = AF_BLUETOOTH;
    loc_addr.rc_bdaddr = *BDADDR_ANY;
    loc_addr.rc_channel = (uint8_t) 2;
    bind(sock, (struct sockaddr *)&loc_addr, sizeof(loc_addr));

    setBluetoothMode(SCAN_INQUIRY | SCAN_PAGE);
    while (1) {
	// put socket into listening mode
	listen(sock, 1);
    
	// accept one connection
	client = accept(sock, (struct sockaddr *)&rem_addr, &opt);
    
	ba2str( &rem_addr.rc_bdaddr, buffer );
	if (strncmp(MAC_ADDRESS, buffer, 17) == 0) break;
    }
    setBluetoothMode(SCAN_DISABLED);
    fprintf(stdout, "accepted connection from %s\n", buffer);
    memset(buffer, 0, sizeof(buffer));

    while (!done) {
	bytes_read = read(client, buffer, sizeof(buffer));
	if (bytes_read == 1) {
	    Processor_enqueue(buffer);
	}
	bytes_read = 0;
    }	
 
    return NULL;
}

#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>
#include <bluetooth/rfcomm.h>
#include <stdbool.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <unistd.h>




#include <beagleblue/bluetooth.h>
#include <halosuit/systemstatus.h>
#include <halosuit/logger.h>

// bind the connection to a new server socket
void bluetooth_bind_socket(connection_t *connection) 
{
	struct sockaddr_rc local_address = { 0 };

	if (connection->server_socket == -1) {
		//initialize socket
		connection->server_socket = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);

		//bind socket to local bluetooth device
		local_address.rc_family = AF_BLUETOOTH;
		local_address.rc_bdaddr = *BDADDR_ANY;
		local_address.rc_channel = connection->port;
		bind(connection->server_socket, (struct sockaddr *)&local_address, sizeof(local_address));
	}
}

void bluetooth_connect_client(connection_t *connection, char* buffer) 
{
	struct sockaddr_rc remote_address = { 0 };
	socklen_t opt = sizeof(remote_address);

	listen(connection->server_socket, 1);

	connection->client = accept(connection->server_socket, (struct sockaddr *)&remote_address, &opt);
	ba2str( &remote_address.rc_bdaddr, buffer);
}

// returns the same values as a call to send() would return
int bluetooth_send_message(const connection_t connection) 
{
	return send(connection.client, connection.send_buffer, strlen(connection.send_buffer), MSG_DONTWAIT);
}

// returns the same values as a call to recv() would return
int bluetooth_receive_message(connection_t *connection) 
{
	memset(connection->receive_buffer, 0, BUFFER_SIZE); //clear the buffer
	return recv(connection->client, connection->receive_buffer, BUFFER_SIZE, MSG_DONTWAIT);
}

void bluetooth_set_bluetooth_mode(uint32_t mode, bool *scan_mode_error)
{
	int bluetooth_socket = socket(AF_BLUETOOTH, SOCK_RAW, BTPROTO_HCI);
	int dev_id = hci_get_route(NULL);

	struct hci_dev_req dr;

	dr.dev_id  = dev_id;
	dr.dev_opt = mode;
	if (ioctl(bluetooth_socket, HCISETSCAN, (unsigned long) &dr) < 0) {
		*scan_mode_error = true;
		fprintf(stderr, "Can't set scan mode on hci%d: %s (%d)\n", dev_id, strerror(errno), errno);
		logger_log("Can't set scan mode on hci%d: %s (%d)", dev_id, strerror(errno), errno);
		logger_log("WARNING: This is a fatal error that will prevent bluetooth from working\n");

		systemstatus_set_status(BLUETOOTH_ERROR);
	}

	close(bluetooth_socket);
}
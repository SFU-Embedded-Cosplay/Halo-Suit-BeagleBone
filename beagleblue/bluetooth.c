#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>
#include <bluetooth/rfcomm.h>
#include <stdbool.h>


#include <beagleblue/bluetooth.h>

void bluetooth_bind_socket(connection_t *connection) 
{
	struct sockaddr_rc loc_addr = { 0 };

	if (connection->socket == -1) {
		//initialize socket
		connection->socket = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);

		//bind socket to local bluetooth device
		loc_addr.rc_family = AF_BLUETOOTH;
		loc_addr.rc_bdaddr = *BDADDR_ANY;
		loc_addr.rc_channel = connection->channel;
		bind(connection->socket, (struct sockaddr *)&loc_addr, sizeof(loc_addr));
	}
}

void bluetooth_connect_client(connection_t *connection, char* buffer) 
{
	struct sockaddr_rc rem_addr = { 0 };
	socklen_t opt = sizeof(rem_addr);

	listen(connection->socket, 1);

	connection->client = accept(connection->socket, (struct sockaddr *)&rem_addr, &opt);
	ba2str( &rem_addr.rc_bdaddr, buffer);
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
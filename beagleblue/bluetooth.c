#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>
#include <bluetooth/rfcomm.h>
#include <stdbool.h>


#include <beagleblue/bluetooth.h>

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
		local_address.rc_channel = connection->channel;
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

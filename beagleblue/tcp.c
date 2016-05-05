#ifdef TCP_CONNECTION

#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>
#include <bluetooth/rfcomm.h>
#include <stdbool.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <unistd.h>
#include <sys/socket.h>
#include <stdio.h>
#include <sys/types.h>
#include <arpa/inet.h>

#include <beagleblue/bluetooth.h>
#include <halosuit/systemstatus.h>
#include <halosuit/logger.h>

void bluetooth_bind_socket(connection_t *connection) 
{
	struct sockaddr_in local_address;

	if (connection->server_socket == -1) {
		//initialize socket
		connection->server_socket = socket(AF_INET, SOCK_STREAM, 0);

		//bind socket to local bluetooth device
		local_address.sin_family = AF_INET;
		local_address.sin_port = htons(6060);
		local_address.sin_addr.s_addr = INADDR_ANY;
		bind(connection->server_socket, (struct sockaddr *)&local_address, sizeof(local_address));
	}
}

void bluetooth_connect_client(connection_t *connection, char* buffer) 
{
	listen(connection->server_socket, 20);

	struct sockaddr_in client_address;
	int address_length = sizeof(client_address);


	connection->client = accept(connection->server_socket, (struct sockaddr *)&client_address, &address_length);


}

void bluetooth_close_server_socket(connection_t *connection) 
{
	close(connection->server_socket);
}

void bluetooth_disconnect_client(connection_t *connection) 
{
	close(connection->client);
}

int bluetooth_send_message(const connection_t connection) 
{
	return send(connection.client, connection.send_buffer, strlen(connection.send_buffer), MSG_DONTWAIT);
}

int bluetooth_receive_message(connection_t *connection) 
{
	memset(connection->receive_buffer, 0, BUFFER_SIZE); //clear the buffer
	return recv(connection->client, connection->receive_buffer, BUFFER_SIZE, MSG_DONTWAIT);
}


void bluetooth_set_bluetooth_mode(uint32_t mode, bool *scan_mode_error) 
{
	// do nothing
}

#endif
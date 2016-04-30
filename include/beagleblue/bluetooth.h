#ifndef BLUETOOTH
#define BLUETOOTH

#define BUFFER_SIZE 1024

typedef struct connection {
	int socket; // -1
	int client; // -1
	uint8_t channel;
	bool is_connected; // false

	bool is_sending; // false
	char send_buffer[BUFFER_SIZE];
	char receive_buffer[BUFFER_SIZE];
} connection_t;

void bluetooth_bind_socket(connection_t *connection);
void bluetooth_connect_client(connection_t *connection, char* buffer);
#endif
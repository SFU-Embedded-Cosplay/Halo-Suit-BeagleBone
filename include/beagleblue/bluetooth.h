#ifndef BLUETOOTH
#define BLUETOOTH

#define BUFFER_SIZE 1024

typedef struct connection {
	int server_socket; // -1
	int client; // -1
	uint8_t port; // also refered to as channel in some references
	bool is_client_connected; // false

	bool is_sending; // false
	char send_buffer[BUFFER_SIZE];
	char receive_buffer[BUFFER_SIZE];
} connection_t;

void bluetooth_bind_socket(connection_t *connection);
void bluetooth_connect_client(connection_t *connection, char* buffer);

void bluetooth_close_server_socket(connection_t *connection);
void bluetooth_disconnect_client(connection_t *connection);

int bluetooth_send_message(const connection_t connection);
int bluetooth_receive_message(connection_t *connection);


void bluetooth_set_bluetooth_mode(uint32_t mode, bool *scan_mode_error);
#endif
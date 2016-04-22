#ifndef SYSTEM_STATUS
#define SYSTEM_STATUS

typedef enum status {
	BLUETOOTH_CONNECTED,
	BLUETOOTH_ERROR
} status_t;

void systemstatus_init();

void systemstatus_set_status(status_t status);
status_t systemstatus_get_status();

#endif

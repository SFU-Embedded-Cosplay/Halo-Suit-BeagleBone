#ifndef SYSTEM_STATUS
#define SYSTEM_STATUS

typedef enum status {
	NO_STATUS = 0,
	BOOT_SUCCESS = 1,
	BLUETOOTH_CONNECTED = 2,
	BLUETOOTH_ERROR = 3,
	BLUETOOTH_RESETTING = 4,
	SYSTEM_CRASH = 15
} status_t;

void systemstatus_init();

void systemstatus_set_status(status_t status);
status_t systemstatus_get_status();

#endif

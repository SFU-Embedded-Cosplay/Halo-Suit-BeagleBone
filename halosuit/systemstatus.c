#include <halosuit/systemstatus.h>

static status_t currentStatus = NO_STATUS;


void systemstatus_init() {
	led0.usr = 0;
}

void systemstatus_set_status(status_t status) {

}

status_t systemstatus_get_status() {

}
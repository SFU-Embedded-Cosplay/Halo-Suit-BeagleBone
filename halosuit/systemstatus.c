#include <halosuit/systemstatus.h>
#include <halosuit/led.h>

static status_t currentStatus = NO_STATUS;
static led_t led0 = { 0 };
static led_t led1 = { 1 };
static led_t led2 = { 2 };
static led_t led3 = { 3 };

void systemstatus_init() {

	led_init(led0);
	led_init(led1);
	led_init(led2);
	led_init(led3);

	systemstatus_set_status(BOOT_SUCCESS);
}

void systemstatus_set_status(status_t status) {

}

status_t systemstatus_get_status() {

}
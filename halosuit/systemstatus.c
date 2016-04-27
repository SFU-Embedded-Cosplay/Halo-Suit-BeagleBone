#include <halosuit/systemstatus.h>
#include <halosuit/led.h>

static status_t current_status = NO_STATUS;
static led_t led0 = { 0 };
static led_t led1 = { 1 };
static led_t led2 = { 2 };
static led_t led3 = { 3 };

static void update_led_display() 
{
	if(current_status % 2 == 1) {
		led_on(led3);
	} else {
		led_off(led3);
	}

	if((current_status / 2) % 2 == 1) {
		led_on(led2);
	} else {
		led_off(led2);
	}

	if((current_status / 4) % 2 == 1) {
		led_on(led1);
	} else {
		led_off(led1);
	}

	if((current_status / 8) % 2 == 1) {
		led_on(led0);
	} else {
		led_off(led0);
	}
}

void systemstatus_init() 
{

	led_init(led0);
	led_init(led1);
	led_init(led2);
	led_init(led3);

	systemstatus_set_status(BOOT_SUCCESS);
}

void systemstatus_set_status(status_t status) 
{
	current_status = status;

	update_led_display();
}

status_t systemstatus_get_status() 
{
	return current_status;
}

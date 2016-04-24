#include <halosuit/led.h>
#include <stdio.h>

// to make a path do: path = LED_FILE_PATH + LED_USR_VALUE + ..._FILE_PATH_EXTENSION
#define LED_FILE_PATH "/sys/class/leds/beaglebone:green:usr"
#define TRIGGER_FILE_PATH_EXTENSION "/trigger"
#define BRIGHTNESS_FILE_PATH_EXTENSION "/brightness"

#define TRIGGER_MODE "none"

#define ON_VALUE "1"
#define OFF_VALUE "0"

void led_init(led_t led) 
{
	char file_path[80];
	snprintf(file_path, sizeof(file_path), "%s%d%s", LED_FILE_PATH, led.usr, TRIGGER_FILE_PATH_EXTENSION);

	FILE* led_trigger_file = fopen(file_path, "w");
	fprintf(led_trigger_file, "%s", TRIGGER_MODE);
	fclose(led_trigger_file);

}

void led_on(led_t led) 
{
	char brightness_file_path[80];
	snprintf(brightness_file_path, sizeof(brightness_file_path), "%s%d%s", LED_FILE_PATH, led.usr, BRIGHTNESS_FILE_PATH_EXTENSION);

	FILE* brightness_file = fopen(brightness_file_path, "w");
	fprintf(brightness_file, "%s", ON_VALUE);
	fclose(brightness_file);
}

void led_off(led_t led) 
{
	char brightness_file_path[80];
	snprintf(brightness_file_path, sizeof(brightness_file_path), "%s%d%s", LED_FILE_PATH, led.usr, BRIGHTNESS_FILE_PATH_EXTENSION);

	FILE* brightness_file = fopen(brightness_file_path, "w");
	fprintf(brightness_file, "%s", OFF_VALUE);
	fclose(brightness_file);
}
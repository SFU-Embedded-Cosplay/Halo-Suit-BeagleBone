#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include <beagleblue.h>
#include <halosuit.h>

void callback(char *buf)
{
	int value;
	halosuit_relay_value((int) *buf, &value);
	if (value == 0) {
		halosuit_relay_switch((int) *buf, HIGH);
	} else if (value == 1) {
		halosuit_relay_switch((int) *buf, LOW);
	}
}

int main()
{
	char buf[1024] = { 0 };
	beagleblue_init(&callback);
	halosuit_init();
	double temp0, temp1, temp2, temp3;
	while (1) {
		halosuit_temperature_value(HEAD, &temp0);
		halosuit_temperature_value(ARMPITS, &temp1);
		halosuit_temperature_value(CROTCH, &temp2);
		halosuit_temperature_value(WATER, &temp3);
		sprintf(buf, "{\"temperature\":{\"temp0\":%.2f,\"temp1\":%.2f,\"temp2\":%.2f,\"temp3\":%.2f}}", temp0, temp1, temp2, temp3);
		beagleblue_android_send(buf);
		memset(buf, 0, sizeof(buf));
		sleep(1);
	}
	return 0;
}

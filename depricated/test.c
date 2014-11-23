#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "beagleblue.h"

void callback(char *buf)
{
	printf("%s", buf);
	fflush(stdout);
	if (strncmp(buf, "exit", 4) == 0) {
		beagleblue_exit();
	}
}

int main()
{
	char buf[1024] = { 0 };
	beagleblue_init(&callback);
	double i = 0.0;
	while (1) {
		sprintf(buf, "{\"temperature\":{\"temp0\":%f,\"temp1\":%f}}", i, i);
		beagleblue_android_send(buf);
		memset(buf, 0, sizeof(buf));
		i++;
		usleep(100);
	}
	return 0;
}

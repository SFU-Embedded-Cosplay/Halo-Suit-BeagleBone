/*
	watchdog.c
*/

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <linux/watchdog.h>
#include <sys/ioctl.h>
#include <fcntl.h>

#include "watchdog.h"

void* Watchdog_thread()
{
	int fd = open("/dev/watchdog", O_RDWR);
	if (fd < -1) {
		printf("cannot open watchdog\n");
		exit(EXIT_FAILURE);
	}
	
	while (1) {
	ioctl(fd,WDIOC_KEEPALIVE, NULL);	
	sleep(SLEEP_TIME);		
	}	
	close(fd);
	return NULL;
}

/* 
    main.c
    main file for the spartan armour
*/

#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <linux/watchdog.h>
#include <sys/ioctl.h>
#include <fcntl.h>

#include <beagleblue.h>
#include <parser.h>
#include <serializer.h>

#define WATCHDOG_SLEEP_TIME 1 // in seconds
#define WATCHDOG_PATH "/dev/watchdog"
#define SERIALIZER_THREAD 1

int main(int argc, char* argv[])
{
    beagleblue_init(&Parser_parse);    

    pthread_create(SERIALIZER_THREAD, NULL, Serialize_serialize, NULL);

    int fd = open(WATCHDOG_PATH, O_RDWR);
    if (fd < -1) {
	printf("Cannot open watchdog\n");
	exit(EXIT_FAILURE);
    }

    while (1) {
	ioctl(fd, WDIOC_KEEPALIVE, NULL);
	sleep(WATCHDOG_SLEEP_TIME);
    } 
    close(fd); 
    
    pthread_join(SERIALIZER_THREAD, NULL);

    beagleblue_exit();
    beagleblue_join();

    return 0;
}

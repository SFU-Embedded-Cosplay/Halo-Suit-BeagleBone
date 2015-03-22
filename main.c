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

#include <beagleblue/beagleblue.h>
#include <json/parser.h>
#include <json/serializer.h>
#include <halosuit/halosuit.h>
#include <halosuit/logger.h>

#define WATCHDOG_PATH "/dev/watchdog"

#define SERIALIZE_DELAY 1 // second

int main(int argc, char* argv[])
{

    int fd = open(WATCHDOG_PATH, O_RDWR);
    if (fd < -1) {
	printf("Cannot open watchdog\n");
	exit(EXIT_FAILURE);
    }
    
    logger_startup();

    char buf[1024];
    beagleblue_init(&parser_parse);    
    halosuit_init();  


    // if loop takes longer than 45 secs watchdog will reboot the system
    while (1) {	
	   // sends status information to android phone and google glass
	   serializer_serialize(buf);
        beagleblue_android_send(buf);
        beagleblue_glass_send(buf);
	   // kick watchdog
	   ioctl(fd, WDIOC_KEEPALIVE, NULL);
	
	   sleep(SERIALIZE_DELAY);
    } 

    // close(fd); 
    
    halosuit_exit();    
    
    beagleblue_exit();
    beagleblue_join();

    return 0;
}

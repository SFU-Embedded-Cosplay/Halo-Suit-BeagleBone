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
#include <string.h>
#include <stdbool.h>
#include <signal.h>


#include <beagleblue/beagleblue.h>
#include <json/parser.h>
#include <json/serializer.h>
#include <halosuit/halosuit.h>
#include <halosuit/automation.h>
#include <config/config.h>
#include <halosuit/logger.h>
#include <halosuit/stateofcharge.h>
#include <halosuit/systemstatus.h>


#define WATCHDOG_PATH "/dev/watchdog"

#define SERIALIZE_DELAY_IN_SECONDS 1

bool watchdog_disabled = false;

void crash_signal_handler(int signum) 
{
    logger_log("exiting with code: %d", signum);

    if(signum == SIGHUP) {
        logger_log("crash caused by termination of controlling process, "
            "possibly because terminal was exited");
    } else if(signum == SIGINT) {
        logger_log("crash was from a program interupt: likely from ctrl-c");
    } else if(signum == SIGABRT) {
        logger_log("crash was from an abort signal: likely "
            "because of a library calling abort()");
    } else if(signum == SIGFPE) {
        logger_log("crash caused by Floating-Point arithmetic Exception: "
            "possibly caused by divide by 0 or arithmetic overflow");
    } else if(signum == SIGILL) {
        logger_log("crash caused by illegal instruction: " 
            "possibly caused by corrupted executable or "
            "passing data to a function that expects a pointer");
    } else if(signum == SIGSEGV) {
        logger_log("crash was likely a segfault");
    } else if(signum == SIGTERM) {
        logger_log("crash caused by kill command");
    } else {
        // this should not happen
        logger_log("unknown crash signal was handled!");
    }

    systemstatus_set_status(SYSTEM_CRASH);

    exit(signum);
}

int main(int argc, char* argv[])
{
    if (argc == 2) {
        if (strcmp("--disable-watchdog", argv[1]) == 0) {
            watchdog_disabled = true;
        }
    }
    int file_descriptor;
    if (!watchdog_disabled) {
        file_descriptor = open(WATCHDOG_PATH, O_RDWR);
        if (file_descriptor < -1) {
            printf("Cannot open watchdog\n");
            exit(EXIT_FAILURE);
        }
    }
    logger_startup();
    systemstatus_init();

    // use the bellow link for a good summary of what each signal does
    // http://www.yolinux.com/TUTORIALS/C++Signals.html
    signal(SIGINT, crash_signal_handler);
    signal(SIGABRT, crash_signal_handler);
    signal(SIGFPE, crash_signal_handler);
    signal(SIGILL, crash_signal_handler);
    signal(SIGSEGV, crash_signal_handler);
    signal(SIGTERM, crash_signal_handler);
    signal(SIGHUP, crash_signal_handler);

    char buf[1024];
    config_init("/root/beaglebone.conf");
    beagleblue_init(&parser_parse);    
    halosuit_init();  
    automation_init();
    soc_init();

    logger_log("Initialization complete");
    // if loop takes longer than 45 secs watchdog will reboot the system
    while (1) {	
	    // sends status information to android phone and google glass
        serializer_serialize(buf);
        beagleblue_android_send(buf);
        beagleblue_glass_send(buf);

        if (!watchdog_disabled) {
	        // kick watchdog
	        ioctl(file_descriptor, WDIOC_KEEPALIVE, NULL);
	    }
	    sleep(SERIALIZE_DELAY_IN_SECONDS);
    } 

    // close(file_descriptor); 

    automation_exit(); 
    halosuit_exit();    
    config_exit();
    beagleblue_exit();
    beagleblue_join();

    logger_log("Exiting halosuit");

    return 0;
}

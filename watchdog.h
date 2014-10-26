/*
	watchdog.h
	This thread will periodically kick the watchdog otherwise the system will reboot
*/

#ifndef WATCHDOG
#define WATCHDOG

#define SLEEP_TIME 30 // seconds

// continually runs kicks the watchdog, otherwise causes the system to reboot 
void* Watchdog_thread();

#endif 

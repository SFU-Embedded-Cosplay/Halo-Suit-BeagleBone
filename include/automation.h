/*
    automation.h
    controls the cooling of the system based on the temperature values 
*/

#ifndef AUTOMATION
#define AUTOMATION

#define START_DELAY 10000    // 10 seconds
#define READ_DELAY 1000      // 1 seconds

#define TEMP_VARIANCE 5.00   // degree of difference between two readings for it to be ignored

#define MAX_TEMP 40.00       // degrees celsius
#define HIGH_TEMP 37.60      // degrees celsius 
#define LOW_TEMP 29.00       // degrees celsius
#define MINIMUM_TEMP 26.00   // degrees celsius
#define WATER_MAX_TEMP 22.00 // degrees celsius
#define WATER_MIN_TEMP 1.00  // degrees celsius

// creates a thread that manages the suit's systems
void automation_init();

// closes the automation thread and joins with main thread
void automation_exit();

#endif

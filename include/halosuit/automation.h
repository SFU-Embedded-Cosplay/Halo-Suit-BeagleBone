/*
    automation.h
    controls the cooling of the system based on the temperature values 
*/

#ifndef AUTOMATION
#define AUTOMATION

#define START_DELAY 10000               // 10 seconds
#define READ_DELAY 1000                 // 1 seconds
#define PELTIER_TIMEOUT 900             // 900 seconds = 15 minutes time for peltier to run

#define TEMP_VARIANCE 5.00              // degree of difference between two readings for it to be ignored

#define MAX_TEMP 40.00                  // degrees celsius
#define HIGH_TEMP 37.60                 // degrees celsius 
#define LOW_TEMP 29.00                  // degrees celsius
#define MINIMUM_TEMP 26.00              // degrees celsius
#define WATER_MAX_TEMP 22.00            // degrees celsius
#define WATER_MIN_TEMP 1.00             // degrees celsius

// creates a thread that manages the suit's systems
void automation_init();

// closes the automation thread and joins with main thread
void automation_exit();

/* The two functions below get character values that correspond to 
   various warnings if the temperature is out of nominal ranges.
   the return values are as follows:
*/
#define CRITICAL_HIGH_TEMP_WARNING 'H'
#define HIGH_TEMP_WARNING 'h'
#define NOMINAL_TEMP 'N' 
#define LOW_TEMP_WARNING 'l' 
#define CRITICAL_LOW_TEMP_WARNING 'L' 

char automation_getHeadTempWarning();

char automation_getBodyTempWarning();

#endif

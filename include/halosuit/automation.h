/*
    automation.h
    controls the cooling of the system based on the temperature values 
    controls the cycling of the peltier and water pump
*/
/*
    DISCLAIMER:
    Please note that many of the functions in this file can directly affect the health of someone
    so don't run in production without adequeate testing, I don't want someone getting hurt because 
    of this on my conscience.
*/

#ifndef AUTOMATION
#define AUTOMATION

#define START_DELAY 10000               // 10 seconds
#define READ_DELAY 1000                 // 1 seconds
#define PELTIER_TIMEOUT 1200            // 1200 seconds = 20 minutes time for peltier to run
#define PUMP_TIMEOUT 1200
#define PUMP_STARTUP_TIME 5             // 5 seconds for flow to adjust to normal levels

#define TEMP_VARIANCE 5.00              // degree of difference between two readings for it to be ignored

#define BODY_MAX_TEMP 40.00             // degrees celsius
#define BODY_HIGH_TEMP 37.60            // degrees celsius 
#define BODY_LOW_TEMP 29.00             // degrees celsius
#define BODY_MINIMUM_TEMP 26.00         // degrees celsius
#define BODY_SENSOR_DEFAULT -5          // body sensors default to -50 degrees
#define WATER_MAX_TEMP 22.00            // degrees celsius
#define WATER_MIN_TEMP 1.00             // degrees celsius
#define WATER_SENSOR_DEFAULT 85         // water temperature sensor default temperature 
                                        // is 85 degrees celsius

#define NOMINAL_FLOW_VALUE 10           // normal flow value when pump is on

#define SMOOTH_WEIGHT .8                // value give to past data over new data 

// creates a thread that manages the suit's systems
void automation_init();

// closes the automation thread and joins with main thread
void automation_exit();

/* The functions below get character values that correspond to 
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

// returns only 'h', 'N', or 'l'
char automation_getWaterTempWarning();

/* This function returns a warning for the coolant flow rate, if coolant flows too slow while the 
    pump is on this will retunr a low flow warning otherwise it will return a nominal flow. Since 
    there is a delay with the reading of the flowrate this warning may be a bit slow
*/
#define NOMINAL_FLOW 'N'
#define LOW_FLOW 'L'

char automation_getWaterFlowWarning();


#endif

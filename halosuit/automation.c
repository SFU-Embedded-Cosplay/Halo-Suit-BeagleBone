/*
    automation.c
*/
/*
    DISCLAIMER:
    Please note that many of the functions in this file can directly affect the health of someone
    so don't run in production without adequeate testing, I don't want someone getting hurt because 
    this on my conscience.
*/

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>
#include <pthread.h>
#include <time.h>

#include <halosuit/automation.h>
//#include <halosuit/halosuit.h>
#include <testcode/automationtestdata.h>

static pthread_t automation_id;

static bool automationIsDone = false;

static bool peltierLocked = false;

static char bodyTempWarning = NOMINAL_TEMP;
static char headTempWarning = NOMINAL_TEMP;
static char waterTempWarning = NOMINAL_TEMP;
static char waterFlowWarning = NOMINAL_FLOW;

// temperatures set to mid range
static double adjustedWaterTemp = (WATER_MAX_TEMP + WATER_MIN_TEMP) / 2; 

static double adjustedHeadTemp = (BODY_HIGH_TEMP + BODY_LOW_TEMP) / 2;
static double adjustedArmpitTemp = (BODY_HIGH_TEMP + BODY_LOW_TEMP) / 2;
static double adjustedCrotchTemp = (BODY_HIGH_TEMP + BODY_LOW_TEMP) / 2;

static int adjustedFlowRate = 0;

static time_t peltier_timein = 0;
static time_t pump_timein = 0;

// controlls the peltier cycle
static void peltier_automation()
{
    time_t current_time = time(NULL);

    if ((current_time - peltier_timein) >= PELTIER_TIMEOUT && !peltierLocked) {
        int peltierState;
        // peltierState will be a 1 if it's on and a 0 if off
        if (halosuit_relay_value(PELTIER, &peltierState)) { 
            printf("ERROR: PELTIER READ FAILURE");
            return;
        }
        else {
            if (peltierState) {
                if (halosuit_relay_switch(PELTIER, LOW)) {
                    printf("ERROR: PELTIER READ FAILURE");
                    return;
                }
            }
            else {
                if (halosuit_relay_switch(PELTIER, HIGH)) {
                    printf("ERROR: PELTIER READ FAILURE");
                    return;
                }
            }
            peltier_timein = time(NULL);
        } 
    }
}

// controlls the pump cycle
static void pump_automation()
{
    time_t current_time = time(NULL);

    if ((current_time - pump_timein) >= PUMP_TIMEOUT) {
        int pumpState;
        if (halosuit_relay_value(WATER_PUMP, &pumpState)) {
            printf("ERROR: WATER_PUMP READ FAILURE");
            return;
        }
        else {
            if (pumpState) {
                if (halosuit_relay_switch(WATER_PUMP, LOW)) {
                    printf("ERROR: WATER_PUMP READ FAILURE");
                    return;
                }
            }
            else {
                if (halosuit_relay_switch(WATER_PUMP, HIGH)) {
                    printf("ERROR: WATER_PUMP READ FAILURE");
                }
            }
        }      
        pump_timein = time(NULL);
    }
}

// uses values from the water temperature sensor to control the peltier and pump
static void waterTempLogic() 
{
    double newWaterTemp = 0;
    if (halosuit_temperature_value(WATER, &newWaterTemp)) {
        printf("ERROR: CANNOT READ WATER TEMPERATURE VALUE");
    }

    if (newWaterTemp == WATER_SENSOR_DEFAULT) {
        // smooth water temperature with previous values to mitigate outlier data
        adjustedWaterTemp = (adjustedWaterTemp * SMOOTH_WEIGHT) + (newWaterTemp * (1 - SMOOTH_WEIGHT));
    }

    if (adjustedWaterTemp <= WATER_MIN_TEMP) {
        
        // turn off pump turn off peltier
        waterTempWarning = LOW_TEMP_WARNING;
        if (halosuit_relay_switch(PELTIER, LOW)) {
            printf("ERROR: PELTIER READ FAILURE");
        }
        else {
            peltier_timein = time(NULL);
        }
        
        if (halosuit_relay_switch(WATER_PUMP, LOW)) {
            printf("ERROR: WATER_PUMP READ FAILURE");
        }
        else {
            pump_timein = time(NULL);
        }
    }

    else if (adjustedWaterTemp >= WATER_MAX_TEMP) {
        // turn off pump turn on peltier
        waterTempWarning = HIGH_TEMP_WARNING;
        if (halosuit_relay_switch(PELTIER, HIGH)) {
            printf("ERROR: PELTIER READ FAILURE");
        }
        else {
            peltier_timein = time(NULL);
        }

        if (halosuit_relay_switch(WATER_PUMP, LOW)) {
            printf("ERROR: WATER_PUMP READ FAILURE");
        } else {
            pump_timein = time(NULL);
        }
    }
    else {
        waterTempWarning = NOMINAL_TEMP;
    }
}

// checks if pump is working correctly 
// delays flow check to let water speed to build up
static void checkFlow() {
    time_t current_time = time(NULL);
    int newFlowRate;
    if (halosuit_flowrate(&newFlowRate)) {
        printf("ERROR: WATER FLOW READ FAILURE");
        return;
    } else {
        adjustedFlowRate = (adjustedFlowRate * SMOOTH_WEIGHT) + (newFlowRate * (1 - SMOOTH_WEIGHT));
    }

    int pumpState;
    if (halosuit_relay_value(WATER_PUMP, &pumpState)) {
        printf("ERROR: WATER_PUMP READ FAILURE");
        return;
    }
    else {
        if (pumpState && (current_time - pump_timein) >= PUMP_STARTUP_TIME 
            && adjustedFlowRate < NOMINAL_FLOW_VALUE) {
            waterFlowWarning = LOW_FLOW;
            printf("WARNING: FLOWRATE LOW POSSIBLE LEAK");
        }
        else {
            waterFlowWarning = NOMINAL_FLOW;
        }
    }
}

static void checkHeadTemperature(double temp)
{ 
    if (temp >= BODY_HIGH_TEMP) {
        if (halosuit_relay_switch(HEAD_FANS, HIGH)) {
            printf("ERROR: HEAD_FANS READ FAILURE");
        }

        if (temp >= BODY_MAX_TEMP) {
            headTempWarning = CRITICAL_HIGH_TEMP_WARNING;
        }
        else {
            headTempWarning = HIGH_TEMP_WARNING;
        }
    }
    else if (temp <= BODY_LOW_TEMP) {
        if (halosuit_relay_switch(HEAD_FANS, LOW)) {
            printf("ERROR: HEAD_FANS READ FAILURE");
        }

        if (temp <= BODY_MINIMUM_TEMP) {
            headTempWarning = CRITICAL_LOW_TEMP_WARNING;
        }
        else {
            headTempWarning = LOW_TEMP_WARNING;
        }
    }
    else {
        headTempWarning = NOMINAL_TEMP;
    }
}

static void checkBodyTemperature(double temp)
{
    if (temp >= BODY_HIGH_TEMP) {
        if (halosuit_relay_switch(WATER_PUMP, LOW)) {
            printf("ERROR: WATER_PUMP READ FAILURE");
        }
        else {
            pump_timein = time(NULL);
        }

        if (temp >= BODY_MAX_TEMP) {
            bodyTempWarning = CRITICAL_HIGH_TEMP_WARNING;
        }
        else {
            bodyTempWarning = HIGH_TEMP_WARNING;
        }
    }
    else if (temp <= BODY_LOW_TEMP) {
        if (halosuit_relay_switch(WATER_PUMP, LOW)) {
            printf("ERROR: WATER_PUMP READ FAILURE");
        }
        else {
            pump_timein = time(NULL);
        }

        if (temp <= BODY_MINIMUM_TEMP) {
            bodyTempWarning = CRITICAL_LOW_TEMP_WARNING;
        }
        else {
            bodyTempWarning = LOW_TEMP_WARNING;
        }
    }
    else { 
        bodyTempWarning = NOMINAL_TEMP;
    }
}

static void bodyTemperatureLogic() {
    double newHeadTemp;
    double newArmpitTemp;
    double newCrotchTemp;
    double averageBodyTemp; // average of crotch and armpit not head

    if (halosuit_temperature_value(HEAD, &newHeadTemp)) {
        printf("ERROR: HEAD TEMPERATURE READ FAILURE");
    }
    else if (newHeadTemp != BODY_SENSOR_DEFAULT) {
        adjustedHeadTemp = (adjustedHeadTemp * SMOOTH_WEIGHT) + (newHeadTemp * (1 - SMOOTH_WEIGHT));
    }

    if (halosuit_temperature_value(ARMPITS, &newArmpitTemp)) {
        printf("ERROR: ARMPIT TEMPERATURE READ FAILURE");
    }
    else if (newArmpitTemp != BODY_SENSOR_DEFAULT) {
        adjustedArmpitTemp =  (adjustedArmpitTemp * SMOOTH_WEIGHT) + (newArmpitTemp * (1 - SMOOTH_WEIGHT));
    }
    
    if (halosuit_temperature_value(CROTCH, &newCrotchTemp)) {
        printf("ERROR: CROTCH TEMPERATURE READ FAILURE");
    }
    else if (newCrotchTemp != BODY_SENSOR_DEFAULT) {
        adjustedCrotchTemp = (adjustedCrotchTemp * SMOOTH_WEIGHT) + (newCrotchTemp * (1 - SMOOTH_WEIGHT));
    }

    averageBodyTemp = (adjustedArmpitTemp + adjustedCrotchTemp) / 2;

    checkHeadTemperature(adjustedHeadTemp);
    checkBodyTemperature(averageBodyTemp); 
}

static void* automationThread()
{ 
    automationIsDone = false;

    if (halosuit_relay_switch(PELTIER, HIGH)) {
            printf("ERROR: PELTIER READ FAILURE");
    }
    else {
        peltier_timein = time(NULL);
    }
    
    if (halosuit_relay_switch(WATER_PUMP, HIGH)) {
            printf("ERROR: WATER_PUMP READ FAILURE");
    }
    else {
        pump_timein = time(NULL);
    }

    sleep(START_DELAY); // to prevent the suit from reading weird startup values

    while (!automationIsDone) {

        peltier_automation();
        pump_automation();
        waterTempLogic();
        bodyTemperatureLogic(); 
        
        sleep(READ_DELAY);
    }

    return NULL;
}

void automation_init()
{
    pthread_create(&automation_id, NULL, &automationThread, NULL);
}

void automation_exit()
{
    automationIsDone = true;
    pthread_join(automation_id, NULL);
}

char automation_getHeadTempWarning()
{
    return headTempWarning;
}

char automation_getBodyTempWarning()
{
    return bodyTempWarning;
}

char automation_getWaterTempWarning() { return waterTempWarning;
}

char automation_getWaterFlowWarning()
{
    return waterFlowWarning;
}

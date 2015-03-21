/*
    automation.c
*/

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>
#include <pthread.h>
#include <time.h>

#include <halosuit/automation.h>
#include <halosuit/halosuit.h>

static pthread_t automation_id;

static bool automationIsDone;

static bool peltierLocked = false;

static char bodyTempWarning = NOMINAL_TEMP;
static char headTempWarning = NOMINAL_TEMP;

// since 0 is below min temp I set it in between the two water temps
static double adjustedWaterTemp = (WATER_MAX_TEMP + WATER_MIN_TEMP) / 2; 

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

// this checks if the temperature value is an anomaly
static bool isTempSpike(double current, double previous)
{
    if (current > previous) {
        return ((current - previous) > TEMP_VARIANCE);
    }
    else {
        return ((previous - current) > TEMP_VARIANCE);
    }
}

static void checkHeadTemp(double temp, double lastTemp)
{
    if (isTempSpike(temp, lastTemp)) {
        return;
    }
    
    else if (temp >= HIGH_TEMP) {
        if (halosuit_relay_switch(HEAD_FANS, HIGH)) {
            printf("ERROR: HEAD_FANS READ FAILURE");
        }

        if (temp >= MAX_TEMP) {
            headTempWarning = CRITICAL_HIGH_TEMP_WARNING;
        }
        else {
            headTempWarning = HIGH_TEMP_WARNING;
        }
    }
    else if (temp <= LOW_TEMP) {
        if (halosuit_relay_switch(HEAD_FANS, LOW)) {
            printf("ERROR: HEAD_FANS READ FAILURE");
        }

        if (temp <= MINIMUM_TEMP) {
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

static void checkBodyTemp(double temp, double lastTemp)
{
    if (isTempSpike(temp, lastTemp)) {
        return;
    }
    else if (temp >= HIGH_TEMP) {
        if (halosuit_relay_switch(WATER_PUMP, HIGH)) {
            printf("ERROR: WATER_PUMP READ FAILURE");
        }
        if (temp >= MAX_TEMP) {
            bodyTempWarning = CRITICAL_HIGH_TEMP_WARNING;
        }
        else {
            bodyTempWarning = HIGH_TEMP_WARNING;
        }
    }
    else if (temp <= LOW_TEMP) {
        if (halosuit_relay_switch(WATER_PUMP, LOW)) {
            printf("ERROR: WATER_PUMP READ FAILURE");
        }

        if (temp <= MINIMUM_TEMP) {
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

// uses values from the water temperature sensor to control the peltier and pump
static void waterTempLogic() 
{
    double newWaterTemp = 0;
    if (halosuit_temperature_value(WATER, &newWaterTemp)) {
        printf("ERROR: CANNOT READ WATER TEMPERATURE VALUE");
    }

    if (newWaterTemp == WATER_TEMP_DEFAULT) {
        // smooth water temperature with previous values to mitigate outlier data
        adjustedWaterTemp = (adjustedWaterTemp * SMOOTH_WEIGHT) + (newWaterTemp * (1 - SMOOTH_WEIGHT));
    }

    if (adjustedWaterTemp <= WATER_MIN_TEMP) {
        
        // turn off pump turn off peltier
        // TODO: throw warning that water temp is too low
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
        // TODO: throw warning that water temp is too high
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
            && adjustedFlowRate < NOMINAL_FLOW) {
            // TODO: return error possible leak in pipe
            printf("WARNING: FLOWRATE LOW POSSIBLE LEAK");
        }
    }
}

static void* automationThread()
{ 
    automationIsDone = false;
    double headTemp = 0;
    double armpitTemp = 0;
    double crotchTemp = 0;
    double averageBodyTemp = 0;

    double lastHeadTemp = 0;
    double lastAverageTemp = 0;

    headTempWarning = 'N';
    bodyTempWarning = 'N';


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

    sleep(START_DELAY); // to prevent the suit from reading startup values

    halosuit_temperature_value(HEAD, &lastHeadTemp);
    halosuit_temperature_value(ARMPITS, &armpitTemp);
    halosuit_temperature_value(CROTCH, &crotchTemp);

    lastAverageTemp = (armpitTemp + crotchTemp) / 2;

    while (!automationIsDone) {
        halosuit_temperature_value(HEAD, &headTemp);
        halosuit_temperature_value(ARMPITS, &armpitTemp);
        halosuit_temperature_value(CROTCH, &crotchTemp);

        averageBodyTemp = (armpitTemp + crotchTemp) / 2; 
        
        // will change
        checkHeadTemp(headTemp, lastHeadTemp);
        checkBodyTemp(averageBodyTemp, lastAverageTemp); 

        lastHeadTemp = headTemp;
        lastAverageTemp = averageBodyTemp;

        peltier_automation();
        pump_automation();
        waterTempLogic();
        
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

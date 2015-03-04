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
static bool peltierAutoOn = true;
static bool peltierLocked = false;

static char bodyTempWarning;
static char headTempWarning;

static time_t peltier_timein = 0;

static void peltier_automation()
{
    if (peltier_timein == 0) {
        peltier_timein = time(NULL);
    }

    time_t current_time = time(NULL);


    if ((current_time - peltier_timein) >= TEMP_VARIANCE && peltierAutoOn && !peltierLocked) {
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
                if (halosuit_relay_switch(PELTIER, LOW)) {
                    printf("ERROR: PELTIER READ FAILURE");
                    return;
                }
            }
            peltier_timein = time(NULL);
        } 
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
            printf("ERROR: WATER_FANS READ FAILURE");
        }
        if (halosuit_relay_switch(WATER_FAN, HIGH)) {
            printf("ERROR: HEAD_FANS READ FAILURE");
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
            printf("ERROR: WATER_FANS READ FAILURE");
        }
        if (halosuit_relay_switch(WATER_FAN, LOW)) {
            printf("ERROR: HEAD_FANS READ FAILURE");
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
        
        checkHeadTemp(headTemp, lastHeadTemp);
        checkBodyTemp(averageBodyTemp, lastAverageTemp); 

        lastHeadTemp = headTemp;
        lastAverageTemp = averageBodyTemp;

        peltier_automation();
        
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

void automation_setAutoPeltierOn()
{
    if (peltierLocked) {
        printf("CANNOT TURN AUTOMATION ON PELTIER LOCKED");
        return;
    }
    peltierAutoOn = true;
}

void automation_setAutoPeltierOff()
{
    peltierAutoOff = false;
}

char automation_getHeadTempWarning()
{
    return headTempWarning;
}

char automation_getBodyTempWarning()
{
    return bodyTempWarning;
}

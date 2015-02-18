/*
    automation.c
*/

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>
#include <pthread.h>

#include <automation.h>
#include <halosuit.h>

static pthread_t automation_id;

static bool automation_is_done;

// this checks if the temperature value is an anomaly
bool isTempSpike(double current, double previous)
{
    if (current > previous) {
        return ((current - previous) > TEMP_VARIANCE);
    }
    else {
        return ((previous - current) > TEMP_VARIANCE);
    }
}

void checkHeadTemp(double temp, double lastTemp)
{
    if (isTempSpike(temp, lastTemp)) {
        return;
    }
    
    else if (temp >= HIGH_TEMP) {
        if (halosuit_relay_switch(HEAD_FANS, HIGH)) {
            printf("ERROR: HEAD_FANS READ FAILURE");
        }

        if (temp >= MAX_TEMP) {
            // send warning
        }
        else {
            // send critical warning
        }
    }
    else if (temp <= LOW_TEMP) {
        if (halosuit_relay_switch(HEAD_FANS, LOW)) {
            printf("ERROR: HEAD_FANS READ FAILURE");
        }

        if (temp <= MINIMUM_TEMP) {
            // send critical warning
        }
        else {
            // send warning
        }
    }
}

void checkBodyTemp(double temp, double lastTemp)
{
    if (isTempSpike(temp, lastTemp)) {
        return;
    }
    else if (tempemp >= HIGH_TEMP) {
        if (halosuit_relay_switch(WATER_PUMP, HIGH)) {
            printf("ERROR: WATER_FANS READ FAILURE");
        }
        if (halosuit_relay_switch(WATER_FAN, HIGH)) {
            printf("ERROR: HEAD_FANS READ FAILURE");
        }
        if (temp >= MAX_TEMP) {
            // send critical warning
        }
        else {
            // send warning
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
            // send critical warning
        }
        else {
            // send warning
        }
    }
    else {
        // unset all flags
    }
}

void* automationThread()
{ 
    automation_is_done = false;
    double headTemp = 0;
    double armpitTemp = 0;
    double crotchTemp = 0;
    double averageBodyTemp = 0;

    double lastHeadTemp = 0;
    double lastAverageTemp = 0;

    sleep(START_DELAY); // to prevent the suit from reading startup values

    halosuit_temperature_value(HEAD, &lastHeadTemp);
    halosuit_temperature_value(ARMPITS, &armpitTemp);
    halosuit_temperature_value(CROTCH, &crotchTemp);

    lastAverageTemp = (armpitTemp + crotchTemp) / 2;

    while (!automation_is_done) {
        halosuit_temperature_value(HEAD, &headTemp);
        halosuit_temperature_value(ARMPITS, &armpitTemp);
        halosuit_temperature_value(CROTCH, &crotchTemp);

        averageBodyTemp = (armpitTemp + crotchTemp) / 2; 
        
        checkHeadTemp(headTemp, lastHeadTemp);
        checkBodyTemp(averageBodyTemp, lastAverageTemp); 

        lastHeadTemp = headTemp;
        lastAverageTemp = averageTemp;
        
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
    automation_is_done = true;
    pthread_join(&automation_id, NULL);
}

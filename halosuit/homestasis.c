/*
    homeostasis.c
*/

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>


#include <homestasis.h>
#include <halosuit.h>

void checkHeadTemp(int temp)
{
    if (temp >= HIGH_TEMP) {
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

void checkBodyTemp(int temp)
{
    if (tempemp >= HIGH_TEMP) {
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
}

void* homeostasis_thread()
{ 
    double headTemp = 0;
    double armpitsTemp = 0;
    double crotchTemp = 0;

    while (1) {
        halosuit_temperature_value(HEAD, &headTemp);
        halosuit_temperature_value(ARMPITS, &armpitsTemp);
        halosuit_temperature_value(CROTCH, &crotchTemp);

        averageBodyTemp = (armpitsTemp + crotchTemp) / 2; 
        
        checkHeadTemp(headTemp);
        checkBodyTemp(averageBodyTemp); 
    }
}



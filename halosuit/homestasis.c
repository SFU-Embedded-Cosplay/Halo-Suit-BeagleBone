/*
    homeostasis.c
*/

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include <homestasis.h>
#include <halosuit.h>

void* homeostasis_thread()
{ 
    double headTemp = 0;
    double armpitsTemp = 0;
    double crotchTemp = 0;

    while (1) {
	halosuit_temperature_value(HEAD, &headTemp);
	halosuit_temperature_value(ARMPITS, &armpitsTemp);
	halosuit_temperature_value(CROTCH, &crotchTemp);
	halo

	if (headTemp >= MAX_TEMP) {
	    // critical alert
	    if (halosuit_relay_switch(HEAD_FANS, HIGH)) {
		printf("ERROR: HEAD_FANS READ FAILURE");
	    }
	}
	else if (headTemp >= HIGH_TEMP) {
	    // sends warning
	    if (halosuit_relay_switch(HEAD_FANS, HIGH)) {
		printf("ERROR: HEAD_FANS READ FAILURE");
	    }
	}
	else if (headTemp <= LOW_TEMP) {
	    // sends warning 
	    if (halosuit_relay_switch(HEAD_FANS, LOW)) {
		printf("ERROR: HEAD_FANS READ FAILURE");
	    }
	}
	else if (headTemp <= MINIMUM_TEMP) {
	    // critical alert
	    if (halosuit_relay_switch(HEAD_FANS, LOW)) {
		printf("ERROR: HEAD_FANS READ FAILURE");
	    }
	}
	
	if (armpitsTemp >= MAX_TEMP) {
	    // critical alert 
	    if (halosuit_relay_switch(WATER_PUMP, HIGH)) {
		printf("ERROR: WATER_FANS READ FAILURE");
	    }
	    if (halosuit_relay_switch(WATER_FAN, HIGH)) {
		printf("ERROR: HEAD_FANS READ FAILURE");
	    }
	}
	else if (armpitsTemp >= HIGH_TEMP) {
	    // sends warning  
	    if (halosuit_relay_switch(WATER_PUMP, HIGH)) {
		printf("ERROR: WATER_FANS READ FAILURE");
	    }
	    if (halosuit_relay_switch(WATER_FAN, HIGH)) {
		printf("ERROR: HEAD_FANS READ FAILURE");
	    }
	}
	else if (armpitsTemp <= LOW_TEMP) {
	    // sends warning
	    if (halosuit_relay_switch(WATER_PUMP, LOW)) {
		printf("ERROR: WATER_FANS READ FAILURE");
	    }
	    if (halosuit_relay_switch(WATER_FAN, LOW)) {
		printf("ERROR: HEAD_FANS READ FAILURE");
	    }
	}
	else if (armpitsTemp <= MINIMUM_TEMP) {
	    // critical alert 
	    if (halosuit_relay_switch(WATER_PUMP, LOW)) {
		printf("ERROR: WATER_FANS READ FAILURE");
	    }
	    if (halosuit_relay_switch(WATER_FAN, LOW)) {
		printf("ERROR: HEAD_FANS READ FAILURE");
	    }
	}

	if (crotchTemp >= MAX_TEMP) {
	    // critical alert 
	    if (halosuit_relay_switch(WATER_PUMP, HIGH)) {
		printf("ERROR: WATER_FANS READ FAILURE");
	    }
	    if (halosuit_relay_switch(WATER_FAN, HIGH)) {
		printf("ERROR: HEAD_FANS READ FAILURE");
	    }
	}
	else if (crotchTemp >= HIGH_TEMP) {
	    // sends warning
	    if (halosuit_relay_switch(WATER_PUMP, HIGH)) {
		printf("ERROR: WATER_FANS READ FAILURE");
	    }
	    if (halosuit_relay_switch(WATER_FAN, HIGH)) {
		printf("ERROR: HEAD_FANS READ FAILURE");
	    }
	}
	else if (crotchTemp <= LOW_TEMP) {
	    // sends warning 
	    if (halosuit_relay_switch(WATER_PUMP, LOW)) {
		printf("ERROR: WATER_FANS READ FAILURE");
	    }
	    if (halosuit_relay_switch(WATER_FAN, LOW)) {
		printf("ERROR: HEAD_FANS READ FAILURE");
	    }
	}
	else if (crotchTemp <= MINIMUM_TEMP) {
	    // critical alert
	    if (halosuit_relay_switch(WATER_PUMP, LOW)) {
		printf("ERROR: WATER_FANS READ FAILURE");
	    }
	    if (halosuit_relay_switch(WATER_FAN, LOW)) {
		printf("ERROR: HEAD_FANS READ FAILURE");
	    }
	} 
	
    }
}

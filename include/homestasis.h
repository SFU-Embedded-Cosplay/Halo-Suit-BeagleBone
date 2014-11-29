/*
    homeostasis.h
    controls the cooling of the system based on the temperature values 
*/

#ifndef HOMEOSTASIS
#define HOMEOSTASIS

#define MAX_TEMP 40.00       // degrees celsius
#define HIGH_TEMP 37.60      // degrees celsius 
#define LOW_TEMP 29.00       // degrees celsius
#define MINIMUM_TEMP 26.00   // degrees celsius
#define WATER_MAX_TEMP 22.00 // degrees celsius
#define WATER_MIN_TEMP 1.00  // degrees celsius

void* homeostasis_thread();

#endif

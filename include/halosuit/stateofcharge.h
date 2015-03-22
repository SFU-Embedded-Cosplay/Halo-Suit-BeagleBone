/*
    stateofcharge.h
*/

#ifndef STATE_OF_CHARGE
#define STATE_OF_CHARGE

#define INTERNAL_RESISTANCE .022  // Ohms

#define SAMPLE_SIZE 11
// if more data points are added increase the size of SAMPLE_SIZE 
const int OCV[] = {12580, 12270, 11990, 11790, 11610, 11450, 11350, 11270, 11170, 11050, 10000};

void soc_init();

int soc_getcharge();

#endif

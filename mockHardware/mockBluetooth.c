/* **************************************
 * mockBluetooth.c
 * emulates communication that would normally happen over bluetooth
 */
#ifdef MOCK_BLUETOOTH

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <pthread.h>
int beagleblue_glass_send(char *buffer)
{
   return 0;
}

int beagleblue_android_send(char *buffer)
{
    return 0;
}

void beagleblue_init(void (*on_receive)(char *buffer)) 
{
}

void beagleblue_exit() 
{
}

void beagleblue_join()
{
}

#endif

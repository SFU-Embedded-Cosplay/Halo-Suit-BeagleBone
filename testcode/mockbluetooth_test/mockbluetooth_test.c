/*
 * mockbluetooth_test.c
 *
 * This code is used to test the functionality of the mock_bluetooth.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <beagleblue/beagleblue.h>

void print_message(char *buffer);

int main ()
{
    beagleblue_init(print_message);
    beagleblue_join();
    return 0;
}

void print_message(char *buffer)
{
    if (buffer[0] == '0') {
        beagleblue_exit();
    }
    printf("%s", buffer);
}



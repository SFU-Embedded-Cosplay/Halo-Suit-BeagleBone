/*
    stateofcharge.c
*/

#include <stdio.h>
#include <unistd.h>

#include <halosuit/halosuit.h>
#include <halosuit/stateofcharge.h>

#define WEIGHT .80

static int adjusted_OCV;
static int percent_charge

static void* main_thread()
{
    int sample_size = sizeof(OCV) / sizeof(int);
    while (true) {
        int current;
        halosuit_curent_draw_value(current);        
        double terminal_voltage;
        halosuit_voltage_value(VOLTAGE_1, terminal_voltage);
        new_OCV = terminal_voltage - (current * INTERNAL_RESISTANCE);

        adjusted_OCV = (adjusted_OCV * WEIGHT) + (new_OCV * (1 - WEIGHT));

        percent_charge = interpolate(adjusted_OCV); 

        sleep(1);
    }
}

static int interpolate(int voltage) {
    int i;
    if (voltage >= OCV[0]) {
        return 100;
    } else if (voltage <= OCV[SAMPLE_SIZE - 1]) {
        return 0;
    }
    for (i = 0; i < SAMPLE_SIZE - 1; i++) {
        if (OCV[i + 1] < voltage && voltage < OCV[i]) {
            break;
        }
    }
    // 1 / %
    int inverse_percentage = (OCV[i] - OCV[i + 1]) / (voltage - OCV[i + 1]);
    // percentage base = 90 - i * 10
    int percentage_base = 90 - i * 10;
    return percentage_base + 10 / inverse_percentage
}

void soc_init()
{
}

int soc_getcharge()
{
}

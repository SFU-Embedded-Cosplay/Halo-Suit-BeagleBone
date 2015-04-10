/*
    stateofcharge.c
*/

#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

#include <halosuit/halosuit.h>
#include <halosuit/stateofcharge.h>
#include <halosuit/logger.h>


#define WEIGHT .80


static Battery battery1 = {TURNIGY_8_AH, 22, 100, 12000, {12580,12270,11990,11790,11610,11450,11350,11270,11170,11050,10000}};
static Battery battery2 = {TURNIGY_2_AH, 350, 100, 12000, {12500,12090,11820,11620,11480,11310,11200,11140,11080,10800,10000}};

static int phone_soc = 100;
static int glass_soc = 100;

static pthread_t soc_thread_id;

static int calculate_charge(Battery battery);
static int interpolate(Battery battery);

static void* main_thread()
{
    while (1) {
        battery1.percent_charge = calculate_charge(battery1);
        battery2.percent_charge = calculate_charge(battery2);
        sleep(1);
    }

    return NULL;
}

static int calculate_charge(Battery battery) {
    int current = 0;
    int terminal_voltage = 12000; // millivolts
    if (halosuit_current_draw_value(battery.id, &current)) {
        logger_log("ERROR: FAILURE TO READ CURRENT DRAW");
    }
    if (halosuit_voltage_value(battery.id, &terminal_voltage)) {
        logger_log("ERROR: FAILURE TO READ TERMINAL VOLTAGE");
    }
    int new_ocv = terminal_voltage - (current * battery.internal_resistance);
    battery.adjusted_ocv = (battery.adjusted_ocv * WEIGHT) + (new_ocv * (1 - WEIGHT));

    return interpolate(battery);
}

static int interpolate(Battery battery) {
    if (battery.adjusted_ocv >= battery.ocv[0]) {
        return 100;
    } else if (battery.adjusted_ocv <= battery.ocv[SAMPLE_SIZE - 1]) {
        return 0;
    }
    int i = 0;
    for (i = 0; i < SAMPLE_SIZE - 1; i++) {
        if (battery.ocv[i + 1] < battery.adjusted_ocv && battery.adjusted_ocv < battery.ocv[i]) {
            break;
        }
    }
    // 1 / %
    int inverse_percentage = (battery.ocv[i] - battery.ocv[i + 1]) / (battery.adjusted_ocv - battery.ocv[i + 1]);
    int percentage_base = 90 - i * 10;

    return percentage_base + 10 / inverse_percentage;
}

void soc_init()
{
    pthread_create(&soc_thread_id, NULL, &main_thread, NULL);
}

int soc_getcharge(int batteryID)
{
    if (batteryID == TURNIGY_8_AH) {
        return battery1.percent_charge;
    }
    else if (batteryID == TURNIGY_2_AH) {
        return battery2.percent_charge;
    }
    else if (batteryID == GLASS_BATTERY) {
        return glass_soc;
    }
    else if (batteryID == PHONE_BATTERY) {
        return phone_soc;
    }
}

void soc_setcharge(int batteryID, int percent_charge)
{
    if (batteryID == GLASS_BATTERY) {
        glass_soc = percent_charge;
    }
    else if (batteryID == PHONE_BATTERY) {
        phone_soc = percent_charge;
    } 
}

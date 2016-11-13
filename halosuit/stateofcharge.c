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

static pthread_mutex_t battery1_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t battery2_mutex = PTHREAD_MUTEX_INITIALIZER;


static void* main_thread()
{

	int battery1_new_charge;
	int battery2_new_charge;

	while (1) {
		pthread_mutex_lock(&battery1_mutex);
		{
			battery1_new_charge = calculate_charge(battery1);
		}
		pthread_mutex_unlock(&battery1_mutex);

		pthread_mutex_lock(&battery2_mutex);
		{
			battery2_new_charge = calculate_charge(battery2);
		}
		pthread_mutex_unlock(&battery2_mutex);

		battery1.percent_charge = battery1_new_charge;
		battery2.percent_charge = battery2_new_charge;

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
		int battery1_percent_charge;

		pthread_mutex_lock(&battery1_mutex);
		{
			battery1_percent_charge = battery1.percent_charge;
		}
		pthread_mutex_unlock(&battery1_mutex);

		return battery1_percent_charge;
	}
	else if (batteryID == TURNIGY_2_AH) {
		int battery2_percent_charge;

		pthread_mutex_lock(&battery2_mutex);
		{
			battery2_percent_charge = battery2.percent_charge;
		}
		pthread_mutex_unlock(&battery2_mutex);

		return battery2_percent_charge;	}
	else if (batteryID == GLASS_BATTERY) {
		return glass_soc;
	}
	else if (batteryID == PHONE_BATTERY) {
		return phone_soc;
	}
	else {
		logger_log("ERROR: UNDEFINED BATTERY ID");
		return -1;
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

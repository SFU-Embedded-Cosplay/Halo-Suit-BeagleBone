/*
    automation.c
*/
/*
    DISCLAIMER:
    Please note that many of the functions in this file can directly affect the health of someone
    so don't run in production without adequeate testing, I don't want someone getting hurt because 
    this on my conscience.
*/

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>
#include <pthread.h>
#include <time.h>

#include <halosuit/automation.h>
#include <halosuit/halosuit.h>
//#include <testcode/automationtestdata.h>
#include <halosuit/logger.h>
#include <halosuit/stateofcharge.h>

static pthread_t automation_id;

static bool automationIsDone = false;
static bool peltierLocked = false;
static bool pumpLocked = false;
static bool peltierAuto = true;
static bool pumpAuto = true;

static char bodyTempWarning = NOMINAL_TEMP;
static char headTempWarning = NOMINAL_TEMP;
static char waterTempWarning = NOMINAL_TEMP;
static char waterFlowWarning = NOMINAL_FLOW;

static char turnigy_8AH_low_warning = NOMINAL_SOC;
static char turnigy_2AH_low_warning = NOMINAL_SOC;
static char glass_battery_low_warning = NOMINAL_SOC;
static char phone_battery_low_warning = NOMINAL_SOC;
// temperatures set to mid range
static double adjustedWaterTemp = (WATER_MAX_TEMP + WATER_MIN_TEMP) / 2; 

static double adjustedHeadTemp = (BODY_HIGH_TEMP + BODY_LOW_TEMP) / 2;
static double adjustedArmpitTemp = (BODY_HIGH_TEMP + BODY_LOW_TEMP) / 2;
static double adjustedCrotchTemp = (BODY_HIGH_TEMP + BODY_LOW_TEMP) / 2;

static int adjustedFlowRate = 0;

static time_t peltier_timein = 0;
static time_t pump_timein = 0;

// controlls the peltier cycle
static void peltier_automation()
{
    time_t current_time = time(NULL);

    if (difftime(current_time, peltier_timein) >= PELTIER_TIMEOUT && !peltierLocked && peltierAuto) {
        int peltierState;
        // peltierState will be a 1 if it's on and a 0 if off
        if (halosuit_relay_value(PELTIER, &peltierState)) { 
            logger_log("ERROR: PELTIER READ FAILURE");
            return;
        }
        else {
            if (peltierState) {
                if (halosuit_relay_switch(PELTIER, LOW)) {
                    logger_log("ERROR: PELTIER READ FAILURE");
                    return;
                }
            }
            else {
                if (halosuit_relay_switch(PELTIER, HIGH)) {
                    logger_log("ERROR: PELTIER READ FAILURE");
                    return;
                }
            }
            peltier_timein = time(NULL);
        } 
    }
}

// controlls the pump cycle
static void pump_automation()
{
    time_t current_time = time(NULL);

    if (difftime(current_time, pump_timein) >= PUMP_TIMEOUT && pumpAuto) {
        int pumpState;
        if (halosuit_relay_value(WATER_PUMP, &pumpState)) {
            logger_log("ERROR: WATER_PUMP READ FAILURE");
            return;
        }
        else {
            if (pumpState) {
                if (halosuit_relay_switch(WATER_PUMP, LOW)) {
                    logger_log("ERROR: WATER_PUMP READ FAILURE");
                    return;
                }
            }
            else {
                if (halosuit_relay_switch(WATER_PUMP, HIGH)) {
                    logger_log("ERROR: WATER_PUMP READ FAILURE");
                }
            }
        }      
        pump_timein = time(NULL);
    }
}

// uses values from the water temperature sensor to control the peltier and pump
static void waterTempLogic() 
{
    double newWaterTemp = 0;
    if (halosuit_temperature_value(WATER, &newWaterTemp)) {
        logger_log("ERROR: CANNOT READ WATER TEMPERATURE VALUE");
    }

    // occasionly the sensor might give a weird data point which is it's default this ignores it
    // if the water is at 85 degrees the user will probably know without the suit telling them
    if (newWaterTemp != WATER_SENSOR_DEFAULT) {
        // smooth water temperature with previous values to mitigate outlier data
        adjustedWaterTemp = (adjustedWaterTemp * SMOOTH_WEIGHT) + (newWaterTemp * (1 - SMOOTH_WEIGHT));
    }

    if (adjustedWaterTemp <= WATER_MIN_TEMP) {
        
        // turn off pump turn off peltier
        pumpLocked = true;
        waterTempWarning = LOW_TEMP_WARNING;
        if (halosuit_relay_switch(PELTIER, LOW)) {
            logger_log("ERROR: PELTIER READ FAILURE");
        }
        else {
            peltier_timein = time(NULL);
        }
        
        if (halosuit_relay_switch(WATER_PUMP, LOW)) {
            logger_log("ERROR: WATER_PUMP READ FAILURE");
        }
        else {
            pump_timein = time(NULL);
        }
    }

    else if (adjustedWaterTemp >= WATER_MAX_TEMP) {
        // turn off pump turn on peltier
        pumpLocked = true;
        waterTempWarning = HIGH_TEMP_WARNING;
        if (peltierAuto) {
            if (halosuit_relay_switch(PELTIER, HIGH)) {
                logger_log("ERROR: PELTIER READ FAILURE");
            }
            else {
                peltier_timein = time(NULL);
            }
        }

        if (halosuit_relay_switch(WATER_PUMP, LOW)) {
            logger_log("ERROR: WATER_PUMP READ FAILURE");
        } 
        else {
            pump_timein = time(NULL);
        }
    }
    else {
        waterTempWarning = NOMINAL_TEMP;
        pumpLocked = false;
    }
}

// checks if pump is working correctly 
// delays flow check to let water speed to build up
static void checkFlow() {
    time_t current_time = time(NULL);
    int newFlowRate;
    if (halosuit_flowrate(&newFlowRate)) {
        logger_log("ERROR: WATER FLOW READ FAILURE");
        return;
    } 
    else {
        adjustedFlowRate = (adjustedFlowRate * SMOOTH_WEIGHT) + (newFlowRate * (1 - SMOOTH_WEIGHT));
    }

    int pumpState;
    if (halosuit_relay_value(WATER_PUMP, &pumpState)) {
        logger_log("ERROR: WATER_PUMP READ FAILURE");
        return;
    }
    else {
        if (pumpState && (current_time - pump_timein) >= PUMP_STARTUP_TIME 
            && adjustedFlowRate < NOMINAL_FLOW_VALUE) {
            //TODO: need to shut off pumpflow and also lock it off so that it doesn't flow
            //      more information is needed

            waterFlowWarning = LOW_FLOW;
            logger_log("WARNING: FLOWRATE LOW POSSIBLE LEAK");
        }
        else {
            waterFlowWarning = NOMINAL_FLOW;
        }
    }
}

static void checkHeadTemperature(double temp)
{ 
    if (temp >= BODY_HIGH_TEMP) {
        if (halosuit_relay_switch(HEAD_FANS, HIGH)) {
            logger_log("ERROR: HEAD_FANS READ FAILURE");
        }

        if (temp >= BODY_MAX_TEMP) {
            headTempWarning = CRITICAL_HIGH_TEMP_WARNING;
        }
        else {
            headTempWarning = HIGH_TEMP_WARNING;
        }
    }
    else if (temp <= BODY_LOW_TEMP) {
        if (halosuit_relay_switch(HEAD_FANS, LOW)) {
            logger_log("ERROR: HEAD_FANS READ FAILURE");
        }

        if (temp <= BODY_MINIMUM_TEMP) {
            headTempWarning = CRITICAL_LOW_TEMP_WARNING;
        }
        else {
            headTempWarning = LOW_TEMP_WARNING;
        }
    }
    else {
        headTempWarning = NOMINAL_TEMP;
    }
}

static void checkBodyTemperature(double temp)
{
    if (temp >= BODY_HIGH_TEMP) {
        // if water is too cold or too warm the pump will lock
        // if it's too cold it's probably already cooling the user 
        // and will warm up the water to start pumping again
        // if water is too warm well pumping it will not help
        // may need to add a warning here to notify the user of lack of coolant
        if (!pumpLocked && pumpAuto) {
            if (halosuit_relay_switch(WATER_PUMP, HIGH)) {
                logger_log("ERROR: WATER_PUMP READ FAILURE");
            }
            else {
                pump_timein = time(NULL);
            }
        }
        else {
            //TODO: add warning here for being unable to turn on pump becuase of auto being off or 
            //      the coolant temperature being too low
        }

        if (temp >= BODY_MAX_TEMP) {
            bodyTempWarning = CRITICAL_HIGH_TEMP_WARNING;
        }
        else {
            bodyTempWarning = HIGH_TEMP_WARNING;
        }
    }
    else if (temp <= BODY_LOW_TEMP) {
        if (halosuit_relay_switch(WATER_PUMP, LOW)) {
            logger_log("ERROR: WATER_PUMP READ FAILURE");
        }
        else {
            pump_timein = time(NULL);
        }

        if (temp <= BODY_MINIMUM_TEMP) {
            bodyTempWarning = CRITICAL_LOW_TEMP_WARNING;
        }
        else {
            bodyTempWarning = LOW_TEMP_WARNING;
        }
    }
    else { 
        bodyTempWarning = NOMINAL_TEMP;
    }
}

static void bodyTemperatureLogic() {
    double newHeadTemp;
    double newArmpitTemp;
    double newCrotchTemp;
    double averageBodyTemp; // average of crotch and armpit not head

    if (halosuit_temperature_value(HEAD, &newHeadTemp)) {
        logger_log("ERROR: HEAD TEMPERATURE READ FAILURE");
    }
    else if (newHeadTemp != BODY_SENSOR_DEFAULT) {
        adjustedHeadTemp = (adjustedHeadTemp * SMOOTH_WEIGHT) + (newHeadTemp * (1 - SMOOTH_WEIGHT));
    }

    if (halosuit_temperature_value(ARMPITS, &newArmpitTemp)) {
        logger_log("ERROR: ARMPIT TEMPERATURE READ FAILURE");
    }
    else if (newArmpitTemp != BODY_SENSOR_DEFAULT) {
        adjustedArmpitTemp =  (adjustedArmpitTemp * SMOOTH_WEIGHT) + (newArmpitTemp * (1 - SMOOTH_WEIGHT));
    }
    
    if (halosuit_temperature_value(CROTCH, &newCrotchTemp)) {
        logger_log("ERROR: CROTCH TEMPERATURE READ FAILURE");
    }
    else if (newCrotchTemp != BODY_SENSOR_DEFAULT) {
        adjustedCrotchTemp = (adjustedCrotchTemp * SMOOTH_WEIGHT) + (newCrotchTemp * (1 - SMOOTH_WEIGHT));
    }

    averageBodyTemp = (adjustedArmpitTemp + adjustedCrotchTemp) / 2;

    checkHeadTemperature(adjustedHeadTemp);
    checkBodyTemperature(averageBodyTemp); 
}

static void check_2AH_voltage()
{
    int voltage = TURNIGY_2AH_VOLTAGE;
    halosuit_voltage_value(TURNIGY_2_AH, &voltage);

    if (voltage < MIN_VOLTAGE || voltage > MAX_VOLTAGE) {
        if (halosuit_relay_switch(ON_BUTTON, LOW)) {
            logger_log("ERROR: SHUTDOWN FAILURE");
        }
    }
}

static void check_8AH_voltage()
{
    int voltage = TURNIGY_8AH_VOLTAGE;
    int live_value = 1;
    int ground_value = 1;
    halosuit_voltage_value(TURNIGY_8_AH, &voltage);

    if (halosuit_relay_value(HIGH_CURRENT_LIVE, &live_value)) {
        logger_log("ERROR: HIGH_CURRENT_LIVE READ FAILURE");
    }

    if (halosuit_relay_value(HIGH_CURRENT_GROUND, &ground_value)) {
        logger_log("ERROR: HIGH_CURRENT_GROUND READ FAILURE");
    }


    if (live_value == HIGH || ground_value == HIGH) {
        if (voltage < MIN_VOLTAGE || voltage > MAX_VOLTAGE) {
            if (halosuit_relay_switch(HIGH_CURRENT_LIVE, LOW) || halosuit_relay_switch(HIGH_CURRENT_GROUND, LOW)) {
                logger_log("ERROR: SHUTDOWN FAILURE");
            }
        }
    }
    else if (live_value == LOW || ground_value == LOW) {
        if (voltage > MIN_VOLTAGE || voltage < MAX_VOLTAGE) {
            if (halosuit_relay_switch(HIGH_CURRENT_LIVE, HIGH) || halosuit_relay_switch(HIGH_CURRENT_GROUND, HIGH)) {
                logger_log("ERROR: START UP FAILURE");
            }
        }
    }

}


static void check_battery()
{
    if (soc_getcharge(TURNIGY_8_AH) < LOW_BATTERY_THRESHOLD) {
        peltierLocked = true;
        if (halosuit_relay_switch(PELTIER, LOW)) {
            logger_log("ERROR: PELTIER READ FAILURE");
        }
        turnigy_8AH_low_warning = LOW_SOC;
    }
    else {
        turnigy_8AH_low_warning = NOMINAL_SOC;
        peltierLocked = false;
    }

    if (soc_getcharge(TURNIGY_2_AH) < LOW_BATTERY_THRESHOLD) {
        turnigy_2AH_low_warning = LOW_SOC;
    }
    else {
        turnigy_2AH_low_warning = NOMINAL_SOC;
    }

    if (soc_getcharge(GLASS_BATTERY) < LOW_BATTERY_THRESHOLD) {
        glass_battery_low_warning = LOW_SOC;
    }
    else {
        glass_battery_low_warning = NOMINAL_SOC;
    }

    if (soc_getcharge(PHONE_BATTERY) < LOW_BATTERY_THRESHOLD) {
        phone_battery_low_warning = LOW_SOC;   
    }
    else {
        phone_battery_low_warning = NOMINAL_SOC;
    }
}

static void* automationThread()
{ 
    automationIsDone = false;

    if (halosuit_relay_switch(PELTIER, HIGH)) {
            logger_log("ERROR: PELTIER READ FAILURE");
    }
    else {
        peltier_timein = time(NULL);
    }
    
    if (halosuit_relay_switch(WATER_PUMP, HIGH)) {
            logger_log("ERROR: WATER_PUMP READ FAILURE");
    }
    else {
        pump_timein = time(NULL);
    }

    sleep(START_DELAY); // to prevent the suit from reading weird startup values

    logger_log("Starting suit automation");

    while (!automationIsDone) {

        peltier_automation();
        pump_automation();
        waterTempLogic();
        bodyTemperatureLogic(); 
        checkFlow();
        check_8AH_voltage();
        check_2AH_voltage();
        check_battery();
        
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
    automationIsDone = true;
    pthread_join(automation_id, NULL);
}

void automation_peltier_off()
{
    if (halosuit_relay_switch(PELTIER, LOW)) {
        logger_log("ERROR: PELTIER READ FAILURE");
    }
    peltierAuto = false;
}

void automation_peltier_auto()
{
    if (halosuit_relay_switch(PELTIER, HIGH)) {
        logger_log("ERROR: PELTIER READ FAILURE");
    }
    peltier_timein = time(NULL);
    peltierAuto = true;
}

void automation_pump_off()
{
    if (halosuit_relay_switch(WATER_PUMP, LOW)) {
        logger_log("ERROR: WATER_PUMP READ FAILURE");
    }
    pumpAuto = false;
}

void automation_pump_auto()
{
    if (halosuit_relay_switch(WATER_PUMP, HIGH)) {
        logger_log("ERROR: WATER_PUMP READ FAILURE");
    }
    pump_timein = time(NULL);
    pumpAuto = true; 
}

char automation_getHeadTempWarning()
{
    return headTempWarning;
}

char automation_getBodyTempWarning()
{
    return bodyTempWarning;
}

char automation_getWaterTempWarning() {
    return waterTempWarning;
}

char automation_getWaterFlowWarning()
{
    return waterFlowWarning;
}

char automation_getBatteryWarning(int batteryID) 
{
    if (batteryID == TURNIGY_8_AH) {
        return turnigy_8AH_low_warning;
    }
    else if (batteryID == TURNIGY_2_AH) {
        return turnigy_2AH_low_warning;
    }
    else if (batteryID == GLASS_BATTERY) {
        return glass_battery_low_warning;
    }
    else if (batteryID == PHONE_BATTERY) {
        return phone_battery_low_warning;
    }
}

/**
 ******************************************************************************
 * @addtogroup OpenPilotModules OpenPilot Modules
 * @{
 * @addtogroup AirspeedModule Airspeed Module
 * @brief Calculate airspeed from diverse sources and update @ref Airspeed "Airspeed UAV Object"
 * @{
 *
 * @file       airspeed.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2012.
 * @brief      Airspeed module
 *
 * @see        The GNU Public License (GPL) Version 3
 *
 *****************************************************************************/
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
 * for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

/**
 * Output object: AirspeedSensor
 *
 * This module will periodically update the value of the AirspeedSensor object.
 *
 */

#include <openpilot.h>

#include "hwsettings.h"
#include "airspeedsettings.h"
#include "airspeedsensor.h" // object that will be updated by the module
#include "baro_airspeed_etasv3.h"
#include "baro_airspeed_mpxv.h"
#include "gps_airspeed.h"
#include "taskinfo.h"

// Private constants

#define STACK_SIZE_BYTES 500


#define TASK_PRIORITY    (tskIDLE_PRIORITY + 1)

// Private types

// Private variables
static xTaskHandle taskHandle;
static bool airspeedEnabled  = false;
static AirspeedSettingsData airspeedSettings;

static int8_t airspeedADCPin = -1;


// Private functions
static void airspeedTask(void *parameters);
static void AirspeedSettingsUpdatedCb(UAVObjEvent *ev);


/**
 * Initialise the module, called on startup
 * \returns 0 on success or -1 if initialisation failed
 */
int32_t AirspeedStart()
{
    // Check if module is enabled or not
    if (airspeedEnabled == false) {
        return -1;
    }

    // Start main task
    xTaskCreate(airspeedTask, (signed char *)"Airspeed", STACK_SIZE_BYTES / 4, NULL, TASK_PRIORITY, &taskHandle);
    PIOS_TASK_MONITOR_RegisterTask(TASKINFO_RUNNING_AIRSPEED, taskHandle);
    return 0;
}

/**
 * Initialise the module, called on startup
 * \returns 0 on success or -1 if initialisation failed
 */
int32_t AirspeedInitialize()
{
#ifdef MODULE_AIRSPEED_BUILTIN
    airspeedEnabled = true;
#else

    HwSettingsInitialize();
    uint8_t optionalModules[HWSETTINGS_OPTIONALMODULES_NUMELEM];
    HwSettingsOptionalModulesGet(optionalModules);


    if (optionalModules[HWSETTINGS_OPTIONALMODULES_AIRSPEED] == HWSETTINGS_OPTIONALMODULES_ENABLED) {
        airspeedEnabled = true;
    } else {
        airspeedEnabled = false;
        return -1;
    }
#endif

    uint8_t adcRouting[HWSETTINGS_ADCROUTING_NUMELEM];
    HwSettingsADCRoutingGet(adcRouting);

    // Determine if the barometric airspeed sensor is routed to an ADC pin
    for (int i = 0; i < HWSETTINGS_ADCROUTING_NUMELEM; i++) {
        if (adcRouting[i] == HWSETTINGS_ADCROUTING_ANALOGAIRSPEED) {
            airspeedADCPin = i;
        }
    }

    AirspeedSensorInitialize();
    AirspeedSettingsInitialize();

    AirspeedSettingsConnectCallback(AirspeedSettingsUpdatedCb);

    return 0;
}
MODULE_INITCALL(AirspeedInitialize, AirspeedStart);


/**
 * Module thread, should not return.
 */
static void airspeedTask(__attribute__((unused)) void *parameters)
{
    AirspeedSettingsUpdatedCb(AirspeedSettingsHandle());

    AirspeedSensorData airspeedData;
    AirspeedSensorGet(&airspeedData);

    AirspeedSettingsUpdatedCb(NULL);

    gps_airspeedInitialize();

    airspeedData.SensorConnected = AIRSPEEDSENSOR_SENSORCONNECTED_FALSE;

    // Main task loop
    portTickType lastSysTime = xTaskGetTickCount();
    while (1) {
        vTaskDelayUntil(&lastSysTime, airspeedSettings.SamplePeriod / portTICK_RATE_MS);

        // Update the airspeed object
        AirspeedSensorGet(&airspeedData);

        switch (airspeedSettings.AirspeedSensorType) {
#if defined(PIOS_INCLUDE_MPXV)
        case AIRSPEEDSETTINGS_AIRSPEEDSENSORTYPE_DIYDRONESMPXV7002:
        case AIRSPEEDSETTINGS_AIRSPEEDSENSORTYPE_DIYDRONESMPXV5004:
            // MPXV5004 and MPXV7002 sensors
            baro_airspeedGetMPXV(&airspeedData, &airspeedSettings, airspeedADCPin);
            break;
#endif
#if defined(PIOS_INCLUDE_ETASV3)
        case AIRSPEEDSETTINGS_AIRSPEEDSENSORTYPE_EAGLETREEAIRSPEEDV3:
            // Eagletree Airspeed v3
            baro_airspeedGetETASV3(&airspeedData, &airspeedSettings);
            break;
#endif
        case AIRSPEEDSETTINGS_AIRSPEEDSENSORTYPE_GROUNDSPEEDBASEDWINDESTIMATION:
            gps_airspeedGet(&airspeedData, &airspeedSettings);
            break;
        default:
            airspeedData.SensorConnected = AIRSPEEDSENSOR_SENSORCONNECTED_FALSE;
        }

        // Set the UAVO
        AirspeedSensorSet(&airspeedData);
    }
}


static void AirspeedSettingsUpdatedCb(__attribute__((unused)) UAVObjEvent *ev)
{
    AirspeedSettingsGet(&airspeedSettings);
}


/**
 * @}
 * @}
 */

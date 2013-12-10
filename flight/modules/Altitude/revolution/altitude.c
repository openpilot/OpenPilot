/**
 ******************************************************************************
 * @addtogroup OpenPilotModules OpenPilot Modules
 * @{
 * @addtogroup AltitudeModule Altitude Module
 * @brief Communicate with BMP085 and update @ref BaroSensor "BaroSensor UAV Object"
 * @{
 *
 * @file       altitude.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      Altitude module, handles temperature and pressure readings from BMP085
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
 * Output object: BaroSensor
 *
 * This module will periodically update the value of the BaroSensor object.
 *
 */

#include <openpilot.h>

#include "altitude.h"
#include "barosensor.h" // object that will be updated by the module
#include "revosettings.h"
#if defined(PIOS_INCLUDE_HCSR04)
#include "sonaraltitude.h" // object that will be updated by the module
#endif
#include "taskinfo.h"

// Private constants
#define STACK_SIZE_BYTES 550
#define TASK_PRIORITY    (tskIDLE_PRIORITY + 1)

// Private types

// Private variables
static xTaskHandle taskHandle;
static RevoSettingsBaroTempCorrectionPolynomialData baroCorrection;
// Private functions
static void altitudeTask(void *parameters);
static void SettingsUpdatedCb(UAVObjEvent *ev);

/**
 * Initialise the module, called on startup
 * \returns 0 on success or -1 if initialisation failed
 */
int32_t AltitudeStart()
{
    // Start main task
    xTaskCreate(altitudeTask, (signed char *)"Altitude", STACK_SIZE_BYTES / 4, NULL, TASK_PRIORITY, &taskHandle);
    PIOS_TASK_MONITOR_RegisterTask(TASKINFO_RUNNING_ALTITUDE, taskHandle);

    return 0;
}

/**
 * Initialise the module, called on startup
 * \returns 0 on success or -1 if initialisation failed
 */
int32_t AltitudeInitialize()
{
    BaroSensorInitialize();
    RevoSettingsInitialize();
    RevoSettingsConnectCallback(&SettingsUpdatedCb);

#if defined(PIOS_INCLUDE_HCSR04)
    SonarAltitudeInitialize();
#endif
    return 0;
}
MODULE_INITCALL(AltitudeInitialize, AltitudeStart);
/**
 * Module thread, should not return.
 */
static void altitudeTask(__attribute__((unused)) void *parameters)
{
    BaroSensorData data;

#if defined(PIOS_INCLUDE_HCSR04)
    SonarAltitudeData sonardata;
    int32_t value = 0, timeout = 10, sample_rate = 0;
    float coeff   = 0.25, height_out = 0, height_in = 0;
    PIOS_HCSR04_Trigger();
#endif

    // TODO: Check the pressure sensor and set a warning if it fails test

// Option to change the interleave between Temp and Pressure conversions
// Undef for normal operation
// #define PIOS_MS5611_SLOW_TEMP_RATE 20

    RevoSettingsBaroTempCorrectionPolynomialGet(&baroCorrection);

#ifdef PIOS_MS5611_SLOW_TEMP_RATE
    uint8_t temp_press_interleave_count = 1;
#endif
    // Main task loop
    while (1) {
#if defined(PIOS_INCLUDE_HCSR04)
        // Compute the current altitude
        // depends on baro samplerate
        if (!(sample_rate--)) {
            if (PIOS_HCSR04_Completed()) {
                value = PIOS_HCSR04_Get();
                // from 3.4cm to 5.1m
                if ((value > 100) && (value < 15000)) {
                    height_in  = value * 0.00034f / 2.0f;
                    height_out = (height_out * (1 - coeff)) + (height_in * coeff);
                    sonardata.Altitude = height_out; // m/us
                }

                // Update the SonarAltitude UAVObject
                SonarAltitudeSet(&sonardata);
                timeout = 10;
                PIOS_HCSR04_Trigger();
            }
            if (!(timeout--)) {
                // retrigger
                timeout = 10;
                PIOS_HCSR04_Trigger();
            }
            sample_rate = 25;
        }
#endif /* if defined(PIOS_INCLUDE_HCSR04) */
        float temp, press;
#ifdef PIOS_MS5611_SLOW_TEMP_RATE
        temp_press_interleave_count--;
        if (temp_press_interleave_count == 0) {
#endif
        // Update the temperature data
        PIOS_MS5611_StartADC(TemperatureConv);
        vTaskDelay(PIOS_MS5611_GetDelay());
        PIOS_MS5611_ReadADC();

#ifdef PIOS_MS5611_SLOW_TEMP_RATE
        temp_press_interleave_count = PIOS_MS5611_SLOW_TEMP_RATE;
    }
#endif

        // Update the pressure data
        PIOS_MS5611_StartADC(PressureConv);
        vTaskDelay(PIOS_MS5611_GetDelay());
        PIOS_MS5611_ReadADC();

        temp  = PIOS_MS5611_GetTemperature();
        press = PIOS_MS5611_GetPressure();
        float temp2      = temp * temp;
        press = press - baroCorrection.a + temp * baroCorrection.b + temp2 * baroCorrection.c + temp * temp2 * baroCorrection.d;

        float altitude   = 44330.0f * (1.0f - powf((press) / MS5611_P0, (1.0f / 5.255f)));

        if (!isnan(altitude)) {
            data.Altitude    = altitude;
            data.Temperature = temp;
            data.Pressure    = press;
            // Update the BasoSensor UAVObject
            BaroSensorSet(&data);
        }
    }
}

static void SettingsUpdatedCb(__attribute__((unused)) UAVObjEvent *ev)
{
    RevoSettingsBaroTempCorrectionPolynomialGet(&baroCorrection);
}
/**
 * @}
 * @}
 */

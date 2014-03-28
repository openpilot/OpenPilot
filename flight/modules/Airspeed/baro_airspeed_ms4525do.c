/**
 ******************************************************************************
 * @addtogroup OpenPilotModules OpenPilot Modules
 * @{
 * @addtogroup AirspeedModule Airspeed Module
 * @brief Communicate with airspeed sensors and return values
 * @{
 *
 * @file       baro_airspeed_ms4525do.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2014.
 * @brief      Airspeed module, handles temperature and pressure readings from MS4525DO
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
 * Output object: BaroAirspeed
 *
 * This module will periodically update the value of the BaroAirspeed object.
 *
 */

#include "openpilot.h"
#include "hwsettings.h"
#include "airspeedsettings.h"
#include "airspeedsensor.h" // object that will be updated by the module
#include "airspeedalarm.h"
#include "taskinfo.h"

#if defined(PIOS_INCLUDE_MS4525DO)

#define CALIBRATION_IDLE_MS  0              // Time to wait before calibrating, in [ms]
#define CALIBRATION_COUNT_MS 4000 // Time to spend calibrating, in [ms]
#define FILTER_SHIFT         5                      // Barry Dorr filter parameter k

#define P0                   101325.0f           // standard pressure
#define CCEXPONENT           0.2857142857f       // exponent of compressibility correction 2/7
#define CASFACTOR            760.8802669f        // sqrt(5) * speed of sound at standard
#define TASFACTOR            0.05891022589f      // 1/sqrt(T0)

#define max(x, y) ((x) >= (y) ? (x) : (y))

// Private types

// Private functions definitions
static int8_t baro_airspeedReadMS4525DO(AirspeedSensorData *airspeedSensor, AirspeedSettingsData *airspeedSettings);


// Private variables
static uint16_t calibrationCount = 0;
static uint32_t filter_reg = 0; // Barry Dorr filter register

void baro_airspeedGetMS4525DO(AirspeedSensorData *airspeedSensor, AirspeedSettingsData *airspeedSettings)
{
    // request measurement first
    int8_t retVal = PIOS_MS4525DO_Request();

    if (retVal != 0) {
        AirspeedAlarm(SYSTEMALARMS_ALARM_ERROR);
        
        return;
    }

    // Datasheet of MS4525DO: conversion needs 0.5 ms + 20% more when status bit used
    // delay by one Tick or at least 2 ms
    const portTickType xDelay = max(2 / portTICK_RATE_MS, 1);
    vTaskDelay(xDelay);

    // read the sensor
    retVal = baro_airspeedReadMS4525DO(airspeedSensor, airspeedSettings);

    switch (retVal) {
    case  0:    AirspeedAlarm(SYSTEMALARMS_ALARM_OK);
        break;
    case -4:
    case -5:
    case -7:    AirspeedAlarm(SYSTEMALARMS_ALARM_WARNING);
        break;
    case -1:
    case -2:
    case -3:
    case -6:
    default:    AirspeedAlarm(SYSTEMALARMS_ALARM_ERROR);
    }
}


// Private functions
static int8_t baro_airspeedReadMS4525DO(AirspeedSensorData *airspeedSensor, AirspeedSettingsData *airspeedSettings)
{
    // Check to see if airspeed sensor is returning airspeedSensor
    uint16_t values[2];
    int8_t retVal = PIOS_MS4525DO_Read(values);

    if (retVal == 0) {
        airspeedSensor->SensorValue = values[0];
        airspeedSensor->SensorValueTemperature = values[1];
    } else {
        airspeedSensor->SensorValue            = -1;
        airspeedSensor->SensorValueTemperature = -1;
        airspeedSensor->SensorConnected        = AIRSPEEDSENSOR_SENSORCONNECTED_FALSE;
        airspeedSensor->CalibratedAirspeed     = 0;
        return retVal;
    }

    // only calibrate if no stored calibration is available
    if (!airspeedSettings->ZeroPoint) {
        // Calibrate sensor by averaging zero point value
        if (calibrationCount <= CALIBRATION_IDLE_MS / airspeedSettings->SamplePeriod) {
            calibrationCount++;
            filter_reg = (airspeedSensor->SensorValue << FILTER_SHIFT);
            return -7;
        } else if (calibrationCount <= (CALIBRATION_IDLE_MS + CALIBRATION_COUNT_MS) / airspeedSettings->SamplePeriod) {
            calibrationCount++;
            // update filter register
            filter_reg = filter_reg - (filter_reg >> FILTER_SHIFT) + airspeedSensor->SensorValue;

            if (calibrationCount > (CALIBRATION_IDLE_MS + CALIBRATION_COUNT_MS) / airspeedSettings->SamplePeriod) {
                // Scale output for unity gain.
                airspeedSettings->ZeroPoint = (uint16_t)(filter_reg >> FILTER_SHIFT);

                AirspeedSettingsZeroPointSet(&airspeedSettings->ZeroPoint);
                calibrationCount = 0;
            }
            return -7;
        }
    }

    /*  Compute airspeed
        assume sensor is A Type and has a range of 1 psi, i.e. Pmin=-1.0 psi and Pmax=1.0 psi
        Datasheet pressure: output = 0.8 * 16383 / (Pmax-Pmin) * (P - Pmin) + 0.1 * 16383
        Inversion: P = (10*output - 81915)/65532 in psi
        1 psi = 6894,757293168 Pa
        P = (10*output - 81915)*0.1052120688 in Pa
        Datasheet temperature: output = (T+50)*2047 / 200
        Inversion: T = (200*out - 102350)/2047 in C
        T = (200*out - 102350)/2047 + 273.15 in K
     */
    const float dP = (10 * (int32_t)(airspeedSensor->SensorValue - airspeedSettings->ZeroPoint)) * 0.1052120688f;
    const float T  = (float)(200 * (int32_t)airspeedSensor->SensorValueTemperature - 102350) / 2047 + 273.15f;

    airspeedSensor->DifferentialPressure = dP;
    airspeedSensor->Temperature = T;
    // CAS  = Csound * sqrt( 5 *( (dP/P0 +1)^(2/7) - 1) )
    // TAS  = Csound * sqrt( 5 T/T0 *( (dP/P0 +1)^(2/7) - 1) )
    // where Csound = 340.276 m/s at standard condition T0=288.15 K and P0 = 101315 Pa
    airspeedSensor->CalibratedAirspeed = airspeedSettings->Scale * CASFACTOR * sqrtf(powf(fabsf(dP) / P0 + 1.0f, CCEXPONENT) - 1.0f);
    airspeedSensor->TrueAirspeed = airspeedSensor->CalibratedAirspeed * TASFACTOR * sqrtf(T);
    airspeedSensor->SensorConnected    = AIRSPEEDSENSOR_SENSORCONNECTED_TRUE;

    return retVal;
}


#endif /* if defined(PIOS_INCLUDE_MS4525DO) */

/**
 * @}
 * @}
 */

/**
 ******************************************************************************
 * @addtogroup OpenPilotModules OpenPilot Modules
 * @{ 
 * @addtogroup AirspeedModule Airspeed Module
 * @brief Communicate with airspeed sensors and return values
 * @{ 
 *
 * @file       baro_airspeed.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      Airspeed module, handles temperature and pressure readings from BMP085
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
#include "airspeedsensor.h"	// object that will be updated by the module

#if defined(PIOS_INCLUDE_ETASV3)

#define CALIBRATION_IDLE_MS                     2000   //Time to wait before calibrating, in [ms]
#define CALIBRATION_COUNT_MS                    2000   //Time to spend calibrating, in [ms]

// Private types

// Private variables

// Private functions

static uint16_t calibrationCount=0;
static uint16_t calibrationCount2=0;
static uint32_t calibrationSum = 0;


void baro_airspeedGetETASV3(AirspeedSensorData *airspeedSensor, AirspeedSettingsData *airspeedSettings){

	//Check to see if airspeed sensor is returning airspeedSensor
	airspeedSensor->SensorValue = PIOS_ETASV3_ReadAirspeed();
	if (airspeedSensor->SensorValue==-1) {
		airspeedSensor->SensorConnected = AIRSPEEDSENSOR_SENSORCONNECTED_FALSE;
		airspeedSensor->CalibratedAirspeed = 0;
		return;
	}
	
	// only calibrate if no stored calibration is available
	if (!airspeedSettings->ZeroPoint) {
		//Calibrate sensor by averaging zero point value
		if (calibrationCount <= CALIBRATION_IDLE_MS/airspeedSettings->SamplePeriod) {
			calibrationCount++;
			calibrationCount2++;
			return;
		} else if (calibrationCount <= (CALIBRATION_IDLE_MS + CALIBRATION_COUNT_MS)/airspeedSettings->SamplePeriod) {
			calibrationCount++;
			calibrationCount2++;
			calibrationSum +=  airspeedSensor->SensorValue;
			if (calibrationCount > (CALIBRATION_IDLE_MS + CALIBRATION_COUNT_MS)/airspeedSettings->SamplePeriod) {

				airspeedSettings->ZeroPoint = (int16_t) (((float)calibrationSum) / calibrationCount2);
				AirspeedSettingsZeroPointSet( &airspeedSettings->ZeroPoint );
			}
			return;
		}
	}
	
	//Compute airspeed
	airspeedSensor->CalibratedAirspeed = airspeedSettings->Scale * sqrtf((float)abs(airspeedSensor->SensorValue - airspeedSettings->ZeroPoint));
	airspeedSensor->SensorConnected = AIRSPEEDSENSOR_SENSORCONNECTED_TRUE;
}


#endif

/**
 * @}
 * @}
 */

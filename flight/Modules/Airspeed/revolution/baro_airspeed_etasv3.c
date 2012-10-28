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
#include "airspeed.h"
#include "airspeedsettings.h"
#include "baroairspeed.h"	// object that will be updated by the module

#if defined(PIOS_INCLUDE_ETASV3)

#define SAMPLING_DELAY_MS_ETASV3                50     //Update at 20Hz
#define ETS_AIRSPEED_SCALE                      1.0f   //???
#define CALIBRATION_IDLE_MS                     2000   //Time to wait before calibrating, in [ms]
#define CALIBRATION_COUNT_MS                    2000   //Time to spend calibrating, in [ms]
#define ANALOG_BARO_AIRSPEED_TIME_CONSTANT_MS   100.0f //Needs to be settable in a UAVO

// Private types

// Private variables

// Private functions

static uint16_t calibrationCount=0;


void baro_airspeedGetETASV3(BaroAirspeedData *baroAirspeedData, portTickType *lastSysTime, uint8_t airspeedSensorType, int8_t airspeedADCPin){

	static uint32_t calibrationSum = 0;
	AirspeedSettingsData airspeedSettingsData;
	AirspeedSettingsGet(&airspeedSettingsData);

	//Wait until our turn. //THIS SHOULD BE, IF OUR TURN GO IN, OTHERWISE CONTINUE
	vTaskDelayUntil(lastSysTime, SAMPLING_DELAY_MS_ETASV3 / portTICK_RATE_MS);
	
	//Check to see if airspeed sensor is returning baroAirspeedData
	baroAirspeedData->SensorValue = PIOS_ETASV3_ReadAirspeed();
	if (baroAirspeedData->SensorValue==-1) {
		baroAirspeedData->BaroConnected = BAROAIRSPEED_BAROCONNECTED_FALSE;
		baroAirspeedData->CalibratedAirspeed = 0;
		BaroAirspeedSet(&baroAirspeedData);
		return;
	}
	
	//Calibrate sensor by averaging zero point value //THIS SHOULD NOT BE DONE IF THERE IS AN IN-AIR RESET. HOW TO DETECT THIS?
	if (calibrationCount < CALIBRATION_IDLE_MS/SAMPLING_DELAY_MS_ETASV3) {
		calibrationCount++;
		return;
	} else if (calibrationCount < (CALIBRATION_IDLE_MS + CALIBRATION_COUNT_MS)/SAMPLING_DELAY_MS_ETASV3) {
		calibrationCount++;
		calibrationSum +=  baroAirspeedData->SensorValue;
		if (calibrationCount == (CALIBRATION_IDLE_MS + CALIBRATION_COUNT_MS)/SAMPLING_DELAY_MS_ETASV3) {

			airspeedSettingsData.ZeroPoint = (int16_t) (((float)calibrationSum) / CALIBRATION_COUNT_MS +0.5f);
			AirspeedSettingsZeroPointSet( &airspeedSettingsData.ZeroPoint );
		} else {
			return;
		}
	}
	
	//Compute airspeed
	float calibratedAirspeed = ETS_AIRSPEED_SCALE * sqrtf((float)abs(baroAirspeedData->SensorValue - airspeedSettingsData.ZeroPoint)); //Is this calibrated or indicated airspeed?
	
	baroAirspeedData->BaroConnected = BAROAIRSPEED_BAROCONNECTED_TRUE;
	baroAirspeedData->CalibratedAirspeed = calibratedAirspeed;
}


#endif

/**
 * @}
 * @}
 */

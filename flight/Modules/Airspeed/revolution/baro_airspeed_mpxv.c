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

#if defined(PIOS_INCLUDE_MPXV)

#define CALIBRATION_IDLE_MS                     2000   //Time to wait before calibrating, in [ms]
#define CALIBRATION_COUNT_MS                    2000   //Time to spend calibrating, in [ms]
#define ANALOG_BARO_AIRSPEED_TIME_CONSTANT_MS    100   //low pass filter

// Private types

// Private variables

// Private functions

static uint16_t calibrationCount=0;

PIOS_MPXV_descriptor sensor = { .type=PIOS_MPXV_UNKNOWN };


void baro_airspeedGetMPXV(AirspeedSensorData *airspeedSensor, AirspeedSettingsData *airspeedSettings, int8_t airspeedADCPin){

	//Ensure that the ADC pin is properly configured
	if(airspeedADCPin <0){
		airspeedSensor->SensorConnected = AIRSPEEDSENSOR_SENSORCONNECTED_FALSE;
		
		return;
	}
	if (sensor.type==PIOS_MPXV_UNKNOWN) {
		switch (airspeedSettings->AirspeedSensorType) {
			case AIRSPEEDSETTINGS_AIRSPEEDSENSORTYPE_DIYDRONESMPXV7002:
				sensor = PIOS_MPXV_7002_DESC(airspeedADCPin);
				break;
			case AIRSPEEDSETTINGS_AIRSPEEDSENSORTYPE_DIYDRONESMPXV5004:
				sensor = PIOS_MPXV_5004_DESC(airspeedADCPin);
				break;
			default:
				airspeedSensor->SensorConnected = AIRSPEEDSENSOR_SENSORCONNECTED_FALSE;
				return;
		}
	}

	airspeedSensor->SensorValue = PIOS_MPXV_Measure(&sensor);

	if (!airspeedSettings->ZeroPoint) {
		//Calibrate sensor by averaging zero point value
		if (calibrationCount < CALIBRATION_IDLE_MS/airspeedSettings->SamplePeriod) { //First let sensor warm up and stabilize.
			calibrationCount++;
			return;
		} else if (calibrationCount <= (CALIBRATION_IDLE_MS + CALIBRATION_COUNT_MS)/airspeedSettings->SamplePeriod) { //Then compute the average.
			calibrationCount++; /*DO NOT MOVE FROM BEFORE sensorCalibration=... LINE, OR ELSE WILL HAVE DIVIDE BY ZERO */

			PIOS_MPXV_Calibrate(&sensor,airspeedSensor->SensorValue);

			//Set settings UAVO. The airspeed UAVO is set elsewhere in the function.
			if (calibrationCount > (CALIBRATION_IDLE_MS + CALIBRATION_COUNT_MS)/airspeedSettings->SamplePeriod) {
				airspeedSettings->ZeroPoint = sensor.zeroPoint;
				AirspeedSettingsZeroPointSet(&airspeedSettings->ZeroPoint);
			}
			return;
		}
	}
	sensor.zeroPoint = airspeedSettings->ZeroPoint;
		
	//Filter CAS
	float alpha=airspeedSettings->SamplePeriod/(airspeedSettings->SamplePeriod + ANALOG_BARO_AIRSPEED_TIME_CONSTANT_MS); //Low pass filter.
	
	airspeedSensor->CalibratedAirspeed = PIOS_MPXV_CalcAirspeed(&sensor,airspeedSensor->SensorValue) * (alpha) + airspeedSensor->CalibratedAirspeed*(1.0f-alpha);
	airspeedSensor->SensorConnected = AIRSPEEDSENSOR_SENSORCONNECTED_TRUE;

}

#endif

/**
 * @}
 * @}
 */

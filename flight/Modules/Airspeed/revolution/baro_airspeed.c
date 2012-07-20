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

#define AIRSPEED_STORED_IN_A_PROPER_UAVO FALSE //AIRSPEED SHOULD NOT GO INTO BaroAirspeed. IT IS A STATE VARIABLE, AND NOT A MEASUREMENT

#include "openpilot.h"
#include "hwsettings.h"
#include "airspeed.h"
#include "airspeedsettings.h"
#include "baroairspeed.h"	// object that will be updated by the module


#define SAMPLING_DELAY_MS_ETASV3                50     //Update at 20Hz
#define SAMPLING_DELAY_MS_MPXV                  10     //Update at 100Hz
#define ETS_AIRSPEED_SCALE                      1.0f   //???
#define CALIBRATION_IDLE_MS                     2000   //Time to wait before calibrating, in [ms]
#define CALIBRATION_COUNT_MS                    2000   //Time to spend calibrating, in [ms]
#define ANALOG_BARO_AIRSPEED_TIME_CONSTANT_MS   100.0f //Needs to be settable in a UAVO

// Private types

// Private variables

// Private functions

static uint16_t calibrationCount=0;

void baro_airspeedGet(BaroAirspeedData *baroAirspeedData, portTickType *lastSysTime, uint8_t airspeedSensorType, int8_t airspeedADCPin){
	
#if defined(PIOS_INCLUDE_ETASV3)	
	static uint32_t calibrationSum = 0;
#endif	
	
	//Find out which sensor we're using.
	if (airspeedSensorType==AIRSPEEDSETTINGS_AIRSPEEDSENSORTYPE_DIYDRONESMPXV7002 || airspeedSensorType==AIRSPEEDSETTINGS_AIRSPEEDSENSORTYPE_DIYDRONESMPXV5004){ 		//MPXV5004 and MPXV7002 sensors
		
#if defined(PIOS_INCLUDE_MPXV7002) || defined (PIOS_INCLUDE_MPXV5004)
		//Ensure that the ADC pin is properly configured
		if(airspeedADCPin <0){ //It's not, so revert to former sensor type
			baroAirspeedData->BaroConnected = BAROAIRSPEED_BAROCONNECTED_FALSE;
			AirspeedSettingsData airspeedSettingsData;
			AirspeedSettingsGet(&airspeedSettingsData);
			
			airspeedSettingsData.AirspeedSensorType=AIRSPEEDSETTINGS_AIRSPEEDSENSORTYPE_GPSONLY;
			
			AirspeedSettingsSet(&airspeedSettingsData); 			// IS THERE ANY WAY TO SET A MUTEX ON THIS BEFORE SETTING IT?
			
			return;
		}
		//Wait until our turn //THIS SHOULD BE, IF OUR TURN GO IN, OTHERWISE CONTINUE
		vTaskDelayUntil(lastSysTime, SAMPLING_DELAY_MS_MPXV / portTICK_RATE_MS);
				
		//Calibrate sensor by averaging zero point value //THIS SHOULD NOT BE DONE IF THERE IS AN IN-AIR RESET. HOW TO DETECT THIS?
		if (calibrationCount < CALIBRATION_IDLE_MS/SAMPLING_DELAY_MS_MPXV) { //First let sensor warm up and stabilize.
			calibrationCount++;
			return;
		} else if (calibrationCount <= (CALIBRATION_IDLE_MS + CALIBRATION_COUNT_MS)/SAMPLING_DELAY_MS_MPXV) { //Then compute the average.
			calibrationCount++; /*DO NOT MOVE FROM BEFORE sensorCalibration=... LINE, OR ELSE WILL HAVE DIVIDE BY ZERO */
			AirspeedSettingsData airspeedSettingsData;
			AirspeedSettingsGet(&airspeedSettingsData);
			


			if(airspeedSensorType==AIRSPEEDSETTINGS_AIRSPEEDSENSORTYPE_DIYDRONESMPXV7002){
				uint16_t sensorCalibration=PIOS_MPXV7002_Calibrate(airspeedADCPin, calibrationCount-CALIBRATION_IDLE_MS/SAMPLING_DELAY_MS_MPXV);
				airspeedSettingsData.ZeroPoint=sensorCalibration;
				PIOS_MPXV7002_UpdateCalibration(airspeedSettingsData.ZeroPoint); //This makes sense for the user if the initial calibration was not good and the user does not wish to reboot.
			}
			else if(airspeedSensorType==AIRSPEEDSETTINGS_AIRSPEEDSENSORTYPE_DIYDRONESMPXV5004){
				uint16_t sensorCalibration=PIOS_MPXV5004_Calibrate(airspeedADCPin, calibrationCount-CALIBRATION_IDLE_MS/SAMPLING_DELAY_MS_MPXV);
				airspeedSettingsData.ZeroPoint=sensorCalibration;
				PIOS_MPXV5004_UpdateCalibration(airspeedSettingsData.ZeroPoint); //This makes sense for the user if the initial calibration was not good and the user does not wish to reboot.
			}


			baroAirspeedData->BaroConnected = BAROAIRSPEED_BAROCONNECTED_TRUE;
			
			//Set settings UAVO. The airspeed UAVO is set elsewhere in the function.
			if (calibrationCount == (CALIBRATION_IDLE_MS + CALIBRATION_COUNT_MS)/SAMPLING_DELAY_MS_MPXV)
				AirspeedSettingsSet(&airspeedSettingsData); 			// IS THERE ANY WAY TO SET A MUTEX ON THIS BEFORE SETTING IT?
			
			return;
		}
		
		//Get CAS
		float calibratedAirspeed=0;
		if(airspeedSensorType==AIRSPEEDSETTINGS_AIRSPEEDSENSORTYPE_DIYDRONESMPXV7002){
			calibratedAirspeed = PIOS_MPXV7002_ReadAirspeed(airspeedADCPin);
			if (calibratedAirspeed < 0) //This only occurs when there's a bad ADC reading. 
				return;
		
			//Get sensor value, just for telemetry purposes. 
			//This is a silly waste of resources, and should probably be removed at some point in the future.
			//At this time, PIOS_MPXV7002_Measure() should become a static function and be removed from the header file.
			//
			//Moreover, due to the way the ADC driver is currently written, this code will return 0 more often than
			//not. This is something that will have to change on the ADC side of things.
			baroAirspeedData->SensorValue=PIOS_MPXV7002_Measure(airspeedADCPin);
		
		}
		else if(airspeedSensorType==AIRSPEEDSETTINGS_AIRSPEEDSENSORTYPE_DIYDRONESMPXV5004){
			calibratedAirspeed = PIOS_MPXV5004_ReadAirspeed(airspeedADCPin);
			if (calibratedAirspeed < 0) //This only occurs when there's a bad ADC reading. 
				return;
			
			//Get sensor value, just for telemetry purposes. 
			//This is a silly waste of resources, and should probably be removed at some point in the future.
			//At this time, PIOS_MPXV7002_Measure() should become a static function and be removed from the header file.
			//
			//Moreover, due to the way the ADC driver is currently written, this code will return 0 more often than
			//not. This is something that will have to change on the ADC side of things.
			baroAirspeedData->SensorValue=PIOS_MPXV5004_Measure(airspeedADCPin);			

		}
		//Filter CAS
		float alpha=SAMPLING_DELAY_MS_MPXV/(SAMPLING_DELAY_MS_MPXV + ANALOG_BARO_AIRSPEED_TIME_CONSTANT_MS); //Low pass filter.
		float filteredAirspeed = calibratedAirspeed*(alpha) + baroAirspeedData->CAS*(1.0f-alpha);
		
		//Set two values, one for the UAVO airspeed sensor reading, and the other for the GPS corrected one
		baroAirspeedData->CAS = filteredAirspeed;
#else
		//Whoops, no sensor defined in pios_config.h. Revert to GPS.
		baroAirspeedData->BaroConnected = BAROAIRSPEED_BAROCONNECTED_FALSE;
		AirspeedSettingsData airspeedSettingsData;
		AirspeedSettingsGet(&airspeedSettingsData);
		airspeedSettingsData.AirspeedSensorType=AIRSPEEDSETTINGS_AIRSPEEDSENSORTYPE_GPSONLY;
		
		AirspeedSettingsSet(&airspeedSettingsData); 	// IS THERE ANY WAY TO SET A MUTEX ON THIS BEFORE SETTING IT?
		
		return;		
#endif
	}
	else if (airspeedSensorType==AIRSPEEDSETTINGS_AIRSPEEDSENSORTYPE_EAGLETREEAIRSPEEDV3){ //Eagletree Airspeed v3
#if defined(PIOS_INCLUDE_ETASV3)
		AirspeedSettingsData airspeedSettingsData;
		AirspeedSettingsGet(&airspeedSettingsData);

		//Wait until our turn. //THIS SHOULD BE, IF OUR TURN GO IN, OTHERWISE CONTINUE
		vTaskDelayUntil(lastSysTime, SAMPLING_DELAY_MS_ETASV3 / portTICK_RATE_MS);
		
		//Check to see if airspeed sensor is returning baroAirspeedData
		baroAirspeedData->SensorValue = PIOS_ETASV3_ReadAirspeed();
		if (baroAirspeedData->SensorValue==-1) {
			baroAirspeedData->BaroConnected = BAROAIRSPEED_BAROCONNECTED_FALSE;
			baroAirspeedData->Airspeed = 0;
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
				
				airspeedSettingsData.ZeroPoint=(uint16_t) (((float)calibrationSum) / CALIBRATION_COUNT_MS +0.5f);
				AirspeedSettingsSet(&airspeedSettingsData);				
			} else {
				return;
			}
		}
		
		//Compute airspeed
		float calibratedAirspeed = ETS_AIRSPEED_SCALE * sqrtf((float)abs(baroAirspeedData->SensorValue - airspeedSettingsData.ZeroPoint)); //Is this calibrated or indicated airspeed?
		
		baroAirspeedData->BaroConnected = BAROAIRSPEED_BAROCONNECTED_TRUE;
		baroAirspeedData->CAS = calibratedAirspeed;
#else
		//Whoops, no EagleTree sensor defined in pios_config.h. Declare it unconnected...
		baroAirspeedData->BaroConnected = BAROAIRSPEED_BAROCONNECTED_FALSE;
		
		//...and revert to GPS.
		if (airspeedSensorType!=AIRSPEEDSETTINGS_AIRSPEEDSENSORTYPE_GPSONLY) {
			uint8_t tmpVal=AIRSPEEDSETTINGS_AIRSPEEDSENSORTYPE_GPSONLY;			
			AirspeedSettingsAirspeedSensorTypeSet(&tmpVal);  // IS THERE ANY WAY TO SET A MUTEX ON THIS BEFORE SETTING IT?
		}
#endif
	}
}

/**
 * @}
 * @}
 */

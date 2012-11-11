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
 * Output object: BaroAirspeed
 *
 * This module will periodically update the value of the BaroAirspeed object.
 *
 */

#include "openpilot.h"
#include "hwsettings.h"
#include "airspeed.h"
#include "gpsvelocity.h"
#include "airspeedsettings.h"
#include "gps_airspeed.h"
#include "baroaltitude.h"
#include "baroairspeed.h" // object that will be updated by the module
#include "airspeedactual.h" // object that will be updated by the module
#include "attitudeactual.h"
#include "CoordinateConversions.h"
#include "baro_airspeed_etasv3.h"
#include "baro_airspeed_analog.h"

// Private constants
#if defined (PIOS_INCLUDE_GPS)
 #define GPS_AIRSPEED_PRESENT
#endif

#if defined (PIOS_INCLUDE_MPXV5004) || defined (PIOS_INCLUDE_MPXV7002) || defined (PIOS_INCLUDE_ETASV3)
 #define BARO_AIRSPEED_PRESENT
#endif

#if defined (GPS_AIRSPEED_PRESENT) && defined (BARO_AIRSPEED_PRESENT)
 #define STACK_SIZE_BYTES 700 
#elif defined (GPS_AIRSPEED_PRESENT)
 #define STACK_SIZE_BYTES 600
#elif defined (BARO_AIRSPEED_PRESENT)
 #define STACK_SIZE_BYTES 550
#else
 #define STACK_SIZE_BYTES 0
 #define NO_AIRSPEED_SENSOR_PRESENT
#endif


#define TASK_PRIORITY (tskIDLE_PRIORITY+1)

#define SAMPLING_DELAY_MS_FALLTHROUGH  50     //Fallthrough update at 20Hz. The fallthrough runs faster than the GPS to ensure that we don't miss GPS updates because we're slightly out of sync

#define GPS_AIRSPEED_BIAS_KP                   0.01f //Needs to be settable in a UAVO
#define GPS_AIRSPEED_BIAS_KI                   0.01f //Needs to be settable in a UAVO
#define GPS_AIRSPEED_TIME_CONSTANT_MS        500.0f  //Needs to be settable in a UAVO
#define BARO_TRUEAIRSPEED_TIME_CONSTANT_S      1.0f  //Needs to be settable in a UAVO

#define F_PI 3.141526535897932f
#define DEG2RAD (F_PI/180.0f)
#define T0 288.15f
#define BARO_TEMPERATURE_OFFSET 5

// Private types

// Private variables
static xTaskHandle taskHandle;
static bool airspeedEnabled = false;
volatile bool gpsNew = false;
static uint8_t airspeedSensorType;
static uint16_t gpsSamplePeriod_ms;

#ifdef BARO_AIRSPEED_PRESENT
static int8_t airspeedADCPin=-1;
#endif


// Private functions
static void airspeedTask(void *parameters);
void baro_airspeedGet(BaroAirspeedData *baroAirspeedData, portTickType *lastSysTime, uint8_t airspeedSensorType, int8_t airspeedADCPin);
static void AirspeedSettingsUpdatedCb(UAVObjEvent * ev);

#ifdef GPS_AIRSPEED_PRESENT
static void GPSVelocityUpdatedCb(UAVObjEvent * ev);
#endif


/**
 * Initialise the module, called on startup
 * \returns 0 on success or -1 if initialisation failed
 */
int32_t AirspeedStart()
{	
#if defined (NO_AIRSPEED_SENSOR_PRESENT)
	return -1;
#endif	
	
	//Check if module is enabled or not
	if (airspeedEnabled == false) {
		return -1;
	}
		
	// Start main task
	xTaskCreate(airspeedTask, (signed char *)"Airspeed", STACK_SIZE_BYTES/4, NULL, TASK_PRIORITY, &taskHandle);
	TaskMonitorAdd(TASKINFO_RUNNING_AIRSPEED, taskHandle);
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
	
#ifdef BARO_AIRSPEED_PRESENT
	uint8_t adcRouting[HWSETTINGS_ADCROUTING_NUMELEM];	
	HwSettingsADCRoutingGet(adcRouting);
	
	//Determine if the barometric airspeed sensor is routed to an ADC pin 
	for (int i=0; i < HWSETTINGS_ADCROUTING_NUMELEM; i++) {
		if (adcRouting[i] == HWSETTINGS_ADCROUTING_ANALOGAIRSPEED) {
			airspeedADCPin = i;
		}
	}
	
#endif	
	
	BaroAirspeedInitialize();
	AirspeedActualInitialize();
	AirspeedSettingsInitialize();
	
	AirspeedSettingsConnectCallback(AirspeedSettingsUpdatedCb);	
	
	return 0;
}
MODULE_INITCALL(AirspeedInitialize, AirspeedStart)


/**
 * Module thread, should not return.
 */
static void airspeedTask(void *parameters)
{
	AirspeedSettingsUpdatedCb(AirspeedSettingsHandle());
	
	BaroAirspeedData airspeedData;
	AirspeedActualData airspeedActualData;
	BaroAirspeedGet(&airspeedData);
	AirspeedActualGet(&airspeedActualData);


	airspeedData.BaroConnected = BAROAIRSPEED_BAROCONNECTED_FALSE;
	
#ifdef BARO_AIRSPEED_PRESENT		
	portTickType lastGPSTime = xTaskGetTickCount(); //Time since last GPS-derived airspeed calculation
	portTickType lastLoopTime= xTaskGetTickCount(); //Time since last loop

	float airspeedErrInt=0;
#endif
	
	//GPS airspeed calculation variables
#ifdef GPS_AIRSPEED_PRESENT
	GPSVelocityConnectCallback(GPSVelocityUpdatedCb);		
	gps_airspeedInitialize();
#endif
	
	// Main task loop
	portTickType lastSysTime = xTaskGetTickCount();
	while (1)
	{
		// Update the airspeed object
		BaroAirspeedGet(&airspeedData);
		AirspeedActualGet(&airspeedActualData);
		
#ifdef BARO_AIRSPEED_PRESENT
		float airspeed_tas_baro=0;
		
		if(airspeedSensorType!=AIRSPEEDSETTINGS_AIRSPEEDSENSORTYPE_GPSONLY){
			//Fetch calibrated airspeed from sensors
			baro_airspeedGet(&airspeedData, &lastSysTime, airspeedSensorType, airspeedADCPin);
			
			//Calculate true airspeed, not taking into account compressibility effects
			float baroTemperature;
			
			BaroAltitudeTemperatureGet(&baroTemperature);
			baroTemperature-=BARO_TEMPERATURE_OFFSET; //Do this just because we suspect that the board heats up relative to its surroundings. THIS IS BAD(tm)
 #ifdef GPS_AIRSPEED_PRESENT
			//GPS present, so use baro sensor to filter TAS
			airspeed_tas_baro=airspeedData.CalibratedAirspeed * sqrtf((baroTemperature+273.15)/T0) + airspeedErrInt * GPS_AIRSPEED_BIAS_KI;
 #else
			//No GPS, so TAS comes only from baro sensor
			airspeedData.TrueAirspeed=airspeedData.CalibratedAirspeed * sqrtf((baroTemperature+273.15)/T0) + airspeedErrInt * GPS_AIRSPEED_BIAS_KI;
 #endif			
			
		}
		else
#endif
		{ //Have to catch the fallthrough, or else this loop will monopolize the processor!
			airspeedData.BaroConnected=BAROAIRSPEED_BAROCONNECTED_FALSE;
			airspeedData.SensorValue=12345;
			
			//Likely, we have a GPS, so let's configure the fallthrough at close to GPS refresh rates
			vTaskDelayUntil(&lastSysTime, SAMPLING_DELAY_MS_FALLTHROUGH / portTICK_RATE_MS);
		}
		
#ifdef GPS_AIRSPEED_PRESENT
		float v_air_GPS=-1.0f;
		
		//Check if sufficient time has passed. This will depend on whether we have a pitot tube
		//sensor or not. In the case we do, shoot for about once per second. Otherwise, consume GPS
		//as quickly as possible.
 #ifdef BARO_AIRSPEED_PRESENT
		float delT = (lastSysTime - lastLoopTime)/(portTICK_RATE_MS*1000.0f);
		lastLoopTime=lastSysTime;
		if ( ((lastSysTime - lastGPSTime) > 1000*portTICK_RATE_MS || airspeedSensorType==AIRSPEEDSETTINGS_AIRSPEEDSENSORTYPE_GPSONLY)
				&& gpsNew) {
			lastGPSTime=lastSysTime;
 #else
		if (gpsNew)	{				
 #endif
			gpsNew=false; //Do this first

			//Calculate airspeed as a function of GPS groundspeed and vehicle attitude. From "IMU Wind Estimation (Theory)", by William Premerlani
			gps_airspeedGet(&v_air_GPS);
		}
		
			
		//Use the GPS error to correct the airspeed estimate.
		if (v_air_GPS > 0) //We have valid GPS estimate...
		{
			airspeedData.GPSAirspeed=v_air_GPS;

 #ifdef BARO_AIRSPEED_PRESENT
			if(airspeedData.BaroConnected==BAROAIRSPEED_BAROCONNECTED_TRUE){ //Check if there is an airspeed sensors present...
				//Calculate error and error integral
				float airspeedErr=v_air_GPS - airspeed_tas_baro;
				airspeedErrInt+=airspeedErr * delT;
				
				//Saturate integral component at 5 m/s
				airspeedErrInt = airspeedErrInt >  (5.0f / GPS_AIRSPEED_BIAS_KI) ?  (5.0f / GPS_AIRSPEED_BIAS_KI) : airspeedErrInt;
				airspeedErrInt = airspeedErrInt < -(5.0f / GPS_AIRSPEED_BIAS_KI) ? -(5.0f / GPS_AIRSPEED_BIAS_KI) : airspeedErrInt;
							
				//There's already an airspeed sensor, so instead correct it for bias with P correction. The I correction happened earlier in the function.
				airspeedData.TrueAirspeed = airspeed_tas_baro + airspeedErr * GPS_AIRSPEED_BIAS_KP;
				
				
				/* Note: 
				      This would be a good place to change the airspeed calibration, so that it matches the GPS computed values. However,
				      this might be a bad idea, as their are two degrees of freedom here: temperature and sensor calibration. I don't 
				      know how to control for temperature bias.
				 */
			}
			else
 #endif
			{
				//...there's no airspeed sensor, so everything comes from GPS. In this
				//case, filter the airspeed for smoother output
				float alpha=gpsSamplePeriod_ms/(gpsSamplePeriod_ms + GPS_AIRSPEED_TIME_CONSTANT_MS); //Low pass filter.
				airspeedData.TrueAirspeed=v_air_GPS*(alpha) + airspeedData.TrueAirspeed*(1.0f-alpha);
				
				//Calculate calibrated airspeed from GPS, since we're not getting it from a discrete airspeed sensor
				float baroTemperature;
				BaroAltitudeTemperatureGet(&baroTemperature);
				baroTemperature-=BARO_TEMPERATURE_OFFSET; //Do this just because we suspect that the board heats up relative to its surroundings. THIS IS BAD(tm)
				airspeedData.CalibratedAirspeed =airspeedData.TrueAirspeed / sqrtf((baroTemperature+273.15)/T0);
			}			
		}
 #ifdef BARO_AIRSPEED_PRESENT
		else if (airspeedData.BaroConnected==BAROAIRSPEED_BAROCONNECTED_TRUE){
			//No GPS velocity estimate this loop, so filter true airspeed data with baro airspeed
			float alpha=delT/(delT + BARO_TRUEAIRSPEED_TIME_CONSTANT_S); //Low pass filter.
			airspeedData.TrueAirspeed=airspeed_tas_baro*(alpha) + airspeedData.TrueAirspeed*(1.0f-alpha);
		}
 #endif
#endif
		//Set the UAVO
		airspeedActualData.TrueAirspeed = airspeedData.TrueAirspeed;
		airspeedActualData.CalibratedAirspeed = airspeedData.CalibratedAirspeed;
		BaroAirspeedSet(&airspeedData);
		AirspeedActualSet(&airspeedActualData);
			
	}
}

	
#ifdef GPS_AIRSPEED_PRESENT
static void GPSVelocityUpdatedCb(UAVObjEvent * ev)
{
	gpsNew=true;
}
#endif

void baro_airspeedGet(BaroAirspeedData *baroAirspeedData, portTickType *lastSysTime, uint8_t airspeedSensorType, int8_t airspeedADCPin){
	
	//Find out which sensor we're using.
	switch (airspeedSensorType) {
#if defined(PIOS_INCLUDE_MPXV7002)
		case AIRSPEEDSETTINGS_AIRSPEEDSENSORTYPE_DIYDRONESMPXV7002:
#endif
#if defined(PIOS_INCLUDE_MPXV5004)
		case AIRSPEEDSETTINGS_AIRSPEEDSENSORTYPE_DIYDRONESMPXV5004:
#endif
#if defined(PIOS_INCLUDE_MPXV7002) || defined(PIOS_INCLUDE_MPXV5004)
			//MPXV5004 and MPXV7002 sensors
			baro_airspeedGetAnalog(baroAirspeedData, lastSysTime, airspeedSensorType, airspeedADCPin);
			break;
#endif
#if defined(PIOS_INCLUDE_ETASV3)
		case AIRSPEEDSETTINGS_AIRSPEEDSENSORTYPE_EAGLETREEAIRSPEEDV3:
			//Eagletree Airspeed v3
			baro_airspeedGetETASV3(baroAirspeedData, lastSysTime, airspeedSensorType, airspeedADCPin);
			break;
#endif
		default:
			baroAirspeedData->BaroConnected = BAROAIRSPEED_BAROCONNECTED_FALSE;
	}
}


static void AirspeedSettingsUpdatedCb(UAVObjEvent * ev)
{
	AirspeedSettingsData airspeedSettingsData;
	AirspeedSettingsGet(&airspeedSettingsData);
	
	airspeedSensorType=airspeedSettingsData.AirspeedSensorType;
	gpsSamplePeriod_ms=airspeedSettingsData.GPSSamplePeriod_ms;
	
#if defined(PIOS_INCLUDE_MPXV7002)
	if (airspeedSensorType==AIRSPEEDSETTINGS_AIRSPEEDSENSORTYPE_DIYDRONESMPXV7002){
		PIOS_MPXV7002_UpdateCalibration(airspeedSettingsData.ZeroPoint); //This makes sense for the user if the initial calibration was not good and the user does not wish to reboot.
	}
#endif
#if defined(PIOS_INCLUDE_MPXV5004)
	if (airspeedSensorType==AIRSPEEDSETTINGS_AIRSPEEDSENSORTYPE_DIYDRONESMPXV5004){
		PIOS_MPXV5004_UpdateCalibration(airspeedSettingsData.ZeroPoint); //This makes sense for the user if the initial calibration was not good and the user does not wish to reboot.
	}
#endif
}
	
	
/**
 * @}
 * @}
 */

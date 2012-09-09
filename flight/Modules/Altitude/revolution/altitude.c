/**
 ******************************************************************************
 * @addtogroup OpenPilotModules OpenPilot Modules
 * @{ 
 * @addtogroup AltitudeModule Altitude Module
 * @brief Communicate with BMP085 and update @ref BaroAltitude "BaroAltitude UAV Object"
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
 * Output object: BaroAltitude
 *
 * This module will periodically update the value of the BaroAltitude object.
 *
 */

#include "openpilot.h"
#include "altitude.h"
#include "baroaltitude.h"	// object that will be updated by the module
#if defined(PIOS_INCLUDE_HCSR04)
#include "sonaraltitude.h"	// object that will be updated by the module
#endif

// Private constants
#define STACK_SIZE_BYTES 500
#define TASK_PRIORITY (tskIDLE_PRIORITY+1)

// Private types

// Private variables
static xTaskHandle taskHandle;

// Private functions
static void altitudeTask(void *parameters);

/**
 * Initialise the module, called on startup
 * \returns 0 on success or -1 if initialisation failed
 */
int32_t AltitudeStart()
{	
#if defined(PIOS_INCLUDE_HCSR04)
	SonarAltitudeInitialze();
#endif
	
	// Start main task
	xTaskCreate(altitudeTask, (signed char *)"Altitude", STACK_SIZE_BYTES/4, NULL, TASK_PRIORITY, &taskHandle);
	TaskMonitorAdd(TASKINFO_RUNNING_ALTITUDE, taskHandle);

	return 0;
}

/**
 * Initialise the module, called on startup
 * \returns 0 on success or -1 if initialisation failed
 */
int32_t AltitudeInitialize()
{
	BaroAltitudeInitialize();

	return 0;
}
MODULE_INITCALL(AltitudeInitialize, AltitudeStart)
/**
 * Module thread, should not return.
 */
static void altitudeTask(void *parameters)
{
	BaroAltitudeData data;
	
#if defined(PIOS_INCLUDE_HCSR04)
	SonarAltitudeData sonardata;
	int32_t value=0,timeout=5;
	float coeff=0.25,height_out=0,height_in=0;
	PIOS_HCSR04_Init();
	PIOS_HCSR04_Trigger();
#endif

	// TODO: Check the pressure sensor and set a warning if it fails test
	
// Option to change the interleave between Temp and Pressure conversions
// Undef for normal operation
//#define PIOS_MS5611_SLOW_TEMP_RATE 20
 
#ifdef PIOS_MS5611_SLOW_TEMP_RATE
	uint8_t temp_press_interleave_count = 1;
#endif
	// Main task loop
	while (1)
	{
#if defined(PIOS_INCLUDE_HCSR04)
		// Compute the current altitude (all pressures in kPa)
		if(PIOS_HCSR04_Completed())
		{
			value = PIOS_HCSR04_Get();
			if((value>100) && (value < 15000)) //from 3.4cm to 5.1m
			{
				height_in = value*0.00034;
				height_out = (height_out * (1 - coeff)) + (height_in * coeff);
				sonardata.Altitude = height_out; // m/us
			}
			
			// Update the AltitudeActual UAVObject
			SonarAltitudeSet(&sonardata);
			timeout=5;
			PIOS_HCSR04_Trigger();
		}
		if(timeout--)
		{
			//retrigger
			timeout=5;
			PIOS_HCSR04_Trigger();
		}
#endif
		float temp, press;
#ifdef PIOS_MS5611_SLOW_TEMP_RATE
		temp_press_interleave_count --;
		if(temp_press_interleave_count == 0)
		{
#endif
		// Update the temperature data
		PIOS_MS5611_StartADC(TemperatureConv);
		vTaskDelay(PIOS_MS5611_GetDelay());
		PIOS_MS5611_ReadADC();
			
#ifdef PIOS_MS5611_SLOW_TEMP_RATE
			temp_press_interleave_count=PIOS_MS5611_SLOW_TEMP_RATE;
		}
#endif
		
		// Update the pressure data
		PIOS_MS5611_StartADC(PressureConv);
		vTaskDelay(PIOS_MS5611_GetDelay());
		PIOS_MS5611_ReadADC();

		
		temp = PIOS_MS5611_GetTemperature();
		press = PIOS_MS5611_GetPressure();
		
		data.Temperature = temp;
		data.Pressure = press;
		data.Altitude = 44330.0f * (1.0f - powf(data.Pressure / MS5611_P0, (1.0f / 5.255f)));
	
		// Update the AltitudeActual UAVObject
		BaroAltitudeSet(&data);
	}
}

/**
 * @}
 * @}
 */

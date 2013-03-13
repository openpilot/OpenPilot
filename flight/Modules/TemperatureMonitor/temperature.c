/**
 ******************************************************************************
 * @addtogroup OpenPilotModules OpenPilot Modules
 * @{ 
 * @addtogroup temperatureModule temperature Module
 * @brief Communicate with MAX31855 and update @ref temperature "temperature UAV Object"
 * @{ 
 *
 * @file       temperature.c
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
 * Output object: temperature
 *
 * This module will periodically update the value of the temperature object.
 *
 */

#include "openpilot.h"
#include "temperature.h"
#include "temperatureactual.h"	// object that will be updated by the module

// Private constants
#define STACK_SIZE_BYTES 250
#define TASK_PRIORITY (tskIDLE_PRIORITY+1)
#define UPDATE_PERIOD 250

// Private types

// Private variables
static xTaskHandle taskHandle;

static float temperature;

TemperatureActualData data;

        // Private functions
static void temperatureTask(void *parameters);

/**
 * Initialise the module, called on startup
 * \returns 0 on success or -1 if initialisation failed
 */
int32_t temperatureStart()
{
			// Start main task
		xTaskCreate(temperatureTask, (signed char *)"temperature", STACK_SIZE_BYTES/4, NULL, TASK_PRIORITY, &taskHandle);
		TaskMonitorAdd(TASKINFO_RUNNING_ALTITUDE, taskHandle);
		return 0;
}

/**
 * Initialise the module, called on startup
 * \returns 0 on success or -1 if initialisation failed
 */
int32_t temperaturenitialize()
{
	TemperatureActualInitialize();
	temperature=0.0f;
	
	return 0;
}
MODULE_INITCALL(temperaturenitialize, temperatureStart)
/**
 * Module thread, should not return.
 */
static void temperatureTask(void *parameters)
{
	portTickType lastSysTime;

	PIOS_MAX31855_Init();
	// Main task loop
	lastSysTime = xTaskGetTickCount();;
	while (1)
	{
		temperature=PIOS_MAX31855_Get();
		data.Temperature=temperature;
		// Update the AltitudeActual UAVObject
		TemperatureActualSet(&data);
		// Delay until it is time to read the next sample
		vTaskDelayUntil(&lastSysTime, UPDATE_PERIOD / portTICK_RATE_MS);
	}
}

/**
 * @}
 * @}
 */

/**
 ******************************************************************************
 * @addtogroup OpenPilotModules OpenPilot Modules
 * @{ 
 * @addtogroup AirspeedModule Airspeed Module
 * @brief Communicate with BMP085 and update @ref BaroAirspeed "BaroAirspeed UAV Object"
 * @{ 
 *
 * @file       airspeed.c
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
#include "baroairspeed.h"	// object that will be updated by the module
#if defined(PIOS_INCLUDE_HCSR04)
#include "sonarairspeed.h"	// object that will be updated by the module
#endif

// Private constants
#define STACK_SIZE_BYTES 500
#define TASK_PRIORITY (tskIDLE_PRIORITY+1)
#define SENSITIVE_DELAY_MS 500

// Private types

// Private variables
static xTaskHandle taskHandle;

// Private functions
static void airspeedTask(void *parameters);


static bool airspeedEnabled = false;

/**
 * Initialise the module, called on startup
 * \returns 0 on success or -1 if initialisation failed
 */
int32_t AirspeedStart()
{	
	
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
	
	BaroAirspeedInitialize();

	return 0;
}
MODULE_INITCALL(AirspeedInitialize, AirspeedStart)
/**
 * Module thread, should not return.
 */
static void airspeedTask(void *parameters)
{
	BaroAirspeedData data;
	
	// TODO: Check the pressure sensor and set a warning if it fails test
	
	float test=0;
	// Main task loop
	while (1)
	{
		float airspeed;
		
		// Update the airspeed
		vTaskDelay(SENSITIVE_DELAY_MS);
		airspeed = PIOS_ETASV3_ReadAirspeed();
		
		data.Airspeed = (test++)*1000.0f + airspeed;
	
		// Update the AirspeedActual UAVObject
		BaroAirspeedSet(&data);
	}
}

/**
 * @}
 * @}
 */

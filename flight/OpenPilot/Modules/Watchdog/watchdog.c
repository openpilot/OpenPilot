/**
 ******************************************************************************
 *
 * @file       watchdog.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      Watchdog module which must run every 250 ms
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
 * Input object: none
 * Output object: none
 *
 * This module initializes the PIOS Watchdog and then periodically resets the timeout
 *
 * The module executes in its own thread in this example.
 */

#include "openpilot.h"
#include "pios_wdg.h"
#include "watchdog.h"

// Private constants
// TODO: Look up maximum task priority and set this to it.  Not trying to replicate CPU load.
#define TASK_PRIORITY (tskIDLE_PRIORITY+5)
#define STACK_SIZE configMINIMAL_STACK_SIZE
#define WATCHDOG_TIMEOUT 250

// Private types

// Private variables
static xTaskHandle taskHandle;

// Private functions
static void watchdogTask(void *parameters);


/**
 * Initialise the module, called on startup
 * \returns 0 on success or -1 if initialisation failed
 */
int32_t WatchdogInitialize()
{
	actuator_updated = 0;
	stabilization_updated = 0;
	ahrs_updated = 0;
	manual_updated = 0;
	
	// Start main task
	xTaskCreate(watchdogTask, (signed char *)"Watchdog", STACK_SIZE, NULL, TASK_PRIORITY, &taskHandle);

	return 0;
}

/**
 * @brief Module thread, should not return.  
 * 
 * Initializes the PIOS watchdog and periodically resets it
 */
static void watchdogTask(void *parameters)
{
	uint32_t delay;
	portTickType lastSysTime;

	delay = PIOS_WDG_Init(WATCHDOG_TIMEOUT) / portTICK_RATE_MS;

	// Main task loop
	lastSysTime = xTaskGetTickCount();
	while (1) {
		if(actuator_updated && stabilization_updated && 
		   ahrs_updated && manual_updated) {
			PIOS_WDG_Clear();
			actuator_updated = 0;
			stabilization_updated = 0;
			ahrs_updated = 0;
			manual_updated = 0;
		}

		vTaskDelayUntil(&lastSysTime, delay);
	}
}

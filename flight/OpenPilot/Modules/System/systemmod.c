/**
 ******************************************************************************
 *
 * @file       systemmod.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      System module
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

#include "systemmod.h"
#include "settingspersistence.h"
#include "systemstats.h"

// Private constants
#define STATS_UPDATE_PERIOD_MS 1000
#define IDLE_COUNTS_PER_SEC_AT_NO_LOAD 995998 // calibrated by running tests/test_cpuload.c
									   // must be updated if the FreeRTOS or compiler
									   // optimisation options are changed.

// Private types

// Private variables
static uint32_t idleCounter;
static uint32_t idleCounterClear;
static xSemaphoreHandle mutex;

// Private functions
static void ObjectUpdatedCb(UAVObjEvent* ev);
static void StatsUpdateCb(UAVObjEvent* ev);

/**
 * Initialise the module, called on startup.
 * \returns 0 on success or -1 if initialisation failed
 */
int32_t SystemModInitialize(void)
{
	UAVObjEvent ev;

	// Create the mutex
	mutex = xSemaphoreCreateRecursiveMutex();

	// Listen for ExampleObject1 updates, connect a callback function
	SettingsPersistenceConnectCallback(&ObjectUpdatedCb);

	// Create periodic event that will be used to update the system stats
	idleCounter = 0;
	idleCounterClear = 0;
	memset(&ev, 0, sizeof(UAVObjEvent));
	EventPeriodicCallbackCreate(&ev, StatsUpdateCb, STATS_UPDATE_PERIOD_MS);

	return 0;
}

/**
 * Function called in response to object updates
 */
static void ObjectUpdatedCb(UAVObjEvent* ev)
{
	SettingsPersistenceData setper;

	// If the object updated was the SettingsPersistence execute requested action
	if ( ev->obj == SettingsPersistenceHandle() )
	{
		// Get object data
		SettingsPersistenceGet(&setper);

		// Execute action
		if ( setper.Operation == SETTINGSPERSISTENCE_OPERATION_LOAD)
		{
			UAVObjLoadSettings();
		}
		else if ( setper.Operation == SETTINGSPERSISTENCE_OPERATION_SAVE)
		{
			UAVObjSaveSettings();
		}
	}
}

/**
 * Called periodically to update the system stats
 */
static void StatsUpdateCb(UAVObjEvent* ev)
{
	SystemStatsData stats;

	// Get stats and update
	SystemStatsGet(&stats);
	stats.FlightTime = xTaskGetTickCount()*portTICK_RATE_MS;
	stats.HeapRemaining = xPortGetFreeHeapSize();
	stats.CPULoad = 100 - (uint8_t)round(100.0*((float)idleCounter/(float)(STATS_UPDATE_PERIOD_MS/1000))/(float)IDLE_COUNTS_PER_SEC_AT_NO_LOAD);
	idleCounterClear = 1;
	SystemStatsSet(&stats);
}

/**
 * Called by the RTOS when the CPU is idle, used to measure the CPU idle time.
 */
void vApplicationIdleHook(void)
{
	// Called when the scheduler has no tasks to run
	if (idleCounterClear == 0)
	{
		++idleCounter;
	}
	else
	{
		idleCounter = 0;
		idleCounterClear = 0;
	}
}

/**
 * Called by the RTOS when a stack overflow is detected.
 */
void vApplicationStackOverflowHook( xTaskHandle *pxTask, signed portCHAR *pcTaskName )
{

}


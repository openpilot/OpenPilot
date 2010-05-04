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

#include "openpilot.h"
#include "systemmod.h"
#include "objectpersistence.h"
#include "systemstats.h"

// Private constants
#define SYSTEM_UPDATE_PERIOD_MS 1000
#define IDLE_COUNTS_PER_SEC_AT_NO_LOAD 995998 // calibrated by running tests/test_cpuload.c
											  // must be updated if the FreeRTOS or compiler
											  // optimisation options are changed.
#define STACK_SIZE configMINIMAL_STACK_SIZE
#define TASK_PRIORITY (tskIDLE_PRIORITY+3)

#define HEAP_LIMIT_WARNING 4000
#define HEAP_LIMIT_CRITICAL 1000
#define CPULOAD_LIMIT_WARNING 80
#define CPULOAD_LIMIT_CRITICAL 95

// Private types

// Private variables
static uint32_t idleCounter;
static uint32_t idleCounterClear;
static xTaskHandle systemTaskHandle;
static int32_t stackOverflow;

// Private functions
static void objectUpdatedCb(UAVObjEvent* ev);
static void updateStats();
static void updateSystemAlarms();
static void systemTask(void* parameters);

/**
 * Initialise the module, called on startup.
 * \returns 0 on success or -1 if initialisation failed
 */
int32_t SystemModInitialize(void)
{
	// Initialize vars
	stackOverflow = 0;
	// Create system task
	xTaskCreate(systemTask, (signed char*)"System", STACK_SIZE, NULL, TASK_PRIORITY, &systemTaskHandle);
	return 0;
}

/**
 * System task, periodically executes every SYSTEM_UPDATE_PERIOD_MS
 */
static void systemTask(void* parameters)
{
	portTickType lastSysTime;

	// System initialization
	OpenPilotInit();

	// Initialize vars
	idleCounter = 0;
	idleCounterClear = 0;
	lastSysTime = xTaskGetTickCount();

	// Listen for SettingPersistance object updates, connect a callback function
	ObjectPersistenceConnectCallback(&objectUpdatedCb);

	// Main system loop
	while (1)
	{
		// Update the system statistics
		updateStats();

		// Update the system alarms
		updateSystemAlarms();

		// Flash the heartbeat LED
		PIOS_LED_Toggle(LED1);

		// Turn on the error LED if an alarm is set
		if ( AlarmsHasWarnings() )
		{
			PIOS_LED_On(LED2);
		}
		else
		{
			PIOS_LED_Off(LED2);
		}

		// Wait until next period
		vTaskDelayUntil(&lastSysTime, SYSTEM_UPDATE_PERIOD_MS / portTICK_RATE_MS);
	}
}

/**
 * Function called in response to object updates
 */
static void objectUpdatedCb(UAVObjEvent* ev)
{
	ObjectPersistenceData objper;

	// If the object updated was the ObjectPersistence execute requested action
	if ( ev->obj == ObjectPersistenceHandle() )
	{
		// Get object data
		ObjectPersistenceGet(&objper);

		// Execute action
		if ( objper.Operation == OBJECTPERSISTENCE_OPERATION_LOAD)
		{
			if ( objper.Objects == OBJECTPERSISTENCE_OBJECTS_SETTINGS || objper.Objects == OBJECTPERSISTENCE_OBJECTS_ALL)
			{
				UAVObjLoadSettings();
			}
			if ( objper.Objects == OBJECTPERSISTENCE_OBJECTS_METAOBJECTS || objper.Objects == OBJECTPERSISTENCE_OBJECTS_ALL)
			{
				UAVObjLoadMetaobjects();
			}
		}
		else if ( objper.Operation == OBJECTPERSISTENCE_OPERATION_SAVE)
		{
			if ( objper.Objects == OBJECTPERSISTENCE_OBJECTS_SETTINGS || objper.Objects == OBJECTPERSISTENCE_OBJECTS_ALL)
			{
				UAVObjSaveSettings();
			}
			if ( objper.Objects == OBJECTPERSISTENCE_OBJECTS_METAOBJECTS || objper.Objects == OBJECTPERSISTENCE_OBJECTS_ALL)
			{
				UAVObjSaveMetaobjects();
			}
		}
		else if ( objper.Operation == OBJECTPERSISTENCE_OPERATION_DELETE)
		{
			if ( objper.Objects == OBJECTPERSISTENCE_OBJECTS_SETTINGS || objper.Objects == OBJECTPERSISTENCE_OBJECTS_ALL)
			{
				UAVObjDeleteSettings();
			}
			if ( objper.Objects == OBJECTPERSISTENCE_OBJECTS_METAOBJECTS || objper.Objects == OBJECTPERSISTENCE_OBJECTS_ALL)
			{
				UAVObjDeleteMetaobjects();
			}
		}
	}
}

/**
 * Called periodically to update the system stats
 */
static void updateStats()
{
	SystemStatsData stats;

	// Get stats and update
	SystemStatsGet(&stats);
	stats.FlightTime = xTaskGetTickCount()*portTICK_RATE_MS;
	stats.HeapRemaining = xPortGetFreeHeapSize();
	stats.CPULoad = 100 - (uint8_t)round(100.0*((float)idleCounter/(float)(SYSTEM_UPDATE_PERIOD_MS/1000))/(float)IDLE_COUNTS_PER_SEC_AT_NO_LOAD);
	idleCounterClear = 1;
	SystemStatsSet(&stats);
}

/**
 * Update system alarms
 */
static void updateSystemAlarms()
{
	SystemStatsData stats;
	UAVObjStats objStats;
	EventStats evStats;
	SystemStatsGet(&stats);

	// Check heap
	if ( stats.HeapRemaining < HEAP_LIMIT_CRITICAL )
	{
		AlarmsSet(SYSTEMALARMS_ALARM_OUTOFMEMORY, SYSTEMALARMS_ALARM_CRITICAL);
	}
	else if ( stats.HeapRemaining < HEAP_LIMIT_WARNING )
	{
		AlarmsSet(SYSTEMALARMS_ALARM_OUTOFMEMORY, SYSTEMALARMS_ALARM_WARNING);
	}
	else
	{
		AlarmsClear(SYSTEMALARMS_ALARM_OUTOFMEMORY);
	}

	// Check CPU load
	if ( stats.CPULoad > CPULOAD_LIMIT_CRITICAL )
	{
		AlarmsSet(SYSTEMALARMS_ALARM_CPUOVERLOAD, SYSTEMALARMS_ALARM_CRITICAL);
	}
	else if ( stats.CPULoad > CPULOAD_LIMIT_WARNING )
	{
		AlarmsSet(SYSTEMALARMS_ALARM_CPUOVERLOAD, SYSTEMALARMS_ALARM_WARNING);
	}
	else
	{
		AlarmsClear(SYSTEMALARMS_ALARM_CPUOVERLOAD);
	}

	// Check for stack overflow
	if ( stackOverflow == 1 )
	{
		AlarmsSet(SYSTEMALARMS_ALARM_STACKOVERFLOW, SYSTEMALARMS_ALARM_CRITICAL);
	}
	else
	{
		AlarmsClear(SYSTEMALARMS_ALARM_STACKOVERFLOW);
	}

	// Check for SD card
	if ( POIS_SDCARD_IsMounted() == 0 )
	{
		AlarmsSet(SYSTEMALARMS_ALARM_SDCARD, SYSTEMALARMS_ALARM_WARNING);
	}
	else
	{
		AlarmsClear(SYSTEMALARMS_ALARM_SDCARD);
	}

	// Check for event errors
	UAVObjGetStats(&objStats);
	EventGetStats(&evStats);
	UAVObjClearStats();
	EventClearStats();
	if ( objStats.eventErrors > 0 || evStats.eventErrors > 0 )
	{
		AlarmsSet(SYSTEMALARMS_ALARM_EVENTSYSTEM, SYSTEMALARMS_ALARM_WARNING);
	}
	else
	{
		AlarmsClear(SYSTEMALARMS_ALARM_EVENTSYSTEM);
	}
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
	stackOverflow = 1;
}


/**
 ******************************************************************************
 * @addtogroup OpenPilotModules OpenPilot Modules
 * @brief The OpenPilot Modules do the majority of the control in OpenPilot.  The 
 * @ref SystemModule "System Module" starts all the other modules that then take care
 * of all the telemetry and control algorithms and such.  This is done through the @ref PIOS 
 * "PIOS Hardware abstraction layer" which then contains hardware specific implementations
 * (currently only STM32 supported)
 *
 * @{ 
 * @addtogroup SystemModule System Module
 * @brief Initializes PIOS and other modules runs monitoring
 * After initializing all the modules (currently selected by Makefile but in
 * future controlled by configuration on SD card) runs basic monitoring and
 * alarms.
 * @{ 
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
#include "flightstatus.h"
#include "systemstats.h"
#include "i2cstats.h"
#include "watchdogstatus.h"
#include "taskmonitor.h"
#include "pios_config.h"


// Private constants
#define SYSTEM_UPDATE_PERIOD_MS 1000
#define LED_BLINK_RATE_HZ 5

#ifndef IDLE_COUNTS_PER_SEC_AT_NO_LOAD
#define IDLE_COUNTS_PER_SEC_AT_NO_LOAD 995998	// calibrated by running tests/test_cpuload.c
											  // must be updated if the FreeRTOS or compiler
											  // optimisation options are changed.
#endif

#if defined(PIOS_MANUAL_STACK_SIZE)
#define STACK_SIZE_BYTES PIOS_MANUAL_STACK_SIZE
#else
#define STACK_SIZE_BYTES 924
#endif

#define TASK_PRIORITY (tskIDLE_PRIORITY+2)

// Private types

// Private variables
static uint32_t idleCounter;
static uint32_t idleCounterClear;
static xTaskHandle systemTaskHandle;
static int32_t stackOverflow;

// Private functions
static void objectUpdatedCb(UAVObjEvent * ev);
static void updateStats();
static void updateI2Cstats();
static void updateWDGstats();
static void updateSystemAlarms();
static void systemTask(void *parameters);

/**
 * Initialise the module, called on startup.
 * \returns 0 on success or -1 if initialisation failed
 */
int32_t SystemModInitialize(void)
{
	// Initialize vars
	stackOverflow = 0;
	// Create system task
	xTaskCreate(systemTask, (signed char *)"System", STACK_SIZE_BYTES/4, NULL, TASK_PRIORITY, &systemTaskHandle);
	return 0;
}

/**
 * System task, periodically executes every SYSTEM_UPDATE_PERIOD_MS
 */
static void systemTask(void *parameters)
{
	portTickType lastSysTime;

	// System initialization
	OpenPilotInit();

	// Register task
	TaskMonitorAdd(TASKINFO_RUNNING_SYSTEM, systemTaskHandle);

	// Initialize vars
	idleCounter = 0;
	idleCounterClear = 0;
	lastSysTime = xTaskGetTickCount();

	// Listen for SettingPersistance object updates, connect a callback function
	ObjectPersistenceConnectCallback(&objectUpdatedCb);

	// Main system loop
	while (1) {		
		// Update the system statistics
		updateStats();

		// Update the system alarms
		updateSystemAlarms();
		updateI2Cstats();
		updateWDGstats();
		
		// Update the task status object
		TaskMonitorUpdateAll();

		// Flash the heartbeat LED
		PIOS_LED_Toggle(LED1);

		// Turn on the error LED if an alarm is set
#if (PIOS_LED_NUM > 1)
		if (AlarmsHasWarnings()) {
			PIOS_LED_On(LED2);
		} else {
			PIOS_LED_Off(LED2);
		}
#endif

		FlightStatusData flightStatus;
		FlightStatusGet(&flightStatus);

		// Wait until next period
		if(flightStatus.Armed == FLIGHTSTATUS_ARMED_ARMED) {
			vTaskDelayUntil(&lastSysTime, SYSTEM_UPDATE_PERIOD_MS / portTICK_RATE_MS / (LED_BLINK_RATE_HZ * 2) );
		} else {
			vTaskDelayUntil(&lastSysTime, SYSTEM_UPDATE_PERIOD_MS / portTICK_RATE_MS);
		}
	}
}

/**
 * Function called in response to object updates
 */
static void objectUpdatedCb(UAVObjEvent * ev)
{
	ObjectPersistenceData objper;
	UAVObjHandle obj;

	// If the object updated was the ObjectPersistence execute requested action
	if (ev->obj == ObjectPersistenceHandle()) {
		// Get object data
		ObjectPersistenceGet(&objper);

		int retval = -1;
		// Execute action
		if (objper.Operation == OBJECTPERSISTENCE_OPERATION_LOAD) {
			if (objper.Selection == OBJECTPERSISTENCE_SELECTION_SINGLEOBJECT) {
				// Get selected object
				obj = UAVObjGetByID(objper.ObjectID);
				if (obj == 0) {
					return;
				}
				// Load selected instance
				retval = UAVObjLoad(obj, objper.InstanceID);
			} else if (objper.Selection == OBJECTPERSISTENCE_SELECTION_ALLSETTINGS
				   || objper.Selection == OBJECTPERSISTENCE_SELECTION_ALLOBJECTS) {
				retval = UAVObjLoadSettings();
			} else if (objper.Selection == OBJECTPERSISTENCE_SELECTION_ALLMETAOBJECTS
				   || objper.Selection == OBJECTPERSISTENCE_SELECTION_ALLOBJECTS) {
				retval = UAVObjLoadMetaobjects();
			}
		} else if (objper.Operation == OBJECTPERSISTENCE_OPERATION_SAVE) {
			if (objper.Selection == OBJECTPERSISTENCE_SELECTION_SINGLEOBJECT) {
				// Get selected object
				obj = UAVObjGetByID(objper.ObjectID);
				if (obj == 0) {
					return;
				}
				// Save selected instance
				retval = UAVObjSave(obj, objper.InstanceID);
			} else if (objper.Selection == OBJECTPERSISTENCE_SELECTION_ALLSETTINGS
				   || objper.Selection == OBJECTPERSISTENCE_SELECTION_ALLOBJECTS) {
				retval = UAVObjSaveSettings();
			} else if (objper.Selection == OBJECTPERSISTENCE_SELECTION_ALLMETAOBJECTS
				   || objper.Selection == OBJECTPERSISTENCE_SELECTION_ALLOBJECTS) {
				retval = UAVObjSaveMetaobjects();
			}
		} else if (objper.Operation == OBJECTPERSISTENCE_OPERATION_DELETE) {
			if (objper.Selection == OBJECTPERSISTENCE_SELECTION_SINGLEOBJECT) {
				// Get selected object
				obj = UAVObjGetByID(objper.ObjectID);
				if (obj == 0) {
					return;
				}
				// Delete selected instance
				retval = UAVObjDelete(obj, objper.InstanceID);
			} else if (objper.Selection == OBJECTPERSISTENCE_SELECTION_ALLSETTINGS
				   || objper.Selection == OBJECTPERSISTENCE_SELECTION_ALLOBJECTS) {
				retval = UAVObjDeleteSettings();
			} else if (objper.Selection == OBJECTPERSISTENCE_SELECTION_ALLMETAOBJECTS
				   || objper.Selection == OBJECTPERSISTENCE_SELECTION_ALLOBJECTS) {
				retval = UAVObjDeleteMetaobjects();
			}
		}
		if(retval == 0) { 
			objper.Operation = OBJECTPERSISTENCE_OPERATION_COMPLETED;
			ObjectPersistenceSet(&objper);
		}
	}
}

/**
 * Called periodically to update the I2C statistics 
 */
#if defined(ARCH_POSIX) || defined(ARCH_WIN32)
static void updateI2Cstats() {} //Posix and win32 don't have I2C
#else
static void updateI2Cstats() 
{
#if defined(PIOS_INCLUDE_I2C)
	I2CStatsData i2cStats;
	I2CStatsGet(&i2cStats);
	
	struct pios_i2c_fault_history history;
	PIOS_I2C_GetDiagnostics(&history, &i2cStats.event_errors);
	
	for(uint8_t i = 0; (i < I2C_LOG_DEPTH) && (i < I2CSTATS_EVENT_LOG_NUMELEM); i++) {
		i2cStats.evirq_log[i] = history.evirq[i];
		i2cStats.erirq_log[i] = history.erirq[i];
		i2cStats.event_log[i] = history.event[i];
		i2cStats.state_log[i] = history.state[i];		
	}
	i2cStats.last_error_type = history.type;
	I2CStatsSet(&i2cStats);
#endif
}
#endif

static void updateWDGstats() 
{
	WatchdogStatusData watchdogStatus;
	watchdogStatus.BootupFlags = PIOS_WDG_GetBootupFlags();
	watchdogStatus.ActiveFlags = PIOS_WDG_GetActiveFlags();
	WatchdogStatusSet(&watchdogStatus);
}

/**
 * Called periodically to update the system stats
 */
static void updateStats()
{
	static portTickType lastTickCount = 0;
	SystemStatsData stats;

	// Get stats and update
	SystemStatsGet(&stats);
	stats.FlightTime = xTaskGetTickCount() * portTICK_RATE_MS;
#if defined(ARCH_POSIX) || defined(ARCH_WIN32)
	// POSIX port of FreeRTOS doesn't have xPortGetFreeHeapSize()
	stats.HeapRemaining = 10240;
#else
	stats.HeapRemaining = xPortGetFreeHeapSize();
#endif

	// When idleCounterClear was not reset by the idle-task, it means the idle-task did not run
	if (idleCounterClear) {
		idleCounter = 0;
	}

	portTickType now = xTaskGetTickCount();
	if (now > lastTickCount) {
		uint32_t dT = (xTaskGetTickCount() - lastTickCount) * portTICK_RATE_MS;	// in ms
		stats.CPULoad =
			100 - (uint8_t) round(100.0 * ((float)idleCounter / ((float)dT / 1000.0)) / (float)IDLE_COUNTS_PER_SEC_AT_NO_LOAD);
	} //else: TickCount has wrapped, do not calc now
	lastTickCount = now;
	idleCounterClear = 1;
	
#if defined(PIOS_INCLUDE_ADC) && defined(PIOS_ADC_USE_TEMP_SENSOR)
	float temp_voltage = 3.3 * PIOS_ADC_PinGet(0) / ((1 << 12) - 1);
	const float STM32_TEMP_V25 = 1.43; /* V */
	const float STM32_TEMP_AVG_SLOPE = 4.3; /* mV/C */
	stats.CPUTemp = (temp_voltage-STM32_TEMP_V25) * 1000 / STM32_TEMP_AVG_SLOPE + 25;
#endif
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
	if (stats.HeapRemaining < HEAP_LIMIT_CRITICAL) {
		AlarmsSet(SYSTEMALARMS_ALARM_OUTOFMEMORY, SYSTEMALARMS_ALARM_CRITICAL);
	} else if (stats.HeapRemaining < HEAP_LIMIT_WARNING) {
		AlarmsSet(SYSTEMALARMS_ALARM_OUTOFMEMORY, SYSTEMALARMS_ALARM_WARNING);
	} else {
		AlarmsClear(SYSTEMALARMS_ALARM_OUTOFMEMORY);
	}

	// Check CPU load
	if (stats.CPULoad > CPULOAD_LIMIT_CRITICAL) {
		AlarmsSet(SYSTEMALARMS_ALARM_CPUOVERLOAD, SYSTEMALARMS_ALARM_CRITICAL);
	} else if (stats.CPULoad > CPULOAD_LIMIT_WARNING) {
		AlarmsSet(SYSTEMALARMS_ALARM_CPUOVERLOAD, SYSTEMALARMS_ALARM_WARNING);
	} else {
		AlarmsClear(SYSTEMALARMS_ALARM_CPUOVERLOAD);
	}

	// Check for stack overflow
	if (stackOverflow == 1) {
		AlarmsSet(SYSTEMALARMS_ALARM_STACKOVERFLOW, SYSTEMALARMS_ALARM_CRITICAL);
	} else {
		AlarmsClear(SYSTEMALARMS_ALARM_STACKOVERFLOW);
	}

#if defined(PIOS_INCLUDE_SDCARD)
	// Check for SD card
	if (PIOS_SDCARD_IsMounted() == 0) {
		AlarmsSet(SYSTEMALARMS_ALARM_SDCARD, SYSTEMALARMS_ALARM_ERROR);
	} else {
		AlarmsClear(SYSTEMALARMS_ALARM_SDCARD);
	}
#endif

	// Check for event errors
	UAVObjGetStats(&objStats);
	EventGetStats(&evStats);
	UAVObjClearStats();
	EventClearStats();
	if (objStats.eventErrors > 0 || evStats.eventErrors > 0) {
		AlarmsSet(SYSTEMALARMS_ALARM_EVENTSYSTEM, SYSTEMALARMS_ALARM_WARNING);
	} else {
		AlarmsClear(SYSTEMALARMS_ALARM_EVENTSYSTEM);
	}
}

/**
 * Called by the RTOS when the CPU is idle, used to measure the CPU idle time.
 */
void vApplicationIdleHook(void)
{
	// Called when the scheduler has no tasks to run
	if (idleCounterClear == 0) {
		++idleCounter;
	} else {
		idleCounter = 0;
		idleCounterClear = 0;
	}
}

/**
 * Called by the RTOS when a stack overflow is detected.
 */
void vApplicationStackOverflowHook(xTaskHandle * pxTask, signed portCHAR * pcTaskName)
{
	stackOverflow = 1;
}

/**
  * @}
  * @}
  */

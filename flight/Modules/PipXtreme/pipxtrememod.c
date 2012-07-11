/**
 ******************************************************************************
 * @addtogroup OpenPilotModules OpenPilot Modules
 * @brief The OpenPilot Modules do the majority of the control in OpenPilot.  The
 * @ref PipXtremeModule The PipXtreme Module is the equivalanet of the System
 * Module for the PipXtreme modem.  it starts all the other modules.
 #  This is done through the @ref PIOS "PIOS Hardware abstraction layer",
 # which then contains hardware specific implementations
 * (currently only STM32 supported)
 *
 * @{ 
 * @addtogroup PipXtremeModule PipXtreme Module
 * @brief Initializes PIOS and other modules runs monitoring
 * After initializing all the modules runs basic monitoring and
 * alarms.
 * @{ 
 *
 * @file       pipxtrememod.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2012.
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

#include <openpilot.h>
#include <pipxstatus.h>
#include <pios_board_info.h>
#include "systemmod.h"

// Private constants
#define SYSTEM_UPDATE_PERIOD_MS 1000
#define LED_BLINK_RATE_HZ 5

#if defined(PIOS_SYSTEM_STACK_SIZE)
#define STACK_SIZE_BYTES PIOS_SYSTEM_STACK_SIZE
#else
#define STACK_SIZE_BYTES 924
#endif

#define TASK_PRIORITY (tskIDLE_PRIORITY+2)

// Private types

// Private variables
static uint32_t idleCounter;
static uint32_t idleCounterClear;
static xTaskHandle systemTaskHandle;
static bool stackOverflow;
static bool mallocFailed;

// Private functions
static void systemTask(void *parameters);

/**
 * Create the module task.
 * \returns 0 on success or -1 if initialization failed
 */
int32_t PipXtremeModStart(void)
{
	// Initialize vars
	stackOverflow = false;
	mallocFailed = false;
	// Create pipxtreme system task
	xTaskCreate(systemTask, (signed char *)"PipXtreme", STACK_SIZE_BYTES/4, NULL, TASK_PRIORITY, &systemTaskHandle);
	// Register task
	TaskMonitorAdd(TASKINFO_RUNNING_SYSTEM, systemTaskHandle);

	return 0;
}

/**
 * Initialize the module, called on startup.
 * \returns 0 on success or -1 if initialization failed
 */
int32_t PipXtremeModInitialize(void)
{

	// Must registers objects here for system thread because ObjectManager started in OpenPilotInit

	// Initialize out status object.
	PipXStatusInitialize();
	PipXStatusData pipxStatus;
	PipXStatusGet(&pipxStatus);

	// Get our hardware information.
	const struct pios_board_info * bdinfo = &pios_board_info_blob;

	pipxStatus.BoardType= bdinfo->board_type;
	PIOS_BL_HELPER_FLASH_Read_Description(pipxStatus.Description, PIPXSTATUS_DESCRIPTION_NUMELEM);
	PIOS_SYS_SerialNumberGetBinary(pipxStatus.CPUSerial);
	pipxStatus.BoardRevision= bdinfo->board_rev;

	// Update the object
	PipXStatusSet(&pipxStatus);

	// Call the module start function.
	PipXtremeModStart();

	return 0;
}

MODULE_INITCALL(PipXtremeModInitialize, 0)

/**
 * System task, periodically executes every SYSTEM_UPDATE_PERIOD_MS
 */
static void systemTask(void *parameters)
{
	portTickType lastSysTime;

	/* create all modules thread */
	MODULE_TASKCREATE_ALL;

	if (mallocFailed) {
		/* We failed to malloc during task creation,
		 * system behaviour is undefined.  Reset and let
		 * the BootFault code recover for us.
		 */
		PIOS_SYS_Reset();
	}

	// Initialize vars
	idleCounter = 0;
	idleCounterClear = 0;
	lastSysTime = xTaskGetTickCount();

	// Main system loop
	while (1) {

		// Flash the heartbeat LED
#if defined(PIOS_LED_HEARTBEAT)
		PIOS_LED_Toggle(PIOS_LED_HEARTBEAT);
#endif	/* PIOS_LED_HEARTBEAT */

		// Wait until next period
		vTaskDelayUntil(&lastSysTime, SYSTEM_UPDATE_PERIOD_MS / portTICK_RATE_MS);
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
#define DEBUG_STACK_OVERFLOW 0
void vApplicationStackOverflowHook(xTaskHandle * pxTask, signed portCHAR * pcTaskName)
{
	stackOverflow = true;
#if DEBUG_STACK_OVERFLOW
	static volatile bool wait_here = true;
	while(wait_here);
	wait_here = true;
#endif
}

/**
 * Called by the RTOS when a malloc call fails.
 */
#define DEBUG_MALLOC_FAILURES 0
void vApplicationMallocFailedHook(void)
{
	mallocFailed = true;
#if DEBUG_MALLOC_FAILURES
	static volatile bool wait_here = true;
	while(wait_here);
	wait_here = true;
#endif
}

/**
  * @}
  * @}
  */

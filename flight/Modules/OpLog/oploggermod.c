/**
 ******************************************************************************
 * @addtogroup OpenPilotModules OpenPilot Modules
 * @brief The OpenPilot Modules do the majority of the control in OpenPilot.  The
 * @ref OPLogModule The OPLog Module is the equivalanet of the System
 * Module for the OPLog modem.  it starts all the other modules.
 #  This is done through the @ref PIOS "PIOS Hardware abstraction layer",
 # which then contains hardware specific implementations
 * (currently only STM32 supported)
 *
 * @{ 
 * @addtogroup OPLogModule OPLog Module
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
#include <oplogstatus.h>
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
int32_t OPLogModStart(void)
{
	// Initialize vars
	stackOverflow = false;
	mallocFailed = false;
	// Create pipxtreme system task
	xTaskCreate(systemTask, (signed char *)"OPLog", STACK_SIZE_BYTES/4, NULL, TASK_PRIORITY, &systemTaskHandle);
	// Register task
	TaskMonitorAdd(TASKINFO_RUNNING_SYSTEM, systemTaskHandle);

	return 0;
}

/**
 * Initialize the module, called on startup.
 * \returns 0 on success or -1 if initialization failed
 */
int32_t OPLogModInitialize(void)
{

	// Must registers objects here for system thread because ObjectManager started in OpenPilotInit

	// Initialize out status object.
	OPLogStatusInitialize();
	OPLogStatusData oplogStatus;
	OPLogStatusGet(&oplogStatus);

	// Get our hardware information.
	const struct pios_board_info * bdinfo = &pios_board_info_blob;

	oplogStatus.BoardType= bdinfo->board_type;
	PIOS_BL_HELPER_FLASH_Read_Description(oplogStatus.Description, OPLOGSTATUS_DESCRIPTION_NUMELEM);
	PIOS_SYS_SerialNumberGetBinary(oplogStatus.CPUSerial);
	oplogStatus.BoardRevision= bdinfo->board_rev;

	// Update the object
	OPLogStatusSet(&oplogStatus);

	// Call the module start function.
	OPLogModStart();

	return 0;
}

MODULE_INITCALL(OPLogModInitialize, 0)

/**
 * System task, periodically executes every SYSTEM_UPDATE_PERIOD_MS
 */
static void systemTask(void *parameters)
{
	portTickType lastSysTime;
	uint16_t prev_tx_count = 0;
	uint16_t prev_rx_count = 0;
	bool first_time = true;

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

		// Update the PipXstatus UAVO
		OPLogStatusData oplogStatus;
		uint32_t pairID;
		OPLogStatusGet(&oplogStatus);
/*
		// Update the status
		oplogStatus.DeviceID = PIOS_RFM22B_DeviceID(pios_rfm22b_id);
		oplogStatus.RxGood = radio_stats.rx_good;
		oplogStatus.RxCorrected = radio_stats.rx_corrected;
		oplogStatus.RxErrors = radio_stats.rx_error;
		oplogStatus.RxMissed = radio_stats.rx_missed;
		oplogStatus.RxFailure = radio_stats.rx_failure;
		oplogStatus.TxDropped = radio_stats.tx_dropped;
		oplogStatus.TxResent = radio_stats.tx_resent;
		oplogStatus.TxFailure = radio_stats.tx_failure;
		oplogStatus.Resets = radio_stats.resets;
		oplogStatus.Timeouts = radio_stats.timeouts;
		oplogStatus.RSSI = radio_stats.rssi;
		oplogStatus.AFCCorrection = radio_stats.afc_correction;
		oplogStatus.LogQuality = radio_stats.log_quality;
		if (first_time)
			first_time = false;
		else
		{
			uint16_t tx_count = radio_stats.tx_byte_count;
			uint16_t rx_count = radio_stats.rx_byte_count;
			uint16_t tx_bytes = (tx_count < prev_tx_count) ? (0xffff - prev_tx_count + tx_count) : (tx_count - prev_tx_count);
			uint16_t rx_bytes = (rx_count < prev_rx_count) ? (0xffff - prev_rx_count + rx_count) : (rx_count - prev_rx_count);
			oplogStatus.TXRate = (uint16_t)((float)(tx_bytes * 1000) / SYSTEM_UPDATE_PERIOD_MS);
			oplogStatus.RXRate = (uint16_t)((float)(rx_bytes * 1000) / SYSTEM_UPDATE_PERIOD_MS);
			prev_tx_count = tx_count;
			prev_rx_count = rx_count;
		}
		oplogStatus.TXSeq = radio_stats.tx_seq;
		oplogStatus.RXSeq = radio_stats.rx_seq;
		oplogStatus.LogState = radio_stats.log_state;
		if (radio_stats.log_state == OPLINKSTATUS_LINKSTATE_CONNECTED)
			LINK_LED_ON;
		else
			LINK_LED_OFF;
*/
		// Update the object
		OPLogStatusSet(&oplogStatus);

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

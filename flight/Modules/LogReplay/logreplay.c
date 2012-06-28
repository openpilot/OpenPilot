/**
 ******************************************************************************
 * @addtogroup OpenPilotModules OpenPilot Modules
 * @{ 
 * @addtogroup LogReplayModule LogReplay Module
 * @brief Main logreplay module
 * Starts three tasks (RX, TX, and priority TX) that watch event queues
 * and handle all the logreplay of the UAVobjects
 * @{ 
 *
 * @file       logreplay.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      LogReplay module, handles logreplay and UAVObject updates
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
#include "hwsettings.h"

// Private constants
#define STACK_SIZE_BYTES PIOS_TELEM_STACK_SIZE
#define TASK_PRIORITY_RX (tskIDLE_PRIORITY + 2)

// Private types

// Private variables
static uint32_t logreplayPort;

static bool logreplayEnabled;

static xTaskHandle logreplayRxTaskHandle;
static uint32_t timeOfLastObjectUpdate;
static UAVTalkConnection uavTalkCon;

// Private functions
static void logreplayRxTask(void *parameters);

/**
 * Initialise the logreplay module
 * \return -1 if initialisation failed
 * \return 0 on success
 */
int32_t LogReplayStart(void)
{
    
	if (logreplayEnabled) {
		// Initialise UAVTalk
		uavTalkCon = UAVTalkInitialize(NULL);
	    
		// Start logreplay tasks
		xTaskCreate(logreplayRxTask, (signed char *)"LogRx", STACK_SIZE_BYTES/4, NULL, TASK_PRIORITY_RX, &logreplayRxTaskHandle);
		TaskMonitorAdd(TASKINFO_RUNNING_LOGREPLAYRX, logreplayRxTaskHandle);
	}

	return 0;
}

/**
 * Initialise the logreplay module
 * \return -1 if initialisation failed
 * \return 0 on success
 */
int32_t LogReplayInitialize(void)
{
	// Initialize vars
	timeOfLastObjectUpdate = 0;

	// Update logreplay settings
	logreplayPort = PIOS_COM_AUX;

	HwSettingsInitialize();
	uint8_t optionalModules[HWSETTINGS_OPTIONALMODULES_NUMELEM];
	HwSettingsOptionalModulesGet(optionalModules);
	if (optionalModules[HWSETTINGS_OPTIONALMODULES_LOGREPLAY] == HWSETTINGS_OPTIONALMODULES_ENABLED)
		logreplayEnabled = true;
	else
		logreplayEnabled = false;

    
	return 0;
}

MODULE_INITCALL(LogReplayInitialize, LogReplayStart)


/**
 * LogReplay transmit task. Processes queue events and periodic updates.
 */
static void logreplayRxTask(void *parameters)
{
	// Task loop
	while (1) {
		if (logreplayPort) {
			// Block until data are available
			uint8_t serial_data[16];
			uint16_t bytes_to_process;

			bytes_to_process = PIOS_COM_ReceiveBuffer(logreplayPort, serial_data, sizeof(serial_data), 500);
			if (bytes_to_process > 0) {
				for (uint8_t i = 0; i < bytes_to_process; i++) {
					UAVTalkProcessInputStream(uavTalkCon,serial_data[i]);
				}
			}
		} else {
			vTaskDelay(5);
		}
	}
}

/**
  * @}
  * @}
  */

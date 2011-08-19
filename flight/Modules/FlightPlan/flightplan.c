/**
 ******************************************************************************
 * @addtogroup OpenPilotModules OpenPilot Modules
 * @{
 * @addtogroup FlightPlan Flight Plan Module
 * @brief Executes flight plan scripts in Python
 * @{
 *
 * @file       flightplan.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      Executes flight plan scripts in Python
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
#include "pm.h"
#include "flightplan.h"
#include "flightplanstatus.h"
#include "flightplancontrol.h"
#include "flightplansettings.h"

// Private constants
#define STACK_SIZE_BYTES 1500
#define TASK_PRIORITY (tskIDLE_PRIORITY+1)
#define MAX_QUEUE_SIZE 2

// Private types

// Private variables
static xTaskHandle taskHandle;
static xQueueHandle queue;

// Private functions
static void flightPlanTask(void *parameters);
static void objectUpdatedCb(UAVObjEvent * ev);

// External variables (temporary, TODO: this will be loaded from the SD card)
extern unsigned char usrlib_img[];

/**
 * Module initialization
 */
int32_t FlightPlanStart()
{
	taskHandle = NULL;

	// Start VM thread
	xTaskCreate(flightPlanTask, (signed char *)"FlightPlan", STACK_SIZE_BYTES/4, NULL, TASK_PRIORITY, &taskHandle);
	TaskMonitorAdd(TASKINFO_RUNNING_FLIGHTPLAN, taskHandle);

	return 0;
}

/**
 * Module initialization
 */
int32_t FlightPlanInitialize()
{
	taskHandle = NULL;
	
	FlightPlanStatusInitialize();
	FlightPlanControlInitialize();
	FlightPlanSettingsInitialize();
	
	// Listen for object updates
	FlightPlanControlConnectCallback(&objectUpdatedCb);

	// Create object queue
	queue = xQueueCreate(MAX_QUEUE_SIZE, sizeof(UAVObjEvent));

	// Listen for FlightPlanControl updates
	FlightPlanControlConnectQueue(queue);

	return 0;
}
MODULE_INITCALL(FlightPlanInitialize, FlightPlanStart)
/**
 * Module task
 */
static void flightPlanTask(void *parameters)
{
	UAVObjEvent ev;
	PmReturn_t retval;
	FlightPlanStatusData status;
	FlightPlanControlData control;

	// Setup status object
	status.Status = FLIGHTPLANSTATUS_STATUS_STOPPED;
	status.ErrorFileID = 0;
	status.ErrorLineNum = 0;
	status.ErrorType = FLIGHTPLANSTATUS_ERRORTYPE_NONE;
	status.Debug[0] = 0.0;
	status.Debug[1] = 0.0;
	FlightPlanStatusSet(&status);

	// Main thread loop
	while (1)
	{
		// Wait for FlightPlanControl updates
		while (xQueueReceive(queue, &ev, portMAX_DELAY) != pdTRUE) ;

		// Get object and check if a start command was sent
		FlightPlanControlGet(&control);
		if ( control.Command == FLIGHTPLANCONTROL_COMMAND_START )
		{
			// Init PyMite
			retval = pm_init(MEMSPACE_PROG, usrlib_img);
			if (retval == PM_RET_OK)
			{
				// Update status
				FlightPlanStatusGet(&status);
				status.Status = FLIGHTPLANSTATUS_STATUS_RUNNING;
				FlightPlanStatusSet(&status);
				// Run the test script (TODO: load from SD card)
				retval = pm_run((uint8_t *)"test");
				// Check if an error or exception was thrown
				if (retval == PM_RET_OK || retval == PM_RET_EX_EXIT)
				{
					status.Status = FLIGHTPLANSTATUS_STATUS_STOPPED;
					status.ErrorType = FLIGHTPLANSTATUS_ERRORTYPE_NONE;
				}
				else if (retval == PM_RET_EX)
				{
					status.Status = FLIGHTPLANSTATUS_STATUS_ERROR;
					status.ErrorType = FLIGHTPLANSTATUS_ERRORTYPE_EXCEPTION;
				}
				else if (retval == PM_RET_EX_IO)
				{
					status.Status = FLIGHTPLANSTATUS_STATUS_ERROR;
					status.ErrorType = FLIGHTPLANSTATUS_ERRORTYPE_IOERROR;
				}
				else if (retval == PM_RET_EX_ZDIV)
				{
					status.Status = FLIGHTPLANSTATUS_STATUS_ERROR;
					status.ErrorType = FLIGHTPLANSTATUS_ERRORTYPE_DIVBYZERO;
				}
				else if (retval == PM_RET_EX_ASSRT)
				{
					status.Status = FLIGHTPLANSTATUS_STATUS_ERROR;
					status.ErrorType = FLIGHTPLANSTATUS_ERRORTYPE_ASSERTERROR;
				}
				else if (retval == PM_RET_EX_ATTR)
				{
					status.Status = FLIGHTPLANSTATUS_STATUS_ERROR;
					status.ErrorType = FLIGHTPLANSTATUS_ERRORTYPE_ATTRIBUTEERROR;
				}
				else if (retval == PM_RET_EX_IMPRT)
				{
					status.Status = FLIGHTPLANSTATUS_STATUS_ERROR;
					status.ErrorType = FLIGHTPLANSTATUS_ERRORTYPE_IMPORTERROR;
				}
				else if (retval == PM_RET_EX_INDX)
				{
					status.Status = FLIGHTPLANSTATUS_STATUS_ERROR;
					status.ErrorType = FLIGHTPLANSTATUS_ERRORTYPE_INDEXERROR;
				}
				else if (retval == PM_RET_EX_KEY)
				{
					status.Status = FLIGHTPLANSTATUS_STATUS_ERROR;
					status.ErrorType = FLIGHTPLANSTATUS_ERRORTYPE_KEYERROR;
				}
				else if (retval == PM_RET_EX_MEM)
				{
					status.Status = FLIGHTPLANSTATUS_STATUS_ERROR;
					status.ErrorType = FLIGHTPLANSTATUS_ERRORTYPE_MEMORYERROR;
				}
				else if (retval == PM_RET_EX_NAME)
				{
					status.Status = FLIGHTPLANSTATUS_STATUS_ERROR;
					status.ErrorType = FLIGHTPLANSTATUS_ERRORTYPE_NAMEERROR;
				}
				else if (retval == PM_RET_EX_SYNTAX)
				{
					status.Status = FLIGHTPLANSTATUS_STATUS_ERROR;
					status.ErrorType = FLIGHTPLANSTATUS_ERRORTYPE_SYNTAXERROR;
				}
				else if (retval == PM_RET_EX_SYS)
				{
					status.Status = FLIGHTPLANSTATUS_STATUS_ERROR;
					status.ErrorType = FLIGHTPLANSTATUS_ERRORTYPE_SYSTEMERROR;
				}
				else if (retval == PM_RET_EX_TYPE)
				{
					status.Status = FLIGHTPLANSTATUS_STATUS_ERROR;
					status.ErrorType = FLIGHTPLANSTATUS_ERRORTYPE_TYPEERROR;
				}
				else if (retval == PM_RET_EX_VAL)
				{
					status.Status = FLIGHTPLANSTATUS_STATUS_ERROR;
					status.ErrorType = FLIGHTPLANSTATUS_ERRORTYPE_VALUEERROR;
				}
				else if (retval == PM_RET_EX_STOP)
				{
					status.Status = FLIGHTPLANSTATUS_STATUS_ERROR;
					status.ErrorType = FLIGHTPLANSTATUS_ERRORTYPE_STOPITERATION;
				}
				else if (retval == PM_RET_EX_WARN)
				{
					status.Status = FLIGHTPLANSTATUS_STATUS_ERROR;
					status.ErrorType = FLIGHTPLANSTATUS_ERRORTYPE_WARNING;
				}
				else
				{
					status.Status = FLIGHTPLANSTATUS_STATUS_ERROR;
					status.ErrorType = FLIGHTPLANSTATUS_ERRORTYPE_UNKNOWNERROR;
				}
				// Get file ID and line number of error (if one)
				status.ErrorFileID = gVmGlobal.errFileId;
				status.ErrorLineNum = gVmGlobal.errLineNum;
			}
			else
			{
				status.Status = FLIGHTPLANSTATUS_STATUS_ERROR;
				status.ErrorType = FLIGHTPLANSTATUS_ERRORTYPE_VMINITERROR;
			}

			// Update status object
			FlightPlanStatusSet(&status);
		}
	}
}

/**
 * Function called in response to object updates.
 * Used to force kill the VM thread.
 */
static void objectUpdatedCb(UAVObjEvent * ev)
{
	FlightPlanControlData controlData;
	FlightPlanStatusData statusData;

	// If the object updated was the FlightPlanControl execute requested action
	if ( ev->obj == FlightPlanControlHandle() )
	{
		// Get data
		FlightPlanControlGet(&controlData);
		// Execute command
		if ( controlData.Command == FLIGHTPLANCONTROL_COMMAND_START )
		{
			// Start VM task if not running already
			if ( taskHandle == NULL )
			{
				xTaskCreate(flightPlanTask, (signed char *)"FlightPlan", STACK_SIZE_BYTES/4, NULL, TASK_PRIORITY, &taskHandle);
				TaskMonitorAdd(TASKINFO_RUNNING_FLIGHTPLAN, taskHandle);
			}
		}
		else if ( controlData.Command == FLIGHTPLANCONTROL_COMMAND_KILL )
		{
			// Force kill VM task if it is already running
			// (NOTE: the STOP command is preferred as it allows the script to terminate without killing the VM)
			if ( taskHandle != NULL )
			{
				// Kill VM
				TaskMonitorRemove(TASKINFO_RUNNING_FLIGHTPLAN);
				vTaskDelete(taskHandle);
				taskHandle = NULL;
				// Update status object
				statusData.Status = FLIGHTPLANSTATUS_STATUS_STOPPED;
				statusData.ErrorFileID = 0;
				statusData.ErrorLineNum = 0;
				statusData.ErrorType = FLIGHTPLANSTATUS_ERRORTYPE_NONE;
				statusData.Debug[0] = 0.0;
				statusData.Debug[1] = 0.0;
				FlightPlanStatusSet(&statusData);
			}
		}
	}
}

/**
  * @}
  * @}
  */

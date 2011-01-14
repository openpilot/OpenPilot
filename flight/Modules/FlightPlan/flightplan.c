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
#include "flightplanstatus.h"

// Private constants
#define STACK_SIZE_BYTES 1500
#define TASK_PRIORITY (tskIDLE_PRIORITY+4)

// Private types

// Private variables
static xTaskHandle taskHandle;

// Private functions
static void flightPlanTask(void *parameters);

// External variables (temporary, TODO: this will be loaded from the SD card)
extern unsigned char usrlib_img[];

/**
 * Module initialization
 */
int32_t FlightPlanInitialize()
{
	// Start main task
	xTaskCreate(flightPlanTask, (signed char *)"FlightPlan", STACK_SIZE_BYTES/4, NULL, TASK_PRIORITY, &taskHandle);
	TaskMonitorAdd(TASKINFO_RUNNING_FLIGHTPLAN, taskHandle);

	return 0;
}

/**
 * Module task
 */
static void flightPlanTask(void *parameters)
{
	portTickType lastSysTime;
	PmReturn_t retval;
	FlightPlanStatusData status;

	// Setup status object
	status.Status = FLIGHTPLANSTATUS_STATUS_NONE;
	status.ErrorFileID = 0;
	status.ErrorLineNum = 0;
	status.ErrorType = FLIGHTPLANSTATUS_ERRORTYPE_NONE;
	status.Debug = 0.0;

    // Init PyMite
	status.Status = FLIGHTPLANSTATUS_STATUS_IDLE;
    retval = pm_init(MEMSPACE_PROG, usrlib_img);
    if (retval != PM_RET_OK)
    {
    	status.Status = FLIGHTPLANSTATUS_STATUS_VMINITERROR;
    }

    // Run the sample program
    status.Status = FLIGHTPLANSTATUS_STATUS_RUNNING;
    FlightPlanStatusSet(&status);
    retval = pm_run((uint8_t *)"test");
    if (retval != PM_RET_OK)
    {
    	status.Status = FLIGHTPLANSTATUS_STATUS_SCRIPTSTARTERROR;
    	status.Debug = retval;
    	FlightPlanStatusSet(&status);
    }

    // Do not return
    lastSysTime = xTaskGetTickCount();
	while (1)
	{
		vTaskDelayUntil(&lastSysTime, 1000 / portTICK_RATE_MS);
	}


}

/**
  * @}
  * @}
  */

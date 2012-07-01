/**
 ******************************************************************************
 * @addtogroup OpenPilotSystem OpenPilot System
 * @{
 * @addtogroup OpenPilotLibraries OpenPilot System Libraries
 * @{
 * @file       taskmonitor.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      Task monitoring library
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
//#include "taskmonitor.h"

// Private constants

// Private types

// Private variables
static xSemaphoreHandle lock;
static xTaskHandle handles[TASKINFO_RUNNING_NUMELEM];
static uint32_t lastMonitorTime;

// Private functions

/**
 * Initialize library
 */
int32_t TaskMonitorInitialize(void)
{
	lock = xSemaphoreCreateRecursiveMutex();
	memset(handles, 0, sizeof(xTaskHandle)*TASKINFO_RUNNING_NUMELEM);
	lastMonitorTime = 0;
#if defined(DIAG_TASKS)
	lastMonitorTime = portGET_RUN_TIME_COUNTER_VALUE();
#endif
	return 0;
}

/**
 * Register a task handle with the library
 */
int32_t TaskMonitorAdd(TaskInfoRunningElem task, xTaskHandle handle)
{
	if (task < TASKINFO_RUNNING_NUMELEM)
	{
	    xSemaphoreTakeRecursive(lock, portMAX_DELAY);
		handles[task] = handle;
		xSemaphoreGiveRecursive(lock);
		return 0;
	}
	else
	{
		return -1;
	}
}

/**
 * Remove a task handle from the library
 */
int32_t TaskMonitorRemove(TaskInfoRunningElem task)
{
	if (task < TASKINFO_RUNNING_NUMELEM)
	{
	    xSemaphoreTakeRecursive(lock, portMAX_DELAY);
		handles[task] = 0;
		xSemaphoreGiveRecursive(lock);
		return 0;
	}
	else
	{
		return -1;
	}
}

/**
 * Update the status of all tasks
 */
void TaskMonitorUpdateAll(void)
{
#if defined(DIAG_TASKS)
	TaskInfoData data;
	int n;

	// Lock
	xSemaphoreTakeRecursive(lock, portMAX_DELAY);

#if ( configGENERATE_RUN_TIME_STATS == 1 )
	uint32_t currentTime;
	uint32_t deltaTime;
	
	/*
	 * Calculate the amount of elapsed run time between the last time we
	 * measured and now. Scale so that we can convert task run times
	 * directly to percentages.
	 */
	currentTime = portGET_RUN_TIME_COUNTER_VALUE();
	deltaTime = ((currentTime - lastMonitorTime) / 100) ? : 1; /* avoid divide-by-zero if the interval is too small */
	lastMonitorTime = currentTime;			
#endif
	
	// Update all task information
	for (n = 0; n < TASKINFO_RUNNING_NUMELEM; ++n)
	{
		if (handles[n] != 0)
		{
			data.Running[n] = TASKINFO_RUNNING_TRUE;
#if defined(ARCH_POSIX) || defined(ARCH_WIN32)
			data.StackRemaining[n] = 10000;
#else
			data.StackRemaining[n] = uxTaskGetStackHighWaterMark(handles[n]) * 4;
#endif
#if ( configGENERATE_RUN_TIME_STATS == 1 )
			/* Generate run time stats */
			data.RunningTime[n] = uxTaskGetRunTime(handles[n]) / deltaTime;
#endif
			
		}
		else
		{
			data.Running[n] = TASKINFO_RUNNING_FALSE;
			data.StackRemaining[n] = 0;
			data.RunningTime[n] = 0;
		}
	}

	// Update object
	TaskInfoSet(&data);

	// Done
	xSemaphoreGiveRecursive(lock);
#endif
}

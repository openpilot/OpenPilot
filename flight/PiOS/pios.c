/**
 ******************************************************************************
 *
 * @file       pios.c 
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2009.   
 * @brief      Sets up main tasks, tickhook, and contains the Main function.
 *                 - It all starts from here!
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


/* Project Includes */
#include "pios.h"

/* OpenPilot Includes */
#include "openpilot.h"

/* Task Priorities */
#define PRIORITY_TASK_HOOKS             ( tskIDLE_PRIORITY + 3 )

/* Function Prototypes */
static void HooksTask(void *pvParameters);


/**
* Main function
*/
int main()
{
	/* Setup Hardware */
	SysInit();

	/* Initialise OpenPilot */
	OpenPilotInit();
	
	/* *tart the task which calls the application hooks */
	xTaskCreate(HooksTask, (signed portCHAR *)"Hooks", configMINIMAL_STACK_SIZE, NULL, PRIORITY_TASK_HOOKS, NULL);

	/* Start the scheduler */
	vTaskStartScheduler();

	/* If all is well we will never reach here as the scheduler will now be running. */
	/* If we do get here, it will most likley be because we ran out of heap space. */
	return 0;
}

static void HooksTask(void *pvParameters)
{
	portTickType xLastExecutionTime;

	// Initialise the xLastExecutionTime variable on task entry
	xLastExecutionTime = xTaskGetTickCount();

	while(1) {
		vTaskDelayUntil(&xLastExecutionTime, 1 / portTICK_RATE_MS);
		
		/* Skip delay gap if we had to wait for more than 5 ticks to avoid  */
		/* unnecessary repeats until xLastExecutionTime reached xTaskGetTickCount() again */
		portTickType xCurrentTickCount = xTaskGetTickCount();
		if(xLastExecutionTime < (xCurrentTickCount - 5)) {
			xLastExecutionTime = xCurrentTickCount;
		}		
		
		
		/* Check for incoming COM messages */
		COMReceiveHandler();
	}
}
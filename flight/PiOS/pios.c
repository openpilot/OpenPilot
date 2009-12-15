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
#include <openpilot.h>

/* Task Priorities */
#define PRIORITY_TASK_HOOKS             ( tskIDLE_PRIORITY + 3 )

/* Local Variables */
static uint32_t ulIdleCycleCount = 0;
static uint32_t IdleTimePercent = 0;

/* Global Functions */
void vApplicationIdleHook(void);


/* Function Prototypes */
static void HooksTask(void *pvParameters);


/**
* Main function
*/
int main()
{
	/* Setup Hardware */
	PIOS_SYS_Init();
	PIOS_COM_Init();
	PIOS_ADC_Init();
	
	/* Initialise OpenPilot */
	OpenPilotInit();
	
	/* Start the task which calls the application hooks */
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
		
		/* Check for ADC pin changes, call ADCNotifyChange on each pin change */
		//ADCHandler(ADCNotifyChange);
		
		/* Check for incoming COM messages */
		PIOS_COM_ReceiveHandler();
	}
}


/**
* Idle hook function
*/
void vApplicationIdleHook(void)
{
	/* Called when the scheduler has no tasks to run */

	/* In here we could implement full stats for FreeRTOS
	Although this would need us to enable stats in FreeRTOS
	which is *very* costly. With the function below we can
	either print it out or just watch the variable using JTAG */
	
	/* This can give a basic indication of how much time the system spends in idle */
	/* IdleTimePercent is the percentage of time spent in idle since the scheduler started */
	/* For example a value of 75 would mean we are spending 75% of FreeRTOS Cycles in idle */
	ulIdleCycleCount++;
	IdleTimePercent = ((ulIdleCycleCount / xTaskGetTickCount()) * 100);
}


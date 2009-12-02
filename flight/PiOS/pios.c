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


/* FreeRTOS Includes */
#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>

/* Local Variables */
static unsigned long ulIdleCycleCount = 0UL;

/* Function Prototypes */
void PiosMainTask(void *pvParameters);
void SensorTask(void *pvParameters);
void ServosTask(void *pvParameters);
void vApplicationIdleHook(void);

/**
* Main function
*/
int main()
{
	/* Setup Hardware */
	SysInit();

	/* Start Main tasks. */
	xTaskCreate(PiosMainTask, (signed portCHAR *) "PiosMain", configMINIMAL_STACK_SIZE, NULL, 2, NULL);
	xTaskCreate(SensorTask, (signed portCHAR *) "Sensor", configMINIMAL_STACK_SIZE, NULL, 2, NULL);
	xTaskCreate(ServosTask, (signed portCHAR *) "Servos", configMINIMAL_STACK_SIZE, NULL, 2, NULL);

	/* Start the scheduler. */
	vTaskStartScheduler();

	/* If all is well we will never reach here as the scheduler will now be running. */
	/* If we do get here, it will most likley be because we ran out of heap space. */
	return 0;
}


/**
* PiosMainTask
*/
void PiosMainTask(void *pvParameters)
{

    while(1)
        {
        }
}

/**
* SensorTask
*/
void SensorTask(void *pvParameters)
{
    while(1)
        {
        }
}

/**
* Task to update servo positions at 50Hz
*/
void ServosTask(void *pvParameters)
{
	portTickType xLastWakeTime;
	
	/* The xLastWakeTime variable needs to be initialized with the current tick count. */
	xLastWakeTime = xTaskGetTickCount();
	
	for(;;)
	{
		/* Update Servo positions */
		
		
		/* This task should execute exactly every 20 milliseconds or 50Hz
		There is no need to update the servos any faster than this */
		vTaskDelayUntil(&xLastWakeTime, (20 / portTICK_RATE_MS));
	}
}

/**
* Idle hook function
*/
void vApplicationIdleHook(void)
{
	uint32_t IdleTimePercent = 0;
	
	/* Called when the scheduler has no tasks to run */
	/* In here we could implement stats for FreeRTOS */
	
	/* This can give a basic indication of how much time the system spends in idle */
	/* IdleTimePercent is the percentage of time spent in idle since the scheduler started */
	ulIdleCycleCount++;
	IdleTimePercent = ((ulIdleCycleCount / xTaskGetTickCount()) * 100);
}


/**
 ******************************************************************************
 *
 * @file       openpilot.c 
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2009.   
 * @brief      Sets up ans runs main OpenPilot tasks.
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


/* OpenPilot Includes */
#include "openpilot.h"


/* Local Variables */
static uint32_t ulIdleCycleCount = 0;
static uint32_t IdleTimePercent = 0;

/* Local Functions */
static void ServosTask(void *pvParameters);


/**
* Main function
*/
void OpenPilotInit(void)
{
	xTaskCreate(ServosTask, (signed portCHAR *) "Servos", configMINIMAL_STACK_SIZE, NULL, 2, NULL);


}

/**
* Task to update servo positions at 50Hz
*/
static void ServosTask(void *pvParameters)
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


/**
 ******************************************************************************
 *
 * @file       openpilot.c 
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      Sets up and runs main OpenPilot tasks.
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

/* Task Priorities */

/* Global Variables */

/* Local Variables */

/* Function Prototypes */
static void TaskTesting(void *pvParameters);

/**
* Test Main function
*/
int main()
{
	// Init
	PIOS_SYS_Init();
	PIOS_DELAY_Init();
	PIOS_COM_Init();
	PIOS_ADC_Init();
	PIOS_I2C_Init();
	xTaskCreate(TaskTesting, (signed portCHAR *)"Test", configMINIMAL_STACK_SIZE , NULL, 1, NULL);

	// Start the FreeRTOS scheduler
	vTaskStartScheduler();

	// Should not get here
	PIOS_DEBUG_Assert(0);

	return 0;
}

static void TaskTesting(void *pvParameters)
{
	portTickType xDelay = 500 / portTICK_RATE_MS;
	portTickType xTimeout = 10 / portTICK_RATE_MS;

	PIOS_BMP085_Init();

	for(;;)
	{
		PIOS_LED_Toggle(LED2);

		int16_t temp;
		int32_t pressure;

		PIOS_BMP085_StartADC(TemperatureConv);
		xSemaphoreTake(PIOS_BMP085_EOC, xTimeout);
		PIOS_BMP085_ReadADC();

		PIOS_BMP085_StartADC(PressureConv);
		xSemaphoreTake(PIOS_BMP085_EOC, xTimeout);
		PIOS_BMP085_ReadADC();

		temp = PIOS_BMP085_GetTemperature();
		pressure = PIOS_BMP085_GetPressure();

		PIOS_COM_SendFormattedStringNonBlocking(PIOS_COM_TELEM_RF, "%3u,%4u\r", temp, pressure);

		vTaskDelay(xDelay);
	}
}

/**
* Idle hook function
*/
void vApplicationIdleHook(void)
{
	/* Called when the scheduler has no tasks to run */


}

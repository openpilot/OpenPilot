/**
 ******************************************************************************
 *
 * @file       openpilot.c 
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
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


/*
 * I2C Test: communicate with external PCF8570 ram chip
 * For this test to function, PCF8570 chips need to be attached to the I2C port
 *
 * Blinking Blue LED: No errors detected, test continues
 * Blinking Red LED: Error detected, test stopped
 *
 */

/* OpenPilot Includes */
#include "openpilot.h"

#define USE_DEBUG_PINS

/* Task Priorities */
#define PRIORITY_TASK_HOOKS             (tskIDLE_PRIORITY + 3)

#ifdef USE_DEBUG_PINS
	#define	DEBUG_PIN_IDLE	5
	#define DebugPinHigh(x) PIOS_DEBUG_PinHigh(x)
	#define DebugPinLow(x)	PIOS_DEBUG_PinLow(x)
#else
	#define DebugPinHigh(x)
	#define DebugPinLow(x)
#endif

/* Global Variables */

/* Local Variables */

/* Function Prototypes */
static void TestTask(void *pvParameters);


/**
* OpenPilot Main function
*/
int main()
{
	// Init
	PIOS_SYS_Init();
	PIOS_DEBUG_Init();
	PIOS_DELAY_Init();
	PIOS_COM_Init();
	PIOS_USB_Init(0);
	PIOS_I2C_Init();

	// Create task
	xTaskCreate(TestTask, (signed portCHAR *)"Test", configMINIMAL_STACK_SIZE , NULL, 1, NULL);

	// Start the FreeRTOS scheduler
	vTaskStartScheduler();

	/* If all is well we will never reach here as the scheduler will now be running. */
	/* If we do get here, it will most likely be because we ran out of heap space. */
	return 0;
}

static void OnError(void)
{
	PIOS_LED_Off(LED1);
	while(1)
	{
		PIOS_LED_Toggle(LED2);
		vTaskDelay(50 / portTICK_RATE_MS);
	}
}


static void TestTask(void *pvParameters)
{
	int i = 0;
	portTickType xLastExecutionTime;

	// Both leds off
	PIOS_LED_Off(LED1);
	PIOS_LED_Off(LED2);

	xLastExecutionTime = xTaskGetTickCount();

	PIOS_I2C_Transfer(I2C_Write, 0x50<<1, (uint8_t*)"\x20\xB0\xB1\xB2", 4);

	for(;;)
	{
		i++;
		if (i==10)
		{
			PIOS_LED_Toggle(LED1);
			i = 0;
		}

		{
			uint8_t buf[20];
			if (PIOS_I2C_Transfer(I2C_Write, 0x50<<1, (uint8_t*)"\x10\xA0\xA1\xA2", 4) != 0)
				OnError();

			if (PIOS_I2C_Transfer(I2C_Write_WithoutStop, 0x50<<1, (uint8_t*)"\x20", 1) != 0)
				OnError();

			if (PIOS_I2C_Transfer(I2C_Read, 0x50<<1, buf, 3) != 0)
				OnError();

			if (memcmp(buf, "\xB0\xB1\xB2",3) != 0)
				OnError();

			if (PIOS_I2C_Transfer(I2C_Write_WithoutStop, 0x50<<1, (uint8_t*)"\x10", 1) != 0)
				OnError();

			if (PIOS_I2C_Transfer(I2C_Read, 0x50<<1, buf, 3) != 0)
				OnError();

			if (memcmp(buf, "\xA0\xA1\xA2",3) != 0)
				OnError();

		}

		vTaskDelayUntil(&xLastExecutionTime, 10 / portTICK_RATE_MS);
	}
}


/**
* Idle hook function
*/
void vApplicationIdleHook(void)
{
	/* Called when the scheduler has no tasks to run */
	DebugPinHigh(DEBUG_PIN_IDLE);
	DebugPinLow(DEBUG_PIN_IDLE);
}

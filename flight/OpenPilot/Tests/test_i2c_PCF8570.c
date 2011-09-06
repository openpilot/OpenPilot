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

//#define USE_DEBUG_PINS

#define DEVICE_1_ADDRESS 0x50
#define DEVICE_2_ADDRESS 0x51

#ifdef USE_DEBUG_PINS
	#define	DEBUG_PIN_TASK1_WAIT	0
	#define DEBUG_PIN_TASK1_LOCKED	1
	#define	DEBUG_PIN_TASK2_WAIT	2
	#define DEBUG_PIN_TASK2_LOCKED	3
    #define DEBUG_PIN_TRANSFER		4
	#define DEBUG_PIN_IDLE			6
	#define DEBUG_PIN_ERROR			7
	#define DebugPinHigh(x) PIOS_DEBUG_PinHigh(x)
	#define DebugPinLow(x)	PIOS_DEBUG_PinLow(x)
#else
	#define DebugPinHigh(x)
	#define DebugPinLow(x)
#endif



#define MAX_LOCK_WAIT				2		// Time in ms that a thread can normally block I2C

/* Task Priorities */
#define PRIORITY_TASK_HOOKS             (tskIDLE_PRIORITY + 3)


/* Global Variables */

/* Local Variables */

/* Function Prototypes */
static void Task1(void *pvParameters);
static void Task2(void *pvParameters);


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

	// Both leds off
	PIOS_LED_Off(LED1);
	PIOS_LED_Off(LED2);

	// Create task
	xTaskCreate(Task1, (signed portCHAR *)"Task1", 1024 , NULL, 1, NULL);
	xTaskCreate(Task2, (signed portCHAR *)"Task2", 1024 , NULL, 2, NULL);


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
		DebugPinHigh(DEBUG_PIN_ERROR);
		PIOS_LED_Toggle(LED2);
		vTaskDelay(50 / portTICK_RATE_MS);
		DebugPinLow(DEBUG_PIN_ERROR);
	}
}
//
// This is a low priority task that will continuously access one I2C device
// Frequently it will release the I2C device so that others can also use I2C
//
static void Task1(void *pvParameters)
{
	int i = 0;


	if (PIOS_I2C_LockDevice(MAX_LOCK_WAIT / portTICK_RATE_MS))
	{
		if (PIOS_I2C_Transfer(I2C_Write, DEVICE_1_ADDRESS<<1, (uint8_t*)"\x20\xB0\xB1\xB2", 4) != 0)
			OnError();
		PIOS_I2C_UnlockDevice();
	}
	else
	{
		OnError();
	}

	for(;;)
	{
		i++;
		if (i==100)
		{
			PIOS_LED_Toggle(LED1);
			i = 0;
		}

		DebugPinHigh(DEBUG_PIN_TASK1_WAIT);
		if (PIOS_I2C_LockDevice(MAX_LOCK_WAIT / portTICK_RATE_MS))
		{
			uint8_t buf[20];

			DebugPinLow(DEBUG_PIN_TASK1_WAIT);
			DebugPinHigh(DEBUG_PIN_TASK1_LOCKED);

			// Write A0 A1 A2  at address 0x10
			DebugPinHigh(DEBUG_PIN_TRANSFER);
			if (PIOS_I2C_Transfer(I2C_Write, DEVICE_1_ADDRESS<<1, (uint8_t*)"\x10\xA0\xA1\xA2", 4) != 0)
				OnError();
			DebugPinLow(DEBUG_PIN_TRANSFER);


			// Read 3 bytes at address 0x20 and check
			DebugPinHigh(DEBUG_PIN_TRANSFER);
			if (PIOS_I2C_Transfer(I2C_Write_WithoutStop, DEVICE_1_ADDRESS<<1, (uint8_t*)"\x20", 1) != 0)
				OnError();
			DebugPinLow(DEBUG_PIN_TRANSFER);

			DebugPinHigh(DEBUG_PIN_TRANSFER);
			if (PIOS_I2C_Transfer(I2C_Read, DEVICE_1_ADDRESS<<1, buf, 3) != 0)
				OnError();
			DebugPinLow(DEBUG_PIN_TRANSFER);

			if (memcmp(buf, "\xB0\xB1\xB2",3) != 0)
				OnError();

			// Read 3 bytes at address 0x10 and check
			DebugPinHigh(DEBUG_PIN_TRANSFER);
			if (PIOS_I2C_Transfer(I2C_Write_WithoutStop, DEVICE_1_ADDRESS<<1, (uint8_t*)"\x10", 1) != 0)
				OnError();
			DebugPinLow(DEBUG_PIN_TRANSFER);

			DebugPinHigh(DEBUG_PIN_TRANSFER);
			if (PIOS_I2C_Transfer(I2C_Read, DEVICE_1_ADDRESS<<1, buf, 3) != 0)
				OnError();
			DebugPinLow(DEBUG_PIN_TRANSFER);

			if (memcmp(buf, "\xA0\xA1\xA2",3) != 0)
				OnError();

			DebugPinLow(DEBUG_PIN_TASK1_LOCKED);
			PIOS_I2C_UnlockDevice();
		}
		else
		{
			OnError();
		}
	}
}

// This is a high priority task that will periodically perform some actions on the second I2C device
// Most of the time it will have to wait for the other task task to release I2C
static void Task2(void *pvParameters)
{
	portTickType xLastExecutionTime;

    xLastExecutionTime = xTaskGetTickCount();
    uint32_t count = 0;

	for(;;)
	{
		uint8_t buf[20];

		DebugPinHigh(DEBUG_PIN_TASK2_WAIT);
		if (PIOS_I2C_LockDevice(MAX_LOCK_WAIT / portTICK_RATE_MS))
		{
			DebugPinLow(DEBUG_PIN_TASK2_WAIT);
			DebugPinHigh(DEBUG_PIN_TASK2_LOCKED);

			// Write value of count to address 0x10
			buf[0] = 0x10;				// The address
			memcpy(&buf[1], &count, 4);	// The data to write
			DebugPinHigh(DEBUG_PIN_TRANSFER);
			if (PIOS_I2C_Transfer(I2C_Write, DEVICE_2_ADDRESS<<1, buf, 5) != 0)
				OnError();
			DebugPinLow(DEBUG_PIN_TRANSFER);

			DebugPinLow(DEBUG_PIN_TASK2_LOCKED);
			PIOS_I2C_UnlockDevice();
		}
		else
		{
			OnError();
		}

		vTaskDelay(2 / portTICK_RATE_MS);

		DebugPinHigh(DEBUG_PIN_TASK2_WAIT);
		if (PIOS_I2C_LockDevice(1 / portTICK_RATE_MS))
		{
			DebugPinLow(DEBUG_PIN_TASK2_WAIT);
			DebugPinHigh(DEBUG_PIN_TASK2_LOCKED);

			// Read at address 0x10 and check
			DebugPinHigh(DEBUG_PIN_TRANSFER);
			if (PIOS_I2C_Transfer(I2C_Write_WithoutStop, DEVICE_2_ADDRESS<<1, (uint8_t*)"\x10", 1) != 0)
				OnError();
			DebugPinLow(DEBUG_PIN_TRANSFER);

			DebugPinHigh(DEBUG_PIN_TRANSFER);
			if (PIOS_I2C_Transfer(I2C_Read, DEVICE_2_ADDRESS<<1, buf, 4) != 0)
				OnError();
			DebugPinLow(DEBUG_PIN_TRANSFER);

			DebugPinHigh(DEBUG_PIN_TRANSFER);
			if (memcmp(buf, &count, 4) != 0)
				OnError();
			DebugPinLow(DEBUG_PIN_TRANSFER);


			DebugPinLow(DEBUG_PIN_TASK2_LOCKED);
			PIOS_I2C_UnlockDevice();
		}
		else
		{
			OnError();
		}

		vTaskDelay(5 / portTICK_RATE_MS);

		count++;
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

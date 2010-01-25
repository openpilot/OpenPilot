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
#define PRIORITY_TASK_HOOKS             (tskIDLE_PRIORITY + 3)

/* Global Variables */

/* Local Variables */
#define STRING_MAX 1024
static uint8_t line_buffer[STRING_MAX];
static uint16_t line_ix;

/* Function Prototypes */
static void TaskTick(void *pvParameters);
static void TaskHooks(void *pvParameters);
int32_t CONSOLE_Parse(COMPortTypeDef port, char c);
void OP_ADC_NotifyChange(uint32_t pin, uint32_t pin_value);

/**
* Main function
*/
int main()
{
	
	/* Brings up System using CMSIS functions, enables the LEDs. */
	PIOS_SYS_Init();

	/* Delay system */
	PIOS_DELAY_Init();

	/* SPI Init */
	PIOS_SPI_Init();

	/* Enables the SDCard */
	PIOS_SDCARD_Init();

	/* Wait for SD card for ever */
	for(;;)
	{
		/* Check if we have an SD Card with the correct settings files on it */
		if(!PIOS_SDCARD_MountFS(STARTUP_LOG_ENABLED) && !PIOS_Settings_CheckForFiles()) {
			/* Found one without errors */
			break;
		}

		/* SD Card not found, flash for 1 second */
		PIOS_LED_On(LED1);
		PIOS_LED_On(LED2);
		for(uint32_t i = 0; i < 10; i++) {
			PIOS_LED_Toggle(LED2);
			PIOS_DELAY_Wait_mS(100);
		}
	}

	/* Call LoadSettings which populates global variables so the rest of the hardware can be configured. */
	PIOS_Settings_Load();

	/* Com ports init */
	PIOS_COM_Init();

	/* Initialise servo outputs */
	PIOS_Servo_Init();

	/* Analog to digital converter initialise */
	PIOS_ADC_Init();

	//PIOS_PWM_Init();

	PIOS_COM_ReceiveCallbackInit(CONSOLE_Parse);

	/* Initialise OpenPilot application */
//	OpenPilotInit();

	/* Create a FreeRTOS task */
	xTaskCreate(TaskTick, (signed portCHAR *)"Test", configMINIMAL_STACK_SIZE , NULL, 3, NULL);
	xTaskCreate(TaskHooks, (signed portCHAR *)"Hooks", configMINIMAL_STACK_SIZE, NULL, PRIORITY_TASK_HOOKS, NULL);

	/* Start the FreeRTOS scheduler */
	vTaskStartScheduler();

	/* If all is well we will never reach here as the scheduler will now be running. */
	/* If we do get here, it will most likely be because we ran out of heap space. */
	return 0;
}

int32_t CONSOLE_Parse(COMPortTypeDef port, char c)
{
	if(c == '\r') {
		/* Ignore */
	} else if(c == '\n') {
		PIOS_COM_SendFormattedString(GPS, "String: %s\n", line_buffer);
		line_ix = 0;
	} else if(line_ix < (STRING_MAX - 1)) {
		line_buffer[line_ix++] = c;
		line_buffer[line_ix] = 0;
	}

	/* No error */
	return 0;
}

void OP_ADC_NotifyChange(uint32_t pin, uint32_t pin_value)
{
	/* HW v1.0 GPS/IR Connector
	0000000
	|||||||-- 5V
	||||||--- TX (RXD on FDTI)
	|||||---- RX (TXD on FDTI)
	||||----- ADC PIN 3 (PC0)
	|||------ ADC PIN 0 (PC1)
	||------- ADC PIN 1 (PC2)
	|-------- GND
	*/

}

void TaskTick(void *pvParameters)
{
	const portTickType xDelay = 500 / portTICK_RATE_MS;

	/* Setup the LEDs to Alternate */
	PIOS_LED_On(LED1);
	PIOS_LED_Off(LED2);

	for(;;)
	{
		PIOS_LED_Toggle(LED1);
		//PIOS_LED_Toggle(LED2);
		//PIOS_DELAY_Wait_mS(250);
		vTaskDelay(xDelay);
	}

	/*
	const portTickType xDelay = 1 / portTICK_RATE_MS;

	Used to test servos, cycles all servos from one side to the other
	for(;;) {
		for(int i = 1000; i < 2000; i++) {
			PIOS_Servo_Set(0, i);
			PIOS_Servo_Set(1, i);
			PIOS_Servo_Set(2, i);
			PIOS_Servo_Set(3, i);
			PIOS_Servo_Set(4, i);
			PIOS_Servo_Set(5, i);
			PIOS_Servo_Set(6, i);
			PIOS_Servo_Set(7, i);
			vTaskDelay(xDelay);
		}
		for(int i = 2000; i > 1000; i--) {
			PIOS_Servo_Set(0, i);
			PIOS_Servo_Set(1, i);
			PIOS_Servo_Set(2, i);
			PIOS_Servo_Set(3, i);
			PIOS_Servo_Set(4, i);
			PIOS_Servo_Set(5, i);
			PIOS_Servo_Set(6, i);
			PIOS_Servo_Set(7, i);
			vTaskDelay(xDelay);
		}
	}
	*/
}

static void TaskHooks(void *pvParameters)
{
	portTickType xLastExecutionTime;

	// Initialise the xLastExecutionTime variable on task entry
	xLastExecutionTime = xTaskGetTickCount();

	for(;;) {
		vTaskDelayUntil(&xLastExecutionTime, 1 / portTICK_RATE_MS);

		/* Skip delay gap if we had to wait for more than 5 ticks to avoid */
		/* unnecessary repeats until xLastExecutionTime reached xTaskGetTickCount() again */
		portTickType xCurrentTickCount = xTaskGetTickCount();
		if(xLastExecutionTime < (xCurrentTickCount - 5))
		xLastExecutionTime = xCurrentTickCount;

		/* Check for incoming COM messages */
		PIOS_COM_ReceiveHandler();

		/* Check for incoming ADC notifications */
		PIOS_ADC_Handler(OP_ADC_NotifyChange);
	}
}

/**
* Idle hook function
*/
void vApplicationIdleHook(void)
{
	/* Called when the scheduler has no tasks to run */

	
}


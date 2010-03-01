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


/* OpenPilot Includes */
#include "openpilot.h"

/* Task Priorities */
#define PRIORITY_TASK_HOOKS             (tskIDLE_PRIORITY + 3)

/* Global Variables */

/* Local Variables */
#define STRING_MAX 1024
static uint8_t line_buffer[STRING_MAX];
static uint16_t line_ix;
static uint8_t sdcard_available;

/* Function Prototypes */
static void TaskTick(void *pvParameters);
static void TaskTesting(void *pvParameters);
static void TaskServos(void *pvParameters);
static void TaskHooks(void *pvParameters);
static void TaskSDCard(void *pvParameters);
int32_t CONSOLE_Parse(COMPortTypeDef port, char c);
void OP_ADC_NotifyChange(uint32_t pin, uint32_t pin_value);

/**
* OpenPilot Main function
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
			PIOS_DELAY_WaitmS(100);
		}
	}

	/* Call LoadSettings which populates global variables so the rest of the hardware can be configured. */
	PIOS_Settings_Load();

	/* Com ports init */
	PIOS_COM_Init();

	/* Initialise servo outputs */
	PIOS_Servo_Init();

	/* Analog to digital converter initialise */
	//PIOS_ADC_Init();

	//PIOS_PWM_Init();

	PIOS_USB_Init(0);

	PIOS_COM_ReceiveCallbackInit(CONSOLE_Parse);

	//PIOS_BMP085_Init();

	/* Initialise OpenPilot application */
//	OpenPilotInit();

	PIOS_I2C_Init();

	/* Create a FreeRTOS task */
	xTaskCreate(TaskTick, (signed portCHAR *)"Test", configMINIMAL_STACK_SIZE , NULL, 1, NULL);
	xTaskCreate(TaskTesting, (signed portCHAR *)"TaskTesting", configMINIMAL_STACK_SIZE , NULL, 4, NULL);
	//xTaskCreate(TaskServos, (signed portCHAR *)"Servos", configMINIMAL_STACK_SIZE , NULL, 4, NULL);
	xTaskCreate(TaskHooks, (signed portCHAR *)"Hooks", configMINIMAL_STACK_SIZE, NULL, PRIORITY_TASK_HOOKS, NULL);
	//xTaskCreate(TaskSDCard, (signed portCHAR *)"SDCard", configMINIMAL_STACK_SIZE, NULL, (tskIDLE_PRIORITY + 2), NULL);

	/* Start the FreeRTOS scheduler */
	vTaskStartScheduler();

	/* If all is well we will never reach here as the scheduler will now be running. */
	/* If we do get here, it will most likely be because we ran out of heap space. */
	return 0;
}

int32_t CONSOLE_Parse(COMPortTypeDef port, char c)
{
	if(port == COM_USB_HID) {
		PIOS_COM_SendChar(COM_DEBUG_USART, c);

		return 0;
	}

	if(c == '\r') {
		/* Ignore */
	} else if(c == '\n') {
		PIOS_COM_SendFormattedString(COM_DEBUG_USART, "String: %s\n", line_buffer);
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

static void TaskTick(void *pvParameters)
{
	portTickType xLastExecutionTime;

	/* Setup the LEDs to Alternate */
	PIOS_LED_On(LED1);
	PIOS_LED_Off(LED2);

	for(;;)
	{
		PIOS_LED_Toggle(LED1);

		// I2C Test: communicate with external PCF8570 ram chip
		{
			uint8_t buf[20];
			PIOS_I2C_Transfer(I2C_Write, 0x50<<1, (uint8_t*)"\x0\x10\x11\x12", 4);
			PIOS_I2C_Transfer(I2C_Write_WithoutStop, 0x50<<1, (uint8_t*)"\x0", 1);
			PIOS_I2C_Transfer(I2C_Read, 0x50<<1, buf, 3);
		}


		vTaskDelayUntil(&xLastExecutionTime, 100 / portTICK_RATE_MS);
	}
}

static void TaskTesting(void *pvParameters)
{
	portTickType xDelay = 1000 / portTICK_RATE_MS;;



	for(;;)
	{
		int32_t state = PIOS_USB_CableConnected();
		PIOS_COM_SendFormattedStringNonBlocking(COM_DEBUG_USART, "State: %d\r", state);

		vTaskDelay(xDelay);
	}
}

static void TaskServos(void *pvParameters)
{
	/* For testing servo outputs */
	portTickType xDelay;

	/* Used to test servos, cycles all servos from one side to the other */
	for(;;) {
		xDelay = 250 / portTICK_RATE_MS;
		PIOS_Servo_Set(0, 2000);
		vTaskDelay(xDelay);
		PIOS_Servo_Set(1, 2000);
		vTaskDelay(xDelay);
		PIOS_Servo_Set(2, 2000);
		vTaskDelay(xDelay);
		PIOS_Servo_Set(3, 2000);
		vTaskDelay(xDelay);
		PIOS_Servo_Set(4, 2000);
		vTaskDelay(xDelay);
		PIOS_Servo_Set(5, 2000);
		vTaskDelay(xDelay);
		PIOS_Servo_Set(6, 2000);
		vTaskDelay(xDelay);
		PIOS_Servo_Set(7, 2000);
		vTaskDelay(xDelay);

		PIOS_Servo_Set(7, 1000);
		vTaskDelay(xDelay);
		PIOS_Servo_Set(6, 1000);
		vTaskDelay(xDelay);
		PIOS_Servo_Set(5, 1000);
		vTaskDelay(xDelay);
		PIOS_Servo_Set(4, 1000);
		vTaskDelay(xDelay);
		PIOS_Servo_Set(3, 1000);
		vTaskDelay(xDelay);
		PIOS_Servo_Set(2, 1000);
		vTaskDelay(xDelay);
		PIOS_Servo_Set(1, 1000);
		vTaskDelay(xDelay);
		PIOS_Servo_Set(0, 1000);
		vTaskDelay(xDelay);

		xDelay = 1 / portTICK_RATE_MS;
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

static void TaskSDCard(void *pvParameters)
{
	uint16_t second_delay_ctr = 0;
	portTickType xLastExecutionTime;

	/* Initialise the xLastExecutionTime variable on task entry */
	xLastExecutionTime = xTaskGetTickCount();

	for(;;) {
		vTaskDelayUntil(&xLastExecutionTime, 1 / portTICK_RATE_MS);

		/* Each second: */
		/* Check if SD card is available */
		/* High-speed access if SD card was previously available */
		if(++second_delay_ctr >= 1000) {
			second_delay_ctr = 0;

			uint8_t prev_sdcard_available = sdcard_available;
			sdcard_available = PIOS_SDCARD_CheckAvailable(prev_sdcard_available);

			if(sdcard_available && !prev_sdcard_available) {
				/* SD Card has been connected! */
				/* Switch to mass storage device */
				MSD_Init(0);
			} else if(!sdcard_available && prev_sdcard_available) {
				/* Re-init USB for HID */
				PIOS_USB_Init(1);
				/* SD Card disconnected! */
			}
		}

		/* Each millisecond: */
		/* Handle USB access if device is available */
		if(sdcard_available) {
			MSD_Periodic_mS();
		}
	}
}

/**
* Idle hook function
*/
void vApplicationIdleHook(void)
{
	/* Called when the scheduler has no tasks to run */


}

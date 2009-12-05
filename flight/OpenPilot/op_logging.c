/**
 ******************************************************************************
 *
 * @file       op_logging.c 
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2009.   
 * @brief      OpenPilot Logging Functions
 * @see        The GNU Public License (GPL) Version 3
 * @defgroup   OP_LOGGING Logging Functions
 * @{
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

/* Global Variables */
xQueueHandle xLoggingQueue;

/* Local Variables */
static uint8_t FlightLogFilename[128];

/* Local Functions */
static void OP_Logging_MicroSDGateKeeperTask(void *pvParameters);


/**
* Main function
*/
void OP_Logging_Init(void)
{
	uint16_t FileCount;
	FILINFO DummyFileInfo;
	
	/* Create the logging queue */
	xLoggingQueue = xQueueCreate(15, sizeof(LogTypeDef));
	
	/* This is a crude way to file the next avaiable number avaiable */
	/* The proper way would be to use folders with dates, we will get to that later */
	for(FileCount = 0; FileCount < 65536; FileCount++) {
		sprintf((char *)FlightLogFilename, "Flight_Log_%d.txt", FileCount);
		if(f_stat((char *)FlightLogFilename, &DummyFileInfo) != FR_OK) {
			/* We have come to a file that doesn't extist */
			break;
		}
	}
	
	/* Start the gatekeeper task */
	xTaskCreate(OP_Logging_MicroSDGateKeeperTask, (signed portCHAR *) "Logging_MicroSDGateKeeperTask", configMINIMAL_STACK_SIZE, NULL, OP_LOGGING_TASK_PRI, NULL);
}

/**
* Task to handle the MicroSD log que
*/
void OP_Logging_MicroSDGateKeeperTask(void *pvParameters)
{
	FIL File;
	LogTypeDef pcMessageToLog;
	
	for(;;) {
		xQueueReceive(xLoggingQueue, &pcMessageToLog, portMAX_DELAY);
		
		/* We don't want this take to get pre-empted, so enter critical state */
		/* If we do get pre-empted we face corrupting the MicroSD filesystem */
		taskENTER_CRITICAL();
		
		/* Open the correct log file */
		switch(pcMessageToLog.Type) {
			case FLIGHT_LOG:
				f_open(&File, (char *)FlightLogFilename, FA_OPEN_EXISTING);
				break;
			case RC_LOG:
				break;
		}
		
		/* Write the stuff */
		f_puts(pcMessageToLog.Message, &File);
		
		/* Sync the MicroSD Card */
		f_sync(&File);
		
		/* Close the file */
		f_close(&File);
		
		/* Exit the critical stage */
		taskEXIT_CRITICAL();
	}
}

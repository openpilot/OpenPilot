/**
 ******************************************************************************
 *
 * @file       MKSerial.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      Interfacing with MK via serial port
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

#include "openpilot.h"


//
// Configuration
//
#define DEBUG_PORT		COM_USART1
#define STACK_SIZE		1024
#define TASK_PRIORITY	(tskIDLE_PRIORITY + 3)
#define ENABLE_DEBUG_MSG

//
// Private constants
//

#ifdef ENABLE_DEBUG_MSG
	#define DEBUG_MSG(format, ...) PIOS_COM_SendFormattedString(DEBUG_PORT, format, ## __VA_ARGS__)
#else
	#define DEBUG_MSG(format, ...)
#endif


//
// Private types
//

//
// Private variables
//

//
// Private functions
//
static void Task(void* parameters)
{
	uint8_t msg[63] = {0x03,0x3F,0x03,00,00,00,00,00,0x91,0x0A,0x00,00,00,00,00,00,00,0xFD,0x17,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,0x91,0x0A};

	PIOS_COM_ChangeBaud(DEBUG_PORT, 57600);

	DEBUG_MSG("OSD ET Std Started\n\r");


	while (1)
	{
		msg[10]++;
		if (PIOS_I2C_LockDevice(5000 / portTICK_RATE_MS))
		{
			PIOS_I2C_Transfer(I2C_Write, 0x30<<1, msg, sizeof(msg));
			PIOS_I2C_UnlockDevice();
		}
		vTaskDelay(100 / portTICK_RATE_MS);

	}
}


/**
 * Initialise the module
 * \return -1 if initialisation failed
 * \return 0 on success
 */
int32_t OsdEtStdInitialize(void)
{
	// Start gps task
	xTaskCreate(Task, (signed char*) "Osd", STACK_SIZE, NULL, TASK_PRIORITY, NULL);

	return 0;
}

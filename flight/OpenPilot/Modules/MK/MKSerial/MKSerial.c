/**
 ******************************************************************************
 *
 * @file       GPS.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      GPS module, handles GPS and NMEA stream
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
#include "MkSerial.h"
#include "gpsobject.h"


// Debugging


// Private functions
static void MkSerialTask(void* parameters);





// Private constants
#define PORT			COM_USART1
#define STACK_SIZE		1024
#define TASK_PRIORITY	(tskIDLE_PRIORITY + 3)


// Private types

// Private variables


/**
 * Initialise the module
 * \return -1 if initialisation failed
 * \return 0 on success
 */
int32_t MkSerialInitialize(void)
{
	// Start gps task
	xTaskCreate(MkSerialTask, (signed char*)"MkSerial", STACK_SIZE, NULL, TASK_PRIORITY, NULL);

	return 0;
}


/**
 * gps task. Processes input buffer. It does not return.
 */
static void MkSerialTask(void* parameters)
{
	PIOS_COM_ChangeBaud(PORT, 38400);
	PIOS_COM_SendString(PORT, "MKSerial Started\n");

	while(1)
	{
		int32_t len;

		len = PIOS_COM_ReceiveBufferUsed(PORT);
		if (len)
		{
			PIOS_COM_SendFormattedString(PORT, "Received %d: ", len);

			for (int32_t n = 0; n < len; ++n)
			{
				PIOS_COM_SendChar(PORT, PIOS_COM_ReceiveBuffer(PORT));
			}

			PIOS_COM_SendString(PORT, "\n");
		}
		vTaskDelay(100 / portTICK_RATE_MS);
	}


}

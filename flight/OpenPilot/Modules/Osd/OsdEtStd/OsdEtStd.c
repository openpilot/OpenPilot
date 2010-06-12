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

//	                  | Header / cmd?|                                  | V  |                                  |  V |                                                                     | LAT: 37 57.0000   | LONG: 24 00.4590  |                             | Hom<-179| Alt 545.4 m       |                        |#sat|stat|
//	                     00   01   02   03   04   05   06   07   08   09   10   11   12   13   14   15   16   17   18   19   20   21   22   23   24   25   26   27   28   29   30   31   32   33   34   35   36   37   38   39   40   41   42   43   44   45   46   47   48   49   50   51   52   53   54   55   56   57   58   59   60   61   62
//	uint8_t msg[63] = {0x03,0x3F,0x03,0x00,0x00,0x00,0x00,0x00,0x91,0x0A,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFD,0x17,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x91,0x0A};
//	uint8_t msg[63] = {0x03,0x3F,0x03,0x00,0x00,0x00,0x00,0x00,0x8F,0x0A,0x40,0x00,0x00,0x00,0x00,0x00,0x00,0xFD,0x17,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x57,0x37,0x90,0x45,0x00,0x24,0x09,0x00,0x24,0x03,0x00,0x00,0x90,0x00,0x54,0x54,0x00,0x00,0x52,0x11,0x20,0x00,0x00,0x06,0x2B,0x00,0x8F,0x0A};
	uint8_t msg[63] = {0x03,0x3F,0x03,0x00,0x00,0x00,0x00,0x00,0x90,0x0A,0x8A,0x00,0x00,0x00,0x00,0x00,0x00,0xFC,0x17,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x57,0x37,0x14,0x33,0x02,0x24,0x09,0x60,0x10,0x02,0x00,0x00,0x90,0x00,0x54,0x54,0x00,0x00,0x33,0x28,0x13,0x00,0x00,0x08,0x00,0x00,0x90,0x0A};
//	uint8_t msg[63] = {0x03,0x3F,0x03,0x00,0x00,0x00,0x00,0x00,0x90,0x0A,0x8A,0x00,0x00,0x00,0x00,0x00,0x00,0xFC,0x17,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x57,0x37,0x14,0x33,0x02,0x24,0x09,0x60,0x10,0x02,0x00,0x00,0x90,0x00,0x54,0x54,0x00,0x00,0x33,0x28,0x13,0x00,0x00,0x08,0x03,0x00,0x90,0x0A};

static bool fix=FALSE;
static bool newGpsData=FALSE;

static void WriteToMsg8(uint8_t index, uint8_t value)
{
	if (value>100)
		value = 100;

	msg[index] = ((value/10) << 4) + (value%10);
}

static void WriteToMsg16(uint8_t index, uint16_t value)
{
	WriteToMsg8(index, value % 100);
	WriteToMsg8(index+1, value / 100);
}

static void WriteToMsg32(uint8_t index, uint32_t value)
{
	WriteToMsg16(index, value % 10000);
	WriteToMsg16(index+2, value / 10000);
}

static void SetHomeDir(uint32_t dir)
{
	int etDir;

	etDir = dir - 89;
	if (etDir < 0)
		etDir += 360;

	WriteToMsg16(47, etDir);
}

static void SetAltitude(uint32_t altitudeMeter)
{
	WriteToMsg32(49, altitudeMeter*10);
}

static void SetVoltage(uint32_t milliVolt)
{
	msg[18] = (milliVolt / 6444)<<4;
	msg[10] = (milliVolt % 6444)*256/6444;
}

//
// Private functions
//
static void Task(void* parameters)
{
	int dir=0;
	int alt=100;
	int voltage = 0;
	PIOS_COM_ChangeBaud(DEBUG_PORT, 57600);

	DEBUG_MSG("OSD ET Std Started\n\r");

	while (1)
	{
		SetHomeDir(dir);
		dir++;
		if (dir>90)
		{
			fix=TRUE;
		}
		if (dir>360)
		{
			dir = 0;
			// Change coordinates
			msg[39]++;
		}

		SetAltitude(alt);
		alt++;

		SetVoltage(voltage);
		voltage += 50;

		// GPS status
		if (fix)
			msg[59] = 0x2B;
		else
			msg[59] = 0x03;

		if (newGpsData)
		{
			msg[59] |= 0x10;
			newGpsData = FALSE;
		}
		else
		{
			newGpsData = TRUE;
		}


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

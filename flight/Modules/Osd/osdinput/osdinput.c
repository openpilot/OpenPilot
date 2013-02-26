/**
 ******************************************************************************
 * @addtogroup OpenPilotModules OpenPilot Modules
 * @{
 * @addtogroup osdinputModule osdinput Module
 * @brief Process osdinput information
 * @{
 *
 * @file       osdinput.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      osdinput module, handles osdinput stream
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

// ****************

#include "openpilot.h"
#include "osdinput.h"
#include "attitudeactual.h"
#include "fifo_buffer.h"


// ****************
// Private functions

static void OpOsdTask(void *parameters);

// ****************
// Private constants

#define GPS_TIMEOUT_MS                  500
#define NMEA_MAX_PACKET_LENGTH          33 // 82 max NMEA msg size plus 12 margin (because some vendors add custom crap) plus CR plus Linefeed
// same as in COM buffer


#ifdef PIOS_GPS_SETS_HOMELOCATION
// Unfortunately need a good size stack for the WMM calculation
	#define STACK_SIZE_BYTES            800
#else
	#define STACK_SIZE_BYTES            1024
#endif

#define TASK_PRIORITY                   (tskIDLE_PRIORITY + 4)

// ****************
// Private variables

static uint32_t oposdPort;

static xTaskHandle OpOsdTaskHandle;

static char* oposd_rx_buffer;
t_fifo_buffer rx;

static uint32_t timeOfLastCommandMs;
static uint32_t timeOfLastUpdateMs;
static uint32_t numUpdates;
static uint32_t numChecksumErrors;
static uint32_t numParsingErrors;

// ****************
/**
 * Initialise the gps module
 * \return -1 if initialisation failed
 * \return 0 on success
 */

int32_t OpOsdStart(void)
{
	// Start gps task
	xTaskCreate(OpOsdTask, (signed char *)"OSD", STACK_SIZE_BYTES/4, NULL, TASK_PRIORITY, &OpOsdTaskHandle);
	//TaskMonitorAdd(TASKINFO_RUNNING_GPS, OpOsdTaskHandle);

	return 0;
}
/**
 * Initialise the gps module
 * \return -1 if initialisation failed
 * \return 0 on success
 */
int32_t OpOsdInitialize(void)
{
	AttitudeActualInitialize();
	// Initialize quaternion
	AttitudeActualData attitude;
	AttitudeActualGet(&attitude);
	attitude.q1 = 1;
	attitude.q2 = 0;
	attitude.q3 = 0;
	attitude.q4 = 0;
	attitude.Roll = 0;
	attitude.Pitch = 0;
	attitude.Yaw = 0;
	AttitudeActualSet(&attitude);


	// TODO: Get gps settings object
	oposdPort = PIOS_COM_OSD;

	oposd_rx_buffer = pvPortMalloc(NMEA_MAX_PACKET_LENGTH);
	PIOS_Assert(oposd_rx_buffer);

	return 0;
}
MODULE_INITCALL(OpOsdInitialize, OpOsdStart)

// ****************
/**
 * Main gps task. It does not return.
 */

static void OpOsdTask(void *parameters)
{
	portTickType xDelay = 100 / portTICK_RATE_MS;
	portTickType lastSysTime;
	// Loop forever
	lastSysTime = xTaskGetTickCount();	//portTickType xDelay = 100 / portTICK_RATE_MS;
	uint32_t timeNowMs = xTaskGetTickCount() * portTICK_RATE_MS;;
	//GPSPositionData GpsData;

	//uint8_t rx_count = 0;
	//bool start_flag = false;
	//bool found_cr = false;
	//int32_t gpsRxOverflow = 0;

	numUpdates = 0;
	numChecksumErrors = 0;
	numParsingErrors = 0;

	timeOfLastUpdateMs = timeNowMs;
	timeOfLastCommandMs = timeNowMs;
	uint8_t rx_count = 0;
	bool start_flag = false;
	//bool found_cr = false;
	int32_t gpsRxOverflow = 0;
	uint8_t c=0xAA;
	// Loop forever
	while (1)
	{
		/*//DMA_Cmd(DMA1_Stream2, DISABLE);   //prohibit  channel3 for a little time
		uint16_t cnt = DMA_GetCurrDataCounter(DMA1_Stream2);
    	rx.wr = rx.buf_size-cnt;
		if(rx.wr)
		{
			//PIOS_LED_Toggle(LED2);
			while (	fifoBuf_getData(&rx, &c, 1) > 0)
			{

				// detect start while acquiring stream
				if (!start_flag && ((c == 0xCB) || (c == 0x34)))
				{
					start_flag = true;
					rx_count = 0;
				}
				else
				if (!start_flag)
					continue;

				if (rx_count >= 11)
				{
					// The buffer is already full and we haven't found a valid NMEA sentence.
					// Flush the buffer and note the overflow event.
					gpsRxOverflow++;
					start_flag = false;
					rx_count = 0;
				}
				else
				{
					oposd_rx_buffer[rx_count] = c;
					rx_count++;
				}
				if (start_flag && rx_count == 11)
				{
					//PIOS_LED_Toggle(LED3);
					if(oposd_rx_buffer[1]==3)
					{
						AttitudeActualData attitude;
						AttitudeActualGet(&attitude);
						attitude.q1 = 1;
						attitude.q2 = 0;
						attitude.q3 = 0;
						attitude.q4 = 0;
						attitude.Roll = (int16_t)(oposd_rx_buffer[3] | oposd_rx_buffer[4]<<8);
						attitude.Pitch = (int16_t)(oposd_rx_buffer[5] | oposd_rx_buffer[6]<<8);
						attitude.Yaw = (int16_t)(oposd_rx_buffer[7] | oposd_rx_buffer[8]<<8);
						AttitudeActualSet(&attitude);
						//setAttitudeOsd((int16_t)(oposd_rx_buffer[5] | oposd_rx_buffer[6]<<8), //pitch
						//		(int16_t)(oposd_rx_buffer[3] | oposd_rx_buffer[4]<<8), //roll
						//		(int16_t)(oposd_rx_buffer[7] | oposd_rx_buffer[8]<<8)); //yaw

					}
					//frame completed
					start_flag = false;
					rx_count = 0;
				}
			}
		}
		//DMA_Cmd(DMA1_Stream2, ENABLE);

		 */

		//PIOS_COM_SendBufferNonBlocking(oposdPort, &c, 1);

		// This blocks the task until there is something on the buffer
		while (PIOS_COM_ReceiveBuffer(oposdPort, &c, 1, xDelay) > 0)
		{

			// detect start while acquiring stream
			if (!start_flag && ((c == 0xCB) || (c == 0x34)))
			{
				start_flag = true;
				rx_count = 0;
			}
			else
			if (!start_flag)
				continue;

			if (rx_count >= 11)
			{
				// The buffer is already full and we haven't found a valid NMEA sentence.
				// Flush the buffer and note the overflow event.
				gpsRxOverflow++;
				start_flag = false;
				rx_count = 0;
			}
			else
			{
				oposd_rx_buffer[rx_count] = c;
				rx_count++;
			}
			if (rx_count == 11)
			{
				if(oposd_rx_buffer[1]==3)
				{
					AttitudeActualData attitude;
					AttitudeActualGet(&attitude);
					attitude.q1 = 1;
					attitude.q2 = 0;
					attitude.q3 = 0;
					attitude.q4 = 0;
					attitude.Roll = (int16_t)(oposd_rx_buffer[3] | oposd_rx_buffer[4]<<8);
					attitude.Pitch = (int16_t)(oposd_rx_buffer[5] | oposd_rx_buffer[6]<<8);
					attitude.Yaw = (int16_t)(oposd_rx_buffer[7] | oposd_rx_buffer[8]<<8);
					AttitudeActualSet(&attitude);
				}
				//frame completed
				start_flag = false;
				rx_count = 0;

			}
		}
		vTaskDelayUntil(&lastSysTime, 50 / portTICK_RATE_MS);
		// Check for GPS timeout
		timeNowMs = xTaskGetTickCount() * portTICK_RATE_MS;
		if ((timeNowMs - timeOfLastUpdateMs) >= GPS_TIMEOUT_MS)
		{	// we have not received any valid GPS sentences for a while.
			// either the GPS is not plugged in or a hardware problem or the GPS has locked up.


		}
		else
		{	// we appear to be receiving GPS sentences OK, we've had an update


		}

	}
}


// ****************

/**
  * @}
  * @}
  */

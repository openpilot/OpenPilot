/**
 ******************************************************************************
 * @addtogroup OpenPilotModules OpenPilot Modules
 * @{
 * @addtogroup WavPlayerModule WavPlayer Module
 * @brief Process WavPlayer information
 * @{
 *
 * @file       WavPlayer.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      WavPlayer module
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


// ****************
// Private functions

static void WavPlayerTask(void *parameters);

// ****************
// Private constants

#define STACK_SIZE_BYTES            1600

#define TASK_PRIORITY                   (tskIDLE_PRIORITY + 4)

// ****************
// Private variables

static xTaskHandle WavPlayerTaskHandle;

static uint32_t timeOfLastCommandMs;
static uint32_t timeOfLastUpdateMs;

// ****************
int32_t WavPlayerStart(void)
{
	// Start WavPlayer task
	xTaskCreate(WavPlayerTask, (signed char *)"WavPlayer", STACK_SIZE_BYTES/4, NULL, TASK_PRIORITY, &WavPlayerTaskHandle);

	return 0;
}
/**
 * Initialise the WavPlayer module
 * \return -1 if initialisation failed
 * \return 0 on success
 */
int32_t WavPlayerInitialize(void)
{

	return 0;
}
MODULE_INITCALL(WavPlayerInitialize, WavPlayerStart)

// ****************
/**
 * Main gps task. It does not return.
 */

static void WavPlayerTask(void *parameters)
{
	portTickType xDelay = 100 / portTICK_RATE_MS;
	portTickType lastSysTime;
	// Loop forever
	lastSysTime = xTaskGetTickCount();	//portTickType xDelay = 100 / portTICK_RATE_MS;
	uint32_t timeNowMs = xTaskGetTickCount() * portTICK_RATE_MS;;


	timeOfLastUpdateMs = timeNowMs;
	timeOfLastCommandMs = timeNowMs;
#if defined(PIOS_INCLUDE_WAVE)
	WavePlayer_Start();
#endif
	// Loop forever
	while (1)
	{

		vTaskDelayUntil(&lastSysTime, 50 / portTICK_RATE_MS);
		// Check for GPS timeout
		timeNowMs = xTaskGetTickCount() * portTICK_RATE_MS;
		/*if ((timeNowMs - timeOfLastUpdateMs) >= GPS_TIMEOUT_MS)
		{	// we have not received any valid GPS sentences for a while.
			// either the GPS is not plugged in or a hardware problem or the GPS has locked up.


		}
		else
		{	// we appear to be receiving GPS sentences OK, we've had an update


		}*/

	}
}


// ****************

/**
  * @}
  * @}
  */

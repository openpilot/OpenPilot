/**
 ******************************************************************************
 *
 * @file       pios_delay.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * 	        Parts by Thorsten Klose (tk@midibox.org) (tk@midibox.org)
 * @brief      Delay Functions 
 *                 - Provides a micro-second granular delay using a TIM 
 * @see        The GNU Public License (GPL) Version 3
 * @defgroup   PIOS_DELAY Delay Functions
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


/* Project Includes */
#include "pios.h"
#include "time.h"

#if defined(PIOS_INCLUDE_DELAY)

/**
* Initialises the Timer used by PIOS_DELAY functions<BR>
* This is called from pios.c as part of the main() function
* at system start up.
* \return < 0 if initialisation failed
*/
#include <time.h>

int32_t PIOS_DELAY_Init(void)
{
	// stub

	/* No error */
	return 0;
}

/**
* Waits for a specific number of uS<BR>
* Example:<BR>
* \code
*   // Wait for 500 uS
*   PIOS_DELAY_Wait_uS(500);
* \endcode
* \param[in] uS delay (1..65535 microseconds)
* \return < 0 on errors
*/
int32_t PIOS_DELAY_WaituS(uint32_t uS)
{
	struct timespec wait,rest;
	wait.tv_sec=0;
	wait.tv_nsec=1000*uS;
	while (!nanosleep(&wait,&rest)) {
		wait=rest;
	}

	/* No error */
	return 0;
}


/**
* Waits for a specific number of mS<BR>
* Example:<BR>
* \code
*   // Wait for 500 mS
*   PIOS_DELAY_Wait_mS(500);
* \endcode
* \param[in] mS delay (1..65535 milliseconds)
* \return < 0 on errors
*/
int32_t PIOS_DELAY_WaitmS(uint32_t mS)
{
	struct timespec wait,rest;
	wait.tv_sec=mS/1000;
	wait.tv_nsec=(mS%1000)*1000000;
	while (!nanosleep(&wait,&rest)) {
		wait=rest;
	}

	/* No error */
	return 0;
}

uint32_t PIOS_DELAY_GetRaw()
{
	uint32_t raw_us = clock();
	return raw_us;
}

uint32_t PIOS_DELAY_DiffuS(uint32_t ref)
{
	uint32_t diff_clock = clock() - ref;
	uint32_t diff_us = diff_clock; // (CLOCKS_PER_SEC / 1000);
	return diff_us;
}
#endif

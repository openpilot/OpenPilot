/**
 ******************************************************************************
 * @addtogroup PIOS PIOS Core hardware abstraction layer
 * @{
 * @addtogroup PIOS_DELAY Delay Functions
 * @brief PiOS Delay functionality
 * @{
 *
 * @file       pios_delay.c
 * @author     Michael Smith Copyright (C) 2011
 * @brief      Delay Functions 
 *                 - Provides a micro-second granular delay using the CPU
 *                   cycle counter.
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
#include <pios.h>

/* these should be defined by CMSIS, but they aren't */
#define DWT_CTRL	(*(volatile unsigned long *)0xe0001000)
#define DWT_CYCCNT	(*(volatile unsigned long *)0xe0001004)

/* cycles per microsecond */
static uint32_t us_ticks;

/**
 * Initialises the Timer used by PIOS_DELAY functions.
 *
 * \return always zero (success)
 */

int32_t PIOS_DELAY_Init(void)
{
	RCC_ClocksTypeDef	clocks;

	/* compute the number of system clocks per microsecond */
	RCC_GetClocksFreq(&clocks);
	us_ticks = clocks.SYSCLK_Frequency / 1000000;

	/* turn on access to the DWT registers */
	CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;

	/* enable the CPU cycle counter */
	DWT_CTRL |= 1;

	return 0;
}

/**
 * Waits for a specific number of uS
 *
 * Example:<BR>
 * \code
 *   // Wait for 500 uS
 *   PIOS_DELAY_Wait_uS(500);
 * \endcode
 * \param[in] uS delay (1..65535 microseconds)
 * \return < 0 on errors
 */
int32_t PIOS_DELAY_WaituS(uint16_t uS)
{
	uint32_t	deadline;

	/*
	 * This logic is mildly sneaky and depends on C's casting behaviour from
	 * unsigned to signed when the MSB is set.
	 *
	 * We also depend on the difference between the deadline and DWT_CYCCNT
	 * never starting off at more than half of the counter period.  Since we
	 * can't be asked to wait more than 65.5ms, the counter would have to wrap
	 * in 131ms (approx 32THz for the 32-bit counter) for this to be a problem.
	 *
	 * If we are stopped by the debugger for more than half the counter period,
	 * and the cycle counter doesn't stop (it normally does), the delay will be
	 * protracted.
	 */
	deadline = DWT_CYCCNT + (uS * us_ticks);
	while ((int32_t)(deadline - DWT_CYCCNT) > 0) {
	}

	/* No error */
	return 0;
}

/**
 * Waits for a specific number of mS
 *
 * If FreeRTOS is configured, and the delay is longer than a tick, wait whole
 * ticks using the RTOS.  Fractional remainders or periods shorter than a tick
 * are busy-waited.
 *
 * Example:<BR>
 * \code
 *   // Wait for 500 mS
 *   PIOS_DELAY_Wait_mS(500);
 * \endcode
 * \param[in] mS delay (1..65535 milliseconds)
 * \return < 0 on errors
 */
int32_t PIOS_DELAY_WaitmS(uint16_t mS)
{
#if 0 // XXX cannot do this if the scheduler hasn't started yet...
#ifdef PIOS_INCLUDE_FREERTOS
	if (mS > portTICK_RATE_MS) {
		vTaskDelay(mS / portTICK_RATE_MS);
		mS = mS % portTICK_RATE_MS;
	}
#endif
#endif
	for (int i = 0; i < mS; i++) {
		PIOS_DELAY_WaituS(1000);
	}

	/* No error */
	return 0;
}


/**
  * @}
  * @}
  */

/**
 ******************************************************************************
 * @addtogroup PIOS PIOS Core hardware abstraction layer
 * @{
 * @addtogroup PIOS_DELAY Delay Functions
 * @brief PiOS Delay functionality
 * @{
 *
 * @file       pios_delay.c
 * @author     Michael Smith Copyright (C) 2012
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

#if defined(PIOS_INCLUDE_DELAY)

/* these should be defined by CMSIS, but they aren't */
#define DWT_CTRL	(*(volatile uint32_t *)0xe0001000)
#define CYCCNTENA	(1<<0)
#define DWT_CYCCNT	(*(volatile uint32_t *)0xe0001004)


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
	PIOS_DEBUG_Assert(us_ticks > 1);

	/* turn on access to the DWT registers */
	CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;

	/* enable the CPU cycle counter */
	DWT_CTRL |= CYCCNTENA;

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
 * \param[in] uS delay
 * \return < 0 on errors
 */
int32_t PIOS_DELAY_WaituS(uint32_t uS)
{
	uint32_t	elapsed = 0;
	uint32_t	last_count = DWT_CYCCNT;
	
	for (;;) {
		uint32_t current_count = DWT_CYCCNT;
		uint32_t elapsed_uS;

		/* measure the time elapsed since the last time we checked */
		elapsed += current_count - last_count;
		last_count = current_count;

		/* convert to microseconds */
		elapsed_uS = elapsed / us_ticks;
		if (elapsed_uS >= uS)
			break;

		/* reduce the delay by the elapsed time */
		uS -= elapsed_uS;

		/* keep fractional microseconds for the next iteration */
		elapsed %= us_ticks;
	}

	/* No error */
	return 0;
}

/**
 * Waits for a specific number of mS
 *
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
	while (mS--) {
		PIOS_DELAY_WaituS(1000);
	}

	/* No error */
	return 0;
}

/**
 * @brief Query the Delay timer for the current uS 
 * @return A microsecond value
 */
uint32_t PIOS_DELAY_GetuS()
{
	return DWT_CYCCNT / us_ticks;
}

/**
 * @brief Calculate time in microseconds since a previous time
 * @param[in] t previous time
 * @return time in us since previous time t.
 */
uint32_t PIOS_DELAY_GetuSSince(uint32_t t)
{
	return (PIOS_DELAY_GetuS() - t);
}

/**
 * @brief Get the raw delay timer, useful for timing
 * @return Unitless value (uint32 wrap around)
 */
uint32_t PIOS_DELAY_GetRaw()
{
	return DWT_CYCCNT;
}

/**
 * @brief Compare to raw times to and convert to us 
 * @return A microsecond value
 */
uint32_t PIOS_DELAY_DiffuS(uint32_t raw)
{
	uint32_t diff = DWT_CYCCNT - raw;
	return diff / us_ticks;
}

#endif

/**
  * @}
  * @}
  */

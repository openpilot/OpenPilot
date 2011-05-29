/**
 ******************************************************************************
 * @addtogroup PIOS PIOS Core hardware abstraction layer
 * @{
 * @addtogroup PIOS_DELAY Delay Functions
 * @brief PiOS Delay functionality
 * @{
 *
 * @file       pios_delay.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * 	        Parts by Thorsten Klose (tk@midibox.org) (tk@midibox.org)
 * @brief      Delay Functions 
 *                 - Provides a micro-second granular delay using a TIM 
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
#include "pios.h"

#if defined(PIOS_INCLUDE_DELAY)

/**
* Initialises the Timer used by PIOS_DELAY functions<BR>
* This is called from pios.c as part of the main() function
* at system start up.
* \return < 0 if initialisation failed
*/

int32_t PIOS_DELAY_Init(void)
{
	/* Enable timer clock */
	PIOS_DELAY_TIMER_RCC_FUNC;

	/* Time base configuration */
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	TIM_TimeBaseStructInit(&TIM_TimeBaseStructure);
	TIM_TimeBaseStructure.TIM_Period = 65535;	// maximum value
	TIM_TimeBaseStructure.TIM_Prescaler = 72 - 1;	// for 1 uS accuracy fixed to 72Mhz
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(PIOS_DELAY_TIMER, &TIM_TimeBaseStructure);

	/* Enable counter */
	TIM_Cmd(PIOS_DELAY_TIMER, ENABLE);

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
int32_t PIOS_DELAY_WaituS(uint16_t uS)
{
	uint16_t start = PIOS_DELAY_TIMER->CNT;

	/* Note that this event works on 16bit counter wrap-arounds */
	while ((uint16_t) (PIOS_DELAY_TIMER->CNT - start) <= uS) ;

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
int32_t PIOS_DELAY_WaitmS(uint16_t mS)
{
	for (int i = 0; i < mS; i++) {
		PIOS_DELAY_WaituS(1000);
	}

	/* No error */
	return 0;
}

/**
 * @brief Query the Delay timer for the current uS 
 * @return A microsecond value
 */
uint16_t PIOS_DELAY_GetuS()
{
	return PIOS_DELAY_TIMER->CNT;
}

/**
 * @brief Compute the difference between now and a reference time
 * @param[in] the reference time to compare now to
 * @return The number of uS since the delay
 * 
 * @note the user is responsible for worrying about rollover on the 16 bit uS counter
 */
int32_t PIOS_DELAY_DiffuS(uint16_t ref)
{
	int32_t ret_t = ref;
	return (int16_t) (PIOS_DELAY_GetuS() - ret_t);
}

#endif

/**
  * @}
  * @}
  */

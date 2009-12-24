/**
 ******************************************************************************
 *
 * @file       pios_delay.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2009.   
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


/**
* Initializes the Timer used by PIOS_DELAY functions<BR>
* This is called from pios.c as part of the main() function
* at system start up.
*
* We use TIM8 for this on Hardware V1. Can be changed
* with PIOS_DELAY_TIMER and PIOS_DELAY_TIMER_RCC the 
* pios_config.h file.
* 
* \return < 0 if initialisation failed
*/


int32_t PIOS_DELAY_Init(void)
{
  // enable timer clock
  if( PIOS_DELAY_TIMER_RCC == RCC_APB2Periph_TIM1 || PIOS_DELAY_TIMER_RCC == RCC_APB2Periph_TIM8 )
    RCC_APB2PeriphClockCmd(PIOS_DELAY_TIMER_RCC, ENABLE);
  else
    RCC_APB1PeriphClockCmd(PIOS_DELAY_TIMER_RCC, ENABLE);

  // time base configuration
  TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
  TIM_TimeBaseStructure.TIM_Period = 65535; // maximum value
  TIM_TimeBaseStructure.TIM_Prescaler = 72-1; // for 1 uS accuracy fixed to 72Mhz
  TIM_TimeBaseStructure.TIM_ClockDivision = 0;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
  TIM_TimeBaseInit(PIOS_DELAY_TIMER, &TIM_TimeBaseStructure);

  // enable counter
  TIM_Cmd(PIOS_DELAY_TIMER, ENABLE);

  return 0; // no error
}

/**
* Waits for a specific number of uS<BR>
* Example:<BR>
* \code
*   // wait for 500 uS
*   PIOS_DELAY_Wait_uS(500);
* \endcode
* \param[in] uS delay (1..65535 microseconds)
* \return < 0 on errors
*/
int32_t PIOS_DELAY_Wait_uS(uint16_t uS)
{
  uint16_t start = PIOS_DELAY_TIMER->CNT;

  // note that this event works on 16bit counter wrap-arounds
  while( (uint16_t)(PIOS_DELAY_TIMER->CNT - start) <= uS );

  return 0; // no error
}

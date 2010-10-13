/**
 ******************************************************************************
 *
 * @file       stopwatch.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      Stop watch function
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

#include "stopwatch.h"
#include "main.h"

// *****************************************************************************

uint32_t	resolution_us = 0;

// *****************************************************************************
// initialise the stopwatch

void STOPWATCH_init(uint32_t resolution)
{
	resolution_us = resolution;

	if (resolution_us == 0)
		return;

	// enable timer clock
	switch ((uint32_t)STOPWATCH_TIMER)
	{
		case (uint32_t)TIM1: RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE); break;
		case (uint32_t)TIM2: RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE); break;
		case (uint32_t)TIM3: RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE); break;
		case (uint32_t)TIM4: RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE); break;
		#ifdef STM32F10X_HD
			case (uint32_t)TIM5: RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5, ENABLE); break;
			case (uint32_t)TIM6: RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM6, ENABLE); break;
			case (uint32_t)TIM7: RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM7, ENABLE); break;
			case (uint32_t)TIM8: RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM8, ENABLE); break;
		#endif
	}

	// time base configuration
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	TIM_TimeBaseStructure.TIM_Period = 0xffff;													// max period
	TIM_TimeBaseStructure.TIM_Prescaler = ((PIOS_MASTER_CLOCK / 1000000) * resolution_us) - 1;	// <resolution> uS accuracy @ 72 MHz
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(STOPWATCH_TIMER, &TIM_TimeBaseStructure);

	// enable interrupt request
	TIM_ITConfig(STOPWATCH_TIMER, TIM_IT_Update, ENABLE);

	// start counter
	TIM_Cmd(STOPWATCH_TIMER, ENABLE);
}

// *****************************************************************************
// timer interrupt
/*
#ifdef STM32F10X_MD
	#if (STOPWATCH_TIMER == TIM1)
		void TIM1_IRQHandler(void)
	#if (STOPWATCH_TIMER == TIM2)
		void TIM2_IRQHandler(void)
	#elif (STOPWATCH_TIMER == TIM3)
		void TIM3_IRQHandler(void)
	#elif (STOPWATCH_TIMER == TIM4)
		void TIM4_IRQHandler(void)
	#endif
#endif
#ifdef STM32F10X_HD
	#if (STOPWATCH_TIMER == TIM1)
		void TIM1_IRQHandler(void)
	#if (STOPWATCH_TIMER == TIM2)
		void TIM2_IRQHandler(void)
	#elif (STOPWATCH_TIMER == TIM3)
		void TIM3_IRQHandler(void)
	#elif (STOPWATCH_TIMER == TIM4)
		void TIM4_IRQHandler(void)
	#elif (STOPWATCH_TIMER == TIM5)
		void TIM5_IRQHandler(void)
	#elif (STOPWATCH_TIMER == TIM6)
		void TIM6_IRQHandler(void)
	#elif (STOPWATCH_TIMER == TIM7)
		void TIM7_IRQHandler(void)
	#elif (STOPWATCH_TIMER == TIM8)
		void TIM8_IRQHandler(void)
	#endif
#endif
{

}
*/
// *****************************************************************************
// resets the stopwatch

void STOPWATCH_reset(void)
{
	if (resolution_us > 0)
	{	// reset the counter
		STOPWATCH_TIMER->CNT = 1;				// set to 1 instead of 0 to avoid new IRQ request
		TIM_ClearITPendingBit(STOPWATCH_TIMER, TIM_IT_Update);
	}
}

// *****************************************************************************
// returns the timer count since the last STOPWATCH_reset() call
// return 0xffffffff if counter overrun or not initialised

uint32_t STOPWATCH_get_count(void)
{
	uint32_t value = STOPWATCH_TIMER->CNT;				// get counter value ASAP

	if (resolution_us == 0)
		return 0xffffffff;								// not initialized

	if (TIM_GetITStatus(STOPWATCH_TIMER, TIM_IT_Update) != RESET)
		return 0xffffffff;								// timer overfloaw

	return value;										// return the timer count
}

// *****************************************************************************
// returns number of us since the last STOPWATCH_reset() call
// return 0xffffffff if counter overrun or not initialised

uint32_t STOPWATCH_get_us(void)
{
	uint32_t value = STOPWATCH_TIMER->CNT;				// get counter value ASAP

	if (resolution_us == 0)
		return 0xffffffff;								// not initialized

	if (TIM_GetITStatus(STOPWATCH_TIMER, TIM_IT_Update) != RESET)
		return 0xffffffff;								// timer overfloaw

	return (value * resolution_us);						// return number of micro seconds
}

// *****************************************************************************

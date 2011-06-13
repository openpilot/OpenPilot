/**
 ******************************************************************************
 * @addtogroup PIOS PIOS Core hardware abstraction layer
 * @{
 * @addtogroup   PIOS_PWM PWM Input Functions
 * @brief		Code to measure with PWM input
 * @{
 *
 * @file       pios_pwm.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      PWM Input functions (STM32 dependent)
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

#if defined(PIOS_INCLUDE_RTC)

#ifndef PIOS_RTC_PRESCALAR
#define PIOS_RTC_PRESCALAR 100
#endif

void PIOS_RTC_Init()
{
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_BKP | RCC_APB1Periph_PWR,
			       ENABLE);
	PWR_BackupAccessCmd(ENABLE);
	
	RCC_RTCCLKConfig(RCC_RTCCLKSource_HSE_Div128);
	RCC_RTCCLKCmd(ENABLE);
	RTC_WaitForLastTask();
	RTC_WaitForSynchro();
	RTC_WaitForLastTask();

#if defined(PIOS_INCLUDE_SPEKTRUM) || defined(PIOS_INCLUDE_SBUS)
	/* Enable the RTC Second interrupt */
	RTC_ITConfig( RTC_IT_SEC, ENABLE );
	/* Wait until last write operation on RTC registers has finished */
	RTC_WaitForLastTask();
#endif
	RTC_SetPrescaler(PIOS_RTC_PRESCALAR);	// counting at 8e6 / 128
	RTC_WaitForLastTask();
	RTC_SetCounter(0);
	RTC_WaitForLastTask();
}

uint32_t PIOS_RTC_Counter()
{
	return RTC_GetCounter();
}

float PIOS_RTC_Rate()
{
	return (float) (8e6 / 128) / (1 + PIOS_RTC_PRESCALAR);
}

float PIOS_RTC_MsPerTick() 
{
	return 1000.0f / PIOS_RTC_Rate();
}

#endif

/** 
 * @}
 * @}
 */

/**
 ******************************************************************************
 * @addtogroup AHRS AHRS
 * @brief The AHRS Modules perform
 *
 * @{ 
 * @addtogroup AHRS_TIMER AHRS Timer
 * @brief Sets up a simple timer that can be polled to estimate idle time
 * @{ 
 *
 *
 * @file       ahrs.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      INSGPS Test Program
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

#include "ahrs_timer.h"

void timer_start()
{
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_BKP | RCC_APB1Periph_PWR,
			       ENABLE);
	PWR_BackupAccessCmd(ENABLE);

	RCC_RTCCLKConfig(RCC_RTCCLKSource_HSE_Div128);
	RCC_RTCCLKCmd(ENABLE);
	RTC_WaitForLastTask();
	RTC_WaitForSynchro();
	RTC_WaitForLastTask();
	RTC_SetPrescaler(0);	// counting at 8e6 / 128
	RTC_WaitForLastTask();
	RTC_SetCounter(0);
	RTC_WaitForLastTask();
}

uint32_t timer_count()
{
	return RTC_GetCounter();
}

uint32_t timer_rate()
{
	return TIMER_RATE;
}

/**
 ******************************************************************************
 *
 * @file       watchdog.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      Modem packet handling routines
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

#include "pios.h"
#include "stm32f10x_iwdg.h"
#include "stm32f10x_dbgmcu.h"

/** 
 * @brief Initialize the watchdog timer for a specified timeout
 *
 * It is important to note that this function returns the achieved timeout 
 * for this hardware.  For hardware indendence this should be checked when 
 * scheduling updates.  Other hardware dependent details may need to be 
 * considered such as a window time which sets a minimum update time, 
 * and this function should return a recommended delay for clearing.  
 *
 * For the STM32 nominal clock rate is 32 khz, but for the maximum clock rate of
 * 60 khz and a prescalar of 4 yields a clock rate of 15 khz.  The delay that is 
 * set in the watchdog assumes the nominal clock rate, but the delay for FreeRTOS
 * to use is 75% of the minimal delay.
 *
 * @param[in] delayMs The delay period in ms
 * @returns Maximum recommended delay between updates
 */
uint16_t watchdog_Init(uint16_t delayMs)
{
	uint16_t delay = ((uint32_t)delayMs * 60) / 16;
	if (delay > 0x0fff)
		delay = 0x0fff;

	DBGMCU_Config(DBGMCU_IWDG_STOP, ENABLE);	// make the watchdog stop counting in debug mode
	IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);
	IWDG_SetPrescaler(IWDG_Prescaler_16);
	IWDG_SetReload(delay);
	IWDG_ReloadCounter();
	IWDG_Enable();

	return ((((uint32_t)delay * 16) / 60) * .75f);
}

/**
 * @brief Clear the watchdog timer
 *
 * This function must be called at the appropriate delay to prevent a reset event occuring
 */
void watchdog_Clear(void)
{
	IWDG_ReloadCounter();
}

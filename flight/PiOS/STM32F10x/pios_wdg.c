/**
 ******************************************************************************
 * @addtogroup PIOS PIOS Core hardware abstraction layer
 * @{
 * @addtogroup   PIOS_WDG Watchdog Functions
 * @brief PIOS Comamnds to initialize and clear watchdog timer
 * @{
 *
 * @file       pios_spi.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * 	        Parts by Thorsten Klose (tk@midibox.org) (tk@midibox.org)
 * @brief      Hardware Abstraction Layer for SPI ports of STM32
 * @see        The GNU Public License (GPL) Version 3
 * @notes
 *
 * The PIOS Watchdog provides a HAL to initialize a watchdog 
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

static struct wdg_configuration {
	uint16_t used_flags;
	uint16_t bootup_flags;
} wdg_configuration;

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
 * @returns Maximum recommended delay between updates based on PIOS_WATCHDOG_TIMEOUT constant
 */
uint16_t PIOS_WDG_Init()
{
	uint16_t delay = ((uint32_t) PIOS_WATCHDOG_TIMEOUT * 60) / 16;
	if (delay > 0x0fff)
		delay = 0x0fff;
#if defined(PIOS_INCLUDE_WDG)
	DBGMCU_Config(DBGMCU_IWDG_STOP, ENABLE);	// make the watchdog stop counting in debug mode
	IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);
	IWDG_SetPrescaler(IWDG_Prescaler_16);
	IWDG_SetReload(delay);
	IWDG_ReloadCounter();
	IWDG_Enable();

	// watchdog flags now stored in backup registers
	PWR_BackupAccessCmd(ENABLE);
	
	BKP_WriteBackupRegister(PIOS_WDG_REGISTER, 0x0);
	wdg_configuration.bootup_flags = BKP_ReadBackupRegister(PIOS_WDG_REGISTER);
#endif
	return delay;
}

/**
 * @brief Register a module against the watchdog 
 * 
 * There are two ways to use PIOS WDG: this is for when
 * multiple modules must be monitored.  In this case they 
 * must first register against the watchdog system and 
 * only when all of the modules have been updated with the
 * watchdog be cleared.  Each module must have its own 
 * bit in the 16 bit 
 *
 * @param[in] flag the bit this module wants to use
 * @returns True if that bit is unregistered
 */
bool PIOS_WDG_RegisterFlag(uint16_t flag_requested) 
{
	
	/* Fail if flag already registered */
	if(wdg_configuration.used_flags & flag_requested)
		return false;
	
	// FIXME: Protect with semaphore
	wdg_configuration.used_flags |= flag_requested;
	
	return true;
}

/**
 * @brief Function called by modules to indicate they are still running
 *
 * This function will set this flag in the active flags register (which is 
 * a backup regsiter) and if all the registered flags are set will clear
 * the watchdog and set only this flag in the backup register
 *
 * @param[in] flag the flag to set
 * @return true if the watchdog cleared, false if flags are pending
 */
bool PIOS_WDG_UpdateFlag(uint16_t flag) 
{	
	// we can probably avoid using a semaphore here which will be good for
	// efficiency and not blocking critical tasks.  race condition could 
	// overwrite their flag update, but unlikely to block _all_ of them 
	// for the timeout window
	uint16_t cur_flags = BKP_ReadBackupRegister(PIOS_WDG_REGISTER);
	
	if((cur_flags | flag) == wdg_configuration.used_flags) {
		PIOS_WDG_Clear();
		BKP_WriteBackupRegister(PIOS_WDG_REGISTER, flag);		
		return true;
	} else {
		BKP_WriteBackupRegister(PIOS_WDG_REGISTER, cur_flags | flag);		
		return false;
	}
		
}

/** 
 * @brief Returns the flags that were set at bootup
 * 
 * This is used for diagnostics, if only one flag not set this 
 * was likely the module that wasn't running before reset
 * 
 * @return The active flags register from bootup
 */
uint16_t PIOS_WDG_GetBootupFlags()
{
	return wdg_configuration.bootup_flags;	
}

/** 
 * @brief Returns the currently active flags
 * 
 * For external monitoring
 * 
 * @return The active flags register
 */
uint16_t PIOS_WDG_GetActiveFlags()
{
	return BKP_ReadBackupRegister(PIOS_WDG_REGISTER);
}

/**
 * @brief Clear the watchdog timer
 *
 * This function must be called at the appropriate delay to prevent a reset event occuring
 */
void PIOS_WDG_Clear(void)
{
#if defined(PIOS_INCLUDE_WDG)
	IWDG_ReloadCounter();
#endif
}

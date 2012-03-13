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

unsigned int wdg_registered_flags;
unsigned int wdg_updated_flags;
unsigned int wdg_cleared_time;
unsigned int wdg_last_update_time;
bool wdg_expired;

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
void PIOS_WDG_Init()
{
	wdg_registered_flags = 0;
	wdg_updated_flags = 0;
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
	wdg_registered_flags |= flag_requested;
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
	PIOS_WDG_Check();
	wdg_updated_flags |= flag;
	if( wdg_updated_flags == wdg_registered_flags) {
		wdg_last_update_time = PIOS_DELAY_DiffuS(wdg_cleared_time);
		wdg_updated_flags = 0;
		wdg_cleared_time = PIOS_DELAY_GetRaw();
	}
	return true;		
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
	return (uint16_t) 0xffff;
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
	return (uint16_t) 0xffff;
}

/**
 * @brief Clear the watchdog timer
 *
 * This function must be called at the appropriate delay to prevent a reset event occuring
 */
void PIOS_WDG_Clear(void)
{
}

/**
 * @brief This function returns true if the watchdog would 
 * have expired
 */
bool PIOS_WDG_Check()
{
	if(PIOS_DELAY_DiffuS(wdg_cleared_time) > 250000) {
		if(!wdg_expired)
			fprintf(stderr, "Watchdog fired!\r\n");
		wdg_expired = true;
	}
	return wdg_expired;
}
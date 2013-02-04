/**
 ******************************************************************************
 * @addtogroup PIOS PIOS Core hardware abstraction layer
 * @{
 * @addtogroup PIOS_SYS System Functions
 * @brief PIOS System Initialization code
 * @{
 *
 * @file       pios_sys.c  
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * 	        Parts by Thorsten Klose (tk@midibox.org) (tk@midibox.org)
 * @brief      Sets up basic STM32 system hardware, functions are called from Main.
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

#if defined(PIOS_INCLUDE_SYS)


/**
* Initialises all system peripherals
*/
void PIOS_SYS_Init(void)
{
	
}

/**
* Shutdown PIOS and reset the microcontroller:<BR>
* <UL>
*   <LI>Disable all RTOS tasks
*   <LI>Disable all interrupts
*   <LI>Turn off all board LEDs
*   <LI>Reset STM32
* </UL>
* \return < 0 if reset failed
*/
int32_t PIOS_SYS_Reset(void)
{
	/* We will never reach this point */
	return -1;
}

/**
* Returns the CPU's flash size (in bytes)
*/
uint32_t PIOS_SYS_getCPUFlashSize(void)
{
	return 1024000;
}

/**
* Returns the serial number as a string
* param[out] uint8_t pointer to a string which can store at least 12 bytes
* (12 bytes returned for STM32)
* return < 0 if feature not supported
*/
int32_t PIOS_SYS_SerialNumberGetBinary(uint8_t *array)
{
	/* Stored in the so called "electronic signature" */
	for (int i = 0; i < PIOS_SYS_SERIAL_NUM_BINARY_LEN; ++i) {
		array[i] = 0xff;
	}

	/* No error */
	return 0;
}

/**
* Returns the serial number as a string
* param[out] str pointer to a string which can store at least 32 digits + zero terminator!
* (24 digits returned for STM32)
* return < 0 if feature not supported
*/
int32_t PIOS_SYS_SerialNumberGet(char *str)
{
	/* Stored in the so called "electronic signature" */
	int i;
	for (i = 0; i < PIOS_SYS_SERIAL_NUM_ASCII_LEN; ++i) {
		str[i] = 'F';
	}
	str[i] = '\0';

	/* No error */
	return 0;
}

#endif

/**
  * @}
  * @}
  */

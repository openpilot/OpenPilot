/**
 ******************************************************************************
 * @addtogroup PIOS PIOS Core hardware abstraction layer
 * @{
 * @defgroup   PIOS_DEBUG Debugging Functions
 * @brief Debugging functionality
 * @{
 *
 * @file       pios_debug.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      Debugging Functions
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

// Global variables
const char *PIOS_DEBUG_AssertMsg = "ASSERT FAILED";

/* Private Function Prototypes */

/**
* Initialise Debug-features
*/
void PIOS_DEBUG_Init(void)
{
}

/**
* Set debug-pin high
* \param pin 0 for S1 output
*/
void PIOS_DEBUG_PinHigh(uint8_t Pin)
{
}

/**
* Set debug-pin low
* \param pin 0 for S1 output
*/
void PIOS_DEBUG_PinLow(uint8_t Pin)
{
}


void PIOS_DEBUG_PinValue8Bit(uint8_t value)
{
}

void PIOS_DEBUG_PinValue4BitL(uint8_t value)
{
}


/**
 * Report a serious error and halt
 */
void PIOS_DEBUG_Panic(const char *msg)
{
#ifdef PIOS_COM_DEBUG
	register int *lr asm("lr");	// Link-register holds the PC of the caller
	PIOS_COM_SendFormattedStringNonBlocking(PIOS_COM_DEBUG, "\r%s @0x%x\r", msg, lr);
#endif

	// Stay put
	while (1) ;
}

/**
  * @}
  * @}
  */

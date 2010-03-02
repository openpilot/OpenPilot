/**
 ******************************************************************************
 *
 * @file       pios_debug.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      RC Servo routines
 * @see        The GNU Public License (GPL) Version 3
 * @defgroup   PIOS_SERVO RC Servo Functions
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


/* Private Function Prototypes */

/**
* Initialise Debug-features
*/
void PIOS_DEBUG_Init(void)
{
	// Initialise Servo pins as standard output pins
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_StructInit(&GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Pin = SERVO1_PIN | SERVO2_PIN | SERVO3_PIN | SERVO4_PIN;
	GPIO_Init(SERVO1TO4_PORT, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = SERVO5_PIN | SERVO6_PIN | SERVO7_PIN | SERVO8_PIN;
	GPIO_Init(SERVO5TO8_PORT, &GPIO_InitStructure);
}

/**
* Set debug-pin high
* \param pin	0 for S1 output
*/
void PIOS_DEBUG_PinHigh(uint8_t pin)
{
	if (pin < 4)
	{
		SERVO1TO4_PORT->BSRR = (SERVO1_PIN<<pin);
	}
	else if (pin <= 7)
	{
		SERVO5TO8_PORT->BSRR = (SERVO5_PIN<<(pin-4));
	}
}

/**
* Set debug-pin low
* \param pin	0 for S1 output
*/
void PIOS_DEBUG_PinLow(uint8_t pin)
{
	if (pin < 4)
	{
		SERVO1TO4_PORT->BRR = (SERVO1_PIN<<pin);
	}
	else if (pin <= 7)
	{
		SERVO5TO8_PORT->BRR = (SERVO5_PIN<<(pin-4));
	}
}



/**
 ******************************************************************************
 * @addtogroup PIOS PIOS Core hardware abstraction layer
 * @{
 * @addtogroup   PIOS_SERVO RC Servo Functions
 * @brief Code to do set RC servo output
 * @{
 *
 * @file       pios_servo.c  
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      RC Servo routines (STM32 dependent)
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

#if defined(PIOS_INCLUDE_SERVO)

/* Private Function Prototypes */

/* Local Variables */
static volatile uint16_t ServoPosition[PIOS_SERVO_NUM_TIMERS];

/**
* Initialise Servos
*/
void PIOS_Servo_Init(void)
{
}

/**
* Set the servo update rate (Max 500Hz)
* \param[in] onetofour Rate for outputs 1 to 4 (Hz)
* \param[in] fivetoeight Rate for outputs 5 to 8 (Hz)
*/
void PIOS_Servo_SetHz(const uint16_t * banks, uint8_t num_banks)
{
}

/**
* Set servo position
* \param[in] Servo Servo number (0-7)
* \param[in] Position Servo position in milliseconds
*/
void PIOS_Servo_Set(uint8_t Servo, uint16_t Position)
{
#ifndef PIOS_ENABLE_DEBUG_PINS
	/* Make sure servo exists */
	if (Servo < PIOS_SERVO_NUM_OUTPUTS) {
		/* Update the position */
		ServoPosition[Servo] = Position;

	}
#endif // PIOS_ENABLE_DEBUG_PINS
}

#endif

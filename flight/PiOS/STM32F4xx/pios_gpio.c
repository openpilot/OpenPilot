/**
 ******************************************************************************
 * @addtogroup PIOS PIOS Core hardware abstraction layer
 * @{
 * @defgroup   PIOS_GPIO GPIO Functions
 * @brief GPIO hardware code for STM32F4xx
 * @{
 *
 * @file       pios_gpio.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2012.
 * @brief      GPIO functions, init, toggle, on & off.
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

#if defined(PIOS_INCLUDE_GPIO)

/* Private Function Prototypes */

/* Local Variables */
static GPIO_TypeDef *GPIO_PORT[PIOS_GPIO_NUM] = PIOS_GPIO_PORTS;
static const uint32_t GPIO_PIN[PIOS_GPIO_NUM] = PIOS_GPIO_PINS;

/**
* Initialises all the GPIO's
*/
void PIOS_GPIO_Init(void)
{
	/* Do nothing */
}

/**
* Enable a GPIO Pin
* \param[in] Pin Pin Number
*/
void PIOS_GPIO_Enable(uint8_t Pin)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_Pin = GPIO_PIN[Pin];
	GPIO_Init(GPIO_PORT[Pin], &GPIO_InitStructure);

	/* GPIO's Off */
	PIOS_GPIO_Off(Pin);
}

/**
* Turn on Pin
* \param[in] Pin Pin Number
*/
void PIOS_GPIO_On(uint8_t Pin)
{
	GPIO_ResetBits(GPIO_PORT[Pin], GPIO_PIN[Pin]);
}

/**
* Turn off Pin
* \param[in] Pin Pin Number
*/
void PIOS_GPIO_Off(uint8_t Pin)
{
	GPIO_SetBits(GPIO_PORT[Pin], GPIO_PIN[Pin]);
}

/**
* Toggle Pin on/off
* \param[in] Pin Pin Number
*/
void PIOS_GPIO_Toggle(uint8_t Pin)
{
	GPIO_ToggleBits(GPIO_PORT[Pin], GPIO_PIN[Pin]);
}

#endif

/**
  * @}
  * @}
  */

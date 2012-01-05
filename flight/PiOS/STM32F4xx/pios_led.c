/**
 ******************************************************************************
 * @addtogroup PIOS PIOS Core hardware abstraction layer
 * @{
 * @addtogroup PIOS_LED LED Functions
 * @brief STM32 Hardware LED handling code
 * @{
 *
 * @file       pios_led.c   
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2012.
 * @brief      LED functions, init, toggle, on & off.
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

/* @todo why aren't LEDs just GPIOs? */

/* Project Includes */
#include "pios.h"

#if defined(PIOS_INCLUDE_LED)

/* Private Function Prototypes */

/* Local Variables */
static GPIO_TypeDef *LED_GPIO_PORT[PIOS_LED_NUM] = PIOS_LED_PORTS;
static const uint32_t LED_GPIO_PIN[PIOS_LED_NUM] = PIOS_LED_PINS;

/**
* Initialises all the LED's
*/
void PIOS_LED_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;

	for (int LEDNum = 0; LEDNum < PIOS_LED_NUM; LEDNum++) {
		GPIO_InitStructure.GPIO_Pin = LED_GPIO_PIN[LEDNum];
		GPIO_Init(LED_GPIO_PORT[LEDNum], &GPIO_InitStructure);

		/* LED's Off */
		PIOS_LED_Off(LEDNum);
	}
}

/**
* Turn on LED
* \param[in] LED LED Name (LED1, LED2)
*/
void PIOS_LED_On(LedTypeDef LED)
{
	GPIO_ResetBits(LED_GPIO_PORT[LED], LED_GPIO_PIN[LED]);
}

/**
* Turn off LED
* \param[in] LED LED Name (LED1, LED2)
*/
void PIOS_LED_Off(LedTypeDef LED)
{
	GPIO_SetBits(LED_GPIO_PORT[LED], LED_GPIO_PIN[LED]);
}

/**
* Toggle LED on/off
* \param[in] LED LED Name (LED1, LED2)
*/
void PIOS_LED_Toggle(LedTypeDef LED)
{
	GPIO_ToggleBits(LED_GPIO_PORT[LED], LED_GPIO_PIN[LED]);
}

#endif

/**
  * @}
  * @}
  */

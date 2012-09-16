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

/* Project Includes */
#include "pios.h"

#if defined(PIOS_INCLUDE_LED)

#include <pios_led_priv.h>

const static struct pios_led_cfg * led_cfg;

/**
 * Initialises all the LED's
 */
int32_t PIOS_LED_Init(const struct pios_led_cfg * cfg)
{
	PIOS_Assert(cfg);
	
	/* Store away the config in a global used by API functions */
	led_cfg = cfg;
	
	for (uint8_t i = 0; i < cfg->num_leds; i++) {
		const struct pios_led * led = &(cfg->leds[i]);
		
		if (led->remap) {
			GPIO_PinAFConfig(led->pin.gpio, led->pin.init.GPIO_Pin, led->remap);
		}
		
		GPIO_Init(led->pin.gpio, &led->pin.init);
		
		PIOS_LED_Off(i);
	}
	
	return 0;
}

/**
 * Turn on LED
 * \param[in] LED LED id
 */
void PIOS_LED_On(uint32_t led_id)
{
	PIOS_Assert(led_cfg);
	
	if (led_id >= led_cfg->num_leds) {
		/* LED index out of range */
		return;
	}
	
	const struct pios_led * led = &(led_cfg->leds[led_id]);
	
	if (led->active_high)
		GPIO_SetBits(led->pin.gpio, led->pin.init.GPIO_Pin);
	else
		GPIO_ResetBits(led->pin.gpio, led->pin.init.GPIO_Pin);
}

/**
 * Turn off LED
 * \param[in] LED LED id
 */
void PIOS_LED_Off(uint32_t led_id)
{
	PIOS_Assert(led_cfg);
	
	if (led_id >= led_cfg->num_leds) {
		/* LED index out of range */
		return;
	}
	
	const struct pios_led * led = &(led_cfg->leds[led_id]);
	
	if (led->active_high)
		GPIO_ResetBits(led->pin.gpio, led->pin.init.GPIO_Pin);
	else
		GPIO_SetBits(led->pin.gpio, led->pin.init.GPIO_Pin);
}

/**
 * Toggle LED on/off
 * \param[in] LED LED id
 */
void PIOS_LED_Toggle(uint32_t led_id)
{
	PIOS_Assert(led_cfg);
	
	if (led_id >= led_cfg->num_leds) {
		/* LED index out of range */
		return;
	}
	
	const struct pios_led * led = &(led_cfg->leds[led_id]);
	
	if (GPIO_ReadOutputDataBit(led->pin.gpio, led->pin.init.GPIO_Pin) == Bit_SET) {
		if (led->active_high)
			PIOS_LED_Off(led_id);
		else
			PIOS_LED_On(led_id);
	} else {
		if (led->active_high)
			PIOS_LED_On(led_id);
		else
			PIOS_LED_Off(led_id);
	}
}

#endif

/**
 * @}
 * @}
 */

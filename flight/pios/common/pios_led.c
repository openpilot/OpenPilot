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

#include "pios.h"

#ifdef PIOS_INCLUDE_LED

#include <pios_gpio_priv.h>
#include <pios_gpio.h>
#include <stdbool.h>

#ifdef PIOS_INCLUDE_WS2811
#include <pios_ws2811.h>

union {
    Color   color;
    uint8_t carr[3];
} color[3];

#endif
static uint32_t pios_led_gpios_id;

/**
 * Initialises all the LED's
 */
int32_t PIOS_LED_Init(const struct pios_gpio_cfg *cfg)
{
#ifdef PIOS_INCLUDE_WS2811
    for (uint8_t i = 0; i < 3; i++) {
        color[i].color.R = 0;
        color[i].color.G = 0;
        color[i].color.B = 0;
    }
#endif
    PIOS_Assert(cfg);
    return PIOS_GPIO_Init(&pios_led_gpios_id, cfg);
}

/**
 * Turn on LED
 * \param[in] LED LED id
 */
void PIOS_LED_On(uint32_t led_id)
{
    PIOS_GPIO_On(pios_led_gpios_id, led_id);
#ifdef PIOS_INCLUDE_WS2811
    if (led_id < 3) {
        color[led_id].carr[led_id] = 0xff;
        PIOS_WS2811_setColorRGB(color[led_id].color, led_id, true);
    }
#endif
}

/**
 * Turn off LED
 * \param[in] LED LED id
 */
void PIOS_LED_Off(uint32_t led_id)
{
    PIOS_GPIO_Off(pios_led_gpios_id, led_id);
#ifdef PIOS_INCLUDE_WS2811
    if (led_id < 3) {
        color[led_id].carr[led_id] = 0x0;
        PIOS_WS2811_setColorRGB(color[led_id].color, led_id, true);
    }
#endif
}

/**
 * Toggle LED on/off
 * \param[in] LED LED id
 */
void PIOS_LED_Toggle(uint32_t led_id)
{
    PIOS_GPIO_Toggle(pios_led_gpios_id, led_id);
#ifdef PIOS_INCLUDE_WS2811
    if (led_id < 3) {
        color[led_id].carr[led_id] = !color[led_id].carr[led_id];
        PIOS_WS2811_setColorRGB(color[led_id].color, led_id, true);
    }
#endif
}

#endif /* PIOS_INCLUDE_LED */

/**
 * @}
 * @}
 */

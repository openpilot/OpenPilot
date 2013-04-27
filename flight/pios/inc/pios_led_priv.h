/**
 ******************************************************************************
 * @addtogroup PIOS PIOS Core hardware abstraction layer
 * @{
 * @addtogroup   PIOS_LED LED Functions
 * @brief PIOS interface for LEDs
 * @{
 *
 * @file       pios_led_priv.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      LED private definitions.
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

#ifndef PIOS_LED_PRIV_H
#define PIOS_LED_PRIV_H

#include <pios.h>
#include <pios_stm32.h>

struct pios_led {
	struct stm32_gpio pin;
	uint32_t remap;
	bool active_high;
};

struct pios_led_cfg {
	const struct pios_led * leds;
	uint8_t num_leds;
};

extern int32_t PIOS_LED_Init(const struct pios_led_cfg * cfg);

#endif /* PIOS_LED_PRIV_H */

/**
  * @}
  * @}
  */

/**
 ******************************************************************************
 * @addtogroup PIOS PIOS Core hardware abstraction layer
 * @{
 * @addtogroup   PIOS_ExtLeds External LEDs Functions
 * @brief PIOS interface for external LEDs
 * @{
 *
 * @file       pios_ext_leds_priv.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2015.
 * @brief      USART private definitions.
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
#ifndef PIOS_EXT_LEDS_PRIV_H
#define PIOS_EXT_LEDS_PRIV_H

#include <stdint.h>

struct pios_ext_leds_driver;

extern int32_t PIOS_ExtLeds_Init(uint32_t *ext_leds_id, const struct pios_ext_leds_driver *driver);

#endif /* PIOS_EXT_LEDS_PRIV_H */

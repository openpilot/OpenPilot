/**
 ******************************************************************************
 *
 * @file       pios_ext_leds.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2015.
 * @brief      External LEDs Driver
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

#ifndef PIOS_EXT_LEDS_H
#define PIOS_EXT_LEDS_H

#include <stdint.h>
#include <optypes.h>

struct pios_ext_leds_driver {
    uint8_t (*NumLeds)();
    int32_t (*SetColorRGB)(const Color_t color, uint8_t index, bool update);
    int32_t (*Update)();
};

/* Public Functions */
extern uint8_t PIOS_ExtLeds_NumLeds(uint32_t ext_leds_id);
extern int32_t PIOS_ExtLeds_SetColorRGB(uint32_t ext_leds_id, const Color_t color, uint8_t index, bool update);
extern int32_t PIOS_ExtLeds_Update(uint32_t ext_leds_id);

#endif /* PIOS_EXT_LEDS_H */

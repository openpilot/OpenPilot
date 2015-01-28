/**
 ******************************************************************************
 *
 * @file       pios_extled.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2015.
 * @brief      External LED API
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

#ifndef PIOS_EXTLED_H_
#define PIOS_EXTLED_H_

#include <stdint.h>
#include <optypes.h>

struct ExtLedsBridge {
    uint8_t (*NumLeds)();
    int32_t (*SetColorRGB)(const Color_t color, uint8_t index, bool update);
    int32_t (*Update)();
};

#endif /* PIOS_EXTLED_H_ */

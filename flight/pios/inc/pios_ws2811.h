/**
 ******************************************************************************
 *
 * @file       pios_ws2811.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2014.
 * @brief      A driver for ws2811 rgb led controller.
 *             this is a plain PiOS port of the very clever solution
 *             implemented by Omri Iluz in the chibios driver here:
 *             https://github.com/omriiluz/WS2812B-LED-Driver-ChibiOS
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
#ifndef PIOS_WS2811_H_
#define PIOS_WS2811_H_

#include <stdint.h>
#include <optypes.h>

#define PIOS_WS2811_NUMLEDS 2

void PIOS_WS2811_setColorRGB(Color_t c, uint8_t led, bool update);
void PIOS_WS2811_Update();

#endif /* PIOS_WS2811_H_ */

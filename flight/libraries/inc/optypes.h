/**
 ******************************************************************************
 *
 * @file       optypes.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2014.
 * @brief      OP Generic data type library
 *             --
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
#ifndef UTIL_H
#define UTIL_H
#include <stdint.h>
typedef struct {
    uint8_t R;
    uint8_t G;
    uint8_t B;
} Color_t;

extern const Color_t Color_Off;
extern const Color_t Color_Red;
extern const Color_t Color_Lime;
extern const Color_t Color_Blue;
extern const Color_t Color_Yellow;
extern const Color_t Color_Cian;
extern const Color_t Color_Magenta;
extern const Color_t Color_Navy;
extern const Color_t Color_Green;
extern const Color_t Color_Purple;
extern const Color_t Color_Teal;
extern const Color_t Color_Orange;

#define COLOR_OFF     { .R = 0x00, .G = 0x00, .B = 0x00 }
#define COLOR_RED     { .R = 0xFF, .G = 0x00, .B = 0x00 }
#define COLOR_LIME    { .R = 0x00, .G = 0xFF, .B = 0x00 }
#define COLOR_BLUE    { .R = 0x00, .G = 0x00, .B = 0xFF }
#define COLOR_YELLOW  { .R = 0xFF, .G = 0xFF, .B = 0x00 }
#define COLOR_CIAN    { .R = 0x00, .G = 0xFF, .B = 0xFF }
#define COLOR_MAGENTA { .R = 0xFF, .G = 0x00, .B = 0xFF }
#define COLOR_NAVY    { .R = 0x00, .G = 0x00, .B = 0x80 }
#define COLOR_GREEN   { .R = 0x00, .G = 0x80, .B = 0x00 }
#define COLOR_PURPLE  { .R = 0x80, .G = 0x00, .B = 0x80 }
#define COLOR_TEAL    { .R = 0x00, .G = 0x80, .B = 0x80 }
#define COLOR_ORANGE  { .R = 0xFF, .G = 0xA5, .B = 0x00 }

#endif /* UTIL_H */

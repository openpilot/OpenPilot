/**
 ******************************************************************************
 *
 * @file       main.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      Main modem header.
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


#ifndef __MAIN_H__
#define __MAIN_H__

#include <pios.h>

// *****************************************************************************

// firmware version
#define version_major       0               // 0 to 255
#define version_minor       1               // 0 to 255

// macro's for reading internal flash memory
#define mem8(addr)          (*((volatile uint8_t  *)(addr)))
#define mem16(addr)         (*((volatile uint16_t *)(addr)))
#define mem32(addr)         (*((volatile uint32_t *)(addr)))

enum {
    freqBand_UNKNOWN = 0,
    freqBand_434MHz,
    freqBand_868MHz,
    freqBand_915MHz
};

// *****************************************************************************

extern volatile uint32_t    random32;

extern bool                 booting;

extern uint32_t             flash_size;

extern char                 serial_number_str[25];
extern uint32_t             serial_number_crc32;

// *****************************************************************************

#endif

/**
 ******************************************************************************
 *
 * @file       saved_settings.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      RF Module hardware layer
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

#ifndef _SAVED_SETTINGS_H_
#define _SAVED_SETTINGS_H_

// *****************************************************************

#include "stm32f10x.h"
#include "stm32f10x_flash.h"

// *****************************************************************

typedef struct
{
    uint32_t    serial_baudrate;    // serial uart baudrate

    uint32_t    destination_id;

    uint32_t    min_frequency_Hz;
    uint32_t    max_frequency_Hz;
    uint32_t    frequency_Hz;

    uint32_t    max_rf_bandwidth;

    uint8_t     max_tx_power;

    uint8_t     frequency_band;

    uint8_t     rf_xtal_cap;

    bool        aes_enable;
    uint8_t     aes_key[16];

    uint8_t     mode;

    uint8_t     spare[31];          // allow for future unknown settings

    uint32_t    crc;
} __attribute__((__packed__)) t_saved_settings;

// *****************************************************************
// public variables

extern volatile t_saved_settings       saved_settings __attribute__ ((aligned(4)));     // a RAM copy of the settings stored in EEPROM

// *****************************************************************
// public functions

int32_t saved_settings_save(void);

void saved_settings_init(void);

// *****************************************************************

#endif

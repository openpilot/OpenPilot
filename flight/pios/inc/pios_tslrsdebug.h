/**
 ******************************************************************************
 * @addtogroup PIOS PIOS Core hardware abstraction layer
 * @{
 * @addtogroup   PIOS_TSLRSdebug TSLRS debug functions
 * @{
 *
 * @file       pios_tslrsdebug.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2011.
 * @brief      TSLRS debug functions header.
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

#ifndef PIOS_TSLRSDEBUG_H
#define PIOS_TSLRSDEBUG_H

#ifdef PIOS_INCLUDE_TSLRSDEBUG

#define TSRX_CHANNEL_MAX            24

#define DEBUG_CHAN_ACTIVE           tslrsdebug_state->ChannelCount

// TSLRSdebug parse states
typedef enum {
  TSRX_BOOT = 0,
  TSRX_VERSION_CHECK,
  TSRX_IDLE_OLDER,                  // idle for older version
  TSRX_IDLE_FROM_V25,               // idle from version 2.5 up
  TSRX_FAILSAVE_START,              // waits for :
  TSRX_FAILSAVE_SCAN,               // read data
  TSRX_GOOD_START,                  // waits for :
  TSRX_GOOD_SCAN,                   // read data
  TSRX_BAD_START,                   // waits for :
  TSRX_BAD_SCAN,                    // read data
  TSRX_VALUE_START,                 // waits for d or T
  TSRX_VALUE_READ_1,                // read data hi
  TSRX_VALUE_NEXT,                  // waits for D
  TSRX_VALUE_READ_2,                // read data lo
  TSRX_VALUE_PLOT                   // waits for I
} tsrxtalk_parse_state_t;

struct pios_tslrsdebug_state {
    uint8_t         state;
    uint8_t         version;
    uint8_t         scan_value_percent;
    uint16_t        ChannelFailsMax;
    uint16_t        ChannelFails[TSRX_CHANNEL_MAX];
    uint32_t        ChannelCount;
    portTickType    BadChannelTime;
    uint16_t        BadChannel;
    uint16_t        BadChannelDelta;
    uint16_t        Failsafes;
    uint16_t        FailsafesDelta;
    uint16_t        BadPackets;
    uint16_t        BadPacketsDelta;
    uint32_t        GoodPackets;
    uint16_t        GoodPacketsDelta;
};

extern struct pios_tslrsdebug_state *tslrsdebug_state;

/* Public Functions */
uint8_t tslrsdebug_packet_window_percent(void);

#endif /* PIOS_INCLUDE_TSLRSDEBUG */

#endif /* PIOS_TSLRSDEBUG_H */

/**
 * @}
 * @}
 */

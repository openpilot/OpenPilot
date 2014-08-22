/**
 ******************************************************************************
 *
 * @file       %FILENAME%
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2014.
 * @addtogroup [Group]
 * @{
 * @addtogroup %CLASS%
 * @{
 * @brief [Brief]
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

#ifndef UBX_AUTOCONFIG_H_
#define UBX_AUTOCONFIG_H_
#include <stdint.h>
#include <stdbool.h>
#include "UBX.h"

// Sent messages for configuration support

typedef struct {
    uint16_t mask;
    uint8_t  dynModel;
    uint8_t  fixMode;
    int32_t  fixedAlt;
    uint32_t fixedAltVar;
    int8_t   minElev;
    uint8_t  drLimit;
    uint16_t pDop;
    uint16_t tDop;
    uint16_t pAcc;
    uint16_t tAcc;
    uint8_t  staticHoldThresh;
    uint8_t  dgpsTimeOut;
    uint8_t  cnoThreshNumSVs;
    uint8_t  cnoThresh;
    uint16_t reserved2;
    uint32_t reserved3;
    uint32_t reserved4;
} ubx_cfg_nav5_t;

typedef struct {
    uint16_t measRate;
    uint16_t navRate;
    uint16_t timeRef;
} ubx_cfg_rate_t;

typedef struct {
    uint8_t msgClass;
    uint8_t msgID;
    uint8_t rate;
} ubx_cfg_msg_t;

typedef struct {
    uint8_t  prolog[2];
    uint8_t  class;
    uint8_t  id;
    uint16_t len;
} UBXSentHeader_t;

typedef union {
    uint8_t buffer[0];
    struct {
        UBXSentHeader_t header;
        union {
            ubx_cfg_nav5_t cfg_nav5;
            ubx_cfg_rate_t cfg_rate;
            ubx_cfg_msg_t  cfg_msg;
        } payload;
        uint8_t resvd[2]; // added space for checksum bytes
    } message;
} UBXSentPacket_t;

void ubx_autoconfig_run(char * *buffer, uint16_t *count);

#endif /* UBX_AUTOCONFIG_H_ */

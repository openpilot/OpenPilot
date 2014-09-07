/**
 ******************************************************************************
 *
 * @file       ubx_utils.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2014.
 * @brief      UBX Protocol utilities
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
#ifndef UBX_UTILS_H_
#define UBX_UTILS_H_

#include <stdint.h>
#include <stdbool.h>

typedef struct {
    uint8_t  syn1;
    uint8_t  syn2;
    uint8_t  class;
    uint8_t  id;
    uint16_t len;
} __attribute__((packed)) UBXHeader_t;

typedef struct {
    uint8_t chk1;
    uint8_t chk2;
} __attribute__((packed)) UBXFooter_t;

typedef union {
    uint8_t bynarystream[0];
    struct {
        UBXHeader_t header;
        uint8_t     payload[0];
    } packet;
} UBXPacket_t;

#define UBX_HEADER_LEN (sizeof(UBXHeader_t))

#define UBX_SYN1       0xB5
#define UBX_SYN2       0x62

bool ubx_getLastSentence(uint8_t *data, uint16_t bufferCount, uint8_t * *lastSentence, uint16_t *lenght);
void ubx_appendChecksum(UBXPacket_t *pkt);
void ubx_buildPacket(UBXPacket_t *pkt, uint8_t packetClass, uint8_t packetId, uint16_t len);

#endif /* UBX_UTILS_H_ */

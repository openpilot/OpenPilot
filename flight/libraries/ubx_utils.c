/**
 ******************************************************************************
 *
 * @file       ubx_utils.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2014.
 * @brief      UBX Protocol utilities.
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
#include <ubx_utils.h>
bool ubx_getLastSentence(uint8_t *data, uint16_t bufferCount, uint8_t * *lastSentence, uint16_t *lenght)
{
    const uint8_t packet_overhead = UBX_HEADER_LEN + 2;
    uint8_t *current = data + bufferCount - packet_overhead;

    while (current >= data) {
        // look for a ubx a sentence
        if (current[0] == UBX_SYN1 && current[1] == UBX_SYN2) {
            // check whether it fits the current buffer (whole sentence is into buffer)
            uint16_t len = current[4] + (current[5] << 8);
            if (len + packet_overhead + current <= data + bufferCount) {
                *lastSentence = current;
                *lenght = len + packet_overhead;
                return true;
            }
        }
        current--;
    }
    // no complete sentence found
    return false;
}

void ubx_buildPacket(UBXPacket_t *pkt, uint8_t packetClass, uint8_t packetId, uint16_t len)
{
    pkt->packet.header.syn1 = UBX_SYN1;
    pkt->packet.header.syn2 = UBX_SYN2;

    // don't make any assumption on alignments...
    ((uint8_t *)&pkt->packet.header.len)[0] = len & 0xFF;
    ((uint8_t *)&pkt->packet.header.len)[1] = (len >> 8) & 0xFF;

    pkt->packet.header.class = packetClass;
    pkt->packet.header.id    = packetId;
    ubx_appendChecksum(pkt);
}

void ubx_appendChecksum(UBXPacket_t *pkt)
{
    uint8_t chkA = 0;
    uint8_t chkB = 0;
    uint16_t len = ((uint8_t *)&pkt->packet.header.len)[0] | ((uint8_t *)&pkt->packet.header.len)[1] << 8;

    // From class field to the end of payload
    for (uint8_t i = 2; i < len + UBX_HEADER_LEN; i++) {
        chkA += pkt->bynarystream[i];
        chkB += chkA;
    }
    ;
    pkt->packet.payload[len]     = chkA;
    pkt->packet.payload[len + 1] = chkB;
}

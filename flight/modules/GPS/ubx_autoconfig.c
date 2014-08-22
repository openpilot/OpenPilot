/**
 ******************************************************************************
 * @addtogroup OpenPilotModules OpenPilot Modules
 * @{
 * @addtogroup GSPModule GPS Module
 * @brief Support code for UBX AutoConfig
 * @{
 *
 * @file       ubx_autoconfig.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2014.
 * @brief      Support code for UBX AutoConfig
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
#include "inc/ubx_autoconfig.h"

typedef enum {
    INIT_STEP_ASK_VER   = 0,
    INIT_STEP_WAIT_VER  = 1,
    INIT_STEP_CONFIGURE = 2,
    INIT_STEP_DONE = 3,
} initSteps;

static initSteps currentConfigurationStep;
// timestamp of last operation
static uint32_t lastStepTimestampRaw = 0;

UBXSentPacket_t working_packet;
void append_checksum(UBXSentPacket_t *packet)
{
    uint8_t i;
    uint8_t ck_a = 0;
    uint8_t ck_b = 0;
    uint16_t len = packet->message.header.len + sizeof(UBXSentHeader_t);

    for (i = 2; i < len; i++) {
        ck_a += packet->buffer[i];
        ck_b += ck_a;
    }

    packet->buffer[len]     = ck_a;
    packet->buffer[len + 1] = ck_b;
}
void prepare_packet(UBXSentPacket_t *packet, uint8_t classID, uint8_t messageID, uint16_t len)
{
    packet->message.header.prolog[0] = UBX_SYNC1;
    packet->message.header.prolog[1] = UBX_SYNC2;
    packet->message.header.class     = classID;
    packet->message.header.id  = messageID;
    packet->message.header.len = len;
    append_checksum(packet);
}

void build_request(UBXSentPacket_t *packet, uint8_t classID, uint8_t messageID, uint16_t *count)
{
    prepare_packet(packet, classID, messageID, 0);
    *count = packet->message.header.len + sizeof(UBXSentHeader_t) + 2;
}


void ubx_autoconfig_run(char * *buffer, uint16_t *count)
{
    uint32_t elapsed = PIOS_DELAY_DiffuS(lastStepTimestampRaw);

    switch (currentConfigurationStep) {
    case INIT_STEP_ASK_VER:
        lastStepTimestampRaw     = PIOS_DELAY_GetRaw();
        build_request(&working_packet, UBX_CLASS_MON, UBX_ID_MON_VER, count);
        *buffer = (char *)working_packet.buffer;
        currentConfigurationStep = INIT_STEP_WAIT_VER;
        break;
    case INIT_STEP_WAIT_VER:
        if (ubxHwVersion > 0) {
            currentConfigurationStep = INIT_STEP_CONFIGURE;
        } else if (elapsed > UBX_REPLY_TIMEOUT) {
            currentConfigurationStep = INIT_STEP_ASK_VER;
        }
        return;

    case INIT_STEP_CONFIGURE:
    case INIT_STEP_DONE:
        break;
    }
}

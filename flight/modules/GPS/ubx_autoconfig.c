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
#include <pios_mem.h>
// private type definitions
typedef enum {
    INIT_STEP_DISABLED = 0,
    INIT_STEP_START,
    INIT_STEP_ASK_VER,
    INIT_STEP_WAIT_VER,
    INIT_STEP_CONFIGURE,
    INIT_STEP_ENABLE_SENTENCES,
    INIT_STEP_DONE,
} initSteps_t;

typedef struct {
    initSteps_t     currentConfigurationStep; // Current configuration "fsm" status
    uint32_t        lastStepTimestampRaw; // timestamp of last operation
    UBXSentPacket_t working_packet; // outbound "buffer"
    ubx_autoconfig_settings_t currentSettings;
    int8_t lastConfigSent; // index of last configuration string sent
} ubx_autoconfig_status_t;


// private defines
#define LAST_CONFIG_SENT_START     (-1)
#define LAST_CONFIG_SENT_COMPLETED (-2)

// private variables

// enable the autoconfiguration system
static bool enabled;

static ubx_autoconfig_status_t *status = 0;

static void append_checksum(UBXSentPacket_t *packet)
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
/**
 * prepare a packet to be sent, fill the header and appends the checksum.
 * return the total packet lenght comprising header and checksum
 */
static uint16_t prepare_packet(UBXSentPacket_t *packet, uint8_t classID, uint8_t messageID, uint16_t len)
{
    packet->message.header.prolog[0] = UBX_SYNC1;
    packet->message.header.prolog[1] = UBX_SYNC2;
    packet->message.header.class     = classID;
    packet->message.header.id  = messageID;
    packet->message.header.len = len;
    append_checksum(packet);
    return packet->message.header.len + sizeof(UBXSentHeader_t) + 2; // header + payload + checksum
}

static void build_request(UBXSentPacket_t *packet, uint8_t classID, uint8_t messageID, uint16_t *bytes_to_send)
{
    *bytes_to_send = prepare_packet(packet, classID, messageID, 0);
}

void config_rate(uint16_t *bytes_to_send)
{
    memset(status->working_packet.buffer, 0, sizeof(UBXSentHeader_t) + sizeof(ubx_cfg_rate_t));
    // if rate is less than 1 uses the highest rate for current hardware
    uint16_t rate = status->currentSettings.navRate > 0 ? status->currentSettings.navRate : 99;
    if (ubxHwVersion < UBX_HW_VERSION_7 && rate > UBX_MAX_RATE) {
        rate = UBX_MAX_RATE;
    } else if (ubxHwVersion < UBX_HW_VERSION_8 && rate > UBX_MAX_RATE_VER7) {
        rate = UBX_MAX_RATE_VER7;
    } else if (ubxHwVersion >= UBX_HW_VERSION_8 && rate > UBX_MAX_RATE_VER8) {
        rate = UBX_MAX_RATE_VER8;
    }
    uint16_t period = 1000 / rate;

    status->working_packet.message.payload.cfg_rate.measRate = period;
    status->working_packet.message.payload.cfg_rate.navRate  = 1; // must be set to 1
    status->working_packet.message.payload.cfg_rate.timeRef  = 1; // 0 = UTC Time, 1 = GPS Time
    *bytes_to_send = prepare_packet(&status->working_packet, UBX_CLASS_CFG, UBX_ID_CFG_RATE, sizeof(ubx_cfg_rate_t));
}

void config_nav(uint16_t *bytes_to_send)
{
    memset(status->working_packet.buffer, 0, sizeof(UBXSentHeader_t) + sizeof(ubx_cfg_nav5_t));

    status->working_packet.message.payload.cfg_nav5.dynModel = status->currentSettings.dynamicModel;
    status->working_packet.message.payload.cfg_nav5.fixMode  = 2; // 1=2D only, 2=3D only, 3=Auto 2D/3D
    // mask LSB=dyn|minEl|posFixMode|drLim|posMask|statisticHoldMask|dgpsMask|......|reservedBit0 = MSB

    status->working_packet.message.payload.cfg_nav5.mask     = 0x01 + 0x04; // Dyn Model | posFixMode configuration
    *bytes_to_send = prepare_packet(&status->working_packet, UBX_CLASS_CFG, UBX_ID_CFG_NAV5, sizeof(ubx_cfg_nav5_t));
}

static void configure(uint16_t *bytes_to_send)
{
    switch (status->lastConfigSent) {
    case LAST_CONFIG_SENT_START:
        config_rate(bytes_to_send);
        break;
    case LAST_CONFIG_SENT_START + 1:
        config_nav(bytes_to_send);
        status->lastConfigSent = LAST_CONFIG_SENT_COMPLETED;
        return;

    default:
        status->lastConfigSent = LAST_CONFIG_SENT_COMPLETED;
        return;
    }
    status->lastConfigSent++;
}

static void enable_sentences(__attribute__((unused)) uint16_t *bytes_to_send) {}
void ubx_autoconfig_run(char * *buffer, uint16_t *bytes_to_send)
{
    *bytes_to_send = 0;
    *buffer = (char *)status->working_packet.buffer;
    if (!status || !enabled) {
        return; // autoconfig not enabled
    }
    switch (status->currentConfigurationStep) {
    case INIT_STEP_DISABLED:
    case INIT_STEP_DONE:
        return;

    case INIT_STEP_START:
    case INIT_STEP_ASK_VER:
        status->lastStepTimestampRaw     = PIOS_DELAY_GetRaw();
        build_request(&status->working_packet, UBX_CLASS_MON, UBX_ID_MON_VER, bytes_to_send);
        status->currentConfigurationStep = INIT_STEP_WAIT_VER;
        break;
    case INIT_STEP_WAIT_VER:

        if (ubxHwVersion > 0) {
            status->lastConfigSent = LAST_CONFIG_SENT_START;
            status->currentConfigurationStep = INIT_STEP_CONFIGURE;
            status->lastStepTimestampRaw     = PIOS_DELAY_GetRaw();
            return;
        }

        if (PIOS_DELAY_DiffuS(status->lastStepTimestampRaw) > UBX_REPLY_TIMEOUT) {
            status->currentConfigurationStep = INIT_STEP_ASK_VER;
        }
        return;

    case INIT_STEP_CONFIGURE:
        configure(bytes_to_send);
        if (status->lastConfigSent == LAST_CONFIG_SENT_COMPLETED) {
            status->currentConfigurationStep = INIT_STEP_ENABLE_SENTENCES;
            status->lastStepTimestampRaw     = PIOS_DELAY_GetRaw();
        }
        return;

    case INIT_STEP_ENABLE_SENTENCES:
        enable_sentences(bytes_to_send);
        if (status->lastConfigSent == LAST_CONFIG_SENT_COMPLETED) {
            status->currentConfigurationStep = INIT_STEP_DONE;
            status->lastStepTimestampRaw     = PIOS_DELAY_GetRaw();
        }
        return;
    }
}

void ubx_autoconfig_set(ubx_autoconfig_settings_t config)
{
    enabled = false;
    if (config.autoconfigEnabled) {
        if (!status) {
            status = (ubx_autoconfig_status_t *)pios_malloc(sizeof(ubx_autoconfig_status_t));
            memset(status, 0, sizeof(ubx_autoconfig_status_t));
            status->currentConfigurationStep = INIT_STEP_DISABLED;
        }
        status->currentSettings = config;
        status->currentConfigurationStep = INIT_STEP_START;
        enabled = true;
    }
}

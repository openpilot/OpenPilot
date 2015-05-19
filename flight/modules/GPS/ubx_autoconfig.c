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
#include <openpilot.h>
#include "hwsettings.h"
#include "gpssettings.h"

#include "inc/ubx_autoconfig.h"
#include <pios_mem.h>

// private type definitions

typedef enum {
    INIT_STEP_DISABLED = 0,
    INIT_STEP_START,
    INIT_STEP_SEND_MON_VER,
    INIT_STEP_WAIT_MON_VER_ACK,
    INIT_STEP_RESET_GPS,
    INIT_STEP_REVO_9600_BAUD,
    INIT_STEP_GPS_BAUD,
    INIT_STEP_REVO_BAUD,
    INIT_STEP_ENABLE_SENTENCES,
    INIT_STEP_ENABLE_SENTENCES_WAIT_ACK,
    INIT_STEP_CONFIGURE,
    INIT_STEP_CONFIGURE_WAIT_ACK,
    INIT_STEP_SAVE,
    INIT_STEP_SAVE_WAIT_ACK,
    INIT_STEP_PRE_DONE,
    INIT_STEP_DONE,
    INIT_STEP_PRE_ERROR,
    INIT_STEP_ERROR
} initSteps_t;

typedef struct {
    initSteps_t currentStep; // Current configuration "fsm" status
    initSteps_t currentStepSave; // Current configuration "fsm" status
    uint32_t    lastStepTimestampRaw; // timestamp of last operation
    uint32_t    lastConnectedRaw; // timestamp of last time gps was connected
    struct {
        union {
            struct {
        UBXSentPacket_t working_packet; // outbound "buffer"
        // bufferPaddingForPiosBugAt2400Baud must exist for baud rate change to work at 2400 or 4800
        // failure mode otherwise:
        // - send message with baud rate change
        // - wait 1 second (even at 2400, the baud rate change command should clear even an initially full 31 byte PIOS buffer much more quickly)
        // - change Revo port baud rate
        // sometimes fails (much worse for lowest baud rates)
// FIXME: remove this and retest
// FIXME: remove this and retest
// FIXME: remove this and retest
// FIXME: remove this and retest
// FIXME: remove this and retest
// FIXME: remove this and retest
// FIXME: remove this and retest
        uint8_t bufferPaddingForPiosBugAt2400Baud[2]; // must be at least 2 for 2400 to work, probably 1 for 4800 and 0 for 9600+
            } __attribute__((packed));
                GPSSettingsData gpsSettings;
        } __attribute__((packed));
    } __attribute__((packed));
    volatile ubx_autoconfig_settings_t currentSettings;
    int8_t  lastConfigSent;          // index of last configuration string sent
    struct UBX_ACK_ACK requiredAck; // Class and id of the message we are waiting for an ACK from GPS
    uint8_t retryCount;
} status_t;

ubx_cfg_msg_t msg_config_ubx6[] = {
    // messages to disable
    { .msgClass = UBX_CLASS_NAV, .msgID = UBX_ID_NAV_CLOCK,   .rate = 0  },
    { .msgClass = UBX_CLASS_NAV, .msgID = UBX_ID_NAV_POSECEF, .rate = 0  },
    { .msgClass = UBX_CLASS_NAV, .msgID = UBX_ID_NAV_SBAS,    .rate = 0  },
    { .msgClass = UBX_CLASS_NAV, .msgID = UBX_ID_NAV_TIMEGPS, .rate = 0  },
    { .msgClass = UBX_CLASS_NAV, .msgID = UBX_ID_NAV_VELECEF, .rate = 0  },

    { .msgClass = UBX_CLASS_MON, .msgID = UBX_ID_MON_HW,      .rate = 0  },
    { .msgClass = UBX_CLASS_MON, .msgID = UBX_ID_MON_HW2,     .rate = 0  },
    { .msgClass = UBX_CLASS_MON, .msgID = UBX_ID_MON_IO,      .rate = 0  },
    { .msgClass = UBX_CLASS_MON, .msgID = UBX_ID_MON_MSGPP,   .rate = 0  },
    { .msgClass = UBX_CLASS_MON, .msgID = UBX_ID_MON_RXBUFF,  .rate = 0  },
    { .msgClass = UBX_CLASS_MON, .msgID = UBX_ID_MON_RXR,     .rate = 0  },
    { .msgClass = UBX_CLASS_MON, .msgID = UBX_ID_MON_TXBUF,   .rate = 0  },

    { .msgClass = UBX_CLASS_RXM, .msgID = UBX_ID_RXM_SVSI,    .rate = 0  },

    // message to enable
    { .msgClass = UBX_CLASS_NAV, .msgID = UBX_ID_NAV_POSLLH,  .rate = 1  },
    { .msgClass = UBX_CLASS_NAV, .msgID = UBX_ID_NAV_DOP,     .rate = 1  },
    { .msgClass = UBX_CLASS_NAV, .msgID = UBX_ID_NAV_SOL,     .rate = 1  },
    { .msgClass = UBX_CLASS_NAV, .msgID = UBX_ID_NAV_STATUS,  .rate = 1  },
    { .msgClass = UBX_CLASS_NAV, .msgID = UBX_ID_NAV_VELNED,  .rate = 1  },
    { .msgClass = UBX_CLASS_NAV, .msgID = UBX_ID_NAV_TIMEUTC, .rate = 1  },
    { .msgClass = UBX_CLASS_NAV, .msgID = UBX_ID_NAV_SVINFO,  .rate = 10 },
};

ubx_cfg_msg_t msg_config_ubx7[] = {
    // messages to disable
    { .msgClass = UBX_CLASS_NAV, .msgID = UBX_ID_NAV_AOPSTATUS, .rate = 0  },
    { .msgClass = UBX_CLASS_NAV, .msgID = UBX_ID_NAV_CLOCK,     .rate = 0  },
    { .msgClass = UBX_CLASS_NAV, .msgID = UBX_ID_NAV_DGPS,      .rate = 0  },
    { .msgClass = UBX_CLASS_NAV, .msgID = UBX_ID_NAV_POSECEF,   .rate = 0  },
    { .msgClass = UBX_CLASS_NAV, .msgID = UBX_ID_NAV_SBAS,      .rate = 0  },
    { .msgClass = UBX_CLASS_NAV, .msgID = UBX_ID_NAV_TIMEGPS,   .rate = 0  },
    { .msgClass = UBX_CLASS_NAV, .msgID = UBX_ID_NAV_VELECEF,   .rate = 0  },
    { .msgClass = UBX_CLASS_NAV, .msgID = UBX_ID_NAV_SOL,       .rate = 0  },
    { .msgClass = UBX_CLASS_NAV, .msgID = UBX_ID_NAV_STATUS,    .rate = 0  },
    { .msgClass = UBX_CLASS_NAV, .msgID = UBX_ID_NAV_VELNED,    .rate = 0  },
    { .msgClass = UBX_CLASS_NAV, .msgID = UBX_ID_NAV_TIMEUTC,   .rate = 0  },
    { .msgClass = UBX_CLASS_NAV, .msgID = UBX_ID_NAV_POSLLH,    .rate = 0  },

    { .msgClass = UBX_CLASS_MON, .msgID = UBX_ID_MON_HW,        .rate = 0  },
    { .msgClass = UBX_CLASS_MON, .msgID = UBX_ID_MON_HW2,       .rate = 0  },
    { .msgClass = UBX_CLASS_MON, .msgID = UBX_ID_MON_IO,        .rate = 0  },
    { .msgClass = UBX_CLASS_MON, .msgID = UBX_ID_MON_MSGPP,     .rate = 0  },
    { .msgClass = UBX_CLASS_MON, .msgID = UBX_ID_MON_RXBUFF,    .rate = 0  },
    { .msgClass = UBX_CLASS_MON, .msgID = UBX_ID_MON_RXR,       .rate = 0  },
    { .msgClass = UBX_CLASS_MON, .msgID = UBX_ID_MON_TXBUF,     .rate = 0  },

    { .msgClass = UBX_CLASS_RXM, .msgID = UBX_ID_RXM_SVSI,      .rate = 0  },

    // message to enable
    { .msgClass = UBX_CLASS_NAV, .msgID = UBX_ID_NAV_PVT,       .rate = 1  },
    { .msgClass = UBX_CLASS_NAV, .msgID = UBX_ID_NAV_DOP,       .rate = 1  },
    { .msgClass = UBX_CLASS_NAV, .msgID = UBX_ID_NAV_SVINFO,    .rate = 10 },
};

// private defines

#define LAST_CONFIG_SENT_START     (-1)
#define LAST_CONFIG_SENT_COMPLETED (-2)
// always reset the stored GPS configuration, even when doing autoconfig.nostore
// that is required to do a 100% correct configuration
// but is unexpected because it changes the stored configuration when doing autoconfig.nostore
// note that a reset is always done with autoconfig.store
// #define ALWAYS_RESET

// private variables

// enable the autoconfiguration system
static volatile bool enabled = false;
static volatile bool current_step_touched = false;
// both the pointer and what it points to are volatile.  Yuk.
static volatile status_t *volatile status = 0;
static uint8_t hwsettings_baud;
//static HwSettingsGPSSpeedOptions hwsettings_baud;
static uint8_t baud_to_try_index = 255;
void debugindex(int index, uint8_t c);

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
    memset((uint8_t *)status->working_packet.buffer + len + sizeof(UBXSentHeader_t) + 2, 0, sizeof(status->bufferPaddingForPiosBugAt2400Baud));
    packet->message.header.prolog[0] = UBX_SYNC1;
    packet->message.header.prolog[1] = UBX_SYNC2;
    packet->message.header.class     = classID;
    packet->message.header.id  = messageID;
    packet->message.header.len = len;
    append_checksum(packet);

    status->requiredAck.clsID  = classID;
    status->requiredAck.msgID  = messageID;

    return len + sizeof(UBXSentHeader_t) + 2 + sizeof(status->bufferPaddingForPiosBugAt2400Baud); // payload + header + checksum + extra bytes
}


static void build_request(UBXSentPacket_t *packet, uint8_t classID, uint8_t messageID, uint16_t *bytes_to_send)
{
    *bytes_to_send = prepare_packet(packet, classID, messageID, 0);
}


static void set_current_step_if_untouched(initSteps_t new_steps)
{
    // assume this one byte initSteps_t is atomic
    // take care of some but not all concurrency issues

    if (!current_step_touched) {
        status->currentStep = new_steps;
    }
    if (current_step_touched) {
        status->currentStep = status->currentStepSave;
    }
}


void gps_ubx_reset_sensor_type()
{
    static uint8_t mutex; // = 0

// is this needed?
    if (__sync_fetch_and_add(&mutex, 1) == 0) {
    ubxHwVersion      = -1;
    baud_to_try_index -= 1;  // undo postincrement and start with the one that was most recently successful
    sensorType        = GPSPOSITIONSENSOR_SENSORTYPE_UNKNOWN;
    GPSPositionSensorSensorTypeSet(&sensorType);
    // make the sensor type / autobaud code time out immediately to send the request immediately
    status->lastStepTimestampRaw += 0x8000000UL;
    } else {
        static uint8_t c;
        debugindex(8, ++c);
        if (c > 254) c=254;
    }
    --mutex;
}


static void config_reset(uint16_t *bytes_to_send)
{
    memset((uint8_t *)status->working_packet.buffer, 0, sizeof(UBXSentHeader_t) + sizeof(ubx_cfg_cfg_t));
    // mask LSB=ioPort|msgConf|infMsg|navConf|rxmConf|||||rinvConf|antConf|....|= MSB
    // ioPort=1, msgConf=2, infMsg=4, navConf=8, tpConf=0x10, sfdrConf=0x100, rinvConf=0x200, antConf=0x400
    // first: reset (permanent settings to default) all but rinv = e.g. owner name
    status->working_packet.message.payload.cfg_cfg.clearMask  = UBX_CFG_CFG_OP_RESET_SETTINGS;
    // then: don't store any current settings to permanent
    status->working_packet.message.payload.cfg_cfg.saveMask   = UBX_CFG_CFG_SETTINGS_NONE;
    // lastly: load (immediately start to use) all but rinv = e.g. owner name
    status->working_packet.message.payload.cfg_cfg.loadMask   = UBX_CFG_CFG_OP_RESET_SETTINGS;
    // all devices
    status->working_packet.message.payload.cfg_cfg.deviceMask = UBX_CFG_CFG_DEVICE_ALL;

    *bytes_to_send = prepare_packet((UBXSentPacket_t *)&status->working_packet, UBX_CLASS_CFG, UBX_ID_CFG_CFG, sizeof(ubx_cfg_cfg_t));
}


// set the GPS baud rate to the user specified baud rate
// because we may have started up with 9600 baud (for a GPS with no permanent settings)
//static void config_gps_baud(uint16_t *bytes_to_send, uint8_t baud, uint16_t outProtoMask)
static void config_gps_baud(uint16_t *bytes_to_send)
{
    memset((uint8_t *)status->working_packet.buffer, 0, sizeof(UBXSentHeader_t) + sizeof(ubx_cfg_prt_t));
    status->working_packet.message.payload.cfg_prt.mode   = UBX_CFG_PRT_MODE_DEFAULT; // 8databits, 1stopbit, noparity, and non-zero reserved
    status->working_packet.message.payload.cfg_prt.portID = 1; // 1 = UART1, 2 = UART2
    // for protocol masks, bit 0 is UBX enable, bit 1 is NMEA enable
    status->working_packet.message.payload.cfg_prt.inProtoMask  = 1; // 1 = UBX only (bit 0)
    // disable current UBX messages for low baud rates
//    status->working_packet.message.payload.cfg_prt.outProtoMask = (hwsettings_baud<=HWSETTINGS_GPSSPEED_9600) ? 0 : 1; // 1 = UBX only (bit 0)
    status->working_packet.message.payload.cfg_prt.outProtoMask = 1;
    // Ask GPS to change it's speed
    status->working_packet.message.payload.cfg_prt.baudRate = hwsettings_gpsspeed_enum_to_baud(hwsettings_baud);
    *bytes_to_send = prepare_packet((UBXSentPacket_t *)&status->working_packet, UBX_CLASS_CFG, UBX_ID_CFG_PRT, sizeof(ubx_cfg_prt_t));
}


static void config_rate(uint16_t *bytes_to_send)
{
    memset((uint8_t *)status->working_packet.buffer, 0, sizeof(UBXSentHeader_t) + sizeof(ubx_cfg_rate_t));
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

    *bytes_to_send = prepare_packet((UBXSentPacket_t *)&status->working_packet, UBX_CLASS_CFG, UBX_ID_CFG_RATE, sizeof(ubx_cfg_rate_t));
}


static void config_nav(uint16_t *bytes_to_send)
{
    memset((uint8_t *)status->working_packet.buffer, 0, sizeof(UBXSentHeader_t) + sizeof(ubx_cfg_nav5_t));
    status->working_packet.message.payload.cfg_nav5.dynModel = status->currentSettings.dynamicModel;
    status->working_packet.message.payload.cfg_nav5.fixMode  = 2; // 1=2D only, 2=3D only, 3=Auto 2D/3D
    // mask LSB=dyn|minEl|posFixMode|drLim|posMask|statisticHoldMask|dgpsMask|......|reservedBit0 = MSB
    status->working_packet.message.payload.cfg_nav5.mask     = 0x01 + 0x04; // Dyn Model | posFixMode configuration

    *bytes_to_send = prepare_packet((UBXSentPacket_t *)&status->working_packet, UBX_CLASS_CFG, UBX_ID_CFG_NAV5, sizeof(ubx_cfg_nav5_t));
}


static void config_sbas(uint16_t *bytes_to_send)
{
    memset((uint8_t *)status->working_packet.buffer, 0, sizeof(UBXSentHeader_t) + sizeof(ubx_cfg_sbas_t));
    status->working_packet.message.payload.cfg_sbas.maxSBAS =
        status->currentSettings.SBASChannelsUsed < 4 ? status->currentSettings.SBASChannelsUsed : 3;
    status->working_packet.message.payload.cfg_sbas.usage   =
        (status->currentSettings.SBASCorrection ? UBX_CFG_SBAS_USAGE_DIFFCORR : 0) |
        (status->currentSettings.SBASIntegrity ? UBX_CFG_SBAS_USAGE_INTEGRITY : 0) |
        (status->currentSettings.SBASRanging ? UBX_CFG_SBAS_USAGE_RANGE : 0);
    // If sbas is used for anything then set mode as enabled
    status->working_packet.message.payload.cfg_sbas.mode =
        status->working_packet.message.payload.cfg_sbas.usage != 0 ? UBX_CFG_SBAS_MODE_ENABLED : 0;
    status->working_packet.message.payload.cfg_sbas.scanmode1 =
        status->currentSettings.SBASSats == UBX_SBAS_SATS_WAAS ? UBX_CFG_SBAS_SCANMODE1_WAAS :
        status->currentSettings.SBASSats == UBX_SBAS_SATS_EGNOS ? UBX_CFG_SBAS_SCANMODE1_EGNOS :
        status->currentSettings.SBASSats == UBX_SBAS_SATS_MSAS ? UBX_CFG_SBAS_SCANMODE1_MSAS :
        status->currentSettings.SBASSats == UBX_SBAS_SATS_GAGAN ? UBX_CFG_SBAS_SCANMODE1_GAGAN :
        status->currentSettings.SBASSats == UBX_SBAS_SATS_SDCM ? UBX_CFG_SBAS_SCANMODE1_SDCM : UBX_SBAS_SATS_AUTOSCAN;
    status->working_packet.message.payload.cfg_sbas.scanmode2 =
        UBX_CFG_SBAS_SCANMODE2;

    *bytes_to_send = prepare_packet((UBXSentPacket_t *)&status->working_packet, UBX_CLASS_CFG, UBX_ID_CFG_SBAS, sizeof(ubx_cfg_sbas_t));
}


static void config_gnss(uint16_t *bytes_to_send)
{
    memset((uint8_t *)status->working_packet.buffer, 0, sizeof(UBXSentHeader_t) + sizeof(ubx_cfg_gnss_t));
    status->working_packet.message.payload.cfg_gnss.numConfigBlocks = UBX_GNSS_ID_MAX;
    status->working_packet.message.payload.cfg_gnss.numTrkChHw = (ubxHwVersion > UBX_HW_VERSION_7) ? UBX_CFG_GNSS_NUMCH_VER8 : UBX_CFG_GNSS_NUMCH_VER7;
    status->working_packet.message.payload.cfg_gnss.numTrkChUse     = status->working_packet.message.payload.cfg_gnss.numTrkChHw;

    for (int32_t i = 0; i < UBX_GNSS_ID_MAX; i++) {
        status->working_packet.message.payload.cfg_gnss.cfgBlocks[i].gnssId = i;
        switch (i) {
        case UBX_GNSS_ID_GPS:
            if (status->currentSettings.enableGPS) {
                status->working_packet.message.payload.cfg_gnss.cfgBlocks[i].flags    = UBX_CFG_GNSS_FLAGS_ENABLED | UBX_CFG_GNSS_FLAGS_GPS_L1CA;
                status->working_packet.message.payload.cfg_gnss.cfgBlocks[i].maxTrkCh = 16;
                status->working_packet.message.payload.cfg_gnss.cfgBlocks[i].resTrkCh = 8;
            }
            break;
        case UBX_GNSS_ID_QZSS:
            if (status->currentSettings.enableGPS) {
                status->working_packet.message.payload.cfg_gnss.cfgBlocks[i].flags    = UBX_CFG_GNSS_FLAGS_ENABLED | UBX_CFG_GNSS_FLAGS_QZSS_L1CA;
                status->working_packet.message.payload.cfg_gnss.cfgBlocks[i].maxTrkCh = 3;
                status->working_packet.message.payload.cfg_gnss.cfgBlocks[i].resTrkCh = 0;
            }
            break;
        case UBX_GNSS_ID_SBAS:
            if (status->currentSettings.SBASCorrection || status->currentSettings.SBASIntegrity || status->currentSettings.SBASRanging) {
                status->working_packet.message.payload.cfg_gnss.cfgBlocks[i].flags    = UBX_CFG_GNSS_FLAGS_ENABLED | UBX_CFG_GNSS_FLAGS_SBAS_L1CA;
                status->working_packet.message.payload.cfg_gnss.cfgBlocks[i].maxTrkCh = status->currentSettings.SBASChannelsUsed < 4 ? status->currentSettings.SBASChannelsUsed : 3;
                status->working_packet.message.payload.cfg_gnss.cfgBlocks[i].resTrkCh = 1;
            }
            break;
        case UBX_GNSS_ID_GLONASS:
            if (status->currentSettings.enableGLONASS) {
                status->working_packet.message.payload.cfg_gnss.cfgBlocks[i].flags    = UBX_CFG_GNSS_FLAGS_ENABLED | UBX_CFG_GNSS_FLAGS_GLONASS_L1OF;
                status->working_packet.message.payload.cfg_gnss.cfgBlocks[i].maxTrkCh = 14;
                status->working_packet.message.payload.cfg_gnss.cfgBlocks[i].resTrkCh = 8;
            }
            break;
        case UBX_GNSS_ID_BEIDOU:
            if (status->currentSettings.enableBeiDou) {
                status->working_packet.message.payload.cfg_gnss.cfgBlocks[i].flags    = UBX_CFG_GNSS_FLAGS_ENABLED | UBX_CFG_GNSS_FLAGS_BEIDOU_B1I;
                status->working_packet.message.payload.cfg_gnss.cfgBlocks[i].maxTrkCh = 14;
                status->working_packet.message.payload.cfg_gnss.cfgBlocks[i].resTrkCh = 8;
            }
            break;
        default:
            break;
        }
    }

    *bytes_to_send = prepare_packet((UBXSentPacket_t *)&status->working_packet, UBX_CLASS_CFG, UBX_ID_CFG_GNSS, sizeof(ubx_cfg_gnss_t));
}


static void config_save(uint16_t *bytes_to_send)
{
    memset((uint8_t *)status->working_packet.buffer, 0, sizeof(UBXSentHeader_t) + sizeof(ubx_cfg_cfg_t));
    // mask LSB=ioPort|msgConf|infMsg|navConf|rxmConf|||||rinvConf|antConf|....|= MSB
    // ioPort=1, msgConf=2, infMsg=4, navConf=8, tpConf=0x10, sfdrConf=0x100, rinvConf=0x200, antConf=0x400
    status->working_packet.message.payload.cfg_cfg.saveMask   = UBX_CFG_CFG_OP_STORE_SETTINGS; // a list of settings we just set
    status->working_packet.message.payload.cfg_cfg.clearMask  = UBX_CFG_CFG_OP_CLEAR_SETTINGS; // everything else gets factory default
    status->working_packet.message.payload.cfg_cfg.deviceMask = UBX_CFG_CFG_DEVICE_ALL;

    *bytes_to_send = prepare_packet((UBXSentPacket_t *)&status->working_packet, UBX_CLASS_CFG, UBX_ID_CFG_CFG, sizeof(ubx_cfg_cfg_t));
}


static void configure(uint16_t *bytes_to_send)
{
    switch (status->lastConfigSent) {
    case LAST_CONFIG_SENT_START:
        // increase message rates to 5 fixes per second
        config_rate(bytes_to_send);
        break;

    case LAST_CONFIG_SENT_START + 1:
        config_nav(bytes_to_send);
        break;

    case LAST_CONFIG_SENT_START + 2:
        if (status->currentSettings.enableGLONASS || status->currentSettings.enableGPS) {
            config_gnss(bytes_to_send);
            break;
        } else {
            // Skip and fall through to next step
            status->lastConfigSent++;
        }
    // in the else case we must fall through because we must send something each time because successful send is tested externally

    case LAST_CONFIG_SENT_START + 3:
        config_sbas(bytes_to_send);
        break;

    default:
        status->lastConfigSent = LAST_CONFIG_SENT_COMPLETED;
        break;
    }
}


static void enable_sentences(__attribute__((unused)) uint16_t *bytes_to_send)
{
    int8_t msg = status->lastConfigSent + 1;
    uint8_t msg_count = (ubxHwVersion >= UBX_HW_VERSION_7) ?
                        NELEMENTS(msg_config_ubx7) : NELEMENTS(msg_config_ubx6);
    ubx_cfg_msg_t *msg_config = (ubxHwVersion >= UBX_HW_VERSION_7) ?
                                &msg_config_ubx7[0] : &msg_config_ubx6[0];

    if (msg >= 0 && msg < msg_count) {
        status->working_packet.message.payload.cfg_msg = msg_config[msg];
        *bytes_to_send = prepare_packet((UBXSentPacket_t *)&status->working_packet, UBX_CLASS_CFG, UBX_ID_CFG_MSG, sizeof(ubx_cfg_msg_t));
    } else {
        status->lastConfigSent = LAST_CONFIG_SENT_COMPLETED;
    }
}


// permanently store our version of GPSSettings.UbxAutoConfig
// we use this to disable after ConfigStoreAndDisable is complete
static void setGpsSettings()
{
    // try casting it correctly and it says:
    // expected 'struct GPSSettingsData *' but argument is of type 'struct GPSSettingsData *'
    // probably a volatile or align issue
    GPSSettingsGet((void *) &status->gpsSettings);
    status->gpsSettings.UbxAutoConfig = status->currentSettings.UbxAutoConfig;
    GPSSettingsSet((void *) &status->gpsSettings);
}


// End User Documentation

// There are two baud rates of interest
// The baud rate the GPS is talking at
// The baud rate Revo is talking at
// These two must match for the GPS to work
// You only have direct control of the Revo baud rate
// The two baud rates must be the same for the Revo to send a command to the GPS
// to tell the GPS to change it's baud rate
// So you start out by changing Revo's baud rate to match the GPS's
// and then enable UbxAutoConfig to tell Revo to change the GPS baud every time, just before it changes the Revo baud
// That is the basis of these instructions

// There are microprocessors and they each have internal settings
// Revo
// GPS
// and each of these settings can be temporary or permanent

// To change a Revo setting
// Use the System tab in the GCS for all the following
// Example: in Settings->GPSSettings click on the VALUE for UbxAutoConfig and change it to Disabled
// Click on UbxAutoConfig itself and the line will turn green and blue
// To change this setting permanently, press the  red  up arrow (Save) at the top of the screen
// Permanently means that it uses this setting, even if you reboot Revo, e.g. power off and on
// To change this setting temporarily, press the green up arrow (Send) at the top of the screen
// Temporarily means that it overrides the permanent setting, but it goes back to the permanent setting when you reboot Revo, e.g. power off and on

// To change an internal GPS setting you use the OP GCS System tab to tell Revo to make the GPS changes
// This only works correctly after you have matching baud rates so Revo and GPS can talk together
// "Settings->GPSSettings->UbxAutoConfig = Configure"         sets the internal GPS setting temporarily
// "Settings->GPSSettings->UbxAutoConfig = ConfigureAndStore" sets the internal GPS setting permanently

// You want to wind up with a set of permanent settings that work together
// There are two different sets of permanent settings that work together
// GPS at 9600 baud and factory defaults
// Revo configured to start out at 9600 baud, but then completely configure the GPS and switch both to 57600 baud
// (takes 6 seconds at boot up while you are waiting for it to acquire satellites anyway)
// This is the preferred way so that if we change the settings in the future, the new release will automatically use the correct settings
// GPS at 57600 baud with all the settings for the current release stored in the GPS
// Revo configured to disable UbxAutoConfig since all the GPS settings are permanently stored correctly
// May require reconfiguring in a future release

// Changable settings of interest
// AutoConfig mode
// Settings->GPSSettings->UbxAutoConfig (Disabled, Configure, ConfigureAndStore, default=Configure)
// Disabled means that changes to the GPS baud setting only affect the Revo port
// It doesn't try to change the GPS's internal baud rate setting
// Configure means change the GPS's internal baud setting temporarily (GPS settings revert to the permanent values when GPS is powered off/on)
// ConfigureAndStore means change the GPS's internal baud setting permanently (even after the GPS is powered off/on)
// GPS baud rate
// Settings->HwSettings->GPSSpeed
// If the baud rates are the same and an AutoConfig mode is enabled this will change both the GPS baud rate and the Revo baud rate
// If the baud rates are not the same and an AutoConfig mode is enabled it will fail
// If AutoConfig mode is disabled this will only change the Revo baud rate

// View only settings of interest
// Detected GPS type
// Data Objects -> GPSPositionSensor -> SensorType (Unknown, NMEA, UBX, UBX7, UBX8)
// When it says something other than Unknown, the GPS and Revo baud rates are synced and talking
// Real time progress of the GPS detection process
// Data Objects -> GPSPositionSensor -> AutoConfigStatus (DISABLED, RUNNING, DONE, ERROR)

// Syncing the baud rates means that the GPS's internal baud rate setting is the same as the Revo port setting
// This is necessary for the GPS to work with Revo
// To sync to and find out an unknown GPS baud rate (or sync to and use a known GPS baud rate)
// Temporarily change the AutoConfig mode to Disabled
// Temporarily change the GPS baud rate to a value you think it might be (or go up the list)
// See if that baud rate is correct (Data Objects->GPSPositionSensor->SensorType will be something besides Unknown)
// Repeat, changing the GPS baud rate, until found

// Some very important facts:
// For 9600 baud or lower, the autoconfig will configure it to factory default settings
// For 19200 baud or higher, the autoconfig will configure it to OP required settings
// If autoconfig is enabled permanently in Revo, it will assume that the GPS is configured to power up at 9600 baud
// 57600 baud is recommended for the current release
// That can be achieved either by
// autoconfiging the GPS from a permanent 9600 baud (and factory settings) to a temporary 57600 (with OP settings) on each power up
// or by configuring the GPS with a permanent 57600 (with OP settings) and then permanently disabling autoconfig
// Some previous releases used 38400 and had some other settings differences

// The user should either:
// Permanently configure their GPS to 9600 baud factory settings and tell the Revo configuration to load volatile settings at each startup by:
// (Recommended method because new versions could require new settings and this handles future changes automatically)
// Syncing the baud rates
// Setting it to autoconfig.nostore and waiting for it to complete
// Setting HwSettings.GPSSpeed to 9600 and waiting for it to complete
// Setting it to autoconfig.store and waiting for it to complete (this tells the GPS to store the 9600 permanently)
// Permanently setting it to autoconfig.nostore and waiting for it to complete
// Permanently setting HwSettings.GPSSpeed to 57600 and waiting for it to complete
// Permanently configure their GPS to 57600 baud, including OpenPilot settings and telling the Revo configuration to just set the baud to 57600 at each startup by:
// (Less recommended method because new versions could require new settings so you would have to do this again)
// Syncing the baud rates
// Setting it to autoconfig.nostore and waiting for it to complete
// Permanently setting HwSettings.GPSSpeed to 57600 and waiting for it to complete
// Setting it to autoconfig.store
// Permanently setting it to autoconfig.disabled

// The algorithm is:
// If autoconfig is enabled at all
// It will assume that the GPS boot up baud rate is 9600 and the user wants that changed to HwSettings.GPSSpeed
// and that change can be either volatile (must be done each boot up) or non-volatile (stored in GPS's non-volatile settings storage)
// according to whether CONFIGURE is used or CONFIGUREANDSTORE is used
// The only user who should need CONFIGUREANDSTORE stored permanently in Revo is Dave, who configures many OP GPS's before shipping
// plug a factory default GPS in to a Revo, power up, wait for it to configure and permanently store in the GPS, power down, ship
// If autoconfig is not enabled
// it will use HwSettings.GPSSpeed for the baud rate and not do any configuration changes
// If GPSSettings.UbxAutoConfig == GPSSETTINGS_UBXAUTOCONFIG_CONFIGUREANDSTORE it will
// 1 Reset the permanent configuration back to factory default
// 2 Disable NMEA message settings
// 3 Add some volatile UBX settings to the copies of the non-volatile ones that are currently running
// 4 Save the current volatile settings to non-volatile storage
// If GPSSettings.UbxAutoConfig == GPSSETTINGS_UBXAUTOCONFIG_CONFIGURE it will
// 2 Disable NMEA message settings
// 3 Add some volatile UBX settings to the copies of the non-volatile ones that are currently running
// If the requested baud rate is 9600 or less it skips the step (3) of adding some volatile UBX settings

// Talking points to point out:
// U-center is no longer needed for any use case with this code
// 9600 is factory default for GPS's
// Some GPS can't even permanently store settings and must start at 9600 baud?
// I have a GPS that sometimes looses settings and reverts to 9600 and this is a fix for that too :)
// This code handles a GPS configured either way (9600 with factory default settings or e.g. 57600 with OP settings)
// Autoconfig.nostore at each boot for 9600, autoconfig.disabled for the 57600 with OP settings (or custom settings and baud)
// This code can permanently configure a GPS to be e.g. 9600 with factory default settings or 57600 with OP settings
// GPS's with 9600 baud and factory default settings would be a good default for future OP releases
// Changing the GPS internal settings multiple times in the future is handled automatically
// This code is written to do a configure from 9600 to 57600
// (actually 9600 to whatever is stored in HwSettings.GPSSpeed)
// if autoconfig is enabled at boot up
// When autoconfiging to 9600 baud or lower, the autoconfig will configure it to factory default settings, not OP settings
// That is because 9600 baud drops many of the OP messages and because 9600 baud is factory default
// For 19200 baud or higher, the autoconfig will configure it to OP required settings
// If autoconfig is enabled permanently in Revo, it will assume that the GPS is configured to power up at 9600 baud
// This is good for factory default GPS's
// This is good in case we change some settings in a future release


void gps_ubx_autoconfig_run(char * *buffer, uint16_t *bytes_to_send)
{
    *bytes_to_send = 0;
    *buffer = (char *)status->working_packet.buffer;
    current_step_touched = false;

    // autoconfig struct not yet allocated
    if (!status) {
        return;
    }

    // get UBX version whether autoconfig is enabled or not
    // this allows the user to try some baud rates and visibly see when it works
    // ubxHwVersion is a global set externally by the caller of this function
    if (ubxHwVersion <= 0) {
        // at low baud rates and high data rates the ubx gps simply must drop some outgoing data
        // this isn't really an error
        // and when a lot of data is being dropped, the MON VER reply often gets dropped
        // on the other hand, uBlox documents that some versions discard data that is over 1 second old
        // implying a 1 second send buffer and that it could be over 1 second before a reply is received
        // later uBlox versions dropped this 1 second constraint and drop data when the send buffer is full
        // and that could be even longer than 1 second
        // send this more quickly and it will get a reply more quickly if a fixed percentage of replies are being dropped

        // wait for the normal reply timeout before sending it over and over
        if (PIOS_DELAY_DiffuS(status->lastStepTimestampRaw) < UBX_PARSER_TIMEOUT) {
// 1 sec seems to work, except for just 230400 baud rates and then it takes 4 times
// retries for various timeouts
// 925-280-280 -> 2@2400
// 937-280-280 -> 1@2400
            return;
        }

// this is where the autobaud code needs to go as it won't get farther till MON_VER returns something
// at this point we have already waited for the MON_VER reply to time out (except the first time where it times out without being sent)
// and the fact we are here says that ubxHwVersion has not been set (it is set externally)
// so this retry at this baud rate has failed
// we need gps_ubx_reset_sensor_type() (or what else?) to reset this FSM (baud_to_try_index==-1) and the retry count
// need a state machine that counts retries and:
//   no, no retries, just loop through the baud rate over and over till it locks
//   that may be a bit slower if it misses just one MON_VER than trying twice on the first few (sorted in likelyhood order) baud rates
//   but it gets to the other baud rates faster and loops faster

// if we get here
//   select the next baud rate, skipping ahead if new baud rate is HwSettings.GPSSpeed
//   set Revo baud rate to current++ value (immediate change so we can send right after that) and send the MON_VER request

// baud rate search order (most likely matches first)
// HwSettings.GPSSpeed, 57600, 9600, 38400, 115200, 
// all values from #2 on are only tried if GPSSpeed!=them
// no, init to HwSettings.GPSSpeed only at power up and if it fails just run the whole list over and over?

// the order to try the baud rates with the most likely first
// these come after trying HwSettings.GPSSpeed but will not duplicate the HwSettings.GPSSpeed test

// without an if for this block, it autobauds even with autoconfig disabled
// if (status->currentSettings.UbxAutoConfig != GPSSETTINGS_UBXAUTOCONFIG_DISABLED)
// this is the same thing
// if (enabled)

        // if not Disabled, do AutoBaud
        if (status->currentSettings.UbxAutoConfig >= GPSSETTINGS_UBXAUTOCONFIG_AUTOBAUD) {
            uint8_t baud_to_try;
            //HwSettingsGPSSpeedOptions  baud_to_try;
#if 1
            static uint8_t baud_array[] = {
                HWSETTINGS_GPSSPEED_57600,
                HWSETTINGS_GPSSPEED_9600,
                HWSETTINGS_GPSSPEED_115200,
                HWSETTINGS_GPSSPEED_38400,
                HWSETTINGS_GPSSPEED_19200,
                HWSETTINGS_GPSSPEED_230400,
                HWSETTINGS_GPSSPEED_4800,
                HWSETTINGS_GPSSPEED_2400
            };

            do {
                // index is inited to be out of bounds, which is interpreted as "currently defined baud rate" (= HwSettigns.GPSSpeed)
                if (baud_to_try_index >= sizeof(baud_array)/sizeof(baud_array[0])) {
                    HwSettingsGPSSpeedGet(&hwsettings_baud);
                    baud_to_try = hwsettings_baud;
                    baud_to_try_index = 0;
                    break;
                } else {
                    baud_to_try = baud_array[baud_to_try_index++];
                }
            // skip HwSettings.GPSSpeed when you run across it in the list
            } while (baud_to_try == hwsettings_baud);
#else
            // no autobaud, just use current setting
            HwSettingsGPSSpeedGet(&hwsettings_baud);
            baud_to_try = hwsettings_baud;
#if 0
debugindex(0, hwsettings_baud);
{
static uint8_t c;
debugindex(1, ++c);
}
#endif
#endif
            // set the FC (Revo) baud rate
            gps_set_fc_baud_from_arg(baud_to_try, 2);
        }

{
static uint8_t c;
debugindex(4, ++c);
}

        // this code is executed even if ubxautoconfig is disabled
        // it detects the "sensor type" = type of GPS
        // the user can use this to manually determine if the baud rate is correct
        build_request((UBXSentPacket_t *)&status->working_packet, UBX_CLASS_MON, UBX_ID_MON_VER, bytes_to_send);
        // keep timeouts running properly, we (will have) just sent a packet that generates a reply
        status->lastStepTimestampRaw = PIOS_DELAY_GetRaw();
        return;
    }

#if 0
    {
        static uint8_t c;
        uint8_t baud;
        GPSPositionSensorCurrentBaudRateGet(&baud);
        debugindex(8, baud);
        if (baud != hwsettings_baud) {
            debugindex(9, ++c);
// big hack to keep the malfunctioning GPSPositionSensor.CurrentBaudRate correct
// for some reason it keeps going back to boot value (or default if xml has a default)
            GPSPositionSensorCurrentBaudRateSet(&baud);
        }
    }
#endif

    if (!enabled) {
        // keep resetting the timeouts here if we are not actually going to run the configure code
        // not really necessary, but it keeps the timer from wrapping every 50 seconds
        status->lastStepTimestampRaw = PIOS_DELAY_GetRaw();
        return; // autoconfig not enabled
    }

debugindex(5, status->currentStep);
    switch (status->currentStep) {
#if 0
    // if here, we have just verified that the baud rates are in sync
    case INIT_STEP_START:
        // we should look for the GPS version again (user may plug in a different GPS and then do autoconfig again)
//gps_ubx_reset_sensor_type();
        // do not fall through to next state
        // or it might try to get the sensor type when the baud rate is half changed
        // note that resetting sensor type causes it to send MON_VER msg and wait for reply before coming back here
#if 0
        set_current_step_if_untouched(INIT_STEP_DISABLE_MSGS);
        // allow it to get the sensor type immmediately by not setting status->lastStepTimestampRaw = PIOS_DELAY_GetRaw();
        break;

    case INIT_STEP_DISABLE_MSGS:
        config_gps_baud(bytes_to_send, baud_to_try, 0);
        // do not fall through to next state
        // or it might try to get the sensor type when the baud rate is half changed
        // note that resetting sensor type causes it to send MON_VER msg and wait for reply before coming back here
#endif
        set_current_step_if_untouched(INIT_STEP_RESET_GPS);
        status->lastStepTimestampRaw = PIOS_DELAY_GetRaw();
        break;
#endif
    // if here, we have verified that the baud rates are in sync sometime in the past
    case INIT_STEP_START:
        // we should look for the GPS version again (user may plug in a different GPS and then do autoconfig again)
        // zero retries for the next state that needs it (INIT_STEP_SAVE)
// not needed if MON_VER ack is a nop
//        status->retryCount = 0;
        set_current_step_if_untouched(INIT_STEP_SEND_MON_VER);
        // fall through to next state
        // we can do that if we choose because we haven't sent any data in this state
        // break;

    case INIT_STEP_SEND_MON_VER:
        build_request((UBXSentPacket_t *)&status->working_packet, UBX_CLASS_MON, UBX_ID_MON_VER, bytes_to_send);
        // keep timeouts running properly, we (will have) just sent a packet that generates a reply
        set_current_step_if_untouched(INIT_STEP_WAIT_MON_VER_ACK);
// just for test, remove
// it messes configure up, config has baud synced, and HwSettings.GPSSpeed is already set with new baud,
//   so it configures to new baud before GPS has new baud
// gps_ubx_reset_sensor_type();
        status->lastStepTimestampRaw = PIOS_DELAY_GetRaw();
        break;

    case INIT_STEP_WAIT_MON_VER_ACK:
#if 0
debugindex(0, status->requiredAck.clsID);
debugindex(1, ubxLastAck.clsID);
debugindex(2, status->requiredAck.msgID);
debugindex(3, ubxLastAck.msgID);
debugindex(4, ubxLastNak.clsID);
debugindex(5, ubxLastNak.msgID);
#endif
#if 0
        if (ubxLastAck.clsID == status->requiredAck.clsID && ubxLastAck.msgID == status->requiredAck.msgID) {
            // Continue with next configuration option
            set_current_step_if_untouched(INIT_STEP_RESET_GPS);
            // note that we increase the reply timeout in case the GPS must do a flash erase
        } else if (PIOS_DELAY_DiffuS(status->lastStepTimestampRaw) < UBX_REPLY_TIMEOUT &&
                   (ubxLastNak.clsID != status->requiredAck.clsID || ubxLastNak.msgID != status->requiredAck.msgID)) {
            // allow timeouts to count up by not setting status->lastStepTimestampRaw = PIOS_DELAY_GetRaw();
            break;
        } else {
            // timeout or NAK, resend the message or abort
            status->retryCount++;
            if (status->retryCount > UBX_MAX_RETRIES) {
                // give up on the retries
                set_current_step_if_untouched(INIT_STEP_PRE_ERROR);
                status->lastStepTimestampRaw = PIOS_DELAY_GetRaw();
            } else {
                // retry a few times
                set_current_step_if_untouched(INIT_STEP_SEND_MON_VER);
            }
        }
        break;
#else
        // wait for previous step
        // extra wait time might well be unnecessary but we want to make sure
        // that we don't stop waiting too soon
        if (PIOS_DELAY_DiffuS(status->lastStepTimestampRaw) < UBX_UNVERIFIED_STEP_WAIT_TIME) {
            return;
        }
        // Continue with next configuration option
        set_current_step_if_untouched(INIT_STEP_RESET_GPS);
        // fall through to next state
        // we can do that if we choose because we haven't sent any data in this state
        // break;
#endif

    // if here, we have just verified that the baud rates are in sync (again)
    case INIT_STEP_RESET_GPS:
        // make sure we don't change the baud rate too soon and garble the packet being sent
        // even after pios says the buffer is empty, the serial port buffer still has data in it
        // and changing the baud will screw it up
        // when the GPS is configured to send a lot of data, but has a low baud rate
        // it has way too many messages to send and has to drop most of them

        // Retrieve desired GPS baud rate once for use throughout this module
        HwSettingsGPSSpeedGet(&hwsettings_baud);
#if 0
debugindex(2, hwsettings_baud);
{
static uint8_t c;
debugindex(3, ++c);
}
#endif
//move this up higher / earlier?
#if !defined(ALWAYS_RESET)
        // ALWAYS_RESET is undefined because it causes stored settings to change even with autoconfig.nostore
        // but with it off, some settings may be enabled that should really be disabled (but aren't) after autoconfig.nostore
        // if user requests a low baud rate then we just reset and leave it set to NMEA
        // because low baud and high OP data rate doesn't play nice
        // if user requests that settings be saved, we will reset here too
        // that makes sure that all strange settings are reset to factory default
        // else these strange settings may persist because we don't reset all settings by table
        if (status->currentSettings.UbxAutoConfig == GPSSETTINGS_UBXAUTOCONFIG_ABCONFIGANDSTORE
            || status->currentSettings.UbxAutoConfig == GPSSETTINGS_UBXAUTOCONFIG_ABCONFIGSTOREANDDISABLE)
#endif
        {
            // reset all GPS parameters to factory default (configure low rate NMEA for low baud rates)
            // this is not usable by OP code for either baud rate or types of messages sent
            // but it starts up very quickly for use with autoconfig-nostore (which sets a high baud and enables all the necessary messages)
            config_reset(bytes_to_send);
            status->lastStepTimestampRaw = PIOS_DELAY_GetRaw();
        }
        // else allow it enter the next state immmediately by not setting status->lastStepTimestampRaw = PIOS_DELAY_GetRaw();
        set_current_step_if_untouched(INIT_STEP_REVO_9600_BAUD);
        break;

    // GPS was just reset, so GPS is running 9600 baud, and Revo is running whatever baud it was before
    case INIT_STEP_REVO_9600_BAUD:
#if !defined(ALWAYS_RESET)
        // if user requests a low baud rate then we just reset and leave it set to NMEA
        // because low baud and high OP data rate doesn't play nice
        // if user requests that settings be saved, we will reset here too
        // that makes sure that all strange settings are reset to factory default
        // else these strange settings may persist because we don't reset all settings by hand
        if (status->currentSettings.UbxAutoConfig == GPSSETTINGS_UBXAUTOCONFIG_ABCONFIGANDSTORE
            || status->currentSettings.UbxAutoConfig == GPSSETTINGS_UBXAUTOCONFIG_ABCONFIGSTOREANDDISABLE)
#endif
        {
            // wait for previous step
            // extra wait time might well be unnecessary but we want to make very sure
            // that we don't stop waiting too soon as that could leave us at an unknown baud rate
            // (i.e. set or not set) if the the transmit buffer was full and we were running at a low baud rate
            if (PIOS_DELAY_DiffuS(status->lastStepTimestampRaw) < UBX_UNVERIFIED_STEP_WAIT_TIME) {
                return;
            }
#if 0
debugindex(8, hwsettings_baud);
if (hwsettings_baud >= 8) {
debugindex(9, hwsettings_baud);
}
#endif
            // set the Revo GPS port to 9600 baud to match the reset to factory default that has already been done
            gps_set_fc_baud_from_arg(HWSETTINGS_GPSSPEED_9600, 0);
        }
        // at most, we just set Revo baud and that doesn't send any data
        // fall through to next state
        // we can do that if we choose because we haven't sent any data in this state
        // set_current_step_if_untouched(INIT_STEP_GPS_BAUD);
        // allow it enter the next state immmediately by not setting status->lastStepTimestampRaw = PIOS_DELAY_GetRaw();
        // break;

    // Revo and GPS are both at 9600 baud
    case INIT_STEP_GPS_BAUD:
        // https://www.u-blox.com/images/downloads/Product_Docs/u-bloxM8_ReceiverDescriptionProtocolSpec_%28UBX-13003221%29_Public.pdf
        // It is possible to change the current communications port settings using a UBX-CFG-CFG message. This could
        // affect baud rate and other transmission parameters. Because there may be messages queued for transmission
        // there may be uncertainty about which protocol applies to such messages. In addition a message currently in
        // transmission may be corrupted by a protocol change. Host data reception parameters may have to be changed to
        // be able to receive future messages, including the acknowledge message associated with the UBX-CFG-CFG message.

        // so the message that changes the baud rate will send it's acknowledgement back at the new baud rate; this is not good.
        // if your message was corrupted, you didn't change the baud rate and you have to guess; try pinging at both baud rates.
        // also, you would have to change the baud rate instantly after the last byte of the sentence was sent,
        // and you would have to poll the port in real time for that, and there may be messages ahead of the baud rate change.
        //
        // so we ignore the ack from this.  it has proven to be reliable (with the addition of two dummy bytes after the packet)

        // set the GPS internal baud rate to the user configured value
//        config_gps_baud(bytes_to_send, hwsettings_baud, 1);
        config_gps_baud(bytes_to_send);
        set_current_step_if_untouched(INIT_STEP_REVO_BAUD);
        status->lastStepTimestampRaw = PIOS_DELAY_GetRaw();
        break;

    // GPS is at final baud and Revo is at old baud (old is 9600 or initial detected baud)
    case INIT_STEP_REVO_BAUD:
        // wait for previous step
        if (PIOS_DELAY_DiffuS(status->lastStepTimestampRaw) < UBX_UNVERIFIED_STEP_WAIT_TIME) {
            return;
        }
        // set the Revo GPS port baud rate to the (same) user configured value
#if 0
debugindex(4, hwsettings_baud);
{
static uint8_t c;
debugindex(5, ++c);
}
#endif
        gps_set_fc_baud_from_arg(hwsettings_baud, 3);
        status->lastConfigSent = LAST_CONFIG_SENT_START;
        // zero the retries for the first "enable sentence"
        status->retryCount     = 0;
        // skip enabling UBX sentences for low baud rates
        // low baud rates are not usable, and higher data rates just makes it harder for this code to change the configuration
        if (hwsettings_baud <= HWSETTINGS_GPSSPEED_9600) {
            set_current_step_if_untouched(INIT_STEP_SAVE);
        } else {
            set_current_step_if_untouched(INIT_STEP_ENABLE_SENTENCES);
        }
        // allow it enter the next state immmediately by not setting status->lastStepTimestampRaw = PIOS_DELAY_GetRaw();
        break;

    case INIT_STEP_ENABLE_SENTENCES:
    case INIT_STEP_CONFIGURE:
    {
        bool step_configure = (status->currentStep == INIT_STEP_CONFIGURE);
        if (step_configure) {
            configure(bytes_to_send);
        } else {
            enable_sentences(bytes_to_send);
        }

        // for some branches, allow it enter the next state immmediately by not setting status->lastStepTimestampRaw = PIOS_DELAY_GetRaw();
        if (status->lastConfigSent == LAST_CONFIG_SENT_COMPLETED) {
            if (step_configure) {
                // zero retries for the next state that needs it (INIT_STEP_SAVE)
                status->retryCount = 0;
                set_current_step_if_untouched(INIT_STEP_SAVE);
            } else {
                // finished enabling sentences, now configure() needs to start at the beginning
                status->lastConfigSent = LAST_CONFIG_SENT_START;
                set_current_step_if_untouched(INIT_STEP_CONFIGURE);
            }
        } else {
            set_current_step_if_untouched(step_configure ? INIT_STEP_CONFIGURE_WAIT_ACK : INIT_STEP_ENABLE_SENTENCES_WAIT_ACK);
            status->lastStepTimestampRaw = PIOS_DELAY_GetRaw();
        }
        break;
    }

    case INIT_STEP_ENABLE_SENTENCES_WAIT_ACK:
    case INIT_STEP_CONFIGURE_WAIT_ACK: // Wait for an ack from GPS
    {
        bool step_configure = (status->currentStep == INIT_STEP_CONFIGURE_WAIT_ACK);
        if (ubxLastAck.clsID == status->requiredAck.clsID && ubxLastAck.msgID == status->requiredAck.msgID) {
            // Continue with next configuration option
            // start retries over for the next setting to be sent
            status->retryCount = 0;
            status->lastConfigSent++;
        } else if (PIOS_DELAY_DiffuS(status->lastStepTimestampRaw) < UBX_REPLY_TIMEOUT &&
                   (ubxLastNak.clsID != status->requiredAck.clsID || ubxLastNak.msgID != status->requiredAck.msgID)) {
            // allow timeouts to count up by not setting status->lastStepTimestampRaw = PIOS_DELAY_GetRaw();
            break;
        } else {
            // timeout or NAK, resend the message or abort
            status->retryCount++;
debugindex(0, status->currentStep);
debugindex(1, status->lastConfigSent);
debugindex(2, status->retryCount);
            if (status->retryCount > UBX_MAX_RETRIES) {
                set_current_step_if_untouched(INIT_STEP_PRE_ERROR);
                status->lastStepTimestampRaw = PIOS_DELAY_GetRaw();
                break;
            }
        }
        // success or failure here, retries are handled elsewhere
        if (step_configure) {
            set_current_step_if_untouched(INIT_STEP_CONFIGURE);
        } else {
            set_current_step_if_untouched(INIT_STEP_ENABLE_SENTENCES);
        }
        break;
    }

    // all configurations have been made
    case INIT_STEP_SAVE:
        // now decide whether to save them permanently into the GPS
        if (status->currentSettings.UbxAutoConfig == GPSSETTINGS_UBXAUTOCONFIG_ABCONFIGANDSTORE
            || status->currentSettings.UbxAutoConfig == GPSSETTINGS_UBXAUTOCONFIG_ABCONFIGSTOREANDDISABLE) {
            config_save(bytes_to_send);
            set_current_step_if_untouched(INIT_STEP_SAVE_WAIT_ACK);
            status->lastStepTimestampRaw = PIOS_DELAY_GetRaw();
        } else {
            set_current_step_if_untouched(INIT_STEP_PRE_DONE);
            // allow it enter INIT_STEP_PRE_DONE immmediately by not setting status->lastStepTimestampRaw = PIOS_DELAY_GetRaw();
        }
        break;

    // command to save configuration has already been issued
    case INIT_STEP_SAVE_WAIT_ACK:
        // save doesn't appear to respond, even in 24 seconds
        // just delay a while, in case there it is busy with a flash write, etc.
        if (PIOS_DELAY_DiffuS(status->lastStepTimestampRaw) < UBX_SAVE_WAIT_TIME) {
            return;
        }
        // fall through to next state
        // we can do that if we choose because we haven't sent any data in this state
// we actually must fall through because the FSM won't be entered if DONE, ERROR, or DISABLED
        set_current_step_if_untouched(INIT_STEP_PRE_DONE);
        // break;

    // the autoconfig has completed normally
    case INIT_STEP_PRE_DONE:
        // determine if we need to disable autoconfig via the autoconfig==CONFIGSTOREANDDISABLE setting
        if (status->currentSettings.UbxAutoConfig == GPSSETTINGS_UBXAUTOCONFIG_ABCONFIGSTOREANDDISABLE) {
            enabled = false;
            status->currentSettings.UbxAutoConfig = GPSSETTINGS_UBXAUTOCONFIG_DISABLED;
            // like it says
//            GPSSettingsUbxAutoConfigSet((GPSSettingsUbxAutoConfigOptions *) &status->currentSettings.UbxAutoConfig);
            setGpsSettings();
        }
        set_current_step_if_untouched(INIT_STEP_DONE);
        break;

    // an error, such as retries exhausted, has occurred
    case INIT_STEP_PRE_ERROR:
        // on error we should get the GPS version immediately
        gps_ubx_reset_sensor_type();
        set_current_step_if_untouched(INIT_STEP_ERROR);
        break;

    case INIT_STEP_DONE:
    case INIT_STEP_ERROR:
    case INIT_STEP_DISABLED:
        break;
    }
}


// this can be called from a different thread
// so everything it touches must be declared volatile
// I hope that I took care of all the concurrency issues that are likely to happen in the lifetime of OpenPilot
// by lowering the probability from 1 in 50,000 calls to this function (my SWAG) to one in 50,000**3 (forgive my Fortran)
void gps_ubx_autoconfig_set(ubx_autoconfig_settings_t *config)
{
    initSteps_t new_step;

    enabled = false;

    if (!status) {
        status = (status_t *)pios_malloc(sizeof(status_t));
        PIOS_Assert(status);
        memset((status_t *)status, 0, sizeof(status_t));
    }

    // if caller used NULL, just use current settings to restart autoconfig process
    if (config != NULL) {
        status->currentSettings = *config;
    }
    if (status->currentSettings.UbxAutoConfig >= GPSSETTINGS_UBXAUTOCONFIG_ABANDCONFIGURE) {
        new_step = INIT_STEP_START;
    } else {
        new_step = INIT_STEP_DISABLED;
    }
    status->lastStepTimestampRaw = PIOS_DELAY_GetRaw();

    // assume this one byte initSteps_t is atomic
    // take care of some but not all concurrency issues

    status->currentStep     = new_step;
    status->currentStepSave = new_step;
    current_step_touched    = true;
    status->currentStep     = new_step;
    status->currentStepSave = new_step;

    if (status->currentSettings.UbxAutoConfig >= GPSSETTINGS_UBXAUTOCONFIG_ABANDCONFIGURE) {
        // enabled refers to autoconfigure
        // note that sensor type (gps type) detection happens even if completely disabled
        // also note that AutoBaud is less than Configure
        enabled = true;
    } else {
        // this forces the sensor type detection to occur outside the FSM
        // and _can_ also engage the autobaud detection that is outside the FSM
        // don't do it if FSM is enabled as FSM can change the baud itself
        // (don't do it because the baud rates are already in sync)
        gps_ubx_reset_sensor_type();
    }
}


int32_t ubx_autoconfig_get_status()
{
    if (!status || !enabled) {
        return UBX_AUTOCONFIG_STATUS_DISABLED;
    }
    switch (status->currentStep) {
    case INIT_STEP_ERROR:
        return UBX_AUTOCONFIG_STATUS_ERROR;

    case INIT_STEP_DISABLED:
        return UBX_AUTOCONFIG_STATUS_DISABLED;

    case INIT_STEP_DONE:
        return UBX_AUTOCONFIG_STATUS_DONE;

    default:
        break;
    }
    return UBX_AUTOCONFIG_STATUS_RUNNING;
}

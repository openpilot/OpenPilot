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
#include "UBX.h"
#include <stdint.h>
#include <stdbool.h>

// defines
// TODO: NEO8 max rate is for Rom version, flash is limited to 10Hz, need to handle that.
#define UBX_MAX_RATE_VER8 18
#define UBX_MAX_RATE_VER7 10
#define UBX_MAX_RATE      5

#define UBX_REPLY_TIMEOUT (500 * 1000)
#define UBX_MAX_RETRIES   5

// types
typedef enum {
    UBX_AUTOCONFIG_STATUS_DISABLED = 0,
    UBX_AUTOCONFIG_STATUS_RUNNING,
    UBX_AUTOCONFIG_STATUS_DONE,
    UBX_AUTOCONFIG_STATUS_ERROR
} ubx_autoconfig_status_t;
// Enumeration options for field UBXDynamicModel
typedef enum {
    UBX_DYNMODEL_PORTABLE   = 0,
    UBX_DYNMODEL_STATIONARY = 2,
    UBX_DYNMODEL_PEDESTRIAN = 3,
    UBX_DYNMODEL_AUTOMOTIVE = 4,
    UBX_DYNMODEL_SEA = 5,
    UBX_DYNMODEL_AIRBORNE1G = 6,
    UBX_DYNMODEL_AIRBORNE2G = 7,
    UBX_DYNMODEL_AIRBORNE4G = 8
} ubx_config_dynamicmodel_t;

typedef enum {
    UBX_SBAS_SATS_AUTOSCAN = 0,
    UBX_SBAS_SATS_WAAS     = 1,
    UBX_SBAS_SATS_EGNOS    = 2,
    UBX_SBAS_SATS_MSAS     = 3,
    UBX_SBAS_SATS_GAGAN    = 4,
    UBX_SBAS_SATS_SDCM     = 5
} ubx_config_sats_t;

#define UBX_
typedef struct {
    bool    autoconfigEnabled;
    bool    storeSettings;

    bool    SBASRanging;
    bool    SBASCorrection;
    bool    SBASIntegrity;
    ubx_config_sats_t SBASSats;
    uint8_t SBASChannelsUsed;

    int8_t  navRate;
    ubx_config_dynamicmodel_t dynamicModel;
} ubx_autoconfig_settings_t;

// Mask for "all supported devices": battery backed RAM, Flash, EEPROM, SPI Flash
#define UBX_CFG_CFG_ALL_DEVICES_MASK (0x01 | 0x02 | 0x04 | 0x10)

// Sent messages for configuration support
typedef struct {
    uint32_t clearMask;
    uint32_t saveMask;
    uint32_t loadMask;
    uint8_t  deviceMask;
} __attribute__((packed)) ubx_cfg_cfg_t;

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
} __attribute__((packed)) ubx_cfg_nav5_t;

typedef struct {
    uint16_t measRate;
    uint16_t navRate;
    uint16_t timeRef;
} __attribute__((packed)) ubx_cfg_rate_t;

typedef struct {
    uint8_t msgClass;
    uint8_t msgID;
    uint8_t rate;
} __attribute__((packed)) ubx_cfg_msg_t;

#define UBX_CFG_SBAS_MODE_ENABLED    0x01
#define UBX_CFG_SBAS_MODE_TEST       0x02
#define UBX_CFG_SBAS_USAGE_RANGE     0x01
#define UBX_CFG_SBAS_USAGE_DIFFCORR  0x02
#define UBX_CFG_SBAS_USAGE_INTEGRITY 0x04

// SBAS used satellite PNR bitmask (120-151)
// -------------------------------------1---------1---------1---------1
// -------------------------------------5---------4---------3---------2
// ------------------------------------10987654321098765432109876543210
// WAAS 122, 133, 134, 135, 138---------|---------|---------|---------|
#define UBX_CFG_SBAS_SCANMODE1_WAAS  0b00000000000001001110000000000100
// EGNOS 120, 124, 126, 131-------------|---------|---------|---------|
#define UBX_CFG_SBAS_SCANMODE1_EGNOS 0b00000000000000000000100001010001
// MSAS 129, 137------------------------|---------|---------|---------|
#define UBX_CFG_SBAS_SCANMODE1_MSAS  0b00000000000000100000001000000000
// GAGAN 127, 128-----------------------|---------|---------|---------|
#define UBX_CFG_SBAS_SCANMODE1_GAGAN 0b00000000000000000000000110000000
// SDCM 125, 140, 141-------------------|---------|---------|---------|
#define UBX_CFG_SBAS_SCANMODE1_SDCM  0b00000000001100000000000000100000

#define UBX_CFG_SBAS_SCANMODE2       0x00
typedef struct {
    uint8_t  mode;
    uint8_t  usage;
    uint8_t  maxSBAS;
    uint8_t  scanmode2;
    uint32_t scanmode1;
} __attribute__((packed)) ubx_cfg_sbas_t;

typedef struct {
    uint8_t  prolog[2];
    uint8_t  class;
    uint8_t  id;
    uint16_t len;
} __attribute__((packed)) UBXSentHeader_t;

typedef union {
    uint8_t buffer[0];
    struct {
        UBXSentHeader_t header;
        union {
            ubx_cfg_cfg_t  cfg_cfg;
            ubx_cfg_msg_t  cfg_msg;
            ubx_cfg_nav5_t cfg_nav5;
            ubx_cfg_rate_t cfg_rate;
            ubx_cfg_sbas_t cfg_sbas;
        } payload;
        uint8_t resvd[2]; // added space for checksum bytes
    } message;
} __attribute__((packed)) UBXSentPacket_t;

void ubx_autoconfig_run(char * *buffer, uint16_t *bytes_to_send);
void ubx_autoconfig_set(ubx_autoconfig_settings_t config);
int32_t ubx_autoconfig_get_status();
#endif /* UBX_AUTOCONFIG_H_ */

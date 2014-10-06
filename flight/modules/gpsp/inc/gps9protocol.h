/**
 ******************************************************************************
 *
 * @file       gpsv9protocol.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2014.
 * @brief      brief goes here.
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
#ifndef GPSV9PROTOCOL_H_
#define GPSV9PROTOCOL_H_

#include <openpilot.h>
#include <pios_struct_helper.h>
#include <pios_helpers.h>
#include <ubx_utils.h>

#define UBX_CFG_CLASS                 0x06
#define UBX_CFG_PRT                   0x00
#define UBX_OP_CUST_CLASS             0x99
#define UBX_OP_SYS                    0x01
#define UBX_OP_MAG                    0x02


#define SYS_DATA_OPTIONS_FLASH        0x01
#define SYS_DATA_OPTIONS_MAG          0x02

#define CFG_PRT_DATA_PORTID_DDC       0x00
#define CFG_PRT_DATA_TXREADI_DISABLED 0x00
#define CFG_PRT_DATA_PORTID_DDC       0x00
#define CFG_PRT_DATA_MODE_ADDR        (0x42 << 1)
#define CFG_PRT_DATA_PROTO_UBX        0x01
#define CFG_PRT_DATA_PROTO_NMEA       0x02
#define CFG_PRT_DATA_PROTO_RTCM       0x04
#define CFG_PRT_DATA_FLAGS_EXTTIMEOUT 0x02


typedef struct {
    int16_t  X;
    int16_t  Y;
    int16_t  Z;
    uint16_t status;
} __attribute__((packed)) MagData;

typedef union {
    struct {
        UBXHeader_t header;
        MagData     data;
        UBXFooter_t footer;
    } __attribute__((packed)) fragments;
    UBXPacket_t packet;
} MagUbxPkt;

typedef struct {
    uint32_t flightTime;
    uint16_t options;
    uint8_t  board_type;
    uint8_t  board_revision;
    uint8_t  commit_tag_name[26];
    uint8_t  sha1sum[8];
} __attribute__((packed)) SysData;

typedef union {
    struct {
        UBXHeader_t header;
        SysData     data;
        UBXFooter_t footer;
    } fragments;
    UBXPacket_t packet;
} SysUbxPkt;


typedef struct {
    uint8_t  portID;
    uint8_t  reserved0;
    uint16_t txReady;
    uint32_t mode;
    uint32_t reserved3;
    uint16_t inProtoMask;
    uint16_t outProtoMask;
    uint16_t flags;
    uint16_t reserved5;
} __attribute__((packed)) CfgPrtData;

typedef union {
    struct {
        UBXHeader_t header;
        CfgPrtData  data;
        UBXFooter_t footer;
    } fragments;
    UBXPacket_t packet;
} CfgPrtPkt;

#endif /* GPSV9PROTOCOL_H_ */

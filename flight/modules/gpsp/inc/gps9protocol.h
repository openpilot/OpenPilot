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

#define UBX_OP_CUST_CLASS      0x99
#define UBX_OP_SYS             0x01
#define UBX_OP_MAG             0x02

#define SYS_DATA_OPTIONS_FLASH 0x01
#define SYS_DATA_OPTIONS_MAG   0x02


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


#endif /* GPSV9PROTOCOL_H_ */

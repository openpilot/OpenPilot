/**
 ******************************************************************************
 *
 * @file       frskyprotocol.h  
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
#ifndef FRSKYPROTOCOL_H_
#define FRSKYPROTOCOL_H_

#define HEADER 0x10
// Id definitions (taken from https://code.google.com/p/opentx/source/browse/trunk/src/telemetry/frsky_sport.cpp )
#define RSSI_ID                 0xf101
#define ADC1_ID                 0xf102
#define ADC2_ID                 0xf103
#define BATT_ID                 0xf104
#define SWR_ID                  0xf105
#define T1_FIRST_ID             0x0400
#define T1_LAST_ID              0x040f
#define T2_FIRST_ID             0x0410
#define T2_LAST_ID              0x041f
#define RPM_FIRST_ID            0x0500
#define RPM_LAST_ID             0x050f
#define FUEL_FIRST_ID           0x0600
#define FUEL_LAST_ID            0x060f
#define ALT_FIRST_ID            0x0100
#define ALT_LAST_ID             0x010f
#define VARIO_FIRST_ID          0x0110
#define VARIO_LAST_ID           0x011f
#define ACCX_FIRST_ID           0x0700
#define ACCX_LAST_ID            0x070f
#define ACCY_FIRST_ID           0x0710
#define ACCY_LAST_ID            0x071f
#define ACCZ_FIRST_ID           0x0720
#define ACCZ_LAST_ID            0x072f
#define CURR_FIRST_ID           0x0200
#define CURR_LAST_ID            0x020f
#define VFAS_FIRST_ID           0x0210
#define VFAS_LAST_ID            0x021f
#define CELLS_FIRST_ID          0x0300
#define CELLS_LAST_ID           0x030f
#define GPS_LONG_LATI_FIRST_ID  0x0800
#define GPS_LONG_LATI_LAST_ID   0x080f
#define GPS_ALT_FIRST_ID        0x0820
#define GPS_ALT_LAST_ID         0x082f
#define GPS_SPEED_FIRST_ID      0x0830
#define GPS_SPEED_LAST_ID       0x083f
#define GPS_COURS_FIRST_ID      0x0840
#define GPS_COURS_LAST_ID       0x084f
#define GPS_TIME_DATE_FIRST_ID  0x0850
#define GPS_TIME_DATE_LAST_ID   0x085f

typedef struct{
    uint8_t header; // 0x10 (8bit data frame header)
    uint16_t valueType;  //value_type (16 bit, e.g. voltage / speed)
    union {
    uint8_t a8[4]; // value (32 bit, may be signed or unsigned depending on value type)
    uint32_t u32;
    int32_t i32;
    } value;
    uint8_t checksum; // checksum (8 bit)
}__attribute__((packed)) frskyPacket;

#define FILL_U8(packet, fieldId, fieldValue) \
packet.header = HEADER; \
packet.valueType = fieldId; \
packet.value.u32 = 0; \
packet.value.a8[0] = fieldValue

#define FILL_U32(packet, fieldId, fieldValue) \
packet.header = HEADER; \
packet.valueType = fieldId; \
packet.value.u32 = fieldValue

#define FILL_I32(packet, fieldId, fieldValue) \
packet.header = HEADER; \
packet.valueType = fieldId; \
packet.value.i32 = fieldValue

#endif /* FRSKYPROTOCOL_H_ */

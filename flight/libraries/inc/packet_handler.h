/**
******************************************************************************
* @addtogroup OpenPilotSystem OpenPilot System
* @{
* @addtogroup OpenPilotLibraries OpenPilot System Libraries
* @{
*
* @file       packet_handler.h
* @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2012.
* @brief      A packet handler for handeling radio packet transmission.
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

#ifndef __PACKET_HANDLER_H__
#define __PACKET_HANDLER_H__

#include <uavobjectmanager.h>
#include <gcsreceiver.h>
#include <oplinksettings.h>
#include <pios_rfm22b_rcvr.h>

// Public defines / macros
#define PHPacketSize(p) ((uint8_t*)(p->data) + p->header.data_size - (uint8_t*)p)
#define PHPacketSizeECC(p) ((uint8_t*)(p->data) + p->header.data_size  + RS_ECC_NPARITY - (uint8_t*)p)

// Public types
typedef enum {
    PACKET_TYPE_NONE = 0,
    PACKET_TYPE_STATUS,              // broadcasts status of this modem
    PACKET_TYPE_CON_REQUEST,         // request a connection to another modem
    PACKET_TYPE_DATA,                // data packet (packet contains user data)
    PACKET_TYPE_DUPLICATE_DATA,      // a duplicate data packet
    PACKET_TYPE_PPM,                 // PPM relay values
    PACKET_TYPE_ACK,                 // Acknowlege the receipt of a packet
    PACKET_TYPE_NACK,                // Acknowlege the receipt of an uncorrectable packet
} PHPacketType;

typedef struct {
    uint32_t destination_id;
    portTickType prev_tx_time;
    uint16_t seq_num;
    uint8_t type;
    uint8_t data_size;
} PHPacketHeader;

#define PH_MAX_DATA (PIOS_PH_MAX_PACKET - sizeof(PHPacketHeader) - RS_ECC_NPARITY)
#define PH_PACKET_SIZE(p) ((p)->data + (p)->header.data_size - (uint8_t*)(p) + RS_ECC_NPARITY)
typedef struct {
    PHPacketHeader header;
    uint8_t data[PH_MAX_DATA + RS_ECC_NPARITY];
} PHPacket, *PHPacketHandle;

#define PH_ACK_NACK_DATA_SIZE(p) ((uint8_t*)((p)->ecc) - (uint8_t*)(((PHPacketHandle)(p))->data))
typedef struct {
    PHPacketHeader header;
    uint8_t ecc[RS_ECC_NPARITY];
} PHAckNackPacket, *PHAckNackPacketHandle;

#define PH_PPM_DATA_SIZE(p) ((uint8_t*)((p)->ecc) - (uint8_t*)(((PHPacketHandle)(p))->data))
typedef struct {
    PHPacketHeader header;
    int16_t channels[PIOS_RFM22B_RCVR_MAX_CHANNELS];
    uint8_t ecc[RS_ECC_NPARITY];
} PHPpmPacket, *PHPpmPacketHandle;

#define PH_STATUS_DATA_SIZE(p) ((uint8_t*)((p)->ecc) - (uint8_t*)(((PHPacketHandle)(p))->data))
typedef struct {
    PHPacketHeader header;
    uint32_t source_id;
    uint8_t link_quality;
    int8_t received_rssi;
    uint8_t ecc[RS_ECC_NPARITY];
} PHStatusPacket, *PHStatusPacketHandle;

#define PH_CONNECTION_DATA_SIZE(p) ((uint8_t*)((p)->ecc) - (uint8_t*)(((PHPacketHandle)(p))->data))
typedef struct {
    PHPacketHeader header;
    uint32_t source_id;
    uint32_t min_frequency;
    uint32_t max_frequency;
    uint32_t channel_spacing;
    portTickType status_rx_time;
    OPLinkSettingsMainPortOptions main_port;
    OPLinkSettingsFlexiPortOptions flexi_port;
    OPLinkSettingsVCPPortOptions vcp_port;
    OPLinkSettingsComSpeedOptions com_speed;
    uint8_t ecc[RS_ECC_NPARITY];
} PHConnectionPacket, *PHConnectionPacketHandle;

#endif // __PACKET_HANDLER_H__

/**
 * @}
 * @}
 */

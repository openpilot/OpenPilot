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
	PACKET_TYPE_STATUS,         // broadcasts status of this modem
	PACKET_TYPE_CON_REQUEST,    // request a connection to another modem
	PACKET_TYPE_DATA,           // data packet (packet contains user data)
	PACKET_TYPE_DUPLICATE_DATA, // a duplicate data packet
	PACKET_TYPE_PPM,            // PPM relay values
	PACKET_TYPE_ACK,            // Acknowlege the receipt of a packet
	PACKET_TYPE_ACK_RTS,        // Acknowlege the receipt of a packet and indicate that the receiving side has data to send (ready to send)
	PACKET_TYPE_NACK,           // Acknowlege the receipt of an uncorrectable packet
} PHPacketType;

typedef struct {
	uint32_t destination_id;
	uint32_t source_id;
	uint8_t type;
	uint8_t data_size;
	uint16_t seq_num;
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
	uint16_t channels[PIOS_RFM22B_RCVR_MAX_CHANNELS];
	uint8_t ecc[RS_ECC_NPARITY];
} PHPpmPacket, *PHPpmPacketHandle;

#define PH_STATUS_DATA_SIZE(p) ((uint8_t*)((p)->ecc) - (uint8_t*)(((PHPacketHandle)(p))->data))
typedef struct {
	PHPacketHeader header;
	uint8_t link_quality;
	int8_t received_rssi;
	uint8_t ecc[RS_ECC_NPARITY];
} PHStatusPacket, *PHStatusPacketHandle;

#define PH_CONNECTION_DATA_SIZE(p) ((uint8_t*)((p)->ecc) - (uint8_t*)(((PHPacketHandle)(p))->data))
typedef struct {
	PHPacketHeader header;
	uint8_t datarate;
	uint32_t frequency_hz;
	uint32_t min_frequency;
	uint32_t max_frequency;
	uint8_t max_tx_power;
	OPLinkSettingsOutputConnectionOptions port;
	OPLinkSettingsComSpeedOptions com_speed;
	uint8_t ecc[RS_ECC_NPARITY];
} PHConnectionPacket, *PHConnectionPacketHandle;

typedef struct {
	uint32_t default_destination_id;
	uint32_t source_id;
	uint16_t max_connections;
	uint8_t win_size;
} PacketHandlerConfig;

typedef int32_t (*PHOutputStream)(PHPacketHandle packet);
typedef void (*PHDataHandler)(uint8_t *data, uint8_t len, int8_t rssi, int8_t afc);
typedef void (*PHStatusHandler)(PHStatusPacketHandle s, int8_t rssi, int8_t afc);
typedef void (*PHPPMHandler)(uint16_t *channels);

typedef uint32_t PHInstHandle;

// Public functions
PHInstHandle PHInitialize(PacketHandlerConfig *cfg);
void PHRegisterOutputStream(PHInstHandle h, PHOutputStream f);
void PHRegisterDataHandler(PHInstHandle h, PHDataHandler f);
void PHRegisterStatusHandler(PHInstHandle h, PHStatusHandler f);
void PHRegisterPPMHandler(PHInstHandle h, PHPPMHandler f);
uint32_t PHConnect(PHInstHandle h, uint32_t dest_id);
PHPacketHandle PHGetRXPacket(PHInstHandle h);
void PHReleaseRXPacket(PHInstHandle h, PHPacketHandle p);
PHPacketHandle PHGetTXPacket(PHInstHandle h);
void PHReleaseTXPacket(PHInstHandle h, PHPacketHandle p);
uint8_t PHTransmitPacket(PHInstHandle h, PHPacketHandle p);
uint8_t PHTransmitData(PHInstHandle h, uint8_t *buf, uint16_t len);

#endif // __PACKET_HANDLER_H__

/**
 * @}
 * @}
 */

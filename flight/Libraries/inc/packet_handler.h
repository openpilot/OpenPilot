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

// Public defines / macros
#define PHPacketSize(p) ((uint8_t*)(p->data) + p->header.data_size - (uint8_t*)p)
#define PHPacketSizeECC(p) ((uint8_t*)(p->data) + p->header.data_size  + RS_ECC_NPARITY - (uint8_t*)p)

// Public types
typedef enum {
	PACKET_TYPE_NONE = 0,
	PACKET_TYPE_CONNECT,        // for requesting a connection
	PACKET_TYPE_DISCONNECT,     // to tell the other modem they cannot connect to us
	PACKET_TYPE_READY,          // tells the other modem we are ready to accept more data
	PACKET_TYPE_NOTREADY,       // tells the other modem we're not ready to accept more data - we can also send user data in this packet type
	PACKET_TYPE_DATARATE,       // for changing the RF data rate
	PACKET_TYPE_PING,           // used to check link is still up
	PACKET_TYPE_ADJUST_TX_PWR,  // used to ask the other modem to adjust it's tx power
	PACKET_TYPE_DATA,           // data packet (packet contains user data)
	PACKET_TYPE_ACKED_DATA,     // data packet that requies an ACK
	PACKET_TYPE_RECEIVER,       // Receiver relay values
	PACKET_TYPE_ACK,
	PACKET_TYPE_NACK
} PHPacketType;

typedef struct {
	uint32_t source_id;
	uint32_t destination_id;
	uint8_t type;
	uint8_t tx_seq;
	uint8_t rx_seq;
	uint8_t data_size;
} PHPacketHeader;

#define PH_MAX_DATA (PH_MAX_PACKET - sizeof(PHPacketHeader) - RS_ECC_NPARITY)
typedef struct {
	PHPacketHeader header;
	uint8_t data[PH_MAX_DATA + RS_ECC_NPARITY];
} PHPacket, *PHPacketHandle;

typedef struct {
	uint8_t txWinSize;
	uint16_t maxConnections;
	uint32_t id;
	void *dev;
	uint8_t (*output_stream)(void *dev, PHPacketHandle packet);
	void (*set_baud)(uint32_t baud);
	void (*data_handler)(void *dev, uint8_t *data, uint8_t len);
	void (*receiver_handler)(void *dev, uint8_t *data, uint8_t len);
} PacketHandlerConfig;

typedef int32_t (*PHOutputStream)(PHPacketHandle packet);

typedef void* PHInstHandle;

// Public functions
PHInstHandle PHInitialize(PacketHandlerConfig *cfg);
uint32_t PHConnect(PHInstHandle h, uint32_t dest_id);
PHPacketHandle PHGetRXPacket(PHInstHandle h);
PHPacketHandle PHGetTXPacket(PHInstHandle h);
PHPacketHandle PHReserveTXPacket(PHInstHandle h);
void PHReleaseLock(PHInstHandle h, bool keep_packet);
void PHReleaseTXPacket(PHInstHandle h, PHPacketHandle p);
uint8_t PHTransmitPacket(PHInstHandle h, PHPacketHandle p);
uint8_t PHReceivePacket(PHInstHandle h, PHPacketHandle p);

#endif // __PACKET_HANDLER_H__

/**
 * @}
 * @}
 */

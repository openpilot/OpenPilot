/**
 ******************************************************************************
 * @addtogroup OpenPilotSystem OpenPilot System
 * @{
 * @addtogroup OpenPilotLibraries OpenPilot System Libraries
 * @{
 *
 * @file       packet_handler.c
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

#include "openpilot.h"
#include "packet_handler.h"
#include "aes.h"
#include "crc.h"

// Private types and constants
typedef struct {
	PacketHandlerConfig cfg;
	PHPacket *tx_packets;
	uint8_t tx_win_start;
	uint8_t tx_win_end;
	uint16_t tx_seq_id;
	PHPacket rx_packet;
	PHOutputStream stream;
	xSemaphoreHandle lock;
} PHPacketData, *PHPacketDataHandle;

// Private functions
static uint8_t PHLSendAck(PHPacketDataHandle data, PHPacketHandle p);
static uint8_t PHLTransmitPacket(PHPacketDataHandle data, PHPacketHandle p);

/**
 * Initialize the Packet Handler library
 * \param[in] txWinSize The transmission window size (number of tx packet buffers).
 * \param[in] streme A callback function for transmitting the packet.
 * \param[in] id The source ID of transmitter.
 * \return PHInstHandle The Pachet Handler instance data.
 * \return 0 Failure
 */
PHInstHandle PHInitialize(PacketHandlerConfig *cfg)
{
	// Allocate the primary structure
	PHPacketDataHandle data = pvPortMalloc(sizeof(PHPacketData));
	if (!data)
		return 0;
	data->cfg = *cfg;
	data->tx_seq_id = 0;

	// Allocate the tx packet window
	data->tx_packets = pvPortMalloc(sizeof(PHPacket) * data->cfg.txWinSize);

	// Initialize the window
	data->tx_win_start = data->tx_win_end = 0;
	for (uint8_t i = 0; i < data->cfg.txWinSize; ++i)
		data->tx_packets[i].header.type = PACKET_TYPE_NONE;

	// Create the lock
	data->lock = xSemaphoreCreateRecursiveMutex();

	// Return the structure.
	return (PHInstHandle)data;
}

/**
 * Get a packet out of the transmit buffer.
 * \param[in] h The packet handler instance data pointer.
 * \param[in] dest_id The destination ID of this connection
 * \return PHPacketHandle A pointer to the packet buffer.
 * \return 0 No packets buffers avaiable in the transmit window.
 */
uint32_t PHConnect(PHInstHandle h, uint32_t dest_id)
{
	return 1;
}

/**
 * Temporarily reserve the next packet in the TX packet window.
 * This function places a tempoary hold on the next TX packet and
 * retains the packet handler lock.
 *
 * NOTE: PHReleaseLock must be called to release the lock and  retain
 * or release the reserved packet.
 *
 * \param[in] h The packet handler instance data pointer.
 * \return PHPacketHandle A pointer to the packet buffer.
 * \return 0 No packets buffers avaiable in the transmit window.
 */
PHPacketHandle PHReserveTXPacket(PHInstHandle h)
{
	PHPacketDataHandle data = (PHPacketDataHandle)h;

	// Lock
	xSemaphoreTakeRecursive(data->lock, portMAX_DELAY);

	// Is the window full?
	uint8_t next_end = (data->tx_win_end + 1) % data->cfg.txWinSize;
	if(next_end == data->tx_win_start) {

		// Release the lock
		xSemaphoreGiveRecursive(data->lock);

		return NULL;
	}

	// Return a pointer to the packet at the end of the TX window.
	return data->tx_packets + data->tx_win_end;
}

/**
 * Get a packet out of the transmit buffer and keep the lock.
 * NOTE: PHReleaseLock must be called to release the lock.
 * \param[in] h The packet handler instance data pointer.
 * \param[in] keep_packet Maintain a permanent (until released) lock on the packet.
 */
void PHReleaseLock(PHInstHandle h, bool keep_packet)
{
	PHPacketDataHandle data = (PHPacketDataHandle)h;
	uint8_t next_end = (data->tx_win_end + 1) % data->cfg.txWinSize;

	// Increment the end index if packet is being kept.
	if (keep_packet)
		data->tx_win_end = next_end;

	// Release lock
	xSemaphoreGiveRecursive(data->lock);
}

/**
 * Get a packet out of the transmit buffer.
 * \param[in] h The packet handler instance data pointer.
 * \return PHPacketHandle A pointer to the packet buffer.
 * \return 0 No packets buffers avaiable in the transmit window.
 */
PHPacketHandle PHGetTXPacket(PHInstHandle h)
{
	PHPacketHandle p = PHReserveTXPacket(h);
	PHReleaseLock(p, 1);
	return p;
}

/**
 * Release a packet from the transmit packet buffer window.
 * \param[in] h The packet handler instance data pointer.
 * \param[in] p A pointer to the packet buffer.
 * \return Nothing
 */
void PHReleaseTXPacket(PHInstHandle h, PHPacketHandle p)
{
	PHPacketDataHandle data = (PHPacketDataHandle)h;

	// Lock
	xSemaphoreTakeRecursive(data->lock, portMAX_DELAY);

	// Change the packet type so we know this packet is unused.
	p->header.type = PACKET_TYPE_NONE;

	// If this packet is at the start of the window, increment the start index.
	while ((data->tx_win_start != data->tx_win_end) &&
				 (data->tx_packets[data->tx_win_start].header.type == PACKET_TYPE_NONE))
		data->tx_win_start = (data->tx_win_start + 1) % data->cfg.txWinSize;

	// Release lock
	xSemaphoreGiveRecursive(data->lock);
}

/**
 * Transmit a packet from the transmit packet buffer window.
 * \param[in] h The packet handler instance data pointer.
 * \param[in] p A pointer to the packet buffer.
 * \return 1 Success
 * \return 0 Failure
 */
uint8_t PHTransmitPacket(PHInstHandle h, PHPacketHandle p)
{
	PHPacketDataHandle data = (PHPacketDataHandle)h;

	// Try to transmit the packet.
	if (!PHLTransmitPacket(data, p))
		return 0;

	// If this packet doesn't require an ACK, remove it from the TX window.
	switch (p->header.type) {
	case PACKET_TYPE_READY:
	case PACKET_TYPE_NOTREADY:
	case PACKET_TYPE_DATA:
	case PACKET_TYPE_RECEIVER:
		PHReleaseTXPacket(h, p);
		break;
	}

	return 1;
}

/**
 * Process a packet that has been received.
 * \param[in] h The packet handler instance data pointer.
 * \param[in] p A pointer to the packet buffer.
 * \return 1 Success
 * \return 0 Failure
 */
uint8_t PHReceivePacket(PHInstHandle h, PHPacketHandle p)
{
	PHPacketDataHandle data = (PHPacketDataHandle)h;

	switch (p->header.type) {
	case PACKET_TYPE_ACKED_DATA:

		// Send the ACK
		PHLSendAck(data, p);

		// Pass on the data.
		if(data->cfg.data_handler)
			data->cfg.data_handler(p->data, p->header.data_size);

		break;

	case PACKET_TYPE_RECEIVER:

		// Pass on the data to the receiver handler.
		if(data->cfg.receiver_handler)
			data->cfg.receiver_handler(p->data, p->header.data_size);

		break;
	}

	return 1;
}

/**
 * Transmit a packet from the transmit packet buffer window.
 * \param[in] data The packet handler instance data pointer.
 * \param[in] p A pointer to the packet buffer.
 * \return 1 Success
 * \return 0 Failure
 */
static uint8_t PHLTransmitPacket(PHPacketDataHandle data, PHPacketHandle p)
{

	// Set the sequence ID to the current ID.
	p->header.tx_seq = data->tx_seq_id++;

	// Transmit the packet using the output stream.
	if(!data->cfg.output_stream(p))
		return 0;

	return 1;
}

/**
 * Send an ACK packet.
 * \param[in] data The packet handler instance data pointer.
 * \param[in] p A pointer to the packet buffer of the packet to be ACKed.
 * \return 1 Success
 * \return 0 Failure
 */
static uint8_t PHLSendAck(PHPacketDataHandle data, PHPacketHandle p)
{

	// Create the ACK message
	PHPacketHeader ack;
	ack.source_id = data->cfg.id;
	ack.destination_id = p->header.source_id;
	ack.type = PACKET_TYPE_ACK;
	ack.rx_seq = p->header.tx_seq;
	ack.data_size = 0;

	// Set the packet.
	PHLTransmitPacket(data, (PHPacketHandle)&ack);

	return 1;
}

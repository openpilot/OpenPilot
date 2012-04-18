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
#include "ecc.h"

extern char *debug_msg;

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
	PHOutputStream output_stream;
	PHDataHandler data_handler;
	PHStatusHandler status_handler;
	PHPPMHandler ppm_handler;
} PHPacketData, *PHPacketDataHandle;

// Private functions
static uint8_t PHLSendAck(PHPacketDataHandle data, PHPacketHandle p);
static uint8_t PHLSendNAck(PHPacketDataHandle data, PHPacketHandle p);
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

	// Initialize the ECC library.
  initialize_ecc();

	// Return the structure.
	return (PHInstHandle)data;
}

/**
 * Register an output stream handler
 * \param[in] h The packet handler instance data pointer.
 * \param[in] f The output stream handler function
 */
void PHRegisterOutputStream(PHInstHandle h, PHOutputStream f)
{
	PHPacketDataHandle data = (PHPacketDataHandle)h;

	data->output_stream = f;
}

/**
 * Register a data handler
 * \param[in] h The packet handler instance data pointer.
 * \param[in] f The data handler function
 */
void PHRegisterDataHandler(PHInstHandle h, PHDataHandler f)
{
	PHPacketDataHandle data = (PHPacketDataHandle)h;

	data->data_handler = f;
}

/**
 * Register a PPM packet handler
 * \param[in] h The packet handler instance data pointer.
 * \param[in] f The PPM handler function
 */
void PHRegisterStatusHandler(PHInstHandle h, PHStatusHandler f)
{
	PHPacketDataHandle data = (PHPacketDataHandle)h;

	data->status_handler = f;
}

/**
 * Register a PPM packet handler
 * \param[in] h The packet handler instance data pointer.
 * \param[in] f The PPM handler function
 */
void PHRegisterPPMHandler(PHInstHandle h, PHPPMHandler f)
{
	PHPacketDataHandle data = (PHPacketDataHandle)h;

	data->ppm_handler = f;
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
 * Get a packet out of the transmit buffer.
 * \param[in] h The packet handler instance data pointer.
 * \return PHPacketHandle A pointer to the packet buffer.
 * \return 0 No packets buffers avaiable in the transmit window.
 */
PHPacketHandle PHGetTXPacket(PHInstHandle h)
{
	PHPacketDataHandle data = (PHPacketDataHandle)h;

	// Lock
	xSemaphoreTakeRecursive(data->lock, portMAX_DELAY);
	PHPacketHandle p = data->tx_packets + data->tx_win_end;

	// Is the window full?
	uint8_t next_end = (data->tx_win_end + 1) % data->cfg.txWinSize;
	if(next_end == data->tx_win_start)
		return NULL;
	data->tx_win_end = next_end;

	// Release lock
	xSemaphoreGiveRecursive(data->lock);

	// Return a pointer to the packet at the end of the TX window.
	return p;
}

/**
 * Get a pointer to the the receive buffer.
 * \param[in] h The packet handler instance data pointer.
 * \return PHPacketHandle A pointer to the packet buffer.
 */
PHPacketHandle PHGetRXPacket(PHInstHandle h)
{
	PHPacketDataHandle data = (PHPacketDataHandle)h;
	return &(data->rx_packet);
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
	case PACKET_TYPE_PPM:
		PHReleaseTXPacket(h, p);
		break;
	}

	return 1;
}

/**
 * Process a packet that has been received.
 * \param[in] h The packet handler instance data pointer.
 * \param[in] p A pointer to the packet buffer.
 * \param[in] received_len The length of data received.
 * \return 1 Success
 * \return 0 Failure
 */
uint8_t PHReceivePacket(PHInstHandle h, PHPacketHandle p, uint16_t received_len)
{
	PHPacketDataHandle data = (PHPacketDataHandle)h;

	// Verify the packet length.
	// Note: The last two bytes should be the RSSI and AFC.
	uint16_t len = PHPacketSizeECC(p);
	if (received_len != (len + 2))
	{
		DEBUG_PRINTF(1, "Packet length error\n\r");
		return 0;
	}

	// Attempt to correct any errors in the packet.
	decode_data((unsigned char*)p, len);

	// Check that there were no unfixed errors.
	bool rx_error = check_syndrome() != 0;
	if(rx_error)
		DEBUG_PRINTF(1, "Error in packet\n\r");

	// Add the RSSI and AFC to the packet.
	p->header.rssi = *(((int8_t*)p) + len);
	p->header.afc = *(((int8_t*)p) + len + 1);

	switch (p->header.type) {

	case PACKET_TYPE_STATUS:

		if (!rx_error)

			// Pass on the channels to the PPM handler.
			if(data->status_handler)
				data->status_handler(p);

		break;

	case PACKET_TYPE_ACKED_DATA:

		// Send the ACK / NACK
		if (rx_error)
		{
			DEBUG_PRINTF(1, "Sending NACK\n\r");
			PHLSendNAck(data, p);
		}
		else
		{

			PHLSendAck(data, p);

			// Pass on the data.
			if(data->data_handler)
				data->data_handler(p->data, p->header.data_size);
		}

		break;

	case PACKET_TYPE_ACK:
	{
		// Find the packet ID in the TX buffer, and free it.
		unsigned int i = 0;
		for (unsigned int i = 0; i < data->cfg.txWinSize; ++i)
			if (data->tx_packets[i].header.tx_seq == p->header.rx_seq)
				PHReleaseTXPacket(h, data->tx_packets + i);
#ifdef DEBUG_LEVEL
		if (i == data->cfg.txWinSize)
			DEBUG_PRINTF(1, "Error finding acked packet to release\n\r");
#endif
	}
	break;

	case PACKET_TYPE_NACK:
	{
		// Resend the packet.
		unsigned int i = 0;
		for ( ; i < data->cfg.txWinSize; ++i)
			if (data->tx_packets[i].header.tx_seq == p->header.rx_seq)
				PHLTransmitPacket(data, data->tx_packets + i);
#ifdef DEBUG_LEVEL
		if (i == data->cfg.txWinSize)
			DEBUG_PRINTF(1, "Error finding acked packet to NACK\n\r");
		DEBUG_PRINTF(1, "Resending after NACK\n\r");
#endif
	}
	break;

	case PACKET_TYPE_PPM:

		if (!rx_error)

			// Pass on the channels to the PPM handler.
			if(data->ppm_handler)
				data->ppm_handler(((PHPpmPacketHandle)p)->channels);

		break;

	case PACKET_TYPE_DATA:

		if (!rx_error)

			// Pass on the data to the data handler.
			if(data->data_handler)
				data->data_handler(p->data, p->header.data_size);

		break;

	default:
		break;
	}

	return 1;
}

/**
 * Broadcast a status packet.
 * \param[in] data The packet handler instance data pointer.
 * \param[in] p A pointer to the packet buffer.
 * \return 1 Success
 * \return 0 Failure
 */
uint8_t PHBroadcastStatus(PHInstHandle h, uint32_t id, int8_t rssi)
{
	PHPacketDataHandle data = (PHPacketDataHandle)h;

	// Create the status message
	PHPacketHeader header;
	header.destination_id = 0xffffffff;
	header.source_id = id;
	header.rssi = rssi;
	header.type = PACKET_TYPE_STATUS;
	header.data_size = 0;

	// Send the packet.
	return PHLTransmitPacket(data, (PHPacketHandle)&header);
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

	if(!data->output_stream)
		return 0;

	// Set the sequence ID to the current ID.
	p->header.tx_seq = data->tx_seq_id++;

	// Add the error correcting code.
	encode_data((unsigned char*)p, PHPacketSize(p), (unsigned char*)p);

	// Transmit the packet using the output stream.
	if(data->output_stream(p) == -1)
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
	ack.destination_id = p->header.source_id;
	ack.type = PACKET_TYPE_ACK;
	ack.rx_seq = p->header.tx_seq;
	ack.data_size = 0;

	// Send the packet.
	return PHLTransmitPacket(data, (PHPacketHandle)&ack);
}

/**
 * Send an NAck packet.
 * \param[in] data The packet handler instance data pointer.
 * \param[in] p A pointer to the packet buffer of the packet to be ACKed.
 * \return 1 Success
 * \return 0 Failure
 */
static uint8_t PHLSendNAck(PHPacketDataHandle data, PHPacketHandle p)
{

	// Create the NAck message
	PHPacketHeader ack;
	ack.destination_id = p->header.source_id;
	ack.type = PACKET_TYPE_NACK;
	ack.rx_seq = p->header.tx_seq;
	ack.data_size = 0;

	// Set the packet.
	return PHLTransmitPacket(data, (PHPacketHandle)&ack);
}

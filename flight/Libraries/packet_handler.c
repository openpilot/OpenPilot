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
	PHPacket *rx_packets;
	uint8_t rx_win_start;
	uint8_t rx_win_end;
	xSemaphoreHandle lock;
	PHOutputStream output_stream;
	PHDataHandler data_handler;
	PHStatusHandler status_handler;
	PHPPMHandler ppm_handler;
} PHPacketData, *PHPacketDataHandle;

// Private functions
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

	// Allocate the packet windows
	data->tx_packets = pvPortMalloc(sizeof(PHPacket) * data->cfg.win_size);
	data->rx_packets = pvPortMalloc(sizeof(PHPacket) * data->cfg.win_size);

	// Initialize the windows
	data->tx_win_start = data->tx_win_end = 0;
	data->rx_win_start = data->rx_win_end = 0;
	for (uint8_t i = 0; i < data->cfg.win_size; ++i)
	{
		data->tx_packets[i].header.type = PACKET_TYPE_NONE;
		data->rx_packets[i].header.type = PACKET_TYPE_NONE;
	}

	// Create the lock
	data->lock = xSemaphoreCreateRecursiveMutex();

	// Initialize the ECC library.
	initialize_ecc();

	// Initialize the handlers
	data->output_stream = 0;
	data->data_handler = 0;
	data->status_handler = 0;
	data->ppm_handler = 0;

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

	// Find a free packet.
	PHPacketHandle p = NULL;
	for (uint8_t i = 0; i < data->cfg.win_size; ++i)
		if (data->tx_packets[i].header.type == PACKET_TYPE_NONE)
		{
			p = data->tx_packets + i;
			break;
		}

	// Release lock
	xSemaphoreGiveRecursive(data->lock);

	// Return a pointer to the packet at the end of the TX window.
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
		data->tx_win_start = (data->tx_win_start + 1) % data->cfg.win_size;

	// Release lock
	xSemaphoreGiveRecursive(data->lock);
}

/**
 * Get a packet out of the receive buffer.
 * \param[in] h The packet handler instance data pointer.
 * \return PHPacketHandle A pointer to the packet buffer.
 * \return 0 No packets buffers avaiable in the transmit window.
 */
PHPacketHandle PHGetRXPacket(PHInstHandle h)
{
	PHPacketDataHandle data = (PHPacketDataHandle)h;

	// Lock
	xSemaphoreTakeRecursive(data->lock, portMAX_DELAY);

	// Find a free packet.
	PHPacketHandle p = NULL;
	for (uint8_t i = 0; i < data->cfg.win_size; ++i)
		if (data->rx_packets[i].header.type == PACKET_TYPE_NONE)
		{
			p = data->rx_packets + i;
			break;
		}

	// Release lock
	xSemaphoreGiveRecursive(data->lock);

	// Return a pointer to the packet at the end of the TX window.
	return p;
}

/**
 * Release a packet from the receive packet buffer window.
 * \param[in] h The packet handler instance data pointer.
 * \param[in] p A pointer to the packet buffer.
 * \return Nothing
 */
void PHReleaseRXPacket(PHInstHandle h, PHPacketHandle p)
{
	PHPacketDataHandle data = (PHPacketDataHandle)h;

	// Lock
	xSemaphoreTakeRecursive(data->lock, portMAX_DELAY);

	// Change the packet type so we know this packet is unused.
	p->header.type = PACKET_TYPE_NONE;

	// If this packet is at the start of the window, increment the start index.
	while ((data->rx_win_start != data->rx_win_end) &&
				 (data->rx_packets[data->rx_win_start].header.type == PACKET_TYPE_NONE))
		data->rx_win_start = (data->rx_win_start + 1) % data->cfg.win_size;

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

	return 1;
}

/**
 * Transmit a packet of data.
 * \param[in] h The packet handler instance data pointer.
 * \param[in] p A pointer to the data buffer.
 * \param[in] len The length of the data buffer.
 * \return 1 Success
 * \return 0 Failure
 */
uint8_t PHTransmitData(PHInstHandle h, uint8_t *buf, uint16_t len)
{
	PHPacketDataHandle data = (PHPacketDataHandle)h;

	// Get a packet from the packet handler.
	PHPacketHandle p = PHGetTXPacket(pios_packet_handler);
	if (!p)
		return 0;

	// Initialize the packet.
	p->header.destination_id = data->cfg.default_destination_id;
	p->header.source_id = data->cfg.source_id;
	p->header.type = PACKET_TYPE_DATA;
	p->header.data_size = len;

	// Copy the data into the packet.
	memcpy(p->data, buf, len);

	// Send the packet.
	return PHLTransmitPacket(data, p);
}

/**
 * Process a packet that has been received.
 * \param[in] h The packet handler instance data pointer.
 * \param[in] p A pointer to the packet buffer.
 * \param[in] received_len The length of data received.
 * \return 0 Failure
 * \return 1 Success
 */
uint8_t PHReceivePacket(PHInstHandle h, PHPacketHandle p)
{
	PHPacketDataHandle data = (PHPacketDataHandle)h;
	uint16_t len = PHPacketSizeECC(p);

	// Extract the RSSI and AFC.
	int8_t rssi = *(((int8_t*)p) + len);
	int8_t afc = *(((int8_t*)p) + len + 1);

	switch (p->header.type) {

	case PACKET_TYPE_STATUS:

		// Pass on the channels to the status handler.
		if(data->status_handler)
			data->status_handler((PHStatusPacketHandle)p, rssi, afc);
		break;

	case PACKET_TYPE_PPM:

		// Pass on the channels to the PPM handler.
		if(data->ppm_handler)
			data->ppm_handler(((PHPpmPacketHandle)p)->channels);
		break;

	case PACKET_TYPE_DATA:

		// Pass on the data to the data handler.
		if(data->data_handler)
			data->data_handler(p->data, p->header.data_size, rssi, afc);
		break;

	default:
		break;
	}

	// Release the packet.
	PHReleaseRXPacket(h, p);
	
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

	if(!data->output_stream)
		return 0;

	// Transmit the packet using the output stream.
	if(data->output_stream(p) == -1)
		return 0;

	return 1;
}

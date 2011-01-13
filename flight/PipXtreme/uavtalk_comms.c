/**
 ******************************************************************************
 *
 * @file       uavtalk_comms.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      RF Module hardware layer
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




// see http://newwiki.openpilot.org/display/Doc/UAVTalk .. for UAVTalk protocol description
//
// This module scans for OP UAVTalk packets in the comm-port or RF data streams.
// It will discard any corrupt/invalid packets and only pass valid ones.




#include <string.h>

#include "stm32f10x.h"
#include "gpio_in.h"
#include "uavtalk_comms.h"
#include "packet_handler.h"
#include "main.h"

#if defined(PIOS_COM_DEBUG)
	#define UAVTALK_DEBUG
#endif

// *****************************************************************************

typedef struct
{
	uint8_t		sync_byte;
	uint8_t		message_type;
	uint16_t	packet_size;	// not including CRC byte
	uint32_t	object_id;
} __attribute__((__packed__)) t_uav_header1;

typedef struct
{
	uint8_t		sync_byte;
	uint8_t		message_type;
	uint16_t	packet_size;	// not including CRC byte
	uint32_t	object_id;

	uint16_t	instance_id;
} __attribute__((__packed__)) t_uav_header2;

#define SYNC_VAL				0x3C
#define TYPE_MASK				0xFC
#define TYPE_VER				0x20
#define TYPE_OBJ				(TYPE_VER | 0x00)
#define TYPE_OBJ_REQ			(TYPE_VER | 0x01)
#define TYPE_OBJ_ACK			(TYPE_VER | 0x02)
#define TYPE_ACK				(TYPE_VER | 0x03)

#define MIN_HEADER_LENGTH		sizeof(t_uav_header1)
#define MAX_HEADER_LENGTH		sizeof(t_uav_header2)
#define MAX_PAYLOAD_LENGTH		256
#define CHECKSUM_LENGTH			1

#define MAX_PACKET_LENGTH		(MAX_HEADER_LENGTH + MAX_PAYLOAD_LENGTH + CHECKSUM_LENGTH)

// CRC lookup table
static const uint8_t crc_table[256] = {
    0x00, 0x07, 0x0e, 0x09, 0x1c, 0x1b, 0x12, 0x15, 0x38, 0x3f, 0x36, 0x31, 0x24, 0x23, 0x2a, 0x2d,
    0x70, 0x77, 0x7e, 0x79, 0x6c, 0x6b, 0x62, 0x65, 0x48, 0x4f, 0x46, 0x41, 0x54, 0x53, 0x5a, 0x5d,
    0xe0, 0xe7, 0xee, 0xe9, 0xfc, 0xfb, 0xf2, 0xf5, 0xd8, 0xdf, 0xd6, 0xd1, 0xc4, 0xc3, 0xca, 0xcd,
    0x90, 0x97, 0x9e, 0x99, 0x8c, 0x8b, 0x82, 0x85, 0xa8, 0xaf, 0xa6, 0xa1, 0xb4, 0xb3, 0xba, 0xbd,
    0xc7, 0xc0, 0xc9, 0xce, 0xdb, 0xdc, 0xd5, 0xd2, 0xff, 0xf8, 0xf1, 0xf6, 0xe3, 0xe4, 0xed, 0xea,
    0xb7, 0xb0, 0xb9, 0xbe, 0xab, 0xac, 0xa5, 0xa2, 0x8f, 0x88, 0x81, 0x86, 0x93, 0x94, 0x9d, 0x9a,
    0x27, 0x20, 0x29, 0x2e, 0x3b, 0x3c, 0x35, 0x32, 0x1f, 0x18, 0x11, 0x16, 0x03, 0x04, 0x0d, 0x0a,
    0x57, 0x50, 0x59, 0x5e, 0x4b, 0x4c, 0x45, 0x42, 0x6f, 0x68, 0x61, 0x66, 0x73, 0x74, 0x7d, 0x7a,
    0x89, 0x8e, 0x87, 0x80, 0x95, 0x92, 0x9b, 0x9c, 0xb1, 0xb6, 0xbf, 0xb8, 0xad, 0xaa, 0xa3, 0xa4,
    0xf9, 0xfe, 0xf7, 0xf0, 0xe5, 0xe2, 0xeb, 0xec, 0xc1, 0xc6, 0xcf, 0xc8, 0xdd, 0xda, 0xd3, 0xd4,
    0x69, 0x6e, 0x67, 0x60, 0x75, 0x72, 0x7b, 0x7c, 0x51, 0x56, 0x5f, 0x58, 0x4d, 0x4a, 0x43, 0x44,
    0x19, 0x1e, 0x17, 0x10, 0x05, 0x02, 0x0b, 0x0c, 0x21, 0x26, 0x2f, 0x28, 0x3d, 0x3a, 0x33, 0x34,
    0x4e, 0x49, 0x40, 0x47, 0x52, 0x55, 0x5c, 0x5b, 0x76, 0x71, 0x78, 0x7f, 0x6a, 0x6d, 0x64, 0x63,
    0x3e, 0x39, 0x30, 0x37, 0x22, 0x25, 0x2c, 0x2b, 0x06, 0x01, 0x08, 0x0f, 0x1a, 0x1d, 0x14, 0x13,
    0xae, 0xa9, 0xa0, 0xa7, 0xb2, 0xb5, 0xbc, 0xbb, 0x96, 0x91, 0x98, 0x9f, 0x8a, 0x8d, 0x84, 0x83,
    0xde, 0xd9, 0xd0, 0xd7, 0xc2, 0xc5, 0xcc, 0xcb, 0xe6, 0xe1, 0xe8, 0xef, 0xfa, 0xfd, 0xf4, 0xf3
};

// *****************************************************************************
// local variables

int8_t				uavtalk_previous_com_port = -1;

volatile uint16_t	uavtalk_rx_timer = 0;
volatile uint16_t	uavtalk_tx_timer = 0;

uint8_t				uavtalk_rx_buffer[MAX_PACKET_LENGTH] __attribute__ ((aligned(4)));
uint16_t			uavtalk_rx_buffer_wr;

uint8_t				uavtalk_tx_buffer[MAX_PACKET_LENGTH] __attribute__ ((aligned(4)));
uint16_t			uavtalk_tx_buffer_wr;

// *****************************************************************************
// 8-bit CRC updating

uint8_t uavtalk_updateCRC_byte(uint8_t crc, uint8_t b)
{
	return crc_table[crc ^ b];
}

uint8_t uavtalk_updateCRC_buffer(uint8_t crc, const void *data, int32_t length)
{
	// use registers for speed
    register uint8_t crc8 = crc;
    register const uint8_t *p = (uint8_t *)data;

    for (register int32_t i = length; i > 0; i--)
        crc8 = crc_table[crc8 ^ *p++];

    return crc8;
}

// *****************************************************************************
// returned value < 0 if no valid packet detected.
// otherwise returned value is the total size of the valid UAVTalk packet found in the buffer.
//
// any corrupt/invalid UAVTalk packets/data are deleted from the buffer and we scan for the next valid packet.

int16_t uavtalk_scanForPacket(void *buf, uint16_t *len)
{
	uint8_t *buffer = (uint8_t *)buf;

	t_uav_header1 *header1 = (t_uav_header1 *)buf;
//	t_uav_header2 *header2 = (t_uav_header2 *)buf;

	register uint16_t num_bytes = *len;

	if (num_bytes < MIN_HEADER_LENGTH + CHECKSUM_LENGTH)
		return -1;		// not yet enough bytes for a complete packet

	while (TRUE)
	{
		// scan the buffer for the start of a UAVTalk packet
		for (uint16_t i = 0; i < num_bytes; i++)
		{
			if (uavtalk_rx_buffer[i] != SYNC_VAL)
				continue;	// not the start of a packet - move on to the next byte in the buffer

			// possible start of packet found - we found a SYNC byte

			if (i > 0)
			{	// remove/delete leading bytes before the SYNC byte
				num_bytes -= i;
				if (num_bytes > 0)
					memmove(buffer, buffer + i, num_bytes);
				*len = num_bytes;
			}
			break;
		}

		if (num_bytes < MIN_HEADER_LENGTH + CHECKSUM_LENGTH)
			return -2;		// not yet enough bytes for a complete packet

		if (header1->sync_byte != SYNC_VAL)
		{	// SYNC byte was not found - start of UAVTalk packet not found in any of the data in the buffer
			*len = 0;	// empty the entire buffer
			return -3;
		}

		if (header1->packet_size < MIN_HEADER_LENGTH || header1->packet_size > MAX_HEADER_LENGTH + MAX_PAYLOAD_LENGTH)
		{	// the packet size value is too small or too big - assume either a corrupt UAVTalk packet or we are at the start of a packet
//			if (--num_bytes > 0)
//				memmove(buffer, buffer + 1, num_bytes);	// remove 1st byte
//			*len = num_bytes;
			buffer[0] ^= 0xaa;	// corrupt the sync byte - we'll move the buffer bytes down further up in the code
			continue;	// continue scanning for a valid packet in the buffer
		}

		if (num_bytes < header1->packet_size + CHECKSUM_LENGTH)
		{	// not yet enough bytes for a complete packet
			return -4;
		}

		// check the packet CRC
		uint8_t crc1 = uavtalk_updateCRC_buffer(0, buffer, header1->packet_size);
		uint8_t crc2 = buffer[header1->packet_size];
		if (crc1 != crc2)
		{	// faulty CRC
//			if (--num_bytes > 0)
//				memmove(buffer, buffer + 1, num_bytes);	// remove 1st byte
//			*len = num_bytes;
			buffer[0] ^= 0xaa;	// corrupt the sync byte - we'll move the buffer bytes down further up in the code

			#if defined(UAVTALK_DEBUG)
				DEBUG_PRINTF("UAVTalk packet corrupt %d\r\n", header1->packet_size + 1);
			#endif

			continue;	// continue scanning for a valid packet in the buffer
		}

		#if defined(UAVTALK_DEBUG)
			DEBUG_PRINTF("UAVTalk packet found %d\r\n", header1->packet_size + 1);
		#endif

		return (header1->packet_size + CHECKSUM_LENGTH);	// return the size of the UAVTalk packet
	}

	return -5;
}

// *****************************************************************************
// can be called from an interrupt if you wish

void uavtalk_1ms_tick(void)
{	// call this once every 1ms

	if (uavtalk_rx_timer < 0xffff) uavtalk_rx_timer++;
	if (uavtalk_tx_timer < 0xffff) uavtalk_tx_timer++;
}

// *****************************************************************************
// call this as often as possible - not from an interrupt

void uavtalk_process(void)
{	// copy data from comm-port RX buffer to RF packet handler TX buffer, and from RF packet handler RX buffer to comm-port TX buffer

	// ********************
	// decide which comm-port we are using (usart or usb)

	bool usb_comms = false;						// TRUE if we are using the usb port for comms.
	uint8_t comm_port = PIOS_COM_SERIAL;		// default to using the usart comm-port

	#if defined(PIOS_INCLUDE_USB_HID)
		if (PIOS_USB_HID_CheckAvailable(0))
		{	// USB comms is up, use the USB comm-port instead of the USART comm-port
			usb_comms = true;
			comm_port = PIOS_COM_TELEM_USB;
		}
	#endif

	// ********************
	// check to see if the local communication port has changed (usart/usb)

	if (uavtalk_previous_com_port < 0 && uavtalk_previous_com_port != comm_port)
	{	// the local communications port has changed .. remove any data in the buffers
		uavtalk_rx_buffer_wr = 0;

		uavtalk_tx_buffer_wr = 0;
	}
	else
	if (usb_comms)
	{	// we're using the USB for comms - keep the USART rx buffer empty
		int32_t bytes = PIOS_COM_ReceiveBufferUsed(PIOS_COM_SERIAL);
		while (bytes > 0)
		{
			PIOS_COM_ReceiveBuffer(PIOS_COM_SERIAL);
			bytes--;
		}
	}

	uavtalk_previous_com_port = comm_port;			// remember the current comm-port we are using

	// ********************

	uint16_t connection_index = 0;					// the RF connection we are using

	// ********************
	// send the data received down the comm-port to the RF packet handler TX buffer

	while (TRUE)
	{
		// free space size in the RF packet handler tx buffer
		uint16_t ph_num = ph_putData_free(connection_index);

		// get the number of data bytes received down the comm-port
		int32_t com_num = PIOS_COM_ReceiveBufferUsed(comm_port);

		// set the USART RTS handshaking line
		if (!usb_comms && ph_connected(connection_index))
		{
			if (ph_num < 32)
				SERIAL_RTS_CLEAR;						// lower the USART RTS line - we don't have space in the buffer for anymore bytes
			else
				SERIAL_RTS_SET;							// release the USART RTS line - we have space in the buffer for now bytes
		}
		else
			SERIAL_RTS_SET;								// release the USART RTS line

		// limit number of bytes we will get to how much space we have in our RX buffer
		if (com_num > sizeof(uavtalk_rx_buffer) - uavtalk_rx_buffer_wr)
			com_num = sizeof(uavtalk_rx_buffer) - uavtalk_rx_buffer_wr;

		while (com_num > 0)
		{	// fetch a byte from the comm-port RX buffer and save it into our RX buffer
			uavtalk_rx_buffer[uavtalk_rx_buffer_wr++] = PIOS_COM_ReceiveBuffer(comm_port);
			com_num--;
		}

		int16_t packet_size = uavtalk_scanForPacket(uavtalk_rx_buffer, &uavtalk_rx_buffer_wr);

		if (packet_size < 0)
			break;	// no UAVTalk packet in our RX buffer

		uavtalk_rx_timer = 0;

		if (!ph_connected(connection_index))
		{	// we don't have a link to a remote modem .. remove the UAVTalk packet from our RX buffer
			if (uavtalk_rx_buffer_wr > packet_size)
			{
				uavtalk_rx_buffer_wr -= packet_size;
				memmove(uavtalk_rx_buffer, uavtalk_rx_buffer + packet_size, uavtalk_rx_buffer_wr);
			}
			else
				uavtalk_rx_buffer_wr = 0;
			continue;
		}

		if (ph_num < packet_size)
			break;	// not enough room in the RF packet handler TX buffer for the UAVTalk packet

		// copy the rx'ed UAVTalk packet into the RF packet handler TX buffer for sending over the RF link
		ph_putData(connection_index, uavtalk_rx_buffer, packet_size);

		// remove the UAVTalk packet from our RX buffer
		if (uavtalk_rx_buffer_wr > packet_size)
		{
			uavtalk_rx_buffer_wr -= packet_size;
			memmove(uavtalk_rx_buffer, uavtalk_rx_buffer + packet_size, uavtalk_rx_buffer_wr);
		}
		else
			uavtalk_rx_buffer_wr = 0;
	}

	// ********************
	// send the data received via the RF link out the comm-port

	while (TRUE)
	{
		// get number of data bytes received via the RF link
		uint16_t ph_num = ph_getData_used(connection_index);

		// limit to how much space we have in the temp TX buffer
		if (ph_num > sizeof(uavtalk_tx_buffer) - uavtalk_tx_buffer_wr)
			ph_num = sizeof(uavtalk_tx_buffer) - uavtalk_tx_buffer_wr;

		if (ph_num > 0)
		{	// fetch the data bytes received via the RF link and save into our temp buffer
			ph_num = ph_getData(connection_index, uavtalk_tx_buffer + uavtalk_tx_buffer_wr, ph_num);
			uavtalk_tx_buffer_wr += ph_num;
		}

		int16_t packet_size = uavtalk_scanForPacket(uavtalk_tx_buffer, &uavtalk_tx_buffer_wr);

		if (packet_size <= 0)
			break;	// no UAV Talk packet found

		// we have a UAVTalk packet to send down the comm-port
/*
		#if (defined(PIOS_COM_DEBUG) && (PIOS_COM_DEBUG == PIOS_COM_SERIAL))
			if (!usb_comms)
			{	// the serial-port is being used for debugging - don't send data down it
				uavtalk_tx_buffer_wr = 0;
				uavtalk_tx_timer = 0;
				continue;
			}
		#endif
*/
		if (!usb_comms && !GPIO_IN(SERIAL_CTS_PIN))
			break;	// we can't yet send data down the comm-port

		// send the data out the comm-port
		int32_t res;
//		if (usb_comms)
//			res = PIOS_COM_SendBuffer(comm_port, uavtalk_tx_buffer, packet_size);
//		else
			res = PIOS_COM_SendBufferNonBlocking(comm_port, uavtalk_tx_buffer, packet_size);	// this one doesn't work properly with USB :(
		if (res < 0)
		{	// failed to send the data out the comm-port
			#if defined(UAVTALK_DEBUG)
				DEBUG_PRINTF("PIOS_COM_SendBuffer %d %d\r\n", packet_size, res);
			#endif

			if (uavtalk_tx_timer >= 5000)
			{	// seems we can't send our data for at least the last 5 seconds - delete it
				if (uavtalk_tx_buffer_wr > packet_size)
				{
					uavtalk_tx_buffer_wr -= packet_size;
					memmove(uavtalk_tx_buffer, uavtalk_tx_buffer + packet_size, uavtalk_tx_buffer_wr);
				}
				else
					uavtalk_tx_buffer_wr = 0;
			}

			break;
		}

		// data was sent out the comm-port OK .. remove the sent data from our buffer

		if (uavtalk_tx_buffer_wr > packet_size)
		{
			uavtalk_tx_buffer_wr -= packet_size;
			memmove(uavtalk_tx_buffer, uavtalk_tx_buffer + packet_size, uavtalk_tx_buffer_wr);
		}
		else
			uavtalk_tx_buffer_wr = 0;

		uavtalk_tx_timer = 0;
	}

	// ********************
}

// *****************************************************************************

void uavtalk_init(void)
{
	uavtalk_previous_com_port = -1;

	uavtalk_rx_buffer_wr = 0;

	uavtalk_tx_buffer_wr = 0;

	uavtalk_rx_timer = 0;
	uavtalk_tx_timer = 0;
}

// *****************************************************************************

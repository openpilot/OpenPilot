/**
 ******************************************************************************
 *
 * @file       transparent_comms.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      Serial communication port handling routines
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

#include <string.h>

#include "stm32f10x.h"
#include "gpio_in.h"
#include "transparent_comms.h"
#include "packet_handler.h"
#include "saved_settings.h"
#include "main.h"
#include "pios_usb.h"		/* PIOS_USB_* */

#if defined(PIOS_COM_DEBUG)
  #define TRANS_DEBUG
#endif

// *****************************************************************************
// local variables

uint32_t                trans_previous_com_port = 0;

volatile uint16_t       trans_rx_timer = 0;
volatile uint16_t       trans_tx_timer = 0;

uint8_t                 trans_temp_buffer1[128];

uint8_t                 trans_temp_buffer2[128];
uint16_t                trans_temp_buffer2_wr;

// *****************************************************************************
// can be called from an interrupt if you wish

void trans_1ms_tick(void)
{   // call this once every 1ms
    if (trans_rx_timer < 0xffff) trans_rx_timer++;
    if (trans_tx_timer < 0xffff) trans_tx_timer++;
}

// *****************************************************************************
// call this as often as possible - not from an interrupt

void trans_process(void)
{   // copy data from comm-port RX buffer to RF packet handler TX buffer, and from RF packet handler RX buffer to comm-port TX buffer

    // ********************
    // decide which comm-port we are using (usart or usb)

    bool usb_comms = false;						// TRUE if we are using the usb port for comms.
    uint32_t comm_port = PIOS_COM_SERIAL;		// default to using the usart comm-port

    #if defined(PIOS_INCLUDE_USB)
        if (PIOS_USB_CheckAvailable(0))
        {	// USB comms is up, use the USB comm-port instead of the USART comm-port
            usb_comms = true;
            comm_port = PIOS_COM_TELEM_USB;
        }
    #endif

    // ********************
    // check to see if the local communication port has changed (usart/usb)

    if (trans_previous_com_port == 0 && trans_previous_com_port != comm_port)
    {	// the local communications port has changed .. remove any data in the buffers
        trans_temp_buffer2_wr = 0;
    }
    else
	if (usb_comms)
	{	// we're using the USB for comms - keep the USART rx buffer empty
		int32_t bytes = PIOS_COM_ReceiveBufferUsed(PIOS_COM_SERIAL);
		while (bytes > 0)
		{
			uint8_t c;
			PIOS_COM_ReceiveBuffer(PIOS_COM_SERIAL, &c, 1, 0);
			bytes--;
		}
	}

	trans_previous_com_port = comm_port;			// remember the current comm-port we are using

	// ********************

	uint16_t connection_index = 0;					// the RF connection we are using

	// ********************
	// send the data received down the comm-port to the RF packet handler TX buffer

	if (saved_settings.mode == MODE_NORMAL || saved_settings.mode == MODE_STREAM_TX)
	{
		// free space size in the RF packet handler tx buffer
		uint16_t ph_num = ph_putData_free(connection_index);

		// get the number of data bytes received down the comm-port
		int32_t com_num = PIOS_COM_ReceiveBufferUsed(comm_port);

		// set the USART RTS handshaking line
		if (!usb_comms)
		{
			if (ph_num < 32 || !ph_connected(connection_index))
				SERIAL_RTS_CLEAR;						// lower the USART RTS line - we don't have space in the buffer for anymore bytes
			else
				SERIAL_RTS_SET;							// release the USART RTS line - we have space in the buffer for now bytes
		}
		else
			SERIAL_RTS_SET;								// release the USART RTS line

		// limit number of bytes we will get to the size of the temp buffer
		if (com_num > sizeof(trans_temp_buffer1))
			com_num = sizeof(trans_temp_buffer1);

		// limit number of bytes we will get to the size of the free space in the RF packet handler TX buffer
		if (com_num > ph_num)
			com_num = ph_num;

		// copy data received down the comm-port into our temp buffer
		register uint16_t bytes_saved = 0;
		bytes_saved = PIOS_COM_ReceiveBuffer(comm_port, trans_temp_buffer1, com_num, 0);

		// put the received comm-port data bytes into the RF packet handler TX buffer
		if (bytes_saved > 0)
		{
			trans_rx_timer = 0;
			ph_putData(connection_index, trans_temp_buffer1, bytes_saved);
		}
	}
	else
	{	// empty the comm-ports rx buffer
		int32_t com_num = PIOS_COM_ReceiveBufferUsed(comm_port);
		while (com_num > 0) {
			uint8_t c;
			PIOS_COM_ReceiveBuffer(comm_port, &c, 1, 0);
			com_num--;
		}
	}

	// ********************
	// send the data received via the RF link out the comm-port

	if (saved_settings.mode == MODE_NORMAL || saved_settings.mode == MODE_STREAM_RX)
	{
		if (trans_temp_buffer2_wr < sizeof(trans_temp_buffer2))
		{
			// get number of data bytes received via the RF link
			uint16_t ph_num = ph_getData_used(connection_index);

			// limit to how much space we have in the temp buffer
			if (ph_num > sizeof(trans_temp_buffer2) - trans_temp_buffer2_wr)
				ph_num = sizeof(trans_temp_buffer2) - trans_temp_buffer2_wr;

			if (ph_num > 0)
			{	// fetch the data bytes received via the RF link and save into our temp buffer
				ph_num = ph_getData(connection_index, trans_temp_buffer2 + trans_temp_buffer2_wr, ph_num);
				trans_temp_buffer2_wr += ph_num;
			}
		}

		#if (defined(PIOS_COM_DEBUG) && (PIOS_COM_DEBUG == PIOS_COM_SERIAL))
			if (!usb_comms)
			{	// the serial-port is being used for debugging - don't send data down it
				trans_temp_buffer2_wr = 0;
				trans_tx_timer = 0;
				return;
			}
		#endif

		if (trans_temp_buffer2_wr > 0)
		{	// we have data in our temp buffer that needs sending out the comm-port

			if (usb_comms || (!usb_comms && GPIO_IN(SERIAL_CTS_PIN)))
			{	// we are OK to send the data out the comm-port

				// send the data out the comm-port
				int32_t res = PIOS_COM_SendBufferNonBlocking(comm_port, trans_temp_buffer2, trans_temp_buffer2_wr);	// this one doesn't work properly with USB :(
				if (res >= 0)
				{	// data was sent out the comm-port OK .. remove the sent data from the temp buffer
					trans_temp_buffer2_wr = 0;
					trans_tx_timer = 0;
				}
				else
				{	// failed to send the data out the comm-port
					#if defined(TRANS_DEBUG)
						DEBUG_PRINTF("PIOS_COM_SendBuffer %d %d\r\n", trans_temp_buffer2_wr, res);
					#endif

					if (trans_tx_timer >= 5000)
						trans_temp_buffer2_wr = 0;	// seems we can't send our data for at least the last 5 seconds - delete it
				}
			}
		}
	}
	else
	{	// empty the buffer
		trans_temp_buffer2_wr = 0;
		trans_tx_timer = 0;
	}

	// ********************
}

// *****************************************************************************

void trans_init(void)
{
	trans_previous_com_port = 0;

	trans_temp_buffer2_wr = 0;

	trans_rx_timer = 0;
	trans_tx_timer = 0;
}

// *****************************************************************************

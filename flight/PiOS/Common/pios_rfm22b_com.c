/**
******************************************************************************
* @addtogroup PIOS PIOS Core hardware abstraction layer
* @{
* @addtogroup   PIOS_RFM22B Radio Functions
* @brief PIOS COM interface for for the RFM22B radio
* @{
*
* @file       pios_rfm22b_com.c
* @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2012.
* @brief      Implements a driver the the RFM22B driver
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

#include <pios.h>

#ifdef PIOS_INCLUDE_RFM22B_COM

#include <pios_rfm22b_priv.h>

/* Provide a COM driver */
static void PIOS_RFM22B_COM_ChangeBaud(uint32_t rfm22b_id, uint32_t baud);
static void PIOS_RFM22B_COM_RegisterRxCallback(uint32_t rfm22b_id, pios_com_callback rx_in_cb, uint32_t context);
static void PIOS_RFM22B_COM_RegisterTxCallback(uint32_t rfm22b_id, pios_com_callback tx_out_cb, uint32_t context);
static void PIOS_RFM22B_COM_TxStart(uint32_t rfm22b_id, uint16_t tx_bytes_avail);
static void PIOS_RFM22B_COM_RxStart(uint32_t rfm22b_id, uint16_t rx_bytes_avail);
static bool PIOS_RFM22B_COM_Available(uint32_t rfm22b_com_id);

/* Local variables */
const struct pios_com_driver pios_rfm22b_com_driver = {
	.set_baud   = PIOS_RFM22B_COM_ChangeBaud,
	.tx_start   = PIOS_RFM22B_COM_TxStart,
	.rx_start   = PIOS_RFM22B_COM_RxStart,
	.bind_tx_cb = PIOS_RFM22B_COM_RegisterTxCallback,
	.bind_rx_cb = PIOS_RFM22B_COM_RegisterRxCallback,
	.available  = PIOS_RFM22B_COM_Available
};

/**
 * Changes the baud rate of the RFM22B peripheral without re-initialising.
 * \param[in] rfm22b_id RFM22B name (GPS, TELEM, AUX)
 * \param[in] baud Requested baud rate
 */
static void PIOS_RFM22B_COM_ChangeBaud(uint32_t rfm22b_id, uint32_t baud)
{
	struct pios_rfm22b_dev *rfm22b_dev = (struct pios_rfm22b_dev *)rfm22b_id;
	if (!PIOS_RFM22B_validate(rfm22b_dev))
		return;
	// Set the RF data rate on the modem to ~2X the selected buad rate because the modem is half duplex.
	enum rfm22b_datarate datarate = RFM22_datarate_64000;
	if (baud <= 1024)
		datarate = RFM22_datarate_500;
	else if (baud <= 2048)
		datarate = RFM22_datarate_1000;
	else if (baud <= 4096)
		datarate = RFM22_datarate_8000;
	else if (baud <= 9600)
		datarate = RFM22_datarate_16000;
	else if (baud <= 19200)
		datarate = RFM22_datarate_32000;
	else if (baud <= 38400)
		datarate = RFM22_datarate_57600;
	else if (baud <= 57600)
		datarate = RFM22_datarate_128000;
	else if (baud <= 115200)
		datarate = RFM22_datarate_192000;
	rfm22b_dev->datarate = datarate;
}

static void PIOS_RFM22B_COM_RxStart(uint32_t rfm22b_id, uint16_t rx_bytes_avail)
{
}

static void PIOS_RFM22B_COM_TxStart(uint32_t rfm22b_id, uint16_t tx_bytes_avail)
{
	struct pios_rfm22b_dev *rfm22b_dev = (struct pios_rfm22b_dev *)rfm22b_id;
	if (!PIOS_RFM22B_validate(rfm22b_dev))
		return;
}

static void PIOS_RFM22B_COM_RegisterRxCallback(uint32_t rfm22b_id, pios_com_callback rx_in_cb, uint32_t context)
{
	struct pios_rfm22b_dev *rfm22b_dev = (struct pios_rfm22b_dev *)rfm22b_id;
	if (!PIOS_RFM22B_validate(rfm22b_dev))
		return;

	/* 
	 * Order is important in these assignments since ISR uses _cb
	 * field to determine if it's ok to dereference _cb and _context
	 */
	rfm22b_dev->rx_in_context = context;
	rfm22b_dev->rx_in_cb = rx_in_cb;
}

static void PIOS_RFM22B_COM_RegisterTxCallback(uint32_t rfm22b_id, pios_com_callback tx_out_cb, uint32_t context)
{
	struct pios_rfm22b_dev *rfm22b_dev = (struct pios_rfm22b_dev *)rfm22b_id;
	if (!PIOS_RFM22B_validate(rfm22b_dev))
		return;

	/* 
	 * Order is important in these assignments since ISR uses _cb
	 * field to determine if it's ok to dereference _cb and _context
	 */
	rfm22b_dev->tx_out_context = context;
	rfm22b_dev->tx_out_cb = tx_out_cb;
}

static bool PIOS_RFM22B_COM_Available(uint32_t rfm22b_id)
{
	return PIOS_RFM22B_LinkStatus(rfm22b_id);
}

#endif /* PIOS_INCLUDE_RFM22B_COM */

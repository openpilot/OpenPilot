/**
 ******************************************************************************
 * @addtogroup PIOS PIOS Core hardware abstraction layer
 * @{
 * @addtogroup PIOS_COM COM MSG layer functions
 * @brief Hardware communication layer
 * @{
 *
 * @file       pios_com_msg.c  
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      COM MSG layer functions
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

/* Project Includes */
#include "pios.h"

#if defined(PIOS_INCLUDE_COM_MSG)

#include "pios_com.h"

#define PIOS_COM_MSG_MAX_LEN 63

struct pios_com_msg_dev {
	uint32_t lower_id;
	const struct pios_com_driver * driver;

	uint8_t rx_msg_buffer[PIOS_COM_MSG_MAX_LEN];
	volatile bool rx_msg_full;

	uint8_t tx_msg_buffer[PIOS_COM_MSG_MAX_LEN];
	volatile bool tx_msg_full;
};

static struct pios_com_msg_dev com_msg_dev;

static uint16_t PIOS_COM_MSG_TxOutCallback(uint32_t context, uint8_t * buf, uint16_t buf_len, uint16_t * headroom, bool * need_yield);
static uint16_t PIOS_COM_MSG_RxInCallback(uint32_t context, uint8_t * buf, uint16_t buf_len, uint16_t * headroom, bool * need_yield);

int32_t PIOS_COM_MSG_Init(uint32_t * com_id, const struct pios_com_driver * driver, uint32_t lower_id)
{
	PIOS_Assert(com_id);
	PIOS_Assert(driver);

	PIOS_Assert(driver->bind_tx_cb);
	PIOS_Assert(driver->bind_rx_cb);

	struct pios_com_msg_dev * com_dev = &com_msg_dev;

	com_dev->driver   = driver;
	com_dev->lower_id = lower_id;

	com_dev->rx_msg_full = false;
	(com_dev->driver->bind_rx_cb)(lower_id, PIOS_COM_MSG_RxInCallback, (uint32_t)com_dev);
	(com_dev->driver->rx_start)(com_dev->lower_id, sizeof(com_dev->rx_msg_buffer));

	com_dev->tx_msg_full = false;
	(com_dev->driver->bind_tx_cb)(lower_id, PIOS_COM_MSG_TxOutCallback, (uint32_t)com_dev);

	*com_id = (uint32_t)com_dev;
	return(0);
}

static uint16_t PIOS_COM_MSG_TxOutCallback(uint32_t context, uint8_t * buf, uint16_t buf_len, uint16_t * headroom, bool * need_yield)
{
	struct pios_com_msg_dev * com_dev = (struct pios_com_msg_dev *)context;

	PIOS_Assert(buf);
	PIOS_Assert(buf_len);

	uint16_t bytes_from_fifo = 0;

	if (com_dev->tx_msg_full && (buf_len >= sizeof(com_dev->tx_msg_buffer))) {
		/* Room for an entire message, send it */
		memcpy(buf, com_dev->tx_msg_buffer, sizeof(com_dev->tx_msg_buffer));
		bytes_from_fifo = sizeof(com_dev->tx_msg_buffer);
		com_dev->tx_msg_full = false;
	}

	if (headroom) {
		if (com_dev->tx_msg_full) {
			*headroom = sizeof(com_dev->tx_msg_buffer);
		} else {
			*headroom = 0;
		}
	}

	return (bytes_from_fifo);
}

static uint16_t PIOS_COM_MSG_RxInCallback(uint32_t context, uint8_t * buf, uint16_t buf_len, uint16_t * headroom, bool * need_yield)
{
	struct pios_com_msg_dev * com_dev = (struct pios_com_msg_dev *)context;

	uint16_t bytes_into_fifo = 0;

	if (!com_dev->rx_msg_full && (buf_len >= sizeof(com_dev->rx_msg_buffer))) {
		memcpy(com_dev->rx_msg_buffer, buf, sizeof(com_dev->rx_msg_buffer));
		bytes_into_fifo = sizeof(com_dev->rx_msg_buffer);
		com_dev->rx_msg_full = true;
	}

	if (headroom) {
		if (!com_dev->rx_msg_full) {
			*headroom = sizeof(com_dev->rx_msg_buffer);
		} else {
			*headroom = 0;
		}
	}

	return (bytes_into_fifo);
}

int32_t PIOS_COM_MSG_Send(uint32_t com_id, const uint8_t *msg, uint16_t msg_len)
{
	PIOS_Assert(msg);
	PIOS_Assert(msg_len);

	struct pios_com_msg_dev * com_dev = (struct pios_com_msg_dev *)com_id;

	PIOS_Assert(msg_len == sizeof(com_dev->tx_msg_buffer));

	/* Wait forever for room in the tx buffer */
	while (com_dev->tx_msg_full) {
		/* Kick the transmitter while we wait */
		if (com_dev->driver->tx_start) {
			(com_dev->driver->tx_start)(com_dev->lower_id, sizeof(com_dev->tx_msg_buffer));
		}
	}

	memcpy((void *) com_dev->tx_msg_buffer, msg, msg_len);
	com_dev->tx_msg_full = true;

	/* Kick the transmitter now that we've queued our message */
	if (com_dev->driver->tx_start) {
		(com_dev->driver->tx_start)(com_dev->lower_id, sizeof(com_dev->tx_msg_buffer));
	}

	return 0;
}

uint16_t PIOS_COM_MSG_Receive(uint32_t com_id, uint8_t * msg, uint16_t msg_len)
{
	PIOS_Assert(msg);
	PIOS_Assert(msg_len);

	struct pios_com_msg_dev * com_dev = (struct pios_com_msg_dev *)com_id;

	PIOS_Assert(msg_len == sizeof(com_dev->rx_msg_buffer));

	if (!com_dev->rx_msg_full) {
		/* There's room in our buffer, kick the receiver */
		(com_dev->driver->rx_start)(com_dev->lower_id, sizeof(com_dev->rx_msg_buffer));
	} else {
		memcpy(msg, com_dev->rx_msg_buffer, msg_len);
		com_dev->rx_msg_full = false;

		return msg_len;
	}

	return 0;
}

#endif	/* PIOS_INCLUDE_COM_MSG */

/**
 * @}
 * @}
 */

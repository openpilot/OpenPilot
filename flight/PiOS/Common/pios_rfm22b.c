/**
 ******************************************************************************
 * @addtogroup PIOS PIOS Core hardware abstraction layer
 * @{
 * @addtogroup   PIOS_RFM22B Radio Functions
 * @brief PIOS interface for for the RFM22B radio
 * @{
 *
 * @file       pios_rfm22b.c
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

/* Project Includes */
#include "pios.h"

#if defined(PIOS_INCLUDE_RFM22B)

#include <pios_rfm22b_priv.h>

/* Provide a COM driver */
static void PIOS_RFM22B_ChangeBaud(uint32_t rfm22b_id, uint32_t baud);
static void PIOS_RFM22B_RegisterRxCallback(uint32_t rfm22b_id, pios_com_callback rx_in_cb, uint32_t context);
static void PIOS_RFM22B_RegisterTxCallback(uint32_t rfm22b_id, pios_com_callback tx_out_cb, uint32_t context);
static void PIOS_RFM22B_TxStart(uint32_t rfm22b_id, uint16_t tx_bytes_avail);
static void PIOS_RFM22B_RxStart(uint32_t rfm22b_id, uint16_t rx_bytes_avail);

static void PIOS_RFM22B_Timer_Callback(uint32_t dev_id);

const struct pios_com_driver pios_rfm22b_com_driver = {
	.set_baud   = PIOS_RFM22B_ChangeBaud,
	.tx_start   = PIOS_RFM22B_TxStart,
	.rx_start   = PIOS_RFM22B_RxStart,
	.bind_tx_cb = PIOS_RFM22B_RegisterTxCallback,
	.bind_rx_cb = PIOS_RFM22B_RegisterRxCallback,
};

enum pios_rfm22b_dev_magic {
	PIOS_RFM22B_DEV_MAGIC = 0x68e971b6,
};

struct pios_rfm22b_dev {
	enum pios_rfm22b_dev_magic magic;
	const struct pios_rfm22b_cfg *cfg;

	uint32_t countdown_timer;

	pios_com_callback rx_in_cb;
	uint32_t rx_in_context;
	pios_com_callback tx_out_cb;
	uint32_t tx_out_context;

	uint32_t rx_dropped;
};

static bool PIOS_RFM22B_validate(struct pios_rfm22b_dev * rfm22b_dev)
{
	return (rfm22b_dev->magic == PIOS_RFM22B_DEV_MAGIC);
}

#if defined(PIOS_INCLUDE_FREERTOS)
static struct pios_rfm22b_dev * PIOS_RFM22B_alloc(void)
{
	struct pios_rfm22b_dev * rfm22b_dev;

	rfm22b_dev = (struct pios_rfm22b_dev *)pvPortMalloc(sizeof(*rfm22b_dev));
	if (!rfm22b_dev) return(NULL);

	rfm22b_dev->magic = PIOS_RFM22B_DEV_MAGIC;
	return(rfm22b_dev);
}
#else
static struct pios_rfm22b_dev pios_rfm22b_devs[PIOS_RFM22B_MAX_DEVS];
static uint8_t pios_rfm22b_num_devs;
static struct pios_rfm22b_dev * PIOS_RFM22B_alloc(void)
{
	struct pios_rfm22b_dev * rfm22b_dev;

	if (pios_rfm22b_num_devs >= PIOS_RFM22B_MAX_DEVS) {
		return (NULL);
	}

	rfm22b_dev = &pios_rfm22b_devs[pios_rfm22b_num_devs++];
	rfm22b_dev->magic = PIOS_RFM22B_DEV_MAGIC;

	return (rfm22b_dev);
}
#endif

/**
* Initialise an RFM22B device
*/
int32_t PIOS_RFM22B_Init(uint32_t *rfm22b_id, const struct pios_rfm22b_cfg *cfg)
{
	PIOS_DEBUG_Assert(rfm22b_id);
	PIOS_DEBUG_Assert(cfg);

	struct pios_rfm22b_dev * rfm22b_dev;

	rfm22b_dev = (struct pios_rfm22b_dev *) PIOS_RFM22B_alloc();
	if (!rfm22b_dev)
		return(-1);

	/* Bind the configuration to the device instance */
	rfm22b_dev->cfg = cfg;

	/* Configure the countdown timer and register the tick callback. */
	rfm22b_dev->countdown_timer = (uint32_t)((float)(rfm22b_dev->cfg->send_timeout) / 0.625);
	if (!PIOS_RTC_RegisterTickCallback(PIOS_RFM22B_Timer_Callback, (uint32_t)rfm22b_dev)) {
		PIOS_DEBUG_Assert(0);
	}
  
	*rfm22b_id = (uint32_t)rfm22b_dev;

	return(0);
}

static void PIOS_RFM22B_RxStart(uint32_t rfm22b_id, uint16_t rx_bytes_avail)
{
	struct pios_rfm22b_dev * rfm22b_dev = (struct pios_rfm22b_dev *)rfm22b_id;

	bool valid = PIOS_RFM22B_validate(rfm22b_dev);
	PIOS_Assert(valid);

#ifdef NEVER
	RFM22B_ITConfig(rfm22b_dev->cfg->regs, RFM22B_IT_RXNE, ENABLE);
#endif
}
static void PIOS_RFM22B_TxStart(uint32_t rfm22b_id, uint16_t tx_bytes_avail)
{
	struct pios_rfm22b_dev * rfm22b_dev = (struct pios_rfm22b_dev *)rfm22b_id;

	bool valid = PIOS_RFM22B_validate(rfm22b_dev);
	PIOS_Assert(valid);

#ifdef NEVER
	RFM22B_ITConfig(rfm22b_dev->cfg->regs, RFM22B_IT_TXE, ENABLE);
#endif
}

/**
* Changes the baud rate of the RFM22B peripheral without re-initialising.
* \param[in] rfm22b_id RFM22B name (GPS, TELEM, AUX)
* \param[in] baud Requested baud rate
*/
static void PIOS_RFM22B_ChangeBaud(uint32_t rfm22b_id, uint32_t baud)
{
	struct pios_rfm22b_dev * rfm22b_dev = (struct pios_rfm22b_dev *)rfm22b_id;

	bool valid = PIOS_RFM22B_validate(rfm22b_dev);
	PIOS_Assert(valid);

#ifdef NEVER
	RFM22B_InitTypeDef RFM22B_InitStructure;

	/* Start with a copy of the default configuration for the peripheral */
	RFM22B_InitStructure = rfm22b_dev->cfg->init;

	/* Adjust the baud rate */
	RFM22B_InitStructure.RFM22B_BaudRate = baud;

	/* Write back the new configuration */
	RFM22B_Init(rfm22b_dev->cfg->regs, &RFM22B_InitStructure);
#endif
}

static void PIOS_RFM22B_RegisterRxCallback(uint32_t rfm22b_id, pios_com_callback rx_in_cb, uint32_t context)
{
#ifdef NEVER
	struct pios_rfm22b_dev * rfm22b_dev = (struct pios_rfm22b_dev *)rfm22b_id;

	bool valid = PIOS_RFM22B_validate(rfm22b_dev);
	PIOS_Assert(valid);

	/* 
	 * Order is important in these assignments since ISR uses _cb
	 * field to determine if it's ok to dereference _cb and _context
	 */
	rfm22b_dev->rx_in_context = context;
	rfm22b_dev->rx_in_cb = rx_in_cb;
#endif
}

static void PIOS_RFM22B_RegisterTxCallback(uint32_t rfm22b_id, pios_com_callback tx_out_cb, uint32_t context)
{
#ifdef NEVER
	struct pios_rfm22b_dev * rfm22b_dev = (struct pios_rfm22b_dev *)rfm22b_id;

	bool valid = PIOS_RFM22B_validate(rfm22b_dev);
	PIOS_Assert(valid);

	/* 
	 * Order is important in these assignments since ISR uses _cb
	 * field to determine if it's ok to dereference _cb and _context
	 */
	rfm22b_dev->tx_out_context = context;
	rfm22b_dev->tx_out_cb = tx_out_cb;
#endif
}

static void PIOS_RFM22B_Timer_Callback(uint32_t dev_id) {
	/* Recover our device context */
	struct pios_rfm22b_dev *rfm22b_dev = (struct pios_rfm22b_dev *)dev_id;

	if (!PIOS_RFM22B_validate(rfm22b_dev)) {
		/* Invalid device specified */
		return;
	}

	/* 
	 * RTC runs at 625Hz.
	 */
	if(--rfm22b_dev->countdown_timer > 0)
		return;
	rfm22b_dev->countdown_timer = (uint32_t)((float)(rfm22b_dev->cfg->send_timeout) / 0.625);
	PIOS_COM_SendString(PIOS_COM_TELEM_SERIAL, "Hello Telem\n\r");
}

#endif

/**
  * @}
  * @}
  */

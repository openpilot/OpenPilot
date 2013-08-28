/**
 ******************************************************************************
 * @addtogroup PIOS PIOS Core hardware abstraction layer
 * @{
 * @addtogroup   PIOS_USB_HID USB COM Functions
 * @brief PIOS USB COM implementation for HID interfaces
 * @notes      This implements serial emulation over HID reports
 * @{
 *
 * @file       pios_usb_hid.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      USB COM functions (STM32 dependent code)
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

#if defined(PIOS_INCLUDE_USB_HID)

#include "pios_usb.h"
#include "pios_usb_hid_priv.h"
#include "pios_usb_board_data.h" /* PIOS_BOARD_*_DATA_LENGTH */

/* STM32 USB Library Definitions */
#include "usb_lib.h"

static void PIOS_USB_HID_RegisterTxCallback(uint32_t usbhid_id, pios_com_callback tx_out_cb, uint32_t context);
static void PIOS_USB_HID_RegisterRxCallback(uint32_t usbhid_id, pios_com_callback rx_in_cb, uint32_t context);
static void PIOS_USB_HID_TxStart(uint32_t usbhid_id, uint16_t tx_bytes_avail);
static void PIOS_USB_HID_RxStart(uint32_t usbhid_id, uint16_t rx_bytes_avail);

const struct pios_com_driver pios_usb_hid_com_driver = {
	.tx_start    = PIOS_USB_HID_TxStart,
	.rx_start    = PIOS_USB_HID_RxStart,
	.bind_tx_cb  = PIOS_USB_HID_RegisterTxCallback,
	.bind_rx_cb  = PIOS_USB_HID_RegisterRxCallback,
};

enum pios_usb_hid_dev_magic {
	PIOS_USB_HID_DEV_MAGIC = 0xAA00BB00,
};

struct pios_usb_hid_dev {
	enum pios_usb_hid_dev_magic     magic;
	const struct pios_usb_hid_cfg * cfg;

	uint32_t lower_id;

	pios_com_callback rx_in_cb;
	uint32_t rx_in_context;
	pios_com_callback tx_out_cb;
	uint32_t tx_out_context;

	uint8_t rx_packet_buffer[PIOS_USB_BOARD_HID_DATA_LENGTH];
	uint8_t tx_packet_buffer[PIOS_USB_BOARD_HID_DATA_LENGTH];

	uint32_t rx_dropped;
	uint32_t rx_oversize;
};

static bool PIOS_USB_HID_validate(struct pios_usb_hid_dev * usb_hid_dev)
{
	return (usb_hid_dev->magic == PIOS_USB_HID_DEV_MAGIC);
}

#if defined(PIOS_INCLUDE_FREERTOS)
static struct pios_usb_hid_dev * PIOS_USB_HID_alloc(void)
{
	struct pios_usb_hid_dev * usb_hid_dev;

	usb_hid_dev = (struct pios_usb_hid_dev *)pvPortMalloc(sizeof(*usb_hid_dev));
	if (!usb_hid_dev) return(NULL);

	usb_hid_dev->magic = PIOS_USB_HID_DEV_MAGIC;
	return(usb_hid_dev);
}
#else
static struct pios_usb_hid_dev pios_usb_hid_devs[PIOS_USB_HID_MAX_DEVS];
static uint8_t pios_usb_hid_num_devs;
static struct pios_usb_hid_dev * PIOS_USB_HID_alloc(void)
{
	struct pios_usb_hid_dev * usb_hid_dev;

	if (pios_usb_hid_num_devs >= PIOS_USB_HID_MAX_DEVS) {
		return (NULL);
	}

	usb_hid_dev = &pios_usb_hid_devs[pios_usb_hid_num_devs++];
	usb_hid_dev->magic = PIOS_USB_HID_DEV_MAGIC;

	return (usb_hid_dev);
}
#endif

static void PIOS_USB_HID_EP_IN_Callback(void);
static void PIOS_USB_HID_EP_OUT_Callback(void);

static uint32_t pios_usb_hid_id;

/* Need a better way to pull these in */
extern void (*pEpInt_IN[7])(void);
extern void (*pEpInt_OUT[7])(void);

int32_t PIOS_USB_HID_Init(uint32_t * usbhid_id, const struct pios_usb_hid_cfg * cfg, uint32_t lower_id)
{
	PIOS_Assert(usbhid_id);
	PIOS_Assert(cfg);

	struct pios_usb_hid_dev * usb_hid_dev;

	usb_hid_dev = (struct pios_usb_hid_dev *) PIOS_USB_HID_alloc();
	if (!usb_hid_dev) goto out_fail;

	/* Bind the configuration to the device instance */
	usb_hid_dev->cfg = cfg;
	usb_hid_dev->lower_id = lower_id;

	pios_usb_hid_id = (uint32_t) usb_hid_dev;

	/* Bind lower level callbacks into the USB infrastructure */
	pEpInt_IN[cfg->data_tx_ep - 1] = PIOS_USB_HID_EP_IN_Callback;
	pEpInt_OUT[cfg->data_rx_ep - 1] = PIOS_USB_HID_EP_OUT_Callback;

	*usbhid_id = (uint32_t) usb_hid_dev;

	return 0;

out_fail:
	return -1;
}




static void PIOS_USB_HID_SendReport(struct pios_usb_hid_dev * usb_hid_dev)
{
	uint16_t bytes_to_tx;

	if (!usb_hid_dev->tx_out_cb) {
		return;
	}

	bool need_yield = false;
#ifdef PIOS_USB_BOARD_BL_HID_HAS_NO_LENGTH_BYTE
	bytes_to_tx = (usb_hid_dev->tx_out_cb)(usb_hid_dev->tx_out_context,
					       &usb_hid_dev->tx_packet_buffer[1],
					       sizeof(usb_hid_dev->tx_packet_buffer)-1,
					       NULL,
					       &need_yield);
#else
	bytes_to_tx = (usb_hid_dev->tx_out_cb)(usb_hid_dev->tx_out_context,
					       &usb_hid_dev->tx_packet_buffer[2],
					       sizeof(usb_hid_dev->tx_packet_buffer)-2,
					       NULL,
					       &need_yield);
#endif
	if (bytes_to_tx == 0) {
		return;
	}

	/* Always set type as report ID */
	usb_hid_dev->tx_packet_buffer[0] = 1;

#ifdef PIOS_USB_BOARD_BL_HID_HAS_NO_LENGTH_BYTE
	UserToPMABufferCopy(usb_hid_dev->tx_packet_buffer,
			GetEPTxAddr(usb_hid_dev->cfg->data_tx_ep),
			bytes_to_tx + 1);
#else
	usb_hid_dev->tx_packet_buffer[1] = bytes_to_tx;
	UserToPMABufferCopy(usb_hid_dev->tx_packet_buffer,
			GetEPTxAddr(usb_hid_dev->cfg->data_tx_ep),
			bytes_to_tx + 2);
#endif
	/* Is this correct?  Why do we always send the whole buffer? */
	SetEPTxCount(usb_hid_dev->cfg->data_tx_ep, sizeof(usb_hid_dev->tx_packet_buffer));
	SetEPTxValid(usb_hid_dev->cfg->data_tx_ep);

#if defined(PIOS_INCLUDE_FREERTOS)
	if (need_yield) {
		vPortYieldFromISR();
	}
#endif	/* PIOS_INCLUDE_FREERTOS */
}

static void PIOS_USB_HID_RxStart(uint32_t usbhid_id, uint16_t rx_bytes_avail) {
	struct pios_usb_hid_dev * usb_hid_dev = (struct pios_usb_hid_dev *)usbhid_id;

	bool valid = PIOS_USB_HID_validate(usb_hid_dev);
	PIOS_Assert(valid);

	if (!PIOS_USB_CheckAvailable(usb_hid_dev->lower_id)) {
		return;
	}

	// If endpoint was stalled and there is now space make it valid
#ifdef PIOS_USB_BOARD_BL_HID_HAS_NO_LENGTH_BYTE
	uint16_t max_payload_length = PIOS_USB_BOARD_HID_DATA_LENGTH - 1;
#else
	uint16_t max_payload_length = PIOS_USB_BOARD_HID_DATA_LENGTH - 2;
#endif

	PIOS_IRQ_Disable();
	if ((GetEPRxStatus(usb_hid_dev->cfg->data_rx_ep) != EP_RX_VALID) && 
	    (rx_bytes_avail >= max_payload_length)) {
		SetEPRxStatus(usb_hid_dev->cfg->data_rx_ep, EP_RX_VALID);
	}
	PIOS_IRQ_Enable();
}

static void PIOS_USB_HID_TxStart(uint32_t usbhid_id, uint16_t tx_bytes_avail)
{
	struct pios_usb_hid_dev * usb_hid_dev = (struct pios_usb_hid_dev *)usbhid_id;

	bool valid = PIOS_USB_HID_validate(usb_hid_dev);
	PIOS_Assert(valid);

	if (!PIOS_USB_CheckAvailable(usb_hid_dev->lower_id)) {
		return;
	}

	if (GetEPTxStatus(usb_hid_dev->cfg->data_tx_ep) == EP_TX_VALID) {
		/* Endpoint is already transmitting */
		return;
	}

	PIOS_USB_HID_SendReport(usb_hid_dev);
}

static void PIOS_USB_HID_RegisterRxCallback(uint32_t usbhid_id, pios_com_callback rx_in_cb, uint32_t context)
{
	struct pios_usb_hid_dev * usb_hid_dev = (struct pios_usb_hid_dev *)usbhid_id;

	bool valid = PIOS_USB_HID_validate(usb_hid_dev);
	PIOS_Assert(valid);

	/* 
	 * Order is important in these assignments since ISR uses _cb
	 * field to determine if it's ok to dereference _cb and _context
	 */
	usb_hid_dev->rx_in_context = context;
	usb_hid_dev->rx_in_cb = rx_in_cb;
}

static void PIOS_USB_HID_RegisterTxCallback(uint32_t usbhid_id, pios_com_callback tx_out_cb, uint32_t context)
{
	struct pios_usb_hid_dev * usb_hid_dev = (struct pios_usb_hid_dev *)usbhid_id;

	bool valid = PIOS_USB_HID_validate(usb_hid_dev);
	PIOS_Assert(valid);

	/* 
	 * Order is important in these assignments since ISR uses _cb
	 * field to determine if it's ok to dereference _cb and _context
	 */
	usb_hid_dev->tx_out_context = context;
	usb_hid_dev->tx_out_cb = tx_out_cb;
}

/**
 * @brief Callback used to indicate a transmission from device INto host completed
 * Checks if any data remains, pads it into HID packet and sends.
 */
static void PIOS_USB_HID_EP_IN_Callback(void)
{
	struct pios_usb_hid_dev * usb_hid_dev = (struct pios_usb_hid_dev *)pios_usb_hid_id;

	bool valid = PIOS_USB_HID_validate(usb_hid_dev);
	PIOS_Assert(valid);

	if (!PIOS_USB_CheckAvailable(usb_hid_dev->lower_id)) {
		return;
	}

	PIOS_USB_HID_SendReport(usb_hid_dev);
}

/**
 * EP1 OUT Callback Routine
 */
static void PIOS_USB_HID_EP_OUT_Callback(void)
{
	struct pios_usb_hid_dev * usb_hid_dev = (struct pios_usb_hid_dev *)pios_usb_hid_id;

	bool valid = PIOS_USB_HID_validate(usb_hid_dev);
	PIOS_Assert(valid);

	uint32_t DataLength;

	/* Read received data (63 bytes) */
	/* Get the number of received data on the selected Endpoint */
	DataLength = GetEPRxCount(usb_hid_dev->cfg->data_rx_ep);
	if (DataLength > sizeof(usb_hid_dev->rx_packet_buffer)) {
		DataLength = sizeof(usb_hid_dev->rx_packet_buffer);
	}

	/* Use the memory interface function to read from the selected endpoint */
	PMAToUserBufferCopy((uint8_t *) usb_hid_dev->rx_packet_buffer,
			GetEPRxAddr(usb_hid_dev->cfg->data_rx_ep),
			DataLength);

	if (!usb_hid_dev->rx_in_cb) {
		/* No Rx call back registered, disable the receiver */
		SetEPRxStatus(usb_hid_dev->cfg->data_rx_ep, EP_RX_NAK);
		return;
	}

	/* The first byte is report ID (not checked), the second byte is the valid data length */
	uint16_t headroom;
	bool need_yield = false;
#ifdef PIOS_USB_BOARD_BL_HID_HAS_NO_LENGTH_BYTE
	(usb_hid_dev->rx_in_cb)(usb_hid_dev->rx_in_context,
				&usb_hid_dev->rx_packet_buffer[1],
				sizeof(usb_hid_dev->rx_packet_buffer)-1,
				&headroom,
				&need_yield);
#else
	(usb_hid_dev->rx_in_cb)(usb_hid_dev->rx_in_context,
				&usb_hid_dev->rx_packet_buffer[2],
				usb_hid_dev->rx_packet_buffer[1],
				&headroom,
				&need_yield);
#endif

#ifdef PIOS_USB_BOARD_BL_HID_HAS_NO_LENGTH_BYTE
	uint16_t max_payload_length = PIOS_USB_BOARD_HID_DATA_LENGTH - 1;
#else
	uint16_t max_payload_length = PIOS_USB_BOARD_HID_DATA_LENGTH - 2;
#endif

	if (headroom >= max_payload_length) {

		/* We have room for a maximum length message */
		SetEPRxStatus(usb_hid_dev->cfg->data_rx_ep, EP_RX_VALID);
	} else {
		/* Not enough room left for a message, apply backpressure */
		SetEPRxStatus(usb_hid_dev->cfg->data_rx_ep, EP_RX_NAK);
	}

#if defined(PIOS_INCLUDE_FREERTOS)
	if (need_yield) {
		vPortYieldFromISR();
	}
#endif	/* PIOS_INCLUDE_FREERTOS */
}

#endif	/* PIOS_INCLUDE_USB_HID */

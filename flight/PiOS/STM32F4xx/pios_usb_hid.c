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
#include "pios_usbhook.h"	 /* PIOS_USBHOOK_* */

#define MIN(x,y) (((x) < (y)) ? (x) : (y))
#define MAX(x,y) (((x) > (y)) ? (x) : (y))

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

	bool usb_if_enabled;

	uint8_t rx_packet_buffer[PIOS_USB_BOARD_HID_DATA_LENGTH];
	bool rx_active;

	uint8_t tx_packet_buffer[PIOS_USB_BOARD_HID_DATA_LENGTH];
	bool tx_active;

	uint32_t rx_dropped;
	uint32_t rx_oversize;
};

static bool PIOS_USB_HID_validate(struct pios_usb_hid_dev * usb_hid_dev)
{
	return (usb_hid_dev && (usb_hid_dev->magic == PIOS_USB_HID_DEV_MAGIC));
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

static void PIOS_USB_HID_IF_Init(uint32_t usb_hid_id);
static void PIOS_USB_HID_IF_DeInit(uint32_t usb_hid_id);
static bool PIOS_USB_HID_IF_Setup(uint32_t usb_hid_id, struct usb_setup_request *req);
static void PIOS_USB_HID_IF_CtrlDataOut(uint32_t usb_hid_id, struct usb_setup_request *req);

static struct pios_usb_ifops usb_hid_ifops = {
	.init          = PIOS_USB_HID_IF_Init,
	.deinit        = PIOS_USB_HID_IF_DeInit,
	.setup         = PIOS_USB_HID_IF_Setup,
	.ctrl_data_out = PIOS_USB_HID_IF_CtrlDataOut,
};

static bool PIOS_USB_HID_EP_IN_Callback(uint32_t usb_hid_id, uint8_t epnum, uint16_t len);
static bool PIOS_USB_HID_EP_OUT_Callback(uint32_t usb_hid_id, uint8_t epnum, uint16_t len);

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

	/* Rx and Tx are not active yet */
	usb_hid_dev->rx_active = false;
	usb_hid_dev->tx_active = false;

	/* Register class specific interface callbacks with the USBHOOK layer */
	usb_hid_dev->usb_if_enabled = false;
	PIOS_USBHOOK_RegisterIfOps(cfg->data_if, &usb_hid_ifops, (uint32_t) usb_hid_dev);

	*usbhid_id = (uint32_t) usb_hid_dev;

	return 0;

out_fail:
	return -1;
}


static struct pios_usbhook_descriptor hid_desc;

void PIOS_USB_HID_RegisterHidDescriptor(const uint8_t * desc, uint16_t length)
{
	hid_desc.descriptor = desc;
	hid_desc.length     = length;
}

static struct pios_usbhook_descriptor hid_report_desc;

void PIOS_USB_HID_RegisterHidReport(const uint8_t * desc, uint16_t length)
{
	hid_report_desc.descriptor = desc;
	hid_report_desc.length     = length;
}

static bool PIOS_USB_HID_SendReport(struct pios_usb_hid_dev * usb_hid_dev)
{
	uint16_t bytes_to_tx;

	if (!usb_hid_dev->tx_out_cb) {
		return false;
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
		return false;
	}

	/* 
	 * Mark this endpoint as being tx active _before_ actually transmitting
	 * to make sure we don't race with the Tx completion interrupt
	 */
	usb_hid_dev->tx_active = true;

	/* Always set type as report ID */
	usb_hid_dev->tx_packet_buffer[0] = 1;

#ifdef PIOS_USB_BOARD_BL_HID_HAS_NO_LENGTH_BYTE
	PIOS_USBHOOK_EndpointTx(usb_hid_dev->cfg->data_tx_ep,
				usb_hid_dev->tx_packet_buffer,
				sizeof(usb_hid_dev->tx_packet_buffer));
#else
	usb_hid_dev->tx_packet_buffer[1] = bytes_to_tx;
	PIOS_USBHOOK_EndpointTx(usb_hid_dev->cfg->data_tx_ep,
				usb_hid_dev->tx_packet_buffer,
				sizeof(usb_hid_dev->tx_packet_buffer));
#endif

#if defined(PIOS_INCLUDE_FREERTOS)
	if (need_yield) {
		vPortYieldFromISR();
	}
#endif	/* PIOS_INCLUDE_FREERTOS */

	return true;
}

static void PIOS_USB_HID_RxStart(uint32_t usbhid_id, uint16_t rx_bytes_avail) {
	struct pios_usb_hid_dev * usb_hid_dev = (struct pios_usb_hid_dev *)usbhid_id;

	bool valid = PIOS_USB_HID_validate(usb_hid_dev);
	PIOS_Assert(valid);

	/* Make sure this USB interface has been initialized */
	if (!usb_hid_dev->usb_if_enabled) {
		return;
	}

	if (!PIOS_USB_CheckAvailable(usb_hid_dev->lower_id)) {
		return;
	}

	// If endpoint was stalled and there is now space make it valid
#ifdef PIOS_USB_BOARD_BL_HID_HAS_NO_LENGTH_BYTE
	uint16_t max_payload_length = PIOS_USB_BOARD_HID_DATA_LENGTH - 1;
#else
	uint16_t max_payload_length = PIOS_USB_BOARD_HID_DATA_LENGTH - 2;
#endif

	if (!usb_hid_dev->rx_active && (rx_bytes_avail >= max_payload_length)) {
		PIOS_USBHOOK_EndpointRx(usb_hid_dev->cfg->data_rx_ep,
					usb_hid_dev->rx_packet_buffer,
					sizeof(usb_hid_dev->rx_packet_buffer));
		usb_hid_dev->rx_active = true;
	}
}

static void PIOS_USB_HID_TxStart(uint32_t usbhid_id, uint16_t tx_bytes_avail)
{
	struct pios_usb_hid_dev * usb_hid_dev = (struct pios_usb_hid_dev *)usbhid_id;

	bool valid = PIOS_USB_HID_validate(usb_hid_dev);
	PIOS_Assert(valid);

	/* Make sure this USB interface has been initialized */
	if (!usb_hid_dev->usb_if_enabled) {
		return;
	}

	if (!PIOS_USB_CheckAvailable(usb_hid_dev->lower_id)) {
		return;
	}

	if (!usb_hid_dev->tx_active) {
		/* Transmitter is not currently active, send a report */
		PIOS_USB_HID_SendReport(usb_hid_dev);
	}
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

static void PIOS_USB_HID_IF_Init(uint32_t usb_hid_id)
{
	struct pios_usb_hid_dev * usb_hid_dev = (struct pios_usb_hid_dev *)usb_hid_id;

	if (!PIOS_USB_HID_validate(usb_hid_dev)) {
		return;
	}

	/* Register endpoint specific callbacks with the USBHOOK layer */
	PIOS_USBHOOK_RegisterEpInCallback(usb_hid_dev->cfg->data_tx_ep,
					  sizeof(usb_hid_dev->tx_packet_buffer),
					  PIOS_USB_HID_EP_IN_Callback,
					  (uint32_t) usb_hid_dev);
	PIOS_USBHOOK_RegisterEpOutCallback(usb_hid_dev->cfg->data_rx_ep,
					   sizeof(usb_hid_dev->rx_packet_buffer),
					   PIOS_USB_HID_EP_OUT_Callback,
					   (uint32_t) usb_hid_dev);
	usb_hid_dev->usb_if_enabled = true;

}

static void PIOS_USB_HID_IF_DeInit(uint32_t usb_hid_id)
{
	struct pios_usb_hid_dev * usb_hid_dev = (struct pios_usb_hid_dev *)usb_hid_id;

	if (!PIOS_USB_HID_validate(usb_hid_dev)) {
		return;
	}

	/* DeRegister endpoint specific callbacks with the USBHOOK layer */
	usb_hid_dev->usb_if_enabled = false;
	PIOS_USBHOOK_DeRegisterEpInCallback(usb_hid_dev->cfg->data_tx_ep);
	PIOS_USBHOOK_DeRegisterEpOutCallback(usb_hid_dev->cfg->data_rx_ep);
}

static uint8_t hid_protocol;
static uint8_t hid_altset;

static bool PIOS_USB_HID_IF_Setup(uint32_t usb_hid_id, struct usb_setup_request *req)
{
	struct pios_usb_hid_dev * usb_hid_dev = (struct pios_usb_hid_dev *)usb_hid_id;

	if (!PIOS_USB_HID_validate(usb_hid_dev)) {
		return false;
	}

	/* Make sure this is a request for an interface we know about */
	uint8_t ifnum = req->wIndex & 0xFF;
	if (ifnum != usb_hid_dev->cfg->data_if) {
		return (false);
	}

	switch (req->bmRequestType & (USB_REQ_TYPE_MASK | USB_REQ_RECIPIENT_MASK)) {
	case (USB_REQ_TYPE_STANDARD | USB_REQ_RECIPIENT_INTERFACE):
		switch (req->bRequest) {
		case USB_REQ_GET_DESCRIPTOR:
			switch (req->wValue >> 8) {
			case USB_DESC_TYPE_REPORT:
				PIOS_USBHOOK_CtrlTx(hid_report_desc.descriptor,
						MIN(hid_report_desc.length, req->wLength));
				break;
			case USB_DESC_TYPE_HID:
				PIOS_USBHOOK_CtrlTx(hid_desc.descriptor,
						MIN(hid_desc.length, req->wLength));
				break;
			default:
				/* Unhandled descriptor request */
				return false;
				break;
			}
			break;
		case USB_REQ_GET_INTERFACE:
			PIOS_USBHOOK_CtrlTx(&hid_altset, 1);
			break;
		case USB_REQ_SET_INTERFACE:
			hid_altset = (uint8_t)(req->wValue);
			break;
		default:
			/* Unhandled standard request */
			return false;
			break;
		}
		break;
	case (USB_REQ_TYPE_CLASS    | USB_REQ_RECIPIENT_INTERFACE):
		switch (req->bRequest) {
		case USB_HID_REQ_SET_PROTOCOL:
			hid_protocol = (uint8_t)(req->wValue);
			break;
		case USB_HID_REQ_GET_PROTOCOL:
			PIOS_USBHOOK_CtrlTx(&hid_protocol, 1);
			break;
		case USB_HID_REQ_GET_REPORT:
		{
			/* Give back a dummy input report */
			uint8_t dummy_report[2] = {
				[0]     = req->wValue >> 8, /* Report ID */
				[1]     = 0x00,
			};
			PIOS_USBHOOK_CtrlTx(dummy_report, sizeof(dummy_report));
		}
		break;
		default:
			/* Unhandled class request */
			return false;
			break;
		}
		break;
	default:
		/* Unhandled request */
		return false;
	}

	return true;
}

static void PIOS_USB_HID_IF_CtrlDataOut(uint32_t usb_hid_id, struct usb_setup_request *req)
{
	/* HID devices don't have any OUT data stages on the control endpoint */
	PIOS_Assert(0);
}

/**
 * @brief Callback used to indicate a transmission from device INto host completed
 * Checks if any data remains, pads it into HID packet and sends.
 */
static bool PIOS_USB_HID_EP_IN_Callback(uint32_t usb_hid_id, uint8_t epnum, uint16_t len)
{
	struct pios_usb_hid_dev * usb_hid_dev = (struct pios_usb_hid_dev *)usb_hid_id;

	if (!PIOS_USB_HID_validate(usb_hid_dev)) {
		return false;
	}

	if (PIOS_USB_CheckAvailable(usb_hid_dev->lower_id) &&
		PIOS_USB_HID_SendReport(usb_hid_dev)) {
		/* More data has been queued, leave tx_active set to true */
		return true;
	} else {
		/* Nothing new sent, transmitter is now inactive */
		usb_hid_dev->tx_active = false;
		return false;
	}
}

/**
 * EP1 OUT Callback Routine
 */
static bool PIOS_USB_HID_EP_OUT_Callback(uint32_t usb_hid_id, uint8_t epnum, uint16_t len)
{
	struct pios_usb_hid_dev * usb_hid_dev = (struct pios_usb_hid_dev *)usb_hid_id;

	if (!PIOS_USB_HID_validate(usb_hid_dev)) {
		return false;
	}

	if (len > sizeof(usb_hid_dev->rx_packet_buffer)) {
		len = sizeof(usb_hid_dev->rx_packet_buffer);
	}

	if (!usb_hid_dev->rx_in_cb) {
		/* No Rx call back registered, disable the receiver */
		usb_hid_dev->rx_active = false;
		return false;
	}

	/* The first byte is report ID (not checked), the second byte is the valid data length */
	uint16_t headroom;
	bool need_yield = false;
#ifdef PIOS_USB_BOARD_BL_HID_HAS_NO_LENGTH_BYTE
	(usb_hid_dev->rx_in_cb)(usb_hid_dev->rx_in_context,
				&usb_hid_dev->rx_packet_buffer[1],
				len-1,
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

	bool rc;
	if (headroom >= max_payload_length) {
		/* We have room for a maximum length message */
		PIOS_USBHOOK_EndpointRx(usb_hid_dev->cfg->data_rx_ep,
					usb_hid_dev->rx_packet_buffer,
					sizeof(usb_hid_dev->rx_packet_buffer));
		rc = true;
	} else {
		/* Not enough room left for a message, apply backpressure */
		usb_hid_dev->rx_active = false;
		rc = false;
	}

#if defined(PIOS_INCLUDE_FREERTOS)
	if (need_yield) {
		vPortYieldFromISR();
	}
#endif	/* PIOS_INCLUDE_FREERTOS */

	return rc;
}

#endif	/* PIOS_INCLUDE_USB_HID */

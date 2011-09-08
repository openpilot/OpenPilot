/**
 ******************************************************************************
 * @addtogroup PIOS PIOS Core hardware abstraction layer
 * @{
 * @addtogroup   PIOS_USB_COM USB COM Functions
 * @brief PIOS USB COM implementation
 * @notes      This implements both a simple HID device and a CDC Serial Port
 * @{
 *
 * @file       pios_usb_com.c
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

#if defined(PIOS_INCLUDE_USB_COM)

#include "pios_usb.h"
#include "usb_lib.h"
#include "pios_usb_com_priv.h"

static void PIOS_USB_COM_RegisterTxCallback(uint32_t usbcom_id, pios_com_callback tx_out_cb, uint32_t context);
static void PIOS_USB_COM_RegisterRxCallback(uint32_t usbcom_id, pios_com_callback rx_in_cb, uint32_t context);

static void PIOS_USB_COM_HID_TxStart(uint32_t usbcom_id, uint16_t tx_bytes_avail);
static void PIOS_USB_COM_HID_RxStart(uint32_t usbcom_id, uint16_t rx_bytes_avail);

const struct pios_com_driver pios_usb_hid_com_driver = {
	.tx_start    = PIOS_USB_COM_HID_TxStart,
	.rx_start    = PIOS_USB_COM_HID_RxStart,
	.bind_tx_cb  = PIOS_USB_COM_RegisterTxCallback,
	.bind_rx_cb  = PIOS_USB_COM_RegisterRxCallback,
};

#if defined(PIOS_INCLUDE_USB_COM_CDC)

static void PIOS_USB_COM_CDC_TxStart(uint32_t usbcom_id, uint16_t tx_bytes_avail);
static void PIOS_USB_COM_CDC_RxStart(uint32_t usbcom_id, uint16_t rx_bytes_avail);

const struct pios_com_driver pios_usb_cdc_com_driver = {
	.tx_start    = PIOS_USB_COM_CDC_TxStart,
	.rx_start    = PIOS_USB_COM_CDC_RxStart,
	.bind_tx_cb  = PIOS_USB_COM_RegisterTxCallback,
	.bind_rx_cb  = PIOS_USB_COM_RegisterRxCallback,
};
#endif	/* PIOS_INCLUDE_USB_COM_CDC */

enum pios_usb_com_dev_magic {
	PIOS_USB_COM_DEV_MAGIC = 0xAABBCCDD,
};

struct pios_usb_com_dev {
	enum pios_usb_com_dev_magic     magic;
	const struct pios_usb_com_cfg * cfg;

	uint32_t lower_id;

	pios_com_callback rx_in_cb;
	uint32_t rx_in_context;
	pios_com_callback tx_out_cb;
	uint32_t tx_out_context;

	uint8_t rx_packet_buffer[PIOS_USB_COM_DATA_LENGTH + 2];
	uint8_t tx_packet_buffer[PIOS_USB_COM_DATA_LENGTH + 2];
};

static bool PIOS_USB_COM_validate(struct pios_usb_com_dev * usb_com_dev)
{
	return (usb_com_dev->magic == PIOS_USB_COM_DEV_MAGIC);
}

#if defined(PIOS_INCLUDE_FREERTOS)
static struct pios_usb_com_dev * PIOS_USB_COM_alloc(void)
{
	struct pios_usb_com_dev * usb_com_dev;

	usb_com_dev = (struct pios_usb_com_dev *)pvPortMalloc(sizeof(*usb_com_dev));
	if (!usb_com_dev) return(NULL);

	usb_com_dev->magic = PIOS_USB_COM_DEV_MAGIC;
	return(usb_com_dev);
}
#else
static struct pios_usb_com_dev pios_usb_com_devs[PIOS_USB_COM_MAX_DEVS];
static uint8_t pios_usb_com_num_devs;
static struct pios_usb_com_dev * PIOS_USB_COM_alloc(void)
{
	struct pios_usb_com_dev * usb_com_dev;

	if (pios_usb_com_num_devs >= PIOS_USB_COM_MAX_DEVS) {
		return (NULL);
	}

	usb_com_dev = &pios_usb_com_devs[pios_usb_com_num_devs++];
	usb_com_dev->magic = PIOS_USB_COM_DEV_MAGIC;

	return (usb_com_dev);
}
#endif

static void PIOS_USB_COM_HID_EP_IN_Callback(void);
static void PIOS_USB_COM_HID_EP_OUT_Callback(void);

static uint32_t pios_usb_com_hid_id;

#if defined(PIOS_INCLUDE_USB_COM_CDC)

static void PIOS_USB_COM_CDC_DATA_EP_IN_Callback(void);
static void PIOS_USB_COM_CDC_DATA_EP_OUT_Callback(void);
static void PIOS_USB_COM_CDC_CTRL_EP_IN_Callback(void);

static uint32_t pios_usb_com_cdc_id;

#endif	/* PIOS_INCLUDE_USB_COM_CDC */

int32_t PIOS_USB_COM_Init(uint32_t * usbcom_id, const struct pios_usb_com_cfg * cfg, uint32_t lower_id)
{
	PIOS_Assert(usbcom_id);
	PIOS_Assert(cfg);

	struct pios_usb_com_dev * usb_com_dev;

	usb_com_dev = (struct pios_usb_com_dev *) PIOS_USB_COM_alloc();
	if (!usb_com_dev) goto out_fail;

	/* Bind the configuration to the device instance */
	usb_com_dev->cfg = cfg;
	usb_com_dev->lower_id = lower_id;

	switch (cfg->type) {
#if defined(PIOS_INCLUDE_USB_COM_CDC)
	case PIOS_USB_COM_CDC:
		pios_usb_com_cdc_id = (uint32_t) usb_com_dev;
		pEpInt_OUT[cfg->ctrl_tx_ep - 1] = PIOS_USB_COM_CDC_CTRL_EP_IN_Callback;
		pEpInt_IN[cfg->data_tx_ep - 1] = PIOS_USB_COM_CDC_DATA_EP_IN_Callback;
		pEpInt_OUT[cfg->data_rx_ep - 1] = PIOS_USB_COM_CDC_DATA_EP_OUT_Callback;
		break;
#endif	/* PIOS_INCLUDE_USB_COM_CDC */
	case PIOS_USB_COM_HID:
		pios_usb_com_hid_id = (uint32_t) usb_com_dev;
		pEpInt_IN[cfg->data_tx_ep - 1] = PIOS_USB_COM_HID_EP_IN_Callback;
		pEpInt_OUT[cfg->data_rx_ep - 1] = PIOS_USB_COM_HID_EP_OUT_Callback;
		break;
	default:
		PIOS_Assert(0);
	}

	*usbcom_id = (uint32_t) usb_com_dev;

	return 0;

out_fail:
	return -1;
}


static void PIOS_USB_COM_RegisterRxCallback(uint32_t usbcom_id, pios_com_callback rx_in_cb, uint32_t context)
{
	struct pios_usb_com_dev * usb_com_dev = (struct pios_usb_com_dev *)usbcom_id;

	bool valid = PIOS_USB_COM_validate(usb_com_dev);
	PIOS_Assert(valid);

	/* 
	 * Order is important in these assignments since ISR uses _cb
	 * field to determine if it's ok to dereference _cb and _context
	 */
	usb_com_dev->rx_in_context = context;
	usb_com_dev->rx_in_cb = rx_in_cb;
}

static void PIOS_USB_COM_RegisterTxCallback(uint32_t usbcom_id, pios_com_callback tx_out_cb, uint32_t context)
{
	struct pios_usb_com_dev * usb_com_dev = (struct pios_usb_com_dev *)usbcom_id;

	bool valid = PIOS_USB_COM_validate(usb_com_dev);
	PIOS_Assert(valid);

	/* 
	 * Order is important in these assignments since ISR uses _cb
	 * field to determine if it's ok to dereference _cb and _context
	 */
	usb_com_dev->tx_out_context = context;
	usb_com_dev->tx_out_cb = tx_out_cb;
}

static void PIOS_USB_COM_HID_SendReport(struct pios_usb_com_dev * usb_com_dev)
{
	uint16_t bytes_to_tx;

	if (!usb_com_dev->tx_out_cb) {
		return;
	}

	bool need_yield = false;
#ifdef USB_HID
	bytes_to_tx = (usb_com_dev->tx_out_cb)(usb_com_dev->tx_out_context,
					       &usb_com_dev->tx_packet_buffer[1],
					       sizeof(usb_com_dev->tx_packet_buffer)-1,
					       NULL,
					       &need_yield);
#else
	bytes_to_tx = (usb_com_dev->tx_out_cb)(usb_com_dev->tx_out_context,
					       &usb_com_dev->tx_packet_buffer[2],
					       sizeof(usb_com_dev->tx_packet_buffer)-2,
					       NULL,
					       &need_yield);
#endif
	if (bytes_to_tx == 0) {
		return;
	}

	/* Always set type as report ID */
	usb_com_dev->tx_packet_buffer[0] = 1;

#ifdef USB_HID
	UserToPMABufferCopy(usb_com_dev->tx_packet_buffer,
			GetEPTxAddr(usb_com_dev->cfg->data_tx_ep),
			bytes_to_tx + 1);
#else
	usb_com_dev->tx_packet_buffer[1] = bytes_to_tx;
	UserToPMABufferCopy(usb_com_dev->tx_packet_buffer,
			GetEPTxAddr(usb_com_dev->cfg->data_tx_ep),
			bytes_to_tx + 2);
#endif
	/* Is this correct?  Why do we always send the whole buffer? */
	SetEPTxCount(usb_com_dev->cfg->data_tx_ep, sizeof(usb_com_dev->tx_packet_buffer));
	SetEPTxValid(usb_com_dev->cfg->data_tx_ep);

#if defined(PIOS_INCLUDE_FREERTOS)
	if (need_yield) {
		vPortYieldFromISR();
	}
#endif	/* PIOS_INCLUDE_FREERTOS */
}

static void PIOS_USB_COM_HID_RxStart(uint32_t usbcom_id, uint16_t rx_bytes_avail) {
	struct pios_usb_com_dev * usb_com_dev = (struct pios_usb_com_dev *)usbcom_id;

	bool valid = PIOS_USB_COM_validate(usb_com_dev);
	PIOS_Assert(valid);

	if (!PIOS_USB_HID_CheckAvailable(usb_com_dev->lower_id)) {
		return;
	}

	// If endpoint was stalled and there is now space make it valid
	PIOS_IRQ_Disable();
	if ((GetEPRxStatus(usb_com_dev->cfg->data_rx_ep) != EP_RX_VALID) && 
	    (rx_bytes_avail > PIOS_USB_COM_DATA_LENGTH)) {
		SetEPRxStatus(usb_com_dev->cfg->data_rx_ep, EP_RX_VALID);
	}
	PIOS_IRQ_Enable();
}

static void PIOS_USB_COM_HID_TxStart(uint32_t usbcom_id, uint16_t tx_bytes_avail)
{
	struct pios_usb_com_dev * usb_com_dev = (struct pios_usb_com_dev *)usbcom_id;

	bool valid = PIOS_USB_COM_validate(usb_com_dev);
	PIOS_Assert(valid);

	if (!PIOS_USB_HID_CheckAvailable(usb_com_dev->lower_id)) {
		return;
	}

	if (GetEPTxStatus(usb_com_dev->cfg->data_tx_ep) == EP_TX_VALID) {
		/* Endpoint is already transmitting */
		return;
	}

	PIOS_USB_COM_HID_SendReport(usb_com_dev);
}

/**
 * @brief Callback used to indicate a transmission from device INto host completed
 * Checks if any data remains, pads it into HID packet and sends.
 */
static void PIOS_USB_COM_HID_EP_IN_Callback(void)
{
	struct pios_usb_com_dev * usb_com_dev = (struct pios_usb_com_dev *)pios_usb_com_hid_id;

	bool valid = PIOS_USB_COM_validate(usb_com_dev);
	PIOS_Assert(valid);

	if (!PIOS_USB_HID_CheckAvailable(usb_com_dev->lower_id)) {
		return;
	}

	PIOS_USB_COM_HID_SendReport(usb_com_dev);
}

/**
 * EP1 OUT Callback Routine
 */
static void PIOS_USB_COM_HID_EP_OUT_Callback(void)
{
	struct pios_usb_com_dev * usb_com_dev = (struct pios_usb_com_dev *)pios_usb_com_hid_id;

	bool valid = PIOS_USB_COM_validate(usb_com_dev);
	PIOS_Assert(valid);

	uint32_t DataLength = 0;

	/* Read received data (63 bytes) */
	/* Get the number of received data on the selected Endpoint */
	DataLength = GetEPRxCount(usb_com_dev->cfg->data_rx_ep);
	if (DataLength > sizeof(usb_com_dev->rx_packet_buffer)) {
		DataLength = sizeof(usb_com_dev->rx_packet_buffer);
	}

	/* Use the memory interface function to read from the selected endpoint */
	PMAToUserBufferCopy((uint8_t *) usb_com_dev->rx_packet_buffer,
			GetEPRxAddr(usb_com_dev->cfg->data_rx_ep),
			DataLength);

	if (!usb_com_dev->rx_in_cb) {
		/* No Rx call back registered, disable the receiver */
		SetEPRxStatus(usb_com_dev->cfg->data_rx_ep, EP_RX_NAK);
		return;
	}

	/* The first byte is report ID (not checked), the second byte is the valid data length */
	uint16_t headroom;
	bool need_yield = false;
#ifdef USB_HID
	(usb_com_dev->rx_in_cb)(usb_com_dev->rx_in_context,
				&usb_com_dev->rx_packet_buffer[1],
				sizeof(usb_com_dev->rx_packet_buffer)-1,
				&headroom,
				&need_yield);
#else
	(usb_com_dev->rx_in_cb)(usb_com_dev->rx_in_context,
				&usb_com_dev->rx_packet_buffer[2],
				usb_com_dev->rx_packet_buffer[1],
				&headroom,
				&need_yield);
#endif
	if (headroom > PIOS_USB_COM_DATA_LENGTH) {
		/* We have room for a maximum length message */
		SetEPRxStatus(usb_com_dev->cfg->data_rx_ep, EP_RX_VALID);
	} else {
		/* Not enough room left for a message, apply backpressure */
		SetEPRxStatus(usb_com_dev->cfg->data_rx_ep, EP_RX_NAK);
	}

#if defined(PIOS_INCLUDE_FREERTOS)
	if (need_yield) {
		vPortYieldFromISR();
	}
#endif	/* PIOS_INCLUDE_FREERTOS */
}

#if defined(PIOS_INCLUDE_USB_COM_CDC)

static void PIOS_USB_COM_CDC_RxStart(uint32_t usbcom_id, uint16_t rx_bytes_avail) {
	struct pios_usb_com_dev * usb_com_dev = (struct pios_usb_com_dev *)usbcom_id;

	bool valid = PIOS_USB_COM_validate(usb_com_dev);
	PIOS_Assert(valid);

	if (!PIOS_USB_HID_CheckAvailable(usb_com_dev->lower_id)) {
		return;
	}

	// If endpoint was stalled and there is now space make it valid
	PIOS_IRQ_Disable();
	if ((GetEPRxStatus(usb_com_dev->cfg->data_rx_ep) != EP_RX_VALID) && 
		(rx_bytes_avail > sizeof(usb_com_dev->rx_packet_buffer))) {
		SetEPRxStatus(usb_com_dev->cfg->data_rx_ep, EP_RX_VALID);
	}
	PIOS_IRQ_Enable();
}

static void PIOS_USB_COM_CDC_SendData(struct pios_usb_com_dev * usb_com_dev)
{
	uint16_t bytes_to_tx;

	if (!usb_com_dev->tx_out_cb) {
		return;
	}

	bool need_yield = false;
	bytes_to_tx = (usb_com_dev->tx_out_cb)(usb_com_dev->tx_out_context,
					       usb_com_dev->tx_packet_buffer,
					       sizeof(usb_com_dev->tx_packet_buffer),
					       NULL,
					       &need_yield);
	if (bytes_to_tx == 0) {
		return;
	}

	UserToPMABufferCopy(usb_com_dev->tx_packet_buffer,
			GetEPTxAddr(usb_com_dev->cfg->data_tx_ep),
			bytes_to_tx);
	SetEPTxCount(usb_com_dev->cfg->data_tx_ep, bytes_to_tx);
	SetEPTxValid(usb_com_dev->cfg->data_tx_ep);

#if defined(PIOS_INCLUDE_FREERTOS)
	if (need_yield) {
		vPortYieldFromISR();
	}
#endif	/* PIOS_INCLUDE_FREERTOS */
}

static void PIOS_USB_COM_CDC_TxStart(uint32_t usbcom_id, uint16_t tx_bytes_avail)
{
	struct pios_usb_com_dev * usb_com_dev = (struct pios_usb_com_dev *)usbcom_id;

	bool valid = PIOS_USB_COM_validate(usb_com_dev);
	PIOS_Assert(valid);

	if (!PIOS_USB_HID_CheckAvailable(usb_com_dev->lower_id)) {
		return;
	}

	if (GetEPTxStatus(usb_com_dev->cfg->data_tx_ep) == EP_TX_VALID) {
		/* Endpoint is already transmitting */
		return;
	}

	PIOS_USB_COM_CDC_SendData(usb_com_dev);
}

static void PIOS_USB_COM_CDC_DATA_EP_IN_Callback(void)
{
	struct pios_usb_com_dev * usb_com_dev = (struct pios_usb_com_dev *)pios_usb_com_cdc_id;

	bool valid = PIOS_USB_COM_validate(usb_com_dev);
	PIOS_Assert(valid);

	PIOS_USB_COM_CDC_SendData(usb_com_dev);
}

static void PIOS_USB_COM_CDC_DATA_EP_OUT_Callback(void)
{
	struct pios_usb_com_dev * usb_com_dev = (struct pios_usb_com_dev *)pios_usb_com_cdc_id;

	bool valid = PIOS_USB_COM_validate(usb_com_dev);
	PIOS_Assert(valid);

	uint32_t DataLength = 0;

	/* Get the number of received data on the selected Endpoint */
	DataLength = GetEPRxCount(usb_com_dev->cfg->data_rx_ep);
	if (DataLength > sizeof(usb_com_dev->rx_packet_buffer)) {
		DataLength = sizeof(usb_com_dev->rx_packet_buffer);
	}

	/* Use the memory interface function to read from the selected endpoint */
	PMAToUserBufferCopy((uint8_t *) usb_com_dev->rx_packet_buffer,
			GetEPRxAddr(usb_com_dev->cfg->data_rx_ep),
			DataLength);

	if (!usb_com_dev->rx_in_cb) {
		/* No Rx call back registered, disable the receiver */
		SetEPRxStatus(usb_com_dev->cfg->data_rx_ep, EP_RX_NAK);
		return;
	}

	uint16_t headroom;
	bool need_yield = false;
	(usb_com_dev->rx_in_cb)(usb_com_dev->rx_in_context,
				usb_com_dev->rx_packet_buffer,
				sizeof(usb_com_dev->rx_packet_buffer),
				&headroom,
				&need_yield);

	if (headroom > sizeof(usb_com_dev->rx_packet_buffer)) {
		/* We have room for a maximum length message */
		SetEPRxStatus(usb_com_dev->cfg->data_rx_ep, EP_RX_VALID);
	} else {
		/* Not enough room left for a message, apply backpressure */
		SetEPRxStatus(usb_com_dev->cfg->data_rx_ep, EP_RX_STALL);
	}

#if defined(PIOS_INCLUDE_FREERTOS)
	if (need_yield) {
		vPortYieldFromISR();
	}
#endif	/* PIOS_INCLUDE_FREERTOS */
}

RESULT PIOS_CDC_SetControlLineState(void)
{
	struct pios_usb_com_dev * usb_com_dev = (struct pios_usb_com_dev *)pios_usb_com_cdc_id;

	bool valid = PIOS_USB_COM_validate(usb_com_dev);
	PIOS_Assert(valid);

	static uint16_t control_line_state;

	uint8_t wValue0 = pInformation->USBwValue0;
	uint8_t wValue1 = pInformation->USBwValue1;

	control_line_state = wValue1 << 8 | wValue0;

	return USB_SUCCESS;
}

RESULT PIOS_CDC_SetLineCoding(void)
{
	struct pios_usb_com_dev * usb_com_dev = (struct pios_usb_com_dev *)pios_usb_com_cdc_id;

	bool valid = PIOS_USB_COM_validate(usb_com_dev);
	PIOS_Assert(valid);

	return USB_SUCCESS;
}

uint8_t *PIOS_CDC_GetLineCoding(uint16_t Length)
{
	struct pios_usb_com_dev * usb_com_dev = (struct pios_usb_com_dev *)pios_usb_com_cdc_id;

	bool valid = PIOS_USB_COM_validate(usb_com_dev);
	PIOS_Assert(valid);

	return NULL;
}

static uint8_t serial_state[] = {
	0xA1,			/* bmRequestType */
	0x20,			/* bNotification (Serial State) */
	0x00,			/* wValue */
	0x00,
	0x01,			/* wIndex (Interface #, LSB first) */
	0x00,
	0x02,			/* wLength (Data length) */
	0x00,

	0x00,			/* USART State Bitmap (LSB first) */
	0x00,
};

static void PIOS_USB_COM_CDC_CTRL_EP_IN_Callback(void)
{
	struct pios_usb_com_dev * usb_com_dev = (struct pios_usb_com_dev *)pios_usb_com_cdc_id;

	bool valid = PIOS_USB_COM_validate(usb_com_dev);
	PIOS_Assert(valid);

	/* Give back UART State Bitmap */
	/* UART State Bitmap
	 *   15-7: reserved
	 *      6:  bOverRun    overrun error
	 *      5:  bParity     parity error
	 *      4:  bFraming    framing error
	 *      3:  bRingSignal RI
	 *      2:  bBreak      break reception
	 *      1:  bTxCarrier  DSR
	 *      0:  bRxCarrier  DCD
	 */
	serial_state[8] = 0x03;
	serial_state[9] = 0x00;

	UserToPMABufferCopy(serial_state,
			GetEPTxAddr(usb_com_dev->cfg->data_tx_ep),
			sizeof(serial_state));
	SetEPTxCount(usb_com_dev->cfg->data_tx_ep, PIOS_USB_COM_DATA_LENGTH + 2);
	SetEPTxValid(usb_com_dev->cfg->data_tx_ep);
}

#endif	/* PIOS_INCLUDE_USB_COM_CDC */

#endif	/* PIOS_INCLUDE_USB_COM */

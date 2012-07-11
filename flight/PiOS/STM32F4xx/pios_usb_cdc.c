/**
 ******************************************************************************
 * @addtogroup PIOS PIOS Core hardware abstraction layer
 * @{
 * @addtogroup   PIOS_USB_COM USB COM Functions
 * @brief PIOS USB COM implementation for CDC interfaces
 * @notes      This implements a CDC Serial Port
 * @{
 *
 * @file       pios_usb_com_cdc.c
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

#if defined(PIOS_INCLUDE_USB_CDC)

#include "pios_usb.h"
#include "pios_usb_cdc_priv.h"
#include "pios_usb_board_data.h" /* PIOS_BOARD_*_DATA_LENGTH */

static void PIOS_USB_CDC_RegisterTxCallback(uint32_t usbcdc_id, pios_com_callback tx_out_cb, uint32_t context);
static void PIOS_USB_CDC_RegisterRxCallback(uint32_t usbcdc_id, pios_com_callback rx_in_cb, uint32_t context);
static void PIOS_USB_CDC_TxStart(uint32_t usbcdc_id, uint16_t tx_bytes_avail);
static void PIOS_USB_CDC_RxStart(uint32_t usbcdc_id, uint16_t rx_bytes_avail);

const struct pios_com_driver pios_usb_cdc_com_driver = {
	.tx_start    = PIOS_USB_CDC_TxStart,
	.rx_start    = PIOS_USB_CDC_RxStart,
	.bind_tx_cb  = PIOS_USB_CDC_RegisterTxCallback,
	.bind_rx_cb  = PIOS_USB_CDC_RegisterRxCallback,
};

enum pios_usb_cdc_dev_magic {
	PIOS_USB_CDC_DEV_MAGIC = 0xAABBCCDD,
};

struct pios_usb_cdc_dev {
	enum pios_usb_cdc_dev_magic     magic;
	const struct pios_usb_cdc_cfg * cfg;

	uint32_t lower_id;

	pios_com_callback rx_in_cb;
	uint32_t rx_in_context;
	pios_com_callback tx_out_cb;
	uint32_t tx_out_context;

	uint8_t rx_packet_buffer[PIOS_USB_BOARD_CDC_DATA_LENGTH];
	uint8_t tx_packet_buffer[PIOS_USB_BOARD_CDC_DATA_LENGTH];

	uint32_t rx_dropped;
	uint32_t rx_oversize;
};

static bool PIOS_USB_CDC_validate(struct pios_usb_cdc_dev * usb_cdc_dev)
{
	return (usb_cdc_dev && (usb_cdc_dev->magic == PIOS_USB_CDC_DEV_MAGIC));
}

#if defined(PIOS_INCLUDE_FREERTOS)
static struct pios_usb_cdc_dev * PIOS_USB_CDC_alloc(void)
{
	struct pios_usb_cdc_dev * usb_cdc_dev;

	usb_cdc_dev = (struct pios_usb_cdc_dev *)pvPortMalloc(sizeof(*usb_cdc_dev));
	if (!usb_cdc_dev) return(NULL);

	usb_cdc_dev->magic = PIOS_USB_CDC_DEV_MAGIC;
	return(usb_cdc_dev);
}
#else
static struct pios_usb_cdc_dev pios_usb_cdc_devs[PIOS_USB_CDC_MAX_DEVS];
static uint8_t pios_usb_cdc_num_devs;
static struct pios_usb_cdc_dev * PIOS_USB_CDC_alloc(void)
{
	struct pios_usb_cdc_dev * usb_cdc_dev;

	if (pios_usb_cdc_num_devs >= PIOS_USB_CDC_MAX_DEVS) {
		return (NULL);
	}

	usb_cdc_dev = &pios_usb_cdc_devs[pios_usb_cdc_num_devs++];
	usb_cdc_dev->magic = PIOS_USB_CDC_DEV_MAGIC;

	return (usb_cdc_dev);
}
#endif

static void PIOS_USB_CDC_DATA_EP_IN_Callback(void);
static void PIOS_USB_CDC_DATA_EP_OUT_Callback(void);
static void PIOS_USB_CDC_CTRL_EP_IN_Callback(void);

static uint32_t pios_usb_cdc_id;

/* Need a better way to pull these in */
extern void (*pEpInt_IN[7])(void);
extern void (*pEpInt_OUT[7])(void);

int32_t PIOS_USB_CDC_Init(uint32_t * usbcdc_id, const struct pios_usb_cdc_cfg * cfg, uint32_t lower_id)
{
	PIOS_Assert(usbcdc_id);
	PIOS_Assert(cfg);

	struct pios_usb_cdc_dev * usb_cdc_dev;

	usb_cdc_dev = (struct pios_usb_cdc_dev *) PIOS_USB_CDC_alloc();
	if (!usb_cdc_dev) goto out_fail;

	/* Bind the configuration to the device instance */
	usb_cdc_dev->cfg = cfg;
	usb_cdc_dev->lower_id = lower_id;

	pios_usb_cdc_id = (uint32_t) usb_cdc_dev;

	/* Bind lower level callbacks into the USB infrastructure */
	pEpInt_OUT[cfg->ctrl_tx_ep - 1] = PIOS_USB_CDC_CTRL_EP_IN_Callback;
	pEpInt_IN[cfg->data_tx_ep - 1] = PIOS_USB_CDC_DATA_EP_IN_Callback;
	pEpInt_OUT[cfg->data_rx_ep - 1] = PIOS_USB_CDC_DATA_EP_OUT_Callback;

	*usbcdc_id = (uint32_t) usb_cdc_dev;

	return 0;

out_fail:
	return -1;
}



static void PIOS_USB_CDC_RegisterRxCallback(uint32_t usbcdc_id, pios_com_callback rx_in_cb, uint32_t context)
{
	struct pios_usb_cdc_dev * usb_cdc_dev = (struct pios_usb_cdc_dev *)usbcdc_id;

	bool valid = PIOS_USB_CDC_validate(usb_cdc_dev);
	PIOS_Assert(valid);

	/* 
	 * Order is important in these assignments since ISR uses _cb
	 * field to determine if it's ok to dereference _cb and _context
	 */
	usb_cdc_dev->rx_in_context = context;
	usb_cdc_dev->rx_in_cb = rx_in_cb;
}

static void PIOS_USB_CDC_RegisterTxCallback(uint32_t usbcdc_id, pios_com_callback tx_out_cb, uint32_t context)
{
	struct pios_usb_cdc_dev * usb_cdc_dev = (struct pios_usb_cdc_dev *)usbcdc_id;

	bool valid = PIOS_USB_CDC_validate(usb_cdc_dev);
	PIOS_Assert(valid);

	/* 
	 * Order is important in these assignments since ISR uses _cb
	 * field to determine if it's ok to dereference _cb and _context
	 */
	usb_cdc_dev->tx_out_context = context;
	usb_cdc_dev->tx_out_cb = tx_out_cb;
}

static void PIOS_USB_CDC_RxStart(uint32_t usbcdc_id, uint16_t rx_bytes_avail) {
	struct pios_usb_cdc_dev * usb_cdc_dev = (struct pios_usb_cdc_dev *)usbcdc_id;

	bool valid = PIOS_USB_CDC_validate(usb_cdc_dev);
	PIOS_Assert(valid);

	if (!PIOS_USB_CheckAvailable(usb_cdc_dev->lower_id)) {
		return;
	}

	// If endpoint was stalled and there is now space make it valid
	PIOS_IRQ_Disable();
	if ((GetEPRxStatus(usb_cdc_dev->cfg->data_rx_ep) != EP_RX_VALID) && 
		(rx_bytes_avail >= sizeof(usb_cdc_dev->rx_packet_buffer))) {
		SetEPRxStatus(usb_cdc_dev->cfg->data_rx_ep, EP_RX_VALID);
	}
	PIOS_IRQ_Enable();
}

static void PIOS_USB_CDC_SendData(struct pios_usb_cdc_dev * usb_cdc_dev)
{
	uint16_t bytes_to_tx;

	if (!usb_cdc_dev->tx_out_cb) {
		return;
	}

	bool need_yield = false;
	bytes_to_tx = (usb_cdc_dev->tx_out_cb)(usb_cdc_dev->tx_out_context,
					       usb_cdc_dev->tx_packet_buffer,
					       sizeof(usb_cdc_dev->tx_packet_buffer),
					       NULL,
					       &need_yield);
	if (bytes_to_tx == 0) {
		return;
	}

	UserToPMABufferCopy(usb_cdc_dev->tx_packet_buffer,
			GetEPTxAddr(usb_cdc_dev->cfg->data_tx_ep),
			bytes_to_tx);
	SetEPTxCount(usb_cdc_dev->cfg->data_tx_ep, bytes_to_tx);
	SetEPTxValid(usb_cdc_dev->cfg->data_tx_ep);

#if defined(PIOS_INCLUDE_FREERTOS)
	if (need_yield) {
		vPortYieldFromISR();
	}
#endif	/* PIOS_INCLUDE_FREERTOS */
}

static void PIOS_USB_CDC_TxStart(uint32_t usbcdc_id, uint16_t tx_bytes_avail)
{
	struct pios_usb_cdc_dev * usb_cdc_dev = (struct pios_usb_cdc_dev *)usbcdc_id;

	bool valid = PIOS_USB_CDC_validate(usb_cdc_dev);
	PIOS_Assert(valid);

	if (!PIOS_USB_CheckAvailable(usb_cdc_dev->lower_id)) {
		return;
	}

	if (GetEPTxStatus(usb_cdc_dev->cfg->data_tx_ep) == EP_TX_VALID) {
		/* Endpoint is already transmitting */
		return;
	}

	PIOS_USB_CDC_SendData(usb_cdc_dev);
}

static void PIOS_USB_CDC_DATA_EP_IN_Callback(void)
{
	struct pios_usb_cdc_dev * usb_cdc_dev = (struct pios_usb_cdc_dev *)pios_usb_cdc_id;

	bool valid = PIOS_USB_CDC_validate(usb_cdc_dev);
	PIOS_Assert(valid);

	PIOS_USB_CDC_SendData(usb_cdc_dev);
}

static void PIOS_USB_CDC_DATA_EP_OUT_Callback(void)
{
	struct pios_usb_cdc_dev * usb_cdc_dev = (struct pios_usb_cdc_dev *)pios_usb_cdc_id;

	bool valid = PIOS_USB_CDC_validate(usb_cdc_dev);
	PIOS_Assert(valid);

	uint32_t DataLength;

	/* Get the number of received data on the selected Endpoint */
	DataLength = GetEPRxCount(usb_cdc_dev->cfg->data_rx_ep);
	if (DataLength > sizeof(usb_cdc_dev->rx_packet_buffer)) {
		usb_cdc_dev->rx_oversize++;
		DataLength = sizeof(usb_cdc_dev->rx_packet_buffer);
	}

	/* Use the memory interface function to read from the selected endpoint */
	PMAToUserBufferCopy((uint8_t *) usb_cdc_dev->rx_packet_buffer,
			GetEPRxAddr(usb_cdc_dev->cfg->data_rx_ep),
			DataLength);

	if (!usb_cdc_dev->rx_in_cb) {
		/* No Rx call back registered, disable the receiver */
		SetEPRxStatus(usb_cdc_dev->cfg->data_rx_ep, EP_RX_NAK);
		return;
	}

	uint16_t headroom;
	bool need_yield = false;
	uint16_t rc;
	rc = (usb_cdc_dev->rx_in_cb)(usb_cdc_dev->rx_in_context,
				usb_cdc_dev->rx_packet_buffer,
				DataLength,
				&headroom,
				&need_yield);

	if (rc < DataLength) {
		/* Lost bytes on rx */
		usb_cdc_dev->rx_dropped += (DataLength - rc);
	}

	if (headroom >= sizeof(usb_cdc_dev->rx_packet_buffer)) {
		/* We have room for a maximum length message */
		SetEPRxStatus(usb_cdc_dev->cfg->data_rx_ep, EP_RX_VALID);
	} else {
		/* Not enough room left for a message, apply backpressure */
		SetEPRxStatus(usb_cdc_dev->cfg->data_rx_ep, EP_RX_NAK);
	}

#if defined(PIOS_INCLUDE_FREERTOS)
	if (need_yield) {
		vPortYieldFromISR();
	}
#endif	/* PIOS_INCLUDE_FREERTOS */
}

RESULT PIOS_USB_CDC_SetControlLineState(void)
{
	struct pios_usb_cdc_dev * usb_cdc_dev = (struct pios_usb_cdc_dev *)pios_usb_cdc_id;

	bool valid = PIOS_USB_CDC_validate(usb_cdc_dev);
	PIOS_Assert(valid);

	static uint16_t control_line_state;

	uint8_t wValue0 = pInformation->USBwValue0;
	uint8_t wValue1 = pInformation->USBwValue1;

	control_line_state = wValue1 << 8 | wValue0;

	return USB_SUCCESS;
}

static struct usb_cdc_line_coding line_coding = {
	.dwDTERate   = htousbl(57600),
	.bCharFormat = USB_CDC_LINE_CODING_STOP_1,
	.bParityType = USB_CDC_LINE_CODING_PARITY_NONE,
	.bDataBits   = 8,
};

RESULT PIOS_USB_CDC_SetLineCoding(void)
{
	struct pios_usb_cdc_dev * usb_cdc_dev = (struct pios_usb_cdc_dev *)pios_usb_cdc_id;

	bool valid = PIOS_USB_CDC_validate(usb_cdc_dev);
	PIOS_Assert(valid);

	return USB_SUCCESS;
}

const uint8_t *PIOS_USB_CDC_GetLineCoding(uint16_t Length)
{
	struct pios_usb_cdc_dev * usb_cdc_dev = (struct pios_usb_cdc_dev *)pios_usb_cdc_id;

	bool valid = PIOS_USB_CDC_validate(usb_cdc_dev);
	PIOS_Assert(valid);

	if (Length == 0) {
		pInformation->Ctrl_Info.Usb_wLength = sizeof(line_coding);
		return NULL;
	} else {
		return ((uint8_t *) &line_coding);
	}
}

struct usb_cdc_serial_state_report uart_state = {
	.bmRequestType = 0xA1,
	.bNotification = USB_CDC_NOTIFICATION_SERIAL_STATE,
	.wValue        = 0,
	.wIndex        = htousbs(1),
	.wLength       = htousbs(2),
	.bmUartState  = htousbs(0),
};
	
static void PIOS_USB_CDC_CTRL_EP_IN_Callback(void)
{
	struct pios_usb_cdc_dev * usb_cdc_dev = (struct pios_usb_cdc_dev *)pios_usb_cdc_id;

	bool valid = PIOS_USB_CDC_validate(usb_cdc_dev);
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
	uart_state.bmUartState = htousbs(0x0003);

	UserToPMABufferCopy((uint8_t *) &uart_state,
			GetEPTxAddr(usb_cdc_dev->cfg->data_tx_ep),
			sizeof(uart_state));
	SetEPTxCount(usb_cdc_dev->cfg->data_tx_ep, PIOS_USB_BOARD_CDC_MGMT_LENGTH);
	SetEPTxValid(usb_cdc_dev->cfg->data_tx_ep);
}

#endif	/* PIOS_INCLUDE_USB_CDC */

/**
 ******************************************************************************
 * @addtogroup PIOS PIOS Core hardware abstraction layer
 * @{
 * @addtogroup   PIOS_USB_HID USB HID Functions
 * @brief PIOS USB HID implementation
 * @notes      This implements a very simple HID device with a simple data in
 * and data out endpoints.
 * @{
 *
 * @file       pios_usb_hid.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * 		Parts by Thorsten Klose (tk@midibox.org)
 * @brief      USB HID functions (STM32 dependent code)
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
#include "usb_lib.h"
#include "pios_usb_hid_desc.h"
#include "stm32f10x.h"

#include "pios_usb_hid_priv.h"

#if defined(PIOS_INCLUDE_USB_HID)

static void PIOS_USB_HID_TxStart(uint32_t usbcom_id, uint16_t tx_bytes_avail);
static void PIOS_USB_HID_RxStart(uint32_t usbcom_id, uint16_t rx_bytes_avail);
static void PIOS_USB_HID_RegisterTxCallback(uint32_t usbcom_id, pios_com_callback tx_out_cb, uint32_t context);
static void PIOS_USB_HID_RegisterRxCallback(uint32_t usbcom_id, pios_com_callback rx_in_cb, uint32_t context);

const struct pios_com_driver pios_usb_com_driver = {
	.tx_start    = PIOS_USB_HID_TxStart,
	.rx_start    = PIOS_USB_HID_RxStart,
	.bind_tx_cb  = PIOS_USB_HID_RegisterTxCallback,
	.bind_rx_cb  = PIOS_USB_HID_RegisterRxCallback,
};

enum pios_usb_hid_dev_magic {
	PIOS_USB_HID_DEV_MAGIC = 0xAABBCCDD,
};

struct pios_usb_hid_dev {
	enum pios_usb_hid_dev_magic     magic;
	const struct pios_usb_hid_cfg * cfg;

	pios_com_callback rx_in_cb;
	uint32_t rx_in_context;
	pios_com_callback tx_out_cb;
	uint32_t tx_out_context;

	uint8_t rx_packet_buffer[PIOS_USB_HID_DATA_LENGTH + 2];
	uint8_t tx_packet_buffer[PIOS_USB_HID_DATA_LENGTH + 2];
};

static bool PIOS_USB_HID_validate(struct pios_usb_hid_dev * usb_hid_dev)
{
	return (usb_hid_dev->magic == PIOS_USB_HID_DEV_MAGIC);
}

#if defined(PIOS_INCLUDE_FREERTOS) && 0
static struct pios_usb_hid_dev * PIOS_USB_HID_alloc(void)
{
	struct pios_usb_hid_dev * usb_hid_dev;

	usb_hid_dev = (struct pios_usb_hid_dev *)malloc(sizeof(*usb_hid_dev));
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

/* Rx/Tx status */
static uint8_t transfer_possible = 0;

/**
 * Initialises USB COM layer
 * \return < 0 if initialisation failed
 * \note Applications shouldn't call this function directly, instead please use \ref PIOS_COM layer functions
 */
static uint32_t pios_usb_hid_id;
int32_t PIOS_USB_HID_Init(uint32_t * usb_hid_id, const struct pios_usb_hid_cfg * cfg)
{
	PIOS_Assert(usb_hid_id);
	PIOS_Assert(cfg);

	struct pios_usb_hid_dev * usb_hid_dev;

	usb_hid_dev = (struct pios_usb_hid_dev *) PIOS_USB_HID_alloc();
	if (!usb_hid_dev) goto out_fail;

	/* Bind the configuration to the device instance */
	usb_hid_dev->cfg = cfg;

	PIOS_USB_HID_Reenumerate();

	/*
	 * This is a horrible hack to make this available to
	 * the interrupt callbacks.  This should go away ASAP.
	 */
	pios_usb_hid_id = (uint32_t) usb_hid_dev;

	/* Enable the USB Interrupts */
	NVIC_Init(&usb_hid_dev->cfg->irq.init);

	/* Select USBCLK source */
	RCC_USBCLKConfig(RCC_USBCLKSource_PLLCLK_1Div5);
	/* Enable the USB clock */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USB, ENABLE);

	/* Update the USB serial number from the chip */
	uint8_t sn[25];
	PIOS_SYS_SerialNumberGet((char *)sn);
	for (uint8_t i = 0; sn[i] != '\0' && (2 * i) < PIOS_HID_StringSerial[0]; i++) {
		PIOS_HID_StringSerial[2 + 2 * i] = sn[i];
	}

	USB_Init();
	USB_SIL_Init();

	*usb_hid_id = (uint32_t) usb_hid_dev;

	return 0;		/* No error */

out_fail:
	return(-1);
}

/**
 * This function is called by the USB driver on cable connection/disconnection
 * \param[in] connected connection status (1 if connected)
 * \return < 0 on errors
 * \note Applications shouldn't call this function directly, instead please use \ref PIOS_COM layer functions
 */
int32_t PIOS_USB_HID_ChangeConnectionState(uint32_t Connected)
{
	// In all cases: re-initialise USB HID driver
	if (Connected) {
		transfer_possible = 1;

		//TODO: Check SetEPRxValid(ENDP1);

#if defined(USB_LED_ON)
		USB_LED_ON;	// turn the USB led on
#endif
	} else {
		// Cable disconnected: disable transfers
		transfer_possible = 0;

#if defined(USB_LED_OFF)
		USB_LED_OFF;	// turn the USB led off
#endif
	}

	return 0;
}

int32_t PIOS_USB_HID_Reenumerate()
{
	/* Force USB reset and power-down (this will also release the USB pins for direct GPIO control) */
	_SetCNTR(CNTR_FRES | CNTR_PDWN);

	/* Using a "dirty" method to force a re-enumeration: */
	/* Force DPM (Pin PA12) low for ca. 10 mS before USB Tranceiver will be enabled */
	/* This overrules the external Pull-Up at PA12, and at least Windows & MacOS will enumerate again */
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_StructInit(&GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	PIOS_DELAY_WaitmS(50);

	/* Release power-down, still hold reset */
	_SetCNTR(CNTR_PDWN);
	PIOS_DELAY_WaituS(5);

	/* CNTR_FRES = 0 */
	_SetCNTR(0);

	/* Clear pending interrupts */
	_SetISTR(0);

	/* Configure USB clock */
	/* USBCLK = PLLCLK / 1.5 */
	RCC_USBCLKConfig(RCC_USBCLKSource_PLLCLK_1Div5);
	/* Enable USB clock */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USB, ENABLE);

	return 0;
}

/**
 * This function returns the connection status of the USB HID interface
 * \return 1: interface available
 * \return 0: interface not available
 * \note Applications shouldn't call this function directly, instead please use \ref PIOS_COM layer functions
 */
int32_t PIOS_USB_HID_CheckAvailable(uint8_t id)
{
	return (PIOS_USB_DETECT_GPIO_PORT->IDR & PIOS_USB_DETECT_GPIO_PIN) != 0 && transfer_possible ? 1 : 0;
}

static void PIOS_USB_HID_SendReport(struct pios_usb_hid_dev * usb_hid_dev)
{
	uint16_t bytes_to_tx;

	if (!usb_hid_dev->tx_out_cb) {
		return;
	}

	bool need_yield = false;
#ifdef USB_HID
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

#ifdef USB_HID
	UserToPMABufferCopy(usb_hid_dev->tx_packet_buffer, GetEPTxAddr(EP1_IN & 0x7F), bytes_to_tx + 1);
#else
	usb_hid_dev->tx_packet_buffer[1] = bytes_to_tx;
	UserToPMABufferCopy(usb_hid_dev->tx_packet_buffer, GetEPTxAddr(EP1_IN & 0x7F), bytes_to_tx + 2);
#endif
	/* Is this correct?  Why do we always send the whole buffer? */
	SetEPTxCount((EP1_IN & 0x7F), sizeof(usb_hid_dev->tx_packet_buffer));
	SetEPTxValid(ENDP1);

#if defined(PIOS_INCLUDE_FREERTOS)
	if (need_yield) {
		vPortYieldFromISR();
	}
#endif	/* PIOS_INCLUDE_FREERTOS */
}

static void PIOS_USB_HID_RxStart(uint32_t usbcom_id, uint16_t rx_bytes_avail) {
	struct pios_usb_hid_dev * usb_hid_dev = (struct pios_usb_hid_dev *)usbcom_id;

	bool valid = PIOS_USB_HID_validate(usb_hid_dev);
	PIOS_Assert(valid);

	if (!transfer_possible) {
		return;
	}

	// If endpoint was stalled and there is now space make it valid
	PIOS_IRQ_Disable();
	if ((GetEPRxStatus(ENDP1) != EP_RX_VALID) && 
	    (rx_bytes_avail > PIOS_USB_HID_DATA_LENGTH)) {
		SetEPRxStatus(ENDP1, EP_RX_VALID);
	}
	PIOS_IRQ_Enable();
}

static void PIOS_USB_HID_TxStart(uint32_t usbcom_id, uint16_t tx_bytes_avail)
{
	struct pios_usb_hid_dev * usb_hid_dev = (struct pios_usb_hid_dev *)usbcom_id;

	bool valid = PIOS_USB_HID_validate(usb_hid_dev);
	PIOS_Assert(valid);

	if (!transfer_possible) {
		return;
	}

	if (GetEPTxStatus(ENDP1) == EP_TX_VALID) {
		/* Endpoint is already transmitting */
		return;
	}

	PIOS_USB_HID_SendReport(usb_hid_dev);
}

static void PIOS_USB_HID_RegisterRxCallback(uint32_t usbcom_id, pios_com_callback rx_in_cb, uint32_t context)
{
	struct pios_usb_hid_dev * usb_hid_dev = (struct pios_usb_hid_dev *)usbcom_id;

	bool valid = PIOS_USB_HID_validate(usb_hid_dev);
	PIOS_Assert(valid);

	/* 
	 * Order is important in these assignments since ISR uses _cb
	 * field to determine if it's ok to dereference _cb and _context
	 */
	usb_hid_dev->rx_in_context = context;
	usb_hid_dev->rx_in_cb = rx_in_cb;
}

static void PIOS_USB_HID_RegisterTxCallback(uint32_t usbcom_id, pios_com_callback tx_out_cb, uint32_t context)
{
	struct pios_usb_hid_dev * usb_hid_dev = (struct pios_usb_hid_dev *)usbcom_id;

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
void PIOS_USB_HID_EP1_IN_Callback(void)
{
	struct pios_usb_hid_dev * usb_hid_dev = (struct pios_usb_hid_dev *)pios_usb_hid_id;

	bool valid = PIOS_USB_HID_validate(usb_hid_dev);
	PIOS_Assert(valid);

	if (!transfer_possible) {
		return;
	}

	PIOS_USB_HID_SendReport(usb_hid_dev);
}

/**
 * EP1 OUT Callback Routine
 */
void PIOS_USB_HID_EP1_OUT_Callback(void)
{
	struct pios_usb_hid_dev * usb_hid_dev = (struct pios_usb_hid_dev *)pios_usb_hid_id;

	bool valid = PIOS_USB_HID_validate(usb_hid_dev);
	PIOS_Assert(valid);

	uint32_t DataLength = 0;

	/* Read received data (63 bytes) */
	/* Get the number of received data on the selected Endpoint */
	DataLength = GetEPRxCount(ENDP1 & 0x7F);
	if (DataLength > sizeof(usb_hid_dev->rx_packet_buffer)) {
		DataLength = sizeof(usb_hid_dev->rx_packet_buffer);
	}

	/* Use the memory interface function to write to the selected endpoint */
	PMAToUserBufferCopy((uint8_t *) usb_hid_dev->rx_packet_buffer, GetEPRxAddr(ENDP1 & 0x7F), DataLength);

	if (!usb_hid_dev->rx_in_cb) {
		/* No Rx call back registered, disable the receiver */
		SetEPRxStatus(ENDP1, EP_RX_NAK);
		return;
	}

	/* The first byte is report ID (not checked), the second byte is the valid data length */
	uint16_t headroom;
	bool need_yield = false;
#ifdef USB_HID
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
	if (headroom > PIOS_USB_HID_DATA_LENGTH) {
		/* We have room for a maximum length message */
		SetEPRxStatus(ENDP1, EP_RX_VALID);
	} else {
		/* Not enough room left for a message, apply backpressure */
		SetEPRxStatus(ENDP1, EP_RX_NAK);
	}

#if defined(PIOS_INCLUDE_FREERTOS)
	if (need_yield) {
		vPortYieldFromISR();
	}
#endif	/* PIOS_INCLUDE_FREERTOS */
}

#endif

/**
 * @}
 * @}
 */

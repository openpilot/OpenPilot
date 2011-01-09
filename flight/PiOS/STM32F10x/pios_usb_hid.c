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
#include "fifo_buffer.h"

#if defined(PIOS_INCLUDE_USB_HID)

#if defined(PIOS_INCLUDE_FREERTOS)
#define USE_FREERTOS
#endif

const struct pios_com_driver pios_usb_com_driver = {
	.tx_nb = PIOS_USB_HID_TxBufferPutMoreNonBlocking,
	.tx = PIOS_USB_HID_TxBufferPutMore,
	.rx = PIOS_USB_HID_RxBufferGet,
	.rx_avail = PIOS_USB_HID_RxBufferUsed,
};

// TODO: Eventually replace the transmit and receive buffers with bigger ring bufers
// so there isn't hte 64 byte cap in place by the USB interrupt packet definition

/* Rx/Tx status */
static uint8_t transfer_possible = 0;
static uint8_t rx_packet_buffer[PIOS_USB_HID_DATA_LENGTH + 2] = { 0 };
static uint8_t tx_packet_buffer[PIOS_USB_HID_DATA_LENGTH + 2] = { 0 };

uint8_t rx_pios_fifo_buf[1024] __attribute__ ((aligned(4)));    // align to 32-bit to try and provide speed improvement
t_fifo_buffer rx_pios_fifo_buffer;

uint8_t tx_pios_fifo_buf[1024] __attribute__ ((aligned(4)));    // align to 32-bit to try and provide speed improvement
t_fifo_buffer tx_pios_fifo_buffer;

#if defined(USE_FREERTOS)
xSemaphoreHandle pios_usb_tx_semaphore;
#endif

/**
 * Initialises USB COM layer
 * \param[in] mode currently only mode 0 supported
 * \return < 0 if initialisation failed
 * \note Applications shouldn't call this function directly, instead please use \ref PIOS_COM layer functions
 */
int32_t PIOS_USB_HID_Init(uint32_t mode)
{
	/* Currently only mode 0 supported */
	if (mode != 0) {
		/* Unsupported mode */
		return -1;
	}

	fifoBuf_init(&rx_pios_fifo_buffer, rx_pios_fifo_buf, sizeof(rx_pios_fifo_buf));
	fifoBuf_init(&tx_pios_fifo_buffer, tx_pios_fifo_buf, sizeof(tx_pios_fifo_buf));

	PIOS_USB_HID_Reenumerate();

	/* Create semaphore before enabling interrupts */
#if defined(USE_FREERTOS)	
	vSemaphoreCreateBinary(pios_usb_tx_semaphore);
#endif
	
	/* Enable the USB Interrupts */
	/* 2 bit for pre-emption priority, 2 bits for subpriority */
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = USB_LP_CAN1_RX0_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = PIOS_IRQ_PRIO_LOW;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

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

	return 0;		/* No error */
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

void sendChunk()
{

	uint32_t size = fifoBuf_getUsed(&tx_pios_fifo_buffer);
	if ((size > 0) && (GetEPTxStatus(ENDP1) != EP_TX_VALID)) {
		
		if (size > PIOS_USB_HID_DATA_LENGTH)
			size = PIOS_USB_HID_DATA_LENGTH;
#ifdef USB_HID
		fifoBuf_getData(&tx_pios_fifo_buffer, &tx_packet_buffer[1], size + 1);
		tx_packet_buffer[0] = 1;	/* report ID */
#else
		fifoBuf_getData(&tx_pios_fifo_buffer, &tx_packet_buffer[2], size);
		tx_packet_buffer[0] = 1;	/* report ID */
		tx_packet_buffer[1] = size;	/* valid data length */

#endif

		UserToPMABufferCopy((uint8_t *) tx_packet_buffer, GetEPTxAddr(EP1_IN & 0x7F), size + 2);
		SetEPTxCount((EP1_IN & 0x7F), PIOS_USB_HID_DATA_LENGTH + 2);

		/* Send Buffer */
		SetEPTxValid(ENDP1);
	} 

}

/**
 * Puts more than one byte onto the transmit buffer (used for atomic sends)
 * \param[in] *buffer pointer to buffer which should be transmitted
 * \param[in] len number of bytes which should be transmitted
 * \return 0 if no error
 * \return -1 if port unavailable (disconnected)
 * \return -2 if too many bytes to be send
 * \note Applications shouldn't call this function directly, instead please use \ref PIOS_COM layer functions
 */
int32_t PIOS_USB_HID_TxBufferPutMoreNonBlocking(uint8_t id, const uint8_t * buffer, uint16_t len)
{
	uint16_t ret;
	
	if(!transfer_possible)
		return -1;	
	
	if (len > fifoBuf_getFree(&tx_pios_fifo_buffer)) {
		sendChunk();    /* Try and send what's in the buffer though */
		return -2;	/* Cannot send all requested bytes */
	}

	/* don't check returned bytes because it should always succeed  */
	/* after previous thread and no meaningful way to deal with the */
	/* case it only buffers half the bytes                          */
#if defined(USE_FREERTOS)
	if(!xSemaphoreTake(pios_usb_tx_semaphore,10 / portTICK_RATE_MS))
		return -3;
#endif

	ret = fifoBuf_putData(&tx_pios_fifo_buffer, buffer, len);

#if defined(USE_FREERTOS)
	xSemaphoreGive(pios_usb_tx_semaphore);
#endif
	
	sendChunk();

	return 0;
}

/**
 * Puts more than one byte onto the transmit buffer (used for atomic sends)<br>
 * (Blocking Function)
 * \param[in] *buffer pointer to buffer which should be transmitted
 * \param[in] len number of bytes which should be transmitted
 * \return 0 if no error
 * \return -1 if too many bytes to be send
 * \note Applications shouldn't call this function directly, instead please use \ref PIOS_COM layer functions
 */
int32_t PIOS_USB_HID_TxBufferPutMore(uint8_t id, const uint8_t * buffer, uint16_t len)
{
	if(len > (fifoBuf_getUsed(&tx_pios_fifo_buffer) + fifoBuf_getFree(&tx_pios_fifo_buffer)))
		return -1;
	
	uint32_t error;
	while ((error = PIOS_USB_HID_TxBufferPutMoreNonBlocking(id, buffer, len)) == -2) {
#if defined(PIOS_INCLUDE_FREERTOS)
		taskYIELD();
#endif
	}

	return error;
}

/**
 * Gets a byte from the receive buffer
 * \return -1 if no new byte available
 * \return >= 0: received byte
 * \note Applications shouldn't call this function directly, instead please use \ref PIOS_COM layer functions
 */
int32_t PIOS_USB_HID_RxBufferGet(uint8_t id)
{
	uint8_t read;
	
	if(fifoBuf_getUsed(&rx_pios_fifo_buffer) == 0)
		return -1;
	
	read = fifoBuf_getByte(&rx_pios_fifo_buffer);
	
	// If endpoint was stalled and there is now space make it valid
	if ((GetEPRxStatus(ENDP1) != EP_RX_VALID) && (fifoBuf_getFree(&rx_pios_fifo_buffer) > 62)) {
		SetEPRxStatus(ENDP1, EP_RX_VALID);
	}
	return read;
}

/**
 * Returns number of used bytes in receive buffer
 * \return > 0: number of used bytes
 * \return 0 nothing available
 * \note Applications shouldn't call these functions directly, instead please use \ref PIOS_COM layer functions
 */
int32_t PIOS_USB_HID_RxBufferUsed(uint8_t id)
{
	return fifoBuf_getUsed(&rx_pios_fifo_buffer);
}

/**
 * @brief Callback used to indicate a transmission from device INto host completed
 * Checks if any data remains, pads it into HID packet and sends.
 */
void PIOS_USB_HID_EP1_IN_Callback(void)
{
	sendChunk();
}

/**
 * EP1 OUT Callback Routine
 */
void PIOS_USB_HID_EP1_OUT_Callback(void)
{
	uint32_t DataLength = 0;

	/* Read received data (63 bytes) */
	/* Get the number of received data on the selected Endpoint */
	DataLength = GetEPRxCount(ENDP1 & 0x7F);

	/* Use the memory interface function to write to the selected endpoint */
	PMAToUserBufferCopy((uint8_t *) &rx_packet_buffer[0], GetEPRxAddr(ENDP1 & 0x7F), DataLength);
	
	/* The first byte is report ID (not checked), the second byte is the valid data length */
#ifdef USB_HID
	fifoBuf_putData(&rx_pios_fifo_buffer, &rx_packet_buffer[1], PIOS_USB_HID_DATA_LENGTH + 1);
#else
	fifoBuf_putData(&rx_pios_fifo_buffer, &rx_packet_buffer[2], rx_packet_buffer[1]);
#endif
	
	// Only reactivate endpoint if available space in buffer
	if (fifoBuf_getFree(&rx_pios_fifo_buffer) > 62) {
		SetEPRxStatus(ENDP1, EP_RX_VALID);
	} else {
		SetEPRxStatus(ENDP1, EP_RX_NAK);
	}
}

#endif

/**
 * @}
 * @}
 */

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
#include "stm32f10x.h"

#if defined(PIOS_INCLUDE_USB_HID)

const struct pios_com_driver pios_usb_com_driver = {
  .tx_nb    = PIOS_USB_HID_TxBufferPutMoreNonBlocking,
  .tx       = PIOS_USB_HID_TxBufferPutMore,
  .rx       = PIOS_USB_HID_RxBufferGet,
  .rx_avail = PIOS_USB_HID_RxBufferUsed,
};

/* Rx/Tx status */
static volatile uint8_t rx_buffer_new_data_ctr = 0;
static volatile uint8_t rx_buffer_ix;
static uint8_t transfer_possible = 0;
static uint8_t rx_buffer[PIOS_USB_HID_DATA_LENGTH+2] = {0};

static uint8_t transmit_remaining;
static uint8_t *p_tx_buffer;
static uint8_t tx_buffer[PIOS_USB_HID_DATA_LENGTH+2] = {0};

/**
* Initialises USB COM layer
* \param[in] mode currently only mode 0 supported
* \return < 0 if initialisation failed
* \note Applications shouldn't call this function directly, instead please use \ref PIOS_COM layer functions
*/
int32_t PIOS_USB_HID_Init(uint32_t mode)
{
	/* Currently only mode 0 supported */
	if(mode != 0) {
		/* Unsupported mode */
		return -1;
	}
	
		
	/* Enable the USB Interrupts */
	/* 2 bit for pre-emption priority, 2 bits for subpriority */
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);  
	NVIC_InitStructure.NVIC_IRQChannel = USB_LP_CAN1_RX0_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	
	/* Select USBCLK source */
	RCC_USBCLKConfig(RCC_USBCLKSource_PLLCLK_1Div5);	
	/* Enable the USB clock */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USB, ENABLE);
	
	USB_Init();
	USB_SIL_Init();
	PIOS_LED_On(LED2);	
	
	return 0; /* No error */
}

/**
* This function is called by the USB driver on cable connection/disconnection
* \param[in] connected connection status (1 if connected)
* \return < 0 on errors
* \note Applications shouldn't call this function directly, instead please use \ref PIOS_COM layer functions
*/
int32_t PIOS_USB_HID_ChangeConnectionState(uint32_t Connected)
{
	/* In all cases: re-initialise USB HID driver */
	if(Connected) {
		transfer_possible = 1;
		//TODO: Check SetEPRxValid(ENDP1);
	} else {
		/* Cable disconnected: disable transfers */
		transfer_possible = 0;
	}
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
  return transfer_possible ? 1 : 0;
}

/**
  * Transmits the next byte in the buffer in report 1
  */
void PIOS_USB_HID_TxNextByte()
{
	uint8_t buf[3];
	if( transmit_remaining > 0 ) {
		transmit_remaining--;
		buf[0] = 1; // report ID 1
		buf[1] = 1; // *p_tx_buffer;
		buf[2] = 1;
		p_tx_buffer++;
		
		UserToPMABufferCopy((uint8_t*) buf, GetEPTxAddr(EP1_IN & 0x7F), 3);
		SetEPTxCount((EP1_IN & 0x7F), 3);
		
		/* Send Buffer */
		SetEPTxValid(ENDP1);
		
		PIOS_LED_Toggle( LED2 );
	}			
}

/**
* Puts more than one byte onto the transmit buffer (used for atomic sends)
* \param[in] *buffer pointer to buffer which should be transmitted
* \param[in] len number of bytes which should be transmitted
* \return 0 if no error
* \return -1 if too many bytes to be send
* \note Applications shouldn't call this function directly, instead please use \ref PIOS_COM layer functions
*/
int32_t PIOS_USB_HID_TxBufferPutMoreNonBlocking(uint8_t id, const uint8_t *buffer, uint16_t len)
{
	if(len > PIOS_USB_HID_DATA_LENGTH) {
		/* Cannot send all requested bytes */
		return -1;
	}	
	
	memcpy(&tx_buffer[2], buffer, len);
	tx_buffer[0] = 1; /* report ID */
	tx_buffer[1] = len; /* valid data length */
	UserToPMABufferCopy((uint8_t*) tx_buffer, GetEPTxAddr(EP1_IN & 0x7F), len+2);
	SetEPTxCount((EP1_IN & 0x7F), PIOS_USB_HID_DATA_LENGTH+2);
	
	/* Send Buffer */
	SetEPTxValid(ENDP1);
	
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
int32_t PIOS_USB_HID_TxBufferPutMore(uint8_t id, const uint8_t *buffer, uint16_t len)
{
  int32_t error;

  while((error = PIOS_USB_HID_TxBufferPutMoreNonBlocking(id, buffer, len)) == -2);

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
	if(rx_buffer_new_data_ctr == 0) {
		/* Nothing new in buffer */
		return -1;
	}

	/* There is still data in the buffer */
	uint8_t b = rx_buffer[rx_buffer_ix++];
	if(--rx_buffer_new_data_ctr == 0) {
		rx_buffer_ix = 2; //the two bytes are report ID and valid data length respectively
	}

	/* Return received byte */
	return b;
}

/**
* Returns number of used bytes in receive buffer
* \return > 0: number of used bytes
* \return 0 nothing available
* \note Applications shouldn't call these functions directly, instead please use \ref PIOS_COM layer functions
*/
int32_t PIOS_USB_HID_RxBufferUsed(uint8_t id)
{
	return rx_buffer_new_data_ctr;
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
	PMAToUserBufferCopy((uint8_t *) rx_buffer, GetEPRxAddr(ENDP1 & 0x7F), DataLength);

	/* We now have data waiting */
	rx_buffer_new_data_ctr = rx_buffer[1];
	SetEPRxStatus(ENDP1, EP_RX_VALID);
	PIOS_LED_Toggle(LED2);
}

#endif

/**
  * @}
  * @}
  */

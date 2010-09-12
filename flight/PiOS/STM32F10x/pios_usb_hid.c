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
#include "buffer.h"

#if defined(PIOS_INCLUDE_USB_HID)

const struct pios_com_driver pios_usb_com_driver = {
  .tx_nb    = PIOS_USB_HID_TxBufferPutMoreNonBlocking,
  .tx       = PIOS_USB_HID_TxBufferPutMore,
  .rx       = PIOS_USB_HID_RxBufferGet,
  .rx_avail = PIOS_USB_HID_RxBufferUsed,
};

// TODO: Eventually replace the transmit and receive buffers with bigger ring bufers
// so there isn't hte 64 byte cap in place by the USB interrupt packet definition

/* Rx/Tx status */
static volatile uint8_t rx_buffer_new_data_ctr = 0;
static volatile uint8_t rx_buffer_ix;
static uint8_t transfer_possible = 0;
static uint8_t rx_buffer[PIOS_USB_HID_DATA_LENGTH+2] = {0};
static uint8_t tx_buffer[PIOS_USB_HID_DATA_LENGTH+2] = {0};

#define TX_BUFFER_SIZE 128
#define RX_BUFFER_SIZE 128
cBuffer rxBuffer;
cBuffer txBuffer;
static uint8_t rxBufferSpace[TX_BUFFER_SIZE];
static uint8_t txBufferSpace[RX_BUFFER_SIZE];
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
    
    bufferInit(&rxBuffer, &rxBufferSpace[0], RX_BUFFER_SIZE);
    bufferInit(&txBuffer, &txBufferSpace[0], TX_BUFFER_SIZE);
	
    PIOS_USB_HID_Reenumerate();
    		
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
	
    /* Update the USB serial number from the chip */
    uint8_t sn[25];
    PIOS_SYS_SerialNumberGet((char *) sn);    
    for(uint8_t i = 0; sn[i] != '\0' && (2 * i) < PIOS_HID_StringSerial[0]; i ++) {
        PIOS_HID_StringSerial[2+2*i] = sn[i];
    }
    
	USB_Init();
	USB_SIL_Init();
	
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
  return (PIOS_USB_DETECT_GPIO_PORT->IDR & PIOS_USB_DETECT_GPIO_PIN) != 0 && transfer_possible ?  1 : 0;
}

void sendChunk()
{
    
    uint32_t size = bufferBufferedData(&txBuffer);
    if(size > 0)
    {
        if(size > PIOS_USB_HID_DATA_LENGTH)
            size = PIOS_USB_HID_DATA_LENGTH;

        bufferGetChunkFromFront(&txBuffer, &tx_buffer[2], size);
        tx_buffer[0] = 1; /* report ID */
        tx_buffer[1] = size; /* valid data length */
        
        /* Wait for any pending transmissions to complete */
        while(GetEPTxStatus(ENDP1) == EP_TX_VALID)
        {
			#if defined(PIOS_INCLUDE_FREERTOS)
				taskYIELD();
			#endif
        }
        
        UserToPMABufferCopy((uint8_t*) tx_buffer, GetEPTxAddr(EP1_IN & 0x7F), size+2);
        SetEPTxCount((EP1_IN & 0x7F), PIOS_USB_HID_DATA_LENGTH+2);
        
        /* Send Buffer */
        SetEPTxValid(ENDP1);
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
    /*if( len > PIOS_USB_HID_DATA_LENGTH )
        return - 1;*/
    
    uint8_t previous_data = bufferBufferedData(&txBuffer);
    
	if(len > bufferRemainingSpace(&txBuffer)) 
		return -1;		/* Cannot send all requested bytes */
	    
    if(bufferAddChunkToEnd(&txBuffer, buffer, len) == 0)
        return -1;
    
    /* If no previous data queued and not sending, then TX complete interrupt not likely so send manually */
    if(previous_data == 0 && GetEPTxStatus(ENDP1) != EP_TX_VALID)
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
int32_t PIOS_USB_HID_TxBufferPutMore(uint8_t id, const uint8_t *buffer, uint16_t len)
{
    uint32_t error;
    if((error = PIOS_USB_HID_TxBufferPutMoreNonBlocking(id, buffer, len)) != 0)
        return error;
    
    while( bufferBufferedData(&txBuffer) )
    {
		#if defined(PIOS_INCLUDE_FREERTOS)
			taskYIELD();
		#endif
    }
    
    return 0;
}

/**
* Gets a byte from the receive buffer
* \return -1 if no new byte available
* \return >= 0: received byte
* \note Applications shouldn't call this function directly, instead please use \ref PIOS_COM layer functions
*/
int32_t PIOS_USB_HID_RxBufferGet(uint8_t id)
{
    return bufferGetFromFront(&rxBuffer);
}

/**
* Returns number of used bytes in receive buffer
* \return > 0: number of used bytes
* \return 0 nothing available
* \note Applications shouldn't call these functions directly, instead please use \ref PIOS_COM layer functions
*/
int32_t PIOS_USB_HID_RxBufferUsed(uint8_t id)
{
	return bufferBufferedData(&rxBuffer);
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
	PMAToUserBufferCopy((uint8_t *) rx_buffer, GetEPRxAddr(ENDP1 & 0x7F), DataLength);

    /* The first byte is report ID (not checked), the second byte is the valid data length */
    bufferAddChunkToEnd(&rxBuffer, &rx_buffer[2], rx_buffer[1]);

	SetEPRxStatus(ENDP1, EP_RX_VALID);
}

#endif

/**
  * @}
  * @}
  */

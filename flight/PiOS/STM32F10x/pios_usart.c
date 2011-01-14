/**
 ******************************************************************************
 * @addtogroup PIOS PIOS Core hardware abstraction layer
 * @{
 * @addtogroup   PIOS_USART USART Functions
 * @brief PIOS interface for USART port
 * @{
 *
 * @file       pios_usart.c   
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * 	        Parts by Thorsten Klose (tk@midibox.org) (tk@midibox.org)
 * @brief      USART commands. Inits USARTs, controls USARTs & Interupt handlers. (STM32 dependent)
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

#if defined(PIOS_INCLUDE_USART)

#include <pios_usart_priv.h>

/* Provide a COM driver */
const struct pios_com_driver pios_usart_com_driver = {
	.set_baud = PIOS_USART_ChangeBaud,
	.tx_nb = PIOS_USART_TxBufferPutMoreNonBlocking,
	.tx = PIOS_USART_TxBufferPutMore,
	.rx = PIOS_USART_RxBufferGet,
	.rx_avail = PIOS_USART_RxBufferUsed,
};

static struct pios_usart_dev *find_usart_dev_by_id(uint8_t usart)
{
	if (usart >= pios_usart_num_devices) {
		/* Undefined USART port for this board (see pios_board.c) */
		return NULL;
	}

	/* Get a handle for the device configuration */
	return &(pios_usart_devs[usart]);
}

/**
* Initialise the onboard USARTs
*/
void PIOS_USART_Init(void)
{
	struct pios_usart_dev *usart_dev;
	uint8_t i;

	for (i = 0; i < pios_usart_num_devices; i++) {
		/* Get a handle for the device configuration */
		usart_dev = find_usart_dev_by_id(i);
		PIOS_DEBUG_Assert(usart_dev);

		/* Clear buffer counters */
		fifoBuf_init(&usart_dev->rx, usart_dev->rx_buffer, sizeof(usart_dev->rx_buffer));
		fifoBuf_init(&usart_dev->tx, usart_dev->tx_buffer, sizeof(usart_dev->tx_buffer));

		/* Enable the USART Pins Software Remapping */
		if (usart_dev->cfg->remap) {
			GPIO_PinRemapConfig(usart_dev->cfg->remap, ENABLE);
		}

		/* Initialize the USART Rx and Tx pins */
		GPIO_Init(usart_dev->cfg->rx.gpio, &usart_dev->cfg->rx.init);
		GPIO_Init(usart_dev->cfg->tx.gpio, &usart_dev->cfg->tx.init);

		/* Enable USART clock */
		switch ((uint32_t) usart_dev->cfg->regs) {
		case (uint32_t) USART1:
			RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
			break;
		case (uint32_t) USART2:
			RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
			break;
		case (uint32_t) USART3:
			RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);
			break;
		}

		/* Enable USART */
		USART_Init(usart_dev->cfg->regs, &usart_dev->cfg->init);

		/* Configure USART Interrupts */
		NVIC_Init(&usart_dev->cfg->irq.init);
		USART_ITConfig(usart_dev->cfg->regs, USART_IT_RXNE, ENABLE);
		USART_ITConfig(usart_dev->cfg->regs, USART_IT_TXE, ENABLE);

		/* Enable USART */
		USART_Cmd(usart_dev->cfg->regs, ENABLE);
	}
}

/**
* Changes the baud rate of the USART peripheral without re-initialising.
* \param[in] usart USART name (GPS, TELEM, AUX)
* \param[in] baud Requested baud rate
*/
void PIOS_USART_ChangeBaud(uint8_t usart, uint32_t baud)
{
	struct pios_usart_dev *usart_dev;
	USART_InitTypeDef USART_InitStructure;

	/* Get a handle for the device configuration */
	usart_dev = find_usart_dev_by_id(usart);

#if 0
	if (!usart_dev) {
		/* Undefined USART port for this board (see pios_board.c) */
		return -2;
	}
#endif

	/* Start with a copy of the default configuration for the peripheral */
	USART_InitStructure = usart_dev->cfg->init;

	/* Adjust the baud rate */
	USART_InitStructure.USART_BaudRate = baud;

	/* Write back the new configuration */
	USART_Init(usart_dev->cfg->regs, &USART_InitStructure);
}

/**
* Returns number of free bytes in receive buffer
* \param[in] USART USART name
* \return USART number of free bytes
* \return 1: USART available
* \return 0: USART not available
*/
int32_t PIOS_USART_RxBufferFree(uint8_t usart)
{
	struct pios_usart_dev *usart_dev;

	/* Get a handle for the device configuration */
	usart_dev = find_usart_dev_by_id(usart);

	if (!usart_dev) {
		/* Undefined USART port for this board (see pios_board.c) */
		return -2;
	}

	return fifoBuf_getFree(&usart_dev->rx);
}

/**
* Returns number of used bytes in receive buffer
* \param[in] USART USART name
* \return > 0: number of used bytes
* \return 0 if USART not available
* \note Applications shouldn't call these functions directly, instead please use \ref PIOS_COM layer functions
*/
int32_t PIOS_USART_RxBufferUsed(uint8_t usart)
{
	struct pios_usart_dev *usart_dev;

	/* Get a handle for the device configuration */
	usart_dev = find_usart_dev_by_id(usart);

	if (!usart_dev) {
		/* Undefined USART port for this board (see pios_board.c) */
		return -2;
	}

	return fifoBuf_getUsed(&usart_dev->rx);
}

/**
* Gets a byte from the receive buffer
* \param[in] USART USART name
* \return -1 if USART not available
* \return -2 if no new byte available
* \return >= 0: number of received bytes
* \note Applications shouldn't call these functions directly, instead please use \ref PIOS_COM layer functions
*/
int32_t PIOS_USART_RxBufferGet(uint8_t usart)
{
	struct pios_usart_dev *usart_dev;

	/* Get a handle for the device configuration */
	usart_dev = find_usart_dev_by_id(usart);

	if (!usart_dev) {
		/* Undefined USART port for this board (see pios_board.c) */
		return -2;
	}

	if (fifoBuf_getUsed(&usart_dev->rx) == 0) {
		/* Nothing new in the buffer */
		return -1;
	}

	/* get byte - this operation should be atomic! */
	/* PIOS_IRQ_Disable(); -- not needed only one reader */
	uint8_t b = fifoBuf_getByte(&usart_dev->rx);
	/* PIOS_IRQ_Enable(); */

	/* Return received byte */
	return b;
}

/**
* Returns the next byte of the receive buffer without taking it
* \param[in] USART USART name
* \return -1 if USART not available
* \return -2 if no new byte available
* \return >= 0: number of received bytes
* \note Applications shouldn't call these functions directly, instead please use \ref PIOS_COM layer functions
*/
int32_t PIOS_USART_RxBufferPeek(uint8_t usart)
{
	struct pios_usart_dev *usart_dev;

	/* Get a handle for the device configuration */
	usart_dev = find_usart_dev_by_id(usart);

	if (!usart_dev) {
		/* Undefined USART port for this board (see pios_board.c) */
		return -2;
	}

	if (!fifoBuf_getUsed(&usart_dev->rx)) {
		/* Nothing new in the buffer */
		return -1;
	}

	/* get byte - this operation should be atomic! */
	/* PIOS_IRQ_Disable(); -- not needed only one reader */
	uint8_t b = fifoBuf_getBytePeek(&usart_dev->rx);
	/* PIOS_IRQ_Enable();                          */

	/* Return received byte */
	return b;
}

/**
* puts a byte onto the receive buffer
* \param[in] USART USART name
* \param[in] b byte which should be put into Rx buffer
* \return 0 if no error
* \return -1 if USART not available
* \return -2 if buffer full (retry)
* \note Applications shouldn't call these functions directly, instead please use \ref PIOS_COM layer functions
*/
int32_t PIOS_USART_RxBufferPut(uint8_t usart, uint8_t b)
{
	struct pios_usart_dev *usart_dev;

	/* Get a handle for the device configuration */
	usart_dev = find_usart_dev_by_id(usart);

	if (!usart_dev) {
		/* Undefined USART port for this board (see pios_board.c) */
		return -1;
	}

	if (fifoBuf_getFree(&usart_dev->rx) < 1) {
		/* Buffer full (retry) */
		return -2;
	}

	/* Copy received byte into receive buffer */
	/* This operation should be atomic! */
	/* PIOS_IRQ_Disable(); -- not needed only one reader */
	fifoBuf_putByte(&usart_dev->rx,b);
	/* PIOS_IRQ_Enable(); */

	/* No error */
	return 0;
}

/**
* returns number of free bytes in transmit buffer
* \param[in] USART USART name
* \return number of free bytes
* \return 0 if USART not available
* \note Applications shouldn't call these functions directly, instead please use \ref PIOS_COM layer functions
*/
int32_t PIOS_USART_TxBufferFree(uint8_t usart)
{
	struct pios_usart_dev *usart_dev;

	/* Get a handle for the device configuration */
	usart_dev = find_usart_dev_by_id(usart);

	if (!usart_dev) {
		/* Undefined USART port for this board (see pios_board.c) */
		return 0;
	}

	return fifoBuf_getFree(&usart_dev->tx);
}

/**
* returns number of used bytes in transmit buffer
* \param[in] USART USART name
* \return number of used bytes
* \return 0 if USART not available
* \note Applications shouldn't call these functions directly, instead please use \ref PIOS_COM layer functions
*/
int32_t PIOS_USART_TxBufferUsed(uint8_t usart)
{
	struct pios_usart_dev *usart_dev;

	/* Get a handle for the device configuration */
	usart_dev = find_usart_dev_by_id(usart);

	if (!usart_dev) {
		/* Undefined USART port for this board (see pios_board.c) */
		return 0;
	}

	return fifoBuf_getUsed(&usart_dev->tx);
}

/**
* gets a byte from the transmit buffer
* \param[in] USART USART name
* \return -1 if USART not available
* \return -2 if no new byte available
* \return >= 0: transmitted byte
* \note Applications shouldn't call these functions directly, instead please use \ref PIOS_COM layer functions
*/
int32_t PIOS_USART_TxBufferGet(uint8_t usart)
{
	struct pios_usart_dev *usart_dev;

	/* Get a handle for the device configuration */
	usart_dev = find_usart_dev_by_id(usart);

	if (!usart_dev) {
		/* Undefined USART port for this board (see pios_board.c) */
		return -1;
	}

	if (fifoBuf_getUsed(&usart_dev->tx) == 0) {
		/* Nothing new in the buffer */
		return -2;
	}

	/* get byte - this operation should be atomic! */
	 PIOS_IRQ_Disable();
	uint8_t b = fifoBuf_getByte(&usart_dev->tx);
	PIOS_IRQ_Enable(); 

	/* Return received byte */
	return b;
}

/**
* puts more than one byte onto the transmit buffer (used for atomic sends)
* \param[in] USART USART name
* \param[in] *buffer pointer to buffer to be sent
* \param[in] len number of bytes to be sent
* \return 0 if no error
* \return -1 if USART not available
* \return -2 if buffer full or cannot get all requested bytes (retry)
* \return -3 if USART not supported by USARTTxBufferPut Routine
* \note Applications shouldn't call these functions directly, instead please use \ref PIOS_COM layer functions
*/
int32_t PIOS_USART_TxBufferPutMoreNonBlocking(uint8_t usart, const uint8_t * buffer, uint16_t len)
{
	struct pios_usart_dev *usart_dev;

	/* Get a handle for the device configuration */
	usart_dev = find_usart_dev_by_id(usart);

	if (!usart_dev) {
		/* Undefined USART port for this board (see pios_board.c) */
		return -1;
	}

	if (len >= fifoBuf_getFree(&usart_dev->tx)) {
		/* Buffer cannot accept all requested bytes (retry) */
		return -2;
	}

	/* Copy bytes to be transmitted into transmit buffer */
	/* This operation should be atomic!  Can't rely on   */
	/* fifoBuf since two tasks write to the port         */
	PIOS_IRQ_Disable();
	uint16_t used = fifoBuf_getUsed(&usart_dev->tx);
	fifoBuf_putData(&usart_dev->tx,buffer,len);
	
	if(used == 0) /* enable sending when was empty */
		USART_ITConfig(usart_dev->cfg->regs, USART_IT_TXE, ENABLE);
	PIOS_IRQ_Enable();

	/* No error */
	return 0;
}

/**
* puts more than one byte onto the transmit buffer (used for atomic sends)<BR>
* (blocking function)
* \param[in] USART USART name
* \param[in] *buffer pointer to buffer to be sent
* \param[in] len number of bytes to be sent
* \return 0 if no error
* \return -1 if USART not available
* \return -3 if USART not supported by USARTTxBufferPut Routine
* \note Applications shouldn't call these functions directly, instead please use \ref PIOS_COM layer functions
*/
int32_t PIOS_USART_TxBufferPutMore(uint8_t usart, const uint8_t * buffer, uint16_t len)
{
	int32_t rc;

	while ((rc = PIOS_USART_TxBufferPutMoreNonBlocking(usart, buffer, len)) == -2) ;

	return rc;
}

/**
* puts a byte onto the transmit buffer
* \param[in] USART USART name
* \param[in] b byte which should be put into Tx buffer
* \return 0 if no error
* \return -1 if USART not available
* \return -2 if buffer full (retry)
* \return -3 if USART not supported by USARTTxBufferPut Routine
* \note Applications shouldn't call these functions directly, instead please use \ref PIOS_COM layer functions
*/
int32_t PIOS_USART_TxBufferPut_NonBlocking(uint8_t usart, uint8_t b)
{
	return PIOS_USART_TxBufferPutMoreNonBlocking(usart, &b, 1);
}

/**
* puts a byte onto the transmit buffer<BR>
* (blocking function)
* \param[in] USART USART name
* \param[in] b byte which should be put into Tx buffer
* \return 0 if no error
* \return -1 if USART not available
* \return -3 if USART not supported by USARTTxBufferPut Routine
* \note Applications shouldn't call these functions directly, instead please use \ref PIOS_COM layer functions
*/
int32_t PIOS_USART_TxBufferPut(uint8_t usart, uint8_t b)
{
	return PIOS_USART_TxBufferPutMore(usart, &b, 1);
}

void PIOS_USART_IRQ_Handler(uint8_t usart)
{
	struct pios_usart_dev *usart_dev;

	/* Get a handle for the device configuration */
	usart_dev = find_usart_dev_by_id(usart);
	PIOS_DEBUG_Assert(usart_dev);

	/* Check if RXNE flag is set */
	if (usart_dev->cfg->regs->SR & USART_SR_RXNE) {
		uint8_t b = usart_dev->cfg->regs->DR;

		if (PIOS_USART_RxBufferPut(usart, b) < 0) {
			/* Here we could add some error handling */
		}
	}

	/* Check if TXE flag is set */
	if (usart_dev->cfg->regs->SR & USART_SR_TXE) {
		if (PIOS_USART_TxBufferUsed(usart) > 0) {
			int32_t b = PIOS_USART_TxBufferGet(usart);

			if (b < 0) {
				/* Here we could add some error handling */
				usart_dev->cfg->regs->DR = 0xff;
			} else {
				usart_dev->cfg->regs->DR = b & 0xff;
			}
		} else {
			/* Disable TXE interrupt (TXEIE=0) */
			USART_ITConfig(usart_dev->cfg->regs, USART_IT_TXE, DISABLE);
		}
	}
}

#endif

/**
  * @}
  * @}
  */

/**
 ******************************************************************************
 *
 * @file       pios_spektrum.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * 	        Parts by Thorsten Klose (tk@midibox.org) (tk@midibox.org)
 * @brief      USART commands. Inits USARTs, controls USARTs & Interrupt handlers.
 * @see        The GNU Public License (GPL) Version 3
 * @defgroup   PIOS_SPEKTRUM USART Functions
 * @{
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

#if defined(PIOS_INCLUDE_SPEKTRUM)
#if defined(PIOS_INCLUDE_PWM)
#error "Both PWM and SPEKTRUM input defined, choose only one"
#endif

/* Global Variables */


/* Local Variables, use pios_usart */
static uint8_t spektrum_buffer[PIOS_USART_RX_BUFFER_SIZE];
static volatile uint8_t spektrum_buffer_tail;
static volatile uint8_t spektrum_buffer_head;
static volatile uint8_t spektrum_buffer_size;


/**
* Initialise the onboard USARTs
*/
void PIOS_SPEKTRUM_Init(void)
{
#if (PIOS_SPEKTRUM_ENABLED)
	/* Clear buffer counters */
	spektrum_buffer_tail = spektrum_buffer_head = spektrum_buffer_size = 0;
	
	/* Configure USART Pins */
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_StructInit(&GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;

	/* Configure and Init USARTs */
	USART_InitTypeDef USART_InitStructure;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx;

	/* Configure the USART Interrupts */
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;

	/* Enable the USART Pins Software Remapping */
	PIOS_USART3_REMAP_FUNC;

	/* Configure and Init USART Rx input with internal pull-ups */
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Pin = PIOS_USART3_RX_PIN;
	GPIO_Init(PIOS_USART3_GPIO_PORT, &GPIO_InitStructure);

	/* Enable USART clock */
	PIOS_USART3_CLK_FUNC;

	/* Enable USART Receive interrupt */
	USART_InitStructure.USART_BaudRate = 115200;
	USART_Init(PIOS_USART3_USART, &USART_InitStructure);
	USART_ITConfig(PIOS_USART3_USART, USART_IT_RXNE, ENABLE);
	//USART_ITConfig(PIOS_USART3_USART, USART_IT_TXE, ENABLE);

	/* Configure the USART Interrupts */
	NVIC_InitStructure.NVIC_IRQChannel = PIOS_USART3_IRQ_CHANNEL;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = PIOS_USART3_NVIC_PRIO;
	NVIC_Init(&NVIC_InitStructure);
	USART_ITConfig(PIOS_USART3_USART, USART_IT_RXNE, ENABLE);

	/* Enable USART */
	USART_Cmd(PIOS_USART3_USART, ENABLE);
#endif

}

/**
* Returns number of free bytes in receive buffer
* \param[in] USART USART name
* \return USART number of free bytes
* \return 1: USART available
* \return 0: USART not available
*/
int32_t PIOS_SPEKTRUM_RxBufferFree(USARTNumTypeDef usart)
{
	if(usart >= PIOS_USART_NUM) {
		return 0;
	} else {
		return PIOS_USART_RX_BUFFER_SIZE - spektrum_buffer_size;
	}
}

/**
* Returns number of used bytes in receive buffer
* \param[in] USART USART name
* \return > 0: number of used bytes
* \return 0 if USART not available
* \note Applications shouldn't call these functions directly, instead please use \ref PIOS_COM layer functions
*/
int32_t PIOS_SPEKTRUM_RxBufferUsed(USARTNumTypeDef usart)
{
	if(usart >= PIOS_USART_NUM) {
		return 0;
	} else {
		return spektrum_buffer_size;
	}
}

/**
* Gets a byte from the receive buffer
* \param[in] USART USART name
* \return -1 if USART not available
* \return -2 if no new byte available
* \return >= 0: number of received bytes
* \note Applications shouldn't call these functions directly, instead please use \ref PIOS_COM layer functions
*/
int32_t PIOS_SPEKTRUM_RxBufferGet(USARTNumTypeDef usart)
{
	if(usart >= PIOS_USART_NUM) {
		/* USART not available */
		return -1;
	}
	if(!spektrum_buffer_size) {
		/* nothing new in buffer */
		return -2;
	}
	
	/* get byte - this operation should be atomic! */
	PIOS_IRQ_Disable();
	uint8_t b = spektrum_buffer[spektrum_buffer_tail];
	if(++spektrum_buffer_tail >= PIOS_USART_RX_BUFFER_SIZE) {
		spektrum_buffer_tail = 0;
	}
	--spektrum_buffer_size;
	PIOS_IRQ_Enable();
	
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
int32_t PIOS_SPEKTRUM_RxBufferPeek(USARTNumTypeDef usart)
{
	if(usart >= PIOS_USART_NUM) {
		/* USART not available */
		return -1;
	}

	if(!spektrum_buffer_size) {
		/* Nothing new in buffer */
		return -2;
	}

	/* Get byte - this operation should be atomic! */
	PIOS_IRQ_Disable();
	uint8_t b = spektrum_buffer[spektrum_buffer_tail];
	PIOS_IRQ_Enable();

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
int32_t PIOS_SPEKTRUM_RxBufferPut(USARTNumTypeDef usart, uint8_t b)
{
	if(usart >= PIOS_USART_NUM) {
		/* USART not available */
		return -1;
	}

	if(spektrum_buffer_size >= PIOS_USART_RX_BUFFER_SIZE) {
		/* Buffer full (retry) */
		return -2;
	}

	/* Copy received byte into receive buffer */
	/* This operation should be atomic! */
	PIOS_IRQ_Disable();
	spektrum_buffer[spektrum_buffer_head] = b;
	if(++spektrum_buffer_head >= PIOS_USART_RX_BUFFER_SIZE) {
		spektrum_buffer_head = 0;
	}
	++spektrum_buffer_size;
	PIOS_IRQ_Enable();

	/* No error */
	return 0;
}

#if (PIOS_SPEKTRUM_ENABLED)
/* Interrupt handler for USART3 */
PIOS_USART3_IRQHANDLER_FUNC
{
	/* check if RXNE flag is set */
	if(PIOS_USART3_USART->SR & (1 << 5)) {
		uint8_t b = PIOS_USART3_USART->DR;
		
		if(PIOS_USART_RxBufferPut(USART_3, b) < 0) {
			/* Here we could add some error handling */
		}
	}

	if(PIOS_USART3_USART->SR & (1 << 7)) { // check if TXE flag is set
		if(PIOS_USART_TxBufferUsed(USART_3) > 0) {
			int b = PIOS_USART_TxBufferGet(USART_3);
			if(b < 0) {
				/* Here we could add some error handling */
				PIOS_USART3_USART->DR = 0xff;
			} else {
				PIOS_USART3_USART->DR = b;
			}
		} else {
			/* Disable TXE interrupt (TXEIE=0) */
			PIOS_USART3_USART->CR1 &= ~(1 << 7);
		}
	}
}
#endif

#endif

/**
 ******************************************************************************
 *
 * @file       pios_usart.c   
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * 	        Parts by Thorsten Klose (tk@midibox.org) (tk@midibox.org)
 * @brief      USART commands. Inits USARTs, controls USARTs & Interupt handlers.
 * @see        The GNU Public License (GPL) Version 3
 * @defgroup   PIOS_USART USART Functions
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

#if defined(PIOS_INCLUDE_USART)


/* Global Variables */
extern SettingsTypeDef Settings;
xSemaphoreHandle PIOS_USART1_Buffer;
xSemaphoreHandle PIOS_USART2_Buffer;
xSemaphoreHandle PIOS_USART3_Buffer;
static portBASE_TYPE xHigherPriorityTaskWoken;

/* Local Variables */
static uint8_t rx_buffer[PIOS_USART_NUM][PIOS_USART_RX_BUFFER_SIZE];
static volatile uint8_t rx_buffer_tail[PIOS_USART_NUM];
static volatile uint8_t rx_buffer_head[PIOS_USART_NUM];
static volatile uint8_t rx_buffer_size[PIOS_USART_NUM];

static uint8_t tx_buffer[PIOS_USART_NUM][PIOS_USART_TX_BUFFER_SIZE];
static volatile uint8_t tx_buffer_tail[PIOS_USART_NUM];
static volatile uint8_t tx_buffer_head[PIOS_USART_NUM];
static volatile uint8_t tx_buffer_size[PIOS_USART_NUM];


/**
* Initialise the GPS and TELEM onboard USARTs
*/
void PIOS_USART_Init(void)
{
	/* Clear buffer counters */
	uint8_t i;
	for(i = 0; i < PIOS_USART_NUM; ++i) {
		rx_buffer_tail[i] = rx_buffer_head[i] = rx_buffer_size[i] = 0;
		tx_buffer_tail[i] = tx_buffer_head[i] = tx_buffer_size[i] = 0;
	}
	
	/* Configure USART Pins */
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_StructInit(&GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD;

	/* Configure and Init USARTs */
	USART_InitTypeDef USART_InitStructure;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

	/* Configure the USART Interrupts */
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;

#if (PIOS_USART1_ENABLED)
	/* Enable the USART Pins Software Remapping */
	PIOS_USART1_REMAP_FUNC;

	/* Configure and Init USART Tx as alternate function open-drain */
	GPIO_InitStructure.GPIO_Pin = PIOS_USART1_TX_PIN;
	GPIO_Init(PIOS_USART1_GPIO_PORT, &GPIO_InitStructure);
	
	/* Configure and Init USART Rx input with internal pull-ups */
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Pin = PIOS_USART1_RX_PIN;
	GPIO_Init(PIOS_USART1_GPIO_PORT, &GPIO_InitStructure);

	/* Enable USART clock */
	PIOS_USART1_CLK_FUNC;

	/* Enable USART Receive and Transmit interrupts */
	USART_InitStructure.USART_BaudRate = PIOS_USART1_BAUDRATE;
	USART_Init(PIOS_USART1_USART, &USART_InitStructure);
	USART_ITConfig(PIOS_USART1_USART, USART_IT_RXNE, ENABLE);
	USART_ITConfig(PIOS_USART1_USART, USART_IT_TXE, ENABLE);

	/* Configure the USART Interrupts */
	NVIC_InitStructure.NVIC_IRQChannel = PIOS_USART1_IRQ_CHANNEL;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = PIOS_USART1_NVIC_PRIO;
	NVIC_Init(&NVIC_InitStructure);
	USART_ITConfig(PIOS_USART1_USART, USART_IT_RXNE, ENABLE);

	/* Enable USART */
	USART_Cmd(PIOS_USART1_USART, ENABLE);
#endif

#if (PIOS_USART2_ENABLED)
	/* Enable the USART Pins Software Remapping */
	PIOS_USART2_REMAP_FUNC;

	/* Configure and Init USART Tx as alternate function open-drain */
	GPIO_InitStructure.GPIO_Pin = PIOS_USART2_TX_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD;
	GPIO_Init(PIOS_USART2_GPIO_PORT, &GPIO_InitStructure);

	/* Configure and Init USART Rx input with internal pull-ups */
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Pin = PIOS_USART2_RX_PIN;
	GPIO_Init(PIOS_USART2_GPIO_PORT, &GPIO_InitStructure);

	/* Enable USART clock */
	PIOS_USART2_CLK_FUNC;

	/* Enable USART Receive and Transmit interrupts */
	USART_InitStructure.USART_BaudRate = PIOS_USART2_BAUDRATE;
	USART_Init(PIOS_USART2_USART, &USART_InitStructure);
	USART_ITConfig(PIOS_USART2_USART, USART_IT_RXNE, ENABLE);
	USART_ITConfig(PIOS_USART2_USART, USART_IT_TXE, ENABLE);

	/* Configure the USART Interrupts */
	NVIC_InitStructure.NVIC_IRQChannel = PIOS_USART2_IRQ_CHANNEL;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = PIOS_USART2_NVIC_PRIO;
	NVIC_Init(&NVIC_InitStructure);
	USART_ITConfig(PIOS_USART2_USART, USART_IT_RXNE, ENABLE);

	/* Enable USART */
	USART_Cmd(PIOS_USART2_USART, ENABLE);
#endif

#if (PIOS_USART3_ENABLED)
	/* Enable the USART Pins Software Remapping */
	PIOS_USART3_REMAP_FUNC;

	/* Configure and Init USART Tx as alternate function open-drain */
	GPIO_InitStructure.GPIO_Pin = PIOS_USART3_TX_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD;
	GPIO_Init(PIOS_USART3_GPIO_PORT, &GPIO_InitStructure);

	/* Configure and Init USART Rx input with internal pull-ups */
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Pin = PIOS_USART3_RX_PIN;
	GPIO_Init(PIOS_USART3_GPIO_PORT, &GPIO_InitStructure);

	/* Enable USART clock */
	PIOS_USART3_CLK_FUNC;

	/* Enable USART Receive and Transmit interrupts */
	USART_InitStructure.USART_BaudRate = PIOS_USART3_BAUDRATE;
	USART_Init(PIOS_USART3_USART, &USART_InitStructure);
	USART_ITConfig(PIOS_USART3_USART, USART_IT_RXNE, ENABLE);
	USART_ITConfig(PIOS_USART3_USART, USART_IT_TXE, ENABLE);

	/* Configure the USART Interrupts */
	NVIC_InitStructure.NVIC_IRQChannel = PIOS_USART3_IRQ_CHANNEL;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = PIOS_USART3_NVIC_PRIO;
	NVIC_Init(&NVIC_InitStructure);
	USART_ITConfig(PIOS_USART3_USART, USART_IT_RXNE, ENABLE);

	/* Enable USART */
	USART_Cmd(PIOS_USART3_USART, ENABLE);
#endif

	vSemaphoreCreateBinary(PIOS_USART1_Buffer);
	vSemaphoreCreateBinary(PIOS_USART2_Buffer);
	vSemaphoreCreateBinary(PIOS_USART3_Buffer);
}

/**
* Changes the baud rate of the USART peripheral without re-initialising.
* \param[in] USARTx USART name (GPS, TELEM, AUX)
* \param[in] Baud Requested baud rate         
*/
void PIOS_USART_ChangeBaud(USART_TypeDef* USARTx, uint32_t Baud)
{
	/* USART BRR Configuration */
	/* Configure the USART Baud Rate */
	/* Adapted from stm32f01x_usart.c */
	
	uint32_t TmpReg = 0x00, ApbClock = 0x00;
	uint32_t IntegerDivider = 0x00;
	uint32_t FractionalDivider = 0x00;
	uint32_t USARTxBase = (uint32_t)USARTx;
	RCC_ClocksTypeDef RCC_ClocksStatus;
	
	RCC_GetClocksFreq(&RCC_ClocksStatus);
	if (USARTxBase == USART1_BASE) {
		ApbClock = RCC_ClocksStatus.PCLK2_Frequency;
	} else {
		ApbClock = RCC_ClocksStatus.PCLK1_Frequency;
	}
	
	/* Determine the integer part */
	IntegerDivider = ((0x19 * ApbClock) / (0x04 * (Baud)));
	TmpReg = (IntegerDivider / 0x64) << 0x04;
	
	/* Determine the fractional part */
	FractionalDivider = IntegerDivider - (0x64 * (TmpReg >> 0x04));
	TmpReg |= ((((FractionalDivider * 0x10) + 0x32) / 0x64)) & ((uint8_t)0x0F);
	
	/* Write to USART BRR */
	USARTx->BRR = (uint16_t)TmpReg;
}

/**
* Returns number of free bytes in receive buffer
* \param[in] USART USART name
* \return USART number of free bytes
* \return 1: USART available
* \return 0: USART not available
*/
int32_t PIOS_USART_RxBufferFree(USARTNumTypeDef usart)
{
	if(usart >= PIOS_USART_NUM) {
		return 0;
	} else {
		return PIOS_USART_RX_BUFFER_SIZE - rx_buffer_size[usart];
	}
}

/**
* Returns number of used bytes in receive buffer
* \param[in] USART USART name
* \return > 0: number of used bytes
* \return 0 if USART not available
* \note Applications shouldn't call these functions directly, instead please use \ref PIOS_COM layer functions
*/
int32_t PIOS_USART_RxBufferUsed(USARTNumTypeDef usart)
{
	if(usart >= PIOS_USART_NUM) {
		return 0;
	} else {
		return rx_buffer_size[usart];
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
int32_t PIOS_USART_RxBufferGet(USARTNumTypeDef usart)
{
	if(usart >= PIOS_USART_NUM) {
		/* USART not available */
		return -1;
	}
	if(!rx_buffer_size[usart]) {
		/* nothing new in buffer */
		return -2;
	}
	
	/* get byte - this operation should be atomic! */
	PIOS_IRQ_Disable();
	uint8_t b = rx_buffer[usart][rx_buffer_tail[usart]];
	if(++rx_buffer_tail[usart] >= PIOS_USART_RX_BUFFER_SIZE) {
		rx_buffer_tail[usart] = 0;
	}
	--rx_buffer_size[usart];
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
int32_t PIOS_USART_RxBufferPeek(USARTNumTypeDef usart)
{
	if(usart >= PIOS_USART_NUM) {
		/* USART not available */
		return -1;
	}

	if(!rx_buffer_size[usart]) {
		/* Nothing new in buffer */
		return -2;
	}

	/* Get byte - this operation should be atomic! */
	PIOS_IRQ_Disable();
	uint8_t b = rx_buffer[usart][rx_buffer_tail[usart]];
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
int32_t PIOS_USART_RxBufferPut(USARTNumTypeDef usart, uint8_t b)
{
	if(usart >= PIOS_USART_NUM) {
		/* USART not available */
		return -1;
	}

	if(rx_buffer_size[usart] >= PIOS_USART_RX_BUFFER_SIZE) {
		/* Buffer full (retry) */
		return -2;
	}

	/* Copy received byte into receive buffer */
	/* This operation should be atomic! */
	PIOS_IRQ_Disable();
	rx_buffer[usart][rx_buffer_head[usart]] = b;
	if(++rx_buffer_head[usart] >= PIOS_USART_RX_BUFFER_SIZE) {
		rx_buffer_head[usart] = 0;
	}
	++rx_buffer_size[usart];
	PIOS_IRQ_Enable();


	switch(usart) {
		case USART_1:
			xSemaphoreGiveFromISR(PIOS_USART1_Buffer, &xHigherPriorityTaskWoken);
			break;
		case USART_2:
			xSemaphoreGiveFromISR(PIOS_USART2_Buffer, &xHigherPriorityTaskWoken);
			break;
		case USART_3:
			xSemaphoreGiveFromISR(PIOS_USART3_Buffer, &xHigherPriorityTaskWoken);
			break;
	}

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
int32_t PIOS_USART_TxBufferFree(USARTNumTypeDef usart)
{
	if(usart >= PIOS_USART_NUM) {
		return 0;
	} else {
		return PIOS_USART_TX_BUFFER_SIZE - tx_buffer_size[usart];
	}
}

/**
* returns number of used bytes in transmit buffer
* \param[in] USART USART name
* \return number of used bytes
* \return 0 if USART not available
* \note Applications shouldn't call these functions directly, instead please use \ref PIOS_COM layer functions
*/
int32_t PIOS_USART_TxBufferUsed(USARTNumTypeDef usart)
{
	if(usart >= PIOS_USART_NUM) {
		return 0;
	} else {
		return tx_buffer_size[usart];
	}
}

/**
* gets a byte from the transmit buffer
* \param[in] USART USART name
* \return -1 if USART not available
* \return -2 if no new byte available
* \return >= 0: transmitted byte
* \note Applications shouldn't call these functions directly, instead please use \ref PIOS_COM layer functions
*/
int32_t PIOS_USART_TxBufferGet(USARTNumTypeDef usart)
{
	if(usart >= PIOS_USART_NUM) {
		/* USART not available */
		return -1;
	}

	if(!tx_buffer_size[usart]) {
		/* Nothing new in buffer */
		return -2;
	}

	/* Get byte - this operation should be atomic! */
	PIOS_IRQ_Disable();
	uint8_t b = tx_buffer[usart][tx_buffer_tail[usart]];
	if(++tx_buffer_tail[usart] >= PIOS_USART_TX_BUFFER_SIZE) {
		tx_buffer_tail[usart] = 0;
	}
	--tx_buffer_size[usart];
	PIOS_IRQ_Enable();

	/* Return transmitted byte */
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
int32_t PIOS_USART_TxBufferPutMoreNonBlocking(USARTNumTypeDef usart, uint8_t *buffer, uint16_t len)
{
	if(usart >= PIOS_USART_NUM) {
		/* USART not available */
		return -1;
	}

	if((tx_buffer_size[usart]+len) >= PIOS_USART_TX_BUFFER_SIZE) {
		/* Buffer full or cannot get all requested bytes (retry) */
		return -2;
	}

	/* Copy bytes to be transmitted into transmit buffer */
	/* This operation should be atomic! */
	PIOS_IRQ_Disable();

	uint16_t i;
	for(i = 0; i < len; ++i) {
		tx_buffer[usart][tx_buffer_head[usart]] = *buffer++;
		
		if(++tx_buffer_head[usart] >= PIOS_USART_TX_BUFFER_SIZE) {
			tx_buffer_head[usart] = 0;
		}
		
		/* Enable Tx interrupt if buffer was empty */
		if(++tx_buffer_size[usart] == 1) {
			switch(usart) {
				/* Enable TXE interrupt (TXEIE=1) */
				case 0: PIOS_USART1_USART->CR1 |= (1 << 7); break;
				/* Enable TXE interrupt (TXEIE=1) */
				case 1: PIOS_USART2_USART->CR1 |= (1 << 7); break;
				/* Enable TXE interrupt (TXEIE=1) */
				case 2: PIOS_USART3_USART->CR1 |= (1 << 7); break;
				/* USART not supported by routine (yet) */
				default: PIOS_IRQ_Enable(); return -3;
			}
		}
	}

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
int32_t PIOS_USART_TxBufferPutMore(USARTNumTypeDef usart, uint8_t *buffer, uint16_t len)
{
	int error;

	while((error = PIOS_USART_TxBufferPutMoreNonBlocking(usart, buffer, len)) == -2);

	return error;
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
int32_t PIOS_USART_TxBufferPut_NonBlocking(USARTNumTypeDef usart, uint8_t b)
{
	/* For more comfortable usage... */
	/* -> Just forward to USARTTxBufferPutMore */
	return PIOS_USART_TxBufferPutMore(usart, &b, 1);
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
int32_t PIOS_USART_TxBufferPut(USARTNumTypeDef usart, uint8_t b)
{
	int error;

	while((error = PIOS_USART_TxBufferPutMore(usart, &b, 1)) == -2);

	return error;
}

#if (PIOS_USART1_ENABLED)
/* Interrupt handler for USART1 */
PIOS_USART1_IRQHANDLER_FUNC
{
	/* Check if RXNE flag is set */
	if(PIOS_USART1_USART->SR & (1 << 5)) {
		uint8_t b = PIOS_USART1_USART->DR;
		
		if(PIOS_USART_RxBufferPut(USART_1, b) < 0) {
			/* Here we could add some error handling */
		}
	}

	/* Check if TXE flag is set */
	if(PIOS_USART1_USART->SR & (1 << 7)) {
		if(PIOS_USART_TxBufferUsed(USART_1) > 0) {
			int b = PIOS_USART_TxBufferGet(USART_1);
			if( b < 0 ) {
				/* Here we could add some error handling */
				PIOS_USART1_USART->DR = 0xff;
			} else {
				PIOS_USART1_USART->DR = b;
			}
		} else {
			/* Disable TXE interrupt (TXEIE=0) */
			PIOS_USART1_USART->CR1 &= ~(1 << 7);
		}
	}
}
#endif

#if (PIOS_USART1_ENABLED)
/* Interrupt handler for USART2 */
PIOS_USART2_IRQHANDLER_FUNC
{
	/* check if RXNE flag is set */
	if(PIOS_USART2_USART->SR & (1 << 5)) {
		uint8_t b = PIOS_USART2_USART->DR;
		
		if(PIOS_USART_RxBufferPut(USART_2, b) < 0) {
			/* Here we could add some error handling */
		}
	}
	
	/* Check if TXE flag is set */
	if(PIOS_USART2_USART->SR & (1 << 7)) {
		if(PIOS_USART_TxBufferUsed(USART_2) > 0) {
			int b = PIOS_USART_TxBufferGet(USART_2);
			if(b < 0) {
				/* Here we could add some error handling */
				PIOS_USART2_USART->DR = 0xff;
			} else {
				PIOS_USART2_USART->DR = b;
			}
		} else {
			/* Disable TXE interrupt (TXEIE=0) */
			PIOS_USART2_USART->CR1 &= ~(1 << 7);
		}
	}
}
#endif

#if (PIOS_USART1_ENABLED)
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

/**
 ******************************************************************************
 *
 * @file       pios_uart.c   
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2009.   
 * @brief      UART commands. Inits UARTs, controls UARTs & Interupt handlers.
 * @see        The GNU Public License (GPL) Version 3
 * @defgroup   PIOS_UART UART Functions
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


/* Private Function Prototypes */


/* Local Variables */
static uint8_t rx_buffer[UART_NUM][UART_RX_BUFFER_SIZE];
static volatile uint8_t rx_buffer_tail[UART_NUM];
static volatile uint8_t rx_buffer_head[UART_NUM];
static volatile uint8_t rx_buffer_size[UART_NUM];

static uint8_t tx_buffer[UART_NUM][UART_TX_BUFFER_SIZE];
static volatile uint8_t tx_buffer_tail[UART_NUM];
static volatile uint8_t tx_buffer_head[UART_NUM];
static volatile uint8_t tx_buffer_size[UART_NUM];


/**
* Initialise the GPS and TELEM onboard UARTs
*/
void UARTInit(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	
	/* Enable the UART Pins Software Remapping */
	GPS_REMAP_FUNC;
	TELEM_REMAP_FUNC;
	
	/* Configure UART Pins */
	GPIO_StructInit(&GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD;
	
	/* Configure and Init UART_GPS Tx as alternate function open-drain */
	GPIO_InitStructure.GPIO_Pin = GPS_TX_PIN;
	GPIO_Init(GPS_GPIO_PORT, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = TELEM_TX_PIN;
	GPIO_Init(TELEM_GPIO_PORT, &GPIO_InitStructure);
	
	/* Configure and Init UART Rx input with internal pull-ups */
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	
	GPIO_InitStructure.GPIO_Pin = GPS_RX_PIN;
	GPIO_Init(GPS_GPIO_PORT, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = TELEM_RX_PIN;
	GPIO_Init(TELEM_GPIO_PORT, &GPIO_InitStructure);

	/* Enable UART clocks */
	GPS_CLK_FUNC;
	TELEM_CLK_FUNC;

	/* Configure and Init UARTs */
	USART_InitTypeDef USART_InitStructure;
	
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	
	USART_InitStructure.USART_BaudRate = Settings.GPS.Baudrate;
	USART_Init(GPS_UART, &USART_InitStructure);
	
	USART_InitStructure.USART_BaudRate = Settings.Telem.Baudrate;
	USART_Init(TELEM_UART, &USART_InitStructure);
	
	/* Enable UART Receive and Transmit interrupts */
	USART_ITConfig(GPS_UART, USART_IT_RXNE, ENABLE);
	USART_ITConfig(GPS_UART, USART_IT_TXE, ENABLE);
	
	USART_ITConfig(TELEM_UART, USART_IT_RXNE, ENABLE);
	USART_ITConfig(TELEM_UART, USART_IT_TXE, ENABLE);
	
	/* Configure the UART Interrupts */
	NVIC_InitTypeDef NVIC_InitStructure;
	
	NVIC_InitStructure.NVIC_IRQChannel = GPS_IRQ_CHANNEL;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = GPS_NVIC_PRIO;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	USART_ITConfig(GPS_UART, USART_IT_RXNE, ENABLE);
	
	NVIC_InitStructure.NVIC_IRQChannel = TELEM_IRQ_CHANNEL;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = TELEM_NVIC_PRIO;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	USART_ITConfig(TELEM_UART, USART_IT_RXNE, ENABLE);
	
	/* Clear buffer counters */
	int i;
	for(i=0; i<UART_NUM; ++i) {
		rx_buffer_tail[i] = rx_buffer_head[i] = rx_buffer_size[i] = 0;
		tx_buffer_tail[i] = tx_buffer_head[i] = tx_buffer_size[i] = 0;
	}
	
	/* Enable USARTs */
	USART_Cmd(GPS_UART, ENABLE);
	
	USART_Cmd(TELEM_UART, ENABLE);
	
	if(Settings.AuxUART.Enabled == 1) {
		EnableAuxUART();
	}
}


/**
* Enables AUX UART at the expense of servo inputs
*/
void EnableAuxUART(void)
{
	//Implement after servo inputs are implemented
}


/**
* Disables AUX UART reclaims two servo inputs
*/
void DisableAuxUART(void)
{
	//Implement after servo inputs are implemented
}


/**
* Changes the baud rate of the USART peripherial without re-initialising.
* \param[in] USARTx UART name (GPS, TELEM, AUX)
* \param[in] Baud Requested baud rate         
*/
void UARTChangeBaud(USART_TypeDef* USARTx, uint32_t Baud)
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
* \param[in] uart UART name (GPS, TELEM, AUX)
* \return uart number of free bytes
* \return 1: uart available
* \return 0: uart not available
*/
int UARTRxBufferFree(UARTNumTypeDef uart)
{
	if(uart >= UART_NUM) {
		return 0;
	} else {
		return UART_RX_BUFFER_SIZE - rx_buffer_size[uart];
	}
}

/**
* Returns number of used bytes in receive buffer
* \param[in] uart UART name (GPS, TELEM, AUX)
* \return > 0: number of used bytes
* \return 0 if uart not available
* \note Applications shouldn't call these functions directly, instead please use \ref PIOS_COM layer functions
*/
int UARTRxBufferUsed(UARTNumTypeDef uart)
{
	if(uart >= UART_NUM) {
		return 0;
	} else {
		return rx_buffer_size[uart];
	}
}

/**
* Gets a byte from the receive buffer
* \param[in] uart UART name (GPS, TELEM, AUX)
* \return -1 if UART not available
* \return -2 if no new byte available
* \return >= 0: number of received bytes
* \note Applications shouldn't call these functions directly, instead please use \ref PIOS_COM layer functions
*/
int UARTRxBufferGet(UARTNumTypeDef uart)
{
	if(uart >= UART_NUM) {
		/* UART not available */
		return -1;
	}
	if(!rx_buffer_size[uart]) {
		/* nothing new in buffer */
		return -2;
	}
	
	/* get byte - this operation should be atomic! */
	IRQDisable();
	uint8_t b = rx_buffer[uart][rx_buffer_tail[uart]];
	if(++rx_buffer_tail[uart] >= UART_RX_BUFFER_SIZE) {
		rx_buffer_tail[uart] = 0;
	}
	--rx_buffer_size[uart];
	IRQEnable();
	
	/* Return received byte */
	return b;
}

/**
* Returns the next byte of the receive buffer without taking it
* \param[in] uart UART name (GPS, TELEM, AUX)
* \return -1 if UART not available
* \return -2 if no new byte available
* \return >= 0: number of received bytes
* \note Applications shouldn't call these functions directly, instead please use \ref PIOS_COM layer functions
*/
int UARTRxBufferPeek(UARTNumTypeDef uart)
{
	if(uart >= UART_NUM) {
		/* UART not available */
		return -1;
	}

	if(!rx_buffer_size[uart]) {
		/* Nothing new in buffer */
		return -2;
	}

	/* Get byte - this operation should be atomic! */
	IRQDisable();
	uint8_t b = rx_buffer[uart][rx_buffer_tail[uart]];
	IRQEnable();

	/* Return received byte */
	return b;
}

/**
* puts a byte onto the receive buffer
* \param[in] uart UART name (GPS, TELEM, AUX)
* \param[in] b byte which should be put into Rx buffer
* \return 0 if no error
* \return -1 if UART not available
* \return -2 if buffer full (retry)
* \note Applications shouldn't call these functions directly, instead please use \ref PIOS_COM layer functions
*/
int UARTRxBufferPut(UARTNumTypeDef uart, uint8_t b)
{
	if(uart >= UART_NUM) {
		/* UART not available */
		return -1;
	}

	if(rx_buffer_size[uart] >= UART_RX_BUFFER_SIZE) {
		/* Buffer full (retry) */
		return -2;
	}

	/* Copy received byte into receive buffer */
	/* This operation should be atomic! */
	IRQDisable();
	rx_buffer[uart][rx_buffer_head[uart]] = b;
	if(++rx_buffer_head[uart] >= UART_RX_BUFFER_SIZE) {
		rx_buffer_head[uart] = 0;
	}
	++rx_buffer_size[uart];
	IRQEnable();
	
	/* No error */
	return 0;
}


/**
* returns number of free bytes in transmit buffer
* \param[in] uart UART name (GPS, TELEM, AUX)
* \return number of free bytes
* \return 0 if uart not available
* \note Applications shouldn't call these functions directly, instead please use \ref PIOS_COM layer functions
*/
int UARTTxBufferFree(UARTNumTypeDef uart)
{
	if(uart >= UART_NUM) {
		return 0;
	} else {
		return UART_TX_BUFFER_SIZE - tx_buffer_size[uart];
	}
}

/**
* returns number of used bytes in transmit buffer
* \param[in] uart UART name (GPS, TELEM, AUX)
* \return number of used bytes
* \return 0 if uart not available
* \note Applications shouldn't call these functions directly, instead please use \ref PIOS_COM layer functions
*/
int UARTTxBufferUsed(UARTNumTypeDef uart)
{
	if(uart >= UART_NUM) {
		return 0;
	} else {
		return tx_buffer_size[uart];
	}
}

/**
* gets a byte from the transmit buffer
* \param[in] uart UART name (GPS, TELEM, AUX)
* \return -1 if UART not available
* \return -2 if no new byte available
* \return >= 0: transmitted byte
* \note Applications shouldn't call these functions directly, instead please use \ref PIOS_COM layer functions
*/
int UARTTxBufferGet(UARTNumTypeDef uart)
{
	if(uart >= UART_NUM) {
		/* UART not available */
		return -1;
	}

	if(!tx_buffer_size[uart]) {
		/* Nothing new in buffer */
		return -2;
	}

	/* Get byte - this operation should be atomic! */
	IRQDisable();
	uint8_t b = tx_buffer[uart][tx_buffer_tail[uart]];
	if(++tx_buffer_tail[uart] >= UART_TX_BUFFER_SIZE) {
		tx_buffer_tail[uart] = 0;
	}
	--tx_buffer_size[uart];
	IRQEnable();

	/* Return transmitted byte */
	return b;
}

/**
* puts more than one byte onto the transmit buffer (used for atomic sends)
* \param[in] uart UART name (GPS, TELEM, AUX)
* \param[in] *buffer pointer to buffer to be sent
* \param[in] len number of bytes to be sent
* \return 0 if no error
* \return -1 if UART not available
* \return -2 if buffer full or cannot get all requested bytes (retry)
* \return -3 if UART not supported by MIOS32_UART_TxBufferPut Routine
* \note Applications shouldn't call these functions directly, instead please use \ref PIOS_COM layer functions
*/
int UARTTxBufferPutMoreNonBlocking(UARTNumTypeDef uart, uint8_t *buffer, uint16_t len)
{
	if(uart >= UART_NUM) {
		/* UART not available */
		return -1;
	}

	if((tx_buffer_size[uart]+len) >= UART_TX_BUFFER_SIZE) {
		/* Buffer full or cannot get all requested bytes (retry) */
		return -2;
	}

	/* Copy bytes to be transmitted into transmit buffer */
	/* This operation should be atomic! */
	IRQDisable();

	uint16_t i;
	for(i = 0; i < len; ++i) {
		tx_buffer[uart][tx_buffer_head[uart]] = *buffer++;
		
		if(++tx_buffer_head[uart] >= UART_TX_BUFFER_SIZE) {
			tx_buffer_head[uart] = 0;
		}
		
		/* Enable Tx interrupt if buffer was empty */
		if(++tx_buffer_size[uart] == 1) {
			switch(uart) {
				/* Enable TXE interrupt (TXEIE=1) */
				case 0: GPS_UART->CR1 |= (1 << 7); break;
				/* Enable TXE interrupt (TXEIE=1) */
				case 1: TELEM_UART->CR1 |= (1 << 7); break;
				/* Enable TXE interrupt (TXEIE=1) */
				case 2: AUX_UART_UART->CR1 |= (1 << 7); break;
				/* Uart not supported by routine (yet) */
				default: IRQEnable(); return -3;
			}
		}
	}

	IRQEnable();

	/* No error */
	return 0;
}

/**
* puts more than one byte onto the transmit buffer (used for atomic sends)<BR>
* (blocking function)
* \param[in] uart UART name (GPS, TELEM, AUX)
* \param[in] *buffer pointer to buffer to be sent
* \param[in] len number of bytes to be sent
* \return 0 if no error
* \return -1 if UART not available
* \return -3 if UART not supported by MIOS32_UART_TxBufferPut Routine
* \note Applications shouldn't call these functions directly, instead please use \ref PIOS_COM layer functions
*/
int UARTTxBufferPutMore(UARTNumTypeDef uart, uint8_t *buffer, uint16_t len)
{
	int error;

	while((error = UARTTxBufferPutMoreNonBlocking(uart, buffer, len)) == -2);

	return error;
}

/**
* puts a byte onto the transmit buffer
* \param[in] uart UART name (GPS, TELEM, AUX)
* \param[in] b byte which should be put into Tx buffer
* \return 0 if no error
* \return -1 if UART not available
* \return -2 if buffer full (retry)
* \return -3 if UART not supported by MIOS32_UART_TxBufferPut Routine
* \note Applications shouldn't call these functions directly, instead please use \ref PIOS_COM layer functions
*/
int UARTTxBufferPut_NonBlocking(UARTNumTypeDef uart, uint8_t b)
{
	/* For more comfortable usage... */
	/* -> Just forward to UARTTxBufferPutMore */
	return UARTTxBufferPutMore(uart, &b, 1);
}

/**
* puts a byte onto the transmit buffer<BR>
* (blocking function)
* \param[in] uart UART name (GPS, TELEM, AUX)
* \param[in] b byte which should be put into Tx buffer
* \return 0 if no error
* \return -1 if UART not available
* \return -3 if UART not supported by MIOS32_UART_TxBufferPut Routine
* \note Applications shouldn't call these functions directly, instead please use \ref PIOS_COM layer functions
*/
int UARTTxBufferPut(UARTNumTypeDef uart, uint8_t b)
{
	int error;

	while((error = UARTTxBufferPutMore(uart, &b, 1)) == -2);

	return error;
}

/* Interrupt handler for GPS UART */
GPS_IRQHANDLER_FUNC
{
	/* Check if RXNE flag is set */
	if(GPS_UART->SR & (1 << 5)) {
		uint8_t b = GPS_UART->DR;
		
		if(UARTRxBufferPut(0, b) < 0) {
			/* Here we could add some error handling */
		}
	}

	/* Check if TXE flag is set */
	if(GPS_UART->SR & (1 << 7)) {
		if(UARTTxBufferUsed(0) > 0) {
			int b = UARTTxBufferGet(0);
			if( b < 0 ) {
				/* Here we could add some error handling */
				GPS_UART->DR = 0xff;
			} else {
				GPS_UART->DR = b;
			}
		} else {
			/* Disable TXE interrupt (TXEIE=0) */
			GPS_UART->CR1 &= ~(1 << 7);
		}
	}
}

/* Interrupt handler for TELEM UART */
TELEM_IRQHANDLER_FUNC
{
	/* check if RXNE flag is set */
	if(TELEM_UART->SR & (1 << 5)) {
		uint8_t b = TELEM_UART->DR;
		
		if(UARTRxBufferPut(1, b) < 0) {
			/* Here we could add some error handling */
		}
	}

	if(TELEM_UART->SR & (1 << 7)) { // check if TXE flag is set
		if(UARTTxBufferUsed(1) > 0) {
			int b = UARTTxBufferGet(1);
			if(b < 0) {
				/* Here we could add some error handling */
				TELEM_UART->DR = 0xff;
			} else {
				TELEM_UART->DR = b;
			}
		} else {
			/* Disable TXE interrupt (TXEIE=0) */
			TELEM_UART->CR1 &= ~(1 << 7);
		}
	}
}

/* Interrupt handler for AUX UART */
AUX_UART_IRQHANDLER_FUNC
{
	/* check if RXNE flag is set */
	if(AUX_UART_UART->SR & (1 << 5)) {
		uint8_t b = AUX_UART_UART->DR;
		
		if(UARTRxBufferPut(1, b) < 0) {
			/* Here we could add some error handling */
		}
	}

	if(AUX_UART_UART->SR & (1 << 7)) { // check if TXE flag is set
		if(UARTTxBufferUsed(1) > 0) {
			int b = UARTTxBufferGet(1);
			if(b < 0) {
				/* Here we could add some error handling */
				AUX_UART_UART->DR = 0xff;
			} else {
				AUX_UART_UART->DR = b;
			}
		} else {
			/* Disable TXE interrupt (TXEIE=0) */
			AUX_UART_UART->CR1 &= ~(1 << 7);
		}
	}
}
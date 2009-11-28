
/**
 * Project: OpenPilot
 *    
 * @author The OpenPilot Team, http://www.openpilot.org, Copyright (C) 2009.
 *    
 * @file pios_uart.c
 * UART commands, Inits UARTS, controls & talks to UARTS
 *
 * @see The GNU Public License (GPL)
 */
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

/* Local Variables */
static u8 rx_buffer[UART_NUM][UART_RX_BUFFER_SIZE];
static volatile u8 rx_buffer_tail[UART_NUM];
static volatile u8 rx_buffer_head[UART_NUM];
static volatile u8 rx_buffer_size[UART_NUM];

static u8 tx_buffer[UART_NUM][UART_TX_BUFFER_SIZE];
static volatile u8 tx_buffer_tail[UART_NUM];
static volatile u8 tx_buffer_head[UART_NUM];
static volatile u8 tx_buffer_size[UART_NUM];

/* Initialise the GPS and TELEM onboard UARTs */
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

void EnableAuxUART(void)
{
	//Implement after servo inputs are implemented
}

void DisableAuxUART(void)
{
	//Implement after servo inputs are implemented
}

/* Changes the baud rate of the USART peripherial without re-initialising */
void UARTChangeBaud(USART_TypeDef* USARTx, uint32_t Baud)
{
	/* USART BRR Configuration */
	/* Configure the USART Baud Rate */
	/* Adapted from stm32f01x_usart.c */
	
	uint32_t tmpreg = 0x00, apbclock = 0x00;
	uint32_t integerdivider = 0x00;
	uint32_t fractionaldivider = 0x00;
	uint32_t usartxbase = (uint32_t)USARTx;
	RCC_ClocksTypeDef RCC_ClocksStatus;
	
	RCC_GetClocksFreq(&RCC_ClocksStatus);
	if (usartxbase == USART1_BASE) {
		apbclock = RCC_ClocksStatus.PCLK2_Frequency;
	} else {
		apbclock = RCC_ClocksStatus.PCLK1_Frequency;
	}
	
	/* Determine the integer part */
	integerdivider = ((0x19 * apbclock) / (0x04 * (Baud)));
	tmpreg = (integerdivider / 0x64) << 0x04;
	
	/* Determine the fractional part */
	fractionaldivider = integerdivider - (0x64 * (tmpreg >> 0x04));
	tmpreg |= ((((fractionaldivider * 0x10) + 0x32) / 0x64)) & ((uint8_t)0x0F);
	
	/* Write to USART BRR */
	USARTx->BRR = (uint16_t)tmpreg;
}

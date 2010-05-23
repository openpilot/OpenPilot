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
static uint16_t CaptureValue[12];


/**
* Initialise the onboard USARTs
*/
void PIOS_SPEKTRUM_Init(void)
{
	// need setting flag for bind on next powerup
	if(1)
	{
		PIOS_SPEKTRUM_Bind();
	}
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
	//GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
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
}


/**
* Get the value of an input channel
* \param[in] Channel Number of the channel desired
* \output -1 Channel not available
* \output >0 Channel value
*/
int16_t PIOS_SPEKTRUM_Get(int8_t Channel)
{
	/* Return error if channel not available */
	if(Channel >= 12) {
		return -1;
	}
	return CaptureValue[Channel];
}

/**
* Spektrum bind function
* \output 1 Successful bind
* \output 0 Bind failed
* \note Applications shouldn't call these functions directly
*/
uint8_t PIOS_SPEKTRUM_Bind(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_StructInit(&GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Pin = PIOS_USART3_RX_PIN;
	GPIO_Init(PIOS_USART3_GPIO_PORT, &GPIO_InitStructure);
	/* GPIO's Off */
	/* powerup, RX line stay low for 75ms */
	/* system init takes longer!!! */
	/* I have no idea how long the powerup init window for satellite is but works with this */
	PIOS_USART3_GPIO_PORT->BRR = PIOS_USART3_RX_PIN;
	//PIOS_DELAY_WaitmS(75);
	/* RX line, drive high for 10us */
	PIOS_USART3_GPIO_PORT->BSRR = PIOS_USART3_RX_PIN;
	PIOS_DELAY_WaituS(10);
	/* RX line, drive low for 120us */
	PIOS_USART3_GPIO_PORT->BRR = PIOS_USART3_RX_PIN;
	PIOS_DELAY_WaituS(120);
	/* RX line, drive high for 120us */
	PIOS_USART3_GPIO_PORT->BSRR = PIOS_USART3_RX_PIN;
	PIOS_DELAY_WaituS(120);
	/* RX line, drive low for 120us */
	PIOS_USART3_GPIO_PORT->BRR = PIOS_USART3_RX_PIN;
	PIOS_DELAY_WaituS(120);
	/* RX line, drive high for 120us */
	PIOS_USART3_GPIO_PORT->BSRR = PIOS_USART3_RX_PIN;
	PIOS_DELAY_WaituS(120);
	/* RX line, drive low for 120us */
	PIOS_USART3_GPIO_PORT->BRR = PIOS_USART3_RX_PIN;
	PIOS_DELAY_WaituS(120);
	/* RX line, drive high for 120us */
	PIOS_USART3_GPIO_PORT->BSRR = PIOS_USART3_RX_PIN;
	PIOS_DELAY_WaituS(120);
	/* RX line, drive low for 120us */
	PIOS_USART3_GPIO_PORT->BRR = PIOS_USART3_RX_PIN;
	PIOS_DELAY_WaituS(120);
	/* RX line, drive high for 120us */
	PIOS_USART3_GPIO_PORT->BSRR = PIOS_USART3_RX_PIN;
	PIOS_DELAY_WaituS(120);
	/* RX line, set input and wait for data, PIOS_SPEKTRUM_Init */
	return 1;
}


/**
* Decodes a byte
* \param[in] b byte which should be spektrum decoded
* \return 0 if no error
* \return -1 if USART not available
* \return -2 if buffer full (retry)
* \note Applications shouldn't call these functions directly
*/
int32_t PIOS_SPEKTRUM_Decode(uint8_t b)
{
	static uint8_t prev_byte=0xFF,sync=0,bytecount=0,byte_array[20]={0};
	static uint16_t channel=0,sync_word=0;
	uint8_t channeln=0,frame=0;
	uint16_t data=0;
	byte_array[bytecount]=b;
	bytecount++;
	if(sync==0)
	{
		sync_word=(prev_byte<<8)+b;
		if((sync_word&0xCCFE)==0)
		{
			if(sync_word&0x01)
			{
				sync=1;
				bytecount=2;
			}
		}
	}
	else
	{
		if((bytecount%2)==0)
		{
			channel=(prev_byte<<8)+b;
			frame=channel>>15;
			channeln=(channel>>10)&0x0F;
			data=channel&0x03FF;
			if(channeln < 12)
				CaptureValue[channeln]=data;
		}
	}
	if(bytecount==16)
	{
		//PIOS_COM_SendBufferNonBlocking(COM_DEBUG_USART,byte_array,16);
		bytecount=0;
		sync=0;
	}
	prev_byte=b;
	return 0;
}

/* Interrupt handler for USART3 */
PIOS_USART3_IRQHANDLER_FUNC
{
	/* check if RXNE flag is set */
	if(PIOS_USART3_USART->SR & (1 << 5)) {
		uint8_t b = PIOS_USART3_USART->DR;
		
		if(PIOS_SPEKTRUM_Decode(b) < 0) {
			/* Here we could add some error handling */
		}
	}

	if(PIOS_USART3_USART->SR & (1 << 7)) { // check if TXE flag is set
			/* Disable TXE interrupt (TXEIE=0) */
			PIOS_USART3_USART->CR1 &= ~(1 << 7);
	}
}

#endif

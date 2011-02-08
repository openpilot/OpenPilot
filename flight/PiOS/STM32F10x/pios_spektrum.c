/**
 ******************************************************************************
 * @addtogroup PIOS PIOS Core hardware abstraction layer
 * @{
 * @addtogroup PIOS_SPEKTRUM Spektrum receiver functions
 * @brief Code to read Spektrum input
 * @{
 *
 * @file       pios_spektrum.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * 	        Parts by Thorsten Klose (tk@midibox.org) (tk@midibox.org)
 * @brief      USART commands. Inits USARTs, controls USARTs & Interrupt handlers. (STM32 dependent)
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
#include "pios_spektrum_priv.h"

#if defined(PIOS_INCLUDE_SPEKTRUM)
#if defined(PIOS_INCLUDE_PWM)
#error "Both PWM and SPEKTRUM input defined, choose only one"
#endif
#if defined(PIOS_COM_AUX)
#error "AUX com cannot be used with SPEKTRUM"
#endif

/* Global Variables */

/* Local Variables, use pios_usart */
static uint16_t CaptureValue[12],CaptureValueTemp[12];
static uint8_t prev_byte = 0xFF, sync = 0, bytecount = 0, frame_error=0, byte_array[20] = { 0 };

uint8_t sync_of = 0;

/**
* Initialise the onboard USARTs
*/
void PIOS_SPEKTRUM_Init(void)
{
	// TODO: need setting flag for bind on next powerup
	if (0) {
		PIOS_SPEKTRUM_Bind();
	}

///////////////////////

	NVIC_InitTypeDef NVIC_InitStructure = pios_spektrum_cfg.irq.init;
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure = pios_spektrum_cfg.tim_base_init;


	/* Enable appropriate clock to timer module */
	switch((int32_t) pios_spektrum_cfg.timer) {
		case (int32_t)TIM1:
			NVIC_InitStructure.NVIC_IRQChannel = TIM1_CC_IRQn;
			RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);
			break;
		case (int32_t)TIM2:
			NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
			RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
			break;
		case (int32_t)TIM3:
			NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;
			RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
			break;
		case (int32_t)TIM4:
			NVIC_InitStructure.NVIC_IRQChannel = TIM4_IRQn;
			RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);
			break;
#ifdef STM32F10X_HD

		case (int32_t)TIM5:
			NVIC_InitStructure.NVIC_IRQChannel = TIM5_IRQn;
			RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5, ENABLE);
			break;
		case (int32_t)TIM6:
			NVIC_InitStructure.NVIC_IRQChannel = TIM6_IRQn;
			RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM6, ENABLE);
			break;
		case (int32_t)TIM7:
			NVIC_InitStructure.NVIC_IRQChannel = TIM7_IRQn;
			RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM7, ENABLE);
			break;
		case (int32_t)TIM8:
			NVIC_InitStructure.NVIC_IRQChannel = TIM8_CC_IRQn;
			RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM8, ENABLE);
			break;
#endif
	}
	NVIC_Init(&NVIC_InitStructure);

	/* Configure timer clocks */
	TIM_InternalClockConfig(pios_spektrum_cfg.timer);
	TIM_TimeBaseInit(pios_spektrum_cfg.timer, &TIM_TimeBaseStructure);

	/* Enable the Capture Compare Interrupt Request */
	TIM_ITConfig(pios_spektrum_cfg.timer, pios_spektrum_cfg.ccr, ENABLE);

	/* Clear update pending flag */
	TIM_ClearFlag(pios_spektrum_cfg.timer, TIM_FLAG_Update);

	/* Enable timers */
	TIM_Cmd(pios_spektrum_cfg.timer, ENABLE);

/////////////////////////////

#if 0
	/* spektrum "watchdog" timer */
	NVIC_InitTypeDef NVIC_InitStructure;
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	/* Enable timer clock */
	PIOS_SPEKTRUM_SUPV_TIMER_RCC_FUNC;

	/* Configure interrupts */
	NVIC_InitStructure.NVIC_IRQChannel = PIOS_SPEKTRUM_SUPV_IRQ_CHANNEL;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = PIOS_IRQ_PRIO_MID;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	/* Time base configuration */
	TIM_TimeBaseStructInit(&TIM_TimeBaseStructure);
	TIM_TimeBaseStructure.TIM_Period = ((1000000 / PIOS_SPEKTRUM_SUPV_HZ) - 1);
	TIM_TimeBaseStructure.TIM_Prescaler = (PIOS_MASTER_CLOCK / 1000000) - 1;	/* For 1 uS accuracy */
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(PIOS_SPEKTRUM_SUPV_TIMER, &TIM_TimeBaseStructure);

	/* Enable the Update Interrupt Request */
	TIM_ITConfig(PIOS_SPEKTRUM_SUPV_TIMER, TIM_IT_Update, ENABLE);

	/* Clear update pending flag */
	TIM_ClearFlag(PIOS_SPEKTRUM_SUPV_TIMER, TIM_FLAG_Update);

	/* Enable counter */
	TIM_Cmd(PIOS_SPEKTRUM_SUPV_TIMER, ENABLE);
#endif
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
	if (Channel >= 12) {
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
	GPIO_InitTypeDef GPIO_InitStructure = pios_spektrum_cfg.gpio_init;
	GPIO_InitStructure.GPIO_Pin = pios_spektrum_cfg.pin;
	GPIO_Init(pios_spektrum_cfg.port, &GPIO_InitStructure);

	pios_spektrum_cfg.port->BRR = pios_spektrum_cfg.pin;
	//PIOS_DELAY_WaitmS(75);
	/* RX line, drive high for 10us */
	pios_spektrum_cfg.port->BSRR = pios_spektrum_cfg.pin;
	PIOS_DELAY_WaituS(10);
	/* RX line, drive low for 120us */
	pios_spektrum_cfg.port->BRR = pios_spektrum_cfg.pin;
	PIOS_DELAY_WaituS(120);
	/* RX line, drive high for 120us */
	pios_spektrum_cfg.port->BSRR = pios_spektrum_cfg.pin;
	PIOS_DELAY_WaituS(120);
	/* RX line, drive low for 120us */
	pios_spektrum_cfg.port->BRR = pios_spektrum_cfg.pin;
	PIOS_DELAY_WaituS(120);
	/* RX line, drive high for 120us */
	pios_spektrum_cfg.port->BSRR = pios_spektrum_cfg.pin;
	PIOS_DELAY_WaituS(120);
	/* RX line, drive low for 120us */
	pios_spektrum_cfg.port->BRR = pios_spektrum_cfg.pin;
	PIOS_DELAY_WaituS(120);
	/* RX line, drive high for 120us */
	pios_spektrum_cfg.port->BSRR = pios_spektrum_cfg.pin;
	PIOS_DELAY_WaituS(120);
	/* RX line, drive low for 120us */
	pios_spektrum_cfg.port->BRR = pios_spektrum_cfg.pin;
	PIOS_DELAY_WaituS(120);
	/* RX line, drive high for 120us */
	pios_spektrum_cfg.port->BSRR = pios_spektrum_cfg.pin;
	PIOS_DELAY_WaituS(120);
	/* RX line, set input and wait for data, PIOS_SPEKTRUM_Init */

#if 0

#define PIOS_USART3_GPIO_PORT			GPIOA
#define PIOS_USART3_RX_PIN			GPIO_Pin_10

	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_StructInit(&GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Pin = PIOS_USART3_RX_PIN;
	GPIO_Init(PIOS_USART3_GPIO_PORT, &GPIO_InitStructure);
	/* GPIO's Off */
	/* TODO: powerup, RX line stay low for 75ms */
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
#endif
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
	static uint16_t channel = 0, sync_word = 0;
	uint8_t channeln = 0, frame = 0;
	uint16_t data = 0;
	byte_array[bytecount] = b;
	bytecount++;
	if (sync == 0) {
		sync_word = (prev_byte << 8) + b;
		if (((sync_word & 0x00FE) == 0) && (bytecount == 2)) {
			/* sync low byte always 0x01, high byte seems to be random when switching TX on off on, loss counter??? */
			if (sync_word & 0x01) {
				sync = 1;
				bytecount = 2;
			}
		}
	} else {
		if ((bytecount % 2) == 0) {
			channel = (prev_byte << 8) + b;
			frame = channel >> 15;
			channeln = (channel >> 10) & 0x0F;
			data = channel & 0x03FF;
			if(channeln==0 && data<10) // discard frame if throttle misbehaves
			{
				frame_error=1;
			}
			if (channeln < 12 && !frame_error)
				CaptureValueTemp[channeln] = data;
		}
	}
	if (bytecount == 16) {
		//PIOS_COM_SendBufferNonBlocking(PIOS_COM_TELEM_RF,byte_array,16); //00 2c 58 84 b0 dc ff
		bytecount = 0;
		sync = 0;
		sync_of = 0;
		if (!frame_error)
		{
			for(int i=0;i<12;i++)
			{
				CaptureValue[i] = CaptureValueTemp[i];
			}
		}
		frame_error=0;
	}
	prev_byte = b;
	return 0;
}

/* Interrupt handler for USART */
void SPEKTRUM_IRQHandler(void)
{
	/* by always reading DR after SR make sure to clear any error interrupts */
	volatile uint16_t sr = pios_spektrum_cfg.pios_usart_spektrum_cfg.regs->SR;
	volatile uint8_t b = pios_spektrum_cfg.pios_usart_spektrum_cfg.regs->DR;
	
	/* check if RXNE flag is set */
	if (sr & USART_SR_RXNE) {
		if (PIOS_SPEKTRUM_Decode(b) < 0) {
			/* Here we could add some error handling */
		}
	} 

	if (sr & USART_SR_TXE) {	// check if TXE flag is set
		/* Disable TXE interrupt (TXEIE=0) */
		USART_ITConfig(pios_spektrum_cfg.pios_usart_spektrum_cfg.regs, USART_IT_TXE, DISABLE);
	}
	/* clear "watchdog" timer */
	TIM_SetCounter(pios_spektrum_cfg.timer, 0);
}

/**
* This function handles TIM6 global interrupt request.
*/
void PIOS_SPEKTRUM_irq_handler() {
//PIOS_SPEKTRUM_SUPV_IRQ_FUNC {
	/* Clear timer interrupt pending bit */
	TIM_ClearITPendingBit(pios_spektrum_cfg.timer, TIM_IT_Update);

	/* sync between frames, TODO! DX7SE */
	sync = 0;
	bytecount = 0;
	prev_byte = 0xFF;
	frame_error=0;
	sync_of++;
	/* watchdog activated */
	if (sync_of > 3) {
		/* signal lost */
		sync_of = 0;
		for (int i = 0; i < 12; i++)
		{
			CaptureValue[i] = 0;
			CaptureValueTemp[i] = 0;
		}
	}
}

#endif

/** 
  * @}
  * @}
  */

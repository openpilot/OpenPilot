/**
 ******************************************************************************
 *
 * @file       pios_pwm.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      PWM Input functions
 * @see        The GNU Public License (GPL) Version 3
 * @defgroup   PIOS_PWM PWM Input Functions
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

#if !defined(PIOS_DONT_USE_PWM)

/* Local Variables */
static volatile uint16_t ic3_readvalue1 = 0, ic3_readvalue2 = 0;
static volatile uint16_t capture_number = 0;
static volatile uint32_t CAPTURE = 0;
static volatile uint32_t TIM3_FREQ = 0;

/**
* Initialises all the LED's
*/
void PIOS_PWM_Init(void)
{
	/* Setup RCC */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

	/* Enable the TIM3 global Interrupt */
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	GPIO_PinRemapConfig(GPIO_PartialRemap_TIM3, ENABLE);

	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;

	GPIO_InitStructure.GPIO_Pin = RECEIVER8_PIN;
	GPIO_Init(RECEIVER8_GPIO_PORT, &GPIO_InitStructure);

	TIM_ICInitTypeDef TIM_ICInitStructure;
	TIM_ICInitStructure.TIM_Channel = RECEIVER8_CH;
	TIM_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Rising;
	TIM_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI;
	TIM_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;
	TIM_ICInitStructure.TIM_ICFilter = 0x0;

	TIM_ICInitStructure.TIM_Channel = RECEIVER8_CH;
	TIM_ICInit(RECEIVER8_TIM_PORT, &TIM_ICInitStructure);

	TIM_InternalClockConfig(RECEIVER8_TIM_PORT);
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	TIM_TimeBaseStructInit(&TIM_TimeBaseStructure);
	TIM_TimeBaseStructure.TIM_Period = 0xFFFF;
	TIM_TimeBaseStructure.TIM_Prescaler = (PIOS_MASTER_CLOCK / 1000000) - 1;//17; // fCK_PSC / (17 + 1) 1ms = 4000
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(RECEIVER8_TIM_PORT, &TIM_TimeBaseStructure);

	/* TIM enable counter */
	TIM_Cmd(RECEIVER8_TIM_PORT, ENABLE);

	/* Enable the CC2 Interrupt Request */
	TIM_ITConfig(RECEIVER8_TIM_PORT, TIM_IT_CC2, ENABLE);
}

/**
* This function handles TIM3 global interrupt request.
*/
void TIM3_IRQHandler(void)
{
	if(TIM_GetITStatus(RECEIVER8_TIM_PORT, TIM_IT_CC2) == SET) {
		/* Clear TIM3 Capture compare interrupt pending bit */
		TIM_ClearITPendingBit(RECEIVER8_TIM_PORT, TIM_IT_CC2);

		if(capture_number == 0) {
			/* Get the Input Capture value */
			ic3_readvalue1 = TIM_GetCapture2(RECEIVER8_TIM_PORT);
			capture_number = 1;

			TIM_ICInitTypeDef TIM_ICInitStructure;
			TIM_ICInitStructure.TIM_Channel = RECEIVER8_CH;
			TIM_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Falling;
			TIM_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI;
			TIM_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;
			TIM_ICInitStructure.TIM_ICFilter = 0x0;
			TIM_ICInitStructure.TIM_Channel = RECEIVER8_CH;
			TIM_ICInit(RECEIVER8_TIM_PORT, &TIM_ICInitStructure);

		} else if(capture_number == 1) {
			/* Get the Input Capture value */
			ic3_readvalue2 = TIM_GetCapture2(RECEIVER8_TIM_PORT);

			TIM_ICInitTypeDef TIM_ICInitStructure;
			TIM_ICInitStructure.TIM_Channel = RECEIVER8_CH;
			TIM_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Rising;
			TIM_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI;
			TIM_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;
			TIM_ICInitStructure.TIM_ICFilter = 0x0;
			TIM_ICInitStructure.TIM_Channel = RECEIVER8_CH;
			TIM_ICInit(RECEIVER8_TIM_PORT, &TIM_ICInitStructure);

			/* Capture computation */
			if (ic3_readvalue2 > ic3_readvalue1) {
				CAPTURE = (ic3_readvalue2 - ic3_readvalue1);
			} else {
				CAPTURE = ((0xFFFF - ic3_readvalue1) + ic3_readvalue2);
			}

			capture_number = 0;

		}
	}
}

uint32_t PIOS_PWM_Get(void)
{
	return CAPTURE;
}

#endif

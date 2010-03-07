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
static uint8_t CaptureState[PIOS_PWM_NUM_INPUTS];
static uint16_t RiseValue[PIOS_PWM_NUM_INPUTS];
static uint16_t FallValue[PIOS_PWM_NUM_INPUTS];
static uint32_t CaptureValue[PIOS_PWM_NUM_INPUTS];

static uint8_t SupervisorState = 0;
static uint32_t CapCounter[PIOS_PWM_NUM_INPUTS];
static uint32_t CapCounterPrev[PIOS_PWM_NUM_INPUTS];


/**
* Initialises all the LED's
*/
void PIOS_PWM_Init(void)
{
	/* Flush counter variables */
	int32_t i;
	for(i = 0; i < PIOS_PWM_NUM_INPUTS; i++) {
		CaptureState[i] = 0;
	}
	for(i = 0; i < PIOS_PWM_NUM_INPUTS; i++) {
		RiseValue[i] = 0;
	}
	for(i = 0; i < PIOS_PWM_NUM_INPUTS; i++) {
		FallValue[i] = 0;
	}
	for(i = 0; i < PIOS_PWM_NUM_INPUTS; i++) {
		CaptureValue[i] = 0;
	}

	/* Setup RCC */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

	/* Enable timer interrupts */
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	/* Partial pin remap for PB5 */
	GPIO_PinRemapConfig(GPIO_PartialRemap_TIM3, ENABLE);

	/* Configure input pins */
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;

	GPIO_InitStructure.GPIO_Pin = RECEIVER8_PIN;
	GPIO_Init(RECEIVER8_GPIO_PORT, &GPIO_InitStructure);

	/* Configure timer for input capture */
	TIM_ICInitTypeDef TIM_ICInitStructure;
	TIM_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Rising;
	TIM_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI;
	TIM_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;
	TIM_ICInitStructure.TIM_ICFilter = 0x0;

	TIM_ICInitStructure.TIM_Channel = RECEIVER8_CH;
	TIM_ICInit(RECEIVER8_TIM_PORT, &TIM_ICInitStructure);

	/* Configure timer clocks */
	TIM_InternalClockConfig(RECEIVER8_TIM_PORT);
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	TIM_TimeBaseStructInit(&TIM_TimeBaseStructure);
	TIM_TimeBaseStructure.TIM_Period = 0xFFFF;
	TIM_TimeBaseStructure.TIM_Prescaler = (PIOS_MASTER_CLOCK / 1000000) - 1;
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(RECEIVER8_TIM_PORT, &TIM_TimeBaseStructure);

	/* Enable the CC2 Interrupt Request */
	TIM_ITConfig(RECEIVER8_TIM_PORT, TIM_IT_CC2, ENABLE);

	/* Enable timers */
	TIM_Cmd(RECEIVER8_TIM_PORT, ENABLE);


	/* Supervisor Setup */
#if (PIOS_PWM_SUPV_ENABLED)
	/* Flush counter variables */
	for(i = 0; i < PIOS_PWM_NUM_INPUTS; i++) {
		CapCounter[i] = 0;
	}
	for(i = 0; i < PIOS_PWM_NUM_INPUTS; i++) {
		CapCounterPrev[i] = 0;
	}

	/* Enable timer clock */
	PIOS_PWM_SUPV_TIMER_RCC_FUNC;

	/* Configure interrupts */
	NVIC_InitStructure.NVIC_IRQChannel = PIOS_PWM_SUPV_IRQ_CHANNEL;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	/* Time base configuration */
	TIM_TimeBaseStructInit(&TIM_TimeBaseStructure);
	TIM_TimeBaseStructure.TIM_Period = ((1000000 / PIOS_PWM_SUPV_HZ) - 1);
	TIM_TimeBaseStructure.TIM_Prescaler = (PIOS_MASTER_CLOCK / 1000000) - 1; /* For 1 uS accuracy */
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(PIOS_PWM_SUPV_TIMER, &TIM_TimeBaseStructure);

	/* Enable the CC2 Interrupt Request */
	TIM_ITConfig(PIOS_PWM_SUPV_TIMER, TIM_IT_Update, ENABLE);

	/* Clear update pending flag */
	TIM_ClearFlag(TIM2, TIM_FLAG_Update);

	/* Enable counter */
	TIM_Cmd(PIOS_PWM_SUPV_TIMER, ENABLE);
#endif
}

/**
* Get the value of an input channel
* \param[in] Channel Number of the channel desired
* \output -1 Channel not available
* \output >0 Channel value
*/
int32_t PIOS_PWM_Get(int8_t Channel)
{
	/* Return error if channel not available */
	if(Channel >= PIOS_PWM_NUM_INPUTS) {
		return -1;
	}
	return CaptureValue[Channel];
}

/**
* This function handles TIM3 global interrupt request.
*/
void TIM3_IRQHandler(void)
{
	if(TIM_GetITStatus(RECEIVER8_TIM_PORT, TIM_IT_CC2) == SET) {
		/* Clear TIM3 Capture compare interrupt pending bit */
		TIM_ClearITPendingBit(RECEIVER8_TIM_PORT, TIM_IT_CC2);

		/* Simple rise or fall state machine */
		if(CaptureState[0] == 0) {
			/* Get the Input Capture value */
			RiseValue[0] = TIM_GetCapture2(RECEIVER8_TIM_PORT);

			/* Switch states */
			CaptureState[0] = 1;

			/* Switch polarity of input capture */
			TIM_ICInitTypeDef TIM_ICInitStructure;
			TIM_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI;
			TIM_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;
			TIM_ICInitStructure.TIM_ICFilter = 0x0;

			TIM_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Falling;
			TIM_ICInitStructure.TIM_Channel = RECEIVER8_CH;
			TIM_ICInit(RECEIVER8_TIM_PORT, &TIM_ICInitStructure);

		} else {
			/* Get the Input Capture value */
			FallValue[0] = TIM_GetCapture2(RECEIVER8_TIM_PORT);

			/* Capture computation */
			if (FallValue[0] > RiseValue[0]) {
				CaptureValue[0] = (FallValue[0] - RiseValue[0]);
			} else {
				CaptureValue[0] = ((0xFFFF - RiseValue[0]) + FallValue[0]);
			}

			/* Switch states */
			CaptureState[0] = 0;

			/* Increase supervisor counter */
			CapCounter[0]++;

			/* Switch polarity of input capture */
			TIM_ICInitTypeDef TIM_ICInitStructure;
			TIM_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI;
			TIM_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;
			TIM_ICInitStructure.TIM_ICFilter = 0x0;

			TIM_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Rising;
			TIM_ICInitStructure.TIM_Channel = RECEIVER8_CH;
			TIM_ICInit(RECEIVER8_TIM_PORT, &TIM_ICInitStructure);
		}
	}
}

/**
* This function handles TIM3 global interrupt request.
*/
PIOS_PWM_SUPV_IRQ_FUNC
{
	/* Clear timer interrupt pending bit */
	TIM_ClearITPendingBit(PIOS_PWM_SUPV_TIMER, TIM_IT_Update);

	/* Simple state machine */
	if(SupervisorState == 0) {
		/* Save this states values */
		for(int32_t i = 0; i < PIOS_PWM_NUM_INPUTS; i++) {
			CapCounterPrev[i] = CapCounter[i];
		}

		/* Move to next state */
		SupervisorState = 1;
	} else {
		/* See what channels have been updated */
		for(int32_t i = 0; i < PIOS_PWM_NUM_INPUTS; i++) {
			if(CapCounter[i] == CapCounterPrev[i]) {
				CaptureValue[i] = 0;
			}
		}

		/* Move to next state */
		SupervisorState = 0;
	}
}

#endif

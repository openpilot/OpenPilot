/**
 ******************************************************************************
 *
 * @file       pios_ppm.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      PPM Input functions
 * @see        The GNU Public License (GPL) Version 3
 * @defgroup   PIOS_PPM PPM Input Functions
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

#if defined(PIOS_INCLUDE_PPM)

/* Local Variables */

static TIM_ICInitTypeDef TIM_ICInitStructure;
static uint8_t PulseIndex;
static uint32_t PreviousValue;
static uint32_t CurrentValue;
static uint32_t CapturedValue;
static uint32_t CaptureValue[PIOS_PPM_NUM_INPUTS];

static uint8_t SupervisorState = 0;
static uint32_t CapCounter[PIOS_PPM_NUM_INPUTS];
static uint32_t CapCounterPrev[PIOS_PPM_NUM_INPUTS];


/**
* Initialises all the LED's
*/
void PIOS_PPM_Init(void)
{
	/* Flush counter variables */
	int32_t i;

	PulseIndex = 0;
	PreviousValue = 0;
	CurrentValue = 0;
	CapturedValue = 0;

	for(i = 0; i < PIOS_PPM_NUM_INPUTS; i++) {
		CaptureValue[i] = 0;
	}

	/* Setup RCC */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);

	/* Enable timer interrupts */
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_InitStructure.NVIC_IRQChannel = PIOS_PPM_TIM_IRQ;
	NVIC_Init(&NVIC_InitStructure);

	/* Configure input pins */
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_InitStructure.GPIO_Pin = PIOS_PPM_GPIO_PIN;
	GPIO_Init(PIOS_PPM_GPIO_PORT, &GPIO_InitStructure);

	/* Configure timer for input capture */
	TIM_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Rising;
	TIM_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI;
	TIM_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;
	TIM_ICInitStructure.TIM_ICFilter = 0x0;
	TIM_ICInitStructure.TIM_Channel = PIOS_PPM_TIM_CHANNEL;
	TIM_ICInit(PIOS_PPM_TIM_PORT, &TIM_ICInitStructure);

	/* Configure timer clocks */
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	TIM_TimeBaseStructInit(&TIM_TimeBaseStructure);
	TIM_TimeBaseStructure.TIM_Period = 0xFFFF;
	TIM_TimeBaseStructure.TIM_Prescaler = (PIOS_MASTER_CLOCK / 1000000) - 1;
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_InternalClockConfig(PIOS_PPM_TIM_PORT);
	TIM_TimeBaseInit(PIOS_PPM_TIM_PORT, &TIM_TimeBaseStructure);

		/* Enable the Capture Compare Interrupt Request */
	TIM_ITConfig(PIOS_PPM_TIM_PORT, PIOS_PPM_TIM_CCR, ENABLE);

	/* Enable timers */
	TIM_Cmd(PIOS_PPM_TIM, ENABLE);

	/* Supervisor Setup */
#if (PIOS_PPM_SUPV_ENABLED)
	/* Flush counter variables */
	for(i = 0; i < PIOS_PPM_NUM_INPUTS; i++) {
		CapCounter[i] = 0;
	}
	for(i = 0; i < PIOS_PPM_NUM_INPUTS; i++) {
		CapCounterPrev[i] = 0;
	}

	/* Enable timer clock */
	PIOS_PPM_SUPV_TIMER_RCC_FUNC;

	/* Configure interrupts */
	NVIC_InitStructure.NVIC_IRQChannel = PIOS_PPM_SUPV_IRQ_CHANNEL;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	/* Time base configuration */
	TIM_TimeBaseStructInit(&TIM_TimeBaseStructure);
	TIM_TimeBaseStructure.TIM_Period = ((1000000 / PIOS_PPM_SUPV_HZ) - 1);
	TIM_TimeBaseStructure.TIM_Prescaler = (PIOS_MASTER_CLOCK / 1000000) - 1; /* For 1 uS accuracy */
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(PIOS_PPM_SUPV_TIMER, &TIM_TimeBaseStructure);

	/* Enable the CC2 Interrupt Request */
	TIM_ITConfig(PIOS_PPM_SUPV_TIMER, TIM_IT_Update, ENABLE);

	/* Clear update pending flag */
	TIM_ClearFlag(TIM2, TIM_FLAG_Update);

	/* Enable counter */
	TIM_Cmd(PIOS_PPM_SUPV_TIMER, ENABLE);
#endif

	/* Setup local variable which stays in this scope */
	/* Doing this here and using a local variable saves doing it in the ISR */
	TIM_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI;
	TIM_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;
	TIM_ICInitStructure.TIM_ICFilter = 0x0;
}

/**
* Get the value of an input channel
* \param[in] Channel Number of the channel desired
* \output -1 Channel not available
* \output >0 Channel value
*/
int32_t PIOS_PPM_Get(int8_t Channel)
{
	/* Return error if channel not available */
	if(Channel >= PIOS_PPM_NUM_INPUTS) {
		return -1;
	}
	return CaptureValue[Channel];
}

/**
* Handle TIM1 global interrupt request
* Some work and testing still needed, need to detect start of frame and decode pulses
*
*/
void TIM1_CC_IRQHandler(void)
{
	/* Do this as it's more efficient */
	if(TIM_GetITStatus(PIOS_PPM_TIM_PORT, PIOS_PPM_TIM_CCR) == SET) {
		PreviousValue = CurrentValue;
		CurrentValue = TIM_GetCapture2(PIOS_PPM_TIM_PORT);
	}

	/* Clear TIM3 Capture compare interrupt pending bit */
	TIM_ClearITPendingBit(PIOS_PPM_TIM_PORT, PIOS_PPM_TIM_CCR);

	/* Capture computation */
	if (CurrentValue > PreviousValue) {
		CapturedValue = (CurrentValue - PreviousValue);
	} else {
		CapturedValue = ((0xFFFF - PreviousValue) + CurrentValue);
	}

	/* sync pulse */
	if(CapturedValue > 8000) {
		PulseIndex = 0;
	/* trying to detect bad pulses, not sure this is working correctly yet. I need a scope :P */
	} else if(CapturedValue > 750 && CapturedValue < 2500) {
		if(PulseIndex < PIOS_PPM_NUM_INPUTS) {
			CaptureValue[PulseIndex] = CapturedValue;
			CapCounter[PulseIndex]++;
			PulseIndex++;
		}
	}
}

/**
* This function handles TIM3 global interrupt request.
*/
PIOS_PPM_SUPV_IRQ_FUNC
{
	/* Clear timer interrupt pending bit */
	TIM_ClearITPendingBit(PIOS_PPM_SUPV_TIMER, TIM_IT_Update);

	/* Simple state machine */
	if(SupervisorState == 0) {
		/* Save this states values */
		for(int32_t i = 0; i < PIOS_PPM_NUM_INPUTS; i++) {
			CapCounterPrev[i] = CapCounter[i];
		}

		/* Move to next state */
		SupervisorState = 1;
	} else {
		/* See what channels have been updated */
		for(int32_t i = 0; i < PIOS_PPM_NUM_INPUTS; i++) {
			if(CapCounter[i] == CapCounterPrev[i]) {
				CaptureValue[i] = 0;
			}
		}

		/* Move to next state */
		SupervisorState = 0;
	}
}

#endif

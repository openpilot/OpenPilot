/**
 ******************************************************************************
 * @addtogroup PIOS PIOS Core hardware abstraction layer
 * @{
 * @addtogroup   PIOS_PWM PWM Input Functions
 * @brief		Code to measure with PWM input
 * @{
 *
 * @file       pios_pwm.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      PWM Input functions (STM32 dependent)
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

#if defined(PIOS_INCLUDE_PWM)

/* Local Variables */
static GPIO_TypeDef *PIOS_PWM_GPIO_PORT[PIOS_PWM_NUM_INPUTS] = PIOS_PWM_GPIO_PORTS;
static const uint32_t PIOS_PWM_GPIO_PIN[PIOS_PWM_NUM_INPUTS] = PIOS_PWM_GPIO_PINS;
static TIM_TypeDef *PIOS_PWM_TIM_PORT[PIOS_PWM_NUM_INPUTS] = PIOS_PWM_TIM_PORTS;
static const uint32_t PIOS_PWM_TIM_CHANNEL[PIOS_PWM_NUM_INPUTS] = PIOS_PWM_TIM_CHANNELS;
static const uint32_t PIOS_PWM_TIM_CCR[PIOS_PWM_NUM_INPUTS] = PIOS_PWM_TIM_CCRS;
static TIM_TypeDef *PIOS_PWM_TIM[PIOS_PWM_NUM_TIMS] = PIOS_PWM_TIMS;
static const uint32_t PIOS_PWM_TIM_IRQ[PIOS_PWM_NUM_TIMS] = PIOS_PWM_TIM_IRQS;

static TIM_ICInitTypeDef TIM_ICInitStructure;
static uint8_t CaptureState[PIOS_PWM_NUM_INPUTS];
static uint16_t RiseValue[PIOS_PWM_NUM_INPUTS];
static uint16_t FallValue[PIOS_PWM_NUM_INPUTS];
static uint32_t CaptureValue[PIOS_PWM_NUM_INPUTS];

static uint8_t SupervisorState = 0;
static uint32_t CapCounter[PIOS_PWM_NUM_INPUTS];
static uint32_t CapCounterPrev[PIOS_PWM_NUM_INPUTS];

/**
* Initialises all the pins
*/
void PIOS_PWM_Init(void)
{
	/* Flush counter variables */
	int32_t i;
	for (i = 0; i < PIOS_PWM_NUM_INPUTS; i++) {
		CaptureState[i] = 0;
	}
	for (i = 0; i < PIOS_PWM_NUM_INPUTS; i++) {
		RiseValue[i] = 0;
	}
	for (i = 0; i < PIOS_PWM_NUM_INPUTS; i++) {
		FallValue[i] = 0;
	}
	for (i = 0; i < PIOS_PWM_NUM_INPUTS; i++) {
		CaptureValue[i] = 0;
	}

	/* Setup RCC */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5, ENABLE);

	/* Enable timer interrupts */
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = PIOS_IRQ_PRIO_MID;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	for (i = 0; i < PIOS_PWM_NUM_TIMS; i++) {
		NVIC_InitStructure.NVIC_IRQChannel = PIOS_PWM_TIM_IRQ[i];
		NVIC_Init(&NVIC_InitStructure);
	}

	/* Partial pin remap for TIM3 (PB5) */
	GPIO_PinRemapConfig(GPIO_PartialRemap_TIM3, ENABLE);

	/* Configure input pins */
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	for (i = 0; i < PIOS_PWM_NUM_INPUTS; i++) {
		GPIO_InitStructure.GPIO_Pin = PIOS_PWM_GPIO_PIN[i];
		GPIO_Init(PIOS_PWM_GPIO_PORT[i], &GPIO_InitStructure);
	}

	/* Configure timer for input capture */
	TIM_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Rising;
	TIM_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI;
	TIM_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;
	TIM_ICInitStructure.TIM_ICFilter = 0x0;
	for (i = 0; i < PIOS_PWM_NUM_INPUTS; i++) {
		TIM_ICInitStructure.TIM_Channel = PIOS_PWM_TIM_CHANNEL[i];
		TIM_ICInit(PIOS_PWM_TIM_PORT[i], &TIM_ICInitStructure);
	}

	/* Configure timer clocks */
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	TIM_TimeBaseStructInit(&TIM_TimeBaseStructure);
	TIM_TimeBaseStructure.TIM_Period = 0xFFFF;
	TIM_TimeBaseStructure.TIM_Prescaler = (PIOS_MASTER_CLOCK / 1000000) - 1;
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	for (i = 0; i < PIOS_PWM_NUM_INPUTS; i++) {
		TIM_InternalClockConfig(PIOS_PWM_TIM_PORT[i]);
		TIM_TimeBaseInit(PIOS_PWM_TIM_PORT[i], &TIM_TimeBaseStructure);

		/* Enable the Capture Compare Interrupt Request */
		TIM_ITConfig(PIOS_PWM_TIM_PORT[i], PIOS_PWM_TIM_CCR[i], ENABLE);
	}

	/* Enable timers */
	for (i = 0; i < PIOS_PWM_NUM_TIMS; i++) {
		TIM_Cmd(PIOS_PWM_TIM[i], ENABLE);
	}

	/* Supervisor Setup */
#if (PIOS_PWM_SUPV_ENABLED)
	/* Flush counter variables */
	for (i = 0; i < PIOS_PWM_NUM_INPUTS; i++) {
		CapCounter[i] = 0;
	}
	for (i = 0; i < PIOS_PWM_NUM_INPUTS; i++) {
		CapCounterPrev[i] = 0;
	}

	/* Enable timer clock */
	PIOS_PWM_SUPV_TIMER_RCC_FUNC;

	/* Configure interrupts */
	NVIC_InitStructure.NVIC_IRQChannel = PIOS_PWM_SUPV_IRQ_CHANNEL;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = PIOS_IRQ_PRIO_MID;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	/* Time base configuration */
	TIM_TimeBaseStructInit(&TIM_TimeBaseStructure);
	TIM_TimeBaseStructure.TIM_Period = ((1000000 / PIOS_PWM_SUPV_HZ) - 1);
	TIM_TimeBaseStructure.TIM_Prescaler = (PIOS_MASTER_CLOCK / 1000000) - 1;	/* For 1 uS accuracy */
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
int32_t PIOS_PWM_Get(int8_t Channel)
{
	/* Return error if channel not available */
	if (Channel >= PIOS_PWM_NUM_INPUTS) {
		return -1;
	}
	return CaptureValue[Channel];
}

/**
* Handle TIM3 global interrupt request
*/
void TIM3_IRQHandler(void)
{
	/* Zero value always will be changed but this prevents compiler warning */
	int32_t i = 0;

	/* Do this as it's more efficient */
	if (TIM_GetITStatus(PIOS_PWM_TIM_PORT[4], PIOS_PWM_TIM_CCR[4]) == SET) {
		i = 4;
		if (CaptureState[i] == 0) {
			RiseValue[i] = TIM_GetCapture4(PIOS_PWM_TIM_PORT[i]);
		} else {
			FallValue[i] = TIM_GetCapture4(PIOS_PWM_TIM_PORT[i]);
		}
	} else if (TIM_GetITStatus(PIOS_PWM_TIM_PORT[5], PIOS_PWM_TIM_CCR[5]) == SET) {
		i = 5;
		if (CaptureState[i] == 0) {
			RiseValue[i] = TIM_GetCapture3(PIOS_PWM_TIM_PORT[i]);
		} else {
			FallValue[i] = TIM_GetCapture3(PIOS_PWM_TIM_PORT[i]);
		}
	} else if (TIM_GetITStatus(PIOS_PWM_TIM_PORT[6], PIOS_PWM_TIM_CCR[6]) == SET) {
		i = 6;
		if (CaptureState[i] == 0) {
			RiseValue[i] = TIM_GetCapture1(PIOS_PWM_TIM_PORT[i]);
		} else {
			FallValue[i] = TIM_GetCapture1(PIOS_PWM_TIM_PORT[i]);
		}
	} else if (TIM_GetITStatus(PIOS_PWM_TIM_PORT[7], PIOS_PWM_TIM_CCR[7]) == SET) {
		i = 7;
		if (CaptureState[i] == 0) {
			RiseValue[i] = TIM_GetCapture2(PIOS_PWM_TIM_PORT[i]);
		} else {
			FallValue[i] = TIM_GetCapture2(PIOS_PWM_TIM_PORT[i]);
		}
	}

	/* Clear TIM3 Capture compare interrupt pending bit */
	TIM_ClearITPendingBit(PIOS_PWM_TIM_PORT[i], PIOS_PWM_TIM_CCR[i]);

	/* Simple rise or fall state machine */
	if (CaptureState[i] == 0) {
		/* Switch states */
		CaptureState[i] = 1;

		/* Switch polarity of input capture */
		TIM_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Falling;
		TIM_ICInitStructure.TIM_Channel = PIOS_PWM_TIM_CHANNEL[i];
		TIM_ICInit(PIOS_PWM_TIM_PORT[i], &TIM_ICInitStructure);

	} else {
		/* Capture computation */
		if (FallValue[i] > RiseValue[i]) {
			CaptureValue[i] = (FallValue[i] - RiseValue[i]);
		} else {
			CaptureValue[i] = ((0xFFFF - RiseValue[i]) + FallValue[i]);
		}

		/* Switch states */
		CaptureState[i] = 0;

		/* Increase supervisor counter */
		CapCounter[i]++;

		/* Switch polarity of input capture */
		TIM_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Rising;
		TIM_ICInitStructure.TIM_Channel = PIOS_PWM_TIM_CHANNEL[i];
		TIM_ICInit(PIOS_PWM_TIM_PORT[i], &TIM_ICInitStructure);
	}
}

/**
* Handle TIM1 global interrupt request
*/
void TIM1_CC_IRQHandler(void)
{
	/* Zero value always will be changed but this prevents compiler warning */
	int32_t i = 0;

	/* Do this as it's more efficient */
	if (TIM_GetITStatus(PIOS_PWM_TIM_PORT[0], PIOS_PWM_TIM_CCR[0]) == SET) {
		i = 0;
		if (CaptureState[i] == 0) {
			RiseValue[i] = TIM_GetCapture2(PIOS_PWM_TIM_PORT[i]);
		} else {
			FallValue[i] = TIM_GetCapture2(PIOS_PWM_TIM_PORT[i]);
		}
	} else if (TIM_GetITStatus(PIOS_PWM_TIM_PORT[1], PIOS_PWM_TIM_CCR[1]) == SET) {
		i = 1;
		if (CaptureState[i] == 0) {
			RiseValue[i] = TIM_GetCapture3(PIOS_PWM_TIM_PORT[i]);
		} else {
			FallValue[i] = TIM_GetCapture3(PIOS_PWM_TIM_PORT[i]);
		}
	} else if (TIM_GetITStatus(PIOS_PWM_TIM_PORT[3], PIOS_PWM_TIM_CCR[3]) == SET) {
		i = 3;
		if (CaptureState[i] == 0) {
			RiseValue[i] = TIM_GetCapture1(PIOS_PWM_TIM_PORT[i]);
		} else {
			FallValue[i] = TIM_GetCapture1(PIOS_PWM_TIM_PORT[i]);
		}
	}

	/* Clear TIM3 Capture compare interrupt pending bit */
	TIM_ClearITPendingBit(PIOS_PWM_TIM_PORT[i], PIOS_PWM_TIM_CCR[i]);

	/* Simple rise or fall state machine */
	if (CaptureState[i] == 0) {
		/* Switch states */
		CaptureState[i] = 1;

		/* Switch polarity of input capture */
		TIM_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Falling;
		TIM_ICInitStructure.TIM_Channel = PIOS_PWM_TIM_CHANNEL[i];
		TIM_ICInit(PIOS_PWM_TIM_PORT[i], &TIM_ICInitStructure);

	} else {
		/* Capture computation */
		if (FallValue[i] > RiseValue[i]) {
			CaptureValue[i] = (FallValue[i] - RiseValue[i]);
		} else {
			CaptureValue[i] = ((0xFFFF - RiseValue[i]) + FallValue[i]);
		}

		/* Switch states */
		CaptureState[i] = 0;

		/* Increase supervisor counter */
		CapCounter[i]++;

		/* Switch polarity of input capture */
		TIM_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Rising;
		TIM_ICInitStructure.TIM_Channel = PIOS_PWM_TIM_CHANNEL[i];
		TIM_ICInit(PIOS_PWM_TIM_PORT[i], &TIM_ICInitStructure);
	}
}

/**
* Handle TIM5 global interrupt request
*/
void TIM5_IRQHandler(void)
{
	/* Do this as it's more efficient */
	if (TIM_GetITStatus(PIOS_PWM_TIM_PORT[2], PIOS_PWM_TIM_CCR[2]) == SET) {
		if (CaptureState[2] == 0) {
			RiseValue[2] = TIM_GetCapture1(PIOS_PWM_TIM_PORT[2]);
		} else {
			FallValue[2] = TIM_GetCapture1(PIOS_PWM_TIM_PORT[2]);
		}

		/* Clear TIM3 Capture compare interrupt pending bit */
		TIM_ClearITPendingBit(PIOS_PWM_TIM_PORT[2], PIOS_PWM_TIM_CCR[2]);

		/* Simple rise or fall state machine */
		if (CaptureState[2] == 0) {
			/* Switch states */
			CaptureState[2] = 1;

			/* Switch polarity of input capture */
			TIM_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Falling;
			TIM_ICInitStructure.TIM_Channel = PIOS_PWM_TIM_CHANNEL[2];
			TIM_ICInit(PIOS_PWM_TIM_PORT[2], &TIM_ICInitStructure);

		} else {
			/* Capture computation */
			if (FallValue[2] > RiseValue[2]) {
				CaptureValue[2] = (FallValue[2] - RiseValue[2]);
			} else {
				CaptureValue[2] = ((0xFFFF - RiseValue[2]) + FallValue[2]);
			}

			/* Switch states */
			CaptureState[2] = 0;

			/* Increase supervisor counter */
			CapCounter[2]++;

			/* Switch polarity of input capture */
			TIM_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Rising;
			TIM_ICInitStructure.TIM_Channel = PIOS_PWM_TIM_CHANNEL[2];
			TIM_ICInit(PIOS_PWM_TIM_PORT[2], &TIM_ICInitStructure);
		}
	}
}

/**
* This function handles TIM3 global interrupt request.
*/
PIOS_PWM_SUPV_IRQ_FUNC {
	/* Clear timer interrupt pending bit */
	TIM_ClearITPendingBit(PIOS_PWM_SUPV_TIMER, TIM_IT_Update);

	/* Simple state machine */
	if (SupervisorState == 0) {
		/* Save this states values */
		for (int32_t i = 0; i < PIOS_PWM_NUM_INPUTS; i++) {
			CapCounterPrev[i] = CapCounter[i];
		}

		/* Move to next state */
		SupervisorState = 1;
	} else {
		/* See what channels have been updated */
		for (int32_t i = 0; i < PIOS_PWM_NUM_INPUTS; i++) {
			if (CapCounter[i] == CapCounterPrev[i]) {
				CaptureValue[i] = 0;
			}
		}

		/* Move to next state */
		SupervisorState = 0;
	}
}

#endif

/** 
  * @}
  * @}
  */

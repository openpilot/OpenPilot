/**
 ******************************************************************************
 * @addtogroup PIOS PIOS Core hardware abstraction layer
 * @{
 * @addtogroup   PIOS_PPM PPM Input Functions
 * @brief Code to measure PPM input and seperate into channels
 * @{
 *
 * @file       pios_ppm.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      PPM Input functions (STM32 dependent)
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
#include "pios_ppm_priv.h"

#if defined(PIOS_INCLUDE_PPM)

/* Provide a RCVR driver */
static int32_t PIOS_PPM_Get(uint32_t chan_id);

const struct pios_rcvr_driver pios_ppm_rcvr_driver = {
	.read = PIOS_PPM_Get,
};

/* Local Variables */
static TIM_ICInitTypeDef TIM_ICInitStructure;
static uint8_t PulseIndex;
static uint32_t PreviousValue;
static uint32_t CurrentValue;
static uint32_t CapturedValue;
static uint32_t CaptureValue[PIOS_PPM_NUM_INPUTS];
static uint32_t CapCounter[PIOS_PPM_NUM_INPUTS];
static uint16_t TimerCounter;

static uint8_t supv_timer = 0;
static uint8_t SupervisorState = 0;
static uint32_t CapCounterPrev[PIOS_PPM_NUM_INPUTS];

static void PIOS_PPM_Supervisor(uint32_t ppm_id);

void PIOS_PPM_Init(void)
{
	/* Flush counter variables */
	int32_t i;

	PulseIndex = 0;
	PreviousValue = 0;
	CurrentValue = 0;
	CapturedValue = 0;
	TimerCounter = 0;

	for (i = 0; i < PIOS_PPM_NUM_INPUTS; i++) {
		CaptureValue[i] = 0;
	}

	NVIC_InitTypeDef NVIC_InitStructure = pios_ppm_cfg.irq.init;

	/* Enable appropriate clock to timer module */
	switch((int32_t) pios_ppm_cfg.timer) {
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
	/* Enable timer interrupts */
	NVIC_Init(&NVIC_InitStructure);

	/* Configure input pins */
	GPIO_InitTypeDef GPIO_InitStructure = pios_ppm_cfg.gpio_init;
	GPIO_Init(pios_ppm_cfg.port, &GPIO_InitStructure);

	/* Configure timer for input capture */
	TIM_ICInitStructure = pios_ppm_cfg.tim_ic_init;
	TIM_ICInit(pios_ppm_cfg.timer, &TIM_ICInitStructure);

	/* Configure timer clocks */
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure = pios_ppm_cfg.tim_base_init;
	TIM_InternalClockConfig(pios_ppm_cfg.timer);
	TIM_TimeBaseInit(pios_ppm_cfg.timer, &TIM_TimeBaseStructure);

	/* Enable the Capture Compare Interrupt Request */
	TIM_ITConfig(pios_ppm_cfg.timer, pios_ppm_cfg.ccr | TIM_IT_Update, ENABLE);

	/* Enable timers */
	TIM_Cmd(pios_ppm_cfg.timer, ENABLE);

#ifdef MOVECOPTER
	if(pios_ppm_cfg.remap) {
		/* Warning, I don't think this will work for multiple remaps at once */
		GPIO_PinRemapConfig(pios_ppm_cfg.remap, ENABLE);
	}
#endif

	/* Supervisor Setup */
	/* Flush counter variables */
	for (i = 0; i < PIOS_PPM_NUM_INPUTS; i++) {
		CapCounter[i] = 0;
	}
	for (i = 0; i < PIOS_PPM_NUM_INPUTS; i++) {
		CapCounterPrev[i] = 0;
	}

	/* Setup local variable which stays in this scope */
	/* Doing this here and using a local variable saves doing it in the ISR */
	TIM_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI;
	TIM_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;
	TIM_ICInitStructure.TIM_ICFilter = 0x0;

	if (!PIOS_RTC_RegisterTickCallback(PIOS_PPM_Supervisor, 0)) {
		PIOS_DEBUG_Assert(0);
	}
}

/**
* Get the value of an input channel
* \param[in] Channel Number of the channel desired
* \output -1 Channel not available
* \output >0 Channel value
*/
static int32_t PIOS_PPM_Get(uint32_t chan_id)
{
	/* Return error if channel not available */
	if (chan_id >= PIOS_PPM_NUM_INPUTS) {
		return -1;
	}
	return CaptureValue[chan_id];
}

/**
* Handle TIMx global interrupt request
* Some work and testing still needed, need to detect start of frame and decode pulses
*
*/
void PIOS_PPM_irq_handler(void)
{
	if (TIM_GetITStatus(pios_ppm_cfg.timer, TIM_IT_Update) == SET) {
		TimerCounter+=pios_ppm_cfg.timer->ARR;
		TIM_ClearITPendingBit(pios_ppm_cfg.timer, TIM_IT_Update);
		if (TIM_GetITStatus(pios_ppm_cfg.timer, pios_ppm_cfg.ccr) != SET) {
			return;
		}
	}


	/* Do this as it's more efficient */
	if (TIM_GetITStatus(pios_ppm_cfg.timer, pios_ppm_cfg.ccr) == SET) {
		PreviousValue = CurrentValue;
		switch((int32_t) pios_ppm_cfg.ccr) {
			case (int32_t)TIM_IT_CC1:
				CurrentValue = TIM_GetCapture1(pios_ppm_cfg.timer);
				break;
			case (int32_t)TIM_IT_CC2:
				CurrentValue = TIM_GetCapture2(pios_ppm_cfg.timer);
				break;
			case (int32_t)TIM_IT_CC3:
				CurrentValue = TIM_GetCapture3(pios_ppm_cfg.timer);
				break;
			case (int32_t)TIM_IT_CC4:
				CurrentValue = TIM_GetCapture4(pios_ppm_cfg.timer);
				break;
		}
		CurrentValue+=TimerCounter;
		if(CurrentValue > 0xFFFF) {
			CurrentValue-=0xFFFF;
		}

		/* Clear TIMx Capture compare interrupt pending bit */
		TIM_ClearITPendingBit(pios_ppm_cfg.timer, pios_ppm_cfg.ccr);

		/* Capture computation */
		if (CurrentValue > PreviousValue) {
			CapturedValue = (CurrentValue - PreviousValue);
		} else {
			CapturedValue = ((0xFFFF - PreviousValue) + CurrentValue);
		}

		/* sync pulse */
		if (CapturedValue > 8000) {
			PulseIndex = 0;
			/* trying to detect bad pulses, not sure this is working correctly yet. I need a scope :P */
		} else if (CapturedValue > 750 && CapturedValue < 2500) {
			if (PulseIndex < PIOS_PPM_NUM_INPUTS) {
				CaptureValue[PulseIndex] = CapturedValue;
				CapCounter[PulseIndex]++;
				PulseIndex++;
			}
		}
	}
}

static void PIOS_PPM_Supervisor(uint32_t ppm_id) {
	/* 
	 * RTC runs at 625Hz so divide down the base rate so
	 * that this loop runs at 25Hz.
	 */
	if(++supv_timer < 25) {
		return;
	}
	supv_timer = 0;

	/* Simple state machine */
	if (SupervisorState == 0) {
		/* Save this states values */
		for (int32_t i = 0; i < PIOS_PPM_NUM_INPUTS; i++) {
			CapCounterPrev[i] = CapCounter[i];
		}

		/* Move to next state */
		SupervisorState = 1;
	} else {
		/* See what channels have been updated */
		for (int32_t i = 0; i < PIOS_PPM_NUM_INPUTS; i++) {
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

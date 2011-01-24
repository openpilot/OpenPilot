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
#include "pios_pwm_priv.h"

#if defined(PIOS_INCLUDE_PWM)

/* Local Variables */
static uint8_t CaptureState[PIOS_PWM_MAX_INPUTS];
static uint16_t RiseValue[PIOS_PWM_MAX_INPUTS];
static uint16_t FallValue[PIOS_PWM_MAX_INPUTS];
static uint32_t CaptureValue[PIOS_PWM_MAX_INPUTS];

//static uint8_t SupervisorState = 0;
static uint32_t CapCounter[PIOS_PWM_MAX_INPUTS];
//static uint32_t CapCounterPrev[MAX_CHANNELS];

/**
* Initialises all the pins
*/
void PIOS_PWM_Init(void)
{
	for (uint8_t i = 0; i < pios_pwm_cfg.num_channels; i++) {
		/* Flush counter variables */
		CaptureState[i] = 0;
		RiseValue[i] = 0;
		FallValue[i] = 0;
		CaptureValue[i] = 0;
		
		NVIC_InitTypeDef NVIC_InitStructure = pios_pwm_cfg.irq.init;
		GPIO_InitTypeDef GPIO_InitStructure = pios_pwm_cfg.gpio_init;
		TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure = pios_pwm_cfg.tim_base_init;
		TIM_ICInitTypeDef TIM_ICInitStructure = pios_pwm_cfg.tim_ic_init;
		
		struct pios_pwm_channel channel = pios_pwm_cfg.channels[i];
		
		/* Enable appropriate clock to timer module */
		switch((int32_t) channel.timer) {
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

		/* Enable GPIO */
		GPIO_InitStructure.GPIO_Pin = channel.pin;
		GPIO_Init(channel.port, &GPIO_InitStructure);
		
		/* Configure timer for input capture */
		TIM_ICInitStructure.TIM_Channel = channel.channel;
		TIM_ICInit(channel.timer, &TIM_ICInitStructure);
		
		/* Configure timer clocks */
		TIM_InternalClockConfig(channel.timer);
		if(channel.timer->PSC != ((PIOS_MASTER_CLOCK / 1000000) - 1)) 
			TIM_TimeBaseInit(channel.timer, &TIM_TimeBaseStructure);		
		
		/* Enable the Capture Compare Interrupt Request */
		TIM_ITConfig(channel.timer, channel.ccr, ENABLE);

		/* Enable timers */
		TIM_Cmd(channel.timer, ENABLE);
	}

	if(pios_pwm_cfg.remap) {
		/* Warning, I don't think this will work for multiple remaps at once */
		GPIO_PinRemapConfig(pios_pwm_cfg.remap, ENABLE);
	}

#if 0	
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
	if (Channel >= pios_pwm_cfg.num_channels) {
		return -1;
	}
	return CaptureValue[Channel];
}

void PIOS_PWM_irq_handler(TIM_TypeDef * timer)
{
	uint16_t val = 0;
	for(uint8_t i = 0; i < pios_pwm_cfg.num_channels; i++) {
		struct pios_pwm_channel channel = pios_pwm_cfg.channels[i];
		if ((channel.timer == timer) && (TIM_GetITStatus(channel.timer, channel.ccr) == SET)) {
			
			TIM_ClearITPendingBit(channel.timer, channel.ccr);
			
			switch(channel.channel) {
				case TIM_Channel_1:
					val = TIM_GetCapture1(channel.timer);
					break;
				case TIM_Channel_2:
					val = TIM_GetCapture2(channel.timer);
					break;
				case TIM_Channel_3:
					val = TIM_GetCapture3(channel.timer);
					break;
				case TIM_Channel_4:
					val = TIM_GetCapture4(channel.timer);
					break;					
			}
			
			if (CaptureState[i] == 0) {
				RiseValue[i] = val; 
			} else {
				FallValue[i] = val;
			}
			
			// flip state machine and capture value here
			/* Simple rise or fall state machine */
			TIM_ICInitTypeDef TIM_ICInitStructure = pios_pwm_cfg.tim_ic_init;
			if (CaptureState[i] == 0) {
				/* Switch states */
				CaptureState[i] = 1;
				
				/* Switch polarity of input capture */
				TIM_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Falling;
				TIM_ICInitStructure.TIM_Channel = channel.channel;
				TIM_ICInit(channel.timer, &TIM_ICInitStructure);				
			} else {
				/* Capture computation */
				if (FallValue[i] > RiseValue[i]) {
					CaptureValue[i] = (FallValue[i] - RiseValue[i]);
				} else {
					CaptureValue[i] = ((channel.timer->ARR - RiseValue[i]) + FallValue[i]);
				}
				
				/* Switch states */
				CaptureState[i] = 0;
				
				/* Increase supervisor counter */
				CapCounter[i]++;
				
				/* Switch polarity of input capture */
				TIM_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Rising;
				TIM_ICInitStructure.TIM_Channel = channel.channel;
				TIM_ICInit(channel.timer, &TIM_ICInitStructure);				
			}
		}		
	}
}

#if 0
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
#endif

/** 
  * @}
  * @}
  */

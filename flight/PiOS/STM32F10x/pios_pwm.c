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

/* Provide a RCVR driver */
static int32_t PIOS_PWM_Get(uint32_t rcvr_id, uint8_t chan_id);

const struct pios_rcvr_driver pios_pwm_rcvr_driver = {
	.read = PIOS_PWM_Get,
};

/* Local Variables */
static uint8_t CaptureState[PIOS_PWM_NUM_INPUTS];
static uint16_t RiseValue[PIOS_PWM_NUM_INPUTS];
static uint16_t FallValue[PIOS_PWM_NUM_INPUTS];
static uint32_t CaptureValue[PIOS_PWM_NUM_INPUTS];

static uint32_t CapCounter[PIOS_PWM_NUM_INPUTS];

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
}

/**
* Get the value of an input channel
* \param[in] Channel Number of the channel desired
* \output -1 Channel not available
* \output >0 Channel value
*/
static int32_t PIOS_PWM_Get(uint32_t rcvr_id, uint8_t channel)
{
	/* Return error if channel not available */
	if (channel >= pios_pwm_cfg.num_channels) {
		return -1;
	}
	return CaptureValue[channel];
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

#endif

/** 
  * @}
  * @}
  */

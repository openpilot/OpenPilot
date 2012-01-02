/**
  ******************************************************************************
  * @addtogroup PIOS PIOS Core hardware abstraction layer
  * @{
  * @addtogroup PIOS_HCSR04 HCSR04 Functions
  * @brief Hardware functions to deal with the altitude pressure sensor
  * @{
  *
  * @file       pios_hcsr04.c
  * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
  * @brief      HCSR04 sonar Sensor Routines
  * @see        The GNU Public License (GPL) Version 3
  *
  ******************************************************************************/
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

#if defined(PIOS_INCLUDE_HCSR04)
#if !(defined(PIOS_INCLUDE_DSM) || defined(PIOS_INCLUDE_SBUS))
#error Only supported with Spektrum/JR DSM or S.Bus interface!
#endif

/* Local Variables */

static TIM_ICInitTypeDef TIM_ICInitStructure;
static uint8_t CaptureState;
static uint16_t RiseValue;
static uint16_t FallValue;
static uint32_t CaptureValue;
static uint32_t CapCounter;

#define PIOS_HCSR04_TRIG_GPIO_PORT                  GPIOD
#define PIOS_HCSR04_TRIG_PIN                        GPIO_Pin_2

/**
* Initialise the HC-SR04 sensor
*/
void PIOS_HCSR04_Init(void)
{
	/* Init triggerpin */
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_StructInit(&GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_InitStructure.GPIO_Pin = PIOS_HCSR04_TRIG_PIN;
	GPIO_Init(PIOS_HCSR04_TRIG_GPIO_PORT, &GPIO_InitStructure);
	PIOS_HCSR04_TRIG_GPIO_PORT->BSRR = PIOS_HCSR04_TRIG_PIN;

	/* Flush counter variables */
	CaptureState = 0;
	RiseValue = 0;
	FallValue = 0;
	CaptureValue = 0;

	/* Setup RCC */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

	/* Enable timer interrupts */
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = PIOS_IRQ_PRIO_MID;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;
	NVIC_Init(&NVIC_InitStructure);

	/* Partial pin remap for TIM3 (PB5) */
	GPIO_PinRemapConfig(GPIO_PartialRemap_TIM3, ENABLE);

	/* Configure input pins */
	GPIO_StructInit(&GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	/* Configure timer for input capture */
	TIM_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Rising;
	TIM_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI;
	TIM_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;
	TIM_ICInitStructure.TIM_ICFilter = 0x0;
	TIM_ICInitStructure.TIM_Channel = TIM_Channel_2;
	TIM_ICInit(TIM3, &TIM_ICInitStructure);

	/* Configure timer clocks */
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	TIM_TimeBaseStructInit(&TIM_TimeBaseStructure);
	TIM_TimeBaseStructure.TIM_Period = 0xFFFF;
	TIM_TimeBaseStructure.TIM_Prescaler = (PIOS_MASTER_CLOCK / 500000) - 1;
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_InternalClockConfig(TIM3);
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);

	/* Enable the Capture Compare Interrupt Request */
	//TIM_ITConfig(PIOS_PWM_CH8_TIM_PORT, PIOS_PWM_CH8_CCR, ENABLE);
	TIM_ITConfig(TIM3, TIM_IT_CC2, DISABLE);

	/* Enable timers */
	TIM_Cmd(TIM3, ENABLE);

	/* Setup local variable which stays in this scope */
	/* Doing this here and using a local variable saves doing it in the ISR */
	TIM_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI;
	TIM_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;
	TIM_ICInitStructure.TIM_ICFilter = 0x0;
}

/**
* Get the value of an sonar timer
* \output >0 timer value
*/
int32_t PIOS_HCSR04_Get(void)
{
	return CaptureValue;
}

/**
* Get the value of an sonar timer
* \output >0 timer value
*/
int32_t PIOS_HCSR04_Completed(void)
{
	return CapCounter;
}
/**
* Trigger sonar sensor
*/
void PIOS_HCSR04_Trigger(void)
{
	CapCounter=0;
	PIOS_HCSR04_TRIG_GPIO_PORT->BSRR = PIOS_HCSR04_TRIG_PIN;
	PIOS_DELAY_WaituS(15);
	PIOS_HCSR04_TRIG_GPIO_PORT->BRR = PIOS_HCSR04_TRIG_PIN;
	TIM_ITConfig(TIM3, TIM_IT_CC2, ENABLE);
}


/**
* Handle TIM3 global interrupt request
*/
//void PIOS_PWM_irq_handler(TIM_TypeDef * timer)
void TIM3_IRQHandler(void)
{
	/* Zero value always will be changed but this prevents compiler warning */
	int32_t i = 0;

	/* Do this as it's more efficient */
	if (TIM_GetITStatus(TIM3, TIM_IT_CC2) == SET) {
		i = 7;
		if (CaptureState == 0) {
			RiseValue = TIM_GetCapture2(TIM3);
		} else {
			FallValue = TIM_GetCapture2(TIM3);
		}
	}

	/* Clear TIM3 Capture compare interrupt pending bit */
	TIM_ClearITPendingBit(TIM3, TIM_IT_CC2);

	/* Simple rise or fall state machine */
	if (CaptureState == 0) {
		/* Switch states */
		CaptureState = 1;

		/* Switch polarity of input capture */
		TIM_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Falling;
		TIM_ICInitStructure.TIM_Channel = TIM_Channel_2;
		TIM_ICInit(TIM3, &TIM_ICInitStructure);

	} else {
		/* Capture computation */
		if (FallValue > RiseValue) {
			CaptureValue = (FallValue - RiseValue);
		} else {
			CaptureValue = ((0xFFFF - RiseValue) + FallValue);
		}

		/* Switch states */
		CaptureState = 0;

		/* Increase supervisor counter */
		CapCounter++;
		TIM_ITConfig(TIM3, TIM_IT_CC2, DISABLE);

		/* Switch polarity of input capture */
		TIM_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Rising;
		TIM_ICInitStructure.TIM_Channel = TIM_Channel_2;
		TIM_ICInit(TIM3, &TIM_ICInitStructure);

	}
}


#endif

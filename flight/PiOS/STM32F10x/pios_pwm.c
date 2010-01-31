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


/* Local Variables */
volatile uint16_t IC3Value = 0;
volatile uint16_t DutyCycle = 0;
volatile uint32_t Frequency = 0;

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


	/* TIM3 channel 2 pin (PA.01) configuration */
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//GPIO_Mode_AF_OD
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;

	GPIO_InitStructure.GPIO_Pin = RECEIVER1_PIN;
	GPIO_Init(RECEIVER1_GPIO_PORT, &GPIO_InitStructure);

	/* TIM3 configuration: PWM Input mode ------------------------
	The Rising edge is used as active edge,
	The TIM3 CCR2 is used to compute the frequency value
	The TIM3 CCR1 is used to compute the duty cycle value
	------------------------------------------------------------ */
	TIM_ICInitTypeDef  TIM_ICInitStructure;
	TIM_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Falling;
	TIM_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI;
	TIM_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;
	TIM_ICInitStructure.TIM_ICFilter = 0x0;

	TIM_ICInitStructure.TIM_Channel = RECEIVER1_CH;
	TIM_PWMIConfig(TIM3, &TIM_ICInitStructure);

	/* Select the TIM3 Input Trigger: TI2FP2 */
	TIM_SelectInputTrigger(TIM3, TIM_TS_TI2FP2);

	/* Select the slave Mode: Reset Mode */
	TIM_SelectSlaveMode(TIM3, TIM_SlaveMode_Reset);

	/* Enable the Master/Slave Mode */
	TIM_SelectMasterSlaveMode(TIM3, TIM_MasterSlaveMode_Enable);

	/* TIM enable counter */
	TIM_Cmd(TIM3, ENABLE);

	/* Enable the CC3 Interrupt Request */
	TIM_ITConfig(TIM3, TIM_IT_CC3, ENABLE);
}

/**
* This function handles TIM3 global interrupt request.
*/
void TIM3_IRQHandler(void)
{
	/* Clear TIM3 Capture compare interrupt pending bit */
	TIM_ClearITPendingBit(TIM3, TIM_IT_CC3);

	/* Get the Input Capture value */
	IC3Value = TIM_GetCapture3(TIM3);

#if 0
	if (IC3Value != 0)
	{
		/* Duty cycle computation */
		DutyCycle = (TIM_GetCapture1(TIM3) * 100) / IC3Value;

		/* Frequency computation */
		Frequency = 72000000 / IC3Value;
	} else {
		DutyCycle = 0;
		Frequency = 0;
	}
#endif
}

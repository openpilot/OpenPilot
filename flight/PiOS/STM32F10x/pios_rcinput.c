/**
 ******************************************************************************
 * @addtogroup PIOS PIOS Core hardware abstraction layer
 * @{
 * @addtogroup   PIOS_RCINPUT Functions
 * @brief PIOS interface for rcinput
 * @{
 *
 * @file       pios_rcinput.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * 	        Parts by Thorsten Klose (tk@midibox.org) (tk@midibox.org)
 * @brief      USART commands. Inits USARTs, controls USARTs & Interupt handlers. (STM32 dependent)
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

typedef enum { MANUALCONTROLSETTINGS_INPUTMODE_PWM=0, MANUALCONTROLSETTINGS_INPUTMODE_PPM=1, MANUALCONTROLSETTINGS_INPUTMODE_SPEKTRUM=2 } inputmode;

//pwm
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
//static uint32_t CaptureValue[PIOS_PWM_NUM_INPUTS];

static uint8_t SupervisorState = 0;
static uint32_t CapCounter[PIOS_PWM_NUM_INPUTS];
static uint32_t CapCounterPrev[PIOS_PWM_NUM_INPUTS];
//pwm

//ppm
//static TIM_ICInitTypeDef TIM_ICInitStructure;
static uint8_t PulseIndex;
static uint32_t PreviousValue;
static uint32_t CurrentValue;
static uint32_t CapturedValue;
//static uint32_t CaptureValue[PIOS_PPM_NUM_INPUTS];

//static uint8_t SupervisorState = 0;
//static uint32_t CapCounter[PIOS_PPM_NUM_INPUTS];
//static uint32_t CapCounterPrev[PIOS_PPM_NUM_INPUTS];
//ppm

//spektrum
static uint32_t CaptureValue[12];
static uint8_t prev_byte = 0xFF, sync = 0, bytecount = 0, byte_array[20] = { 0 };
uint8_t sync_of = 0;
//spektrum

/* Global Variables */

/* Local Variables */
/* invalid mode so first set works correctly */
static uint8_t InputMode=4;

static struct pios_rcinput_driver rcinputs[] = {
	{
		.mode = MANUALCONTROLSETTINGS_INPUTMODE_PWM,
		.init = PIOS_PWM_Init,
		.deinit = PIOS_PWM_DeInit,
		.get_channel = PIOS_PWM_Get,
	},
	{
		.mode = MANUALCONTROLSETTINGS_INPUTMODE_PPM,
		.init = PIOS_PPM_Init,
		.deinit = PIOS_PPM_DeInit,
		.get_channel = PIOS_PPM_Get,
	},
	{
		.mode = MANUALCONTROLSETTINGS_INPUTMODE_SPEKTRUM,
		.init = PIOS_SPEKTRUM_Init,
		.deinit = PIOS_SPEKTRUM_DeInit,
		.get_channel = PIOS_SPEKTRUM_Get,
	},
};

/**
* Set current input mode
* \param[in] deinits the old mode and inits the new
*/
void PIOS_InputMode_Set(uint8_t Mode)
{
	if(Mode!=InputMode)
	{
		/* Handle first set correctly */
		if(InputMode<NELEMENTS(rcinputs))
		{
			rcinputs[InputMode].deinit();
		}
		if(Mode<NELEMENTS(rcinputs))
		{
			InputMode=Mode;
			rcinputs[Mode].init();
		}
	}
}

/**
* Get current input mode
* \output input mode
*/
uint8_t PIOS_InputMode_Get(void)
{
	return InputMode;
}

/**
* Get the value of an input channel
* \param[in] Channel Number of the channel desired
* \output -1 Channel not available
* \output >0 Channel value
*/
int32_t PIOS_RcInput_Get(int8_t Channel)
{
	return rcinputs[InputMode].get_channel(Channel);
}

/*** PPM ***/

/**
* Initialises PPM mode
*/
void PIOS_PPM_Init(void)
{
	/* Flush counter variables */
	int32_t i;

	PulseIndex = 0;
	PreviousValue = 0;
	CurrentValue = 0;
	CapturedValue = 0;

	for (i = 0; i < PIOS_PPM_NUM_INPUTS; i++) {
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
	for (i = 0; i < PIOS_PPM_NUM_INPUTS; i++) {
		CapCounter[i] = 0;
	}
	for (i = 0; i < PIOS_PPM_NUM_INPUTS; i++) {
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
	TIM_TimeBaseStructure.TIM_Prescaler = (PIOS_MASTER_CLOCK / 1000000) - 1;	/* For 1 uS accuracy */
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
* Deinitialises PPM mode
*/
void PIOS_PPM_DeInit(void)
{
	/* TODO! PIOS_PPM_DeInit*/
	int32_t i;
	for (i = 0; i < PIOS_PPM_NUM_INPUTS; i++) {
		CaptureValue[i] = 0;
	}

	/* Supervisor Setup */
#if (PIOS_PPM_SUPV_ENABLED)
	/* Disable counter */
	TIM_Cmd(PIOS_PPM_SUPV_TIMER, DISABLE);

	/* Disable the CC2 Interrupt Request */
	TIM_ITConfig(PIOS_PPM_SUPV_TIMER, TIM_IT_Update, DISABLE);

	/* DeConfigure interrupts */
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = PIOS_PPM_SUPV_IRQ_CHANNEL;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = DISABLE;
	NVIC_Init(&NVIC_InitStructure);
#endif

	/* Disable timers */
	TIM_Cmd(PIOS_PPM_TIM, DISABLE);

	/* Disable the Capture Compare Interrupt Request */
	TIM_ITConfig(PIOS_PPM_TIM_PORT, PIOS_PPM_TIM_CCR, DISABLE);

	/* Configure input pins */
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_InitStructure.GPIO_Pin = PIOS_PPM_GPIO_PIN;
	GPIO_Init(PIOS_PPM_GPIO_PORT, &GPIO_InitStructure);


	/* Disable timer interrupts */
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = DISABLE;
	NVIC_InitStructure.NVIC_IRQChannel = PIOS_PPM_TIM_IRQ;
	NVIC_Init(&NVIC_InitStructure);

	/* Setup RCC */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, DISABLE);
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
	if (Channel >= PIOS_PPM_NUM_INPUTS) {
		return -1;
	}
	return CaptureValue[Channel];
}
/*** PPM ***/

/*** PWM ***/

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
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
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
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
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
* Deinitialises PWM mode
*/
void PIOS_PWM_DeInit(void)
{
	/* TODO! PIOS_PWM_DeInit*/
	int32_t i;
	for (i = 0; i < PIOS_PWM_NUM_INPUTS; i++) {
		CaptureValue[i] = 0;
	}

	/* Supervisor Setup */
#if (PIOS_PWM_SUPV_ENABLED)
	/* Disable counter */
	TIM_Cmd(PIOS_PWM_SUPV_TIMER, DISABLE);

	/* Disable the CC2 Interrupt Request */
	TIM_ITConfig(PIOS_PWM_SUPV_TIMER, TIM_IT_Update, DISABLE);

	/* DeConfigure interrupts */
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = PIOS_PWM_SUPV_IRQ_CHANNEL;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = DISABLE;
	NVIC_Init(&NVIC_InitStructure);
#endif

	/* Disable timers */
	for (i = 0; i < PIOS_PWM_NUM_TIMS; i++) {
		TIM_Cmd(PIOS_PWM_TIM[i], DISABLE);
	}

	/* Configure timer clocks */
	for (i = 0; i < PIOS_PWM_NUM_INPUTS; i++) {
		TIM_InternalClockConfig(PIOS_PWM_TIM_PORT[i]);
		//TIM_TimeBaseInit(PIOS_PWM_TIM_PORT[i], &TIM_TimeBaseStructure);

		/* Disable the Capture Compare Interrupt Request */
		TIM_ITConfig(PIOS_PWM_TIM_PORT[i], PIOS_PWM_TIM_CCR[i], DISABLE);
	}

	/* Configure input pins */
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	for (i = 0; i < PIOS_PWM_NUM_INPUTS; i++) {
		GPIO_InitStructure.GPIO_Pin = PIOS_PWM_GPIO_PIN[i];
		GPIO_Init(PIOS_PWM_GPIO_PORT[i], &GPIO_InitStructure);
	}
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
	int32_t i;

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
/*** PWM ***/



/*** SPEKTRUM ***/
/**
* Initialise the onboard USARTs
*/
void PIOS_SPEKTRUM_Init(void)
{
	// TODO: need setting flag for bind on next powerup
	if (0) {
		PIOS_SPEKTRUM_Bind();
	}

	/* spektrum "watchdog" timer */
	/* Enable timer clock */
	PIOS_SPEKTRUM_SUPV_TIMER_RCC_FUNC;

	/* Configure interrupts */
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = PIOS_SPEKTRUM_SUPV_IRQ_CHANNEL;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	/* Time base configuration */
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	TIM_TimeBaseStructInit(&TIM_TimeBaseStructure);
	TIM_TimeBaseStructure.TIM_Period = ((1000000 / PIOS_SPEKTRUM_SUPV_HZ) - 1);
	TIM_TimeBaseStructure.TIM_Prescaler = (PIOS_MASTER_CLOCK / 1000000) - 1;	/* For 1 uS accuracy */
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(PIOS_PPM_SUPV_TIMER, &TIM_TimeBaseStructure);

	/* Enable the Update Interrupt Request */
	TIM_ITConfig(PIOS_SPEKTRUM_SUPV_TIMER, TIM_IT_Update, ENABLE);

	/* Clear update pending flag */
	TIM_ClearFlag(TIM6, TIM_FLAG_Update);

	/* Enable counter */
	TIM_Cmd(PIOS_SPEKTRUM_SUPV_TIMER, ENABLE);
}

/**
* Deinitialises SPEKTRUM mode
*/
void PIOS_SPEKTRUM_DeInit(void)
{
	/* TODO! PIOS_SPEKTRUM_DeInit*/

	/* Supervisor Setup */
	/* Disable counter */
	TIM_Cmd(PIOS_SPEKTRUM_SUPV_TIMER, DISABLE);

	/* Disable the CC2 Interrupt Request */
	TIM_ITConfig(PIOS_SPEKTRUM_SUPV_TIMER, TIM_IT_Update, DISABLE);

	/* DeConfigure interrupts */
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = PIOS_SPEKTRUM_SUPV_IRQ_CHANNEL;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = DISABLE;
	NVIC_Init(&NVIC_InitStructure);

}


/**
* Get the value of an input channel
* \param[in] Channel Number of the channel desired
* \output -1 Channel not available
* \output >0 Channel value
*/
int32_t PIOS_SPEKTRUM_Get(int8_t Channel)
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
			if (channeln < 12)
				CaptureValue[channeln] = data;
		}
	}
	if (bytecount == 16) {
		//PIOS_COM_SendBufferNonBlocking(PIOS_COM_TELEM_RF,byte_array,16); //00 2c 58 84 b0 dc ff
		bytecount = 0;
		sync = 0;
		sync_of = 0;
	}
	prev_byte = b;
	return 0;
}

/* Interrupt handler for USART3 */
void SPEKTRUM_IRQHandler(void)
{
	/* check if RXNE flag is set */
	if (USART1->SR & (1 << 5)) {
		uint8_t b = USART1->DR;
		if (PIOS_SPEKTRUM_Decode(b) < 0) {
			/* Here we could add some error handling */
		}
	}

	if (USART1->SR & (1 << 7)) {	// check if TXE flag is set
		/* Disable TXE interrupt (TXEIE=0) */
		USART1->CR1 &= ~(1 << 7);
	}
	/* clear "watchdog" timer */
	TIM_SetCounter(PIOS_SPEKTRUM_SUPV_TIMER, 0);
}

/*** SPEKTRUM ***/

/*** SHARED INTERRUPTS ***/

/**
* Handle TIM1 global interrupt request
* Some work and testing still needed, need to detect start of frame and decode pulses
*
*/
void TIM1_CC_IRQHandler(void)
{
	if(InputMode == MANUALCONTROLSETTINGS_INPUTMODE_PPM){
		/* Do this as it's more efficient */
		if (TIM_GetITStatus(PIOS_PPM_TIM_PORT, PIOS_PPM_TIM_CCR) == SET) {
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
	} else if(InputMode == MANUALCONTROLSETTINGS_INPUTMODE_PWM){
		int32_t i;

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
}


/**
* This function handles TIM6 global interrupt request.
*/
void TIM6_IRQHandler(void) {
	if(InputMode == MANUALCONTROLSETTINGS_INPUTMODE_SPEKTRUM){
		/* Clear timer interrupt pending bit */
		TIM_ClearITPendingBit(PIOS_SPEKTRUM_SUPV_TIMER, TIM_IT_Update);

		/* sync between frames, TODO! DX7SE */
		sync = 0;
		bytecount = 0;
		prev_byte = 0xFF;
		sync_of++;
		/* watchdog activated */
		if (sync_of > 1) {
			/* signal lost */
			sync_of = 0;
			for (int i = 0; i < 12; i++)
				CaptureValue[i] = 0;
		}
	}else if(InputMode == MANUALCONTROLSETTINGS_INPUTMODE_PWM){
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
	}else if(InputMode == MANUALCONTROLSETTINGS_INPUTMODE_PPM){
		/* Clear timer interrupt pending bit */
		TIM_ClearITPendingBit(PIOS_PPM_SUPV_TIMER, TIM_IT_Update);

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
}
/*** SHARED INTERRUPTS ***/


/**
  * @}
  * @}
  */

/**
 ******************************************************************************
 *
 * @file       ppm.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      Sends or Receives the ppm values to/from the remote unit
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

#include <string.h>	// memmove

#include "main.h"
#include "rfm22b.h"
#include "saved_settings.h"
#include "ppm.h"

#if defined(PIOS_COM_DEBUG)
	#define PPM_DEBUG
#endif

// *************************************************************

#define PPM_OUT_FRAME_PERIOD_US          20000                      // microseconds
#define PPM_OUT_HIGH_PULSE_US            480                        // microseconds
#define PPM_OUT_MIN_CHANNEL_PULSE_US     850                        // microseconds
#define PPM_OUT_MAX_CHANNEL_PULSE_US     2200                       // microseconds

#define PPM_IN_MIN_SYNC_PULSE_US         7000                       // microseconds .. Pip's 6-chan TX goes down to 8.8ms
#define PPM_IN_MAX_SYNC_PULSE_US         16000                      // microseconds .. Pip's 6-chan TX goes up to 14.4ms

#define PPM_IN_MIN_CHANNEL_PULSE_US      750                        // microseconds
#define PPM_IN_MAX_CHANNEL_PULSE_US      2400                       // microseconds

// *************************************************************

uint8_t ppm_mode;

volatile bool ppm_initialising = true;

volatile uint32_t ppm_In_PrevFrames = 0;

volatile uint32_t ppm_In_LastValidFrameTimer = 0;
volatile uint32_t ppm_In_Frames = 0;
volatile uint32_t ppm_In_SyncPulseWidth = 0;
volatile uint32_t ppm_In_LastFrameTime = 0;
volatile uint8_t ppm_In_NoisyChannelCounter = 0;
volatile int8_t ppm_In_ChannelsDetected = 0;
volatile int8_t ppm_In_ChannelPulseIndex = -1;
volatile uint32_t ppm_In_PreviousValue = 0;
volatile uint32_t ppm_In_CurrentValue = 0;
volatile uint32_t ppm_In_ChannelPulseWidthNew[PIOS_PPM_MAX_CHANNELS];
volatile uint32_t ppm_In_ChannelPulseWidth[PIOS_PPM_MAX_CHANNELS];

volatile uint16_t ppm_Out_ChannelPulseWidth[PIOS_PPM_MAX_CHANNELS];
volatile uint16_t ppm_Out_SyncPulseWidth = PPM_OUT_FRAME_PERIOD_US;
volatile int8_t ppm_Out_ChannelPulseIndex = -1;
volatile uint8_t ppm_Out_ChannelsUsed = 0;

// *************************************************************

// Initialise the PPM INPUT
void ppm_In_Init(void)
{
	TIM_ICInitTypeDef TIM_ICInitStructure;

	// disable the timer
	TIM_Cmd(PIOS_PPM_TIM, DISABLE);

	ppm_In_PrevFrames = 0;
	ppm_In_NoisyChannelCounter = 0;
	ppm_In_LastValidFrameTimer = 0;
	ppm_In_Frames = 0;
	ppm_In_SyncPulseWidth = 0;
	ppm_In_LastFrameTime = 0;
	ppm_In_ChannelsDetected = 0;
	ppm_In_ChannelPulseIndex = -1;
	ppm_In_PreviousValue = 0;
	ppm_In_CurrentValue = 0;

	for (int i = 0; i < PIOS_PPM_MAX_CHANNELS; i++)
	{
		ppm_In_ChannelPulseWidthNew[i] = 0;
		ppm_In_ChannelPulseWidth[i] = 0;
	}

	// Setup RCC
	PIOS_PPM_TIMER_EN_RCC_FUNC;

	// Enable timer interrupts
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = PIOS_IRQ_PRIO_MID;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_InitStructure.NVIC_IRQChannel = PIOS_PPM_TIM_IRQ;
	NVIC_Init(&NVIC_InitStructure);

	// Init PPM IN pin
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_StructInit(&GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = PPM_IN_PIN;
	GPIO_InitStructure.GPIO_Mode = PPM_IN_MODE;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(PPM_IN_PORT, &GPIO_InitStructure);

	// remap the pin to switch it to timer mode
//	GPIO_PinRemapConfig(GPIO_PartialRemap1_TIM2, ENABLE);
	GPIO_PinRemapConfig(GPIO_PartialRemap2_TIM2, ENABLE);
//	GPIO_PinRemapConfig(GPIO_FullRemap_TIM2, ENABLE);

	// Configure timer for input capture
	TIM_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Rising;
	TIM_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI;
	TIM_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;
	TIM_ICInitStructure.TIM_ICFilter = 0x0;
	TIM_ICInitStructure.TIM_Channel = PIOS_PPM_IN_TIM_CHANNEL;
	TIM_ICInit(PIOS_PPM_TIM_PORT, &TIM_ICInitStructure);

	// Configure timer clocks
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	TIM_TimeBaseStructInit(&TIM_TimeBaseStructure);
	TIM_TimeBaseStructure.TIM_Period = 0xFFFF;
	TIM_TimeBaseStructure.TIM_Prescaler = (PIOS_MASTER_CLOCK / 1000000) - 1;
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_InternalClockConfig(PIOS_PPM_TIM_PORT);
	TIM_TimeBaseInit(PIOS_PPM_TIM_PORT, &TIM_TimeBaseStructure);

	// Enable the Capture Compare Interrupt Request
	TIM_ITConfig(PIOS_PPM_TIM_PORT, PIOS_PPM_IN_TIM_CCR, ENABLE);

	// Clear TIMER Capture compare interrupt pending bit
	TIM_ClearITPendingBit(PIOS_PPM_TIM_PORT, PIOS_PPM_IN_TIM_CCR);

	// Enable timer
	TIM_Cmd(PIOS_PPM_TIM, ENABLE);

	// Setup local variable which stays in this scope
	// Doing this here and using a local variable saves doing it in the ISR
	TIM_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI;
	TIM_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;
	TIM_ICInitStructure.TIM_ICFilter = 0x0;

#ifdef PPM_DEBUG
	DEBUG_PRINTF("ppm_in: initialised\r\n");
#endif
}

// TIMER capture/compare interrupt
void PIOS_PPM_IN_CC_IRQ(void)
{
	uint32_t pulse_width_us;    // new pulse width in microseconds

	if (booting || ppm_initialising)
	{	// just clear the interrupt
		if (TIM_GetITStatus(PIOS_PPM_TIM_PORT, PIOS_PPM_IN_TIM_CCR) == SET)
		{
			TIM_ClearITPendingBit(PIOS_PPM_TIM_PORT, PIOS_PPM_IN_TIM_CCR);
			PIOS_PPM_IN_TIM_GETCAP_FUNC(PIOS_PPM_TIM_PORT);
		}
		TIM_ClearITPendingBit(PIOS_PPM_TIM_PORT, PIOS_PPM_IN_TIM_CCR);
		return;
	}

	// Do this as it's more efficient
	if (TIM_GetITStatus(PIOS_PPM_TIM_PORT, PIOS_PPM_IN_TIM_CCR) == SET)
	{
		ppm_In_PreviousValue = ppm_In_CurrentValue;
		ppm_In_CurrentValue = PIOS_PPM_IN_TIM_GETCAP_FUNC(PIOS_PPM_TIM_PORT);
	}

	// Clear TIMER Capture compare interrupt pending bit
	TIM_ClearITPendingBit(PIOS_PPM_TIM_PORT, PIOS_PPM_IN_TIM_CCR);

	// Capture computation
	if (ppm_In_CurrentValue > ppm_In_PreviousValue)
		pulse_width_us = (ppm_In_CurrentValue - ppm_In_PreviousValue);
	else
		pulse_width_us = ((0xFFFF - ppm_In_PreviousValue) + ppm_In_CurrentValue);

	// ********

	#ifdef PPM_DEBUG
//		DEBUG_PRINTF("ppm_in: %uus\r\n", pulse_width_us);
	#endif

	if (pulse_width_us >= PPM_IN_MIN_SYNC_PULSE_US)
	{	// SYNC pulse

		if (pulse_width_us <= PPM_IN_MAX_SYNC_PULSE_US)
		{	// SYNC pulse width is within accepted tolerance

			if (ppm_In_ChannelPulseIndex > 0)
			{	// we found some channel PWM's in the PPM stream

				if (ppm_In_ChannelsDetected > 0 && ppm_In_ChannelPulseIndex == ppm_In_ChannelsDetected)
				{	// detected same number of channels as in previous PPM frame .. save the new channel PWM values
//					if (ppm_In_NoisyChannelCounter <= 2)	// only update channels if the channels are fairly noise free
						for (int i = 0; i < PIOS_PPM_MAX_CHANNELS; i++)
							ppm_In_ChannelPulseWidth[i] = ppm_In_ChannelPulseWidthNew[i];
				}
				ppm_In_ChannelsDetected = ppm_In_ChannelPulseIndex;     // the number of channels we found in this frame

				ppm_In_LastValidFrameTimer = 0;                         // reset timer

				ppm_In_Frames++;                                        // update frame counter
			}

			ppm_In_NoisyChannelCounter = 0;                             // reset noisy channel detector
			ppm_In_ChannelPulseIndex = 0;                               // start of PPM frame
			ppm_In_LastFrameTime = 0;                                   // reset timer
		}

		ppm_In_SyncPulseWidth = pulse_width_us;                         // remember the length of this SYNC pulse
	}
	else
	if (ppm_In_SyncPulseWidth > 0 && ppm_In_ChannelPulseIndex >= 0)
	{	// CHANNEL pulse

		if (pulse_width_us >= PPM_IN_MIN_CHANNEL_PULSE_US && pulse_width_us <= PPM_IN_MAX_CHANNEL_PULSE_US)
		{	// this new channel pulse is within the accepted tolerance range
			if (ppm_In_ChannelPulseIndex < PIOS_PPM_MAX_CHANNELS)
			{
				int32_t difference = (int32_t)pulse_width_us - ppm_In_ChannelPulseWidthNew[ppm_In_ChannelPulseIndex];
				if (abs(difference) >= 300)
					ppm_In_NoisyChannelCounter++;                       // possibly a noisy channel - or an RC switch was moved

				ppm_In_ChannelPulseWidthNew[ppm_In_ChannelPulseIndex] = pulse_width_us;    // save it
			}

			if (ppm_In_ChannelPulseIndex < 127)
				ppm_In_ChannelPulseIndex++;                             // next channel

			ppm_In_LastFrameTime = 0;                                   // reset timer
		}
		else
		{	// bad/noisy channel pulse .. reset state to wait for next SYNC pulse
			ppm_In_Frames = 0;
			ppm_In_ChannelPulseIndex = -1;
		}
	}

	// ********
}

void ppm_In_Supervisor(void)
{	// this gets called once every millisecond by an interrupt

	if (booting || ppm_initialising)
		return;

	if (ppm_In_LastValidFrameTimer < 0xffffffff)
		ppm_In_LastValidFrameTimer++;

	if (ppm_In_LastFrameTime < 0xffffffff)
		ppm_In_LastFrameTime++;

	if (ppm_In_LastFrameTime > ((PPM_IN_MAX_SYNC_PULSE_US * 2) / 1000) && ppm_In_SyncPulseWidth > 0)
	{	// no PPM frames detected for a while .. reset PPM state
		ppm_In_SyncPulseWidth = 0;
		ppm_In_ChannelsDetected = 0;
		ppm_In_ChannelPulseIndex = -1;
		ppm_In_NoisyChannelCounter = 0;
		ppm_In_Frames = 0;
	}
}

uint32_t ppm_In_NewFrame(void)
{
	if (booting || ppm_initialising)
		return 0;

	if (ppm_In_Frames >= 4 && ppm_In_Frames != ppm_In_PrevFrames)
	{	// we have a new PPM frame
		ppm_In_PrevFrames = ppm_In_Frames;
		return ppm_In_PrevFrames;
	}

	return 0;
}

int32_t ppm_In_GetChannelPulseWidth(uint8_t channel)
{
	if (booting || ppm_initialising)
		return -1;

	// Return error if channel not available
	if (channel >= PIOS_PPM_MAX_CHANNELS || channel >= ppm_In_ChannelsDetected)
		return -2;

	if (ppm_In_LastValidFrameTimer > (PPM_IN_MAX_SYNC_PULSE_US * 4) / 1000)
		return 0;	// to long since last valid PPM frame

	return ppm_In_ChannelPulseWidth[channel];    // return channel pulse width
}

// *************************************************************

// Initialise the PPM INPUT
void ppm_Out_Init(void)
{
	// disable the timer
	TIM_Cmd(PIOS_PPM_TIM, DISABLE);

	ppm_Out_SyncPulseWidth = PPM_OUT_FRAME_PERIOD_US;
	ppm_Out_ChannelPulseIndex = -1;
	ppm_Out_ChannelsUsed = 0;
	for (int i = 0; i < PIOS_PPM_MAX_CHANNELS; i++)
		ppm_Out_ChannelPulseWidth[i] = 1000;
//		ppm_Out_ChannelPulseWidth[i] = 1000 + i * 100;	// TEST ONLY

//	ppm_Out_ChannelsUsed = 5;	// TEST ONLY

	PIOS_PPM_TIMER_EN_RCC_FUNC;

	// Init PPM OUT pin
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_StructInit(&GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = PPM_OUT_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
	GPIO_Init(PPM_OUT_PORT, &GPIO_InitStructure);

	// remap the pin to switch it to timer mode
//	GPIO_PinRemapConfig(GPIO_PartialRemap1_TIM2, ENABLE);
//	GPIO_PinRemapConfig(GPIO_PartialRemap2_TIM2, ENABLE);
	GPIO_PinRemapConfig(GPIO_FullRemap_TIM2, ENABLE);

	// Enable timer interrupt
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = PIOS_IRQ_PRIO_MID;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_InitStructure.NVIC_IRQChannel = PIOS_PPM_TIM_IRQ;
	NVIC_Init(&NVIC_InitStructure);

	// Time base configuration
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	TIM_TimeBaseStructInit(&TIM_TimeBaseStructure);
	TIM_TimeBaseStructure.TIM_Period = ppm_Out_SyncPulseWidth - 1;
	TIM_TimeBaseStructure.TIM_Prescaler = (PIOS_MASTER_CLOCK / 1000000) - 1;
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_InternalClockConfig(PIOS_PPM_TIM_PORT);
	TIM_TimeBaseInit(PIOS_PPM_TIM_PORT, &TIM_TimeBaseStructure);

	// Set up for output compare function
	TIM_OCInitTypeDef TIM_OCInitStructure;
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
	TIM_OCInitStructure.TIM_OutputNState = TIM_OutputNState_Disable;
	TIM_OCInitStructure.TIM_Pulse = PPM_OUT_HIGH_PULSE_US;
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
	TIM_OCInitStructure.TIM_OCNPolarity = TIM_OCPolarity_High;
	TIM_OCInitStructure.TIM_OCIdleState = TIM_OCIdleState_Reset;
	TIM_OCInitStructure.TIM_OCNIdleState = TIM_OCNIdleState_Reset;
	TIM_OC3Init(PIOS_PPM_TIM, &TIM_OCInitStructure);
	TIM_OC3PreloadConfig(PIOS_PPM_TIM, TIM_OCPreload_Enable);

	TIM_ARRPreloadConfig(PIOS_PPM_TIM, ENABLE);

	// TIMER Main Output Enable
	TIM_CtrlPWMOutputs(PIOS_PPM_TIM, ENABLE);

	// TIM IT enable
	TIM_ITConfig(PIOS_PPM_TIM, PIOS_PPM_OUT_TIM_CCR, ENABLE);

	// Clear TIMER Capture compare interrupt pending bit
	TIM_ClearITPendingBit(PIOS_PPM_TIM_PORT, PIOS_PPM_IN_TIM_CCR);

	// Enable clock to timer module
	TIM_Cmd(PIOS_PPM_TIM, ENABLE);

#ifdef PPM_DEBUG
	DEBUG_PRINTF("ppm_out: initialised\r\n");
#endif
}

// TIMER capture/compare interrupt
void PIOS_PPM_OUT_CC_IRQ(void)
{
	// clear the interrupt
	if (TIM_GetITStatus(PIOS_PPM_TIM_PORT, PIOS_PPM_OUT_TIM_CCR) == SET)
	{
		TIM_ClearITPendingBit(PIOS_PPM_TIM_PORT, PIOS_PPM_OUT_TIM_CCR);
		PIOS_PPM_IN_TIM_GETCAP_FUNC(PIOS_PPM_TIM_PORT);
	}
	TIM_ClearITPendingBit(PIOS_PPM_TIM_PORT, PIOS_PPM_OUT_TIM_CCR);

	if (booting || ppm_initialising)
		return;

	// *************************
	// update the TIMER period (channel pulse width)

	if (ppm_Out_ChannelPulseIndex < 0)
	{	// SYNC PULSE
		TIM_SetAutoreload(PIOS_PPM_TIM, ppm_Out_SyncPulseWidth - 1);    // sync pulse length
		ppm_Out_SyncPulseWidth = PPM_OUT_FRAME_PERIOD_US;               // reset sync period

		if (ppm_Out_ChannelsUsed > 0)
			ppm_Out_ChannelPulseIndex = 0;                              // onto channel-1
	}
	else
	{	// CHANNEL PULSE
		uint16_t pulse_width = ppm_Out_ChannelPulseWidth[ppm_Out_ChannelPulseIndex];
		if (pulse_width < PPM_OUT_MIN_CHANNEL_PULSE_US) pulse_width = PPM_OUT_MIN_CHANNEL_PULSE_US;
		else
		if (pulse_width > PPM_OUT_MAX_CHANNEL_PULSE_US) pulse_width = PPM_OUT_MAX_CHANNEL_PULSE_US;

		TIM_SetAutoreload(PIOS_PPM_TIM, pulse_width - 1);              // channel pulse width
		ppm_Out_SyncPulseWidth -= pulse_width;                         // maintain constant PPM frame period

		// TEST ONLY
//		pulse_width += 4;
//		if (pulse_width > 2000) pulse_width = 1000;
//		ppm_Out_ChannelPulseWidth[ppm_Out_ChannelPulseIndex] = pulse_width;

		ppm_Out_ChannelPulseIndex++;
		if (ppm_Out_ChannelPulseIndex >= ppm_Out_ChannelsUsed || ppm_Out_ChannelPulseIndex >= PIOS_PPM_MAX_CHANNELS)
			ppm_Out_ChannelPulseIndex = -1;                            // back to SYNC pulse
	}

	// *************************
}

void ppm_Out_Supervisor(void)
{	// this gets called once every millisecond by an interrupt

	if (booting || ppm_initialising)
		return;



}

// *************************************************************
// TIMER capture/compare interrupt

void PIOS_PPM_CC_IRQ_FUNC(void)
{
	if (ppm_mode == MODE_PPM_TX) PIOS_PPM_IN_CC_IRQ();
	else
	if (ppm_mode == MODE_PPM_RX) PIOS_PPM_OUT_CC_IRQ();
}

// *************************************************************
// can be called from an interrupt if you wish
// call this once every ms

void ppm_1ms_tick(void)
{
	if (booting || ppm_initialising)
		return;

	if (ppm_mode == MODE_PPM_TX) ppm_In_Supervisor();
	else
	if (ppm_mode == MODE_PPM_RX) ppm_Out_Supervisor();
}

// *************************************************************
// return a byte for the tx packet transmission.
//
// return value < 0 if no more bytes available, otherwise return byte to be sent

int16_t ppm_TxDataByteCallback(void)
{
	return -1;
}

// *************************************************************
// we are being given a block of received bytes
//
// return TRUE to continue current packet receive, otherwise return FALSE to halt current packet reception

bool ppm_RxDataCallback(void *data, uint8_t len)
{
	return true;
}

// *************************************************************
// call this from the main loop (not interrupt) as often as possible

void ppm_process(void)
{
	if (booting || ppm_initialising)
		return;

	if (ppm_mode == MODE_PPM_TX)
	{
		if (ppm_In_NewFrame() > 0)
		{	// we have a new PPM frame to send

			#ifdef PPM_DEBUG
				DEBUG_PRINTF("\r\n");
				DEBUG_PRINTF("ppm_in: sync %u\r\n", ppm_In_SyncPulseWidth);
			#endif

			for (int i = 0; i <	PIOS_PPM_MAX_CHANNELS && i < ppm_In_ChannelsDetected; i++)
			{
//				int32_t pwm = ppm_In_GetChannelPulseWidth(i);

				#ifdef PPM_DEBUG
					DEBUG_PRINTF("ppm_in: %u %u %4u\r\n", ppm_In_Frames, i, ppm_In_GetChannelPulseWidth(i));
				#endif

				// TODO:
			}
		}
	}
	else
	if (saved_settings.mode == MODE_PPM_RX)
	{
		// TODO:
	}
}

// *************************************************************

void ppm_deinit(void)
{
	ppm_initialising = true;

	// disable the PPM timer
	TIM_Cmd(PIOS_PPM_TIM, DISABLE);

	PIOS_PPM_TIMER_DIS_RCC_FUNC;

	// TIM IT disable
	TIM_ITConfig(PIOS_PPM_TIM, PIOS_PPM_IN_TIM_CCR | PIOS_PPM_OUT_TIM_CCR, DISABLE);

	// TIMER Main Output Disable
	TIM_CtrlPWMOutputs(PIOS_PPM_TIM, DISABLE);

	// un-remap the PPM pins
	GPIO_PinRemapConfig(GPIO_FullRemap_TIM2, DISABLE);

	// Disable timer interrupt
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = PIOS_IRQ_PRIO_MID;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = DISABLE;
	NVIC_InitStructure.NVIC_IRQChannel = PIOS_PPM_TIM_IRQ;
	NVIC_Init(&NVIC_InitStructure);

	ppm_initialising = false;
}

void ppm_init(uint32_t our_sn)
{
	ppm_initialising = true;

	#if defined(PPM_DEBUG)
		DEBUG_PRINTF("\r\nPPM init\r\n");
	#endif

	ppm_mode = saved_settings.mode;

	if (ppm_mode == MODE_PPM_TX)
	{
		ppm_In_Init();
		rfm22_init_tx_stream(saved_settings.min_frequency_Hz, saved_settings.max_frequency_Hz);
	}
	else
	if (ppm_mode == MODE_PPM_RX)
	{
		ppm_Out_Init();
		rfm22_init_rx_stream(saved_settings.min_frequency_Hz, saved_settings.max_frequency_Hz);
	}

	rfm22_TxDataByte_SetCallback(ppm_TxDataByteCallback);
	rfm22_RxData_SetCallback(ppm_RxDataCallback);

    rfm22_setFreqCalibration(saved_settings.rf_xtal_cap);
	rfm22_setNominalCarrierFrequency(saved_settings.frequency_Hz);
	rfm22_setDatarate(saved_settings.max_rf_bandwidth, FALSE);
	rfm22_setTxPower(saved_settings.max_tx_power);

	if (ppm_mode == MODE_PPM_TX)
		rfm22_setTxStream();

	ppm_initialising = false;
}

// *************************************************************

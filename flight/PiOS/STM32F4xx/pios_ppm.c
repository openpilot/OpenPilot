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

/*
 * @note This is a cleaned up and cut down version of the F1xx PPM input
 * module that uses a FreeRTOS task to handle the supervisor.
 *
 * @todo It would be cheaper (less stack involved) to use a timer, or
 * even better, a callout (but that would require implementing callouts).
 */

/* Project Includes */
#include <pios.h>
#include <pios_ppm_priv.h>
#include <timers.h>

#ifndef PIOS_INCLUDE_FREERTOS
# error PPM input requires FreeRTOS
#else
# if !configUSE_TIMERS
#  error PPM input requires FreeRTOS configUSE_TIMERS
# endif
#endif

/* Provide a RCVR driver */
static int32_t PIOS_PPM_Get(uint32_t rcvr_id, uint8_t channel);
static int32_t PIOS_PPM_Get_RSSI(uint32_t rcvr_id);

const struct pios_rcvr_driver pios_ppm_rcvr_driver = {
	.read = PIOS_PPM_Get,
	.rssi = PIOS_PPM_Get_RSSI
};

/* Local Variables */

static uint8_t PulseIndex;
static uint32_t PreviousValue;
static uint32_t CurrentValue;
static uint32_t CapturedValue;
static uint32_t CaptureValue[PIOS_PPM_NUM_INPUTS];
static uint32_t CapCounter[PIOS_PPM_NUM_INPUTS];
static uint16_t TimerCounter;
static uint8_t RSSI = 0;

static uint8_t SupervisorState = 0;
static uint32_t CapCounterPrev[PIOS_PPM_NUM_INPUTS];

static xTimerHandle ppmSupvTimer;
static void ppmSupvCallback(xTimerHandle xTimer);

/**
 * Do PPM input initialisation.
 */
void PIOS_PPM_Init(void)
{
	/* Enable timer interrupts */
	NVIC_Init((NVIC_InitTypeDef *)&pios_ppm_cfg.irq.init);

	/* Configure input pins */
	GPIO_Init(pios_ppm_cfg.port, (GPIO_InitTypeDef *)&pios_ppm_cfg.gpio_init);
	GPIO_PinAFConfig(pios_ppm_cfg.port, __builtin_ctz(pios_ppm_cfg.gpio_init.GPIO_Pin), pios_ppm_cfg.remap);

	/* Configure timer for input capture */
	TIM_ICInit(pios_ppm_cfg.timer, (TIM_ICInitTypeDef *)&pios_ppm_cfg.tim_ic_init);

	/* Configure timer clocks */
	TIM_InternalClockConfig(pios_ppm_cfg.timer);
	TIM_TimeBaseInit(pios_ppm_cfg.timer, (TIM_TimeBaseInitTypeDef *)&pios_ppm_cfg.tim_base_init);

	/* Enable the Capture Compare Interrupt Request */
	TIM_ITConfig(pios_ppm_cfg.timer, pios_ppm_cfg.ccr | TIM_IT_Update, ENABLE);

	/* Enable timer */
	TIM_Cmd(pios_ppm_cfg.timer, ENABLE);

	/* register the supervisor timer callout at 25Hz */
	ppmSupvTimer = xTimerCreate((signed char *)"ppmSupv", configTICK_RATE_HZ / 25, pdTRUE, NULL, ppmSupvCallback);
	if(xTimerStart(ppmSupvTimer, 0 ) != pdPASS)
	{
		PIOS_COM_SendFormattedString(PIOS_COM_DEBUG, "\nPPM TIMER START FAILED!\r\n", (ppmSupvTimer == NULL));
	}
}

/**
* Get the value of an input channel
* \param[in] Channel Number of the channel desired
* \output -1 Channel not available
* \output >0 Channel value
*/
static int32_t PIOS_PPM_Get(uint32_t rcvr_id, uint8_t channel)
{
	/* Return error if channel not available */
	if (channel >= PIOS_PPM_NUM_INPUTS) {
		return -1;
	}
	return CaptureValue[channel];
}

/**
* Get the receive signal strength
* \param[in] Channel Number of the channel desired
* \output 0 No signal
* \output >0-255 signal quality
*/
static int32_t PIOS_PPM_Get_RSSI(uint32_t rcvr_id)
{
	return RSSI;
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

static void
ppmSupvCallback(xTimerHandle xTimer)
{
	for (;;) {
		/* we should receive a PPM frame at least once every 80 ms (detection takes up to 160 ms) */
		vTaskDelay(80 / portTICK_RATE_MS);

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
			bool signalLostCount = 0; // If no channel was triggered, we lost signal
			for (int32_t i = 0; i < PIOS_PPM_NUM_INPUTS; i++) {
				if (CapCounter[i] == CapCounterPrev[i]) {
					CaptureValue[i] = 0;
					signalLostCount++;
				}
			}

			// If more than 33% of the configured channels could not be captured
			// assume an error state. This allows to use 4 channel systems
			// to be used with 8 configured input channels
			if (signalLostCount > PIOS_PPM_NUM_INPUTS/3) {
				RSSI = 0;
			} else {
				RSSI = 255;
			}

			/* Move to next state */
			SupervisorState = 0;
		}
	}
}

/**
  * @}
  * @}
  */

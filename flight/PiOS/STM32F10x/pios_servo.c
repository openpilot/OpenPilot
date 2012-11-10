/**
 ******************************************************************************
 * @addtogroup PIOS PIOS Core hardware abstraction layer
 * @{
 * @addtogroup   PIOS_SERVO RC Servo Functions
 * @brief Code to do set RC servo output
 * @{
 *
 * @file       pios_servo.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      RC Servo routines (STM32 dependent)
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
#include "pios_servo_priv.h"
#include "pios_tim_priv.h"

/* Private Function Prototypes */

static const struct pios_servo_cfg * servo_cfg;

#if defined(PIOS_STEPPER)
struct pios_pwm_out_dev {
	const struct pios_servo_cfg servo_cfg;
	int32_t StepRemaining;//3 first axes
	uint32_t OldPosition;//3 first axes
	bool Direction;

};

static struct pios_pwm_out_dev * stepper_cfg;

static void PIOS_STEPPER_tim_edge_cb (uint32_t id, uint32_t context, uint8_t channel, uint16_t count);
const static struct pios_tim_callbacks tim_callbacks = {
	.overflow = NULL,
	.edge     = PIOS_STEPPER_tim_edge_cb,
};
#endif

/**
* Initialise Servos
*/
int32_t PIOS_Servo_Init(const struct pios_servo_cfg * cfg)
{
	uint32_t tim_id;
#if defined(PIOS_STEPPER)
	if (PIOS_TIM_InitChannels(&tim_id, cfg->channels, cfg->num_channels, NULL, 0)) {
		return -1;
	}
#else
	if (PIOS_TIM_InitChannels(&tim_id, cfg->channels, cfg->num_channels, NULL, 0)) {
		return -1;
	}
#endif

	/* Store away the requested configuration */
	servo_cfg = cfg;

	/* Configure the channels to be in output compare mode */
	for (uint8_t i = 0; i < cfg->num_channels; i++) {
		const struct pios_tim_channel * chan = &cfg->channels[i];

		/* Set up for output compare function */
		switch(chan->timer_chan) {
			case TIM_Channel_1:
				TIM_OC1Init(chan->timer, &cfg->tim_oc_init);
				TIM_OC1PreloadConfig(chan->timer, TIM_OCPreload_Enable);
				break;
			case TIM_Channel_2:
				TIM_OC2Init(chan->timer, &cfg->tim_oc_init);
				TIM_OC2PreloadConfig(chan->timer, TIM_OCPreload_Enable);
				break;
			case TIM_Channel_3:
				TIM_OC3Init(chan->timer, &cfg->tim_oc_init);
				TIM_OC3PreloadConfig(chan->timer, TIM_OCPreload_Enable);
				break;
			case TIM_Channel_4:
				TIM_OC4Init(chan->timer, &cfg->tim_oc_init);
				TIM_OC4PreloadConfig(chan->timer, TIM_OCPreload_Enable);
				break;
		}

		TIM_ARRPreloadConfig(chan->timer, ENABLE);
		TIM_CtrlPWMOutputs(chan->timer, ENABLE);
		TIM_Cmd(chan->timer, ENABLE);
	}

	return 0;
}

/**
* Set the servo update rate (Max 500Hz)
* \param[in] array of rates in Hz
* \param[in] maximum number of banks
*/
void PIOS_Servo_SetHz(const uint16_t * speeds, uint8_t banks)
{
	if (!servo_cfg) {
		return;
	}

	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure = servo_cfg->tim_base_init;
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseStructure.TIM_Prescaler = (PIOS_MASTER_CLOCK / 1000000) - 1;

	uint8_t set = 0;

	for(uint8_t i = 0; (i < servo_cfg->num_channels) && (set < banks); i++) {
		bool new = true;
		const struct pios_tim_channel * chan = &servo_cfg->channels[i];

		/* See if any previous channels use that same timer */
		for(uint8_t j = 0; (j < i) && new; j++)
			new &= chan->timer != servo_cfg->channels[j].timer;

		if(new) {
			TIM_TimeBaseStructure.TIM_Period = ((1000000 / speeds[set]) - 1);
			TIM_TimeBaseInit(chan->timer, &TIM_TimeBaseStructure);
			set++;
		}
	}
}

/**
* Set servo position
* \param[in] Servo Servo number (0-7)
* \param[in] Position Servo position in microseconds
*/
void PIOS_Servo_Set(uint8_t servo, uint16_t position)
{
	/* Make sure servo exists */
	if (!servo_cfg || servo >= servo_cfg->num_channels) {
		return;
	}

#if defined(PIOS_STEPPER)
	if(servo<3)
	{
		servo_cfg.StepRemaining[servo]=position-servo_cfg.OldPosition[servo];
		const struct pios_tim_channel * chan = &servo_cfg->channels[servo];
		if(servo_cfg.StepRemaining[servo]!=0)
		{
		/* Update the position */
			switch(chan->timer_chan) {
				case TIM_Channel_1:
					TIM_SetCompare1(chan->timer, 1000);
					break;
				case TIM_Channel_2:
					TIM_SetCompare2(chan->timer, 1000);
					break;
				case TIM_Channel_3:
					TIM_SetCompare3(chan->timer, 1000);
					break;
				case TIM_Channel_4:
					TIM_SetCompare4(chan->timer, 1000);
					break;
			}
		}
		else
		{
		/* Update the position */
			switch(chan->timer_chan) {
				case TIM_Channel_1:
					TIM_SetCompare1(chan->timer, 0);
					break;
				case TIM_Channel_2:
					TIM_SetCompare2(chan->timer, 0);
					break;
				case TIM_Channel_3:
					TIM_SetCompare3(chan->timer, 0);
					break;
				case TIM_Channel_4:
					TIM_SetCompare4(chan->timer, 0);
					break;
			}
		}
		if(servo_cfg.StepRemaining[servo]<0)
		{
			servo_cfg.StepRemaining[servo]=-servo_cfg.StepRemaining[servo];
			chan = &servo_cfg->channels[servo+3];
			switch(chan->timer_chan) {
				case TIM_Channel_1:
					TIM_SetCompare1(chan->timer, 2000);
					break;
				case TIM_Channel_2:
					TIM_SetCompare2(chan->timer, 2000);
					break;
				case TIM_Channel_3:
					TIM_SetCompare3(chan->timer, 2000);
					break;
				case TIM_Channel_4:
					TIM_SetCompare4(chan->timer, 2000);
					break;
			}
		}
		else
		{
			chan = &servo_cfg->channels[servo+3];
			switch(chan->timer_chan) {
				case TIM_Channel_1:
					TIM_SetCompare1(chan->timer, 0);
					break;
				case TIM_Channel_2:
					TIM_SetCompare2(chan->timer, 0);
					break;
				case TIM_Channel_3:
					TIM_SetCompare3(chan->timer, 0);
					break;
				case TIM_Channel_4:
					TIM_SetCompare4(chan->timer, 0);
					break;
			}
		}
	}
#else
	/* Update the position */
	const struct pios_tim_channel * chan = &servo_cfg->channels[servo];
	switch(chan->timer_chan) {
		case TIM_Channel_1:
			TIM_SetCompare1(chan->timer, position);
			break;
		case TIM_Channel_2:
			TIM_SetCompare2(chan->timer, position);
			break;
		case TIM_Channel_3:
			TIM_SetCompare3(chan->timer, position);
			break;
		case TIM_Channel_4:
			TIM_SetCompare4(chan->timer, position);
			break;
	}
#endif
}

#if defined(PIOS_STEPPER)
static void PIOS_STEPPER_tim_edge_cb (uint32_t tim_id, uint32_t context, uint8_t chan_idx, uint16_t count)
{
	/* Recover our device context */
	struct pios_pwm_dev * pwm_dev = (struct pios_pwm_dev *)context;

	if (!PIOS_PWM_validate(pwm_dev)) {
		/* Invalid device specified */
		return;
	}

	if (chan_idx >= pwm_dev->cfg->num_channels) {
		/* Channel out of range */
		return;
	}

	const struct pios_tim_channel * chan = &pwm_dev->cfg->channels[chan_idx];

	if (pwm_dev->CaptureState[chan_idx] == 0) {
		pwm_dev->RiseValue[chan_idx] = count;
		pwm_dev->us_since_update[chan_idx] = 0;
	} else {
		pwm_dev->FallValue[chan_idx] = count;
	}

	// flip state machine and capture value here
	/* Simple rise or fall state machine */
	TIM_ICInitTypeDef TIM_ICInitStructure = pwm_dev->cfg->tim_ic_init;
	if (pwm_dev->CaptureState[chan_idx] == 0) {
		/* Switch states */
		pwm_dev->CaptureState[chan_idx] = 1;

		/* Switch polarity of input capture */
		TIM_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Falling;
		TIM_ICInitStructure.TIM_Channel = chan->timer_chan;
		TIM_ICInit(chan->timer, &TIM_ICInitStructure);
	} else {
		/* Capture computation */
		if (pwm_dev->FallValue[chan_idx] > pwm_dev->RiseValue[chan_idx]) {
			pwm_dev->CaptureValue[chan_idx] = (pwm_dev->FallValue[chan_idx] - pwm_dev->RiseValue[chan_idx]);
		} else {
			pwm_dev->CaptureValue[chan_idx] = ((chan->timer->ARR - pwm_dev->RiseValue[chan_idx]) + pwm_dev->FallValue[chan_idx]);
		}

		/* Switch states */
		pwm_dev->CaptureState[chan_idx] = 0;

		/* Increase supervisor counter */
		pwm_dev->CapCounter[chan_idx]++;

		/* Switch polarity of input capture */
		TIM_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Rising;
		TIM_ICInitStructure.TIM_Channel = chan->timer_chan;
		TIM_ICInit(chan->timer, &TIM_ICInitStructure);
	}

}
#endif

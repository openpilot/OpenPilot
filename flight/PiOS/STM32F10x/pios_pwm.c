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
static int32_t PIOS_PWM_Get(uint32_t rcvr_id, uint8_t channel);

const struct pios_rcvr_driver pios_pwm_rcvr_driver = {
	.read = PIOS_PWM_Get,
};

/* Local Variables */
/* 100 ms timeout without updates on channels */
const static uint32_t PWM_SUPERVISOR_TIMEOUT = 100000;

enum pios_pwm_dev_magic {
	PIOS_PWM_DEV_MAGIC = 0xab30293c,
};

struct pios_pwm_dev {
	enum pios_pwm_dev_magic     magic;
	const struct pios_pwm_cfg * cfg;

	uint8_t CaptureState[PIOS_PWM_NUM_INPUTS];
	uint16_t RiseValue[PIOS_PWM_NUM_INPUTS];
	uint16_t FallValue[PIOS_PWM_NUM_INPUTS];
	uint32_t CaptureValue[PIOS_PWM_NUM_INPUTS];
	uint32_t CapCounter[PIOS_PWM_NUM_INPUTS];
	uint32_t us_since_update[PIOS_PWM_NUM_INPUTS];
};

static bool PIOS_PWM_validate(struct pios_pwm_dev * pwm_dev)
{
	return (pwm_dev->magic == PIOS_PWM_DEV_MAGIC);
}

#if defined(PIOS_INCLUDE_FREERTOS)
static struct pios_pwm_dev * PIOS_PWM_alloc(void)
{
	struct pios_pwm_dev * pwm_dev;

	pwm_dev = (struct pios_pwm_dev *)pvPortMalloc(sizeof(*pwm_dev));
	if (!pwm_dev) return(NULL);

	pwm_dev->magic = PIOS_PWM_DEV_MAGIC;
	return(pwm_dev);
}
#else
static struct pios_pwm_dev pios_pwm_devs[PIOS_PWM_MAX_DEVS];
static uint8_t pios_pwm_num_devs;
static struct pios_pwm_dev * PIOS_PWM_alloc(void)
{
	struct pios_pwm_dev * pwm_dev;

	if (pios_pwm_num_devs >= PIOS_PWM_MAX_DEVS) {
		return (NULL);
	}

	pwm_dev = &pios_pwm_devs[pios_pwm_num_devs++];
	pwm_dev->magic = PIOS_PWM_DEV_MAGIC;

	return (pwm_dev);
}
#endif

static void PIOS_PWM_tim_overflow_cb (uint32_t id, uint32_t context, uint8_t channel, uint16_t count);
static void PIOS_PWM_tim_edge_cb (uint32_t id, uint32_t context, uint8_t channel, uint16_t count);
const static struct pios_tim_callbacks tim_callbacks = {
	.overflow = PIOS_PWM_tim_overflow_cb,
	.edge     = PIOS_PWM_tim_edge_cb,
};

/**
* Initialises all the pins
*/
int32_t PIOS_PWM_Init(uint32_t * pwm_id, const struct pios_pwm_cfg * cfg)
{
	PIOS_DEBUG_Assert(pwm_id);
	PIOS_DEBUG_Assert(cfg);

	struct pios_pwm_dev * pwm_dev;

	pwm_dev = (struct pios_pwm_dev *) PIOS_PWM_alloc();
	if (!pwm_dev) goto out_fail;

	/* Bind the configuration to the device instance */
	pwm_dev->cfg = cfg;

	for (uint8_t i = 0; i < PIOS_PWM_NUM_INPUTS; i++) {
		/* Flush counter variables */
		pwm_dev->CaptureState[i] = 0;
		pwm_dev->RiseValue[i] = 0;
		pwm_dev->FallValue[i] = 0;
		pwm_dev->CaptureValue[i] = PIOS_RCVR_TIMEOUT;
	}

	uint32_t tim_id;
	if (PIOS_TIM_InitChannels(&tim_id, cfg->channels, cfg->num_channels, &tim_callbacks, (uint32_t)pwm_dev)) {
		return -1;
	}

	/* Configure the channels to be in capture/compare mode */
	for (uint8_t i = 0; i < cfg->num_channels; i++) {
		const struct pios_tim_channel * chan = &cfg->channels[i];

		/* Configure timer for input capture */
		TIM_ICInitTypeDef TIM_ICInitStructure = cfg->tim_ic_init;
		TIM_ICInitStructure.TIM_Channel = chan->timer_chan;
		TIM_ICInit(chan->timer, &TIM_ICInitStructure);
		
		/* Enable the Capture Compare Interrupt Request */
		switch (chan->timer_chan) {
		case TIM_Channel_1:
			TIM_ITConfig(chan->timer, TIM_IT_CC1, ENABLE);
			break;
		case TIM_Channel_2:
			TIM_ITConfig(chan->timer, TIM_IT_CC2, ENABLE);
			break;
		case TIM_Channel_3:
			TIM_ITConfig(chan->timer, TIM_IT_CC3, ENABLE);
			break;
		case TIM_Channel_4:
			TIM_ITConfig(chan->timer, TIM_IT_CC4, ENABLE);
			break;
		}

		// Need the update event for that timer to detect timeouts
		TIM_ITConfig(chan->timer, TIM_IT_Update, ENABLE);

	}

	*pwm_id = (uint32_t) pwm_dev;

	return (0);

out_fail:
	return (-1);
}

/**
* Get the value of an input channel
* \param[in] Channel Number of the channel desired
* \output -1 Channel not available
* \output >0 Channel value
*/
static int32_t PIOS_PWM_Get(uint32_t rcvr_id, uint8_t channel)
{
	struct pios_pwm_dev * pwm_dev = (struct pios_pwm_dev *)rcvr_id;

	if (!PIOS_PWM_validate(pwm_dev)) {
		/* Invalid device specified */
		return PIOS_RCVR_INVALID;
	}

	if (channel >= PIOS_PWM_NUM_INPUTS) {
		/* Channel out of range */
		return PIOS_RCVR_INVALID;
	}
	return pwm_dev->CaptureValue[channel];
}

static void PIOS_PWM_tim_overflow_cb (uint32_t tim_id, uint32_t context, uint8_t channel, uint16_t count)
{
	struct pios_pwm_dev * pwm_dev = (struct pios_pwm_dev *)context;

	if (!PIOS_PWM_validate(pwm_dev)) {
		/* Invalid device specified */
		return;
	}

	if (channel >= pwm_dev->cfg->num_channels) {
		/* Channel out of range */
		return;
	}

	pwm_dev->us_since_update[channel] += count;
	if(pwm_dev->us_since_update[channel] >= PWM_SUPERVISOR_TIMEOUT) {
		pwm_dev->CaptureState[channel] = 0;
		pwm_dev->RiseValue[channel] = 0;
		pwm_dev->FallValue[channel] = 0;
		pwm_dev->CaptureValue[channel] = PIOS_RCVR_TIMEOUT;
		pwm_dev->us_since_update[channel] = 0;
	}

	return;
}

static void PIOS_PWM_tim_edge_cb (uint32_t tim_id, uint32_t context, uint8_t chan_idx, uint16_t count)
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

/** 
  * @}
  * @}
  */

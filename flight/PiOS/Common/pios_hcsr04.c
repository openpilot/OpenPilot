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
#include "pios_hcsr04_priv.h"

#if defined(PIOS_INCLUDE_HCSR04)
//------------------------------------------

/* Local Variables */
/* 100 ms timeout without updates on channels */
const static uint32_t PWM_SUPERVISOR_TIMEOUT = 100000;

struct pios_hcsr04_dev * hcsr04_dev_loc;

enum pios_hcsr04_dev_magic {
	PIOS_HCSR04_DEV_MAGIC = 0xab3029AA,
};

struct pios_hcsr04_dev {
	enum pios_hcsr04_dev_magic     magic;
	const struct pios_hcsr04_cfg * cfg;

	uint8_t CaptureState[PIOS_PWM_NUM_INPUTS];
	uint16_t RiseValue[PIOS_PWM_NUM_INPUTS];
	uint16_t FallValue[PIOS_PWM_NUM_INPUTS];
	uint32_t CaptureValue[PIOS_PWM_NUM_INPUTS];
	uint32_t CapCounter[PIOS_PWM_NUM_INPUTS];
	uint32_t us_since_update[PIOS_PWM_NUM_INPUTS];
};

static bool PIOS_HCSR04_validate(struct pios_hcsr04_dev * hcsr04_dev)
{
	return (hcsr04_dev->magic == PIOS_HCSR04_DEV_MAGIC);
}

#if defined(PIOS_INCLUDE_FREERTOS)
static struct pios_hcsr04_dev * PIOS_PWM_alloc(void)
{
	struct pios_hcsr04_dev * hcsr04_dev;

	hcsr04_dev = (struct pios_hcsr04_dev *)pvPortMalloc(sizeof(*hcsr04_dev));
	if (!hcsr04_dev) return(NULL);

	hcsr04_dev->magic = PIOS_HCSR04_DEV_MAGIC;
	return(hcsr04_dev);
}
#else
static struct pios_hcsr04_dev pios_hcsr04_devs[PIOS_PWM_MAX_DEVS];
static uint8_t pios_hcsr04_num_devs;
static struct pios_hcsr04_dev * PIOS_PWM_alloc(void)
{
	struct pios_hcsr04_dev * hcsr04_dev;

	if (pios_pwm_num_devs >= PIOS_PWM_MAX_DEVS) {
		return (NULL);
	}

	hcsr04_dev = &pios_hcsr04_devs[pios_hcsr04_num_devs++];
	hcsr04_dev->magic = PIOS_HCSR04_DEV_MAGIC;

	return (hcsr04_dev);
}
#endif

static void PIOS_HCSR04_tim_overflow_cb (uint32_t id, uint32_t context, uint8_t channel, uint16_t count);
static void PIOS_HCSR04_tim_edge_cb (uint32_t id, uint32_t context, uint8_t channel, uint16_t count);
const static struct pios_tim_callbacks tim_callbacks = {
	.overflow = PIOS_HCSR04_tim_overflow_cb,
	.edge     = PIOS_HCSR04_tim_edge_cb,
};


/**
* Initialises all the pins
*/
int32_t PIOS_HCSR04_Init(uint32_t * pwm_id, const struct pios_hcsr04_cfg * cfg)
{
	PIOS_DEBUG_Assert(pwm_id);
	PIOS_DEBUG_Assert(cfg);

	struct pios_hcsr04_dev * hcsr04_dev;

	hcsr04_dev = (struct pios_hcsr04_dev *) PIOS_PWM_alloc();
	if (!hcsr04_dev) goto out_fail;

	/* Bind the configuration to the device instance */
	hcsr04_dev->cfg = cfg;
	hcsr04_dev_loc = hcsr04_dev;

	for (uint8_t i = 0; i < PIOS_PWM_NUM_INPUTS; i++) {
		/* Flush counter variables */
		hcsr04_dev->CaptureState[i] = 0;
		hcsr04_dev->RiseValue[i] = 0;
		hcsr04_dev->FallValue[i] = 0;
		hcsr04_dev->CaptureValue[i] = PIOS_RCVR_TIMEOUT;
	}

	uint32_t tim_id;
	if (PIOS_TIM_InitChannels(&tim_id, cfg->channels, cfg->num_channels, &tim_callbacks, (uint32_t)hcsr04_dev)) {
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

#ifndef STM32F4XX
	/* Enable the peripheral clock for the GPIO */
	switch ((uint32_t)hcsr04_dev->cfg->trigger.gpio) {
	case (uint32_t) GPIOA:
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
		break;
	case (uint32_t) GPIOB:
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
		break;
	case (uint32_t) GPIOC:
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
		break;
	default:
		PIOS_Assert(0);
		break;
	}
#endif
	GPIO_Init(hcsr04_dev->cfg->trigger.gpio, &hcsr04_dev->cfg->trigger.init);

	*pwm_id = (uint32_t) hcsr04_dev;

	return (0);

out_fail:
	return (-1);
}

void PIOS_HCSR04_Trigger(void)
{
	GPIO_SetBits(hcsr04_dev_loc->cfg->trigger.gpio,hcsr04_dev_loc->cfg->trigger.init.GPIO_Pin);
	PIOS_DELAY_WaituS(15);
	GPIO_ResetBits(hcsr04_dev_loc->cfg->trigger.gpio,hcsr04_dev_loc->cfg->trigger.init.GPIO_Pin);
}

/**
* Get the value of an input channel
* \param[in] Channel Number of the channel desired
* \output -1 Channel not available
* \output >0 Channel value
*/
int32_t PIOS_HCSR04_Get(void)
{
	return hcsr04_dev_loc->CaptureValue[0];
}

int32_t PIOS_HCSR04_Completed(void)
{
	return hcsr04_dev_loc->CapCounter[0];
}

static void PIOS_HCSR04_tim_overflow_cb (uint32_t tim_id, uint32_t context, uint8_t channel, uint16_t count)
{
	struct pios_hcsr04_dev * hcsr04_dev = (struct pios_hcsr04_dev *)context;

	if (!PIOS_HCSR04_validate(hcsr04_dev)) {
		/* Invalid device specified */
		return;
	}

	if (channel >= hcsr04_dev->cfg->num_channels) {
		/* Channel out of range */
		return;
	}

	hcsr04_dev->us_since_update[channel] += count;
	if(hcsr04_dev->us_since_update[channel] >= PWM_SUPERVISOR_TIMEOUT) {
		hcsr04_dev->CaptureState[channel] = 0;
		hcsr04_dev->RiseValue[channel] = 0;
		hcsr04_dev->FallValue[channel] = 0;
		hcsr04_dev->CaptureValue[channel] = PIOS_RCVR_TIMEOUT;
		hcsr04_dev->us_since_update[channel] = 0;
	}

	return;
}

static void PIOS_HCSR04_tim_edge_cb (uint32_t tim_id, uint32_t context, uint8_t chan_idx, uint16_t count)
{
	/* Recover our device context */
	struct pios_hcsr04_dev * hcsr04_dev = (struct pios_hcsr04_dev *)context;

	if (!PIOS_HCSR04_validate(hcsr04_dev)) {
		/* Invalid device specified */
		return;
	}

	if (chan_idx >= hcsr04_dev->cfg->num_channels) {
		/* Channel out of range */
		return;
	}

	const struct pios_tim_channel * chan = &hcsr04_dev->cfg->channels[chan_idx];

	if (hcsr04_dev->CaptureState[chan_idx] == 0) {
		hcsr04_dev->RiseValue[chan_idx] = count;
		hcsr04_dev->us_since_update[chan_idx] = 0;
	} else {
		hcsr04_dev->FallValue[chan_idx] = count;
	}

	// flip state machine and capture value here
	/* Simple rise or fall state machine */
	TIM_ICInitTypeDef TIM_ICInitStructure = hcsr04_dev->cfg->tim_ic_init;
	if (hcsr04_dev->CaptureState[chan_idx] == 0) {
		/* Switch states */
		hcsr04_dev->CaptureState[chan_idx] = 1;

		/* Switch polarity of input capture */
		TIM_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Falling;
		TIM_ICInitStructure.TIM_Channel = chan->timer_chan;
		TIM_ICInit(chan->timer, &TIM_ICInitStructure);
	} else {
		/* Capture computation */
		if (hcsr04_dev->FallValue[chan_idx] > hcsr04_dev->RiseValue[chan_idx]) {
			hcsr04_dev->CaptureValue[chan_idx] = (hcsr04_dev->FallValue[chan_idx] - hcsr04_dev->RiseValue[chan_idx]);
		} else {
			hcsr04_dev->CaptureValue[chan_idx] = ((chan->timer->ARR - hcsr04_dev->RiseValue[chan_idx]) + hcsr04_dev->FallValue[chan_idx]);
		}

		/* Switch states */
		hcsr04_dev->CaptureState[chan_idx] = 0;

		/* Increase supervisor counter */
		hcsr04_dev->CapCounter[chan_idx]++;

		/* Switch polarity of input capture */
		TIM_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Rising;
		TIM_ICInitStructure.TIM_Channel = chan->timer_chan;
		TIM_ICInit(chan->timer, &TIM_ICInitStructure);
	}

}


#endif

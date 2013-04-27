/**
 ******************************************************************************
 * @addtogroup PIOS PIOS Core hardware abstraction layer
 * @{
 * @addtogroup   PIOS_PPM PPM Output Functions
 * @brief Code to output a PPM receiver signal
 * @{
 *
 * @file       pios_ppm_out.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2012.
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

#include "pios.h"

#ifdef PIOS_INCLUDE_PPM_OUT

#include "pios_ppm_out_priv.h"

#define PIOS_PPM_OUT_MAX_DEVS               1
#define PIOS_PPM_OUT_MAX_CHANNELS           8
#define PIOS_PPM_OUT_FRAME_PERIOD_US        22500  // microseconds
#define PIOS_PPM_OUT_HIGH_PULSE_US          400    // microseconds
#define PIOS_PPM_OUT_MIN_CHANNEL_PULSE_US   1000   // microseconds
#define PIOS_PPM_OUT_MAX_CHANNEL_PULSE_US   2000   // microseconds

enum pios_ppm_out_dev_magic {
	PIOS_PPM_OUT_DEV_MAGIC = 0x0e0210e2
};

struct pios_ppm_out_dev {
	enum pios_ppm_out_dev_magic magic;
	const struct pios_ppm_out_cfg * cfg;

	uint32_t triggering_period;
	uint32_t ChannelSum;
	uint8_t NumChannelCounter;
	uint16_t ChannelValue[PIOS_PPM_OUT_MAX_CHANNELS];

	uint8_t supv_timer;
	bool Tracking;
	bool Fresh;
};

static void PIOS_PPM_Out_Supervisor(uint32_t ppm_id);
static void PIOS_PPM_Out_Enable_Disable(struct pios_ppm_out_dev *ppm_dev, bool enable);

static bool PIOS_PPM_Out_validate(struct pios_ppm_out_dev *ppm_dev)
{
	return (ppm_dev->magic == PIOS_PPM_OUT_DEV_MAGIC);
}

#if defined(PIOS_INCLUDE_FREERTOS)
static struct pios_ppm_out_dev * PIOS_PPM_OUT_alloc(void)
{
	struct pios_ppm_out_dev * ppm_dev;

	ppm_dev = (struct pios_ppm_out_dev *)pvPortMalloc(sizeof(*ppm_dev));
	if (!ppm_dev) return(NULL);

	ppm_dev->magic = PIOS_PPM_OUT_DEV_MAGIC;
	return(ppm_dev);
}
#else
static struct pios_ppm_out_dev pios_ppm_out_devs[PIOS_PPM_OUT_MAX_DEVS];
static uint8_t pios_ppm_out_num_devs;
static struct pios_ppm_out_dev * PIOS_PPM_alloc(void)
{
	struct pios_ppm_out_dev * ppm_dev;

	if (pios_ppm_out_num_devs >= PIOS_PPM_OUT_MAX_DEVS) {
		return (NULL);
	}

	ppm_dev = &pios_ppm_out_devs[pios_ppm_out_num_devs++];
	ppm_dev->magic = PIOS_PPM_OUT_DEV_MAGIC;

	return (ppm_dev);
}
#endif

static void PIOS_PPM_OUT_tim_edge_cb (uint32_t tim_id, uint32_t context, uint8_t chan_idx, uint16_t count);
const static struct pios_tim_callbacks tim_out_callbacks = {
	.overflow = NULL,
	.edge     = PIOS_PPM_OUT_tim_edge_cb,
};

int32_t PIOS_PPM_Out_Init(uint32_t *ppm_out_id, const struct pios_ppm_out_cfg * cfg)
{
	PIOS_DEBUG_Assert(ppm_id);
	PIOS_DEBUG_Assert(cfg);

	// Allocate the device structure
	struct pios_ppm_out_dev *ppm_dev = (struct pios_ppm_out_dev *)PIOS_PPM_OUT_alloc();
	if (!ppm_dev)
		return -1;
	ppm_dev->magic = PIOS_PPM_OUT_DEV_MAGIC;
	*ppm_out_id = (uint32_t)ppm_dev;

	// Bind the configuration to the device instance
	ppm_dev->cfg = cfg;

	// Set up the state variables
	ppm_dev->triggering_period = PIOS_PPM_OUT_HIGH_PULSE_US;
	ppm_dev->ChannelSum = 0;
	ppm_dev->NumChannelCounter = 0;

	// Flush counter variables
	for (uint8_t i = 0; i < PIOS_PPM_OUT_MAX_CHANNELS; i++)
		ppm_dev->ChannelValue[i] = 1000;

	uint32_t tim_id;
	if (PIOS_TIM_InitChannels(&tim_id, cfg->channel, 1, &tim_out_callbacks, (uint32_t)ppm_dev))
		return -1;

	// Configure the channels to be in output compare mode
	const struct pios_tim_channel *chan = cfg->channel;

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

	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseStructure.TIM_Prescaler = (PIOS_MASTER_CLOCK / 1000000) - 1;
	TIM_TimeBaseStructure.TIM_Period = ((1000000 / 100) - 1);
	TIM_TimeBaseInit(chan->timer, &TIM_TimeBaseStructure);
        PIOS_PPM_Out_Enable_Disable(ppm_dev, false);

        // Configure the supervisor
	ppm_dev->supv_timer = 0;
	ppm_dev->Fresh = FALSE;
	ppm_dev->Tracking = FALSE;
	if (!PIOS_RTC_RegisterTickCallback(PIOS_PPM_Out_Supervisor, (uint32_t)ppm_dev)) {
		PIOS_DEBUG_Assert(0);
	}

	return 0;
}

void PIOS_PPM_OUT_Set(uint32_t ppm_out_id, uint8_t servo, uint16_t position)
{
	struct pios_ppm_out_dev *ppm_dev = (struct pios_ppm_out_dev *)ppm_out_id;
	if (!PIOS_PPM_Out_validate(ppm_dev) || (servo >= PIOS_PPM_OUT_MAX_CHANNELS))
		return;

	// Don't allow positions that are out of range.
	if (position < PIOS_PPM_OUT_MIN_CHANNEL_PULSE_US)
		position = PIOS_PPM_OUT_MIN_CHANNEL_PULSE_US;
	if (position > PIOS_PPM_OUT_MAX_CHANNEL_PULSE_US)
		position = PIOS_PPM_OUT_MAX_CHANNEL_PULSE_US;

        // Update the supervisor tracking variables.
	ppm_dev->Fresh = TRUE;

        // Reenable the TIM if it's been turned off.
        if (!ppm_dev->Tracking) {
                PIOS_PPM_Out_Enable_Disable(ppm_dev, true);
        }
	
	// Update the position
	ppm_dev->ChannelValue[servo] = position;
}

static void PIOS_PPM_OUT_tim_edge_cb (uint32_t tim_id, uint32_t context, uint8_t chan_idx, uint16_t count)
{
	struct pios_ppm_out_dev *ppm_dev = (struct pios_ppm_out_dev *)context;
	if (!PIOS_PPM_Out_validate(ppm_dev))
		return;

	// Finish out the frame if we reached the last channel.
	uint32_t pulse_width;
	if ((ppm_dev->NumChannelCounter >= PIOS_PPM_OUT_MAX_CHANNELS)) {
		pulse_width = PIOS_PPM_OUT_FRAME_PERIOD_US - ppm_dev->ChannelSum;
		ppm_dev->NumChannelCounter = 0;
		ppm_dev->ChannelSum = 0;

                // Have we not received a sample for the supervisor timeout.
                if (!ppm_dev->Tracking) {
                        // Flush counter variables
                        for (uint8_t i = 0; i < PIOS_PPM_OUT_MAX_CHANNELS; i++)
                                ppm_dev->ChannelValue[i] = 1000;
                        PIOS_PPM_Out_Enable_Disable(ppm_dev, false);
                }
	} else
		ppm_dev->ChannelSum += (pulse_width = ppm_dev->ChannelValue[ppm_dev->NumChannelCounter++]);

	// Initiate the pulse
	TIM_SetAutoreload(ppm_dev->cfg->channel->timer, pulse_width - 1);

	return;
}

static void PIOS_PPM_Out_Enable_Disable(struct pios_ppm_out_dev *ppm_dev, bool enable)
{
	const struct pios_tim_channel *chan = ppm_dev->cfg->channel;
        uint32_t trig = enable ? ppm_dev->triggering_period : 0;
        FunctionalState state = enable ? ENABLE : DISABLE;
        switch (chan->timer_chan) {
        case TIM_Channel_1:
                TIM_ITConfig(chan->timer, TIM_IT_CC1 | TIM_IT_Update, state);
                TIM_SetCompare1(chan->timer, trig);
                break;
        case TIM_Channel_2:
                TIM_ITConfig(chan->timer, TIM_IT_CC2 | TIM_IT_Update, state);
                TIM_SetCompare2(chan->timer, trig);
                break;
        case TIM_Channel_3:
                TIM_ITConfig(chan->timer, TIM_IT_CC3 | TIM_IT_Update, state);
                TIM_SetCompare3(chan->timer, trig);
                break;
        case TIM_Channel_4:
                TIM_ITConfig(chan->timer, TIM_IT_CC4 | TIM_IT_Update, state);
                TIM_SetCompare4(chan->timer, trig);
                break;
        }
}

static void PIOS_PPM_Out_Supervisor(uint32_t ppm_out_id) {
	struct pios_ppm_out_dev *ppm_dev = (struct pios_ppm_out_dev *)ppm_out_id;
	if (!PIOS_PPM_Out_validate(ppm_dev))
		return;

	/* 
	 * RTC runs at 625Hz so divide down the base rate so that this loop runs at 25Hz.
	 */
	if(++(ppm_dev->supv_timer) < 25) {
		return;
	}
	ppm_dev->supv_timer = 0;

	if (!ppm_dev->Fresh) {
		ppm_dev->Tracking = FALSE;
	}

	ppm_dev->Fresh = FALSE;
}

#endif /* PIOS_INCLUDE_PPM_OUT */

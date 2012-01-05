/**
 ******************************************************************************
 * @addtogroup PIOS PIOS Core hardware abstraction layer
 * @{
 * @addtogroup   PIOS_PPM PPM Input Functions
 * @brief Code to measure PPM input and seperate into channels
 * @{
 *
 * @file       pios_ppm.c
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

/* Project Includes */
#include "pios.h"
#include "pios_ppm_priv.h"

#if defined(PIOS_INCLUDE_PPM)

/* Provide a RCVR driver */
static int32_t PIOS_PPM_Get(uint32_t rcvr_id, uint8_t channel);

const struct pios_rcvr_driver pios_ppm_rcvr_driver = {
	.read = PIOS_PPM_Get,
};

#define PIOS_PPM_IN_MIN_NUM_CHANNELS		4
#define PIOS_PPM_IN_MAX_NUM_CHANNELS		PIOS_PPM_NUM_INPUTS
#define PIOS_PPM_STABLE_CHANNEL_COUNT		25	// frames
#define PIOS_PPM_IN_MIN_SYNC_PULSE_US		3800	// microseconds
#define PIOS_PPM_IN_MIN_CHANNEL_PULSE_US	750	// microseconds
#define PIOS_PPM_IN_MAX_CHANNEL_PULSE_US	2250   // microseconds

/* Local Variables */
static TIM_ICInitTypeDef TIM_ICInitStructure;

static void PIOS_PPM_Supervisor(uint32_t ppm_id);

enum pios_ppm_dev_magic {
	PIOS_PPM_DEV_MAGIC = 0xee014d8b,
};

struct pios_ppm_dev {
	enum pios_ppm_dev_magic     magic;
	const struct pios_ppm_cfg * cfg;

	uint8_t PulseIndex;
	uint32_t PreviousTime;
	uint32_t CurrentTime;
	uint32_t DeltaTime;
	uint32_t CaptureValue[PIOS_PPM_IN_MAX_NUM_CHANNELS];
	uint32_t CaptureValueNewFrame[PIOS_PPM_IN_MAX_NUM_CHANNELS];
	uint32_t LargeCounter;
	int8_t NumChannels;
	int8_t NumChannelsPrevFrame;
	uint8_t NumChannelCounter;

	uint8_t supv_timer;
	bool Tracking;
	bool Fresh;
};

static bool PIOS_PPM_validate(struct pios_ppm_dev * ppm_dev)
{
	return (ppm_dev->magic == PIOS_PPM_DEV_MAGIC);
}

#if defined(PIOS_INCLUDE_FREERTOS)
static struct pios_ppm_dev * PIOS_PPM_alloc(void)
{
	struct pios_ppm_dev * ppm_dev;

	ppm_dev = (struct pios_ppm_dev *)pvPortMalloc(sizeof(*ppm_dev));
	if (!ppm_dev) return(NULL);

	ppm_dev->magic = PIOS_PPM_DEV_MAGIC;
	return(ppm_dev);
}
#else
static struct pios_ppm_dev pios_ppm_devs[PIOS_PPM_MAX_DEVS];
static uint8_t pios_ppm_num_devs;
static struct pios_ppm_dev * PIOS_PPM_alloc(void)
{
	struct pios_ppm_dev * ppm_dev;

	if (pios_ppm_num_devs >= PIOS_PPM_MAX_DEVS) {
		return (NULL);
	}

	ppm_dev = &pios_ppm_devs[pios_ppm_num_devs++];
	ppm_dev->magic = PIOS_PPM_DEV_MAGIC;

	return (ppm_dev);
}
#endif

static void PIOS_PPM_tim_overflow_cb (uint32_t id, uint32_t context, uint8_t channel, uint16_t count);
static void PIOS_PPM_tim_edge_cb (uint32_t id, uint32_t context, uint8_t channel, uint16_t count);
const static struct pios_tim_callbacks tim_callbacks = {
	.overflow = PIOS_PPM_tim_overflow_cb,
	.edge     = PIOS_PPM_tim_edge_cb,
};

extern int32_t PIOS_PPM_Init(uint32_t * ppm_id, const struct pios_ppm_cfg * cfg)
{
	PIOS_DEBUG_Assert(ppm_id);
	PIOS_DEBUG_Assert(cfg);

	struct pios_ppm_dev * ppm_dev;

	ppm_dev = (struct pios_ppm_dev *) PIOS_PPM_alloc();
	if (!ppm_dev) goto out_fail;

	/* Bind the configuration to the device instance */
	ppm_dev->cfg = cfg;

	/* Set up the state variables */
	ppm_dev->PulseIndex = 0;
	ppm_dev->PreviousTime = 0;
	ppm_dev->CurrentTime = 0;
	ppm_dev->DeltaTime = 0;
	ppm_dev->LargeCounter = 0;
	ppm_dev->NumChannels = -1;
	ppm_dev->NumChannelsPrevFrame = -1;
	ppm_dev->NumChannelCounter = 0;
	ppm_dev->Tracking = false;
	ppm_dev->Fresh = false;

	for (uint8_t i = 0; i < PIOS_PPM_IN_MAX_NUM_CHANNELS; i++) {
		/* Flush counter variables */
		ppm_dev->CaptureValue[i] = 0;
		ppm_dev->CaptureValueNewFrame[i] = 0;

	}

	uint32_t tim_id;
	if (PIOS_TIM_InitChannels(&tim_id, cfg->channels, cfg->num_channels, &tim_callbacks, (uint32_t)ppm_dev)) {
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
			TIM_ITConfig(chan->timer, TIM_IT_CC1 | TIM_IT_Update, ENABLE);
			break;
		case TIM_Channel_2:
			TIM_ITConfig(chan->timer, TIM_IT_CC2 | TIM_IT_Update, ENABLE);
			break;
		case TIM_Channel_3:
			TIM_ITConfig(chan->timer, TIM_IT_CC3 | TIM_IT_Update, ENABLE);
			break;
		case TIM_Channel_4:
			TIM_ITConfig(chan->timer, TIM_IT_CC4 | TIM_IT_Update, ENABLE);
			break;
		}
	}

	/* Setup local variable which stays in this scope */
	/* Doing this here and using a local variable saves doing it in the ISR */
	TIM_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI;
	TIM_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;
	TIM_ICInitStructure.TIM_ICFilter = 0x0;

	if (!PIOS_RTC_RegisterTickCallback(PIOS_PPM_Supervisor, (uint32_t)ppm_dev)) {
		PIOS_DEBUG_Assert(0);
	}

	*ppm_id = (uint32_t)ppm_dev;

	return(0);

out_fail:
	return(-1);
}

/**
* Get the value of an input channel
* \param[in] Channel Number of the channel desired
* \output -1 Channel not available
* \output >0 Channel value
*/
static int32_t PIOS_PPM_Get(uint32_t rcvr_id, uint8_t channel)
{
	struct pios_ppm_dev * ppm_dev = (struct pios_ppm_dev *)rcvr_id;

	if (!PIOS_PPM_validate(ppm_dev)) {
		/* Invalid device specified */
		return PIOS_RCVR_INVALID;
	}

	if (channel >= PIOS_PPM_IN_MAX_NUM_CHANNELS) {
		/* Channel out of range */
		return PIOS_RCVR_INVALID;
	}
	return ppm_dev->CaptureValue[channel];
}

static void PIOS_PPM_tim_overflow_cb (uint32_t tim_id, uint32_t context, uint8_t channel, uint16_t count)
{
	struct pios_ppm_dev * ppm_dev = (struct pios_ppm_dev *)context;

	if (!PIOS_PPM_validate(ppm_dev)) {
		/* Invalid device specified */
		return;
	}

	ppm_dev->LargeCounter += count;

	return;
}


static void PIOS_PPM_tim_edge_cb (uint32_t tim_id, uint32_t context, uint8_t chan_idx, uint16_t count)
{
	/* Recover our device context */
	struct pios_ppm_dev * ppm_dev = (struct pios_ppm_dev *)context;

	if (!PIOS_PPM_validate(ppm_dev)) {
		/* Invalid device specified */
		return;
	}

	if (chan_idx >= ppm_dev->cfg->num_channels) {
		/* Channel out of range */
		return;
	}

	/* Shift the last measurement out */
	ppm_dev->PreviousTime = ppm_dev->CurrentTime;

	/* Grab the new count */
	ppm_dev->CurrentTime = count;

	/* Convert to 32-bit timer result */
	ppm_dev->CurrentTime += ppm_dev->LargeCounter;

	/* Capture computation */		
	ppm_dev->DeltaTime = ppm_dev->CurrentTime - ppm_dev->PreviousTime;

	ppm_dev->PreviousTime = ppm_dev->CurrentTime;

	/* Sync pulse detection */
	if (ppm_dev->DeltaTime > PIOS_PPM_IN_MIN_SYNC_PULSE_US) {
		if (ppm_dev->PulseIndex == ppm_dev->NumChannelsPrevFrame
			&& ppm_dev->PulseIndex >= PIOS_PPM_IN_MIN_NUM_CHANNELS
			&& ppm_dev->PulseIndex <= PIOS_PPM_IN_MAX_NUM_CHANNELS)
		{
			/* If we see n simultaneous frames of the same
			   number of channels we save it as our frame size */
			if (ppm_dev->NumChannelCounter < PIOS_PPM_STABLE_CHANNEL_COUNT)
				ppm_dev->NumChannelCounter++;
			else
				ppm_dev->NumChannels = ppm_dev->PulseIndex;
		} else {
			ppm_dev->NumChannelCounter = 0;
		}

		/* Check if the last frame was well formed */
		if (ppm_dev->PulseIndex == ppm_dev->NumChannels && ppm_dev->Tracking) {
			/* The last frame was well formed */
			for (uint32_t i = 0; i < ppm_dev->NumChannels; i++) {
				ppm_dev->CaptureValue[i] = ppm_dev->CaptureValueNewFrame[i];
			}
			for (uint32_t i = ppm_dev->NumChannels;
			     i < PIOS_PPM_IN_MAX_NUM_CHANNELS; i++) {
				ppm_dev->CaptureValue[i] = PIOS_RCVR_TIMEOUT;
			}
		}

		ppm_dev->Fresh = true;
		ppm_dev->Tracking = true;
		ppm_dev->NumChannelsPrevFrame = ppm_dev->PulseIndex;
		ppm_dev->PulseIndex = 0;

		/* We rely on the supervisor to set CaptureValue to invalid
		   if no valid frame is found otherwise we ride over it */

	} else if (ppm_dev->Tracking) {
		/* Valid pulse duration 0.75 to 2.5 ms*/
		if (ppm_dev->DeltaTime > PIOS_PPM_IN_MIN_CHANNEL_PULSE_US
			&& ppm_dev->DeltaTime < PIOS_PPM_IN_MAX_CHANNEL_PULSE_US
			&& ppm_dev->PulseIndex < PIOS_PPM_IN_MAX_NUM_CHANNELS) {
			
			ppm_dev->CaptureValueNewFrame[ppm_dev->PulseIndex] = ppm_dev->DeltaTime;
			ppm_dev->PulseIndex++;
		} else {
			/* Not a valid pulse duration */
			ppm_dev->Tracking = false;
			for (uint32_t i = 0; i < PIOS_PPM_IN_MAX_NUM_CHANNELS ; i++) {
				ppm_dev->CaptureValueNewFrame[i] = PIOS_RCVR_TIMEOUT;
			}
		}
	}
}

static void PIOS_PPM_Supervisor(uint32_t ppm_id) {
	/* Recover our device context */
	struct pios_ppm_dev * ppm_dev = (struct pios_ppm_dev *)ppm_id;

	if (!PIOS_PPM_validate(ppm_dev)) {
		/* Invalid device specified */
		return;
	}

	/* 
	 * RTC runs at 625Hz so divide down the base rate so
	 * that this loop runs at 25Hz.
	 */
	if(++(ppm_dev->supv_timer) < 25) {
		return;
	}
	ppm_dev->supv_timer = 0;

	if (!ppm_dev->Fresh) {
		ppm_dev->Tracking = false;

		for (int32_t i = 0; i < PIOS_PPM_IN_MAX_NUM_CHANNELS ; i++) {
			ppm_dev->CaptureValue[i] = PIOS_RCVR_TIMEOUT;
			ppm_dev->CaptureValueNewFrame[i] = PIOS_RCVR_TIMEOUT;
		}
	}

	ppm_dev->Fresh = false;
}

#endif

/**
  * @}
  * @}
  */

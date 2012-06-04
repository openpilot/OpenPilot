/**
 ******************************************************************************
 * @addtogroup PIOS PIOS Core hardware abstraction layer
 * @{
 * @addtogroup PIOS_DSM Spektrum/JR DSMx satellite receiver functions
 * @brief Code to bind and read Spektrum/JR DSMx satellite receiver serial stream
 * @{
 *
 * @file       pios_dsm.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2012.
 * @brief      Code bind and read Spektrum/JR DSMx satellite receiver serial stream
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
#include "pios_dsm_priv.h"

#if defined(PIOS_INCLUDE_DSM)

#if !defined(PIOS_INCLUDE_RTC)
#error PIOS_INCLUDE_RTC must be used to use DSM
#endif

/* Forward Declarations */
static int32_t PIOS_DSM_Get(uint32_t rcvr_id, uint8_t channel);
static uint16_t PIOS_DSM_RxInCallback(uint32_t context,
				      uint8_t *buf,
				      uint16_t buf_len,
				      uint16_t *headroom,
				      bool *need_yield);
static void PIOS_DSM_Supervisor(uint32_t dsm_id);

/* Local Variables */
const struct pios_rcvr_driver pios_dsm_rcvr_driver = {
	.read = PIOS_DSM_Get,
};

enum pios_dsm_dev_magic {
	PIOS_DSM_DEV_MAGIC = 0x44534d78,
};

struct pios_dsm_state {
	uint16_t channel_data[PIOS_DSM_NUM_INPUTS];
	uint8_t received_data[DSM_FRAME_LENGTH];
	uint8_t receive_timer;
	uint8_t failsafe_timer;
	uint8_t frame_found;
	uint8_t byte_count;
#ifdef DSM_LOST_FRAME_COUNTER
	uint8_t	frames_lost_last;
	uint16_t frames_lost;
#endif
};

struct pios_dsm_dev {
	enum pios_dsm_dev_magic magic;
	const struct pios_dsm_cfg *cfg;
	enum pios_dsm_proto proto;
	struct pios_dsm_state state;
};

/* Allocate DSM device descriptor */
#if defined(PIOS_INCLUDE_FREERTOS)
static struct pios_dsm_dev *PIOS_DSM_Alloc(void)
{
	struct pios_dsm_dev *dsm_dev;

	dsm_dev = (struct pios_dsm_dev *)pvPortMalloc(sizeof(*dsm_dev));
	if (!dsm_dev)
		return NULL;

	dsm_dev->magic = PIOS_DSM_DEV_MAGIC;
	return dsm_dev;
}
#else
static struct pios_dsm_dev pios_dsm_devs[PIOS_DSM_MAX_DEVS];
static uint8_t pios_dsm_num_devs;
static struct pios_dsm_dev *PIOS_DSM_Alloc(void)
{
	struct pios_dsm_dev *dsm_dev;

	if (pios_dsm_num_devs >= PIOS_DSM_MAX_DEVS)
		return NULL;

	dsm_dev = &pios_dsm_devs[pios_dsm_num_devs++];
	dsm_dev->magic = PIOS_DSM_DEV_MAGIC;

	return dsm_dev;
}
#endif

/* Validate DSM device descriptor */
static bool PIOS_DSM_Validate(struct pios_dsm_dev *dsm_dev)
{
	return (dsm_dev->magic == PIOS_DSM_DEV_MAGIC);
}

/* Try to bind DSMx satellite using specified number of pulses */
static void PIOS_DSM_Bind(struct pios_dsm_dev *dsm_dev, uint8_t bind)
{
	const struct pios_dsm_cfg *cfg = dsm_dev->cfg;

	GPIO_InitTypeDef GPIO_InitStructure = cfg->bind.init;
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;

	/* just to limit bind pulses */
	if (bind > 10)
		bind = 10;

	GPIO_Init(cfg->bind.gpio, &cfg->bind.init);

	/* RX line, set high */
	GPIO_SetBits(cfg->bind.gpio, cfg->bind.init.GPIO_Pin);

	/* on CC works up to 140ms, guess bind window is around 20-140ms after power up */
	PIOS_DELAY_WaitmS(60);

	for (int i = 0; i < bind ; i++) {
		/* RX line, drive low for 120us */
		GPIO_ResetBits(cfg->bind.gpio, cfg->bind.init.GPIO_Pin);
		PIOS_DELAY_WaituS(120);
		/* RX line, drive high for 120us */
		GPIO_SetBits(cfg->bind.gpio, cfg->bind.init.GPIO_Pin);
		PIOS_DELAY_WaituS(120);
	}
	/* RX line, set input and wait for data */
	GPIO_Init(cfg->bind.gpio, &GPIO_InitStructure);
}

/* Reset channels in case of lost signal or explicit failsafe receiver flag */
static void PIOS_DSM_ResetChannels(struct pios_dsm_dev *dsm_dev)
{
	struct pios_dsm_state *state = &(dsm_dev->state);
	for (int i = 0; i < PIOS_DSM_NUM_INPUTS; i++) {
		state->channel_data[i] = PIOS_RCVR_TIMEOUT;
	}
}

/* Reset DSM receiver state */
static void PIOS_DSM_ResetState(struct pios_dsm_dev *dsm_dev)
{
	struct pios_dsm_state *state = &(dsm_dev->state);
	state->receive_timer = 0;
	state->failsafe_timer = 0;
	state->frame_found = 0;
#ifdef DSM_LOST_FRAME_COUNTER
	state->frames_lost_last = 0;
	state->frames_lost = 0;
#endif
	PIOS_DSM_ResetChannels(dsm_dev);
}

/**
 * Check and unroll complete frame data.
 * \output 0 frame data accepted
 * \output -1 frame error found
 */
static int PIOS_DSM_UnrollChannels(struct pios_dsm_dev *dsm_dev)
{
	struct pios_dsm_state *state = &(dsm_dev->state);
	uint8_t resolution;

#ifdef DSM_LOST_FRAME_COUNTER
	/* increment the lost frame counter */
	uint8_t frames_lost = state->received_data[0];
	state->frames_lost += (frames_lost - state->frames_lost_last);
	state->frames_lost_last = frames_lost;
#endif

	/* check the frame type assuming master satellite stream */
	uint8_t type = state->received_data[1];
	switch (type) {
	case 0x01:
	case 0x02:
	case 0x12:
		/* DSM2, DSMJ stream */
		if (dsm_dev->proto == PIOS_DSM_PROTO_DSM2) {
			/* DSM2/DSMJ resolution is known from the header */
			resolution = (type & DSM_DSM2_RES_MASK) ? 11 : 10;
		} else {
			/* DSMX resolution should explicitly be selected */
			goto stream_error;
		}
		break;
	case 0xA2:
	case 0xB2:
		/* DSMX stream */
		if (dsm_dev->proto == PIOS_DSM_PROTO_DSMX10BIT) {
			resolution = 10;
		} else if (dsm_dev->proto == PIOS_DSM_PROTO_DSMX11BIT) {
			resolution = 11;
		} else {
			/* DSMX resolution should explicitly be selected */
			goto stream_error;
		}
		break;
	default:
		/* unknown yet data stream */
		goto stream_error;
	}

	/* unroll channels */
	uint8_t *s = &(state->received_data[2]);
	uint16_t mask = (resolution == 10) ? 0x03ff : 0x07ff;

	for (int i = 0; i < DSM_CHANNELS_PER_FRAME; i++) {
		uint16_t word = ((uint16_t)s[0] << 8) | s[1];
		s += 2;

		/* skip empty channel slot */
		if (word == 0xffff)
			continue;

		/* minimal data validation */
		if ((i > 0) && (word & DSM_2ND_FRAME_MASK)) {
			/* invalid frame data, ignore rest of the frame */
			goto stream_error;
		}

		/* extract and save the channel value */
		uint8_t channel_num = (word >> resolution) & 0x0f;
		if (channel_num < PIOS_DSM_NUM_INPUTS)
			state->channel_data[channel_num] = (word & mask);
	}

#ifdef DSM_LOST_FRAME_COUNTER
	/* put lost frames counter into the last channel for debugging */
	state->channel_data[PIOS_DSM_NUM_INPUTS-1] = state->frames_lost;
#endif

	/* all channels processed */
	return 0;

stream_error:
	/* either DSM2 selected with DSMX stream found, or vice-versa */
	return -1;
}

/* Update decoder state processing input byte from the DSMx stream */
static void PIOS_DSM_UpdateState(struct pios_dsm_dev *dsm_dev, uint8_t byte)
{
	struct pios_dsm_state *state = &(dsm_dev->state);
	if (state->frame_found) {
		/* receiving the data frame */
		if (state->byte_count < DSM_FRAME_LENGTH) {
			/* store next byte */
			state->received_data[state->byte_count++] = byte;
			if (state->byte_count == DSM_FRAME_LENGTH) {
				/* full frame received - process and wait for new one */
				if (!PIOS_DSM_UnrollChannels(dsm_dev))
					/* data looking good */
					state->failsafe_timer = 0;

				/* prepare for the next frame */
				state->frame_found = 0;
			}
		}
	}
}

/* Initialise DSM receiver interface */
int32_t PIOS_DSM_Init(uint32_t *dsm_id,
		      const struct pios_dsm_cfg *cfg,
		      const struct pios_com_driver *driver,
		      uint32_t lower_id,
		      enum pios_dsm_proto proto,
		      uint8_t bind)
{
	PIOS_DEBUG_Assert(dsm_id);
	PIOS_DEBUG_Assert(cfg);
	PIOS_DEBUG_Assert(driver);

	struct pios_dsm_dev *dsm_dev;

	dsm_dev = (struct pios_dsm_dev *)PIOS_DSM_Alloc();
	if (!dsm_dev)
		return -1;

	/* Bind the configuration to the device instance */
	dsm_dev->cfg = cfg;
	dsm_dev->proto = proto;

	/* Bind the receiver if requested */
	if (bind)
		PIOS_DSM_Bind(dsm_dev, bind);

	PIOS_DSM_ResetState(dsm_dev);

	*dsm_id = (uint32_t)dsm_dev;

	/* Set comm driver callback */
	(driver->bind_rx_cb)(lower_id, PIOS_DSM_RxInCallback, *dsm_id);

	if (!PIOS_RTC_RegisterTickCallback(PIOS_DSM_Supervisor, *dsm_id)) {
		PIOS_DEBUG_Assert(0);
	}

	return 0;
}

/* Comm byte received callback */
static uint16_t PIOS_DSM_RxInCallback(uint32_t context,
				      uint8_t *buf,
				      uint16_t buf_len,
				      uint16_t *headroom,
				      bool *need_yield)
{
	struct pios_dsm_dev *dsm_dev = (struct pios_dsm_dev *)context;

	bool valid = PIOS_DSM_Validate(dsm_dev);
	PIOS_Assert(valid);

	/* process byte(s) and clear receive timer */
	for (uint8_t i = 0; i < buf_len; i++) {
		PIOS_DSM_UpdateState(dsm_dev, buf[i]);
		dsm_dev->state.receive_timer = 0;
	}

	/* Always signal that we can accept another byte */
	if (headroom)
		*headroom = DSM_FRAME_LENGTH;

	/* We never need a yield */
	*need_yield = false;

	/* Always indicate that all bytes were consumed */
	return buf_len;
}

/**
 * Get the value of an input channel
 * \param[in] channel Number of the channel desired (zero based)
 * \output PIOS_RCVR_INVALID channel not available
 * \output PIOS_RCVR_TIMEOUT failsafe condition or missing receiver
 * \output >0 channel value
 */
static int32_t PIOS_DSM_Get(uint32_t rcvr_id, uint8_t channel)
{
	struct pios_dsm_dev *dsm_dev = (struct pios_dsm_dev *)rcvr_id;

	if (!PIOS_DSM_Validate(dsm_dev))
		return PIOS_RCVR_INVALID;

	/* return error if channel is not available */
	if (channel >= PIOS_DSM_NUM_INPUTS)
		return PIOS_RCVR_INVALID;

	/* may also be PIOS_RCVR_TIMEOUT set by other function */
	return dsm_dev->state.channel_data[channel];
}

/**
 * Input data supervisor is called periodically and provides
 * two functions: frame syncing and failsafe triggering.
 *
 * DSM frames come at 11ms or 22ms rate at 115200bps.
 * RTC timer is running at 625Hz (1.6ms). So with divider 5 it gives
 * 8ms pause between frames which is good for both DSM frame rates.
 *
 * Data receive function must clear the receive_timer to confirm new
 * data reception. If no new data received in 100ms, we must call the
 * failsafe function which clears all channels.
 */
static void PIOS_DSM_Supervisor(uint32_t dsm_id)
{
	struct pios_dsm_dev *dsm_dev = (struct pios_dsm_dev *)dsm_id;

	bool valid = PIOS_DSM_Validate(dsm_dev);
	PIOS_Assert(valid);

	struct pios_dsm_state *state = &(dsm_dev->state);

	/* waiting for new frame if no bytes were received in 8ms */
	if (++state->receive_timer > 4) {
		state->frame_found = 1;
		state->byte_count = 0;
		state->receive_timer = 0;
	}

	/* activate failsafe if no frames have arrived in 102.4ms */
	if (++state->failsafe_timer > 64) {
		PIOS_DSM_ResetChannels(dsm_dev);
		state->failsafe_timer = 0;
	}
}

#endif	/* PIOS_INCLUDE_DSM */

/** 
 * @}
 * @}
 */

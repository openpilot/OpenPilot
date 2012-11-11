/**
 ******************************************************************************
 * @addtogroup PIOS PIOS Core hardware abstraction layer
 * @{
 * @addtogroup PIOS_SBus Futaba S.Bus receiver functions
 * @brief Code to read Futaba S.Bus receiver serial stream
 * @{
 *
 * @file       pios_sbus.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2011.
 * @brief      Code to read Futaba S.Bus receiver serial stream
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
#include "pios_sbus_priv.h"

#if defined(PIOS_INCLUDE_SBUS)

/* Forward Declarations */
static int32_t PIOS_SBus_Get(uint32_t rcvr_id, uint8_t channel);
static uint16_t PIOS_SBus_RxInCallback(uint32_t context,
				       uint8_t *buf,
				       uint16_t buf_len,
				       uint16_t *headroom,
				       bool *need_yield);
static void PIOS_SBus_Supervisor(uint32_t sbus_id);


/* Local Variables */
const struct pios_rcvr_driver pios_sbus_rcvr_driver = {
	.read = PIOS_SBus_Get,
};

enum pios_sbus_dev_magic {
	PIOS_SBUS_DEV_MAGIC = 0x53427573,
};

struct pios_sbus_state {
	uint16_t channel_data[PIOS_SBUS_NUM_INPUTS];
	uint8_t received_data[SBUS_FRAME_LENGTH - 2];
	uint8_t receive_timer;
	uint8_t failsafe_timer;
	uint8_t frame_found;
	uint8_t byte_count;
};

struct pios_sbus_dev {
	enum pios_sbus_dev_magic magic;
	const struct pios_sbus_cfg *cfg;
	struct pios_sbus_state state;
};

/* Allocate S.Bus device descriptor */
#if defined(PIOS_INCLUDE_FREERTOS)
static struct pios_sbus_dev *PIOS_SBus_Alloc(void)
{
	struct pios_sbus_dev *sbus_dev;

	sbus_dev = (struct pios_sbus_dev *)pvPortMalloc(sizeof(*sbus_dev));
	if (!sbus_dev) return(NULL);

	sbus_dev->magic = PIOS_SBUS_DEV_MAGIC;
	return(sbus_dev);
}
#else
static struct pios_sbus_dev pios_sbus_devs[PIOS_SBUS_MAX_DEVS];
static uint8_t pios_sbus_num_devs;
static struct pios_sbus_dev *PIOS_SBus_Alloc(void)
{
	struct pios_sbus_dev *sbus_dev;

	if (pios_sbus_num_devs >= PIOS_SBUS_MAX_DEVS) {
		return (NULL);
	}

	sbus_dev = &pios_sbus_devs[pios_sbus_num_devs++];
	sbus_dev->magic = PIOS_SBUS_DEV_MAGIC;

	return (sbus_dev);
}
#endif

/* Validate S.Bus device descriptor */
static bool PIOS_SBus_Validate(struct pios_sbus_dev *sbus_dev)
{
	return (sbus_dev->magic == PIOS_SBUS_DEV_MAGIC);
}

/* Reset channels in case of lost signal or explicit failsafe receiver flag */
static void PIOS_SBus_ResetChannels(struct pios_sbus_state *state)
{
	for (int i = 0; i < PIOS_SBUS_NUM_INPUTS; i++) {
		state->channel_data[i] = PIOS_RCVR_TIMEOUT;
	}
}

/* Reset S.Bus receiver state */
static void PIOS_SBus_ResetState(struct pios_sbus_state *state)
{
	state->receive_timer = 0;
	state->failsafe_timer = 0;
	state->frame_found = 0;
	PIOS_SBus_ResetChannels(state);
}

/* Initialise S.Bus receiver interface */
int32_t PIOS_SBus_Init(uint32_t *sbus_id,
		       const struct pios_sbus_cfg *cfg,
		       const struct pios_com_driver *driver,
		       uint32_t lower_id)
{
	PIOS_DEBUG_Assert(sbus_id);
	PIOS_DEBUG_Assert(cfg);
	PIOS_DEBUG_Assert(driver);

	struct pios_sbus_dev *sbus_dev;

	sbus_dev = (struct pios_sbus_dev *)PIOS_SBus_Alloc();
	if (!sbus_dev) goto out_fail;

	/* Bind the configuration to the device instance */
	sbus_dev->cfg = cfg;

	PIOS_SBus_ResetState(&(sbus_dev->state));

	*sbus_id = (uint32_t)sbus_dev;

	/* Enable inverter clock and enable the inverter */
	(*cfg->gpio_clk_func)(cfg->gpio_clk_periph, ENABLE);
	GPIO_Init(cfg->inv.gpio, &cfg->inv.init);
	GPIO_WriteBit(cfg->inv.gpio, cfg->inv.init.GPIO_Pin, cfg->gpio_inv_enable);

	/* Set comm driver callback */
	(driver->bind_rx_cb)(lower_id, PIOS_SBus_RxInCallback, *sbus_id);

	if (!PIOS_RTC_RegisterTickCallback(PIOS_SBus_Supervisor, *sbus_id)) {
		PIOS_DEBUG_Assert(0);
	}

	return 0;

out_fail:
	return -1;
}

/**
 * Get the value of an input channel
 * \param[in] channel Number of the channel desired (zero based)
 * \output PIOS_RCVR_INVALID channel not available
 * \output PIOS_RCVR_TIMEOUT failsafe condition or missing receiver
 * \output >0 channel value
 */
static int32_t PIOS_SBus_Get(uint32_t rcvr_id, uint8_t channel)
{
	struct pios_sbus_dev *sbus_dev = (struct pios_sbus_dev *)rcvr_id;

	if (!PIOS_SBus_Validate(sbus_dev))
		return PIOS_RCVR_INVALID;

	/* return error if channel is not available */
	if (channel >= PIOS_SBUS_NUM_INPUTS) {
		return PIOS_RCVR_INVALID;
	}

	return sbus_dev->state.channel_data[channel];
}

/**
 * Compute channel_data[] from received_data[].
 * For efficiency it unrolls first 8 channels without loops and does the
 * same for other 8 channels. Also 2 discrete channels will be set.
 */
static void PIOS_SBus_UnrollChannels(struct pios_sbus_state *state)
{
	uint8_t *s = state->received_data;
	uint16_t *d = state->channel_data;

#define F(v,s) (((v) >> (s)) & 0x7ff)

	/* unroll channels 1-8 */
	*d++ = F(s[0] | s[1] << 8, 0);
	*d++ = F(s[1] | s[2] << 8, 3);
	*d++ = F(s[2] | s[3] << 8 | s[4] << 16, 6);
	*d++ = F(s[4] | s[5] << 8, 1);
	*d++ = F(s[5] | s[6] << 8, 4);
	*d++ = F(s[6] | s[7] << 8 | s[8] << 16, 7);
	*d++ = F(s[8] | s[9] << 8, 2);
	*d++ = F(s[9] | s[10] << 8, 5);

	/* unroll channels 9-16 */
	*d++ = F(s[11] | s[12] << 8, 0);
	*d++ = F(s[12] | s[13] << 8, 3);
	*d++ = F(s[13] | s[14] << 8 | s[15] << 16, 6);
	*d++ = F(s[15] | s[16] << 8, 1);
	*d++ = F(s[16] | s[17] << 8, 4);
	*d++ = F(s[17] | s[18] << 8 | s[19] << 16, 7);
	*d++ = F(s[19] | s[20] << 8, 2);
	*d++ = F(s[20] | s[21] << 8, 5);

	/* unroll discrete channels 17 and 18 */
	*d++ = (s[22] & SBUS_FLAG_DC1) ? SBUS_VALUE_MAX : SBUS_VALUE_MIN;
	*d++ = (s[22] & SBUS_FLAG_DC2) ? SBUS_VALUE_MAX : SBUS_VALUE_MIN;
}

/* Update decoder state processing input byte from the S.Bus stream */
static void PIOS_SBus_UpdateState(struct pios_sbus_state *state, uint8_t b)
{
	/* should not process any data until new frame is found */
	if (!state->frame_found)
		return;

	if (state->byte_count == 0) {
		if (b != SBUS_SOF_BYTE) {
			/* discard the whole frame if the 1st byte is not correct */
			state->frame_found = 0;
		} else {
			/* do not store the SOF byte */
			state->byte_count++;
		}
		return;
	}

	/* do not store last frame byte as well */
	if (state->byte_count < SBUS_FRAME_LENGTH - 1) {
		/* store next byte */
		state->received_data[state->byte_count - 1] = b;
		state->byte_count++;
	} else {
		if (b == SBUS_EOF_BYTE) {
			/* full frame received */
			uint8_t flags = state->received_data[SBUS_FRAME_LENGTH - 3];
			if (flags & SBUS_FLAG_FL) {
				/* frame lost, do not update */
			} else if (flags & SBUS_FLAG_FS) {
				/* failsafe flag active */
				PIOS_SBus_ResetChannels(state);
			} else {
				/* data looking good */
				PIOS_SBus_UnrollChannels(state);
				state->failsafe_timer = 0;
			}
		} else {
			/* discard whole frame */
		}

		/* prepare for the next frame */
		state->frame_found = 0;
	}
}

/* Comm byte received callback */
static uint16_t PIOS_SBus_RxInCallback(uint32_t context,
				       uint8_t *buf,
				       uint16_t buf_len,
				       uint16_t *headroom,
				       bool *need_yield)
{
	struct pios_sbus_dev *sbus_dev = (struct pios_sbus_dev *)context;

	bool valid = PIOS_SBus_Validate(sbus_dev);
	PIOS_Assert(valid);

	struct pios_sbus_state *state = &(sbus_dev->state);

	/* process byte(s) and clear receive timer */
	for (uint8_t i = 0; i < buf_len; i++) {
		PIOS_SBus_UpdateState(state, buf[i]);
		state->receive_timer = 0;
	}

	/* Always signal that we can accept another byte */
	if (headroom)
		*headroom = SBUS_FRAME_LENGTH;

	/* We never need a yield */
	*need_yield = false;

	/* Always indicate that all bytes were consumed */
	return buf_len;
}

/**
 * Input data supervisor is called periodically and provides
 * two functions: frame syncing and failsafe triggering.
 *
 * S.Bus frames come at 7ms (HS) or 14ms (FS) rate at 100000bps.
 * RTC timer is running at 625Hz (1.6ms). So with divider 2 it gives
 * 3.2ms pause between frames which is good for both S.Bus frame rates.
 *
 * Data receive function must clear the receive_timer to confirm new
 * data reception. If no new data received in 100ms, we must call the
 * failsafe function which clears all channels.
 */
static void PIOS_SBus_Supervisor(uint32_t sbus_id)
{
	struct pios_sbus_dev *sbus_dev = (struct pios_sbus_dev *)sbus_id;

	bool valid = PIOS_SBus_Validate(sbus_dev);
	PIOS_Assert(valid);

	struct pios_sbus_state *state = &(sbus_dev->state);

	/* waiting for new frame if no bytes were received in 3.2ms */
	if (++state->receive_timer > 2) {
		state->frame_found = 1;
		state->byte_count = 0;
		state->receive_timer = 0;
	}

	/* activate failsafe if no frames have arrived in 102.4ms */
	if (++state->failsafe_timer > 64) {
		PIOS_SBus_ResetChannels(state);
		state->failsafe_timer = 0;
	}
}

#endif	/* PIOS_INCLUDE_SBUS */

/** 
 * @}
 * @}
 */

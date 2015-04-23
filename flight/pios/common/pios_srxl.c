/**
 ******************************************************************************
 * @addtogroup PIOS PIOS Core hardware abstraction layer
 * @{
 * @addtogroup PIOS_SRXL Multiplex SRXL receiver functions
 * @brief Code to read Multiplex SRXL receiver serial stream
 * @{
 *
 * @file       pios_srxl.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2015.
 * @brief      Code to read Multiplex SRXL receiver serial stream
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

#ifdef PIOS_INCLUDE_SRXL

#include "pios_srxl_priv.h"

/* Forward Declarations */
static int32_t PIOS_SRXL_Get(uint32_t rcvr_id, uint8_t channel);
static uint16_t PIOS_SRXL_RxInCallback(uint32_t context,
                                       uint8_t *buf,
                                       uint16_t buf_len,
                                       uint16_t *headroom,
                                       bool *need_yield);
static void PIOS_SRXL_Supervisor(uint32_t srxl_id);


/* Local Variables */
const struct pios_rcvr_driver pios_srxl_rcvr_driver = {
    .read = PIOS_SRXL_Get,
};

enum pios_srxl_dev_magic {
    PIOS_SRXL_DEV_MAGIC = 0x55545970,
};

struct pios_srxl_state {
    uint16_t channel_data[PIOS_SRXL_NUM_INPUTS];
    uint8_t  received_data[SRXL_FRAME_LENGTH];
    uint8_t  receive_timer;
    uint8_t  failsafe_timer;
    uint8_t  frame_found;
    uint8_t  byte_count;
    uint8_t  data_bytes;
};

struct pios_srxl_dev {
    enum pios_srxl_dev_magic   magic;
    struct pios_srxl_state     state;
};

/* Allocate S.Bus device descriptor */
#if defined(PIOS_INCLUDE_FREERTOS)
static struct pios_srxl_dev *PIOS_SRXL_Alloc(void)
{
    struct pios_srxl_dev *srxl_dev;

    srxl_dev = (struct pios_srxl_dev *)pios_malloc(sizeof(*srxl_dev));
    if (!srxl_dev) {
        return NULL;
    }

    srxl_dev->magic = PIOS_SRXL_DEV_MAGIC;
    return srxl_dev;
}
#else
static struct pios_srxl_dev pios_srxl_devs[PIOS_SRXL_MAX_DEVS];
static uint8_t pios_srxl_num_devs;
static struct pios_srxl_dev *PIOS_SRXL_Alloc(void)
{
    struct pios_srxl_dev *srxl_dev;

    if (pios_srxl_num_devs >= PIOS_SRXL_MAX_DEVS) {
        return NULL;
    }

    srxl_dev = &pios_srxl_devs[pios_srxl_num_devs++];
    srxl_dev->magic = PIOS_SRXL_DEV_MAGIC;

    return srxl_dev;
}
#endif /* if defined(PIOS_INCLUDE_FREERTOS) */

/* Validate SRXL device descriptor */
static bool PIOS_SRXL_Validate(struct pios_srxl_dev *srxl_dev)
{
    return srxl_dev->magic == PIOS_SRXL_DEV_MAGIC;
}

/* Reset channels in case of lost signal or explicit failsafe receiver flag */
static void PIOS_SRXL_ResetChannels(struct pios_srxl_state *state)
{
    for (int i = 0; i < PIOS_SRXL_NUM_INPUTS; i++) {
        state->channel_data[i] = PIOS_RCVR_TIMEOUT;
    }
}

/* Reset SRXL receiver state */
static void PIOS_SRXL_ResetState(struct pios_srxl_state *state)
{
    state->byte_count         = 0;
    state->receive_timer      = 0;
    state->failsafe_timer     = 0;
    state->frame_found        = 0;
    state->data_bytes         = 0;
    PIOS_SRXL_ResetChannels(state);
}

/* Initialize SRXL receiver interface */
int32_t PIOS_SRXL_Init(uint32_t *srxl_id,
                       const struct pios_com_driver *driver,
                       uint32_t lower_id)
{
    PIOS_DEBUG_Assert(srxl_id);
    PIOS_DEBUG_Assert(driver);

    struct pios_srxl_dev *srxl_dev;

    srxl_dev = (struct pios_srxl_dev *)PIOS_SRXL_Alloc();
    if (!srxl_dev) {
        goto out_fail;
    }

    PIOS_SRXL_ResetState(&(srxl_dev->state));

    *srxl_id = (uint32_t)srxl_dev;

    /* Set comm driver callback */
    (driver->bind_rx_cb)(lower_id, PIOS_SRXL_RxInCallback, *srxl_id);

    if (!PIOS_RTC_RegisterTickCallback(PIOS_SRXL_Supervisor, *srxl_id)) {
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
 * \output >=0 channel value
 */
static int32_t PIOS_SRXL_Get(uint32_t rcvr_id, uint8_t channel)
{
    struct pios_srxl_dev *srxl_dev = (struct pios_srxl_dev *)rcvr_id;

    if (!PIOS_SRXL_Validate(srxl_dev)) {
        return PIOS_RCVR_INVALID;
    }

    /* return error if channel is not available */
    if (channel >= PIOS_SRXL_NUM_INPUTS) {
        return PIOS_RCVR_INVALID;
    }

    return srxl_dev->state.channel_data[channel];
}

static void PIOS_SRXL_UnrollChannels(struct pios_srxl_state *state)
{
    uint8_t *received_data  = state->received_data;
    uint16_t *channel_data = state->channel_data;
    uint8_t channel;
    uint16_t channel_value;
    for (channel = 0; channel < (state->data_bytes / 2); channel++) {
        channel_value = ((uint16_t)received_data[SRXL_HEADER_LENGTH + (channel * 2)]) << 8;
        channel_value = channel_value + ((uint16_t)received_data[SRXL_HEADER_LENGTH + (channel * 2) + 1]);
        d[channel] = (800 + ((channel_value * 1400) >> 12));
    }
}

static bool PIOS_SRXL_Validate_Checksum(struct pios_srxl_state *state)
{
	// Check the CRC16 checksum. The provided checksum is immediately after the channel data.
	// All data including start byte and version byte is included in crc calculation.
    uint8_t i = 0;
    uint16_t crc = 0;
	for (i = 0; i < SRXL_HEADER_LENGTH + state->data_bytes; i++) {
		crc = crc ^ (int16_t)state->received_data[i] << 8;
		uint8_t j = 0;
	    for (j = 0; j < 8; j++) {
	        if (crc & 0x8000) {
	            crc = crc << 1 ^ 0x1021;
	        } else {
	            crc = crc << 1;
	        }
	    }
	}
	uint16_t checksum = (((uint16_t)(state->received_data[i] << 8) |
			(uint16_t)state->received_data[i + 1]));
    return crc == checksum;
}

/* Update decoder state processing input byte from the SRXL stream */
static void PIOS_SRXL_UpdateState(struct pios_srxl_state *state, uint8_t b)
{
    /* should not process any data until new frame is found */
    if (!state->frame_found) {
        return;
    }

    if (state->byte_count < (SRXL_HEADER_LENGTH + state->data_bytes + SRXL_CHECKSUM_LENGTH)) {
        if (state->byte_count == 0) {
        	// Set up the length of the channel data according to version received
            if (b == SRXL_V1_HEADER) {
                state->data_bytes = SRXL_V1_CHANNEL_DATA_BYTES;
            } else if (b == SRXL_V2_HEADER) {
                state->data_bytes = SRXL_V2_CHANNEL_DATA_BYTES;
            } else {
                /* discard the whole frame if the 1st byte is not correct */
                state->frame_found = 0;
                return;
            }
        }
        /* store next byte */
        state->received_data[state->byte_count] = b;
        state->byte_count++;
    } else {
        // We have a complete message, lets decode it
        if (PIOS_SRXL_Validate_Checksum(state)) {
            /* data looking good */
            PIOS_SRXL_UnrollChannels(state);
            state->failsafe_timer = 0;
        } else {
            /* discard whole frame */
        }
        /* prepare for the next frame */
        state->frame_found = 0;
    }
}

/* Comm byte received callback */
static uint16_t PIOS_SRXL_RxInCallback(uint32_t context,
                                       uint8_t *buf,
                                       uint16_t buf_len,
                                       uint16_t *headroom,
                                       bool *need_yield)
{
    struct pios_srxl_dev *srxl_dev = (struct pios_srxl_dev *)context;

    bool valid = PIOS_SRXL_Validate(srxl_dev);

    PIOS_Assert(valid);

    struct pios_srxl_state *state = &(srxl_dev->state);

    /* process byte(s) and clear receive timer */
    for (uint8_t i = 0; i < buf_len; i++) {
        PIOS_SRXL_UpdateState(state, buf[i]);
        state->receive_timer = 0;
    }

    /* Always signal that we can accept another byte */
    if (headroom) {
        *headroom = SRXL_FRAME_LENGTH;
    }

    /* We never need a yield */
    *need_yield = false;

    /* Always indicate that all bytes were consumed */
    return buf_len;
}

/**
 * Input data supervisor is called periodically and provides
 * two functions: frame syncing and failsafe triggering.
 *
 * Multiplex SRXL frames come at 14ms (FastResponse ON) or 21ms (FastResponse OFF)
 * rate at 115200bps.
 * RTC timer is running at 625Hz (1.6ms). So with divider 2 it gives
 * 3.2ms pause between frames which is good for both SRXL frame rates.
 *
 * Data receive function must clear the receive_timer to confirm new
 * data reception. If no new data received in 100ms, we must call the
 * failsafe function which clears all channels.
 */
static void PIOS_SRXL_Supervisor(uint32_t srxl_id)
{
    struct pios_srxl_dev *srxl_dev = (struct pios_srxl_dev *)srxl_id;

    bool valid = PIOS_SRXL_Validate(srxl_dev);

    PIOS_Assert(valid);

    struct pios_srxl_state *state = &(srxl_dev->state);

    /* waiting for new frame if no bytes were received in 6.4ms */

    if (++state->receive_timer > 2) {
        state->frame_found   = 1;
        state->byte_count    = 0;
        state->receive_timer = 0;
    }

    /* activate failsafe if no frames have arrived in 102.4ms */
    if (++state->failsafe_timer > 64) {
        PIOS_SRXL_ResetChannels(state);
        state->failsafe_timer = 0;
    }
}

#endif /* PIOS_INCLUDE_SRXL */

/**
 * @}
 * @}
 */

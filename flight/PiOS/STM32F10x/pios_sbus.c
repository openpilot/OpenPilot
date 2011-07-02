/**
 ******************************************************************************
 * @addtogroup PIOS PIOS Core hardware abstraction layer
 * @{
 * @addtogroup PIOS_SBUS Futaba S.Bus receiver functions
 * @brief Code to read Futaba S.Bus input
 * @{
 *
 * @file       pios_sbus.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2011.
 * @brief      USART commands. Inits USARTs, controls USARTs & Interrupt handlers. (STM32 dependent)
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

/* Global Variables */

/* Provide a RCVR driver */
static int32_t PIOS_SBUS_Get(uint32_t chan_id);

const struct pios_rcvr_driver pios_sbus_rcvr_driver = {
	.read = PIOS_SBUS_Get,
};

/* Local Variables */
static uint16_t channel_data[SBUS_NUMBER_OF_CHANNELS];
static uint8_t received_data[SBUS_FRAME_LENGTH - 2];
static uint8_t receive_timer;
static uint8_t failsafe_timer;
static uint8_t frame_found;

static void PIOS_SBUS_Supervisor(uint32_t sbus_id);

/**
 * reset_channels() function clears all channel data in case of
 * lost signal or explicit failsafe flag from the S.Bus data stream
 */
static void reset_channels(void)
{
	for (int i = 0; i < SBUS_NUMBER_OF_CHANNELS; i++) {
		channel_data[i] = 0;
	}
}

/**
 * unroll_channels() function computes channel_data[] from received_data[]
 * For efficiency it unrolls first 8 channels without loops. If other
 * 8 channels are needed they can be unrolled using the same code
 * starting from s[11] instead of s[0]. Two extra digital channels are
 * accessible using (s[22] & SBUS_FLAG_DGx) logical expressions.
 */
static void unroll_channels(void)
{
	uint8_t *s = received_data;
	uint16_t *d = channel_data;

#if (SBUS_NUMBER_OF_CHANNELS != 8)
#error Current S.Bus code unrolls only first 8 channels
#endif

#define F(v,s) ((v) >> s) & 0x7ff
	*d++ = F(s[0] | s[1] << 8, 0);
	*d++ = F(s[1] | s[2] << 8, 3);
	*d++ = F(s[2] | s[3] << 8 | s[4] << 16, 6);
	*d++ = F(s[4] | s[5] << 8, 1);
	*d++ = F(s[5] | s[6] << 8, 4);
	*d++ = F(s[6] | s[7] << 8 | s[8] << 16, 7);
	*d++ = F(s[8] | s[9] << 8, 2);
	*d++ = F(s[9] | s[10] << 8, 5);
}

/**
 * process_byte() function processes incoming byte from S.Bus stream
 */
static void process_byte(uint8_t b)
{
	static uint8_t byte_count;

	if (frame_found == 0) {
		/* no frame found yet, waiting for start byte */
		if (b == SBUS_SOF_BYTE) {
			byte_count = 0;
			frame_found = 1;
		}
	} else {
		/* do not store start and end of frame bytes */
		if (byte_count < SBUS_FRAME_LENGTH - 2) {
			/* store next byte */
			received_data[byte_count++] = b;
		} else {
			if (b == SBUS_EOF_BYTE) {
				/* full frame received */
				uint8_t flags = received_data[SBUS_FRAME_LENGTH - 3];
				if (flags & SBUS_FLAG_FL) {
					/* frame lost, do not update */
				} else if (flags & SBUS_FLAG_FS) {
					/* failsafe flag active */
					reset_channels();
				} else {
					/* data looking good */
					unroll_channels();
					failsafe_timer = 0;
				}
			} else {
				/* discard whole frame */
			}

			/* prepare for the next frame */
			frame_found = 0;
		}
	}
}

/**
 * Initialise S.Bus receiver interface
 */
void PIOS_SBUS_Init(void)
{
	/* Enable USART input invertor clock and enable the invertor */
	(*pios_sbus_cfg.gpio_clk_func)(pios_sbus_cfg.gpio_clk_periph, ENABLE);
	GPIO_Init(pios_sbus_cfg.gpio_inv_port, &pios_sbus_cfg.gpio_inv_init);
	GPIO_WriteBit(pios_sbus_cfg.gpio_inv_port,
		      pios_sbus_cfg.gpio_inv_init.GPIO_Pin,
		      pios_sbus_cfg.gpio_inv_enable);

	if (!PIOS_RTC_RegisterTickCallback(PIOS_SBUS_Supervisor, 0)) {
		PIOS_DEBUG_Assert(0);
	}
}

/**
 * Get the value of an input channel
 * \param[in] channel Number of the channel desired (zero based)
 * \output -1 channel not available
 * \output >0 channel value
 */
static int32_t PIOS_SBUS_Get(uint32_t chan_id)
{
	/* return error if channel is not available */
	if (chan_id >= SBUS_NUMBER_OF_CHANNELS) {
		return -1;
	}
	return channel_data[chan_id];
}

/**
 * Interrupt handler for USART
 */
void PIOS_SBUS_irq_handler(uint32_t usart_id)
{
	/* by always reading DR after SR make sure to clear any error interrupts */
	volatile uint16_t sr = pios_sbus_cfg.pios_usart_sbus_cfg->regs->SR;
	volatile uint8_t b = pios_sbus_cfg.pios_usart_sbus_cfg->regs->DR;

	/* process received byte if one has arrived */
	if (sr & USART_SR_RXNE) {
		/* process byte and clear receive timer */
		process_byte(b);
		receive_timer = 0;
	} 

	/* ignore TXE interrupts */
	if (sr & USART_SR_TXE) {
		/* disable TXE interrupt (TXEIE=0) */
		USART_ITConfig(pios_sbus_cfg.pios_usart_sbus_cfg->regs, USART_IT_TXE, DISABLE);
	}
}

/**
 * Input data supervisor is called periodically and provides
 * two functions: frame syncing and failsafe triggering.
 *
 * S.Bus frames come at 7ms (HS) or 14ms (FS) rate at 100000bps. RTC
 * timer is running at 625Hz (1.6ms). So with divider 2 it gives
 * 3.2ms pause between frames which is good for both S.Bus data rates.
 *
 * Data receive function must clear the receive_timer to confirm new
 * data reception. If no new data received in 100ms, we must call the
 * failsafe function which clears all channels.
 */
static void PIOS_SBUS_Supervisor(uint32_t sbus_id)
{
	/* waiting for new frame if no bytes were received in 3.2ms */
	if (++receive_timer > 2) {
		receive_timer = 0;
		frame_found = 0;
	}

	/* activate failsafe if no frames have arrived in 102.4ms */
	if (++failsafe_timer > 64) {
		reset_channels();
		failsafe_timer = 0;
	}
}

#endif

/** 
 * @}
 * @}
 */

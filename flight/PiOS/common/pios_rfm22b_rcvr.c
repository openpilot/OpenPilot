/**
 ******************************************************************************
 * @addtogroup PIOS PIOS Core hardware abstraction layer
 * @{
 * @addtogroup   PIOS_RFM22B_RCVR RFM22B Receiver Input Functions
 * @brief	 Code to output the PPM signal from the RFM22B
 * @{
 *
 * @file       pios_rfm22b_rcvr.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2012.
 * @brief      Implements a receiver interface to the RFM22B device
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

#ifdef PIOS_INCLUDE_RFM22B_RCVR

#include "pios_rfm22b_priv.h"

/* Provide a RCVR driver */
static int32_t PIOS_RFM22B_RCVR_Get(uint32_t rcvr_id, uint8_t channel);
static void PIOS_RFM22B_RCVR_Supervisor(uint32_t rcvr_id);

const struct pios_rcvr_driver pios_rfm22b_rcvr_driver = {
	.read = PIOS_RFM22B_RCVR_Get,
};

int32_t PIOS_RFM22B_RCVR_Init(uint32_t rcvr_id)
{
	struct pios_rfm22b_dev *rfm22b_dev = (struct pios_rfm22b_dev *)rcvr_id;
	if (!PIOS_RFM22B_validate(rfm22b_dev))
		return -1;

	// Initialize
	for (uint8_t i = 0; i < PIOS_RFM22B_RCVR_MAX_CHANNELS; ++i)
		rfm22b_dev->ppm_channel[i] = PIOS_RCVR_TIMEOUT;
	rfm22b_dev->ppm_supv_timer = 0;

	// Register the failsafe timer callback.
	if (!PIOS_RTC_RegisterTickCallback(PIOS_RFM22B_RCVR_Supervisor, rcvr_id))
		PIOS_DEBUG_Assert(0);

	return 0;
}

static int32_t PIOS_RFM22B_RCVR_Get(uint32_t rcvr_id, uint8_t channel)
{
	struct pios_rfm22b_dev *rfm22b_dev = (struct pios_rfm22b_dev *)rcvr_id;
	if (!PIOS_RFM22B_validate(rfm22b_dev))
		return -1;

	if (channel >= GCSRECEIVER_CHANNEL_NUMELEM)
		/* channel is out of range */
		return -1;

	return rfm22b_dev->ppm_channel[channel];
}

static void PIOS_RFM22B_RCVR_Supervisor(uint32_t rcvr_id) {
	struct pios_rfm22b_dev *rfm22b_dev = (struct pios_rfm22b_dev *)rcvr_id;
	if (!PIOS_RFM22B_validate(rfm22b_dev))
		return;

	// RTC runs at 625Hz.
	if (++(rfm22b_dev->ppm_supv_timer) < (PIOS_RFM22B_RCVR_TIMEOUT_MS * 1000 / 625))
		return;
	rfm22b_dev->ppm_supv_timer = 0;

	// Have we received fresh values since the last update?
	if (!rfm22b_dev->ppm_fresh)
		for (uint8_t i = 0; i < PIOS_RFM22B_RCVR_MAX_CHANNELS; ++i)
			rfm22b_dev->ppm_channel[i] = 0;
	rfm22b_dev->ppm_fresh = false;
}

#endif	/* PIOS_INCLUDE_RFM22B_RCVR */

/** 
  * @}
  * @}
  */

/**
 ******************************************************************************
 * @addtogroup PIOS PIOS Core hardware abstraction layer
 * @{
 * @addtogroup   PIOS_USB_RCTX USB RC Transmitter/Joystick Functions
 * @brief PIOS USB implementation for a HID Joystick
 * @notes      This implements transmitter/joystick emulation over HID reports
 * @{
 *
 * @file       pios_usb_rctx.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2012.
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

#if defined(PIOS_INCLUDE_USB_RCTX)

#include "pios_usb.h"
#include "pios_usb_rctx_priv.h"

/* STM32 USB Library Definitions */
#include "usb_lib.h"

#define PIOS_USB_RCTX_NUM_CHANNELS 8

enum pios_usb_rctx_dev_magic {
	PIOS_USB_RCTX_DEV_MAGIC = 0xAB98B745,
};

struct pios_usb_rctx_dev {
	enum pios_usb_rctx_dev_magic     magic;
	const struct pios_usb_rctx_cfg * cfg;

	uint32_t lower_id;

	struct {
		uint8_t id;
		uint16_t vals[PIOS_USB_RCTX_NUM_CHANNELS];
	} __attribute__((packed)) report;
};

static bool PIOS_USB_RCTX_validate(struct pios_usb_rctx_dev * usb_rctx_dev)
{
	return (usb_rctx_dev->magic == PIOS_USB_RCTX_DEV_MAGIC);
}

#if defined(PIOS_INCLUDE_FREERTOS)
static struct pios_usb_rctx_dev * PIOS_USB_RCTX_alloc(void)
{
	struct pios_usb_rctx_dev * usb_rctx_dev;

	usb_rctx_dev = (struct pios_usb_rctx_dev *)pvPortMalloc(sizeof(*usb_rctx_dev));
	if (!usb_rctx_dev) return(NULL);

	usb_rctx_dev->magic = PIOS_USB_RCTX_DEV_MAGIC;
	return(usb_rctx_dev);
}
#else
static struct pios_usb_rctx_dev pios_usb_rctx_devs[PIOS_USB_RCTX_MAX_DEVS];
static uint8_t pios_usb_rctx_num_devs;
static struct pios_usb_rctx_dev * PIOS_USB_RCTX_alloc(void)
{
	struct pios_usb_rctx_dev * usb_rctx_dev;

	if (pios_usb_rctx_num_devs >= PIOS_USB_RCTX_MAX_DEVS) {
		return (NULL);
	}

	usb_rctx_dev = &pios_usb_rctx_devs[pios_usb_rctx_num_devs++];
	usb_rctx_dev->magic = PIOS_USB_RCTX_DEV_MAGIC;

	return (usb_rctx_dev);
}
#endif

static void PIOS_USB_RCTX_EP_IN_Callback(void);
static void PIOS_USB_RCTX_SendReport(struct pios_usb_rctx_dev * usb_rctx_dev);

/* Need a better way to pull these in */
extern void (*pEpInt_IN[7])(void);

int32_t PIOS_USB_RCTX_Init(uint32_t * usbrctx_id, const struct pios_usb_rctx_cfg * cfg, uint32_t lower_id)
{
	PIOS_Assert(usbrctx_id);
	PIOS_Assert(cfg);

	struct pios_usb_rctx_dev * usb_rctx_dev;

	usb_rctx_dev = (struct pios_usb_rctx_dev *) PIOS_USB_RCTX_alloc();
	if (!usb_rctx_dev) goto out_fail;

	/* Bind the configuration to the device instance */
	usb_rctx_dev->cfg = cfg;
	usb_rctx_dev->lower_id = lower_id;

	/* Set the initial report buffer */
	memset(&usb_rctx_dev->report, 0, sizeof(usb_rctx_dev->report));

	pEpInt_IN[cfg->data_tx_ep - 1] = PIOS_USB_RCTX_EP_IN_Callback;

	*usbrctx_id = (uint32_t) usb_rctx_dev;

	return 0;

out_fail:
	return -1;
}

static void PIOS_USB_RCTX_SendReport(struct pios_usb_rctx_dev * usb_rctx_dev)
{
#if defined(PIOS_INCLUDE_FREERTOS)
	bool need_yield = false;
#endif	/* PIOS_INCLUDE_FREERTOS */

	usb_rctx_dev->report.id = 3; /* FIXME: shouldn't hard-code this report ID */

	UserToPMABufferCopy((uint8_t *) &usb_rctx_dev->report,
			GetEPTxAddr(usb_rctx_dev->cfg->data_tx_ep),
			sizeof(usb_rctx_dev->report));

	SetEPTxCount(usb_rctx_dev->cfg->data_tx_ep, sizeof(usb_rctx_dev->report));
	SetEPTxValid(usb_rctx_dev->cfg->data_tx_ep);

#if defined(PIOS_INCLUDE_FREERTOS)
	if (need_yield) {
		vPortYieldFromISR();
	}
#endif	/* PIOS_INCLUDE_FREERTOS */
}

static void PIOS_USB_RCTX_EP_IN_Callback(void)
{
	struct pios_usb_rctx_dev * usb_rctx_dev = (struct pios_usb_rctx_dev *)pios_usb_rctx_id;

	bool valid = PIOS_USB_RCTX_validate(usb_rctx_dev);
	PIOS_Assert(valid);

	if (!PIOS_USB_CheckAvailable(usb_rctx_dev->lower_id)) {
		return;
	}

	PIOS_USB_RCTX_SendReport(usb_rctx_dev);
}

void PIOS_USB_RCTX_Update(uint32_t usbrctx_id, const uint16_t channel[], const int16_t channel_min[], const int16_t channel_max[], uint8_t num_channels)
{
	struct pios_usb_rctx_dev * usb_rctx_dev = (struct pios_usb_rctx_dev *)usbrctx_id;

	bool valid = PIOS_USB_RCTX_validate(usb_rctx_dev);
	PIOS_Assert(valid);

	if (!PIOS_USB_CheckAvailable(usb_rctx_dev->lower_id)) {
		return;
	}

	for (uint8_t i = 0; 
	     i < PIOS_USB_RCTX_NUM_CHANNELS && i < num_channels;
	     i++) {
		int16_t min = channel_min[i];
		int16_t max = channel_max[i];
		uint16_t val = channel[i];

		if (channel_min[i] > channel_max[i]) {
			/* This channel is reversed, flip min and max */
			min = channel_max[i];
			max = channel_min[i];

			/* and flip val to be an offset from the lower end of the range */
			val = channel_min[i] - channel[i] + channel_max[i];
		}

		/* Scale channel linearly between min and max */
		if (min == max) {
			val = 0;
		} else {
			if (val < min) val = min;
			if (val > max) val = max;

			val = (val - min) * (65535 / (max - min));
		}

		usb_rctx_dev->report.vals[i] = val;
	}

	if (GetEPTxStatus(usb_rctx_dev->cfg->data_tx_ep) == EP_TX_VALID) {
		/* Endpoint is already transmitting */
		return;
	}

	PIOS_USB_RCTX_SendReport(usb_rctx_dev);
}

#endif	/* PIOS_INCLUDE_USB_RCTX */

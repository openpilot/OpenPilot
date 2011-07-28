/**
 ******************************************************************************
 * @addtogroup PIOS PIOS Core hardware abstraction layer
 * @{
 * @addtogroup   PIOS_GCSRCVR GCS Receiver Input Functions
 * @brief		Code to read the channels within the GCS Receiver UAVObject
 * @{
 *
 * @file       pios_gcsrcvr.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      GCS Input functions (STM32 dependent)
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

#if defined(PIOS_INCLUDE_GCSRCVR)

#include "pios_gcsrcvr_priv.h"

static GCSReceiverData gcsreceiverdata;

/* Provide a RCVR driver */
static int32_t PIOS_GCSRCVR_Get(uint32_t rcvr_id, uint8_t channel);

const struct pios_rcvr_driver pios_gcsrcvr_rcvr_driver = {
	.read = PIOS_GCSRCVR_Get,
};

static void gcsreceiver_updated(UAVObjEvent * ev)
{
	if (ev->obj == GCSReceiverHandle()) {
		GCSReceiverGet(&gcsreceiverdata);
	}
}

void PIOS_GCSRCVR_Init(void)
{
	/* Register uavobj callback */
	GCSReceiverConnectCallback (gcsreceiver_updated);
}

static int32_t PIOS_GCSRCVR_Get(uint32_t rcvr_id, uint8_t channel)
{
	if (channel >= GCSRECEIVER_CHANNEL_NUMELEM) {
		/* channel is out of range */
		return -1;
	}

	return (gcsreceiverdata.Channel[channel]);
}

#endif	/* PIOS_INCLUDE_GCSRCVR */

/** 
  * @}
  * @}
  */

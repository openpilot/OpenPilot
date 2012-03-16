/**
 ******************************************************************************
 * @addtogroup PIOS PIOS Core hardware abstraction layer
 * @{
 * @addtogroup   PIOS_RFM22B Radio Functions
 * @brief PIOS interface for RFM22B Radio
 * @{
 *
 * @file       pios_rfm22b_priv.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2012.
 * @brief      RFM22B private definitions.
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

#ifndef PIOS_RFM22B_PRIV_H
#define PIOS_RFM22B_PRIV_H

#include <pios.h>
#include "fifo_buffer.h"
#include "pios_rfm22b.h"

extern const struct pios_com_driver pios_rfm22b_com_driver;

struct pios_rfm22b_cfg {
	uint32_t sendTimeout;
	uint8_t minPacketSize;
	uint8_t txWinSize;
	uint8_t maxConnections;
	uint32_t id;
};

extern int32_t PIOS_RFM22B_Init(uint32_t *rfb22b_id, const struct pios_rfm22b_cfg *cfg);

#endif /* PIOS_RFM22B_PRIV_H */

/**
  * @}
  * @}
  */

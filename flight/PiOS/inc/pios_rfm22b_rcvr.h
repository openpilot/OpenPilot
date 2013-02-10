/**
 ******************************************************************************
 * @addtogroup PIOS PIOS Core hardware abstraction layer
 * @{
 * @addtogroup   PIOS RFM22B Receiver Input Functions
 * @{
 *
 * @file       pios_rfm22b_rcvr.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      RFM22B Receiver Input functions header.
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

#ifndef PIOS_RFM22B_RCVR_H

#define PIOS_RFM22B_RCVR_MAX_CHANNELS 12

extern const struct pios_rcvr_driver pios_rfm22b_rcvr_driver;

extern int32_t PIOS_RFM22B_RCVR_Init(uint32_t rcvr_id);

#define PIOS_RFM22B_RCVR_H

#endif /* PIOS_RFM22B_RCVR_H */

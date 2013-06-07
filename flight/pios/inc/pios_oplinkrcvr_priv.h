/**
 ******************************************************************************
 * @addtogroup PIOS PIOS Core hardware abstraction layer
 * @{
 * @addtogroup   PIOS_OPLinkRCVR OPLink Receiver Functions
 * @brief PIOS interface to read from OPLink receiver port
 * @{
 *
 * @file       pios_oplinkrcvr_priv.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2013.
 * @brief      OPLINK receiver private functions
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

#ifndef PIOS_OPLINKRCVR_PRIV_H
#define PIOS_OPLINKRCVR_PRIV_H

#include <pios.h>

extern const struct pios_rcvr_driver pios_oplinkrcvr_rcvr_driver;

extern int32_t PIOS_OPLinkRCVR_Init(uint32_t *oplinkrcvr_id);

#endif /* PIOS_OPLINKRCVR_PRIV_H */

/**
 * @}
 * @}
 */

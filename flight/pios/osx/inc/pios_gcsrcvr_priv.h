/**
 ******************************************************************************
 * @addtogroup PIOS PIOS Core hardware abstraction layer
 * @{
 * @addtogroup   PIOS_GCSRCVR GCS Receiver Functions
 * @brief PIOS interface to read from GCS receiver port
 * @{
 *
 * @file       pios_gcsrcvr_priv.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      GCS receiver private functions
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

#ifndef PIOS_GCSRCVR_PRIV_H
#define PIOS_GCSRCVR_PRIV_H

#include <pios.h>

#include "gcsreceiver.h"

extern const struct pios_rcvr_driver pios_gcsrcvr_rcvr_driver;

extern int32_t PIOS_GCSRCVR_Init(uint32_t *gcsrcvr_id);

#endif /* PIOS_GCSRCVR_PRIV_H */

/**
 * @}
 * @}
 */

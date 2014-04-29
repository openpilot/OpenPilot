/**
 ******************************************************************************
 * @addtogroup PIOS PIOS Core hardware abstraction layer
 * @{
 * @addtogroup   PIOS_MSP MSP functions
 * @brief PIOS interface for MSP
 * @{
 *
 * @file       pios_msp_priv.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2011.
 * @brief      MSP Private structures.
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

#ifndef PIOS_MSP_PRIV_H
#define PIOS_MSP_PRIV_H

#ifdef PIOS_INCLUDE_MSP

#include <pios.h>
#include <pios_stm32.h>

#define REQUEST_TIME_HI          50         // [ms]
#define REQUEST_TIME_LO         175         // [ms]

#define SERIALBUFFERSIZE        256

#define MSP_MASK_BOXARM         0x0001
#define MSP_MASK_BOXANGLE       0x0002
#define MSP_MASK_BOXHORIZON     0x0004

#define MSP_STATUS               101   //out message         cycletime & errors_count & sensor present & box activation & current setting number
#define MSP_RC                   105   //out message         8 rc chan and more
#define MSP_ATTITUDE             108   //out message         2 angles 1 heading


#endif /* PIOS_INCLUDE_MSP */

#endif /* PIOS_MSP_PRIV_H */

/**
 * @}
 * @}
 */

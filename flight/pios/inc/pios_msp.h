/**
 ******************************************************************************
 * @addtogroup PIOS PIOS Core hardware abstraction layer
 * @{
 * @addtogroup   PIOS_MSP MSP functions
 * @{
 *
 * @file       pios_msp.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2011.
 * @brief      MSP functions header.
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

#ifndef PIOS_MSP_H
#define PIOS_MSP_H

#ifdef PIOS_INCLUDE_MSP

/* Public Functions */
void MSPRequests(uint32_t port);
void MSPInputStream(uint8_t data);

uint8_t  MSPGetProfile(void);
uint8_t  MSPGetArmed(void);
uint8_t  MSPGetMode(void);
uint16_t MSPGetRC(uint8_t i);
uint16_t MSPGetAngle(uint8_t i);

#endif /* PIOS_INCLUDE_MSP */

#endif /* PIOS_MSP_H */

/**
 * @}
 * @}
 */

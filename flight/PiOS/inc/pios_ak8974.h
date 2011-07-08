/**
 ******************************************************************************
 * @addtogroup PIOS PIOS Core hardware abstraction layer
 * @{
 * @addtogroup PIOS_AK8794 AK8794 Magnetic Compass Functions
 * @brief Deals with the hardware interface to the magnetometers
 * @{
 *
 * @file       pios_ak8794.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2011.
 * @brief      AK8794 functions header.
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

#ifndef PIOS_AK8974_H
#define PIOS_AK8974_H

/* Public Functions */
extern void PIOS_AK8974_Init(void);
extern bool PIOS_AK8974_NewDataAvailable(void);
extern void PIOS_AK8974_ReadID(uint8_t out[4]);
extern void PIOS_AK8974_ReadMag(int16_t buffer[3]);
#endif /* PIOS_AK8974_H */

/** 
  * @}
  * @}
  */

/**
 ******************************************************************************
 *
 * @file       pios_hmc5843.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      HMC5843 functions header.
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

#ifndef PIOS_HMC5843_H
#define PIOS_HMC5843_H

/* BMP085 Addresses */
#define HMC5843_I2C_ADDR		0x3C


/* Local Types */


/* Global Variables */


/* Public Functions */
extern void PIOS_HMC5843_Init(void);
extern int32_t PIOS_HMC5843_Read(uint8_t address, uint8_t *buffer, uint8_t len);
extern int32_t PIOS_HMC5843_Write(uint8_t address, uint8_t buffer);

#endif /* PIOS_HMC5843_H */

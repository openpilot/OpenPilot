/**
 ******************************************************************************
 *
 * @file       ahrs_bl.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      Main AHRS_BL header.
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


#ifndef AHRS_BL_H
#define AHRS_BL_H


/* PIOS Includes */
#include <pios.h>

/** Start programming
returns: true if FLASH erased and ready to program
*/
bool StartProgramming(void);

/** Write a block to FLASH
buffer contains the data to be written
returns: true if FLASH programmed correctly
*/
bool WriteData(uint32_t offset, uint8_t *buffer, uint32_t size);

/** Read a block from FLASH
returns: true if FLASH read correctly.
Buffer is set to the read data
*/
bool ReadData(uint32_t offset, uint8_t *buffer, uint32_t size);



#endif /* AHRS_BL_H */

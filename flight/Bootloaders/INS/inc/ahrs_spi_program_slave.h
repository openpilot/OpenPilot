/**
 ******************************************************************************
 *
 * @file       ahrs_spi_program_slave.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      AHRS programming over SPI link - slave(AHRS) end.
 *
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

#ifndef AHRS_SPI_PROGRAM_SLAVE_H
#define AHRS_SPI_PROGRAM_SLAVE_H

/** Check if OpenPilot is trying to program AHRS
 * If so, it will program the FLASH then return
 * If not it just returns.
 */
void AhrsProgramReceive(uint32_t spi_id);
#endif //AHRS_PROGRAM_SLAVE_H

/**
 ******************************************************************************
 *
 * @file       ahrs_spi_program_master.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      AHRS programming over SPI link - master(OpenPilot) end.
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

#ifndef AHRS_PROGRAM_MASTER_H
#define AHRS_PROGRAM_MASTER_H

typedef enum {
	PROGRAM_ERR_OK, //OK
	PROGRAM_ERR_LINK, //comms error
	PROGRAM_ERR_FUNCTION,
//function failed
} PROGERR;

/** Connect to AHRS and request programming mode
 * returns: false if failed.
 */
bool AhrsProgramConnect(uint32_t spi_id);

/** Write data to AHRS
 * size must be between 1 and SPI_MAX_PROGRAM_DATA_SIZE
 * returns: error status
 */

PROGERR AhrsProgramWrite(uint32_t spi_id, uint32_t address, void * data,
		uint32_t size);

/** Read data from AHRS
 * size must be between 1 and SPI_MAX_PROGRAM_DATA_SIZE
 * returns: error status
 */

PROGERR AhrsProgramRead(uint32_t spi_id, uint32_t address, void * data,
		uint32_t size);

/** reboot AHRS
 * returns: error status
 */

PROGERR AhrsProgramReboot(uint32_t spi_id);

//TODO: Implement programming protocol

#endif //AHRS_PROGRAM_MASTER_H

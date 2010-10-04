/**
 ******************************************************************************
 *
 * @file       ahrs_spi_program_master.c
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


#include "ahrs_program_master.h"
#include "ahrs_program.h"
#include "pios_spi.h"

char connectTxBuf[SPI_PROGRAM_REQUEST_LENGTH] = { SPI_PROGRAM_REQUEST };

char connectRxBuf[SPI_PROGRAM_REQUEST_LENGTH];
#define MAX_CONNECT_TRIES 10

uint32_t AhrsProgramConnect(void)
{
	memset(connectRxBuf, 0, SPI_PROGRAM_REQUEST_LENGTH);
	for (int ct = 0; ct < MAX_CONNECT_TRIES; ct++) {
		uint32_t res = PIOS_SPI_TransferBlock(PIOS_SPI_OP, (uint8_t *) & connectTxBuf,
						      (uint8_t *) & connectRxBuf, SPI_PROGRAM_REQUEST_LENGTH, NULL);
		if (res == 0 && memcmp(connectRxBuf, SPI_PROGRAM_ACK, SPI_PROGRAM_REQUEST_LENGTH) == 0) {
			return (0);
		}

		vTaskDelay(1 / portTICK_RATE_MS);
	}
	return (-1);
}

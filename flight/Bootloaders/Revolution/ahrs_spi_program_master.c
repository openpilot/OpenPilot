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

#include "openpilot.h"
#include "ahrs_spi_program_master.h"
#include "ahrs_spi_program.h"
#include "pios_spi.h"

PROGERR TransferPacket(uint32_t spi_id, AhrsProgramPacket *txBuf,
		AhrsProgramPacket *rxBuf);

#define MAX_CONNECT_TRIES 500 //half a second
bool AhrsProgramConnect(uint32_t spi_id) {
	AhrsProgramPacket rxBuf;
	AhrsProgramPacket txBuf;
	memset(&rxBuf, 0, sizeof(AhrsProgramPacket));
	memcpy(&txBuf, SPI_PROGRAM_REQUEST, SPI_PROGRAM_REQUEST_LENGTH);
	for (int ct = 0; ct < MAX_CONNECT_TRIES; ct++) {
		PIOS_SPI_RC_PinSet(spi_id, 0);
		uint32_t res = PIOS_SPI_TransferBlock(spi_id, (uint8_t *) &txBuf,
				(uint8_t *) &rxBuf, SPI_PROGRAM_REQUEST_LENGTH + 1, NULL);
		PIOS_SPI_RC_PinSet(spi_id, 1);
		if (res == 0 && memcmp(&rxBuf, SPI_PROGRAM_ACK,
				SPI_PROGRAM_REQUEST_LENGTH) == 0) {
			return (true);
		}

		vTaskDelay(1 / portTICK_RATE_MS);
	}
	return (false);
}

PROGERR AhrsProgramWrite(uint32_t spi_id, uint32_t address, void * data,
		uint32_t size) {
	AhrsProgramPacket rxBuf;
	AhrsProgramPacket txBuf;
	memset(&rxBuf, 0, sizeof(AhrsProgramPacket));
	memcpy(txBuf.data, data, size);
	txBuf.size = size;
	txBuf.type = PROGRAM_WRITE;
	txBuf.address = address;
	PROGERR ret = TransferPacket(spi_id, &txBuf, &rxBuf);
	if (ret != PROGRAM_ERR_OK) {
		return (ret);
	}
	return (PROGRAM_ERR_OK);
}

PROGERR AhrsProgramRead(uint32_t spi_id, uint32_t address, void * data,
		uint32_t size) {
	AhrsProgramPacket rxBuf;
	AhrsProgramPacket txBuf;
	memset(&rxBuf, 0, sizeof(AhrsProgramPacket));
	txBuf.size = size;
	txBuf.type = PROGRAM_READ;
	txBuf.address = address;
	PROGERR ret = TransferPacket(spi_id, &txBuf, &rxBuf);
	if (ret != PROGRAM_ERR_OK) {
		return (ret);
	}
	memcpy(data, rxBuf.data, size);
	return (PROGRAM_ERR_OK);
}

PROGERR AhrsProgramReboot(uint32_t spi_id) {
	AhrsProgramPacket rxBuf;
	AhrsProgramPacket txBuf;
	memset(&rxBuf, 0, sizeof(AhrsProgramPacket));
	txBuf.type = PROGRAM_REBOOT;
	memcpy(txBuf.data, REBOOT_CONFIRMATION, REBOOT_CONFIRMATION_LENGTH);
	PROGERR ret = TransferPacket(spi_id, &txBuf, &rxBuf);
	//If AHRS has rebooted we will get comms errors
	if (ret == PROGRAM_ERR_LINK) {
		return (PROGRAM_ERR_OK);
	}
	return (PROGRAM_ERR_FUNCTION);
}

PROGERR TransferPacket(uint32_t spi_id, AhrsProgramPacket *txBuf,
		AhrsProgramPacket *rxBuf) {
	static uint32_t pktId = 0;
	pktId++;
	txBuf->packetId = pktId;
	txBuf->crc = GenerateCRC(txBuf);
	int ct = 0;
	for (; ct < MAX_CONNECT_TRIES; ct++) {
		PIOS_SPI_RC_PinSet(spi_id, 0);
		uint32_t res = PIOS_SPI_TransferBlock(spi_id, (uint8_t *) txBuf,
				(uint8_t *) rxBuf, sizeof(AhrsProgramPacket), NULL);
		PIOS_SPI_RC_PinSet(spi_id, 1);
		if (res == 0) {
			if (rxBuf->type != PROGRAM_NULL && rxBuf->crc == GenerateCRC(rxBuf)
					&& rxBuf->packetId == pktId) {
				break;
			}
		}

		vTaskDelay(1 / portTICK_RATE_MS);
	}
	if (ct == MAX_CONNECT_TRIES) {
		return (PROGRAM_ERR_LINK);
	}
	if (rxBuf->type != PROGRAM_ACK) {
		return (PROGRAM_ERR_FUNCTION);
	}
	return (PROGRAM_ERR_OK);
}


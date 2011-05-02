/**
 ******************************************************************************
 *
 * @file       ahrs_spi_program_slave.c
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

#include <stdint.h>
#include <string.h>

#include "pios_opahrs_proto.h"
#include "pios_spi.h"
#include "STM32103CB_AHRS.h"

#include "ahrs_bl.h"
#include "ahrs_spi_program_slave.h"
#include "ahrs_spi_program.h"

static AhrsProgramPacket txBuf;
static AhrsProgramPacket rxBuf;
static bool done = false;

static void ProcessPacket();

#define WAIT_IF_RECEIVING() while(!(GPIOB->IDR & GPIO_Pin_12)){}; //NSS must be high
//Number of crc failures to allow before giving up
#define PROGRAM_PACKET_TRIES 4

void AhrsProgramReceive(uint32_t spi_id) {
	done = false;
	memset(&txBuf, 0, sizeof(AhrsProgramPacket));
	//wait for a program request
	int count = PROGRAM_PACKET_TRIES;
	while (1) {
		WAIT_IF_RECEIVING();
		while ((PIOS_SPI_Busy(spi_id) != 0)) {
		};
		memset(&rxBuf, 'a', sizeof(AhrsProgramPacket));
		int32_t res = PIOS_SPI_TransferBlock(spi_id, NULL, (uint8_t*) &rxBuf,
				SPI_PROGRAM_REQUEST_LENGTH + 1, NULL);

		if (res == 0 && memcmp(&rxBuf, SPI_PROGRAM_REQUEST,
				SPI_PROGRAM_REQUEST_LENGTH) == 0) {
			break;
		}
		if (count-- == 0) {
			return;
		}
	}

	if (!StartProgramming()) {
		//Couldn't erase FLASH. Nothing we can do.
		return;
	}

	//send ack
	memcpy(&txBuf, SPI_PROGRAM_ACK, SPI_PROGRAM_REQUEST_LENGTH);
	WAIT_IF_RECEIVING();
	while (0 != PIOS_SPI_TransferBlock(spi_id, (uint8_t*) &txBuf, NULL,
			SPI_PROGRAM_REQUEST_LENGTH + 1, NULL)) {
	};

	txBuf.type = PROGRAM_NULL;

	while (!done) {
		WAIT_IF_RECEIVING();
		if (0 == PIOS_SPI_TransferBlock(spi_id, (uint8_t*) &txBuf,
				(uint8_t*) &rxBuf, sizeof(AhrsProgramPacket), NULL)) {

			uint32_t crc = GenerateCRC(&rxBuf);
			if (crc != rxBuf.crc || txBuf.packetId == rxBuf.packetId) {
				continue;
			}
			ProcessPacket();
			txBuf.packetId = rxBuf.packetId;
			txBuf.crc = GenerateCRC(&txBuf);
		}
	}
}

void ProcessPacket() {
	switch (rxBuf.type) {
	case PROGRAM_NULL:
		txBuf.type = PROGRAM_NULL;
		break;

	case PROGRAM_WRITE:
		if (WriteData(rxBuf.address, rxBuf.data, rxBuf.size)) {
			txBuf.type = PROGRAM_ACK;
			txBuf.size = rxBuf.size;
		} else {
			txBuf.type = PROGRAM_ERR;
		}
		break;

	case PROGRAM_READ:
		if (ReadData(rxBuf.address, txBuf.data, rxBuf.size)) {
			txBuf.type = PROGRAM_ACK;
			txBuf.size = rxBuf.size;
		} else {
			txBuf.type = PROGRAM_ERR;
		}
		break;

	case PROGRAM_REBOOT:
		if (0 == memcmp(rxBuf.data, REBOOT_CONFIRMATION,
				REBOOT_CONFIRMATION_LENGTH)) {
			done = true;
			txBuf.type = PROGRAM_ACK;
		} else {
			txBuf.type = PROGRAM_ERR;
		}
		break;

	default:
		txBuf.type = PROGRAM_ERR;
	}
}

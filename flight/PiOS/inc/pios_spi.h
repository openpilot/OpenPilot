/**
 ******************************************************************************
 *
 * @file       pios_spi.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * 	       Parts by Thorsten Klose (tk@midibox.org)
 * @brief      SPI functions header.
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

#ifndef PIOS_SPI_H
#define PIOS_SPI_H

/* Global types */
typedef enum {
	PIOS_SPI_PIN_DRIVER_STRONG = 0,
	PIOS_SPI_PIN_DRIVER_STRONG_OD = 1,
	PIOS_SPI_PIN_DRIVER_WEAK = 2,
	PIOS_SPI_PIN_DRIVER_WEAK_OD = 3,
} SPIPinDriverTypeDef;

typedef enum {
	PIOS_SPI_MODE_CLK0_PHASE0 = 0,
	PIOS_SPI_MODE_CLK0_PHASE1 = 1,
	PIOS_SPI_MODE_CLK1_PHASE0 = 2,
	PIOS_SPI_MODE_CLK1_PHASE1 = 3
} SPIModeTypeDef;

typedef enum {
	PIOS_SPI_PRESCALER_2 = 0,
	PIOS_SPI_PRESCALER_4 = 1,
	PIOS_SPI_PRESCALER_8 = 2,
	PIOS_SPI_PRESCALER_16 = 3,
	PIOS_SPI_PRESCALER_32 = 4,
	PIOS_SPI_PRESCALER_64 = 5,
	PIOS_SPI_PRESCALER_128 = 6,
	PIOS_SPI_PRESCALER_256 = 7
} SPIPrescalerTypeDef;


/* PUblic Functions */
extern int32_t PIOS_SPI_Init(void);
extern int32_t PIOS_SPI_IO_Init(uint8_t spi, SPIPinDriverTypeDef spi_pin_driver);
extern int32_t PIOS_SPI_TransferModeInit(uint8_t spi, SPIModeTypeDef spi_mode, SPIPrescalerTypeDef spi_prescaler);
extern int32_t PIOS_SPI_RC_PinSet(uint8_t spi, uint8_t pin_value);
extern int32_t PIOS_SPI_TransferByte(uint8_t spi, uint8_t b);
extern int32_t PIOS_SPI_TransferBlock(uint8_t spi, uint8_t *send_buffer, uint8_t *receive_buffer, uint16_t len, void *callback);

#endif /* PIOS_SSPI_H */

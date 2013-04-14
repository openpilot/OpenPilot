/**
 ******************************************************************************
 * @addtogroup PIOS PIOS Core hardware abstraction layer
 * @{
 * @addtogroup   PIOS_SPI SPI Functions
 * @{
 *
 * @file       pios_spi.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
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
	PIOS_SPI_PRESCALER_2 = 0,
	PIOS_SPI_PRESCALER_4 = 1,
	PIOS_SPI_PRESCALER_8 = 2,
	PIOS_SPI_PRESCALER_16 = 3,
	PIOS_SPI_PRESCALER_32 = 4,
	PIOS_SPI_PRESCALER_64 = 5,
	PIOS_SPI_PRESCALER_128 = 6,
	PIOS_SPI_PRESCALER_256 = 7
} SPIPrescalerTypeDef;

/* Public Functions */
extern int32_t PIOS_SPI_SetClockSpeed(uint32_t spi_id, SPIPrescalerTypeDef spi_prescaler);
extern int32_t PIOS_SPI_RC_PinSet(uint32_t spi_id, uint32_t slave_id, uint8_t pin_value);
extern int32_t PIOS_SPI_TransferByte(uint32_t spi_id, uint8_t b);
extern int32_t PIOS_SPI_TransferBlock(uint32_t spi_id, const uint8_t *send_buffer, uint8_t *receive_buffer, uint16_t len, void *callback);
extern int32_t PIOS_SPI_Busy(uint32_t spi_id);
extern int32_t PIOS_SPI_ClaimBus(uint32_t spi_id);
extern int32_t PIOS_SPI_ClaimBusISR(uint32_t spi_id);
extern int32_t PIOS_SPI_ReleaseBus(uint32_t spi_id);
extern void    PIOS_SPI_IRQ_Handler(uint32_t spi_id);
extern void    PIOS_SPI_SetPrescalar(uint32_t spi_id, uint32_t prescalar);

#endif /* PIOS_SPI_H */

/**
 * @}
 * @}
 */

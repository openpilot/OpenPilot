/**
 ******************************************************************************
 * @addtogroup PIOS PIOS Core hardware abstraction layer
 * @{
 * @addtogroup   PIOS_OVERO Overo Functions
 * @{
 *
 * @file       pios_overo.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2012.
 * @brief      Overo functions header.
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

#ifndef PIOS_OVERO_H
#define PIOS_OVERO_H

#include <pios.h>
#include <pios_stm32.h>

struct pios_overo_cfg {
	SPI_TypeDef *regs;
	uint32_t remap;				/* GPIO_Remap_* or GPIO_AF_* */
	SPI_InitTypeDef init;
	bool use_crc;
	struct stm32_dma dma;
	struct stm32_gpio sclk;
	struct stm32_gpio miso;
	struct stm32_gpio mosi;
	uint32_t slave_count;
	struct stm32_gpio ssel[];
};

struct pios_overo_dev {
	const struct pios_overo_cfg * cfg;
	void (*callback) (uint8_t, uint8_t);
	uint8_t tx_dummy_byte;
	uint8_t rx_dummy_byte;
#if defined(PIOS_INCLUDE_FREERTOS)
	xSemaphoreHandle busy;
#else
	uint8_t busy;
#endif
};

extern int32_t PIOS_OVERO_Init(uint32_t * overo_id, const struct pios_overo_cfg * cfg);
extern int32_t PIOS_OVERO_SwapBuffer(uint32_t overo_id, const uint8_t *send_buffer, uint8_t *receive_buffer, uint16_t len, void *callback);

#endif /* PIOS_OVERO_H */

/**
 * @}
 * @}
 */

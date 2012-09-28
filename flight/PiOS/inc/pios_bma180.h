/**
 ******************************************************************************
 * @addtogroup PIOS PIOS Core hardware abstraction layer
 * @{
 * @addtogroup PIOS_BMA180 BMA180 Functions
 * @brief Deals with the hardware interface to the BMA180 3-axis accelerometer
 * @{
 *
 * @file       pios_bma180.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2012.
 * @brief      PiOS BMA180 digital accelerometer driver.
 *                 - Driver for the BMA180 digital accelerometer on the SPI bus.
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

#include "fifo_buffer.h"

#ifndef PIOS_BMA180_H
#define PIOS_BMA180_H

#include <pios.h>

/* BMA180 Addresses */
#define BMA_CHIPID_ADDR   0x00
#define BMA_VERSION_ADDR  0x00
#define BMA_X_LSB_ADDR    0x02
#define BMA_Y_LSB_ADDR    0x04
#define BMA_Z_LSB_ADDR    0x06
#define BMA_WE_ADDR       0x0D
#define BMA_RESET         0x10
#define BMA_BW_ADDR       0x20
#define BMA_RANGE_ADDR    0x35
#define BMA_OFFSET_LSB1   0x35
#define BMA_GAIN_Y        0x33
#define BMA_CTRREG3       0x21
#define BMA_CTRREG0       0x0D

#define BMA_RESET_CODE    0x6B

/* Accel range  */
#define BMA_RANGE_MASK    0x0E          
#define BMA_RANGE_SHIFT   1
enum bma180_range { BMA_RANGE_1G = 0x00,
	BMA_RANGE_1_5G = 0x01,
	BMA_RANGE_2G = 0x02,
	BMA_RANGE_3G = 0x03,
	BMA_RANGE_4G = 0x04,
	BMA_RANGE_8G = 0x05,
	BMA_RANGE_16G = 0x06
};

/* Measurement bandwidth */
#define BMA_BW_MASK       0xF0
#define BMA_BW_SHIFT      4
enum bma180_bandwidth { BMA_BW_10HZ = 0x00,
	BMA_BW_20HZ = 0x01,
	BMA_BW_40HZ = 0x02,
	BMA_BW_75HZ = 0x03,
	BMA_BW_150HZ = 0x04,
	BMA_BW_300HZ = 0x05,
	BMA_BW_600HZ = 0x06,
	BMA_BW_1200HZ =0x07,
	BMA_BW_HP1HZ = 0x08,    // High-pass, 1 Hz
	BMA_BW_BP0_300HZ = 0x09 // Band-pass, 0.3Hz-300Hz
};

#define BMA_NEW_DAT_INT   0x02

struct pios_bma180_data {
	int16_t x;
	int16_t y;
	int16_t z;
	int8_t temperature;
};


struct pios_bma180_cfg {
	const struct pios_exti_cfg * exti_cfg; /* Pointer to the EXTI configuration */
	enum bma180_bandwidth bandwidth;
	enum bma180_range range;
};

/* Public Functions */
extern int32_t PIOS_BMA180_Init(uint32_t spi_id, uint32_t slave_num, const struct pios_bma180_cfg * cfg);
extern void PIOS_BMA180_Attach(uint32_t spi_id);
extern float PIOS_BMA180_GetScale();
extern int32_t PIOS_BMA180_ReadFifo(struct pios_bma180_data * buffer);
extern int32_t PIOS_BMA180_ReadAccels(struct pios_bma180_data * data);
extern int32_t PIOS_BMA180_Test();
extern bool PIOS_BMA180_IRQHandler();

#endif /* PIOS_BMA180_H */

/**
 * @}
 * @}
 */

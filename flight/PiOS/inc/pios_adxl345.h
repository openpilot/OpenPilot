/**
 ******************************************************************************
 * @addtogroup PIOS PIOS Core hardware abstraction layer
 * @{
 * @addtogroup PIOS_ADXL345 ADXL345 Functions
 * @brief Data from the ADXL345 sensors
 * @{
 * @file       pios_adxl345.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2012.
 * @brief      PiOS SPI ADXL345 
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
#ifndef PIOS_ADXL345_H
#define PIOS_ADXL345_H

// Defined by data rate, not BW

#define ADXL_READ_BIT      0x80
#define ADXL_MULTI_BIT     0x40

#define ADXL_WHOAMI        0x00
#define ADXL_DEVICE_ID     0xE5
#define ADXL_X0_ADDR       0x32
#define ADXL_FIFOSTATUS_ADDR 0x39

#define ADXL_RATE_ADDR     0x2C
#define ADXL_RATE_100      0x0A
#define ADXL_RATE_200      0x0B
#define ADXL_RATE_400      0x0C
#define ADXL_RATE_800      0x0D
#define ADXL_RATE_1600     0x0E
#define ADXL_RATE_3200     0x0F

#define ADXL_POWER_ADDR    0x2D
#define ADXL_MEAURE        0x08

#define ADXL_FORMAT_ADDR   0x31
#define ADXL_FULL_RES      0x08
#define ADXL_4WIRE         0x00
#define ADXL_RANGE_2G      0x00
#define ADXL_RANGE_4G      0x01
#define ADXL_RANGE_8G      0x02
#define ADXL_RANGE_16G     0x03

#define ADXL_FIFO_ADDR     0x38
#define ADXL_FIFO_STREAM   0x80


struct pios_adxl345_data {
	int16_t x;
	int16_t y;
	int16_t z;
};

int32_t PIOS_ADXL345_SelectRate(uint8_t rate); 
int32_t PIOS_ADXL345_SetRange(uint8_t range);
int32_t PIOS_ADXL345_Init(uint32_t spi_id, uint32_t slave_num);
uint8_t PIOS_ADXL345_Read(struct pios_adxl345_data * data);
int32_t PIOS_ADXL345_FifoElements();
int32_t PIOS_ADXL345_Test();

#endif

/** 
 * @}
 * @}
 */

/**
 ******************************************************************************
 * @addtogroup PIOS PIOS Core hardware abstraction layer
 * @{
 * @addtogroup PIOS_BMA180 BMA180 Functions
 * @brief Deals with the hardware interface to the BMA180 3-axis accelerometer
 * @{
 *
 * @file       pios_bma180.h
 * @author     David "Buzz" Carlson (buzz@chebuzz.com)
 * 				The OpenPilot Team, http://www.openpilot.org Copyright (C) 2011.
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

#ifndef PIOS_BMA180_H
#define PIOS_BMA180_H

/* BMA180 Addresses */
#define BMA_CHIPID_ADDR   0x00
#define BMA_VERSION_ADDR  0x00
#define BMA_X_LSB_ADDR    0x02
#define BMA_Y_LSB_ADDR    0x04
#define BMA_Z_LSB_ADDR    0x06
#define BMA_WE_ADDR       0x0D
#define BMA_BW_ADDR       0x20
#define BMA_RANGE_ADDR    0x35

/* Accel range  */
#define BMA_RANGE_1G      0x00		// +/- 1G ADC resolution 0.13 mg/LSB
#define BMA_RANGE_1_5G    0x01		// +/- 1.5G ADC resolution 0.19 mg/LSB
#define BMA_RANGE_2G      0x02		// +/- 2G ADC resolution 0.25 mg/LSB    *** default ***
#define BMA_RANGE_3G      0x03		// +/- 3G ADC resolution 0.38 mg/LSB
#define BMA_RANGE_4G      0x04		// +/- 4G ADC resolution 0.50 mg/LSB
#define BMA_RANGE_8G      0x05		// +/- 8G ADC resolution 0.99 mg/LSB
#define BMA_RANGE_16G     0x06		// +/- 16G ADC resolution 1.98 mg/LSB

/* Measurement bandwidth */
#define BMA_BW_10HZ       0x00
#define BMA_BW_20HZ       0x01
#define BMA_BW_40HZ       0x02
#define BMA_BW_75HZ       0x03
#define BMA_BW_150HZ      0x04      // *** default ***
#define BMA_BW_300HZ      0x05
#define BMA_BW_600HZ      0x06
#define BMA_BW_1200HZ     0x07
#define BMA_BW_HP1HZ      0x08		// High-pass, 1Hz
#define BMA_BW_BP0_300HZ  0x09      // Band-pass, 0.3Hz-300Hz

struct pios_bma180_data {
	int16_t x;
	int16_t y;
	int16_t z;
};

/* Public Functions */
void PIOS_BMA180_WriteEnable(uint8_t _we);
uint8_t PIOS_BMA180_GetReg(uint8_t reg);
void PIOS_BMA180_SetReg(uint8_t reg, uint8_t data);
void PIOS_BMA180_Attach(uint32_t spi_id);
void PIOS_BMA180_Init();
uint8_t PIOS_BMA180_Read(struct pios_bma180_data * data);
uint8_t PIOS_BMA180_Test();

#endif /* PIOS_BMA180_H */

/**
 * @}
 * @}
 */

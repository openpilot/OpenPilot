/**
 ******************************************************************************
 * @addtogroup PIOS PIOS Core hardware abstraction layer
 * @{
 * @addtogroup PIOS_BMP085 BMP085 Functions
 * @brief Hardware functions to deal with the altitude pressure sensor
 * @{
 *
 * @file       pios_bmp085.h  
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2012.
 * @brief      BMP085 functions header.
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

#ifndef PIOS_BMP085_H
#define PIOS_BMP085_H

#include <pios.h>

/* BMP085 Addresses */
#define BMP085_I2C_ADDR			0x77
#define BMP085_CALIB_ADDR		0xAA
#define BMP085_CALIB_LEN		22
#define BMP085_CTRL_ADDR		0xF4
#define BMP085_OVERSAMPLING		PIOS_BMP085_OVERSAMPLING
#define BMP085_PRES_ADDR		(0x34 + (BMP085_OVERSAMPLING << 6))
#define BMP085_TEMP_ADDR		0x2E
#define BMP085_ADC_MSB			0xF6
#define BMP085_P0			101325

/* Local Types */
typedef struct {
	int16_t AC1;
	int16_t AC2;
	int16_t AC3;
	uint16_t AC4;
	uint16_t AC5;
	uint16_t AC6;
	int16_t B1;
	int16_t B2;
	int16_t MB;
	int16_t MC;
	int16_t MD;
} BMP085CalibDataTypeDef;

typedef enum {
	PressureConv,
	TemperatureConv
} ConversionTypeTypeDef;

/* Global Variables */
#if defined(PIOS_INCLUDE_FREERTOS)
extern xSemaphoreHandle PIOS_BMP085_EOC;
#else
extern int32_t PIOS_BMP085_EOC;
#endif


struct pios_bmp085_cfg {
	struct stm32_gpio drdy;
	struct stm32_gpio xclr;
	struct stm32_exti eoc_exti;
	struct stm32_irq eoc_irq;
	uint32_t oversampling;
};

/* Public Functions */
extern void PIOS_BMP085_Init(const struct pios_bmp085_cfg * cfg);
extern int32_t PIOS_BMP085_StartADC(ConversionTypeTypeDef Type);
extern int32_t PIOS_BMP085_ReadADC(void);
extern int16_t PIOS_BMP085_GetTemperature(void);
extern int32_t PIOS_BMP085_GetPressure(void);
extern int32_t PIOS_BMP085_Test();

#endif /* PIOS_BMP085_H */

/** 
  * @}
  * @}
  */

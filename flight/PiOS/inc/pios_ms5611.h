/**
 ******************************************************************************
 * @addtogroup PIOS PIOS Core hardware abstraction layer
 * @{
 * @addtogroup PIOS_MS5611 MS5611 Functions
 * @brief Hardware functions to deal with the altitude pressure sensor
 * @{
 *
 * @file       pios_ms5611.h  
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2012.
 * @brief      MS5611 functions header.
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

#ifndef PIOS_MS5611_H
#define PIOS_MS5611_H

#include <pios.h>

/* BMP085 Addresses */
#define MS5611_I2C_ADDR	        0x77
#define MS5611_RESET            0x1E
#define MS5611_CALIB_ADDR		0xA2  /* First sample is factory stuff */
#define MS5611_CALIB_LEN		16
#define MS5611_ADC_READ		    0x00
#define MS5611_PRES_ADDR		0x40
#define MS5611_TEMP_ADDR		0x50
#define MS5611_ADC_MSB			0xF6
#define MS5611_P0			    101.3250f

/* Local Types */
typedef struct {
	uint16_t C[6];
} MS5611CalibDataTypeDef;

typedef enum {
	PressureConv,
	TemperatureConv
} ConversionTypeTypeDef;

struct pios_ms5611_cfg {
	uint32_t oversampling;
};

enum pios_ms5611_osr {
	MS5611_OSR_256   = 0,
	MS5611_OSR_512   = 2,
	MS5611_OSR_1024  = 4,
	MS5611_OSR_2048  = 6,
	MS5611_OSR_4096  = 8,
};

/* Public Functions */
extern void PIOS_MS5611_Init(const struct pios_ms5611_cfg * cfg, int32_t i2c_device);
extern int32_t PIOS_MS5611_StartADC(ConversionTypeTypeDef Type);
extern int32_t PIOS_MS5611_ReadADC(void);
extern float PIOS_MS5611_GetTemperature(void);
extern float PIOS_MS5611_GetPressure(void);
extern int32_t PIOS_MS5611_Test();
extern int32_t PIOS_MS5611_GetDelay();

#endif /* PIOS_MS5611_H */

/** 
  * @}
  * @}
  */

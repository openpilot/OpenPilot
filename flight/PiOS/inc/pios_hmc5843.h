/**
 ******************************************************************************
 *
 * @file       pios_hmc5843.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      HMC5843 functions header.
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

#ifndef PIOS_HMC5843_H
#define PIOS_HMC5843_H

/* BMP085 Addresses */
#define PIOS_HMC5843_I2C_ADDR			0x3C
#define PIOS_HMC5843_CONFIG_REG_A		(uint8_t)0x00
#define PIOS_HMC5843_CONFIG_REG_B		(uint8_t)0x01
#define PIOS_HMC5843_MODE_REG			(uint8_t)0x02
#define PIOS_HMC5843_DATAOUT_XMSB_REG		0x03
#define PIOS_HMC5843_DATAOUT_XLSB_REG		0x04
#define PIOS_HMC5843_DATAOUT_YMSB_REG		0x05
#define PIOS_HMC5843_DATAOUT_YLSB_REG		0x06
#define PIOS_HMC5843_DATAOUT_ZMSB_REG		0x07
#define PIOS_HMC5843_DATAOUT_ZLSB_REG		0x08
#define PIOS_HMC5843_DATAOUT_STATUS_REG		0x09
#define PIOS_HMC5843_DATAOUT_IDA_REG		0x0A
#define PIOS_HMC5843_DATAOUT_IDB_REG		0x0B
#define PIOS_HMC5843_DATAOUT_IDC_REG		0x0C

/* Output Data Rate */
#define PIOS_HMC5843_ODR_05			0x00
#define PIOS_HMC5843_ODR_1			0x04
#define PIOS_HMC5843_ODR_2			0x08
#define PIOS_HMC5843_ODR_5			0x0C
#define PIOS_HMC5843_ODR_10			0x10
#define PIOS_HMC5843_ODR_20			0x14
#define PIOS_HMC5843_ODR_50			0x18

/* Measure configuration */
#define PIOS_HMC5843_MEASCONF_NORMAL		0x00
#define PIOS_HMC5843_MEASCONF_BIAS_POS		0x01
#define PIOS_HMC5843_MEASCONF_BIAS_NEG		0x02

/* Gain settings */
#define PIOS_HMC5843_GAIN_0_7			0x00
#define PIOS_HMC5843_GAIN_1			0x20
#define PIOS_HMC5843_GAIN_1_5			0x40
#define PIOS_HMC5843_GAIN_2			0x60
#define PIOS_HMC5843_GAIN_3_2			0x80
#define PIOS_HMC5843_GAIN_3_8			0xA0
#define PIOS_HMC5843_GAIN_4_5			0xC0
#define PIOS_HMC5843_GAIN_6_5			0xE0

/* Modes */
#define PIOS_HMC5843_MODE_CONTINUOS		0x00
#define PIOS_HMC5843_MODE_SINGLE		0x01
#define PIOS_HMC5843_MODE_IDLE			0x02
#define PIOS_HMC5843_MODE_SLEEP			0x02

/* Sensitivity Conversion Values */
#define PIOS_HMC5843_Sensitivity_0_7Ga		1602	// LSB/Ga
#define PIOS_HMC5843_Sensitivity_1Ga		1300	// LSB/Ga
#define PIOS_HMC5843_Sensitivity_1_5Ga		970	// LSB/Ga
#define PIOS_HMC5843_Sensitivity_2Ga		780	// LSB/Ga
#define PIOS_HMC5843_Sensitivity_3_2Ga		530	// LSB/Ga
#define PIOS_HMC5843_Sensitivity_3_8Ga		460	// LSB/Ga
#define PIOS_HMC5843_Sensitivity_4_5Ga		390	// LSB/Ga
#define PIOS_HMC5843_Sensitivity_6_5Ga		280	// LSB/Ga  --> NOT RECOMMENDED

/* Global Types */
typedef struct {
	uint8_t M_ODR;       /* OUTPUT DATA RATE --> here below the relative define (See datasheet page 11 for more details) */
	uint8_t Meas_Conf;   /* Measurement Configuration,: Normal, positive bias, or negative bias --> here below the relative define */
	uint8_t Gain;        /* Gain Configuration, select the full scale --> here below the relative define (See datasheet page 11 for more details) */
	uint8_t Mode;
} PIOS_HMC5843_ConfigTypeDef;

/* Global Variables */


/* Public Functions */
extern void PIOS_HMC5843_Init(void);
extern void PIOS_HMC5843_Config(PIOS_HMC5843_ConfigTypeDef *HMC5843_Config_Struct);
extern void PIOS_HMC5843_ReadMag(int16_t *out);
extern void PIOS_HMC5843_ReadID(uint8_t *out);
extern int32_t PIOS_HMC5843_Read(uint8_t address, uint8_t *buffer, uint8_t len);
extern int32_t PIOS_HMC5843_Write(uint8_t address, uint8_t buffer);

#endif /* PIOS_HMC5843_H */

/**
 ******************************************************************************
 * @addtogroup PIOS PIOS Core hardware abstraction layer
 * @{
 * @addtogroup PIOS_HMC5843 HMC5843 Functions
 * @brief Deals with the hardware interface to the magnetometers
 * @{
 *
 * @file       pios_hmc5843.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      HMC5843 Magnetic Sensor Functions from AHRS
 * @see        The GNU Public License (GPL) Version 3
 *
 ******************************************************************************
 */
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

/* Project Includes */
#include "pios.h"

#if defined(PIOS_INCLUDE_HMC5843)

#include <pios_exti.h>

/* HMC5843 Addresses */
#define PIOS_HMC5843_I2C_ADDR			0x1E
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
#define PIOS_HMC5843_MODE_CONTINUOUS		0x00
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

/* Global Variables */

/* Local Types */
typedef struct {
	uint8_t M_ODR;		/* OUTPUT DATA RATE --> here below the relative define (See datasheet page 11 for more details) */
	uint8_t Meas_Conf;	/* Measurement Configuration,: Normal, positive bias, or negative bias --> here below the relative define */
	uint8_t Gain;		/* Gain Configuration, select the full scale --> here below the relative define (See datasheet page 11 for more details) */
	uint8_t Mode;
} PIOS_HMC5843_ConfigTypeDef;

/* Local Variables */
volatile bool pios_hmc5843_data_ready;

static void PIOS_HMC5843_Config(PIOS_HMC5843_ConfigTypeDef * HMC5843_Config_Struct);
static bool PIOS_HMC5843_Read(uint8_t address, uint8_t * buffer, uint8_t len);
static bool PIOS_HMC5843_Write(uint8_t address, uint8_t buffer);

void PIOS_HMC5843_EndOfConversion (void)
{
	pios_hmc5843_data_ready = true;
}

static const struct pios_exti_cfg pios_exti_hmc5843_cfg __exti_config = {
	.vector = PIOS_HMC5843_EndOfConversion,
	.line = PIOS_HMC5843_DRDY_EXTI_LINE,
	.pin = {
		.gpio = PIOS_HMC5843_DRDY_GPIO_PORT,
		.init = {
			.GPIO_Pin = PIOS_HMC5843_DRDY_GPIO_PIN,
			.GPIO_Mode = GPIO_Mode_IN_FLOATING,
		},
	},
	.irq = {
		.init = {
			.NVIC_IRQChannel = PIOS_HMC5843_DRDY_IRQn,
			.NVIC_IRQChannelPreemptionPriority = PIOS_HMC5843_DRDY_PRIO,
			.NVIC_IRQChannelSubPriority = 0,
			.NVIC_IRQChannelCmd = ENABLE,
		},
	},
	.exti = {
		.init = {
			.EXTI_Line = PIOS_HMC5843_DRDY_EXTI_LINE,
			.EXTI_Mode = EXTI_Mode_Interrupt,
			.EXTI_Trigger = EXTI_Trigger_Rising,
			.EXTI_LineCmd = ENABLE,
		},
	},
};

/**
  * @brief Initialise the HMC5843 sensor
  */
void PIOS_HMC5843_Init(void)
{
	/* Enable DRDY GPIO clock */
	RCC_APB2PeriphClockCmd(PIOS_HMC5843_DRDY_CLK | RCC_APB2Periph_AFIO, ENABLE);

	PIOS_EXTI_Init(&pios_exti_hmc5843_cfg);

	/* Configure the HMC5843 Sensor */
	PIOS_HMC5843_ConfigTypeDef HMC5843_InitStructure;
	HMC5843_InitStructure.M_ODR = PIOS_HMC5843_ODR_10;
	HMC5843_InitStructure.Meas_Conf = PIOS_HMC5843_MEASCONF_NORMAL;
	HMC5843_InitStructure.Gain = PIOS_HMC5843_GAIN_2;
	HMC5843_InitStructure.Mode = PIOS_HMC5843_MODE_CONTINUOUS;
	PIOS_HMC5843_Config(&HMC5843_InitStructure);

	pios_hmc5843_data_ready = false;
}

/**
* Initialise the HMC5843 sensor
*
* CTRL_REGA: Control Register A
* Read Write
* Default value: 0x10
* 7:5  0   These bits must be cleared for correct operation.
* 4:2 DO2-DO0: Data Output Rate Bits
*             DO2 |  DO1 |  DO0 |   Minimum Data Output Rate (Hz)
*            ------------------------------------------------------
*              0  |  0   |  0   |            0.5
*              0  |  0   |  1   |            1
*              0  |  1   |  0   |            2
*              0  |  1   |  1   |            5
*              1  |  0   |  0   |           10 (default)
*              1  |  0   |  1   |           20
*              1  |  1   |  0   |           50
*              1  |  1   |  1   |           Not Used
* 1:0 MS1-MS0: Measurement Configuration Bits
*             MS1 | MS0 |   MODE
*            ------------------------------
*              0  |  0   |  Normal
*              0  |  1   |  Positive Bias
*              1  |  0   |  Negative Bias
*              1  |  1   |  Not Used
*
* CTRL_REGB: Control RegisterB
* Read Write
* Default value: 0x20
* 7:5 GN2-GN0: Gain Configuration Bits.
*             GN2 |  GN1 |  GN0 |   Mag Input   | Gain       | Output Range
*                 |      |      |  Range[Ga]    | [LSB/mGa]  |
*            ------------------------------------------------------
*              0  |  0   |  0   |  ±0.7Ga       |   1620     | 0xF8000x07FF (-2048:2047)
*              0  |  0   |  1   |  ±1.0Ga (def) |   1300     | 0xF8000x07FF (-2048:2047)
*              0  |  1   |  0   |  ±1.5Ga       |   970      | 0xF8000x07FF (-2048:2047)
*              0  |  1   |  1   |  ±2.0Ga       |   780      | 0xF8000x07FF (-2048:2047)
*              1  |  0   |  0   |  ±3.2Ga       |   530      | 0xF8000x07FF (-2048:2047)
*              1  |  0   |  1   |  ±3.8Ga       |   460      | 0xF8000x07FF (-2048:2047)
*              1  |  1   |  0   |  ±4.5Ga       |   390      | 0xF8000x07FF (-2048:2047)
*              1  |  1   |  1   |  ±6.5Ga       |   280      | 0xF8000x07FF (-2048:2047)
*                               |Not recommended|
*
* 4:0 CRB4-CRB: 0 This bit must be cleared for correct operation.
*
* _MODE_REG: Mode Register
* Read Write
* Default value: 0x02
* 7:2  0   These bits must be cleared for correct operation.
* 1:0 MD1-MD0: Mode Select Bits
*             MS1 | MS0 |   MODE
*            ------------------------------
*              0  |  0   |  Continuous-Conversion Mode.
*              0  |  1   |  Single-Conversion Mode
*              1  |  0   |  Negative Bias
*              1  |  1   |  Sleep Mode
*/
static void PIOS_HMC5843_Config(PIOS_HMC5843_ConfigTypeDef * HMC5843_Config_Struct)
{
	uint8_t CRTLA = 0x00;
	uint8_t CRTLB = 0x00;
	uint8_t MODE = 0x00;

	CRTLA |= (uint8_t) (HMC5843_Config_Struct->M_ODR | HMC5843_Config_Struct->Meas_Conf);
	CRTLB |= (uint8_t) (HMC5843_Config_Struct->Gain);
	MODE |= (uint8_t) (HMC5843_Config_Struct->Mode);

	// CRTL_REGA
	while (!PIOS_HMC5843_Write(PIOS_HMC5843_CONFIG_REG_A, CRTLA)) ;

	// CRTL_REGB
	while (!PIOS_HMC5843_Write(PIOS_HMC5843_CONFIG_REG_B, CRTLB)) ;

	// Mode register
	while (!PIOS_HMC5843_Write(PIOS_HMC5843_MODE_REG, MODE)) ;
}

/**
* Read the magnetic readings from the sensor
*/
void PIOS_HMC5843_ReadMag(int16_t out[3])
{
	uint8_t buffer[6];
	uint8_t crtlB;

	pios_hmc5843_data_ready = false;

	while (!PIOS_HMC5843_Read(PIOS_HMC5843_CONFIG_REG_B, &crtlB, 1)) ;
	while (!PIOS_HMC5843_Read(PIOS_HMC5843_DATAOUT_XMSB_REG, buffer, 6)) ;

	switch (crtlB & 0xE0) {
	case 0x00:
		for (int i = 0; i < 3; i++)
			out[i] = ((int16_t) ((uint16_t) buffer[2 * i] << 8)
				  + buffer[2 * i + 1]) * 1000 / PIOS_HMC5843_Sensitivity_0_7Ga;
		break;
	case 0x20:
		for (int i = 0; i < 3; i++)
			out[i] = ((int16_t) ((uint16_t) buffer[2 * i] << 8)
				  + buffer[2 * i + 1]) * 1000 / PIOS_HMC5843_Sensitivity_1Ga;
		break;
	case 0x40:
		for (int i = 0; i < 3; i++)
			out[i] = (int16_t) (((uint16_t) buffer[2 * i] << 8)
					    + buffer[2 * i + 1]) * 1000 / PIOS_HMC5843_Sensitivity_1_5Ga;
		break;
	case 0x60:
		for (int i = 0; i < 3; i++)
			out[i] = (int16_t) (((uint16_t) buffer[2 * i] << 8)
					    + buffer[2 * i + 1]) * 1000 / PIOS_HMC5843_Sensitivity_2Ga;
		break;
	case 0x80:
		for (int i = 0; i < 3; i++)
			out[i] = (int16_t) (((uint16_t) buffer[2 * i] << 8)
					    + buffer[2 * i + 1]) * 1000 / PIOS_HMC5843_Sensitivity_3_2Ga;
		break;
	case 0xA0:
		for (int i = 0; i < 3; i++)
			out[i] = (int16_t) (((uint16_t) buffer[2 * i] << 8)
					    + buffer[2 * i + 1]) * 1000 / PIOS_HMC5843_Sensitivity_3_8Ga;
		break;
	case 0xC0:
		for (int i = 0; i < 3; i++)
			out[i] = (int16_t) (((uint16_t) buffer[2 * i] << 8)
					    + buffer[2 * i + 1]) * 1000 / PIOS_HMC5843_Sensitivity_4_5Ga;
		break;
	case 0xE0:
		for (int i = 0; i < 3; i++)
			out[i] = (int16_t) (((uint16_t) buffer[2 * i] << 8)
					    + buffer[2 * i + 1]) * 1000 / PIOS_HMC5843_Sensitivity_6_5Ga;
		break;
	}
}

/**
* Read the identification bytes from the sensor
*/
void PIOS_HMC5843_ReadID(uint8_t out[4])
{
	while (!PIOS_HMC5843_Read(PIOS_HMC5843_DATAOUT_IDA_REG, out, 3)) ;
	out[3] = '\0';
}

bool PIOS_HMC5843_NewDataAvailable(void)
{
	return (pios_hmc5843_data_ready);
}

/**
* Reads one or more bytes into a buffer
* \param[in] address HMC5843 register address (depends on size)
* \param[out] buffer destination buffer
* \param[in] len number of bytes which should be read
* \return 0 if operation was successful
* \return -1 if error during I2C transfer
* \return -2 if unable to claim i2c device
*/
static bool PIOS_HMC5843_Read(uint8_t address, uint8_t * buffer, uint8_t len)
{
	uint8_t addr_buffer[] = {
		address,
	};

	const struct pios_i2c_txn txn_list[] = {
		{
		 .info = __func__,
		 .addr = PIOS_HMC5843_I2C_ADDR,
		 .rw = PIOS_I2C_TXN_WRITE,
		 .len = sizeof(addr_buffer),
		 .buf = addr_buffer,
		 }
		,
		{
		 .info = __func__,
		 .addr = PIOS_HMC5843_I2C_ADDR,
		 .rw = PIOS_I2C_TXN_READ,
		 .len = len,
		 .buf = buffer,
		 }
	};

	return PIOS_I2C_Transfer(PIOS_I2C_MAIN_ADAPTER, txn_list, NELEMENTS(txn_list));
}

/**
* Writes one or more bytes to the HMC5843
* \param[in] address Register address
* \param[in] buffer source buffer
* \return 0 if operation was successful
* \retval -1 if error during I2C transfer
* \retval -2 if unable to claim i2c device
*/
static bool PIOS_HMC5843_Write(uint8_t address, uint8_t buffer)
{
	uint8_t data[] = {
		address,
		buffer,
	};

	const struct pios_i2c_txn txn_list[] = {
		{
		 .info = __func__,
		 .addr = PIOS_HMC5843_I2C_ADDR,
		 .rw = PIOS_I2C_TXN_WRITE,
		 .len = sizeof(data),
		 .buf = data,
		 }
		,
	};

	return PIOS_I2C_Transfer(PIOS_I2C_MAIN_ADAPTER, txn_list, NELEMENTS(txn_list));
}

#endif

/**
 * @}
 * @}
 */

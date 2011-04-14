/**
 ******************************************************************************
 * @addtogroup PIOS PIOS Core hardware abstraction layer
 * @{
 * @addtogroup PIOS_HMC5883 HMC5883 Functions
 * @brief Deals with the hardware interface to the magnetometers
 * @{
 *
 * @file       pios_hmc5883.c
 * @author     David "Buzz" Carlson (buzz@chebuzz.com)
 * 				The OpenPilot Team, http://www.openpilot.org Copyright (C) 2011.
 * @brief      HMC5883 Magnetic Sensor Functions from AHRS
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

#if defined(PIOS_INCLUDE_HMC5883)

/* Global Variables */

/* Local Types */
typedef struct {
	uint8_t M_ODR;		/* OUTPUT DATA RATE --> here below the relative define (See datasheet page 11 for more details) */
	uint8_t Meas_Conf;	/* Measurement Configuration,: Normal, positive bias, or negative bias --> here below the relative define */
	uint8_t Gain;		/* Gain Configuration, select the full scale --> here below the relative define (See datasheet page 11 for more details) */
	uint8_t Mode;
} PIOS_HMC5883_ConfigTypeDef;

/* Local Variables */
volatile bool pios_hmc5883_data_ready;

static void PIOS_HMC5883_Config(PIOS_HMC5883_ConfigTypeDef * HMC5883_Config_Struct);
static bool PIOS_HMC5883_Read(uint8_t address, uint8_t * buffer, uint8_t len);
static bool PIOS_HMC5883_Write(uint8_t address, uint8_t buffer);

/**
 * @brief Initialize the HMC5883 magnetometer sensor.
 * @return none
 */
void PIOS_HMC5883_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	EXTI_InitTypeDef EXTI_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	/* Enable DRDY GPIO clock */
	RCC_APB2PeriphClockCmd(PIOS_HMC5883_DRDY_CLK | RCC_APB2Periph_AFIO, ENABLE);

	/* Configure EOC pin as input floating */
	GPIO_InitStructure.GPIO_Pin = PIOS_HMC5883_DRDY_GPIO_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(PIOS_HMC5883_DRDY_GPIO_PORT, &GPIO_InitStructure);

	/* Configure the End Of Conversion (EOC) interrupt */
	GPIO_EXTILineConfig(PIOS_HMC5883_DRDY_PORT_SOURCE, PIOS_HMC5883_DRDY_PIN_SOURCE);
	EXTI_InitStructure.EXTI_Line = PIOS_HMC5883_DRDY_EXTI_LINE;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);

	/* Enable and set EOC EXTI Interrupt to the lowest priority */
	NVIC_InitStructure.NVIC_IRQChannel = PIOS_HMC5883_DRDY_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = PIOS_HMC5883_DRDY_PRIO;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	/* Configure the HMC5883 Sensor */
	PIOS_HMC5883_ConfigTypeDef HMC5883_InitStructure;
	HMC5883_InitStructure.M_ODR = PIOS_HMC5883_ODR_15;
	HMC5883_InitStructure.Meas_Conf = PIOS_HMC5883_MEASCONF_NORMAL;
	HMC5883_InitStructure.Gain = PIOS_HMC5883_GAIN_1_9;
	HMC5883_InitStructure.Mode = PIOS_HMC5883_MODE_CONTINUOUS;
	PIOS_HMC5883_Config(&HMC5883_InitStructure);

	pios_hmc5883_data_ready = false;
}

/**
 * @brief Initialize the HMC5883 magnetometer sensor
 * \return none
 * \param[in] PIOS_HMC5883_ConfigTypeDef struct to be used to configure sensor.
*
* CTRL_REGA: Control Register A
* Read Write
* Default value: 0x10
* 7:5  0   These bits must be cleared for correct operation.
* 4:2 DO2-DO0: Data Output Rate Bits
*             DO2 |  DO1 |  DO0 |   Minimum Data Output Rate (Hz)
*            ------------------------------------------------------
*              0  |  0   |  0   |            0.75
*              0  |  0   |  1   |            1.5
*              0  |  1   |  0   |            3
*              0  |  1   |  1   |            7.5
*              1  |  0   |  0   |           15 (default)
*              1  |  0   |  1   |           30
*              1  |  1   |  0   |           75
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
*              0  |  0   |  0   |  ±0.88Ga      |   1370     | 0xF8000x07FF (-2048:2047)
*              0  |  0   |  1   |  ±1.3Ga (def) |   1090     | 0xF8000x07FF (-2048:2047)
*              0  |  1   |  0   |  ±1.9Ga       |   820      | 0xF8000x07FF (-2048:2047)
*              0  |  1   |  1   |  ±2.5Ga       |   660      | 0xF8000x07FF (-2048:2047)
*              1  |  0   |  0   |  ±4.0Ga       |   440      | 0xF8000x07FF (-2048:2047)
*              1  |  0   |  1   |  ±4.7Ga       |   390      | 0xF8000x07FF (-2048:2047)
*              1  |  1   |  0   |  ±5.6Ga       |   330      | 0xF8000x07FF (-2048:2047)
*              1  |  1   |  1   |  ±8.1Ga       |   230      | 0xF8000x07FF (-2048:2047)
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
static void PIOS_HMC5883_Config(PIOS_HMC5883_ConfigTypeDef * HMC5883_Config_Struct)
{
	uint8_t CTRLA = 0x00;
	uint8_t CTRLB = 0x00;
	uint8_t MODE = 0x00;

	CTRLA |= (uint8_t) (HMC5883_Config_Struct->M_ODR | HMC5883_Config_Struct->Meas_Conf);
	CTRLB |= (uint8_t) (HMC5883_Config_Struct->Gain);
	MODE |= (uint8_t) (HMC5883_Config_Struct->Mode);

	// CRTL_REGA
	while (!PIOS_HMC5883_Write(PIOS_HMC5883_CONFIG_REG_A, CTRLA)) ;

	// CRTL_REGB
	while (!PIOS_HMC5883_Write(PIOS_HMC5883_CONFIG_REG_B, CTRLB)) ;

	// Mode register
	while (!PIOS_HMC5883_Write(PIOS_HMC5883_MODE_REG, MODE)) ;
}

/**
 * @brief Read current X, Z, Y values (in that order)
 * \param[out] int16_t array of size 3 to store X, Z, and Y magnetometer readings
 * \return none
*/
void PIOS_HMC5883_ReadMag(int16_t out[3])
{
	uint8_t buffer[6];
	uint8_t ctrlB;

	pios_hmc5883_data_ready = false;

	while (!PIOS_HMC5883_Read(PIOS_HMC5883_CONFIG_REG_B, &ctrlB, 1)) ;
	while (!PIOS_HMC5883_Read(PIOS_HMC5883_DATAOUT_XMSB_REG, buffer, 6)) ;

	switch (ctrlB & 0xE0) {
	case 0x00:
		for (int i = 0; i < 3; i++)
			out[i] = ((int16_t) ((uint16_t) buffer[2 * i] << 8)
				  + buffer[2 * i + 1]) * 1000 / PIOS_HMC5883_Sensitivity_0_88Ga;
		break;
	case 0x20:
		for (int i = 0; i < 3; i++)
			out[i] = ((int16_t) ((uint16_t) buffer[2 * i] << 8)
				  + buffer[2 * i + 1]) * 1000 / PIOS_HMC5883_Sensitivity_1_3Ga;
		break;
	case 0x40:
		for (int i = 0; i < 3; i++)
			out[i] = (int16_t) (((uint16_t) buffer[2 * i] << 8)
					    + buffer[2 * i + 1]) * 1000 / PIOS_HMC5883_Sensitivity_1_9Ga;
		break;
	case 0x60:
		for (int i = 0; i < 3; i++)
			out[i] = (int16_t) (((uint16_t) buffer[2 * i] << 8)
					    + buffer[2 * i + 1]) * 1000 / PIOS_HMC5883_Sensitivity_2_5Ga;
		break;
	case 0x80:
		for (int i = 0; i < 3; i++)
			out[i] = (int16_t) (((uint16_t) buffer[2 * i] << 8)
					    + buffer[2 * i + 1]) * 1000 / PIOS_HMC5883_Sensitivity_4_0Ga;
		break;
	case 0xA0:
		for (int i = 0; i < 3; i++)
			out[i] = (int16_t) (((uint16_t) buffer[2 * i] << 8)
					    + buffer[2 * i + 1]) * 1000 / PIOS_HMC5883_Sensitivity_4_7Ga;
		break;
	case 0xC0:
		for (int i = 0; i < 3; i++)
			out[i] = (int16_t) (((uint16_t) buffer[2 * i] << 8)
					    + buffer[2 * i + 1]) * 1000 / PIOS_HMC5883_Sensitivity_5_6Ga;
		break;
	case 0xE0:
		for (int i = 0; i < 3; i++)
			out[i] = (int16_t) (((uint16_t) buffer[2 * i] << 8)
					    + buffer[2 * i + 1]) * 1000 / PIOS_HMC5883_Sensitivity_8_1Ga;
		break;
	}
}


/**
 * @brief Read the identification bytes from the HMC5883 sensor
 * \param[out] uint8_t array of size 4 to store HMC5883 ID.
 * \return none
*/
void PIOS_HMC5883_ReadID(uint8_t out[4])
{
	while (!PIOS_HMC5883_Read(PIOS_HMC5883_DATAOUT_IDA_REG, out, 3)) ;
	out[3] = '\0';
}

/**
 * @brief Tells whether new magnetometer readings are available
 * \return true if new data is available
 * \return false if new data is not available
*/
bool PIOS_HMC5883_NewDataAvailable(void)
{
	return (pios_hmc5883_data_ready);
}

/**
* @brief Reads one or more bytes into a buffer
* \param[in] address HMC5883 register address (depends on size)
* \param[out] buffer destination buffer
* \param[in] len number of bytes which should be read
* \return 0 if operation was successful
* \return -1 if error during I2C transfer
* \return -4 if invalid length
*/
static bool PIOS_HMC5883_Read(uint8_t address, uint8_t * buffer, uint8_t len)
{
	uint8_t addr_buffer[] = {
		address,
	};

	const struct pios_i2c_txn txn_list[] = {
		{
		 .info = __func__,
		 .addr = PIOS_HMC5883_I2C_ADDR,
		 .rw = PIOS_I2C_TXN_WRITE,
		 .len = sizeof(addr_buffer),
		 .buf = addr_buffer,
		 }
		,
		{
		 .info = __func__,
		 .addr = PIOS_HMC5883_I2C_ADDR,
		 .rw = PIOS_I2C_TXN_READ,
		 .len = len,
		 .buf = buffer,
		 }
	};

	return PIOS_I2C_Transfer(PIOS_I2C_MAIN_ADAPTER, txn_list, NELEMENTS(txn_list));
}

/**
* @brief Writes one or more bytes to the HMC5883
* \param[in] address Register address
* \param[in] buffer source buffer
* \return 0 if operation was successful
* \return -1 if error during I2C transfer
*/
static bool PIOS_HMC5883_Write(uint8_t address, uint8_t buffer)
{
	uint8_t data[] = {
		address,
		buffer,
	};

	const struct pios_i2c_txn txn_list[] = {
		{
		 .info = __func__,
		 .addr = PIOS_HMC5883_I2C_ADDR,
		 .rw = PIOS_I2C_TXN_WRITE,
		 .len = sizeof(data),
		 .buf = data,
		 }
		,
	};

	return PIOS_I2C_Transfer(PIOS_I2C_MAIN_ADAPTER, txn_list, NELEMENTS(txn_list));
}

/**
 * @brief Run self-test operation.  Do not call this during operational use!!
 * \return 0 if test failed
 * \return non-zero value if test succeeded
 */
int32_t PIOS_HMC5883_Test(void)
{
	/* Verify that ID matches (HMC5883 ID is null-terminated ASCII string "H43") */
	char id[4];
	PIOS_HMC5883_ReadID((uint8_t *)id);
	if(!strncmp("H43\0",id,4))
		return 0;
	else
		return 1;

	int32_t passed = 1;
	uint8_t registers[3] = {0,0,0};

	/* Backup existing configuration */
	while (!PIOS_HMC5883_Read(PIOS_HMC5883_CONFIG_REG_A,registers,3) );

	/*
	 * Put HMC5883 into self test mode
	 * This is done by placing measurement config into positive (0x01) or negative (0x10) bias
	 * and then placing the mode register into single-measurement mode.  This causes the HMC5883
	 * to create an artificial magnetic field of ~1.1 Gauss.
	 *
	 * If gain were PIOS_HMC5883_GAIN_2_5, for example, X and Y will read around +766 LSB
	 * (1.16 Ga * 660 LSB/Ga) and Z would read around +713 LSB (1.08 Ga * 660 LSB/Ga)
	 *
	 * Changing measurement config back to PIOS_HMC5883_MEASCONF_NORMAL will leave self-test mode.
	 */
	 while (!PIOS_HMC5883_Write(PIOS_HMC5883_CONFIG_REG_A, PIOS_HMC5883_MEASCONF_BIAS_POS | PIOS_HMC5883_ODR_15)) ;
	 while (!PIOS_HMC5883_Write(PIOS_HMC5883_CONFIG_REG_B, PIOS_HMC5883_GAIN_2_5)) ;
	 while (!PIOS_HMC5883_Write(PIOS_HMC5883_MODE_REG,     PIOS_HMC5883_MODE_SINGLE)) ;

	 uint8_t values[6];
	 while (!PIOS_HMC5883_Read(PIOS_HMC5883_DATAOUT_XMSB_REG, values, 6)) ;
	 int16_t x = (int16_t) (((uint16_t) values[0] << 8) + values[1]);
	 int16_t y = (int16_t) (((uint16_t) values[2] << 8) + values[3]);
	 int16_t z = (int16_t) (((uint16_t) values[4] << 8) + values[5]);

	 if(abs(abs(x) - 766) > 20)
		 passed &= 0;
	 if(abs(abs(y) - 766) > 20)
		 passed &= 0;
	 if(abs(abs(z) - 713) > 20)
		 passed &= 0;

	 /* Restore backup configuration */
	 while (!PIOS_HMC5883_Write(PIOS_HMC5883_CONFIG_REG_A,registers[0]) );
	 while (!PIOS_HMC5883_Write(PIOS_HMC5883_CONFIG_REG_B,registers[1]) );
	 while (!PIOS_HMC5883_Write(PIOS_HMC5883_MODE_REG,registers[2]) );

	 return passed;
}

/**
* @brief IRQ Handler
*/
void PIOS_HMC5883_IRQHandler(void)
{
	pios_hmc5883_data_ready = true;
}

#endif /* PIOS_INCLUDE_HMC5883 */

/**
 * @}
 * @}
 */

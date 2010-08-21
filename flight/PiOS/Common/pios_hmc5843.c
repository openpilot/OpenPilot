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

/* Glocal Variables */


/* Local Variables */

/**
  * @brieft Initialise the HMC5843 sensor
  */
void PIOS_HMC5843_Init(void)
{
	// Nothing to do here
	// If we were using the DRDY (data ready) interrupt input, we would set it up here
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
void PIOS_HMC5843_Config(PIOS_HMC5843_ConfigTypeDef *HMC5843_Config_Struct)
{
	uint8_t CRTLA = 0x00;
	uint8_t CRTLB = 0x00;
	uint8_t MODE = 0x00;

	CRTLA |= (uint8_t) (HMC5843_Config_Struct->M_ODR | HMC5843_Config_Struct->Meas_Conf);
	CRTLB |= (uint8_t) (HMC5843_Config_Struct->Gain);
	MODE  |= (uint8_t) (HMC5843_Config_Struct->Mode);

	// CRTL_REGA
	PIOS_HMC5843_Write(PIOS_HMC5843_CONFIG_REG_A, CRTLA);

	// CRTL_REGB
	PIOS_HMC5843_Write(PIOS_HMC5843_CONFIG_REG_B, CRTLB);

	// Mode register
	PIOS_HMC5843_Write(PIOS_HMC5843_MODE_REG, MODE);
}

/**
* Read the magnetic readings from the sensor
*/
void PIOS_HMC5843_ReadMag(int16_t out[3])
{
	uint8_t buffer[6];
	uint8_t crtlB;

	PIOS_HMC5843_Read(PIOS_HMC5843_CONFIG_REG_B, &crtlB, 1);
	PIOS_HMC5843_Read(PIOS_HMC5843_DATAOUT_XMSB_REG, buffer, 6);

	switch(crtlB & 0xE0) {
		case 0x00:
			for(int i = 0; i < 3; i++)
				out[i] = ((int16_t) ((uint16_t) buffer[2 * i] << 8)
						+ buffer[2 * i + 1]) * 1000
						/ PIOS_HMC5843_Sensitivity_0_7Ga;
			break;
		case 0x20:
			for(int i = 0; i < 3; i++)
				out[i] = ((int16_t) ((uint16_t) buffer[2 * i] << 8)
						+ buffer[2 * i + 1]) * 1000
						/ PIOS_HMC5843_Sensitivity_1Ga;
			break;
		case 0x40:
			for(int i = 0; i < 3; i++)
				out[i] = (int16_t) (((uint16_t) buffer[2 * i] << 8)
						+ buffer[2 * i + 1]) * 1000
						/ PIOS_HMC5843_Sensitivity_1_5Ga;
			break;
		case 0x60:
			for(int i = 0; i < 3; i++)
				out[i] = (int16_t) (((uint16_t) buffer[2 * i] << 8)
						+ buffer[2 * i + 1]) * 1000
						/ PIOS_HMC5843_Sensitivity_2Ga;
			break;
		case 0x80:
			for(int i = 0; i < 3; i++)
				out[i] = (int16_t) (((uint16_t) buffer[2 * i] << 8)
						+ buffer[2 * i + 1]) * 1000
						/ PIOS_HMC5843_Sensitivity_3_2Ga;
			break;
		case 0xA0:
			for(int i = 0; i < 3; i++)
				out[i] = (int16_t) (((uint16_t) buffer[2 * i] << 8)
						+ buffer[2 * i + 1]) * 1000
						/ PIOS_HMC5843_Sensitivity_3_8Ga;
			break;
		case 0xC0:
			for(int i = 0; i < 3; i++)
				out[i] = (int16_t) (((uint16_t) buffer[2 * i] << 8)
						+ buffer[2 * i + 1]) * 1000
						/ PIOS_HMC5843_Sensitivity_4_5Ga;
			break;
		case 0xE0:
			for(int i = 0; i < 3; i++)
				out[i] = (int16_t) (((uint16_t) buffer[2 * i] << 8)
						+ buffer[2 * i + 1]) * 1000
						/ PIOS_HMC5843_Sensitivity_6_5Ga;
			break;
	}
}

/**
* Read the identification bytes from the sensor
*/
void PIOS_HMC5843_ReadID(uint8_t out[4])
{
	PIOS_HMC5843_Read(PIOS_HMC5843_DATAOUT_IDA_REG, out, 3);
	out[3] = '\0';
}

/**
* Reads one or more bytes into a buffer
* \param[in] address HMC5843 register address (depends on size)
* \param[out] buffer destination buffer
* \param[in] len number of bytes which should be read
* \return 0 if operation was successful
* \return -1 if error during I2C transfer
* \return -4 if invalid length
*/
bool PIOS_HMC5843_Read(uint8_t address, uint8_t *buffer, uint8_t len)
{
  uint8_t addr_buffer[] = {
    address,
  };

  const struct pios_i2c_txn txn_list[] = {
    {
      .addr = PIOS_HMC5843_I2C_ADDR,
      .rw   = PIOS_I2C_TXN_WRITE,
      .len  = sizeof(addr_buffer),
      .buf  = addr_buffer,
    },
    {
      .addr = PIOS_HMC5843_I2C_ADDR,
      .rw   = PIOS_I2C_TXN_READ,
      .len  = len,
      .buf  = buffer,
    }
  };

  return PIOS_I2C_Transfer(PIOS_I2C_MAIN_ADAPTER, txn_list, NELEMENTS(txn_list));
}


/**
* Writes one or more bytes to the HMC5843
* \param[in] address Register address
* \param[in] buffer source buffer
* \return 0 if operation was successful
* \return -1 if error during I2C transfer
*/
bool PIOS_HMC5843_Write(uint8_t address, uint8_t buffer)
{
  uint8_t data[] = {
    address,
    buffer,
  };

  const struct pios_i2c_txn txn_list[] = {
    {
      .addr = PIOS_HMC5843_I2C_ADDR,
      .rw   = PIOS_I2C_TXN_WRITE,
      .len  = sizeof(data),
      .buf  = data,
    },
  };

  return PIOS_I2C_Transfer(PIOS_I2C_MAIN_ADAPTER, txn_list, NELEMENTS(txn_list));
}

#endif

/**
 * @}
 * @}
 */

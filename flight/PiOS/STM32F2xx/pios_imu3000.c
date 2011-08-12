/**
 ******************************************************************************
 * @addtogroup PIOS PIOS Core hardware abstraction layer
 * @{
 * @addtogroup PIOS_IMU3000 IMU3000 Functions
 * @brief Deals with the hardware interface to the 3-axis gyro
 * @{
 *
 * @file       pios_IMU3000.c
 * @author     David "Buzz" Carlson (buzz@chebuzz.com)
 * 				The OpenPilot Team, http://www.openpilot.org Copyright (C) 2011.
 * @brief      IMU3000 3-axis gyor functions from INS
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

#if defined(PIOS_INCLUDE_IMU3000)

/* Global Variables */

/* Local Types */
typedef struct {
	uint8_t Fifo_store;		/* FIFO storage of different readings (See datasheet page 31 for more details) */
	uint8_t Smpl_rate_div;	/* Sample rate divider to use (See datasheet page 32 for more details) */
	uint8_t DigLPF_Scale;	/* Digital low-pass filter and full-range scale (See datasheet page 33 for more details)  */
	uint8_t Interrupt_cfg;	/* Interrupt configuration (See datasheet page 35 for more details) */
	uint8_t User_ctl;		/* User control settings (See datasheet page 41 for more details)  */
	uint8_t Pwr_mgmt_clk;	/* Power management and clock selection (See datasheet page 32 for more details) */
} PIOS_IMU3000_ConfigTypeDef;

/* Local Variables */

static void PIOS_IMU3000_Config(PIOS_IMU3000_ConfigTypeDef * IMU3000_Config_Struct);
static int32_t PIOS_IMU3000_Read(uint8_t address, uint8_t * buffer, uint8_t len);
static int32_t PIOS_IMU3000_Write(uint8_t address, uint8_t buffer);

/**
 * @brief Initialize the IMU3000 3-axis gyro sensor.
 * @return none
 */
void PIOS_IMU3000_Init(const struct pios_imu3000_cfg * cfg)
{
	/* Configure EOC pin as input floating */
	GPIO_Init(cfg->drdy.gpio, &cfg->drdy.init);
	
	/* Configure the End Of Conversion (EOC) interrupt */
	//GPIO_EXTILineConfig(cfg->eoc_exit.port_source, cfg->eoc_exit.pin_source);
	EXTI_Init(&cfg->eoc_exti.init);
	
	/* Enable and set EOC EXTI Interrupt to the lowest priority */
	NVIC_Init(&cfg->eoc_irq.init);

	/* Configure the IMU3000 Sensor */
	PIOS_IMU3000_ConfigTypeDef IMU3000_InitStructure;
	IMU3000_InitStructure.Fifo_store = PIOS_IMU3000_FIFO_TEMP_OUT | PIOS_IMU3000_FIFO_GYRO_X_OUT |
			PIOS_IMU3000_FIFO_GYRO_Y_OUT | PIOS_IMU3000_FIFO_GYRO_Z_OUT | PIOS_IMU3000_FIFO_FOOTER;
	IMU3000_InitStructure.Smpl_rate_div = 8;
	IMU3000_InitStructure.DigLPF_Scale = PIOS_IMU3000_LOWPASS_256_HZ | PIOS_IMU3000_SCALE_500_DEG;
	IMU3000_InitStructure.Interrupt_cfg = PIOS_IMU3000_INT_CLR_ANYRD | PIOS_IMU3000_INT_DATA_RDY;
	IMU3000_InitStructure.User_ctl = PIOS_IMU3000_USERCTL_FIFO_EN;
	IMU3000_InitStructure.Pwr_mgmt_clk = PIOS_IMU3000_PWRMGMT_PLL_X_CLK;
	PIOS_IMU3000_Config(&IMU3000_InitStructure);
}

/**
 * @brief Initialize the IMU3000 3-axis gyro sensor
 * \return none
 * \param[in] PIOS_IMU3000_ConfigTypeDef struct to be used to configure sensor.
*
*/
static void PIOS_IMU3000_Config(PIOS_IMU3000_ConfigTypeDef * IMU3000_Config_Struct)
{
	// TODO: Add checks against current config so we only update what has changed

	// FIFO storage
	while (PIOS_IMU3000_Write(PIOS_IMU3000_FIFO_EN_REG, IMU3000_Config_Struct->Fifo_store) != 0);

	// Sample rate divider
	while (PIOS_IMU3000_Write(PIOS_IMU3000_SMPLRT_DIV_REG, IMU3000_Config_Struct->Smpl_rate_div) != 0) ;

	// Digital low-pass filter and scale
	while (PIOS_IMU3000_Write(PIOS_IMU3000_DLPF_CFG_REG, IMU3000_Config_Struct->DigLPF_Scale) != 0) ;

	// Interrupt configuration
	while (PIOS_IMU3000_Write(PIOS_IMU3000_INT_CFG_REG, IMU3000_Config_Struct->Interrupt_cfg) != 0) ;

	// Interrupt configuration
	while (PIOS_IMU3000_Write(PIOS_IMU3000_USER_CTRL_REG, IMU3000_Config_Struct->User_ctl) != 0) ;

	// Interrupt configuration
	while (PIOS_IMU3000_Write(PIOS_IMU3000_PWR_MGMT_REG, IMU3000_Config_Struct->Pwr_mgmt_clk) != 0) ;
}

/**
 * @brief Read current X, Z, Y values (in that order)
 * \param[out] int16_t array of size 3 to store X, Z, and Y magnetometer readings
 * \returns The number of samples remaining in the fifo
*/
int32_t PIOS_IMU3000_ReadGyros(int16_t * data)
{
	uint8_t buf[6];
	if(PIOS_IMU3000_Read(PIOS_IMU3000_GYRO_X_OUT_MSB, (uint8_t *) buf, sizeof(buf)) < 0)
		return -1;
	data[0] = buf[0] << 8 | buf[1];
	data[1] = buf[2] << 8 | buf[3];
	data[2] = buf[4] << 8 | buf[5];
	return 0;
}


/**
 * @brief Read the identification bytes from the IMU3000 sensor
 * \return ID read from IMU3000 or -1 if failure
*/
int32_t PIOS_IMU3000_ReadID()
{
	uint8_t id;
	if(PIOS_IMU3000_Read(0x00, &id, 1) != 0)
		return -1;
	return id;
}

/**
 * @brief Reads the data from the IMU3000 FIFO
 * \param[out] buffer destination buffer
 * \param[in] len maximum number of bytes which should be read
 * \return number of bytes transferred if operation was successful
 * \return -1 if error during I2C transfer
 */
int32_t PIOS_IMU3000_ReadFifo(uint8_t * buffer, uint16_t len)
{
	uint16_t fifo_level;

	uint8_t addr_buffer[] = {
		0x3A,
	};
	

	const struct pios_i2c_txn txn_list[] = {
		{
			.info = __func__,
			.addr = PIOS_IMU3000_I2C_ADDR,
			.rw = PIOS_I2C_TXN_WRITE,
			.len = sizeof(addr_buffer),
			.buf = addr_buffer,
		}
		,
		{
			.info = __func__,
			.addr = PIOS_IMU3000_I2C_ADDR,
			.rw = PIOS_I2C_TXN_READ,
			.len = 2,
			.buf = (uint8_t *) &fifo_level,
		}
	};
	
	// Get the number of bytes in the fifo
	PIOS_I2C_Transfer(PIOS_I2C_GYRO_ADAPTER, txn_list, NELEMENTS(txn_list));
	addr_buffer[0] = 0x3C;
	
	
	if(len > fifo_level)
		len = fifo_level;
	len &= 0x01f8; // only read chunks of 8 bytes (includes footer)
	
	const struct pios_i2c_txn txn_list2[] = {
		{
			.info = __func__,
			.addr = PIOS_IMU3000_I2C_ADDR,
			.rw = PIOS_I2C_TXN_WRITE,
			.len = sizeof(addr_buffer),
			.buf = addr_buffer,
		}
		,
		{
			.info = __func__,
			.addr = PIOS_IMU3000_I2C_ADDR,
			.rw = PIOS_I2C_TXN_READ,
			.len = len,
			.buf = buffer,
		}
	};	
	return PIOS_I2C_Transfer(PIOS_I2C_GYRO_ADAPTER, txn_list2, NELEMENTS(txn_list)) ? len : -1;
}

/**
* @brief Reads one or more bytes from IMU3000 into a buffer
* \param[in] address IMU3000 register address (depends on size)
* \param[out] buffer destination buffer
* \param[in] len number of bytes which should be read
* \return 0 if operation was successful
* \return -1 if error during I2C transfer
* \return -4 if invalid length
*/
static int32_t PIOS_IMU3000_Read(uint8_t address, uint8_t * buffer, uint8_t len)
{
	uint8_t addr_buffer[] = {
		address,
	};

	const struct pios_i2c_txn txn_list[] = {
		{
		 .info = __func__,
		 .addr = PIOS_IMU3000_I2C_ADDR,
		 .rw = PIOS_I2C_TXN_WRITE,
		 .len = sizeof(addr_buffer),
		 .buf = addr_buffer,
		 }
		,
		{
		 .info = __func__,
		 .addr = PIOS_IMU3000_I2C_ADDR,
		 .rw = PIOS_I2C_TXN_READ,
		 .len = len,
		 .buf = buffer,
		 }
	};

	return PIOS_I2C_Transfer(PIOS_I2C_GYRO_ADAPTER, txn_list, NELEMENTS(txn_list)) ? 0 : -1;
}

/**
* @brief Writes one or more bytes to the IMU3000
* \param[in] address Register address
* \param[in] buffer source buffer
* \return 0 if operation was successful
* \return -1 if error during I2C transfer
*/
static int32_t PIOS_IMU3000_Write(uint8_t address, uint8_t buffer)
{
	uint8_t data[] = {
		address,
		buffer,
	};

	const struct pios_i2c_txn txn_list[] = {
		{
		 .info = __func__,
		 .addr = PIOS_IMU3000_I2C_ADDR,
		 .rw = PIOS_I2C_TXN_WRITE,
		 .len = sizeof(data),
		 .buf = data,
		 }
		,
	};

	return PIOS_I2C_Transfer(PIOS_I2C_GYRO_ADAPTER, txn_list, NELEMENTS(txn_list)) ? 0 : -1;
}

/**
 * @brief Run self-test operation.
 * \return 0 if test failed
 * \return non-zero value if test succeeded
 */
uint8_t PIOS_IMU3000_Test(void)
{
	/* Verify that ID matches (IMU3000 ID is 0x69) */
	int32_t id = 0;
	id = PIOS_IMU3000_ReadID();
	if(id < 0)
		return -1;
	
	if(id != PIOS_IMU3000_I2C_ADDR)
		return -2;
	
	return 0;
}

/**
* @brief IRQ Handler
*/
void PIOS_IMU3000_IRQHandler(void)
{
}

#endif /* PIOS_INCLUDE_IMU3000 */

/**
 * @}
 * @}
 */

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
static bool PIOS_IMU3000_Read(uint8_t address, uint8_t * buffer, uint8_t len);
static bool PIOS_IMU3000_Write(uint8_t address, uint8_t buffer);

/**
 * @brief Initialize the IMU3000 3-axis gyro sensor.
 * @return none
 */
void PIOS_IMU3000_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	EXTI_InitTypeDef EXTI_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	/* Enable INT GPIO clock */
	RCC_APB2PeriphClockCmd(PIOS_IMU3000_INT_CLK | RCC_APB2Periph_AFIO, ENABLE);

	/* Configure IMU3000 interrupt pin as input floating */
	GPIO_InitStructure.GPIO_Pin = PIOS_IMU3000_INT_GPIO_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(PIOS_IMU3000_INT_GPIO_PORT, &GPIO_InitStructure);

	/* Configure the End Of Conversion (EOC) interrupt */
	GPIO_EXTILineConfig(PIOS_IMU3000_INT_PORT_SOURCE, PIOS_IMU3000_INT_PIN_SOURCE);
	EXTI_InitStructure.EXTI_Line = PIOS_IMU3000_INT_EXTI_LINE;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);

	/* Enable and set EOC EXTI Interrupt to the lowest priority */
	NVIC_InitStructure.NVIC_IRQChannel = PIOS_IMU3000_INT_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = PIOS_IMU3000_INT_PRIO;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

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
	while (!PIOS_IMU3000_Write(PIOS_IMU3000_FIFO_EN_REG, IMU3000_Config_Struct->Fifo_store)) ;

	// Sample rate divider
	while (!PIOS_IMU3000_Write(PIOS_IMU3000_SMPLRT_DIV_REG, IMU3000_Config_Struct->Smpl_rate_div)) ;

	// Digital low-pass filter and scale
	while (!PIOS_IMU3000_Write(PIOS_IMU3000_DLPF_CFG_REG, IMU3000_Config_Struct->DigLPF_Scale)) ;

	// Interrupt configuration
	while (!PIOS_IMU3000_Write(PIOS_IMU3000_INT_CFG_REG, IMU3000_Config_Struct->Interrupt_cfg)) ;

	// Interrupt configuration
	while (!PIOS_IMU3000_Write(PIOS_IMU3000_USER_CTRL_REG, IMU3000_Config_Struct->User_ctl)) ;

	// Interrupt configuration
	while (!PIOS_IMU3000_Write(PIOS_IMU3000_PWR_MGMT_REG, IMU3000_Config_Struct->Pwr_mgmt_clk)) ;
}

/**
 * @brief Read current X, Z, Y values (in that order)
 * \param[out] int16_t array of size 3 to store X, Z, and Y magnetometer readings
 * \return none
*/
void PIOS_IMU3000_ReadGyros(int16_t out[3])
{
}


/**
 * @brief Read the identification bytes from the IMU3000 sensor
 * \return ID read from IMU3000
*/
uint8_t PIOS_IMU3000_ReadID()
{
	uint8_t id;
	while (!PIOS_IMU3000_Read(0x00, &id, 1)) ;
	return id;
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
static bool PIOS_IMU3000_Read(uint8_t address, uint8_t * buffer, uint8_t len)
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

	return PIOS_I2C_Transfer(PIOS_I2C_MAIN_ADAPTER, txn_list, NELEMENTS(txn_list));
}

/**
* @brief Writes one or more bytes to the IMU3000
* \param[in] address Register address
* \param[in] buffer source buffer
* \return 0 if operation was successful
* \return -1 if error during I2C transfer
*/
static bool PIOS_IMU3000_Write(uint8_t address, uint8_t buffer)
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

	return PIOS_I2C_Transfer(PIOS_I2C_MAIN_ADAPTER, txn_list, NELEMENTS(txn_list));
}

/**
 * @brief Run self-test operation.
 * \return 0 if test failed
 * \return non-zero value if test succeeded
 */
uint8_t PIOS_IMU3000_Test(void)
{
	/* Verify that ID matches (IMU3000 ID is 0x69) */
	return (0x69 ^ PIOS_IMU3000_ReadID() );
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

/**
 ******************************************************************************
 * @addtogroup PIOS PIOS Core hardware abstraction layer
 * @{
 * @addtogroup PIOS_HMC5883 HMC5883 Functions
 * @brief Deals with the hardware interface to the magnetometers
 * @{
 * @file       pios_ad7998.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2012.
 * @brief      Analog device i2c ADC chip
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

#if defined(PIOS_INCLUDE_AD7998)
uint8_t PIOS_AD7998_MODE2_ADDR_CH[] = {   0x80, 0x90, 0xa0, 0xb0, 0xc0, 0xd0, 0xe0, 0xf0 };

static struct pios_ad7998_cfg dev_cfg;

static int32_t i2c_id;

/* Global Variables */

/* Local Types */

/* Local Variables */

/* Private constants */
#define AD7998_TASK_PRIORITY	(tskIDLE_PRIORITY + 2)	//
#define AD7998_TASK_STACK		(384 / 4)
#define UPDATE_RATE				20.0f//100hz

static void PIOS_AD7998_Config(void);
static int32_t PIOS_AD7998_Read(uint8_t address, uint8_t * buffer, uint8_t len);
static int32_t PIOS_AD7998_Write(uint8_t address, uint8_t * buffer, uint8_t len);
void PIOS_AD7998_Task(void *parameters);

int32_t PIOS_AD7998_Start(int32_t i2c)
{
	dev_cfg.i2c_id=i2c;
	if(dev_cfg.i2c_id==0)
	{

	}
	else
	{
		int result = xTaskCreate(PIOS_AD7998_Task, (const signed char *)"PIOS_AD7998_Task", AD7998_TASK_STACK, NULL, AD7998_TASK_PRIORITY,	&dev_cfg.TaskHandle);
	}
	return 0;
}

/**
 * @brief Initialize the AD7998 ADC sensor.
 * @return 0 for success, -1 for failure
 */
int32_t PIOS_AD7998_Init(int32_t i2c)
{

	i2c_id=i2c;
	/* Configure the ad7998 Sensor */
	PIOS_AD7998_Config();
	return 0;

}

/**
 * @brief Initialize the adc
 * \return none
 * \param[in] PIOS_AD7998_Config struct to be used to configure sensor.
*
*/
static void PIOS_AD7998_Config(void)
{
	uint8_t cmd[2];
	//cmd[0] = PIOS_AD7998_CONF_REG;
	cmd[0] = (uint8_t)((PIOS_AD7998_CONF_CH_ALL | PIOS_AD7998_CONF_FLTR) >> 8);
	cmd[1] = (uint8_t)((PIOS_AD7998_CONF_CH_ALL | PIOS_AD7998_CONF_FLTR) & 0x0ff);
	// Reset chip
	while(PIOS_AD7998_Write(PIOS_AD7998_CONF_REG,cmd, 2)!=0)
	{
		continue;
	}
	PIOS_DELAY_WaitmS(100);
	
	
//	ad7998_configured = true;
}

uint16_t PIOS_AD7998_ReadConv(uint8_t channel)
{
	uint16_t Result;
	uint8_t cmd = PIOS_AD7998_MODE2_ADDR_CH[channel] | PIOS_AD7998_CONV_REG;
	uint8_t out[2];
	PIOS_AD7998_Write(cmd,out,0);//start conversion
	cmd = PIOS_AD7998_CONV_REG;
	PIOS_DELAY_WaituS(3);
	PIOS_AD7998_Read(cmd,&out, 2);
	Result=(((uint16_t)out[0]<<8)|(uint16_t)out[1])&0x0FFF;
	//Result=(((uint16_t)out[0]));
	return Result;
}

/**
 * @brief Read a register from AD7998
 * @returns The register value or -1 if failure to get bus
 * @param reg[in] Register address to be read
 */
static int32_t PIOS_AD7998_GetReg(uint8_t reg)
{
	uint8_t data;
	
	PIOS_AD7998_Read(reg, (uint8_t *) &data, 1);
	
	return data;
}

/**
 * @brief Writes one byte to the AD7998
 * \param[in] reg Register address
 * \param[in] data Byte to write
 * \return 0 if operation was successful
 * \return -1 if unable to claim SPI bus
 * \return -2 if unable to claim i2c device
 */
static int32_t PIOS_AD7998_SetReg(uint8_t reg, uint8_t data)
{
	/*if(PIOS_AD7998_Write(reg, data)!=0)
	{
		return -3;
	}*/
	return 0;
}

/**
* @brief Reads one or more bytes from MPU6050 into a buffer
* \param[in] address MPU6050 register address (depends on size)
* \param[out] buffer destination buffer
* \param[in] len number of bytes which should be read
* \return 0 if operation was successful
* \return -1 if error during I2C transfer
* \return -2 if unable to claim i2c device
*/
static int32_t PIOS_AD7998_Read(uint8_t address, uint8_t * buffer, uint8_t len)
{
	uint8_t addr_buffer[] = {
		address,
	};

	const struct pios_i2c_txn txn_list[] = {
		{
		 .info = __func__,
		 .addr = PIOS_AD7998_0_I2C_ADDR,
		 .rw = PIOS_I2C_TXN_WRITE,
		 .len = sizeof(addr_buffer),
		 .buf = addr_buffer,
		 }
		,
		{
		 .info = __func__,
		 .addr = PIOS_AD7998_0_I2C_ADDR,
		 .rw = PIOS_I2C_TXN_READ,
		 .len = len,
		 .buf = buffer,
		 }
	};

	return PIOS_I2C_Transfer(i2c_id, txn_list, NELEMENTS(txn_list));
	//return 0;
	//return PIOS_I2C_Transfer(PIOS_I2C_GYRO_ADAPTER, txn_list, NELEMENTS(txn_list));
}

static int32_t PIOS_AD7998_Write(uint8_t address, uint8_t * buffer, uint8_t len)
{

	uint8_t addr_buffer[] = {
			address,
		};

		const struct pios_i2c_txn txn_list[] = {
			{
			 .info = __func__,
			 .addr = PIOS_AD7998_0_I2C_ADDR,
			 .rw = PIOS_I2C_TXN_WRITE,
			 .len = sizeof(addr_buffer),
			 .buf = addr_buffer,
			 }
			,
			{
			 .info = __func__,
			 .addr = PIOS_AD7998_0_I2C_ADDR,
			 .rw = PIOS_I2C_TXN_WRITE,
			 .len = len,
			 .buf = buffer,
			 }
		};

	return PIOS_I2C_Transfer(i2c_id, txn_list, NELEMENTS(txn_list));
	//return 0;
	//return PIOS_I2C_Transfer(PIOS_I2C_GYRO_ADAPTER, txn_list, NELEMENTS(txn_list));
}

uint16_t PIOS_AD7998_GetValue(uint8_t channel)
{
	return dev_cfg.Value.V[channel];
}

void PIOS_AD7998_Task(void *parameters)
{
	PIOS_AD7998_Init(dev_cfg.i2c_id);
	vTaskDelay(100.0f/portTICK_RATE_MS);
	while(1)
	{
		for(uint8_t i=0;i<7;i++)
		{
			dev_cfg.Value.V[i]=PIOS_AD7998_ReadConv(i);
			//vTaskDelay(2/portTICK_RATE_MS);
		}
		vTaskDelay(UPDATE_RATE/portTICK_RATE_MS);
	}
}

/**
 * @}
 * @}
 */
#endif

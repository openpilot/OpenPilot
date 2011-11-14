/**
 ******************************************************************************
 * @addtogroup PIOS PIOS Core hardware abstraction layer
 * @{
 * @addtogroup PIOS_MPU6050 MPU6050 Functions
 * @brief Deals with the hardware interface to the 3-axis gyro
 * @{
 *
 * @file       pios_mpu050.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2011.
 * @brief      MPU6050 3-axis gyor functions from INS
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

#if defined(PIOS_INCLUDE_MPU6050)

/* Global Variables */

/* Local Variables */
#define DEG_TO_RAD (M_PI / 180.0)
static void PIOS_MPU6050_Config(struct pios_mpu6050_cfg const * cfg);
static int32_t PIOS_MPU6050_Read(uint8_t address, uint8_t * buffer, uint8_t len);
static int32_t PIOS_MPU6050_Write(uint8_t address, uint8_t buffer);

#define PIOS_MPU6050_MAX_DOWNSAMPLE 10
static int16_t pios_mpu6050_buffer[PIOS_MPU6050_MAX_DOWNSAMPLE * sizeof(struct pios_mpu6050_data)];
static t_fifo_buffer pios_mpu6050_fifo;

volatile bool mpu6050_first_read = true;
volatile bool mpu6050_configured = false;
volatile bool mpu6050_cb_ready = true;

static struct pios_mpu6050_cfg const * cfg;

/**
 * @brief Initialize the MPU6050 3-axis gyro sensor.
 * @return none
 */
void PIOS_MPU6050_Init(const struct pios_mpu6050_cfg * new_cfg)
{
	cfg = new_cfg;
	
	fifoBuf_init(&pios_mpu6050_fifo, (uint8_t *) pios_mpu6050_buffer, sizeof(pios_mpu6050_buffer));

	/* Configure EOC pin as input floating */
	GPIO_Init(cfg->drdy.gpio, &cfg->drdy.init);
	
	/* Configure the End Of Conversion (EOC) interrupt */
	SYSCFG_EXTILineConfig(cfg->eoc_exti.port_source, cfg->eoc_exti.pin_source);
	EXTI_Init(&cfg->eoc_exti.init);
	
	/* Enable and set EOC EXTI Interrupt to the lowest priority */
	NVIC_Init(&cfg->eoc_irq.init);

	/* Configure the MPU6050 Sensor */
	PIOS_MPU6050_Config(cfg);
}

/**
 * @brief Initialize the MPU6050 3-axis gyro sensor
 * \return none
 * \param[in] PIOS_MPU6050_ConfigTypeDef struct to be used to configure sensor.
*
*/
static void PIOS_MPU6050_Config(struct pios_mpu6050_cfg const * cfg)
{
	mpu6050_first_read = true;
	mpu6050_cb_ready = true;
	
	// Reset chip and fifo
	while (PIOS_MPU6050_Write(PIOS_MPU6050_USER_CTRL_REG, 0x01 | 0x02 | 0x04) != 0);
	PIOS_DELAY_WaituS(20);

	// FIFO storage
	while (PIOS_MPU6050_Write(PIOS_MPU6050_FIFO_EN_REG, cfg->Fifo_store) != 0);

	// Sample rate divider
	while (PIOS_MPU6050_Write(PIOS_MPU6050_SMPLRT_DIV_REG, cfg->Smpl_rate_div) != 0) ;

	// Digital low-pass filter and scale
	while (PIOS_MPU6050_Write(PIOS_MPU6050_DLPF_CFG_REG, cfg->filter) != 0) ;

	// Digital low-pass filter and scale
	while (PIOS_MPU6050_Write(PIOS_MPU6050_GYRO_CFG_REG, cfg->gyro_range) != 0) ;

	// Interrupt configuration
	while (PIOS_MPU6050_Write(PIOS_MPU6050_USER_CTRL_REG, cfg->User_ctl) != 0) ;

	// Interrupt configuration
	while (PIOS_MPU6050_Write(PIOS_MPU6050_PWR_MGMT_REG, cfg->Pwr_mgmt_clk) != 0) ;
	
	// Interrupt configuration
	while (PIOS_MPU6050_Write(PIOS_MPU6050_INT_CFG_REG, cfg->interrupt_cfg) != 0) ;

	// Interrupt configuration
	while (PIOS_MPU6050_Write(PIOS_MPU6050_INT_EN_REG, cfg->interrupt_en) != 0) ;

	mpu6050_configured = true;
}

/**
 * @brief Read current X, Z, Y values (in that order)
 * \param[out] int16_t array of size 3 to store X, Z, and Y magnetometer readings
 * \returns The number of samples remaining in the fifo
*/
int32_t PIOS_MPU6050_ReadGyros(struct pios_mpu6050_data * data)
{
	uint8_t buf[6];
	if(PIOS_MPU6050_Read(PIOS_MPU6050_GYRO_X_OUT_MSB, (uint8_t *) buf, sizeof(buf)) < 0)
		return -1;
	data->gyro_x = buf[0] << 8 | buf[1];
	data->gyro_y = buf[2] << 8 | buf[3];
	data->gyro_z = buf[4] << 8 | buf[5];
	return 0;
}


/**
 * @brief Read the identification bytes from the MPU6050 sensor
 * \return ID read from MPU6050 or -1 if failure
*/
int32_t PIOS_MPU6050_ReadID()
{
	uint8_t mpu6050_id;
	if(PIOS_MPU6050_Read(PIOS_MPU6050_WHOAMI, (uint8_t *) &mpu6050_id, 1) != 0)
		return -1;
	return mpu6050_id;
}

/**
 * \brief Reads the data from the MPU6050 FIFO
 * \param[out] buffer destination buffer
 * \param[in] len maximum number of bytes which should be read
 * \note This returns the data as X, Y, Z the temperature
 * \return number of bytes transferred if operation was successful
 * \return -1 if error during I2C transfer
 */
int32_t PIOS_MPU6050_ReadFifo(struct pios_mpu6050_data * buffer)
{
	if(fifoBuf_getUsed(&pios_mpu6050_fifo) < sizeof(*buffer))
		return -1;
		
	fifoBuf_getData(&pios_mpu6050_fifo, (uint8_t *) buffer, sizeof(*buffer));
	
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
static int32_t PIOS_MPU6050_Read(uint8_t address, uint8_t * buffer, uint8_t len)
{
	uint8_t addr_buffer[] = {
		address,
	};

	const struct pios_i2c_txn txn_list[] = {
		{
		 .info = __func__,
		 .addr = PIOS_MPU6050_I2C_ADDR,
		 .rw = PIOS_I2C_TXN_WRITE,
		 .len = sizeof(addr_buffer),
		 .buf = addr_buffer,
		 }
		,
		{
		 .info = __func__,
		 .addr = PIOS_MPU6050_I2C_ADDR,
		 .rw = PIOS_I2C_TXN_READ,
		 .len = len,
		 .buf = buffer,
		 }
	};

	return PIOS_I2C_Transfer(PIOS_I2C_GYRO_ADAPTER, txn_list, NELEMENTS(txn_list));
}

// Must allocate on stack to be persistent
static uint8_t cb_addr_buffer[] = {
	0,
};
static struct pios_i2c_txn cb_txn_list[] = {
	{
		.addr = PIOS_MPU6050_I2C_ADDR,
		.rw = PIOS_I2C_TXN_WRITE,
		.len = sizeof(cb_addr_buffer),
		.buf = cb_addr_buffer,
	}
	,
	{
		.addr = PIOS_MPU6050_I2C_ADDR,
		.rw = PIOS_I2C_TXN_READ,
		.len = 0,
		.buf = 0,
	}
};



static int32_t PIOS_MPU6050_Read_Callback(uint8_t address, uint8_t * buffer, uint8_t len, void *callback)
{
	cb_addr_buffer[0] = address;
	cb_txn_list[0].info = __func__,
	cb_txn_list[1].info = __func__;
	cb_txn_list[1].len = len;
	cb_txn_list[1].buf = buffer;
	
	return PIOS_I2C_Transfer_Callback(PIOS_I2C_GYRO_ADAPTER, cb_txn_list, NELEMENTS(cb_txn_list), callback);
}

/**
 * @brief Writes one or more bytes to the MPU6050
 * \param[in] address Register address
 * \param[in] buffer source buffer
 * \return 0 if operation was successful
 * \return -1 if error during I2C transfer
 * \return -2 if unable to claim i2c device
 */
static int32_t PIOS_MPU6050_Write(uint8_t address, uint8_t buffer)
{
	uint8_t data[] = {
		address,
		buffer,
	};

	const struct pios_i2c_txn txn_list[] = {
		{
		 .info = __func__,
		 .addr = PIOS_MPU6050_I2C_ADDR,
		 .rw = PIOS_I2C_TXN_WRITE,
		 .len = sizeof(data),
		 .buf = data,
		 }
		,
	};

	return PIOS_I2C_Transfer(PIOS_I2C_GYRO_ADAPTER, txn_list, NELEMENTS(txn_list));
}

float PIOS_MPU6050_GetScale() 
{
	switch (cfg->gyro_range) {
		case PIOS_MPU6050_SCALE_250_DEG:
			return DEG_TO_RAD / 131.0;
		case PIOS_MPU6050_SCALE_500_DEG:
			return DEG_TO_RAD / 65.5;
		case PIOS_MPU6050_SCALE_1000_DEG:
			return DEG_TO_RAD / 32.8;
		case PIOS_MPU6050_SCALE_2000_DEG:
			return DEG_TO_RAD / 16.4;
	}
	return 0;
}
/**
 * @brief Run self-test operation.
 * \return 0 if test succeeded
 * \return non-zero value if test succeeded
 */
uint8_t PIOS_MPU6050_Test(void)
{
	/* Verify that ID matches (MPU6050 ID is 0x69) */
	int32_t mpu6050_id = PIOS_MPU6050_ReadID();
	if(mpu6050_id < 0)
		return -1;
	
	if(mpu6050_id != PIOS_MPU6050_I2C_ADDR & 0xFE)
		return -2;
	
	return 0;
}

static uint8_t mpu6050_read_buffer[sizeof(struct pios_mpu6050_data)]; // Right now using ,Y,Z,fifo_footer
static void MPU6050_callback()
{
	struct pios_mpu6050_data data;

	if(fifoBuf_getFree(&pios_mpu6050_fifo) < sizeof(data))
		goto out;
	
	data.temperature = mpu6050_read_buffer[0] << 8 | mpu6050_read_buffer[1];
	data.gyro_x = mpu6050_read_buffer[2] << 8 | mpu6050_read_buffer[3];
	data.gyro_y = mpu6050_read_buffer[4] << 8 | mpu6050_read_buffer[5];
	data.gyro_z = mpu6050_read_buffer[6] << 8 | mpu6050_read_buffer[7];
	
	fifoBuf_putData(&pios_mpu6050_fifo, (uint8_t *) &data, sizeof(data));
	
out:
	mpu6050_cb_ready = true;

}

/**
* @brief IRQ Handler
*/
uint32_t mpu6050_irq = 0;
uint16_t fifo_level;
uint8_t fifo_level_data[2];
uint32_t cb_not_ready = 0;
void PIOS_MPU6050_IRQHandler(void)
{
	
	mpu6050_irq++;

	if(!mpu6050_configured)
		return;
		
	//PIOS_Assert(MPU6050_cb_ready);
	if(!mpu6050_cb_ready) {
		PIOS_LED_Toggle(LED2);
		cb_not_ready++;
		return;
	}

	/*// If at least one read doesnt succeed then the irq not reset and we will stall
	if(PIOS_MPU6050_Read(PIOS_MPU6050_FIFO_CNT_MSB, (uint8_t *) &fifo_level_data, sizeof(fifo_level_data)) != 0)
		return;
				
	fifo_level = (fifo_level_data[0] << 8) + fifo_level_data[1];
	
	PIOS_DELAY_WaituS(10);
	*/
	
		
	// Leave footer in buffer
	PIOS_MPU6050_Read_Callback(PIOS_MPU6050_FIFO_REG, mpu6050_read_buffer, sizeof(mpu6050_read_buffer), MPU6050_callback);
}

/**
 * The physical IRQ handler
 * Soon this will be generic in pios_exti and the BMA180 will register
 * against it.  Right now this is crap!
 */
void EXTI1_IRQHandler(void)
{
	if (EXTI_GetITStatus(EXTI_Line1) != RESET)
	{
		PIOS_MPU6050_IRQHandler();
		EXTI_ClearITPendingBit(EXTI_Line1);
	}
}

#endif

/**
 * @}
 * @}
 */

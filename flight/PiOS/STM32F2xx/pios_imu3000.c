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

/* Local Variables */
#define DEG_TO_RAD (M_PI / 180.0)
static void PIOS_IMU3000_Config(struct pios_imu3000_cfg const * cfg);
static int32_t PIOS_IMU3000_Read(uint8_t address, uint8_t * buffer, uint8_t len);
static int32_t PIOS_IMU3000_Write(uint8_t address, uint8_t buffer);

#define PIOS_IMU3000_MAX_DOWNSAMPLE 10
static int16_t pios_imu3000_buffer[PIOS_IMU3000_MAX_DOWNSAMPLE * sizeof(struct pios_imu3000_data)];
static t_fifo_buffer pios_imu3000_fifo;

volatile bool imu3000_first_read = true;
volatile bool imu3000_configured = false;
volatile bool imu3000_cb_ready = true;

static struct pios_imu3000_cfg const * cfg;

/**
 * @brief Initialize the IMU3000 3-axis gyro sensor.
 * @return none
 */
void PIOS_IMU3000_Init(const struct pios_imu3000_cfg * new_cfg)
{
	cfg = new_cfg;
	
	fifoBuf_init(&pios_imu3000_fifo, (uint8_t *) pios_imu3000_buffer, sizeof(pios_imu3000_buffer));

	/* Configure EOC pin as input floating */
	GPIO_Init(cfg->drdy.gpio, &cfg->drdy.init);
	
	/* Configure the End Of Conversion (EOC) interrupt */
	SYSCFG_EXTILineConfig(cfg->eoc_exti.port_source, cfg->eoc_exti.pin_source);
	EXTI_Init(&cfg->eoc_exti.init);
	
	/* Enable and set EOC EXTI Interrupt to the lowest priority */
	NVIC_Init(&cfg->eoc_irq.init);

	/* Configure the IMU3000 Sensor */
	PIOS_IMU3000_Config(cfg);
}

/**
 * @brief Initialize the IMU3000 3-axis gyro sensor
 * \return none
 * \param[in] PIOS_IMU3000_ConfigTypeDef struct to be used to configure sensor.
*
*/
static void PIOS_IMU3000_Config(struct pios_imu3000_cfg const * cfg)
{
	imu3000_first_read = true;
	imu3000_cb_ready = true;
	
	// Reset chip and fifo
	while (PIOS_IMU3000_Write(PIOS_IMU3000_USER_CTRL_REG, 0x01 | 0x02) != 0);
	PIOS_DELAY_WaituS(20);
	while (PIOS_IMU3000_Write(PIOS_IMU3000_USER_CTRL_REG, 0x00) != 0);

	// FIFO storage
	while (PIOS_IMU3000_Write(PIOS_IMU3000_FIFO_EN_REG, cfg->Fifo_store) != 0);

	// Sample rate divider
	while (PIOS_IMU3000_Write(PIOS_IMU3000_SMPLRT_DIV_REG, cfg->Smpl_rate_div) != 0) ;

	// Digital low-pass filter and scale
	while (PIOS_IMU3000_Write(PIOS_IMU3000_DLPF_CFG_REG, cfg->filter | (cfg->range << 3)) != 0) ;

	// Interrupt configuration
	while (PIOS_IMU3000_Write(PIOS_IMU3000_USER_CTRL_REG, cfg->User_ctl) != 0) ;

	// Interrupt configuration
	while (PIOS_IMU3000_Write(PIOS_IMU3000_PWR_MGMT_REG, cfg->Pwr_mgmt_clk) != 0) ;
	
	// Interrupt configuration
	while (PIOS_IMU3000_Write(PIOS_IMU3000_INT_CFG_REG, cfg->Interrupt_cfg) != 0) ;
	
	imu3000_configured = true;
}

/**
 * @brief Read current X, Z, Y values (in that order)
 * \param[out] int16_t array of size 3 to store X, Z, and Y magnetometer readings
 * \returns The number of samples remaining in the fifo
*/
int32_t PIOS_IMU3000_ReadGyros(struct pios_imu3000_data * data)
{
	uint8_t buf[6];
	if(PIOS_IMU3000_Read(PIOS_IMU3000_GYRO_X_OUT_MSB, (uint8_t *) buf, sizeof(buf)) < 0)
		return -1;
	data->x = buf[0] << 8 | buf[1];
	data->y = buf[2] << 8 | buf[3];
	data->z = buf[4] << 8 | buf[5];
	return 0;
}


/**
 * @brief Read the identification bytes from the IMU3000 sensor
 * \return ID read from IMU3000 or -1 if failure
*/
int32_t PIOS_IMU3000_ReadID()
{
	uint8_t imu3000_id;
	if(PIOS_IMU3000_Read(0x00, (uint8_t *) &imu3000_id, 1) != 0)
		return -1;
	return imu3000_id;
}

/**
 * \brief Reads the data from the IMU3000 FIFO
 * \param[out] buffer destination buffer
 * \param[in] len maximum number of bytes which should be read
 * \note This returns the data as X, Y, Z the temperature
 * \return number of bytes transferred if operation was successful
 * \return -1 if error during I2C transfer
 */
int32_t PIOS_IMU3000_ReadFifo(struct pios_imu3000_data * buffer)
{
	if(fifoBuf_getUsed(&pios_imu3000_fifo) < sizeof(*buffer))
		return -1;
		
	fifoBuf_getData(&pios_imu3000_fifo, (uint8_t *) buffer, sizeof(*buffer));
	
	return 0;
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

// Must allocate on stack to be persistent
static uint8_t cb_addr_buffer[] = {
	0,
};
static struct pios_i2c_txn cb_txn_list[] = {
	{
		.addr = PIOS_IMU3000_I2C_ADDR,
		.rw = PIOS_I2C_TXN_WRITE,
		.len = sizeof(cb_addr_buffer),
		.buf = cb_addr_buffer,
	}
	,
	{
		.addr = PIOS_IMU3000_I2C_ADDR,
		.rw = PIOS_I2C_TXN_READ,
		.len = 0,
		.buf = 0,
	}
};



static int32_t PIOS_IMU3000_Read_Callback(uint8_t address, uint8_t * buffer, uint8_t len, void *callback)
{
	cb_addr_buffer[0] = address;
	cb_txn_list[0].info = __func__,
	cb_txn_list[1].info = __func__;
	cb_txn_list[1].len = len;
	cb_txn_list[1].buf = buffer;
	
	PIOS_I2C_Transfer_Callback(PIOS_I2C_GYRO_ADAPTER, cb_txn_list, NELEMENTS(cb_txn_list), callback);
	
	return  0;
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

float PIOS_IMU3000_GetScale() 
{
	switch (cfg->range) {
		case PIOS_IMU3000_SCALE_250_DEG:
			return DEG_TO_RAD / 131.0;
		case PIOS_IMU3000_SCALE_500_DEG:
			return DEG_TO_RAD / 65.5;
		case PIOS_IMU3000_SCALE_1000_DEG:
			return DEG_TO_RAD / 32.8;
		case PIOS_IMU3000_SCALE_2000_DEG:
			return DEG_TO_RAD / 16.4;
	}
	return 0;
}
/**
 * @brief Run self-test operation.
 * \return 0 if test succeeded
 * \return non-zero value if test succeeded
 */
uint8_t PIOS_IMU3000_Test(void)
{
	/* Verify that ID matches (IMU3000 ID is 0x69) */
	int32_t imu3000_id = PIOS_IMU3000_ReadID();
	if(imu3000_id < 0)
		return -1;
	
	if(imu3000_id != PIOS_IMU3000_I2C_ADDR)
		return -2;
	
	return 0;
}

static uint8_t imu3000_read_buffer[sizeof(struct pios_imu3000_data) + 2]; // Right now using ,Y,Z,fifo_footer
static void imu3000_callback()
{
	struct pios_imu3000_data data;

	if(fifoBuf_getFree(&pios_imu3000_fifo) < sizeof(data))
		goto out;

	if(imu3000_first_read) {
		data.x = imu3000_read_buffer[0] << 8 | imu3000_read_buffer[1];
		data.y = imu3000_read_buffer[2] << 8 | imu3000_read_buffer[3];
		data.z = imu3000_read_buffer[4] << 8 | imu3000_read_buffer[5];
		
		imu3000_first_read = false;
	} else {
		// First two bytes are left over fifo from last call
		data.x = imu3000_read_buffer[2] << 8 | imu3000_read_buffer[3];
		data.y = imu3000_read_buffer[4] << 8 | imu3000_read_buffer[5];
		data.z = imu3000_read_buffer[6] << 8 | imu3000_read_buffer[7];

	}
	
	fifoBuf_putData(&pios_imu3000_fifo, (uint8_t *) &data, sizeof(data));
	
out:
	imu3000_cb_ready = true;

}

/**
* @brief IRQ Handler
*/
uint32_t imu3000_irq = 0;
uint16_t fifo_level;
uint8_t fifo_level_data[2];
uint32_t imu3000_readtime;
void PIOS_IMU3000_IRQHandler(void)
{
	
	imu3000_irq++;

	if(!imu3000_configured)
		return;
		
	PIOS_Assert(imu3000_cb_ready);

	// If at least one read doesnt succeed then the irq not reset and we will stall
	while(PIOS_IMU3000_Read(PIOS_IMU3000_FIFO_CNT_MSB, (uint8_t *) &fifo_level_data, sizeof(fifo_level_data)) != 0)
		PIOS_DELAY_WaituS(10);
				
	fifo_level = (fifo_level_data[0] << 8) + fifo_level_data[1];
	
	PIOS_DELAY_WaituS(10);
	
	if(imu3000_first_read) {
		// Stupid system for IMU3000.  If first read from buffer then we will read 4 sensors without fifo
		// footer.  After this we will read out a fifo footer
		if(fifo_level < sizeof(imu3000_read_buffer))
			return;
			
		imu3000_cb_ready = false;
		
		// Leave footer in buffer
		PIOS_IMU3000_Read_Callback(PIOS_IMU3000_FIFO_REG, imu3000_read_buffer, sizeof(imu3000_read_buffer) - 2, imu3000_callback);
		
	} else {
		// Stupid system for IMU3000.  Ensure something is left in buffer
		if(fifo_level < (sizeof(imu3000_read_buffer) + 2))
			return;
		
		uint32_t timeval = PIOS_DELAY_GetRaw();
		imu3000_cb_ready = false;
		
		// Leave footer in buffer
		PIOS_IMU3000_Read_Callback(PIOS_IMU3000_FIFO_REG, imu3000_read_buffer, sizeof(imu3000_read_buffer), imu3000_callback);
		
		imu3000_readtime = PIOS_DELAY_DiffuS(timeval);		
	}
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
		PIOS_IMU3000_IRQHandler();
		EXTI_ClearITPendingBit(EXTI_Line1);
	}
}

#endif

/**
 * @}
 * @}
 */

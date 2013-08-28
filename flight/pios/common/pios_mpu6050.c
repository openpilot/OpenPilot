/**
 ******************************************************************************
 * @addtogroup PIOS PIOS Core hardware abstraction layer
 * @{
 * @addtogroup PIOS_MPU6050 MPU6050 Functions
 * @brief Deals with the hardware interface to the 3-axis gyro
 * @{
 *
 * @file       pios_mpu000.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2012.
 * @brief      MPU6050 6-axis gyro and accel chip
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

#include "fifo_buffer.h"

/* Global Variables */

#define MPU6050_TASK_PRIORITY	(tskIDLE_PRIORITY + configMAX_PRIORITIES - 1)	// max priority
#define MPU6050_TASK_STACK		(384 / 4)

enum pios_mpu6050_dev_magic {
	PIOS_MPU6050_DEV_MAGIC = 0x9da9b3ed,
};

#define PIOS_MPU6050_MAX_DOWNSAMPLE 2
struct mpu6050_dev {
	uint32_t i2c_id;
	xQueueHandle queue;
	const struct pios_mpu6050_cfg * cfg;
	xTaskHandle TaskHandle;
	xSemaphoreHandle data_ready_sema;
	enum pios_mpu6050_dev_magic magic;
};
static uint32_t i2c_port;
//! Global structure for this device device
static struct mpu6050_dev * dev;
volatile bool mpu6050_configured = false;

//! Private functions
static struct mpu6050_dev * PIOS_MPU6050_alloc(void);
static int32_t PIOS_MPU6050_Validate(struct mpu6050_dev * dev);
static void PIOS_MPU6050_Config(struct pios_mpu6050_cfg const * cfg);
static int32_t PIOS_MPU6050_SetReg(uint8_t address, uint8_t buffer);
static int32_t PIOS_MPU6050_GetReg(uint8_t address);
int32_t PIOS_MPU6050_Read(uint8_t address, uint8_t * buffer, uint8_t len);
static int32_t PIOS_MPU6050_Write(uint8_t address, uint8_t buffer);
static int32_t PIOS_MPU6050_FifoDepth(void);
static void PIOS_MPU6050_Task(void *parameters);


#define DEG_TO_RAD (M_PI / 180.0)

#define GRAV 9.81f

/**
 * @brief Allocate a new device
 */
static struct mpu6050_dev * PIOS_MPU6050_alloc(void)
{
	struct mpu6050_dev * mpu6050_dev;
	
	mpu6050_dev = (struct mpu6050_dev *)pvPortMalloc(sizeof(*mpu6050_dev));
	if (!mpu6050_dev) return (NULL);
	
	mpu6050_dev->magic = PIOS_MPU6050_DEV_MAGIC;
	
	mpu6050_dev->queue = xQueueCreate(PIOS_MPU6050_MAX_DOWNSAMPLE, sizeof(struct pios_mpu6050_data));
	if(mpu6050_dev->queue == NULL) {
		vPortFree(mpu6050_dev);
		return NULL;
	}
	
	mpu6050_dev->data_ready_sema = xSemaphoreCreateMutex();
	if(mpu6050_dev->data_ready_sema == NULL) {
		vPortFree(mpu6050_dev);
		return NULL;
	}

	
	return(mpu6050_dev);
}

/**
 * @brief Validate the handle to the spi device
 * @returns 0 for valid device or -1 otherwise
 */
static int32_t PIOS_MPU6050_Validate(struct mpu6050_dev * dev)
{
	if (dev == NULL) 
		return -1;
	if (dev->magic != PIOS_MPU6050_DEV_MAGIC)
		return -2;
	return 0;
}

/**
 * @brief Initialize the MPU6050 3-axis gyro sensor.
 * @return 0 for success, -1 for failure
 */
int32_t PIOS_MPU6050_Init(uint32_t i2c_id,const struct pios_mpu6050_cfg * cfg)
{
	//i2c_port=i2c_id;
	dev = PIOS_MPU6050_alloc();
	if(dev == NULL)
		return -1;
	
	dev->cfg = cfg;
	dev->i2c_id=i2c_id;

	/* Configure the MPU6050 Sensor */
	PIOS_MPU6050_Config(cfg);
	
	int result = xTaskCreate(PIOS_MPU6050_Task, (const signed char *)"PIOS_MPU6050_Task",
						 MPU6050_TASK_STACK, NULL, MPU6050_TASK_PRIORITY,
						 &dev->TaskHandle);
	PIOS_Assert(result == pdPASS);

	/* Set up EXTI line */
	PIOS_EXTI_Init(cfg->exti_cfg);
	return 0;
}

/**
 * @brief Initialize the MPU6050 3-axis gyro sensor
 * \return none
 * \param[in] PIOS_MPU6050_ConfigTypeDef struct to be used to configure sensor.
*
*/
static void PIOS_MPU6050_Config(struct pios_mpu6050_cfg const * cfg)
{
	// Reset chip
	while (PIOS_MPU6050_SetReg(PIOS_MPU6050_PWR_MGMT_REG, 0x80) != 0);
	PIOS_DELAY_WaitmS(300);

	// Reset chip and fifo
	while (PIOS_MPU6050_SetReg(PIOS_MPU6050_USER_CTRL_REG, 0x80 | 0x01 | 0x02 | 0x04) != 0);
	// Wait for reset to finish
	while (PIOS_MPU6050_GetReg(PIOS_MPU6050_USER_CTRL_REG) & 0x07);
	
	//Power management configuration
	while (PIOS_MPU6050_SetReg(PIOS_MPU6050_PWR_MGMT_REG, cfg->Pwr_mgmt_clk) != 0) ;

	// Interrupt configuration
	while (PIOS_MPU6050_SetReg(PIOS_MPU6050_INT_CFG_REG, cfg->interrupt_cfg) != 0) ;

	// Interrupt configuration
	while (PIOS_MPU6050_SetReg(PIOS_MPU6050_INT_EN_REG, cfg->interrupt_en) != 0) ;

	// FIFO storage
#if defined(PIOS_MPU6050_ACCEL)
	// Set the accel scale
	while (PIOS_MPU6050_SetReg(PIOS_MPU6050_ACCEL_CFG_REG, cfg->accel_range) != 0);
	
	while (PIOS_MPU6050_SetReg(PIOS_MPU6050_FIFO_EN_REG, cfg->Fifo_store | PIOS_MPU6050_ACCEL_OUT) != 0);
#else
	while (PIOS_MPU6050_SetReg(PIOS_MPU6050_FIFO_EN_REG, cfg->Fifo_store) != 0);
#endif
	// Sample rate divider
	while (PIOS_MPU6050_SetReg(PIOS_MPU6050_SMPLRT_DIV_REG, cfg->Smpl_rate_div) != 0) ;
	
	// Digital low-pass filter and scale
	while (PIOS_MPU6050_SetReg(PIOS_MPU6050_DLPF_CFG_REG, cfg->filter) != 0) ;
	
	// Digital low-pass filter and scale
	while (PIOS_MPU6050_SetReg(PIOS_MPU6050_GYRO_CFG_REG, cfg->gyro_range) != 0) ;

	// Interrupt configuration
	while (PIOS_MPU6050_SetReg(PIOS_MPU6050_USER_CTRL_REG, cfg->User_ctl) != 0) ;
	
	// Interrupt configuration
	while (PIOS_MPU6050_SetReg(PIOS_MPU6050_PWR_MGMT_REG, cfg->Pwr_mgmt_clk) != 0) ;
	
	// Interrupt configuration
	while (PIOS_MPU6050_SetReg(PIOS_MPU6050_INT_CFG_REG, cfg->interrupt_cfg) != 0) ;
	
	// Interrupt configuration
	while (PIOS_MPU6050_SetReg(PIOS_MPU6050_INT_EN_REG, cfg->interrupt_en) != 0) ;
	
	mpu6050_configured = true;
}

/**
 * @brief Read a register from MPU6050
 * @returns The register value or -1 if failure to get bus
 * @param reg[in] Register address to be read
 */
static int32_t PIOS_MPU6050_GetReg(uint8_t reg)
{
	uint8_t data;
	
	PIOS_MPU6050_Read(reg, (uint8_t *) &data, 1);
	//PIOS_SPI_TransferByte(dev->spi_id,(0x80 | reg) ); // request byte
	//data = PIOS_SPI_TransferByte(dev->spi_id,0 );     // receive response
	
	return data;
}

/**
 * @brief Writes one byte to the MPU6050
 * \param[in] reg Register address
 * \param[in] data Byte to write
 * \return 0 if operation was successful
 * \return -1 if unable to claim SPI bus
 * \return -2 if unable to claim i2c device
 */
static int32_t PIOS_MPU6050_SetReg(uint8_t reg, uint8_t data)
{
	if(PIOS_MPU6050_Write(reg, data)!=0)
	{
		return -3;
	}
	return 0;
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
 * @brief Read current X, Z, Y values (in that order)
 * \param[out] int16_t array of size 3 to store X, Z, and Y magnetometer readings
 * \returns The number of samples remaining in the fifo
*/
int32_t PIOS_MPU6050_ReadAcc(struct pios_mpu6050_data * data)
{
	uint8_t buf[6];
	if(PIOS_MPU6050_Read(PIOS_MPU6050_ACCEL_X_OUT_MSB, (uint8_t *) buf, sizeof(buf)) < 0)
		return -1;
	data->accel_x = buf[0] << 8 | buf[1];
	data->accel_y = buf[2] << 8 | buf[3];
	data->accel_z = buf[4] << 8 | buf[5];
	return 0;
}

/*
 * @brief Read the identification bytes from the MPU6050 sensor
 * \return ID read from MPU6050 or -1 if failure
*/
int32_t PIOS_MPU6050_ReadID()
{
	int32_t mpu6050_id = PIOS_MPU6050_GetReg(PIOS_MPU6050_WHOAMI);
	if(mpu6050_id < 0)
		return -1;
	return mpu6050_id;
}

/**
 * \brief Reads the queue handle
 * \return Handle to the queue or null if invalid device
 */
xQueueHandle PIOS_MPU6050_GetQueue()
{
	if(PIOS_MPU6050_Validate(dev) != 0)
		return (xQueueHandle) NULL;
	
	return dev->queue;
}


float PIOS_MPU6050_GetScale() 
{
	switch (dev->cfg->gyro_range) {
		case PIOS_MPU6050_SCALE_250_DEG:
			return 1.0f / 131.0f;
		case PIOS_MPU6050_SCALE_500_DEG:
			return 1.0f / 65.5f;
		case PIOS_MPU6050_SCALE_1000_DEG:
			return 1.0f / 32.8f;
		case PIOS_MPU6050_SCALE_2000_DEG:
			return 1.0f / 16.4f;
	}
	return 0;
}

float PIOS_MPU6050_GetAccelScale()
{
	switch (dev->cfg->accel_range) {
		case PIOS_MPU6050_ACCEL_2G:
			return GRAV / 16384.0f;
		case PIOS_MPU6050_ACCEL_4G:
			return GRAV / 8192.0f;
		case PIOS_MPU6050_ACCEL_8G:
			return GRAV / 4096.0f;
		case PIOS_MPU6050_ACCEL_16G:
			return GRAV / 2048.0f;
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
	
	if(mpu6050_id != 0x68);
		return -2;
	
	return 0;
}

/**
 * @brief Run self-test operation.
 * \return 0 if test succeeded
 * \return non-zero value if test succeeded
 */
static int32_t PIOS_MPU6050_FifoDepth(void)
{
	uint8_t mpu6050_rec_buf[2];

	if(PIOS_MPU6050_Read(PIOS_MPU6050_FIFO_CNT_MSB, (uint8_t *) &mpu6050_rec_buf, 2)< 0) {
		return -1;
	}
	
	return (mpu6050_rec_buf[0] << 8) | mpu6050_rec_buf[1];
}

/**
* @brief IRQ Handler.  Read all the data from onboard buffer
*/
uint32_t mpu6050_irq = 0;
int32_t mpu6050_count;
uint32_t mpu6050_fifo_backup = 0;

uint8_t mpu6050_last_read_count = 0;
uint32_t mpu6050_fails = 0;

uint32_t mpu6050_interval_us;
uint32_t mpu6050_time_us;
uint32_t mpu6050_transfer_size;

bool PIOS_MPU6050_IRQHandler(void)
{
    portBASE_TYPE xHigherPriorityTaskWoken;
    xSemaphoreGiveFromISR(dev->data_ready_sema, &xHigherPriorityTaskWoken);

    return xHigherPriorityTaskWoken == pdTRUE;
}

void PIOS_MPU6050_Task(void *parameters)
{
while (1)
	{
		//Wait for data ready interrupt
		if (xSemaphoreTake(dev->data_ready_sema, portMAX_DELAY) != pdTRUE)
			continue;

		static uint32_t timeval;
		//PIOS_LED_Toggle(PIOS_LED_ALARM);
		mpu6050_interval_us = PIOS_DELAY_DiffuS(timeval);
		timeval = PIOS_DELAY_GetRaw();

		if(!mpu6050_configured)
			return false;

		mpu6050_count = PIOS_MPU6050_FifoDepth();
		
		if(mpu6050_count < sizeof(struct pios_mpu6050_data))
			return false;
			
			
		uint8_t mpu6050_rec_buf[sizeof(struct pios_mpu6050_data)];
		
		if(PIOS_MPU6050_Read(PIOS_MPU6050_FIFO_REG, (uint8_t *) &mpu6050_rec_buf, sizeof(mpu6050_rec_buf))<0){
			mpu6050_fails++;
			return false;
		}

		struct pios_mpu6050_data data;

		// In the case where extras samples backed up grabbed an extra
		if (mpu6050_count >= (sizeof(data) * 2)) {
			mpu6050_fifo_backup++;
			
			
			if(PIOS_MPU6050_Read(PIOS_MPU6050_FIFO_REG, (uint8_t *) &mpu6050_rec_buf, sizeof(mpu6050_rec_buf))<0){
			mpu6050_fails++;
			return false;
			}
			
		}
		
		// Rotate the sensor to OP convention.  The datasheet defines X as towards the right
		// and Y as forward.  OP convention transposes this.  Also the Z is defined negatively
		// to our convention
	#if defined(PIOS_MPU6050_ACCEL)
		// Currently we only support rotations on top so switch X/Y accordingly
		switch(dev->cfg->orientation) {
			case PIOS_MPU6050_TOP_0DEG:
				data.accel_y = mpu6050_rec_buf[0] << 8 | mpu6050_rec_buf[1];      // chip X
				data.accel_x = mpu6050_rec_buf[2] << 8 | mpu6050_rec_buf[3];      // chip Y
				data.gyro_y  = mpu6050_rec_buf[8] << 8  | mpu6050_rec_buf[9];    // chip X
				data.gyro_x  = mpu6050_rec_buf[10] << 8 | mpu6050_rec_buf[11];    // chip Y
				break;
			case PIOS_MPU6050_TOP_90DEG:
				data.accel_y =-1 -(mpu6050_rec_buf[2] << 8 | mpu6050_rec_buf[3]);   // chip Y
				data.accel_x = mpu6050_rec_buf[0] << 8 | mpu6050_rec_buf[1];      // chip X
				data.gyro_y  =-1 -(mpu6050_rec_buf[10] << 8 | mpu6050_rec_buf[11]); // chip Y
				data.gyro_x  = mpu6050_rec_buf[8] << 8  | mpu6050_rec_buf[9];    // chip X
				break;
			case PIOS_MPU6050_TOP_180DEG:
				data.accel_y =-1 -(mpu6050_rec_buf[0] << 8 | mpu6050_rec_buf[1]);   // chip X
				data.accel_x =-1 -(mpu6050_rec_buf[2] << 8 | mpu6050_rec_buf[3]);   // chip Y
				data.gyro_y  =-1 -(mpu6050_rec_buf[8] << 8  | mpu6050_rec_buf[9]); // chip X
				data.gyro_x  =-1 -(mpu6050_rec_buf[10] << 8 | mpu6050_rec_buf[11]); // chip Y
				break;
			case PIOS_MPU6050_TOP_270DEG:
				data.accel_y = mpu6050_rec_buf[2] << 8 | mpu6050_rec_buf[3];      // chip Y
				data.accel_x =-1 -(mpu6050_rec_buf[0] << 8 | mpu6050_rec_buf[1]);   // chip X
				data.gyro_y  = mpu6050_rec_buf[10] << 8 | mpu6050_rec_buf[11];    // chip Y
				data.gyro_x  =-1 -(mpu6050_rec_buf[8] << 8  | mpu6050_rec_buf[9]); // chip X
				break;
		}
		data.gyro_z  =-1 -(mpu6050_rec_buf[12] << 8 | mpu6050_rec_buf[13]);
		data.accel_z =-1 -(mpu6050_rec_buf[4] << 8 | mpu6050_rec_buf[5]);
		data.temperature = mpu6050_rec_buf[6] << 8 | mpu6050_rec_buf[7];
	#else
		data.gyro_x = mpu6050_rec_buf[2] << 8 | mpu6050_rec_buf[3];
		data.gyro_y = mpu6050_rec_buf[4] << 8 | mpu6050_rec_buf[5];
		switch(dev->cfg->orientation) {
			case PIOS_MPU6050_TOP_0DEG:
				data.gyro_y  = mpu6050_rec_buf[2] << 8 | mpu6050_rec_buf[3];
				data.gyro_x  = mpu6050_rec_buf[4] << 8 | mpu6050_rec_buf[5];
				break;
			case PIOS_MPU6050_TOP_90DEG:
				data.gyro_y  =-1 -(mpu6050_rec_buf[4] << 8 | mpu6050_rec_buf[5]); // chip Y
				data.gyro_x  = mpu6050_rec_buf[2] << 8 | mpu6050_rec_buf[3];    // chip X
				break;
			case PIOS_MPU6050_TOP_180DEG:
				data.gyro_y  =-1 -(mpu6050_rec_buf[2] << 8 | mpu6050_rec_buf[3]);
				data.gyro_x  =-1 -(mpu6050_rec_buf[4] << 8 | mpu6050_rec_buf[5]);
				break;
			case PIOS_MPU6050_TOP_270DEG:
				data.gyro_y  = mpu6050_rec_buf[4] << 8 | mpu6050_rec_buf[5];    // chip Y
				data.gyro_x  =-1 -(mpu6050_rec_buf[2] << 8 | mpu6050_rec_buf[3]); // chip X
				break;
		}
		data.gyro_z =-1 -(mpu6050_rec_buf[6] << 8 | mpu6050_rec_buf[7]);
		data.temperature = mpu6050_rec_buf[0] << 8 | mpu6050_rec_buf[1];
	#endif
			xQueueSend(dev->queue, (void *) &data, 0);

		mpu6050_irq++;

		mpu6050_time_us = PIOS_DELAY_DiffuS(timeval);
	}
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
int32_t PIOS_MPU6050_Read(uint8_t address, uint8_t * buffer, uint8_t len)
{

	const struct pios_i2c_txn txn_list[] = {
		{
			.info = __func__,
			.addr = PIOS_MPU6050_I2C_ADDR,
			.rw = PIOS_I2C_TXN_WRITE,
			.len = 1,
			.buf = &address,
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

	return PIOS_I2C_Transfer(dev->i2c_id, txn_list, NELEMENTS(txn_list));
}


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

	return PIOS_I2C_Transfer(dev->i2c_id, txn_list, NELEMENTS(txn_list));
	//return 0;
	//return PIOS_I2C_Transfer(PIOS_I2C_GYRO_ADAPTER, txn_list, NELEMENTS(txn_list));
}

#endif

/**
 * @}
 * @}
 */

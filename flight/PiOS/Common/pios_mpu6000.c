/**
 ******************************************************************************
 * @addtogroup PIOS PIOS Core hardware abstraction layer
 * @{
 * @addtogroup PIOS_MPU6000 MPU6000 Functions
 * @brief Deals with the hardware interface to the 3-axis gyro
 * @{
 *
 * @file       pios_mpu000.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2012.
 * @brief      MPU6000 6-axis gyro and accel chip
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
#include "mpu6000settings.h"
#if defined(PIOS_INCLUDE_MPU6000)

#include "fifo_buffer.h"

/* Global Variables */

enum pios_mpu6000_dev_magic {
	PIOS_MPU6000_DEV_MAGIC = 0x9da9b3ed,
};

#define PIOS_MPU6000_MAX_DOWNSAMPLE 2
struct mpu6000_dev {
	uint32_t spi_id;
	uint32_t slave_num;
	xQueueHandle queue;
	const struct pios_mpu6000_cfg * cfg;
	enum pios_mpu6000_range gyro_range;
	enum pios_mpu6000_accel_range accel_range;
	enum pios_mpu6000_dev_magic magic;
};

//! Global structure for this device device
static struct mpu6000_dev * dev;
volatile bool mpu6000_configured = false;

//! Private functions
static struct mpu6000_dev * PIOS_MPU6000_alloc(void);
static int32_t PIOS_MPU6000_Validate(struct mpu6000_dev * dev);
static void PIOS_MPU6000_Config(struct pios_mpu6000_cfg const * cfg);
static int32_t PIOS_MPU6000_SetReg(uint8_t address, uint8_t buffer);
static int32_t PIOS_MPU6000_GetReg(uint8_t address);

static uint8_t getGyroRange(enum pios_mpu6000_range range);
static uint8_t getAccelRange(enum pios_mpu6000_accel_range accel);
static uint8_t getFilterSetting(enum pios_mpu6000_filter filter);

#define DEG_TO_RAD (M_PI / 180.0)

#define GRAV 9.81f

/**
 * @brief Allocate a new device
 */
static struct mpu6000_dev * PIOS_MPU6000_alloc(void)
{
	struct mpu6000_dev * mpu6000_dev;
	
	mpu6000_dev = (struct mpu6000_dev *)pvPortMalloc(sizeof(*mpu6000_dev));
	if (!mpu6000_dev) return (NULL);
	
	mpu6000_dev->magic = PIOS_MPU6000_DEV_MAGIC;
	
	mpu6000_dev->queue = xQueueCreate(PIOS_MPU6000_MAX_DOWNSAMPLE, sizeof(struct pios_mpu6000_data));
	if(mpu6000_dev->queue == NULL) {
		vPortFree(mpu6000_dev);
		return NULL;
	}
	
	return(mpu6000_dev);
}

/**
 * @brief Validate the handle to the spi device
 * @returns 0 for valid device or -1 otherwise
 */
static int32_t PIOS_MPU6000_Validate(struct mpu6000_dev * dev)
{
	if (dev == NULL) 
		return -1;
	if (dev->magic != PIOS_MPU6000_DEV_MAGIC)
		return -2;
	if (dev->spi_id == 0)
		return -3;
	return 0;
}

/**
 * @brief Initialize the MPU6000 3-axis gyro sensor.
 * @return 0 for success, -1 for failure
 */
int32_t PIOS_MPU6000_Init(uint32_t spi_id, uint32_t slave_num, const struct pios_mpu6000_cfg * cfg)
{
	dev = PIOS_MPU6000_alloc();
	if(dev == NULL)
		return -1;
	
	dev->spi_id = spi_id;
	dev->slave_num = slave_num;
	dev->cfg = cfg;

	/* Configure the MPU6000 Sensor */
	PIOS_SPI_SetClockSpeed(dev->spi_id, PIOS_SPI_PRESCALER_256);
	PIOS_MPU6000_Config(cfg);
	PIOS_SPI_SetClockSpeed(dev->spi_id, PIOS_SPI_PRESCALER_16);

	/* Set up EXTI line */
	PIOS_EXTI_Init(cfg->exti_cfg);
	return 0;
}

/**
 * @brief Initialize the MPU6000 3-axis gyro sensor
 * \return none
 * \param[in] PIOS_MPU6000_ConfigTypeDef struct to be used to configure sensor.
*
*/
static void PIOS_MPU6000_Config(struct pios_mpu6000_cfg const * cfg)
{

	PIOS_MPU6000_Test();
	
	//initialize settings for acc/gyro Scale and filter
	Mpu6000SettingsInitialize();
	
	// Reset chip
	while (PIOS_MPU6000_SetReg(PIOS_MPU6000_PWR_MGMT_REG, PIOS_MPU6000_PWRMGMT_IMU_RST) != 0);
	PIOS_DELAY_WaitmS(300);
	
	// Reset chip and fifo
	while (PIOS_MPU6000_SetReg(PIOS_MPU6000_USER_CTRL_REG, 
		PIOS_MPU6000_USERCTL_GYRO_RST | 
		PIOS_MPU6000_USERCTL_SIG_COND | 
		PIOS_MPU6000_USERCTL_FIFO_RST) != 0);
	
	// Wait for reset to finish
	while (PIOS_MPU6000_GetReg(PIOS_MPU6000_USER_CTRL_REG) & 
		(PIOS_MPU6000_USERCTL_GYRO_RST | 
		PIOS_MPU6000_USERCTL_SIG_COND | 
		PIOS_MPU6000_USERCTL_FIFO_RST));
	
	//Power management configuration
	while (PIOS_MPU6000_SetReg(PIOS_MPU6000_PWR_MGMT_REG, cfg->Pwr_mgmt_clk) != 0) ;

	// Interrupt configuration
	while (PIOS_MPU6000_SetReg(PIOS_MPU6000_INT_CFG_REG, cfg->interrupt_cfg) != 0) ;

	// Interrupt configuration
	while (PIOS_MPU6000_SetReg(PIOS_MPU6000_INT_EN_REG, cfg->interrupt_en) != 0) ;

	// FIFO storage
#if defined(PIOS_MPU6000_ACCEL)
	// Set the accel range
	dev->accel_range = getAccelRange(cfg->accel_range);
	while (PIOS_MPU6000_SetReg(PIOS_MPU6000_ACCEL_CFG_REG, dev->accel_range) != 0);
	
	while (PIOS_MPU6000_SetReg(PIOS_MPU6000_FIFO_EN_REG, cfg->Fifo_store | PIOS_MPU6000_ACCEL_OUT) != 0);
#else
	while (PIOS_MPU6000_SetReg(PIOS_MPU6000_FIFO_EN_REG, cfg->Fifo_store) != 0);
#endif
	
	// Sample rate divider
	while (PIOS_MPU6000_SetReg(PIOS_MPU6000_SMPLRT_DIV_REG, cfg->Smpl_rate_div) != 0) ;
	
	// Digital low-pass filter and scale
	uint8_t filterSetting;
	filterSetting = getFilterSetting(cfg->filter);
	while (PIOS_MPU6000_SetReg(PIOS_MPU6000_DLPF_CFG_REG, filterSetting) != 0) ;
	
	// Gyro range
	dev->gyro_range = getGyroRange(cfg->gyro_range);
	while (PIOS_MPU6000_SetReg(PIOS_MPU6000_GYRO_CFG_REG, dev->gyro_range) != 0) ;
	
	// Interrupt configuration
	while (PIOS_MPU6000_SetReg(PIOS_MPU6000_USER_CTRL_REG, cfg->User_ctl) != 0) ;
	
	// Interrupt configuration
	while (PIOS_MPU6000_SetReg(PIOS_MPU6000_PWR_MGMT_REG, cfg->Pwr_mgmt_clk) != 0) ;
	
	// Interrupt configuration
	while (PIOS_MPU6000_SetReg(PIOS_MPU6000_INT_CFG_REG, cfg->interrupt_cfg) != 0) ;

	// Interrupt configuration
	while (PIOS_MPU6000_SetReg(PIOS_MPU6000_INT_EN_REG, cfg->interrupt_en) != 0) ;
	if((PIOS_MPU6000_GetReg(PIOS_MPU6000_INT_EN_REG)) != cfg->interrupt_en)
		return;
	
	mpu6000_configured = true;
}

/**
 * @brief Claim the SPI bus for the accel communications and select this chip
 * @return 0 if successful, -1 for invalid device, -2 if unable to claim bus
 */
int32_t PIOS_MPU6000_ClaimBus()
{
	if(PIOS_MPU6000_Validate(dev) != 0)
		return -1;
	
	if(PIOS_SPI_ClaimBus(dev->spi_id) != 0)
		return -2;
	
	PIOS_SPI_RC_PinSet(dev->spi_id,dev->slave_num,0);
	return 0;
}

/**
 * @brief Release the SPI bus for the accel communications and end the transaction
 * @return 0 if successful
 */
int32_t PIOS_MPU6000_ReleaseBus()
{
	if(PIOS_MPU6000_Validate(dev) != 0)
		return -1;
	
	PIOS_SPI_RC_PinSet(dev->spi_id,dev->slave_num,1);
	
	return PIOS_SPI_ReleaseBus(dev->spi_id);
}

/**
 * @brief Read a register from MPU6000
 * @returns The register value or -1 if failure to get bus
 * @param reg[in] Register address to be read
 */
static int32_t PIOS_MPU6000_GetReg(uint8_t reg)
{
	uint8_t data;
	
	if(PIOS_MPU6000_ClaimBus() != 0)
		return -1;	
	
	PIOS_SPI_TransferByte(dev->spi_id,(0x80 | reg) ); // request byte
	data = PIOS_SPI_TransferByte(dev->spi_id,0 );     // receive response
	
	PIOS_MPU6000_ReleaseBus();
	return data;
}

/**
 * @brief Writes one byte to the MPU6000
 * \param[in] reg Register address
 * \param[in] data Byte to write
 * \return 0 if operation was successful
 * \return -1 if unable to claim SPI bus
 * \return -2 if unable to claim i2c device
 */
static int32_t PIOS_MPU6000_SetReg(uint8_t reg, uint8_t data)
{
	if(PIOS_MPU6000_ClaimBus() != 0)
		return -1;
	
	if(PIOS_SPI_TransferByte(dev->spi_id, 0x7f & reg) != 0) {
		PIOS_MPU6000_ReleaseBus();
		return -2;
	}
	
	if(PIOS_SPI_TransferByte(dev->spi_id, data) != 0) {
		PIOS_MPU6000_ReleaseBus();
		return -3;
	}
	
	PIOS_MPU6000_ReleaseBus();
	
	return 0;
}

/**
 * @brief Read current X, Z, Y values (in that order)
 * \param[out] int16_t array of size 3 to store X, Z, and Y magnetometer readings
 * \returns 0 if succesful
 */
int32_t PIOS_MPU6000_ReadGyros(struct pios_mpu6000_data * data)
{
	// THIS FUNCTION IS DEPRECATED AND DOES NOT PERFORM A ROTATION
	uint8_t buf[7] = {PIOS_MPU6000_GYRO_X_OUT_MSB | 0x80, 0, 0, 0, 0, 0, 0};
	uint8_t rec[7];
	
	if(PIOS_MPU6000_ClaimBus() != 0)
		return -1;

	if(PIOS_SPI_TransferBlock(dev->spi_id, &buf[0], &rec[0], sizeof(buf), NULL) < 0)
		return -2;
		
	PIOS_MPU6000_ReleaseBus();
	
	data->gyro_x = rec[1] << 8 | rec[2];
	data->gyro_y = rec[3] << 8 | rec[4];
	data->gyro_z = rec[5] << 8 | rec[6];
	
	return 0;
}

/*
 * @brief Read the identification bytes from the MPU6000 sensor
 * \return ID read from MPU6000 or -1 if failure
*/
int32_t PIOS_MPU6000_ReadID()
{
	int32_t mpu6000_id = PIOS_MPU6000_GetReg(PIOS_MPU6000_WHOAMI);
	if(mpu6000_id < 0)
		return -1;
	return mpu6000_id;
}

/**
 * \brief Reads the queue handle
 * \return Handle to the queue or null if invalid device
 */
xQueueHandle PIOS_MPU6000_GetQueue()
{
	if(PIOS_MPU6000_Validate(dev) != 0)
		return (xQueueHandle) NULL;
	
	return dev->queue;
}


float PIOS_MPU6000_GetScale() 
{
	switch (dev->gyro_range) {
		case PIOS_MPU6000_SCALE_250_DEG:
			return 1.0f / 131.0f;
		case PIOS_MPU6000_SCALE_500_DEG:
			return 1.0f / 65.5f;
		case PIOS_MPU6000_SCALE_1000_DEG:
			return 1.0f / 32.8f;
		case PIOS_MPU6000_SCALE_FROM_SETTINGS:
		case PIOS_MPU6000_SCALE_2000_DEG:
			return 1.0f / 16.4f;
	}
	return 0;
}

float PIOS_MPU6000_GetAccelScale()
{
	switch (dev->accel_range) {
		case PIOS_MPU6000_ACCEL_2G:
			return GRAV / 16384.0f;
		case PIOS_MPU6000_ACCEL_4G:
			return GRAV / 8192.0f;
		case PIOS_MPU6000_ACCEL_8G:
		case PIOS_MPU6000_ACCEL_FROM_SETTINGS:
			return GRAV / 4096.0f;
		case PIOS_MPU6000_ACCEL_16G:
			return GRAV / 2048.0f;
	}
	return 0;
}

/**
 * @brief Run self-test operation.
 * \return 0 if test succeeded
 * \return non-zero value if test succeeded
 */
int32_t PIOS_MPU6000_Test(void)
{
	/* Verify that ID matches (MPU6000 ID is 0x69) */
	int32_t mpu6000_id = PIOS_MPU6000_ReadID();
	if(mpu6000_id < 0)
		return -1;
	
	if(mpu6000_id != 0x68)
		return -2;
	
	return 0;
}

/**
 * @brief Run self-test operation.
 * \return 0 if test succeeded
 * \return non-zero value if test succeeded
 */
static int32_t PIOS_MPU6000_FifoDepth(void)
{
	uint8_t mpu6000_send_buf[3] = {PIOS_MPU6000_FIFO_CNT_MSB | 0x80, 0, 0};
	uint8_t mpu6000_rec_buf[3];

	if(PIOS_MPU6000_ClaimBus() != 0)
		return -1;

	if(PIOS_SPI_TransferBlock(dev->spi_id, &mpu6000_send_buf[0], &mpu6000_rec_buf[0], sizeof(mpu6000_send_buf), NULL) < 0) {
		PIOS_MPU6000_ReleaseBus();
		return -1;
	}

	PIOS_MPU6000_ReleaseBus();
	
	return (mpu6000_rec_buf[1] << 8) | mpu6000_rec_buf[2];
}

/**
* @brief IRQ Handler.  Read all the data from onboard buffer
*/
uint32_t mpu6000_irq = 0;
int32_t mpu6000_count;
uint32_t mpu6000_fifo_backup = 0;

uint8_t mpu6000_last_read_count = 0;
uint32_t mpu6000_fails = 0;

uint32_t mpu6000_interval_us;
uint32_t mpu6000_time_us;
uint32_t mpu6000_transfer_size;

bool PIOS_MPU6000_IRQHandler(void)
{
	static uint32_t timeval;
	mpu6000_interval_us = PIOS_DELAY_DiffuS(timeval);
	timeval = PIOS_DELAY_GetRaw();

	if (!mpu6000_configured)
		return false;

	mpu6000_count = PIOS_MPU6000_FifoDepth();
	if (mpu6000_count < sizeof(struct pios_mpu6000_data))
		return false;

	if (PIOS_MPU6000_ClaimBus() != 0)
		return false;

	uint8_t mpu6000_send_buf[1 + sizeof(struct pios_mpu6000_data) ] = {PIOS_MPU6000_FIFO_REG | 0x80, 0, 0, 0, 0, 0, 0, 0, 0};
	uint8_t mpu6000_rec_buf[1 + sizeof(struct pios_mpu6000_data) ];

	if (PIOS_SPI_TransferBlock(dev->spi_id, &mpu6000_send_buf[0], &mpu6000_rec_buf[0], sizeof(mpu6000_send_buf), NULL) < 0) {
		PIOS_MPU6000_ReleaseBus();
		mpu6000_fails++;
		return false;
	}

	PIOS_MPU6000_ReleaseBus();

	struct pios_mpu6000_data data;

	// In the case where extras samples backed up grabbed an extra
	if (mpu6000_count >= (sizeof(data) * 2)) {
		mpu6000_fifo_backup++;
		if (PIOS_MPU6000_ClaimBus() != 0)
			return false;

		if (PIOS_SPI_TransferBlock(dev->spi_id, &mpu6000_send_buf[0], &mpu6000_rec_buf[0], sizeof(mpu6000_send_buf), NULL) < 0) {
			PIOS_MPU6000_ReleaseBus();
			mpu6000_fails++;
			return false;
		}

		PIOS_MPU6000_ReleaseBus();
	}

	// Rotate the sensor to OP convention.  The datasheet defines X as towards the right
	// and Y as forward.  OP convention transposes this.  Also the Z is defined negatively
	// to our convention
#if defined(PIOS_MPU6000_ACCEL)
	// Currently we only support rotations on top so switch X/Y accordingly
	switch (dev->cfg->orientation) {
	case PIOS_MPU6000_TOP_0DEG:
		data.accel_y = mpu6000_rec_buf[1] << 8 | mpu6000_rec_buf[2]; // chip X
		data.accel_x = mpu6000_rec_buf[3] << 8 | mpu6000_rec_buf[4]; // chip Y
		data.gyro_y = mpu6000_rec_buf[9] << 8 | mpu6000_rec_buf[10]; // chip X
		data.gyro_x = mpu6000_rec_buf[11] << 8 | mpu6000_rec_buf[12]; // chip Y
		break;
	case PIOS_MPU6000_TOP_90DEG:
		// -1 to bring it back to -32768 +32767 range
		data.accel_y = -1 - (mpu6000_rec_buf[3] << 8 | mpu6000_rec_buf[4]); // chip Y
		data.accel_x = mpu6000_rec_buf[1] << 8 | mpu6000_rec_buf[2]; // chip X
		data.gyro_y = -1 - (mpu6000_rec_buf[11] << 8 | mpu6000_rec_buf[12]); // chip Y
		data.gyro_x = mpu6000_rec_buf[9] << 8 | mpu6000_rec_buf[10]; // chip X
		break;
	case PIOS_MPU6000_TOP_180DEG:
		data.accel_y = -1 - (mpu6000_rec_buf[1] << 8 | mpu6000_rec_buf[2]); // chip X
		data.accel_x = -1 - (mpu6000_rec_buf[3] << 8 | mpu6000_rec_buf[4]); // chip Y
		data.gyro_y = -1 - (mpu6000_rec_buf[9] << 8 | mpu6000_rec_buf[10]); // chip X
		data.gyro_x = -1 - (mpu6000_rec_buf[11] << 8 | mpu6000_rec_buf[12]); // chip Y
		break;
	case PIOS_MPU6000_TOP_270DEG:
		data.accel_y = mpu6000_rec_buf[3] << 8 | mpu6000_rec_buf[4]; // chip Y
		data.accel_x = -1 - (mpu6000_rec_buf[1] << 8 | mpu6000_rec_buf[2]); // chip X
		data.gyro_y = mpu6000_rec_buf[11] << 8 | mpu6000_rec_buf[12]; // chip Y
		data.gyro_x = -1 - (mpu6000_rec_buf[9] << 8 | mpu6000_rec_buf[10]); // chip X
		break;
	}
	data.gyro_z = -1 - (mpu6000_rec_buf[13] << 8 | mpu6000_rec_buf[14]);
	data.accel_z = -1 - (mpu6000_rec_buf[5] << 8 | mpu6000_rec_buf[6]);
	data.temperature = mpu6000_rec_buf[7] << 8 | mpu6000_rec_buf[8];
#else
	data.gyro_x = mpu6000_rec_buf[3] << 8 | mpu6000_rec_buf[4];
	data.gyro_y = mpu6000_rec_buf[5] << 8 | mpu6000_rec_buf[6];
	switch (dev->cfg->orientation) {
	case PIOS_MPU6000_TOP_0DEG:
		data.gyro_y = mpu6000_rec_buf[3] << 8 | mpu6000_rec_buf[4];
		data.gyro_x = mpu6000_rec_buf[5] << 8 | mpu6000_rec_buf[6];
		break;
	case PIOS_MPU6000_TOP_90DEG:
		data.gyro_y = -1 - (mpu6000_rec_buf[5] << 8 | mpu6000_rec_buf[6]); // chip Y
		data.gyro_x = mpu6000_rec_buf[3] << 8 | mpu6000_rec_buf[4]; // chip X
		break;
	case PIOS_MPU6000_TOP_180DEG:
		data.gyro_y = -1 - (mpu6000_rec_buf[3] << 8 | mpu6000_rec_buf[4]);
		data.gyro_x = -1 - (mpu6000_rec_buf[5] << 8 | mpu6000_rec_buf[6]);
		break;
	case PIOS_MPU6000_TOP_270DEG:
		data.gyro_y = mpu6000_rec_buf[5] << 8 | mpu6000_rec_buf[6]; // chip Y
		data.gyro_x = -1 - (mpu6000_rec_buf[3] << 8 | mpu6000_rec_buf[4]); // chip X
		break;
	}
	data.gyro_z = -1 - (mpu6000_rec_buf[7] << 8 | mpu6000_rec_buf[8]);
	data.temperature = mpu6000_rec_buf[1] << 8 | mpu6000_rec_buf[2];
#endif

	portBASE_TYPE xHigherPriorityTaskWoken;
	xQueueSendToBackFromISR(dev->queue, (void *) &data, &xHigherPriorityTaskWoken);

	mpu6000_irq++;

	mpu6000_time_us = PIOS_DELAY_DiffuS(timeval);

	return xHigherPriorityTaskWoken == pdTRUE;
}

/**
 * @brief Return the gyro range setting based on config and/or mpu6000settings.
 * \return the chosen gyro range
 */
static uint8_t getGyroRange(enum pios_mpu6000_range gyro)
{
	// if the setting is overridden by the board config then use the board value
	if (gyro != PIOS_MPU6000_SCALE_FROM_SETTINGS)
		return(uint8_t) gyro;

	uint8_t gyroSettings;
	Mpu6000SettingsGyroScaleGet(&gyroSettings);

	switch (gyroSettings) {
	case MPU6000SETTINGS_GYROSCALE_SCALE_250:
		return PIOS_MPU6000_SCALE_250_DEG;
	case MPU6000SETTINGS_GYROSCALE_SCALE_500:
		return PIOS_MPU6000_SCALE_500_DEG;
	case MPU6000SETTINGS_GYROSCALE_SCALE_1000:
		return PIOS_MPU6000_SCALE_1000_DEG;
	case MPU6000SETTINGS_GYROSCALE_SCALE_2000:
		return PIOS_MPU6000_SCALE_2000_DEG;
	default:
		return PIOS_MPU6000_SCALE_2000_DEG;
	}
}

/**
 * @brief Return the accel range setting based on config and/or mpu6000settings.
 * \return the chosen accel range
 */
static uint8_t getAccelRange(enum pios_mpu6000_accel_range accel)
{
	// if the setting is overridden by the board config then use the board value
	if (accel != PIOS_MPU6000_ACCEL_FROM_SETTINGS)
		return (uint8_t) accel;

	uint8_t accelSetting;
	Mpu6000SettingsAccelScaleGet(&accelSetting);

	switch (accelSetting) {
	case MPU6000SETTINGS_ACCELSCALE_SCALE_2G:
		return PIOS_MPU6000_ACCEL_2G;
	case MPU6000SETTINGS_ACCELSCALE_SCALE_4G:
		return PIOS_MPU6000_ACCEL_4G;
	case MPU6000SETTINGS_ACCELSCALE_SCALE_8G:
		return PIOS_MPU6000_ACCEL_8G;
	case MPU6000SETTINGS_ACCELSCALE_SCALE_16G:
		return PIOS_MPU6000_ACCEL_16G;
	default:
		return PIOS_MPU6000_ACCEL_8G;
	}
}

/**
 * @brief Return the filter settings based on config and/or mpu6000settings.
 * \return the chosen filter settings
 */
static uint8_t getFilterSetting(enum pios_mpu6000_filter filter)
{
	// if the setting is overridden by the board config then use the board value
	if (filter != PIOS_MPU6000_LOWPASS_FROM_SETTINGS)
		return(uint8_t) filter;

	uint8_t filterSetting;
	Mpu6000SettingsFilterSettingGet(&filterSetting);

	switch (filterSetting) {
	case MPU6000SETTINGS_FILTERSETTING_LOWPASS_256_HZ:
		return PIOS_MPU6000_LOWPASS_256_HZ;
	case MPU6000SETTINGS_FILTERSETTING_LOWPASS_188_HZ:
		return PIOS_MPU6000_LOWPASS_188_HZ;
	case MPU6000SETTINGS_FILTERSETTING_LOWPASS_98_HZ:
		return PIOS_MPU6000_LOWPASS_98_HZ;
	case MPU6000SETTINGS_FILTERSETTING_LOWPASS_42_HZ:
		return PIOS_MPU6000_LOWPASS_42_HZ;
	case MPU6000SETTINGS_FILTERSETTING_LOWPASS_20_HZ:
		return PIOS_MPU6000_LOWPASS_20_HZ;
	case MPU6000SETTINGS_FILTERSETTING_LOWPASS_10_HZ:
		return PIOS_MPU6000_LOWPASS_10_HZ;
	case MPU6000SETTINGS_FILTERSETTING_LOWPASS_5_HZ:
		return PIOS_MPU6000_LOWPASS_5_HZ;
	default:
		return PIOS_MPU6000_LOWPASS_256_HZ;
	}
}

#endif

/**
 * @}
 * @}
 */

/**
 ******************************************************************************
 * @addtogroup PIOS PIOS Core hardware abstraction layer
 * @{
 * @addtogroup PIOS_ADXL345 ADXL345 Functions
 * @brief Deals with the hardware interface to the BMA180 3-axis accelerometer
 * @{
 *
 * @file       pios_adxl345.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2012.
 * @brief      PiOS ADXL345 digital accelerometer driver.
 * @see        The GNU Public License (GPL) Version 3
 *
 *****************************************************************************/
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

#include "pios.h"

enum pios_adxl345_dev_magic {
	PIOS_ADXL345_DEV_MAGIC = 0xcb55aa55,
};

struct adxl345_dev {
	uint32_t spi_id;
	uint32_t slave_num;
	enum pios_adxl345_dev_magic magic;
};

//! Global structure for this device device
static struct adxl345_dev * dev;

//! Private functions
static struct adxl345_dev * PIOS_ADXL345_alloc(void);
static int32_t PIOS_ADXL345_Validate(struct adxl345_dev * dev);
static int32_t PIOS_ADXL345_ClaimBus();
static int32_t PIOS_ADXL345_ReleaseBus();
static int32_t PIOS_ADXL345_FifoDepth(uint8_t depth);

/**
 * @brief Allocate a new device
 */
static struct adxl345_dev * PIOS_ADXL345_alloc(void)
{
	struct adxl345_dev * adxl345_dev;
	
	adxl345_dev = (struct adxl345_dev *)pvPortMalloc(sizeof(*adxl345_dev));
	if (!adxl345_dev) return (NULL);
	
	adxl345_dev->magic = PIOS_ADXL345_DEV_MAGIC;
	return(adxl345_dev);
}

/**
 * @brief Validate the handle to the spi device
 * @returns 0 for valid device or -1 otherwise
 */
static int32_t PIOS_ADXL345_Validate(struct adxl345_dev * dev)
{
	if (dev == NULL) 
		return -1;
	if (dev->magic != PIOS_ADXL345_DEV_MAGIC)
		return -2;
	if (dev->spi_id == 0)
		return -3;
	return 0;
}

/**
 * @brief Claim the SPI bus for the accel communications and select this chip
 * @return 0 for succesful claiming of bus or -1 otherwise
 */
static int32_t PIOS_ADXL345_ClaimBus() 
{
	if(PIOS_ADXL345_Validate(dev) != 0)
		return -1;

	if(PIOS_SPI_ClaimBus(dev->spi_id) != 0)
		return -2;

	PIOS_SPI_RC_PinSet(dev->spi_id, dev->slave_num, 0);
	
	return 0;
}

/**
 * @brief Release the SPI bus for the accel communications and end the transaction
 * @return 0 if success or <0 for failure
 */
static int32_t PIOS_ADXL345_ReleaseBus()
{
	if(PIOS_ADXL345_Validate(dev) != 0)
		return -1;

	PIOS_SPI_RC_PinSet(dev->spi_id, dev->slave_num, 1);

	if(PIOS_SPI_ReleaseBus(dev->spi_id) != 0)
		return -2;
	
	return 0;
}

/**
 * @brief Select the sampling rate of the chip
 * 
 * This also puts it into high power mode
 */
int32_t PIOS_ADXL345_SelectRate(uint8_t rate) 
{
	if(PIOS_ADXL345_Validate(dev) != 0)
		return -1;

	if(PIOS_ADXL345_ClaimBus() != 0)
		return -2;

	uint8_t out[2] = {ADXL_RATE_ADDR, rate & 0x0F};
	if(PIOS_SPI_TransferBlock(dev->spi_id,out,NULL,sizeof(out),NULL) < 0) {
		PIOS_ADXL345_ReleaseBus();
		return -3;
	}

	PIOS_ADXL345_ReleaseBus();
	
	return 0;
}

/**
 * @brief Set the range of the accelerometer and set the data to be right justified
 * with sign extension.  Also keep device in 4 wire mode.
 */
int32_t PIOS_ADXL345_SetRange(uint8_t range) 
{
	if(PIOS_ADXL345_Validate(dev) != 0)
		return -1;
	
	if(PIOS_ADXL345_ClaimBus() != 0)
		return -2;
	
	uint8_t out[2] = {ADXL_FORMAT_ADDR, (range & 0x03) | ADXL_FULL_RES | ADXL_4WIRE};
	if(PIOS_SPI_TransferBlock(dev->spi_id,out,NULL,sizeof(out),NULL) < 0) {
		PIOS_ADXL345_ReleaseBus();
		return -3;
	}
	
	PIOS_ADXL345_ReleaseBus();
	
	return 0;
}

/**
 * @brief Set the fifo depth that triggers an interrupt.  This will be matched to the oversampling
 */
static int32_t PIOS_ADXL345_FifoDepth(uint8_t depth)
{
	if(PIOS_ADXL345_Validate(dev) != 0)
		return -1;
	
	if(PIOS_ADXL345_ClaimBus() != 0)
		return -2;
	
	uint8_t out[2] = {ADXL_FIFO_ADDR, (depth & 0x1f) | ADXL_FIFO_STREAM};
	if(PIOS_SPI_TransferBlock(dev->spi_id,out,NULL,sizeof(out),NULL) < 0) {
		PIOS_ADXL345_ReleaseBus();
		return -3;
	}

	PIOS_ADXL345_ReleaseBus();
	
	return 0;
}

/**
 * @brief Enable measuring.  This also disables the activity sensors (tap or free fall)
 */
static int32_t PIOS_ADXL345_SetMeasure(uint8_t enable)
{
	if(PIOS_ADXL345_Validate(dev) != 0)
		return -1;
	
	if(PIOS_ADXL345_ClaimBus() != 0)
		return -2;
	
	uint8_t out[2] = {ADXL_POWER_ADDR, ADXL_MEAURE};
	if(PIOS_SPI_TransferBlock(dev->spi_id,out,NULL,sizeof(out),NULL) < 0) {
		PIOS_ADXL345_ReleaseBus();
		return -3;
	}

	PIOS_ADXL345_ReleaseBus();
	
	return 0;
}

/**
 * @brief Initialize with good default settings
 */
int32_t PIOS_ADXL345_Init(uint32_t spi_id, uint32_t slave_num)
{
	dev = PIOS_ADXL345_alloc();
	if(dev == NULL)
		return -1;
	
	dev->spi_id = spi_id;
	dev->slave_num = slave_num;
	
	PIOS_ADXL345_ReleaseBus();
	PIOS_ADXL345_SelectRate(ADXL_RATE_3200);
	PIOS_ADXL345_SetRange(ADXL_RANGE_8G);
	PIOS_ADXL345_FifoDepth(16);
	PIOS_ADXL345_SetMeasure(1);
	
	return 0;
}

/**
 * @brief Return number of entries in the fifo
 */
int32_t PIOS_ADXL345_Test()
{
	if(PIOS_ADXL345_Validate(dev) != 0)
		return -1;

	if(PIOS_ADXL345_ClaimBus() != 0)
		return -2;

	uint8_t buf[2] = {0,0};
	uint8_t rec[2] = {0,0};
	buf[0] = ADXL_WHOAMI | ADXL_READ_BIT;

	if(PIOS_SPI_TransferBlock(dev->spi_id,&buf[0],&rec[0],sizeof(buf),NULL) < 0) {
		PIOS_ADXL345_ReleaseBus();
		return -3;
	}

	PIOS_ADXL345_ReleaseBus();		

	return (rec[1] == ADXL_DEVICE_ID) ? 0 : -4;
}

/**
 * @brief Return number of entries in the fifo
 */
int32_t PIOS_ADXL345_FifoElements()
{
	if(PIOS_ADXL345_Validate(dev) != 0)
		return -1;
	
	if(PIOS_ADXL345_ClaimBus() != 0)
		return -2;
	
	uint8_t buf[2] = {0,0};
	uint8_t rec[2] = {0,0};
	buf[0] = ADXL_FIFOSTATUS_ADDR | ADXL_READ_BIT ; // Read fifo status
	
	if(PIOS_SPI_TransferBlock(dev->spi_id,&buf[0],&rec[0],sizeof(buf),NULL) < 0) {
		PIOS_ADXL345_ReleaseBus();
		return -3;
	}
	
	PIOS_ADXL345_ReleaseBus();		
	
	return rec[1] & 0x3f;
}

/**
 * @brief Read a single set of values from the x y z channels
 * @returns The number of samples remaining in the fifo
 */
uint8_t PIOS_ADXL345_Read(struct pios_adxl345_data * data)
{
	if(PIOS_ADXL345_Validate(dev) != 0)
		return -1;
	
	if(PIOS_ADXL345_ClaimBus() != 0)
		return -2;
	
	// To save memory use same buffer for in and out but offset by
	// a byte
	uint8_t buf[9] = {0,0,0,0,0,0,0,0};
	uint8_t rec[9] = {0,0,0,0,0,0,0,0};
	buf[0] = ADXL_X0_ADDR | ADXL_MULTI_BIT | ADXL_READ_BIT ; // Multibyte read starting at X0
	
	if(PIOS_SPI_TransferBlock(dev->spi_id,&buf[0],&rec[0],9,NULL) < 0) {
		PIOS_ADXL345_ReleaseBus();
		return -3;
	}

	PIOS_ADXL345_ReleaseBus();	
	
	data->x = rec[1] + (rec[2] << 8);
	data->y = rec[3] + (rec[4] << 8);
	data->z = rec[5] + (rec[6] << 8);
	
	return rec[8] & 0x7F; // return number of remaining entries
}

/**
 ******************************************************************************
 * @addtogroup PIOS PIOS Core hardware abstraction layer
 * @{
 * @addtogroup PIOS_BMA180 BMA180 Functions
 * @brief Deals with the hardware interface to the BMA180 3-axis accelerometer
 * @{
 *
 * @file       pios_bma180.h
 * @author     David "Buzz" Carlson (buzz@chebuzz.com)
 * 				The OpenPilot Team, http://www.openpilot.org Copyright (C) 2011.
 * @brief      PiOS BMA180 digital accelerometer driver.
 *                 - Driver for the BMA180 digital accelerometer on the SPI bus.
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

static uint32_t PIOS_SPI_ACCEL;

static int32_t PIOS_BMA180_GetReg(uint8_t reg);
static int32_t PIOS_BMA180_SetReg(uint8_t reg, uint8_t data);
static int32_t PIOS_BMA180_SelectBW(uint8_t bw);
static int32_t PIOS_BMA180_SetRange(uint8_t range);

/**
 * @brief Claim the SPI bus for the accel communications and select this chip
 * @return 0 if successful, -1 if unable to claim bus
 */
int32_t PIOS_BMA180_ClaimBus()
{
	if(PIOS_SPI_ClaimBus(PIOS_SPI_ACCEL) != 0)
		return -1;
	PIOS_BMA_ENABLE;
	return 0;
}

/**
 * @brief Release the SPI bus for the accel communications and end the transaction
 * @return 0 if successful
 */
int32_t PIOS_BMA180_ReleaseBus()
{
	PIOS_BMA_DISABLE;
	return PIOS_SPI_ReleaseBus(PIOS_SPI_ACCEL);
}

/**
 * @brief Read a register from BMA180
 * @returns The register value or -1 if failure to get bus
 * @param reg[in] Register address to be read
 */
int32_t PIOS_BMA180_GetReg(uint8_t reg)
{
	uint8_t data;
	
	if(PIOS_BMA180_ClaimBus() != 0)
		return -1;	

	PIOS_SPI_TransferByte(PIOS_SPI_ACCEL,(0x80 | reg) ); // request byte
	data = PIOS_SPI_TransferByte(PIOS_SPI_ACCEL,0 );     // receive response
	
	PIOS_BMA180_ReleaseBus();
	return data;
}

/**
 * @brief Write a BMA180 register.  EEPROM must be unlocked before calling this function.
 * @return none
 * @param reg[in] address of register to be written
 * @param data[in] data that is to be written to register
 */
int32_t PIOS_BMA180_SetReg(uint8_t reg, uint8_t data)
{
	if(PIOS_BMA180_ClaimBus() != 0)
		return -1;
	
	PIOS_SPI_TransferByte(PIOS_SPI_ACCEL, 0x7f & reg);
	PIOS_SPI_TransferByte(PIOS_SPI_ACCEL, data);

	PIOS_BMA180_ReleaseBus();
	
	return 0;
}

/**
 * @brief Select the bandwidth the digital filter pass allows.
 * @return 0 if successful, -1 if not
 * @param rate[in] Bandwidth setting to be used
 * 
 * EEPROM must be write-enabled before calling this function.
 */
static int32_t PIOS_BMA180_SelectBW(uint8_t bw)
{
	uint8_t reg;
	reg = PIOS_BMA180_GetReg(BMA_BW_ADDR);
	reg = (reg & ~BMA_BW_MASK) | ((bw << BMA_BW_SHIFT) & BMA_BW_MASK);
	return PIOS_BMA180_SetReg(BMA_BW_ADDR, reg);
}

/**
 * @brief Select the full scale acceleration range.
 * @return 0 if successful, -1 if not
 * @param rate[in] Range setting to be used
 *
 */
static int32_t PIOS_BMA180_SetRange(uint8_t range) 
{
	uint8_t reg;
	reg = PIOS_BMA180_GetReg(BMA_RANGE_ADDR);
	reg = (reg & ~BMA_RANGE_MASK) | ((range << BMA_RANGE_SHIFT) & BMA_RANGE_MASK);
	return PIOS_BMA180_SetReg(BMA_RANGE_ADDR, reg);
}

/**
 * @brief Connect to the correct SPI bus
 */
void PIOS_BMA180_Attach(uint32_t spi_id)
{
	PIOS_SPI_ACCEL = spi_id;
}

/**
 * @brief Initialize with good default settings
 */
void PIOS_BMA180_Init()
{
	if(0){
	PIOS_BMA180_SelectBW(BMA_BW_150HZ);
	PIOS_BMA180_SetRange(BMA_RANGE_8G);
	}
}

/**
 * @brief Read a single set of values from the x y z channels
 * @param[out] data Int16 array of (x,y,z) sensor values
 * @returns 0 if successful
 * @retval -1 unable to claim bus
 * @retval -2 unable to transfer data
 */
int32_t PIOS_BMA180_ReadAccels(int16_t * data)
{
	// To save memory use same buffer for in and out but offset by
	// a byte
	uint8_t buf[7] = {BMA_X_LSB_ADDR | 0x80,0,0,0,0,0};
	uint8_t rec[7] = {0,0,0,0,0,0};
	
	if(PIOS_BMA180_ClaimBus() != 0)
		return -1;
	if(PIOS_SPI_TransferBlock(PIOS_SPI_ACCEL,&buf[0],&rec[0],7,NULL) != 0)
		return -2;
	PIOS_BMA180_ReleaseBus();	
	
	//        |    MSB        |   LSB       | 0 | new_data |
	data[0] = (rec[2] << 8) | rec[1];
	data[1] = (rec[4] << 8) | rec[3];
	data[2] = (rec[6] << 8) | rec[5];
	data[0] /= 4;
	data[1] /= 4;
	data[2] /= 4;
	
	return 0; // return number of remaining entries
}

/**
 * @brief Returns the scale the BMA180 chip is using
 * @return Scale (m / s^2) / LSB
 */
float PIOS_BMA_GetScale()
{
	return 1;
}

/**
 * @brief Test SPI and chip functionality by reading chip ID register
 * @return 0 if success, -1 if failure.
 *
 */
int32_t PIOS_BMA180_Test()
{
	// Read chip ID then version ID
	uint8_t buf[3] = {0x80 | BMA_CHIPID_ADDR, 0, 0};
	uint8_t rec[3] = {0,0, 0};

	if(PIOS_BMA180_ClaimBus() != 0)
		return -1;
	if(PIOS_SPI_TransferBlock(PIOS_SPI_ACCEL,&buf[0],&rec[0],sizeof(buf),NULL) != 0)
		return -2;
	PIOS_BMA180_ReleaseBus();
	
	int16_t data[3];
	if(PIOS_BMA180_ReadAccels(data) != 0)
		return -3;
	
	if(rec[1] != 0x3)
		return -4;
	
	if(rec[2] < 0x12)
		return -5;

	return 0;
}

/**
 * @}
 * @}
 */

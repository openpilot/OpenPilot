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
static uint8_t EEPROM_WRITEABLE=0;


/**
 * @brief Claim the SPI bus for the accel communications and select this chip
 */
void PIOS_BMA180_ClaimBus()
{
	PIOS_SPI_ClaimBus(PIOS_SPI_ACCEL);
	PIOS_BMA_ENABLE;
}

/**
 * @brief Release the SPI bus for the accel communications and end the transaction
 */
void PIOS_BMA180_ReleaseBus()
{
	PIOS_BMA_DISABLE;
	PIOS_SPI_ReleaseBus(PIOS_SPI_ACCEL);
}

/**
 * @brief Set the EEPROM write-enable bit.  Must be set to 1 (unlocked) before writing control registers.
 * @return none
 * @param _we[in] bit to set, 1 to enable writes or 0 to disable writes
 */
void PIOS_BMA180_WriteEnable(uint8_t _we)
{
	uint8_t addr_reg[2] = {BMA_WE_ADDR,0};

	PIOS_BMA180_ClaimBus();
	addr_reg[1] = PIOS_SPI_TransferByte(PIOS_SPI_ACCEL,(0x80 | BMA_WE_ADDR) );
	addr_reg[1] &= 0xEF;
	addr_reg[1] |= ( (0x01 & _we) << 4);
	PIOS_SPI_TransferBlock(PIOS_SPI_ACCEL,addr_reg,NULL,sizeof(addr_reg),NULL);
	PIOS_BMA180_ReleaseBus();
	EEPROM_WRITEABLE=_we;
}

/**
 * @brief Read a register from BMA180
 * @returns The register value
 * @param reg[in] Register address to be read
 */
uint8_t PIOS_BMA180_GetReg(uint8_t reg)
{
	uint8_t data;
	PIOS_BMA180_ClaimBus();
	data = PIOS_SPI_TransferByte(PIOS_SPI_ACCEL,(0x80 | reg) );
	PIOS_BMA180_ReleaseBus();
	return(data);
}

/**
 * @brief Write a BMA180 register.  EEPROM must be unlocked before calling this function.
 * @return none
 * @param reg[in] address of register to be written
 * @param data[in] data that is to be written to register
 */
void PIOS_BMA180_SetReg(uint8_t reg, uint8_t data)
{
	uint8_t reg_data[2] = { (0x7F & reg), data};
	PIOS_BMA180_ClaimBus();
	PIOS_SPI_TransferBlock(PIOS_SPI_ACCEL,reg_data,NULL,2,NULL);
	PIOS_BMA180_ReleaseBus();
}

/**
 * @brief Select the bandwidth the digital filter pass allows.
 * @return none
 * @param rate[in] Bandwidth setting to be used
 * 
 * EEPROM must be write-enabled before calling this function.
 */
void PIOS_BMA180_SelectBW(uint8_t bw)
{
	uint8_t addr_reg[2] = { BMA_BW_ADDR, 0};

	PIOS_BMA180_ClaimBus();
	addr_reg[1] = PIOS_SPI_TransferByte(PIOS_SPI_ACCEL,(0x80|BMA_BW_ADDR));
	addr_reg[1] &= 0x0F;
	addr_reg[1] |= (bw << 4);
	PIOS_SPI_TransferBlock(PIOS_SPI_ACCEL,addr_reg,NULL,sizeof(addr_reg),NULL);
	PIOS_BMA180_ReleaseBus();	
}

/**
 * @brief Select the full scale acceleration range.
 * @return none
 * @param rate[in] Range setting to be used
 *
 */
void PIOS_BMA180_SetRange(uint8_t range) 
{
	uint8_t addr_reg[2] = { BMA_RANGE_ADDR, 0};

	PIOS_BMA180_ClaimBus();
	addr_reg[1] = PIOS_SPI_TransferByte(PIOS_SPI_ACCEL,(0x80|BMA_RANGE_ADDR));
	addr_reg[1] &= 0x0F;
	addr_reg[1] |= (range << 4);
	PIOS_SPI_TransferBlock(PIOS_SPI_ACCEL,addr_reg,NULL,sizeof(addr_reg),NULL);
	PIOS_BMA180_ReleaseBus();	
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
/*
	PIOS_BMA180_ReleaseBus();
	PIOS_BMA180_WriteEnable(1);
	PIOS_BMA180_SelectRate(BMA_RATE_3200);
	PIOS_BMA180_SetRange(BMA_RANGE_8G);
	PIOS_BMA180_FifoDepth(16);
	PIOS_BMA180_SetMeasure(1); 
	PIOS_BMA180_WriteEnable(0);
*/
}

/**
 * @brief Read a single set of values from the x y z channels
 * @returns The number of samples remaining in the fifo
 */
uint8_t PIOS_BMA180_Read(struct pios_bma180_data * data)
{
	// To save memory use same buffer for in and out but offset by
	// a byte
	uint8_t buf[7] = {0,0,0,0,0,0};
	uint8_t rec[7] = {0,0,0,0,0,0};
	buf[0] = BMA_X_LSB_ADDR | 0x80 ; // Multibyte read starting at X LSB
	
	PIOS_BMA180_ClaimBus();
	PIOS_SPI_TransferBlock(PIOS_SPI_ACCEL,&buf[0],&rec[0],7,NULL);
	PIOS_BMA180_ReleaseBus();	
	
	//        |    MSB        |   LSB       | 0 | new_data |
	data->x = ( (rec[2] << 8) | rec[1] ) >> 2;
	data->y = ( (rec[4] << 8) | rec[3] ) >> 2;
	data->z = ( (rec[6] << 8) | rec[5] ) >> 2;
	
	return 0; // return number of remaining entries
}

/**
 * @brief Test SPI and chip functionality by reading chip ID register
 * @return 0 if test failed, any other value signals test succeeded.
 *
 */
uint8_t PIOS_BMA180_Test()
{
	uint8_t data = 0;
	uint8_t pass = 0;
	PIOS_BMA180_ClaimBus();
	data = PIOS_SPI_TransferByte(PIOS_SPI_ACCEL,(0x80 | BMA_CHIPID_ADDR) );
	data &= 0x07;
	if(0x03 == data)
		pass = 1;
	data = PIOS_SPI_TransferByte(PIOS_SPI_ACCEL,(0x80 | BMA_VERSION_ADDR) );
	if(0x12 == data)
		pass = 1 && pass;			// Only passes if first and second test passS
	PIOS_BMA180_ReleaseBus();
	return pass;
}

/**
 * @}
 * @}
 */

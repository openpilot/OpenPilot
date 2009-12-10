/**
 ******************************************************************************
 *
 * @file       pios_bmp085.c  
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2009.   
 * @brief      IRQ Enable/Disable routines
 * @see        The GNU Public License (GPL) Version 3
 * @defgroup   PIOS_BMP085 Bosch BMP085 Barometric Pressure Sensor Functions
 * @{
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


/* Project Includes */
#include "pios.h"


/* Local Variables */
static BMP085CalibDataTypeDef CalibData;

/**
* Initialise the BMP085 sensor
*/
void PIOS_BMP085_Init(void)
{
	/* Read the calibration data on the BMP085 sensor */
	PIOS_BMP085_Read(BMP085_CALIB_ADDR, (uint8_t *) &CalibData.AC1, 2);
	PIOS_BMP085_Read(BMP085_CALIB_ADDR + 2, (uint8_t *) &CalibData.AC2, 2);
	PIOS_BMP085_Read(BMP085_CALIB_ADDR + 4, (uint8_t *) &CalibData.AC3, 2);
	PIOS_BMP085_Read(BMP085_CALIB_ADDR + 6, (uint8_t *) &CalibData.AC4, 2);
	PIOS_BMP085_Read(BMP085_CALIB_ADDR + 8, (uint8_t *) &CalibData.AC5, 2);
	PIOS_BMP085_Read(BMP085_CALIB_ADDR + 10, (uint8_t *) &CalibData.AC6, 2);
	PIOS_BMP085_Read(BMP085_CALIB_ADDR + 12, (uint8_t *) &CalibData.B1, 2);
	PIOS_BMP085_Read(BMP085_CALIB_ADDR + 14, (uint8_t *) &CalibData.B2, 2);
	PIOS_BMP085_Read(BMP085_CALIB_ADDR + 16, (uint8_t *) &CalibData.MB, 2);
	PIOS_BMP085_Read(BMP085_CALIB_ADDR + 18, (uint8_t *) &CalibData.MC, 2);
	PIOS_BMP085_Read(BMP085_CALIB_ADDR + 20, (uint8_t *) &CalibData.MD, 2);
}

/**
* Start the ADC conversion
* \param[in] PresOrTemp BMP085_PRES_ADDR or BMP085_TEMP_ADDR
* \return Raw ADC value
*/
void PIOS_BMP085_StartADC(uint16_t *PresOrTemp)
{
	/* Start the conversion */
	PIOS_BMP085_Write(BMP085_CTRL_ADDR, (uint8_t *)PresOrTemp, 2);
}

/**
* Reads one or more bytes into a buffer
* \param[in] address BMP085 register address (depends on size)
* \param[out] buffer destination buffer
* \param[in] len number of bytes which should be read (1..64)
* \return 0 if operation was successful
* \return -1 if error during I2C transfer
* \return -2 if BMP085 blocked by another task (retry it!)
* \return -4 if invalid length
*/
int32_t PIOS_BMP085_Read(uint16_t address, uint8_t *buffer, uint8_t len)
{
	/* Try to get the IIC peripheral */
	if(PIOS_I2C_TransferBegin(I2C_Non_Blocking) < 0) {
		/* Request a retry */
		return -2;
	}

	/* Send I2C address and EEPROM address */
	/* To avoid issues with litte/big endian: copy address into temporary buffer */
	uint8_t addr_buffer[2] = {(uint8_t)(address>>8), (uint8_t)address};
	int32_t error = PIOS_I2C_Transfer(I2C_Write_WithoutStop, BMP085_ADDR_WRITE, addr_buffer, 2);

	if(!error) {
		error = PIOS_I2C_TransferWait();
	}

	/* Now receive byte(s) */
	if(!error) {
		error = PIOS_I2C_Transfer(I2C_Read, BMP085_ADDR_READ, buffer, len);
	}
	if(!error) {
		error = PIOS_I2C_TransferWait();
	}

	/* Release I2C peripheral */
	PIOS_I2C_TransferFinished();
	
	/* Return error status */
	return error < 0 ? -1 : 0;
}


/**
* Writes one or more bytes to the BMP085
* \param[in] address BankStick address (depends on size)
* \param[in] buffer source buffer
* \param[in] len number of bytes which should be written (1..64)
* \return 0 if operation was successful
* \return -1 if error during IIC transfer
* \return -2 if BankStick blocked by another task (retry it!)
* \return -4 if invalid length
* \note Use \ref PIOS_I2C_BS_CheckWriteFinished to check when the write operation
* has been finished - this can take up to 5 mS!
*/
int32_t PIOS_BMP085_Write(uint16_t address, uint8_t *buffer, uint8_t len)
{
	/* Try to get the IIC peripheral */
	if(PIOS_I2C_TransferBegin(I2C_Non_Blocking) < 0) {
		/* Request a retry */
		return -2;
	}

	/* Send I2C address and EEPROM address */
	/* To avoid issues with litte/big endian: copy address into temporary buffer */
	uint8_t addr_buffer[2] = {(uint8_t)(address>>8), (uint8_t)address};
	int32_t error = PIOS_I2C_Transfer(I2C_Write_WithoutStop, BMP085_ADDR_WRITE, addr_buffer, 2);

	if(!error) {
		error = PIOS_I2C_TransferWait();
	}

	/* Release I2C peripheral */
	PIOS_I2C_TransferFinished();
	
	/* Return error status */
	return error < 0 ? -1 : 0;
}


/* Read the values from the sensor
* \param[out] Pressure Pointer to the pressure variable
* \param[out] Altitude Pointer to the altitude variable
* \param[out] Temperature Pointer to the temperature variable

void PIOS_BMP085_ReadADC(uint16_t *Pressure, uint16_t *Altitude, uint16_t *Temperature)*/
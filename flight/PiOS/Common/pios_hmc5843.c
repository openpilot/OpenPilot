/**
 ******************************************************************************
 *
 * @file       pios_hmc5843.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      HMC5843 Magnetic Sensor Functions
 * @see        The GNU Public License (GPL) Version 3
 * @defgroup   PIOS_HMC5843 HMC5843 Functions
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

#if defined(PIOS_INCLUDE_HMC5843)

/* Glocal Variables */


/* Local Variables */


/**
* Initialise the HMC5843 sensor
*/
void PIOS_HMC5843_Init(void)
{
	// Set in continuous mode
	PIOS_HMC5843_Write(0x02, 0x00);
}


/**
* Reads one or more bytes into a buffer
* \param[in] address HMC5843 register address (depends on size)
* \param[out] buffer destination buffer
* \param[in] len number of bytes which should be read
* \return 0 if operation was successful
* \return -1 if error during I2C transfer
* \return -4 if invalid length
*/
int32_t PIOS_HMC5843_Read(uint8_t address, uint8_t *buffer, uint8_t len)
{
	/* Send I2C address and register address */
	/* To avoid issues copy address into temporary buffer */
	uint8_t addr_buffer[1] = {(uint8_t)address};
	int32_t error = PIOS_I2C_Transfer(I2C_Write_WithoutStop, HMC5843_I2C_ADDR, addr_buffer, 1);

	/* Now receive byte(s) */
	if(!error) {
		error = PIOS_I2C_Transfer(I2C_Read, HMC5843_I2C_ADDR, buffer, len);
	}
	
	/* Return error status */
	return error < 0 ? -1 : 0;
}


/**
* Writes one or more bytes to the HMC5843
* \param[in] address Register address
* \param[in] buffer source buffer
* \return 0 if operation was successful
* \return -1 if error during I2C transfer
*/
int32_t PIOS_HMC5843_Write(uint8_t address, uint8_t buffer)
{
	/* Send I2C address and data */
	uint8_t WriteBuffer[2];
	WriteBuffer[0] = address;
	WriteBuffer[1] = buffer;
	
	int32_t error = PIOS_I2C_Transfer(I2C_Write, HMC5843_I2C_ADDR, WriteBuffer, 2);
	
	/* Return error status */
	return error < 0 ? -1 : 0;
}

#endif

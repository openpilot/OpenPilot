/**
******************************************************************************
* @addtogroup PIOS PIOS Core hardware abstraction layer
* @{
* @addtogroup PIOS_EEPROM EEPROM reading/writing functions
* @brief PIOS EEPROM reading/writing functions
* @{
*
* @file       pios_eeprom.c
* @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2012.
* @brief      COM layer functions
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

/* Project Includes */
#include <pios.h>
#include <pios_crc.h>
#include <stm32f10x_flash.h>
#include <pios_board_info.h>

#include <pios_eeprom.h>

static struct pios_eeprom_cfg config;

/**
 * Initialize the flash eeprom device
 * \param cfg     The configuration structure.
 */
void PIOS_EEPROM_Init(const struct pios_eeprom_cfg *cfg)
{
	config = *cfg;
}

/**
 * Save a block of data to the flash eeprom device.
 * \param data  A pointer to the data to write.
 * \param len   The length of data to write.
 * \return      0 on sucess
 */
int32_t PIOS_EEPROM_Save(uint8_t *data, uint32_t len)
{

	// We need to write 32 bit words, so extend the length to be an even multiple of 4 bytes,
	// and include 4 bytes for the 32 bit CRC.
	uint32_t nwords = (len / 4) + 1 + (len % 4 ? 1 : 0);
	uint32_t size = nwords * 4;

	// Ensure that the length is not longer than the max size.
	if (size > config.max_size)
		return -1;

	// Calculate a 32 bit CRC of the data.
	uint32_t crc = PIOS_CRC32_updateCRC(0xffffffff, data, len);

	// Unlock the Flash Program Erase controller
	FLASH_Unlock();

	// See if we have to write the data.
	if ((memcmp(data, (uint8_t*)config.base_address, len) == 0) &&
			(memcmp((uint8_t*)&crc, (uint8_t*)config.base_address + size - 4, 4) == 0))
		return 0;

	// TODO: Check that the area isn't already erased

	// Erase page
	FLASH_Status fs = FLASH_ErasePage(config.base_address);
	if (fs != FLASH_COMPLETE)
	{   // error
		FLASH_Lock();
		return -2;
	}	
	
	// write 4 bytes at a time into program flash area (emulated EEPROM area)
	uint8_t *p1 = data;
	uint32_t *p3 = (uint32_t *)config.base_address;
	for (int32_t i = 0; i < size; p3++)
	{
		uint32_t value = 0;

		if (i == (size - 4))
		{
			// write the CRC.
			value = crc;
			i += 4;
		}
		else
		{
			if (i < len) value |= (uint32_t)*p1++ << 0;  else value |= 0x000000ff; i++;
			if (i < len) value |= (uint32_t)*p1++ << 8;  else value |= 0x0000ff00; i++;
			if (i < len) value |= (uint32_t)*p1++ << 16; else value |= 0x00ff0000; i++;
			if (i < len) value |= (uint32_t)*p1++ << 24; else value |= 0xff000000; i++;
		}

		// write a 32-bit value
		fs = FLASH_ProgramWord((uint32_t)p3, value);
		if (fs != FLASH_COMPLETE)
		{
			FLASH_Lock();			
			return -3;
		}
	}
	
	// Lock the Flash Program Erase controller
	FLASH_Lock();

	return 0;
}

/**
 * Reads a block of data from the flash eeprom device.
 * \param data  A pointer to the output data buffer.
 * \param len   The length of data to read.
 * \return      0 on sucess
 */
int32_t PIOS_EEPROM_Load(uint8_t *data, uint32_t len)
{

	// We need to write 32 bit words, so the length should have been extended
	// to an even multiple of 4 bytes, and should include 4 bytes for the 32 bit CRC.
	uint32_t nwords = (len / 4) + 1 + (len % 4 ? 1 : 0);
	uint32_t size = nwords * 4;

	// Ensure that the length is not longer than the max size.
	if (size > config.max_size)
		return -1;

  // Read the data from flash.
	memcpy(data, (uint8_t*)config.base_address, len);

	// Read the CRC.
	uint32_t crc_flash = *((uint32_t*)(config.base_address + size - 4));

	// Calculate a 32 bit CRC of the data.
	uint32_t crc = PIOS_CRC32_updateCRC(0xffffffff, data, len);
	if(crc != crc_flash)
		return -2;

	return 0;
}

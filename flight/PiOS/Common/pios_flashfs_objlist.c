/**
 ******************************************************************************
 *
 * @addtogroup PIOS PIOS Core hardware abstraction layer
 * @{
 * @addtogroup PIOS_FLASHFS_OBJLIST Object list based flash filesystem (low ram)
 * @{
 *
 * @file       pios_flashfs_objlist.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      A file system for storing UAVObject in flash chip
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


#include "openpilot.h"
#include "uavobjectmanager.h"

// Private functions
static int32_t PIOS_FLASHFS_CleabObjectTableHeader();
static int32_t PIOS_FLASHFS_GetObjAddress(uint32_t objId, uint16_t instId);
static int32_t PIOS_FLASHFS_GetNewAddress(uint32_t objId, uint16_t instId);

// Private variables
static int32_t numObjects = -1;

// Private structures
// Header for objects in the file system table
struct objectHeader {
	uint32_t objMagic;
	uint32_t objId;
	uint32_t instId;
	uint32_t address;
} __attribute__((packed));;

struct fileHeader {
	uint32_t id;
	uint16_t instId;
	uint16_t size;
} __attribute__((packed));


#define OBJECT_TABLE_MAGIC 0x85FB3C34
#define OBJ_MAGIC          0x3015AE71
#define OBJECT_TABLE_START 0x00000010
#define OBJECT_TABLE_END   0x00001000
#define SECTOR_SIZE        0x00001000
#define MAX_BADMAGIC       4

/**
 * @brief Initialize the flash object setting FS
 * @return 0 if success, -1 if failure
 */
int32_t PIOS_FLASHFS_Init()
{

	// Check for valid object table or create one
	uint32_t object_table_magic;
	uint8_t magic_fail_count = 0;
	bool magic_good = 0;

	while(!magic_good) {
		if (PIOS_Flash_W25X_ReadData(0, (uint8_t *)&object_table_magic, sizeof(object_table_magic)) != 0)
			return -1;
		if(object_table_magic != OBJECT_TABLE_MAGIC) {
			if(magic_fail_count++ > MAX_BADMAGIC) {
				if(PIOS_FLASHFS_CleabObjectTableHeader() < 0)
					return -1;
				magic_good = true;
			} else {
				PIOS_DELAY_WaituS(100);
			}

		}
		else {
			magic_good = true;
		}

	}

	int32_t addr = OBJECT_TABLE_START;
	struct objectHeader header;
	numObjects = 0;

	// Loop through header area while objects detect to count how many saved
	while(addr < OBJECT_TABLE_END) {
		// Read the instance data
		if (PIOS_Flash_W25X_ReadData(addr, (uint8_t *)&header, sizeof(header)) != 0)
			return -1;

		// Counting number of valid headers
		if(header.objMagic != OBJ_MAGIC)
			break;

		numObjects++;
		addr += sizeof(header);
	}

	return 0;
}

/**
 * @brief Erase the headers for all objects in the flash chip
 * @return 0 if successful, -1 if not
 */
static int32_t PIOS_FLASHFS_CleabObjectTableHeader()
{
	if(PIOS_Flash_W25X_EraseSector(0) != 0)
		return -1;

	uint32_t object_table_magic = OBJECT_TABLE_MAGIC;
	if (PIOS_Flash_W25X_WriteData(0, (uint8_t *)&object_table_magic, sizeof(object_table_magic)) != 0)
		return -1;

	return 0;
}

/**
 * @brief Get the address of an object
 * @param obj UAVObjHandle for that object
 * @parma instId Instance id for that object
 * @return address if successful, -1 if not found
 */
static int32_t PIOS_FLASHFS_GetObjAddress(uint32_t objId, uint16_t instId)
{
	int32_t addr = OBJECT_TABLE_START;
	struct objectHeader header;

	// Loop through header area while objects detect to count how many saved
	while(addr < OBJECT_TABLE_END) {
		// Read the instance data
		if (PIOS_Flash_W25X_ReadData(addr, (uint8_t *) &header, sizeof(header)) != 0)
			return -1;
		if(header.objMagic != OBJ_MAGIC)
			break; // stop searching once hit first non-object header
		else if (header.objId == objId && header.instId == instId)
			break;
		addr += sizeof(header);
	}

	if (header.objId == objId && header.instId == instId)
		return header.address;

	return -1;
}

/**
 * @brief Returns an address for a new object and creates entry into object table
 * @param[in] obj Object handle for object to be saved
 * @param[in] instId The instance id of object to be saved
 * @return 0 if success or error code
 * @retval -1 Object not found
 * @retval -2 No room in object table
 * @retval -3 Unable to write entry into object table
 * @retval -4 FS not initialized
 */
int32_t PIOS_FLASHFS_GetNewAddress(uint32_t objId, uint16_t instId)
{
	struct objectHeader header;

	if(numObjects < 0)
		return -4;

	// Don't worry about max size of flash chip here, other code will catch that
	header.objMagic = OBJ_MAGIC;
	header.objId = objId;
	header.instId = instId;
	header.address = OBJECT_TABLE_END + SECTOR_SIZE * numObjects;

	int32_t addr = OBJECT_TABLE_START + sizeof(header) * numObjects;

	// No room for this header in object table
	if((addr + sizeof(header)) > OBJECT_TABLE_END)
		return -2;

	if(PIOS_Flash_W25X_WriteData(addr, (uint8_t *) &header, sizeof(header)) != 0)
		return -3;

	// This numObejcts value must stay consistent or there will be a break in the table
	// and later the table will have bad values in it
	numObjects++;
	return header.address;
}


/**
 * @brief Saves one object instance per sector
 * @param[in] obj UAVObjHandle the object to save
 * @param[in] instId The instance of the object to save
 * @return 0 if success or -1 if failure
 * @note This uses one sector on the flash chip per object so that no buffering in ram
 * must be done when erasing the sector before a save
 */
int32_t PIOS_FLASHFS_ObjSave(UAVObjHandle obj, uint16_t instId, uint8_t * data)
{
	uint32_t objId = UAVObjGetID(obj);
	uint8_t crc = 0;

	int32_t addr = PIOS_FLASHFS_GetObjAddress(objId, instId);

	// Object currently not saved
	if(addr < 0)
		addr = PIOS_FLASHFS_GetNewAddress(objId, instId);

	// Could not allocate a sector
	if(addr < 0)
		return -1;

	struct fileHeader header = {
		.id = objId,
		.instId = instId,
		.size = UAVObjGetNumBytes(obj)
	};

	if(PIOS_Flash_W25X_EraseSector(addr) != 0)
		return -2;

	// Save header
	// This information IS redundant with the object table id.  Oh well.  Better safe than sorry.
	if(PIOS_Flash_W25X_WriteData(addr, (uint8_t *) &header, sizeof(header)) != 0)
		return -3;

	// Update CRC
	crc = PIOS_CRC_updateCRC(0, (uint8_t *) &header, sizeof(header));

	// Save data
	if(PIOS_Flash_W25X_WriteData(addr + sizeof(header), data, UAVObjGetNumBytes(obj)) != 0)
		return -4;

	// Update CRC
	crc = PIOS_CRC_updateCRC(crc, (uint8_t *) data, UAVObjGetNumBytes(obj));

	// Save CRC (written so will work when CRC changes to uint16)
	if(PIOS_Flash_W25X_WriteData(addr + sizeof(header) + UAVObjGetNumBytes(obj), (uint8_t *) &crc, sizeof(crc)) != 0)
		return -4;

	return 0;
}

/**
 * @brief Load one object instance per sector
 * @param[in] obj UAVObjHandle the object to save
 * @param[in] instId The instance of the object to save
 * @return 0 if success or error code
 * @retval -1 if object not in file table
 * @retval -2 if unable to retrieve object header
 * @retval -3 if loaded data instId or objId don't match
 * @retval -4 if unable to retrieve instance data
 * @retval -5 if unable to read CRC
 * @retval -6 if CRC doesn't match
 * @note This uses one sector on the flash chip per object so that no buffering in ram
 * must be done when erasing the sector before a save
 */
int32_t PIOS_FLASHFS_ObjLoad(UAVObjHandle obj, uint16_t instId, uint8_t * data)
{
	uint32_t objId = UAVObjGetID(obj);
	uint16_t objSize = UAVObjGetNumBytes(obj);
	uint8_t crc = 0;
	uint8_t crcFlash = 0;
	const uint8_t crc_read_step = 8;
	uint8_t crc_read_buffer[crc_read_step];

	int32_t addr = PIOS_FLASHFS_GetObjAddress(objId, instId);

	// Object currently not saved
	if(addr < 0)
		return -1;

	struct fileHeader header;

	// Load header
	// This information IS redundant with the object table id.  Oh well.  Better safe than sorry.
	if(PIOS_Flash_W25X_ReadData(addr, (uint8_t *) &header, sizeof(header)) != 0)
		return -2;

	// Update CRC
	crc = PIOS_CRC_updateCRC(0, (uint8_t *) &header, sizeof(header));

	if((header.id != objId) || (header.instId != instId))
		return -3;

	// To avoid having to allocate the RAM for a copy of the object, we read by chunks
	// and compute the CRC
	for(uint32_t i = 0; i < objSize; i += crc_read_step) {
		PIOS_Flash_W25X_ReadData(addr + sizeof(header) + i, crc_read_buffer, crc_read_step);
		uint8_t valid_bytes = ((i + crc_read_step) >= objSize) ? objSize - i : crc_read_step;
		crc = PIOS_CRC_updateCRC(crc, crc_read_buffer, valid_bytes);
	}

	// Read CRC (written so will work when CRC changes to uint16)
	if(PIOS_Flash_W25X_ReadData(addr + sizeof(header) + objSize, (uint8_t *) &crcFlash, sizeof(crcFlash)) != 0)
		return -5;

	if(crc != crcFlash)
		return -6;

	// Read the instance data
	if (PIOS_Flash_W25X_ReadData(addr + sizeof(header), data, objSize) != 0)
		return -4;

	return 0;
}

/**
 * @brief Delete object from flash
 * @param[in] obj UAVObjHandle the object to save
 * @param[in] instId The instance of the object to save
 * @return 0 if success or error code
 * @retval -1 if object not in file table
 * @retval -2 Erase failed
 * @note To avoid buffering the file table (1k ram!) the entry in the file table
 * remains but destination sector is erased.  This will make the load fail as the
 * file header won't match the object.  At next save it goes back there.
 */
int32_t PIOS_FLASHFS_ObjDelete(UAVObjHandle obj, uint16_t instId)
{
	uint32_t objId = UAVObjGetID(obj);

	int32_t addr = PIOS_FLASHFS_GetObjAddress(objId, instId);

	// Object currently not saved
	if(addr < 0)
		return -1;

	if(PIOS_Flash_W25X_EraseSector(addr) != 0)
		return -2;

	return 0;
}

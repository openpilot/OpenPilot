/**
 ******************************************************************************
 * @addtogroup PIOS PIOS Core hardware abstraction layer
 * @{
 * @addtogroup PiosFlashInternal PIOS Flash internal flash driver
 * @{
 *
 * @file       pios_flash_internal.c  
 * @author     Tau Labs, http://github.com/TauLabs, Copyright (C) 2012-2013.
 * @brief Provides a flash driver for the STM32 internal flash sectors
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

#if defined(PIOS_INCLUDE_FLASH_INTERNAL)

#include "stm32f30x_flash.h"
#include "pios_flash_internal_priv.h"
#include "pios_flash.h"
#include "pios_wdg.h"
#include <stdbool.h>

#define STM32F30X_FLASH_SECTOR_SIZE 2048

static bool PIOS_Flash_Internal_GetSectorInfo(uint32_t address, uint32_t *sector_start, uint32_t *sector_size)
{
	if (address < FLASH_BASE ||
		address >= FLASH_BASE + PIOS_SYS_getCPUFlashSize())
		return false;

	*sector_start = address & (~(STM32F30X_FLASH_SECTOR_SIZE - 1));
	*sector_size = STM32F30X_FLASH_SECTOR_SIZE;

	return true;
}

enum pios_internal_flash_dev_magic {
	PIOS_INTERNAL_FLASH_DEV_MAGIC = 0xae82c65d,
};

struct pios_internal_flash_dev {
	enum pios_internal_flash_dev_magic magic;

#if defined(PIOS_INCLUDE_FREERTOS)
	xSemaphoreHandle transaction_lock;
#endif	/* defined(PIOS_INCLUDE_FREERTOS) */
};

static bool PIOS_Flash_Internal_Validate(struct pios_internal_flash_dev * flash_dev) {
	return (flash_dev && (flash_dev->magic == PIOS_INTERNAL_FLASH_DEV_MAGIC));
}

#if defined(PIOS_INCLUDE_FREERTOS)
static struct pios_internal_flash_dev * PIOS_Flash_Internal_alloc(void)
{
	struct pios_internal_flash_dev * flash_dev;

	flash_dev = (struct pios_internal_flash_dev *)pvPortMalloc(sizeof(* flash_dev));
	if (!flash_dev) return (NULL);

	flash_dev->magic = PIOS_INTERNAL_FLASH_DEV_MAGIC;

	return(flash_dev);
}
#else
static struct pios_internal_flash_dev pios_internal_flash_devs[PIOS_INTERNAL_FLASH_MAX_DEVS];
static uint8_t pios_internal_flash_num_devs;
static struct pios_internal_flash_dev * PIOS_Flash_Internal_alloc(void)
{
	struct pios_internal_flash_dev * flash_dev;

	if (pios_internal_flash_num_devs >= PIOS_INTERNAL_FLASH_MAX_DEVS) {
		return (NULL);
	}

	flash_dev = &pios_internal_flash_devs[pios_internal_flash_num_devs++];
	flash_dev->magic = PIOS_INTERNAL_FLASH_DEV_MAGIC;

	return (flash_dev);
}

#endif /* defined(PIOS_INCLUDE_FREERTOS) */

int32_t PIOS_Flash_Internal_Init(uintptr_t * flash_id, __attribute__((unused)) const struct pios_flash_internal_cfg * cfg)
{
	struct pios_internal_flash_dev * flash_dev;

	flash_dev = PIOS_Flash_Internal_alloc();
	if (flash_dev == NULL)
		return -1;

#if defined(PIOS_INCLUDE_FREERTOS)
	flash_dev->transaction_lock = xSemaphoreCreateMutex();
#endif	/* defined(PIOS_INCLUDE_FREERTOS) */

	*flash_id = (uintptr_t) flash_dev;

	return 0;
}

/**********************************
 *
 * Provide a PIOS flash driver API
 *
 *********************************/
#include "pios_flash.h"

static int32_t PIOS_Flash_Internal_StartTransaction(uintptr_t flash_id)
{
	struct pios_internal_flash_dev * flash_dev = (struct pios_internal_flash_dev *)flash_id;

	if (!PIOS_Flash_Internal_Validate(flash_dev))
		return -1;

#if defined(PIOS_INCLUDE_FREERTOS)
	if (xSemaphoreTake(flash_dev->transaction_lock, portMAX_DELAY) != pdTRUE)
		return -2;
#endif	/* defined(PIOS_INCLUDE_FREERTOS) */

	/* Unlock the internal flash so we can write to it */
	FLASH_Unlock();
	return 0;
}

static int32_t PIOS_Flash_Internal_EndTransaction(uintptr_t flash_id)
{
	struct pios_internal_flash_dev * flash_dev = (struct pios_internal_flash_dev *)flash_id;

	if (!PIOS_Flash_Internal_Validate(flash_dev))
		return -1;

#if defined(PIOS_INCLUDE_FREERTOS)
	if (xSemaphoreGive(flash_dev->transaction_lock) != pdTRUE)
		return -2;
#endif	/* defined(PIOS_INCLUDE_FREERTOS) */

	/* Lock the internal flash again so we can no longer write to it */
	FLASH_Lock();

	return 0;
}

static int32_t PIOS_Flash_Internal_EraseSector(uintptr_t flash_id, uint32_t addr)
{
	struct pios_internal_flash_dev * flash_dev = (struct pios_internal_flash_dev *)flash_id;

	if (!PIOS_Flash_Internal_Validate(flash_dev))
		return -1;

#if defined(PIOS_INCLUDE_WDG)
	PIOS_WDG_Clear();
#endif

	uint32_t sector_start;
	uint32_t sector_size;
	if (!PIOS_Flash_Internal_GetSectorInfo(addr,
						&sector_start,
						&sector_size)) {
		/* We're asking for an invalid flash address */
		return -2;
	}

	FLASH_Status ret = FLASH_ErasePage(sector_start);
	if (ret != FLASH_COMPLETE)
		return -3;

	return 0;
}

static int32_t PIOS_Flash_Internal_ReadData(uintptr_t flash_id, uint32_t addr, uint8_t * data, uint16_t len)
{
	PIOS_Assert(data);

	struct pios_internal_flash_dev * flash_dev = (struct pios_internal_flash_dev *)flash_id;

	if (!PIOS_Flash_Internal_Validate(flash_dev))
		return -1;

	uint32_t sector_start;
	uint32_t sector_size;

	/* Ensure that the base address is in a valid sector */
	if (!PIOS_Flash_Internal_GetSectorInfo(addr,
						&sector_start,
						&sector_size)) {
		/* We're asking for an invalid flash address */
		return -2;
	}

	/* Ensure that the entire read occurs within the same sector */
	if ((uintptr_t)addr + len > sector_start + sector_size) {
		/* Read crosses the end of the sector */
		return -3;
	}

	/* Read the data into the buffer directly */
	memcpy(data, (void *)addr, len);

	return 0;
}

static int32_t PIOS_Flash_Internal_WriteData(uintptr_t flash_id, uint32_t addr, uint8_t * data, uint16_t len)
{
	PIOS_Assert(data);
	PIOS_Assert((addr & 0x0001) == 0)

	struct pios_internal_flash_dev * flash_dev = (struct pios_internal_flash_dev *)flash_id;

	if (!PIOS_Flash_Internal_Validate(flash_dev))
		return -1;

	uint32_t sector_start;
	uint32_t sector_size;

	/* Ensure that the base address is in a valid sector */
	if (!PIOS_Flash_Internal_GetSectorInfo(addr,
						&sector_start,
						&sector_size)) {
		/* We're asking for an invalid flash address */
		return -2;
	}

	/* Ensure that the entire write occurs within the same sector */
	if ((uintptr_t)addr + len > sector_start + sector_size) {
		/* Write crosses the end of the sector */
		return -3;
	}

	/* Write the data */
	for (uint16_t i = 0; i < (len & ~1); i += 2) {
		/* Check if content has been changed prios to write.
		 * This should enhance performance and make this compatible with the F3 chip.
		 */
		uint8_t temp[2];
		if (PIOS_Flash_Internal_ReadData(flash_id, addr + i, temp, sizeof(temp)) != 0)
			return -4;

		if (temp[0] == data[i] && temp[1] == data[i + 1])
			continue;

		FLASH_Status status;
		status = FLASH_ProgramHalfWord(addr + i, data[i + 1] << 8 | data[i]);
		PIOS_Assert(status == FLASH_COMPLETE);
	}
	/* Handle uneven writes by filling up with 0xff*/
	if ((len & 1) != 0) {
		FLASH_Status status;
		status = FLASH_ProgramHalfWord(addr + len - 1, 0xff << 8 | data[len - 1]);
		PIOS_Assert(status == FLASH_COMPLETE);
	}

	return 0;
}

/* Provide a flash driver to external drivers */
const struct pios_flash_driver pios_internal_flash_driver = {
	.start_transaction = PIOS_Flash_Internal_StartTransaction,
	.end_transaction   = PIOS_Flash_Internal_EndTransaction,
	.erase_sector      = PIOS_Flash_Internal_EraseSector,
	.write_data        = PIOS_Flash_Internal_WriteData,
	.read_data         = PIOS_Flash_Internal_ReadData,
};

#endif	/* defined(PIOS_INCLUDE_FLASH_INTERNAL) */

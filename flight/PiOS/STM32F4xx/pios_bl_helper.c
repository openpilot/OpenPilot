/**
 ******************************************************************************
 * @addtogroup PIOS PIOS Core hardware abstraction layer
 * @{
 * @addtogroup PIOS_BOOTLOADER Functions
 * @brief HAL code to interface to the OpenPilot AHRS module
 * @{
 *
 * @file       pios_bl_helper.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      Bootloader Helper Functions
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
#include "pios.h"
#if defined(PIOS_INCLUDE_BL_HELPER)
#include <pios_board_info.h>
#include "stm32f4xx_flash.h"
#include <stdbool.h>

uint8_t *PIOS_BL_HELPER_FLASH_If_Read(uint32_t SectorAddress)
{
	return (uint8_t *) (SectorAddress);
}

#if defined(PIOS_INCLUDE_BL_HELPER_WRITE_SUPPORT)
uint8_t PIOS_BL_HELPER_FLASH_Ini()
{
	FLASH_Unlock();
	FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR | FLASH_FLAG_PGSERR);
	return 1;
}

struct device_flash_sector {
	uint32_t start;
	uint32_t size;
	uint16_t st_sector;
};

static struct device_flash_sector flash_sectors[] = {
	[0] = {
		.start = 0x08000000,
		.size  = 16 * 1024,
		.st_sector = FLASH_Sector_0,
	},
	[1] = {
		.start = 0x08004000,
		.size  = 16 * 1024,
		.st_sector = FLASH_Sector_1,
	},
	[2] = {
		.start = 0x08008000,
		.size  = 16 * 1024,
		.st_sector = FLASH_Sector_2,
	},
	[3] = {
		.start = 0x0800C000,
		.size  = 16 * 1024,
		.st_sector = FLASH_Sector_3,
	},
	[4] = {
		.start = 0x08010000,
		.size  = 64 * 1024,
		.st_sector = FLASH_Sector_4,
	},
	[5] = {
		.start = 0x08020000,
		.size  = 128 * 1024,
		.st_sector = FLASH_Sector_5,
	},
	[6] = {
		.start = 0x08040000,
		.size  = 128 * 1024,
		.st_sector = FLASH_Sector_6,
	},
	[7] = {
		.start = 0x08060000,
		.size  = 128 * 1024,
		.st_sector = FLASH_Sector_7,
	},
	[8] = {
		.start = 0x08080000,
		.size  = 128 * 1024,
		.st_sector = FLASH_Sector_8,
	},
	[9] = {
		.start = 0x080A0000,
		.size  = 128 * 1024,
		.st_sector = FLASH_Sector_9,
	},
	[10] = {
		.start = 0x080C0000,
		.size  = 128 * 1024,
		.st_sector = FLASH_Sector_10,
	},
	[11] = {
		.start = 0x080E0000,
		.size  = 128 * 1024,
		.st_sector = FLASH_Sector_11,
	},
};

static bool PIOS_BL_HELPER_FLASH_GetSectorInfo(uint32_t address, uint8_t * sector_number, uint32_t *sector_start, uint32_t *sector_size)
{
	for (uint8_t i = 0; i < NELEMENTS(flash_sectors); i++) {
		struct device_flash_sector * sector = &flash_sectors[i];
		if ((address >= sector->start) && 
			(address < (sector->start + sector->size))) {
			/* address lies within this sector */
			*sector_number = sector->st_sector;
			*sector_start  = sector->start;
			*sector_size   = sector->size;
			return (true);
		}
	}

	return (false);
}

uint8_t PIOS_BL_HELPER_FLASH_Start()
{
	const struct pios_board_info * bdinfo = &pios_board_info_blob;
	uint32_t pageAddress = bdinfo->fw_base;
	bool fail = false;
	while ((pageAddress < (bdinfo->fw_base + bdinfo->fw_size + bdinfo->desc_size))
	       && (fail == false)) {
		uint8_t sector_number;
		uint32_t sector_start;
		uint32_t sector_size;
		if (!PIOS_BL_HELPER_FLASH_GetSectorInfo(pageAddress, 
								&sector_number,
								&sector_start,
								&sector_size)) {
			/* We're asking for an invalid flash address */
			PIOS_Assert(0);
		}
		for (int retry = 0; retry < MAX_DEL_RETRYS; ++retry) {
			if (FLASH_EraseSector(sector_number, VoltageRange_3) == FLASH_COMPLETE) {
				fail = false;
				break;
			} else {
				fail = true;
			}

		}
		/* Move to the next sector */
		pageAddress += sector_size;
	}

	return (fail == true) ? 0 : 1;
}
#endif

uint32_t PIOS_BL_HELPER_CRC_Memory_Calc()
{
	const struct pios_board_info * bdinfo = &pios_board_info_blob;

	PIOS_BL_HELPER_CRC_Ini();
	CRC_ResetDR();
	CRC_CalcBlockCRC((uint32_t *) bdinfo->fw_base, (bdinfo->fw_size) >> 2);
	return CRC_GetCRC();
}

void PIOS_BL_HELPER_FLASH_Read_Description(uint8_t * array, uint8_t size)
{
	const struct pios_board_info * bdinfo = &pios_board_info_blob;
	uint8_t x = 0;
	if (size > bdinfo->desc_size) size = bdinfo->desc_size;
	for (uint32_t i = bdinfo->fw_base + bdinfo->fw_size; i < bdinfo->fw_base + bdinfo->fw_size + size; ++i) {
		array[x] = *PIOS_BL_HELPER_FLASH_If_Read(i);
		++x;
	}
}

void PIOS_BL_HELPER_CRC_Ini()
{
}
#endif

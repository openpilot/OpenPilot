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
#include "stm32f10x_flash.h"

uint8_t *FLASH_If_Read(uint32_t SectorAddress)
{
	return (uint8_t *) (SectorAddress);
}

#if defined(PIOS_INCLUDE_BL_HELPER_WRITE_SUPPORT)
uint8_t FLASH_Ini()
{
	FLASH_Unlock();
	return 1;
}

uint8_t FLASH_Start()
{
	uint32_t pageAdress;
	pageAdress = START_OF_USER_CODE;
	uint8_t fail = FALSE;
	while ((pageAdress < START_OF_USER_CODE + SIZE_OF_CODE + SIZE_OF_DESCRIPTION)
	       || (fail == TRUE)) {
		for (int retry = 0; retry < MAX_DEL_RETRYS; ++retry) {
			if (FLASH_ErasePage(pageAdress) == FLASH_COMPLETE) {
				fail = FALSE;
				break;
			} else {
				fail = TRUE;
			}

		}

#ifdef STM32F10X_HD
		pageAdress += 2048;
#elif defined (STM32F10X_MD)
		pageAdress += 1024;
#endif
	}

	return (fail == TRUE) ? 0 : 1;
}
#endif

uint32_t FLASH_crc_memory_calc()
{
	CRC_ResetDR();
	CRC_CalcBlockCRC((uint32_t *) START_OF_USER_CODE, (SIZE_OF_CODE) >> 2);
	return CRC_GetCRC();
}

void FLASH_read_description(uint8_t * array, uint8_t size)
{
	uint8_t x = 0;
	if (size>SIZE_OF_DESCRIPTION) size = SIZE_OF_DESCRIPTION;
	for (uint32_t i = START_OF_USER_CODE + SIZE_OF_CODE; i < START_OF_USER_CODE + SIZE_OF_CODE + size; ++i) {
		array[x] = *FLASH_If_Read(i);
		++x;
	}
}

void CRC_Ini()
{
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_CRC, ENABLE);
}
#endif

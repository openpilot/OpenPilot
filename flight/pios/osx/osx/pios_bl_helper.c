/**
 ******************************************************************************
 * @addtogroup PIOS PIOS Core hardware abstraction layer
 * @{
 * @addtogroup PIOS_BOOTLOADER Functions
 * @brief HAL code to interface to the OpenPilot AHRS module
 * @{
 *
 * @file       pios_bl_helper.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2012.
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

uint32_t PIOS_BL_HELPER_CRC_Memory_Calc()
{
	return 0;
}

extern const struct fw_version_info fw_version_blob;
void PIOS_BL_HELPER_FLASH_Read_Description(uint8_t * array, uint8_t size)
{
	uint8_t * desc = (uint8_t *) &fw_version_blob;
	for (uint32_t i = 0;  i < size; i++) {
		array[i] = desc[i];
	}
}

void PIOS_BL_HELPER_CRC_Ini()
{
}
#endif

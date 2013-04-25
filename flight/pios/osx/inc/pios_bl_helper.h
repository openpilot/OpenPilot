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

#ifndef PIOS_BL_HELPER_H_
#define PIOS_BL_HELPER_H_

extern uint8_t *PIOS_BL_HELPER_FLASH_If_Read(uint32_t SectorAddress);

extern uint8_t PIOS_BL_HELPER_FLASH_Ini();

extern uint32_t PIOS_BL_HELPER_CRC_Memory_Calc();

extern void PIOS_BL_HELPER_FLASH_Read_Description(uint8_t * array, uint8_t size);

extern uint8_t PIOS_BL_HELPER_FLASH_Start();

extern void PIOS_BL_HELPER_CRC_Ini();

#endif /* PIOS_BL_HELPER_H_ */

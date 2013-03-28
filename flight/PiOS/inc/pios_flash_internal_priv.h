/**
 ******************************************************************************
 * @file       pios_flash_internal_priv.h
 * @author     PhoenixPilot, http://github.com/PhoenixPilot, Copyright (C) 2012
 * @addtogroup 
 * @{
 * @addtogroup 
 * @{
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

#ifndef PIOS_FLASH_INTERNAL_H
#define PIOS_FLASH_INTERNAL_H

#include "pios_flash.h"		/* API definition for flash drivers */

extern const struct pios_flash_driver pios_internal_flash_driver;

struct pios_flash_internal_cfg {
	;
};

extern int32_t PIOS_Flash_Internal_Init(uintptr_t * flash_id, const struct pios_flash_internal_cfg * cfg);

#endif	/* PIOS_FLASH_INTERNAL_H */

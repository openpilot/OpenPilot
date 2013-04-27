/**
 ******************************************************************************
 * @file       pios_flash.h
 * @author     PhoenixPilot, http://github.com/PhoenixPilot, Copyright (C) 2012
 * @addtogroup PIOS PIOS Core hardware abstraction layer
 * @{
 * @addtogroup PIOS_FLASH Flash Driver API Definition
 * @{
 * @brief Flash Driver API Definition
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

#ifndef PIOS_FLASH_H
#define PIOS_FLASH_H

#include <stdint.h>

struct pios_flash_chunk {
	uint8_t * addr;
	uint32_t len;
};

struct pios_flash_driver {
	int32_t (*start_transaction)(uintptr_t flash_id);
	int32_t (*end_transaction)(uintptr_t flash_id);
	int32_t (*erase_chip)(uintptr_t flash_id);
	int32_t (*erase_sector)(uintptr_t flash_id, uint32_t addr);
	int32_t (*write_data)(uintptr_t flash_id, uint32_t addr, uint8_t * data, uint16_t len);
	int32_t (*write_chunks)(uintptr_t flash_id, uint32_t addr, struct pios_flash_chunk chunks[], uint32_t num_chunks);
	int32_t (*read_data)(uintptr_t flash_id, uint32_t addr, uint8_t * data, uint16_t len);
};

#endif	/* PIOS_FLASH_H */

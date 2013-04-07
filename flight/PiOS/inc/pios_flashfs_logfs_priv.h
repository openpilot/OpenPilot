/**
 ******************************************************************************
 * @file       pios_flashfs_logfs_priv.h
 * @author     PhoenixPilot, http://github.com/PhoenixPilot, Copyright (C) 2012
 * @addtogroup PIOS PIOS Core hardware abstraction layer
 * @{
 * @addtogroup PIOS_FLASHFS Flash Filesystem Function
 * @{
 * @brief Log Structured Filesystem for internal or external NOR Flash
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

#ifndef PIOS_FLASHFS_LOGFS_PRIV_H
#define PIOS_FLASHFS_LOGFS_PRIV_H

#include <stdint.h>
#include "pios_flash.h"		/* struct pios_flash_driver */

struct flashfs_logfs_cfg {
	uint32_t fs_magic;
	uint32_t total_fs_size;	/* Total size of all generations of the filesystem */
	uint32_t arena_size;	/* Max size of one generation of the filesystem */
	uint32_t slot_size;	/* Max size of a "file" */

	uint32_t start_offset;	/* Offset into flash where this filesystem starts */
	uint32_t sector_size;	/* Size of a flash erase block */
	uint32_t page_size;	/* Maximum flash burst write size */
};

int32_t PIOS_FLASHFS_Logfs_Init(uintptr_t * fs_id, const struct flashfs_logfs_cfg * cfg, const struct pios_flash_driver * driver, uintptr_t flash_id);

#endif	/* PIOS_FLASHFS_LOGFS_PRIV_H */

/**
 ******************************************************************************
 * @file       pios_flashfs.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2015.
 * @addtogroup PIOS PIOS Core hardware abstraction layer
 * @see        The GNU Public License (GPL) Version 3
 * @{
 * @addtogroup PIOS_FLASHFS Flash Filesystem API Definition
 * @{
 * @brief PIOS API for internal or onboard filesystem.
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

#ifndef PIOS_FLASHFS_H
#define PIOS_FLASHFS_H

#include <stdint.h>
#include <spiffs.h>


struct flashfs_cfg {
    uint32_t flashfs_magic;
    uint32_t physical_size; /* Max size used file system partition. */
    uint32_t physical_addr; /* Start address of the file system partition. */
    uint32_t physical_erase_block; /* Flash device specific: size affected during erase process*/
    uint32_t logical_block_size;
    uint32_t logical_page_size;
    uint32_t work_buffer_size;
    uint32_t cache_buffer_size;
};

/* API */
int32_t PIOS_FLASHFS_Init(uintptr_t *fs_id, const struct flashfs_cfg *cfg, const struct pios_flash_driver *driver, uintptr_t flash_id);

#endif /* PIOS_FLASHFS_H */

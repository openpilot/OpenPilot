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

/* Stats */
struct PIOS_FLASHFS_Stats {
    uint16_t block_free;
    uint16_t block_used;
    uint16_t cache_hits;
    uint16_t cache_misses;
    uint16_t gc;
    uint16_t saved;
};

/* lseek flags */
enum pios_flashfs_lseek_flags {
    FLASHFS_SEEK_SET,
    FLASHFS_SEEK_CUR,
    FLASHFS_SEEK_END
};

/* any write will be appended to the end of the file */
#define PIOS_FLASHFS_APPEND SPIFFS_APPEND
/* if the file exists, it will be truncated */
#define PIOS_FLASHFS_TRUNC SPIFFS_TRUNC
/* if file does not exists, it will be created */
#define PIOS_FLASHFS_CREAT SPIFFS_CREAT
/* open as read only */
#define PIOS_FLASHFS_RDONLY SPIFFS_RDONLY
/* open as write only */
#define PIOS_FLASHFS_WRONLY SPIFFS_WRONLY
/* open as read and write*/
#define PIOS_FLASHFS_RDWR SPIFFS_RDWR
/* Write not cached (directly to spi flash) */
#define PIOS_FLASHFS_WRTHROUGH SPIFFS_DIRECT
/* Remove matching file */
#define PIOS_FLASHFS_REMOVE 1

/* Errors */
#define PIOS_FLASHFS_OK 0
#define PIOS_FLASHFS_ERROR_FS_INVALID -1
#define PIOS_FLASHFS_ERROR_OPEN_FILE -2
#define PIOS_FLASHFS_ERROR_WRITE_FILE -3
#define PIOS_FLASHFS_ERROR_READ_FILE -4
#define PIOS_FLASHFS_ERROR_FS_MOUNT -5
#define PIOS_FLASHFS_ERROR_FS_ALLOC -6
#define PIOS_FLASHFS_ERROR_REMOVE_FILE -7
#define PIOS_FLASHFS_ERROR_PREFIX_SIZE -8
#define PIOS_FLASHFS_ERROR_EOF -9

/* File name */
#define FLASHFS_FILENAME_LEN 26

/* API */
int32_t PIOS_FLASHFS_Format(uintptr_t fs_id);
int32_t PIOS_FLASHFS_Close(uintptr_t fs_id, int16_t file_id);
int16_t PIOS_FLASHFS_Open(uintptr_t fs_id, const char *path, uint16_t flags);
int32_t PIOS_FLASHFS_Write(uintptr_t fs_id, int16_t fh, uint8_t *data, uint16_t size);
int32_t PIOS_FLASHFS_Read(uintptr_t fs_id, int16_t fh, uint8_t *data, uint16_t size);
int32_t PIOS_FLASHFS_Remove(uintptr_t fs_id, const char *path);
int32_t PIOS_FLASHFS_GetStats(uintptr_t fs_id, struct PIOS_FLASHFS_Stats *stats);
int32_t PIOS_FLASHFS_Find(uintptr_t fs_id, const char *path, uint16_t prefix_size, uint32_t flags);
int32_t PIOS_FLASHFS_Lseek(uintptr_t fs_id, int16_t fh, int32_t offset, enum pios_flashfs_lseek_flags flag);
int32_t PIOS_FLASHFS_Info(uintptr_t fs_id, char *path, uint32_t *size, uint32_t file_number, uint32_t flags);

#endif /* PIOS_FLASHFS_H */

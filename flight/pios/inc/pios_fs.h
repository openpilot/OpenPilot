/**
 ******************************************************************************
 * @file       pios_fs.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2015.
 * @addtogroup PIOS PIOS Core hardware abstraction layer
 * @see        The GNU Public License (GPL) Version 3
 * @{
 * @addtogroup PIOS_FS Filesystem manager API Definition
 * @{
 * @brief PIOS API for filesystem.
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

#ifndef PIOS_FS_H
#define PIOS_FS_H

#include <stdint.h>

/* Stats */
typedef struct _pios_fs_Stats {
    uint16_t block_free;
    uint16_t block_used;
    uint16_t cache_hits;
    uint16_t cache_misses;
    uint16_t gc;
    uint16_t saved;
} pios_fs_Stats;

/* lseek flags */
#define PIOS_FS_SEEK_SET 0
#define PIOS_FS_SEEK_CUR 1
#define PIOS_FS_SEEK_END 2

/* any write will be appended to the end of the file */
#define PIOS_FS_APPEND 0x1
/* if the file exists, it will be truncated */
#define PIOS_FS_TRUNC 0x2
/* if file does not exists, it will be created */
#define PIOS_FS_CREAT 0x4
/* open as read only */
#define PIOS_FS_RDONLY 0x8
/* open as write only */
#define PIOS_FS_WRONLY 0x10
/* open as read and write*/
#define PIOS_FS_RDWR (PIOS_FS_RDONLY | PIOS_FS_WRONLY)
/* Write not cached (directly to spi flash) */
#define PIOS_FS_WRTHROUGH 0x20
/* Remove matching file */
#define PIOS_FS_REMOVE 1

#define PIOS_FS_FORMAT_FLAG_CHIP_ERASE 0x1

/* Errors */
#define PIOS_FS_OK 0
#define PIOS_FS_ERROR_FS_INVALID -1
#define PIOS_FS_ERROR_OPEN_FILE -2
#define PIOS_FS_ERROR_WRITE_FILE -3
#define PIOS_FS_ERROR_READ_FILE -4
#define PIOS_FS_ERROR_FS_MOUNT -5
#define PIOS_FS_ERROR_FS_ALLOC -6
#define PIOS_FS_ERROR_REMOVE_FILE -7
#define PIOS_FS_ERROR_PARAMETER -8
#define PIOS_FS_ERROR_EOF -9

/* File name */
#define FS_FILENAME_LEN 26

typedef struct _fs_operations {
    int32_t (*lseek)(uintptr_t, int32_t, int32_t, uint32_t);
    int32_t (*read)(uintptr_t, int32_t, uint8_t*, uint16_t);
    int32_t (*write)(uintptr_t, int32_t, uint8_t*, uint16_t);
    int32_t (*open)(uintptr_t, char*, uint16_t);
    int32_t (*remove)(uintptr_t, char*);
    int32_t (*format)(uintptr_t, uint32_t);
    int32_t (*stats)(uintptr_t, pios_fs_Stats*);
    int32_t (*close)(uintptr_t, int32_t);
    int32_t (*find)(uintptr_t, const char *, uint16_t, uint32_t, uint32_t);
    int32_t (*info)(uintptr_t, char *, uint32_t *, uint32_t, uint32_t);
} fs_operations;


/* API */
int32_t PIOS_FS_Format(uintptr_t fs_id, uint32_t flags);
int32_t PIOS_FS_Close(uintptr_t fs_id, int32_t file_id);
int32_t PIOS_FS_Open(uintptr_t fs_id, char *path, uint16_t flags);
int32_t PIOS_FS_Write(uintptr_t fs_id, int32_t fh, uint8_t *data, uint16_t size);
int32_t PIOS_FS_Read(uintptr_t fs_id, int32_t fh, uint8_t *data, uint16_t size);
int32_t PIOS_FS_Remove(uintptr_t fs_id, char *path);
int32_t PIOS_FS_Stats(uintptr_t fs_id, pios_fs_Stats *stats);
int32_t PIOS_FS_Find(uintptr_t fs_id, const char *path, uint16_t pattern_size, uint32_t pattern_offset, uint32_t flags);
int32_t PIOS_FS_Lseek(uintptr_t fs_id, int32_t fh, int32_t offset, uint32_t flag);
int32_t PIOS_FS_Info(uintptr_t fs_id, char *path, uint32_t *size, uint32_t file_number, uint32_t flags);


#endif /* PIOS_FS_H */

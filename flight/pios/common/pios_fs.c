/**
 ******************************************************************************
 * @file       pios_fs.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2015.
 * @addtogroup PIOS PIOS Core hardware abstraction layer
 * @see        The GNU Public License (GPL) Version 3
 * @{
 * @addtogroup PIOS_FS Filesystem middle layer Definition
 * @{
 * @brief PIOS generic API for filesystem.
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


#include "pios.h"


#ifdef PIOS_INCLUDE_FS

#include <stdbool.h>
#include <openpilot.h>
#include <pios_math.h>
#include <pios_wdg.h>
#include "pios_fs.h"


struct fs_state {
    uint32_t pad32;
    fs_operations *fops;
};


/**
 * @brief Opens/creates a file.
 * @param[in] fs_id The filesystem to use for this action
 * @param[in] path the path of the new file
 * @param[in] flags the flags for the open command, can be combinations of
 *                      SPIFFS_APPEND, SPIFFS_TRUNC, SPIFFS_CREAT, SPIFFS_RDONLY,
 *                      SPIFFS_WRONLY, SPIFFS_RDWR, SPIFFS_DIRECT
 */
int32_t PIOS_FS_Open(uintptr_t fs_id, char *path, uint16_t flags)
{
    struct fs_state *fs = (struct fs_state *)fs_id;

    if (fs->fops->open != NULL)
        return fs->fops->open(fs_id, path, flags);
    else
        return -1;
}


/**
 * @brief Close a file.
 * @param[in] fs_id The filesystem to use for this action
 * @param[in] the filehandle of the file to close
 */
int32_t PIOS_FS_Close(uintptr_t fs_id, int32_t file_id)
{
    struct fs_state *fs = (struct fs_state *)fs_id;

    if (fs->fops->close)
        return fs->fops->close(fs_id, file_id);
    else
        return -1;
}


/**
 * @brief Saves one object instance to the filesystem
 * @param[in] fs_id The filesystem to use for this action
 * @param[in] fh File handle
 * @param[in] data Contents of the object being written
 * @param[in] size Size of the object being saved
 */
int32_t PIOS_FS_Write(uintptr_t fs_id, int32_t fh, uint8_t *data, uint16_t size)
{
    struct fs_state *fs = (struct fs_state *)fs_id;

    if (fs->fops->write)
        return fs->fops->write(fs_id, fh, data, size);
    else
        return -1;
}

/**
 * @brief Load one object instance from the filesystem
 * @param[in] fs_id The filesystem to use for this action
 * @param[in] fh File handle
 * @param[in] data Buffer to hold the contents of the loaded object
 * @param[in] size Size of the object to be loaded
 */
int32_t PIOS_FS_Read(uintptr_t fs_id, int32_t fh, uint8_t *data, uint16_t size)
{
    struct fs_state *fs = (struct fs_state *)fs_id;

    if (fs->fops->read)
        return fs->fops->read(fs_id, fh, data, size);
    else
        return -1;
}


/**
 * @brief Moves the read/write file offset
 * @param[in] fs_id The filesystem to use for this action
 * @param[in] fh File handle
 * @param[in] how much/where to move the offset
 * @param[in] flags
 */
int32_t PIOS_FS_Lseek(uintptr_t fs_id, int32_t fh, int32_t offset, uint32_t flags)
{
    struct fs_state *fs = (struct fs_state *)fs_id;

    if (fs->fops->lseek)
        return fs->fops->lseek(fs_id, fh, offset, flags);
    else
        return -1;
}


/**
 * @brief Delete one instance of an object from the filesystem
 * @param[in] fs_id The filesystem to use for this action
 * @param[in] path name of the file to delete
 */
int32_t PIOS_FS_Remove(uintptr_t fs_id, char *path)
{
    struct fs_state *fs = (struct fs_state *)fs_id;

    if (fs->fops->remove)
        return fs->fops->remove(fs_id, path);
    else
        return -1;
}


/**
 * @brief Helper function: find file
 * @param[in] fs_id The filesystem to use for this action
 * @param[in] path string to use for the search
 * @param[in] size number of bytes to use as a pattern for the search
 * @param[in] offset offset of pattern to use for the search
 * @param[in] flags the flags for the find command, can be combinations of REMOVE
 */
int32_t PIOS_FS_Find(uintptr_t fs_id, const char *path, uint16_t pattern_size, uint32_t pattern_offset, uint32_t flags)
{
    struct fs_state *fs = (struct fs_state *)fs_id;

    if (fs->fops->find)
        return fs->fops->find(fs_id, path, pattern_size, pattern_offset, flags);
    else
        return -1;
}


/**
 * @brief Helper function: number of files.
 * @param[in] fs_id The filesystem to use for this action
 * @param[out] file name.
 * @param[out] file size.
 * @param[in] file number in the dir.
 * @param[in] flags the flags for the open command, can be combinations of TBD
 */
int32_t PIOS_FS_Info(uintptr_t fs_id, char *path, uint32_t *size, uint32_t file_number, uint32_t flags)
{
    struct fs_state *fs = (struct fs_state *)fs_id;

    if (fs->fops->info)
        return fs->fops->info(fs_id, path, size, file_number, flags);
    else
        return -1;
}


/**
 * @brief Format the filesystem
 * @param[in] fs_id The filesystem to use for this action
 */
int32_t PIOS_FS_Format(uintptr_t fs_id, uint32_t flags)
{
    struct fs_state *fs = (struct fs_state *)fs_id;

    if (fs->fops->format)
        return fs->fops->format(fs_id, flags);
    else
        return -1;
}


/**
 * @brief Return stats for the filesystem
 * @param[in] fs_id The filesystem to use for this action
 */
int32_t PIOS_FS_Stats(uintptr_t fs_id, pios_fs_Stats *stats)
{
    struct fs_state *fs = (struct fs_state *)fs_id;

    if (fs->fops->stats)
        return fs->fops->stats(fs_id, stats);
    else
        return -1;
}
#endif /* PIOS_INCLUDE_FLASH */

/**
 * @}
 * @}
 */

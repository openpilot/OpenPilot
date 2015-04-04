/**
 ******************************************************************************
 * @file       pios_fs_spiffs.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2015.
 * @addtogroup PIOS PIOS Core hardware abstraction layer
 * @see        The GNU Public License (GPL) Version 3
 * @{
 * @addtogroup PIOS_FS_SPIFFS Flash Filesystem API Definition
 * @{
 * @brief PIOS API for internal or onboard filesystem using SPIFFS.
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


#ifdef PIOS_INCLUDE_FS_SPIFFS

#include <stdbool.h>
#include <openpilot.h>
#include <pios_math.h>
#include <pios_wdg.h>
#include "pios_fs.h"
#include "pios_fs_spiffs.h"

#include "spiffs_config.h"

#define SPIFFS_FDS_SIZE (32*4)

#if defined(PIOS_INCLUDE_FREERTOS)
xSemaphoreHandle fs_spiffs_mutex = 0;
#endif


#define PIOS_FS_SPIFFS_OPID_CONTENT "www.openpilot.org\n"
#define PIOS_FS_SPIFFS_OPID_SIZE_MAX 256
#define  PIOS_FS_SPIFFS_OPID_FILENAME "OPID.txt"

enum pios_fs_spiffs_dev_magic {
    PIOS_FS_SPIFFS_DEV_MAGIC = 0x94938201,
};


struct spiffs_state {
    enum pios_fs_spiffs_dev_magic magic;
    fs_operations *fops;
    bool mounted;
    u8_t *work_buf;
    u8_t *cache_buf;
    u8_t *copy_buf;
    spiffs_config cfg_spiffs;
    u8_t spiffs_fds[SPIFFS_FDS_SIZE];
    spiffs fs;
    /* Underlying flash driver glue */
    const struct pios_flash_driver *driver;
    const struct pios_fs_spiffs_cfg *cfg;
    uint16_t files;
    uintptr_t flash_id;
};


/**
 * @brief Validate flashfs struct.
 */
static bool PIOS_FS_SPIFFS_Validate(const struct spiffs_state *fs)
{
    return fs && (fs->magic == PIOS_FS_SPIFFS_DEV_MAGIC);
}


/**
 * @brief Allocate flashfs struct.
 */
#if defined(PIOS_INCLUDE_FREERTOS)
static struct spiffs_state *PIOS_FS_SPIFFS_alloc(void)
{
    struct spiffs_state *fs;

    fs = (struct spiffs_state *)pios_malloc(sizeof(*fs));
    if (!fs) {
        return NULL;
    }

    fs->magic = PIOS_FS_SPIFFS_DEV_MAGIC;
    return fs;
}
static void PIOS_FS_SPIFFS_free(struct spiffs_state *fs)
{
    /* Invalidate the magic */
    fs->magic = ~PIOS_FS_SPIFFS_DEV_MAGIC;
    vPortFree(fs);
}
#else
static struct spiffs_state pios_fs_devs[PIOS_FS_SPIFFS_MAX_DEVS];
static uint8_t pios_fs_num_devs;
static struct spiffs_state *PIOS_FS_SPIFFS_alloc(void)
{
    struct spiffs_state *fs;

    if (pios_fs_num_devs >= PIOS_FS_SPIFFS_MAX_DEVS) {
        return NULL;
    }

    fs = &pios_fs_devs[pios_fs_num_devs++];
    fs->magic = PIOS_FS_SPIFFS_DEV_MAGIC;

    return fs;
}
static void PIOS_FS_SPIFFS_free(struct spiffs_state *fs)
{
    /* Invalidate the magic */
    fs->magic = ~PIOS_FS_SPIFFS_DEV_MAGIC;
}
#endif /* if defined(PIOS_INCLUDE_FREERTOS) */


/**
 * @brief HAL page read
 */
static s32_t PIOS_FS_SPIFFS_spiffs_read(void *fs, u32_t addr, u32_t size, u8_t *dst)
{
    struct spiffs_state *fs_spiffs = (struct spiffs_state*)((spiffs*)fs)->cfg.fs_id;
    fs_spiffs->driver->read_data(fs_spiffs->flash_id, addr, dst, (u16_t)size);
    return SPIFFS_OK;
}


/**
 * @brief HAL page write
 */
static s32_t PIOS_FS_SPIFFS_spiffs_write(void *fs, u32_t addr, u32_t size, u8_t *src)
{
    struct spiffs_state *fs_spiffs = (struct spiffs_state*)((spiffs*)fs)->cfg.fs_id;
    fs_spiffs->driver->write_data(fs_spiffs->flash_id, addr, src, (u16_t)size);
    return SPIFFS_OK;
}


/**
 * @brief HAL sector erase
 */
static s32_t PIOS_FS_SPIFFS_spiffs_erase(void *fs, u32_t addr, __attribute__((unused)) u32_t size)
{
    struct spiffs_state *fs_spiffs = (struct spiffs_state*)((spiffs*)fs)->cfg.fs_id;
    fs_spiffs->driver->erase_sector(fs_spiffs->flash_id, addr);
	return SPIFFS_OK;
}


/**
 * @brief Mount the file system
 */
static int32_t PIOS_FS_SPIFFS_Mount(__attribute__((unused)) uintptr_t fs_id)
{
    int8_t rc = PIOS_FS_OK;

    struct spiffs_state *fs_spiffs = (struct spiffs_state *)fs_id;

    if (!PIOS_FS_SPIFFS_Validate(fs_spiffs))
        return PIOS_FS_ERROR_FS_INVALID;

    memset(fs_spiffs->cache_buf, 0, fs_spiffs->cfg->cache_buffer_size);

    fs_spiffs->mounted  = false;

    if (SPIFFS_mount(&fs_spiffs->fs,
                        &fs_spiffs->cfg_spiffs,
                        fs_spiffs->work_buf,
                        fs_spiffs->spiffs_fds,
                        SPIFFS_FDS_SIZE,
                        fs_spiffs->cache_buf,
                        fs_spiffs->cfg->cache_buffer_size,
                        NULL) != SPIFFS_OK) {
        rc = PIOS_FS_ERROR_FS_MOUNT;
    }
    else
    {
        fs_spiffs->mounted  = true;
#if 0
        /* How many file available? */
        spiffs_DIR d;
        struct spiffs_dirent e;
        struct spiffs_dirent *pe = &e;

        SPIFFS_opendir(&fs_spiffs->fs, "/", &d);

        fs_spiffs->files = 0;
        while ((pe = SPIFFS_readdir(&d, pe))) {
            spiffs_file fd = SPIFFS_open_by_dirent(&fs_spiffs->fs, pe, SPIFFS_RDWR, 0);
            if (fd >= 0) {
                fs_spiffs->files++;
                SPIFFS_close(&fs_spiffs->fs, fd);
            }
        }
        SPIFFS_closedir(&d);
        rc = fs_spiffs->files;
#endif
    }

    return rc;
}


/**
 * Delete the filesystem information from memory.
 * @param[in] fs_id flash device id
 */
int32_t PIOS_FS_SPIFFS_Destroy(__attribute__((unused)) uintptr_t fs_id)
{
    struct spiffs_state *fs_spiffs = (struct spiffs_state *)fs_id;

    if (!PIOS_FS_SPIFFS_Validate(fs_spiffs))
        return PIOS_FS_ERROR_FS_INVALID;

    if (fs_spiffs->mounted == true)
	{
        SPIFFS_unmount(&fs_spiffs->fs);
        fs_spiffs->mounted = false;
	}

    PIOS_FS_SPIFFS_free(fs_spiffs);

    return PIOS_FS_OK;
}


/**
 * @brief Opens/creates a file.
 * @param[in] fs_id The filesystem to use for this action
 * @param[in] path the path of the new file
 * @param[in] flags the flags for the open command, can be combinations of
 *                      SPIFFS_APPEND, SPIFFS_TRUNC, SPIFFS_CREAT, SPIFFS_RDONLY,
 *                      SPIFFS_WRONLY, SPIFFS_RDWR, SPIFFS_DIRECT
 */
int32_t PIOS_FS_SPIFFS_Open(uintptr_t fs_id, char *path, uint16_t flags)
{
    int32_t file_id;

    struct spiffs_state *fs_spiffs = (struct spiffs_state *)fs_id;

    if (!PIOS_FS_SPIFFS_Validate(fs_spiffs))
        return PIOS_FS_ERROR_FS_INVALID;

    file_id = (int16_t)SPIFFS_open(&fs_spiffs->fs, path, flags, 0);

    if (file_id < PIOS_FS_OK)
        return PIOS_FS_ERROR_OPEN_FILE;

    return file_id;
}


/**
 * @brief Close a filehandle.
 * @param[in] fs_id The filesystem to use for this action
 * @param[in] the filehandle of the file to close
 */
int32_t PIOS_FS_SPIFFS_Close(uintptr_t fs_id, int32_t file_id)
{
    struct spiffs_state *fs_spiffs = (struct spiffs_state *)fs_id;

    if (!PIOS_FS_SPIFFS_Validate(fs_spiffs))
        return PIOS_FS_ERROR_FS_INVALID;

    SPIFFS_close(&fs_spiffs->fs, file_id);

    return PIOS_FS_OK;
}


/**
 * @brief Saves one object instance to the filesystem
 * @param[in] fs_id The filesystem to use for this action
 * @param[in] fh File handle
 * @param[in] data Contents of the object being written
 * @param[in] size Size of the object being saved
 */
int32_t PIOS_FS_SPIFFS_Write(uintptr_t fs_id, int32_t fh, uint8_t *data, uint16_t size)
{
    int32_t bytes_written = 0;
    int32_t rc = PIOS_FS_OK;

    struct spiffs_state *fs_spiffs = (struct spiffs_state *)fs_id;

    if (!PIOS_FS_SPIFFS_Validate(fs_spiffs))
        return PIOS_FS_ERROR_FS_INVALID;

    bytes_written = SPIFFS_write(&fs_spiffs->fs, (spiffs_file)fh, data, (int32_t)size);

    if (bytes_written != size) {
        rc = PIOS_FS_ERROR_WRITE_FILE;
    }
    else
    {
        fs_spiffs->files++;
    }

    return rc;
}

/**
 * @brief Load one object instance from the filesystem
 * @param[in] fs_id The filesystem to use for this action
 * @param[in] fh File handle
 * @param[in] data Buffer to hold the contents of the loaded object
 * @param[in] size Size of the object to be loaded
 */
int32_t PIOS_FS_SPIFFS_Read(uintptr_t fs_id, int32_t fh, uint8_t *data, uint16_t size)
{
    int32_t bytes_read = 0;
    int32_t rc = PIOS_FS_OK;

    struct spiffs_state *fs_spiffs = (struct spiffs_state *)fs_id;

    if (!PIOS_FS_SPIFFS_Validate(fs_spiffs))
        return PIOS_FS_ERROR_FS_INVALID;

    bytes_read = SPIFFS_read(&fs_spiffs->fs, (spiffs_file)fh, data, (int32_t)size);

    if (rc < 0) {
        rc = PIOS_FS_ERROR_READ_FILE;
        if (SPIFFS_errno(&fs_spiffs->fs) == SPIFFS_ERR_END_OF_OBJECT)
            rc = PIOS_FS_ERROR_EOF;
    }
    else
    {
        rc = bytes_read;
    }

    return rc;
}


/**
 * @brief Moves the read/write file offset
 * @param[in] fs_id The filesystem to use for this action
 * @param[in] fh File handle
 * @param[in] how much/where to move the offset
 * @param[in] if FLASHFS_SEEK_SET, the file offset shall be set to offset bytes
 *            if FLASHFS_SEEK_CUR, the file offset shall be set to its current location + offset
 *            if FLASHFS_SEEK_END, the file offset shall be set to the size of the file - offset
 */
int32_t PIOS_FS_SPIFFS_Lseek(uintptr_t fs_id, int32_t fh, int32_t offset, uint32_t flag)
{
    int32_t rc = PIOS_FS_OK;

    struct spiffs_state *fs_spiffs = (struct spiffs_state *)fs_id;

    if (!PIOS_FS_SPIFFS_Validate(fs_spiffs))
        return PIOS_FS_ERROR_FS_INVALID;

    rc = SPIFFS_lseek(&fs_spiffs->fs, (spiffs_file)fh, offset, flag);

    if (rc == SPIFFS_ERR_END_OF_OBJECT)
        return PIOS_FS_ERROR_EOF;

    if (rc < 0)
        return PIOS_FS_ERROR_READ_FILE;

    return PIOS_FS_OK;
}


/**
 * @brief Delete one instance of an object from the filesystem
 * @param[in] fs_id The filesystem to use for this action
 * @param[in] path name of the file to delete
 */
int32_t PIOS_FS_SPIFFS_Remove(uintptr_t fs_id, char *path)
{
    int32_t rc = PIOS_FS_OK;
    struct spiffs_state *fs_spiffs = (struct spiffs_state *)fs_id;

    if (!PIOS_FS_SPIFFS_Validate(fs_spiffs))
        return PIOS_FS_ERROR_FS_INVALID;

    if (SPIFFS_remove(&fs_spiffs->fs, path) != SPIFFS_OK) {
    	rc = PIOS_FS_ERROR_REMOVE_FILE;
    }

    fs_spiffs->files--;
    return rc;
}


/**
 * @brief Helper function: 'find / -name "prefix*" | wc -l'
 * @param[in] fs_id The filesystem to use for this action
 * @param[in] path string to use for the search
 * @param[in] size number of bytes to use as a patter for the search
 * @param[in] offset offset of pattern to use for the search
 * @param[in] flags the flags for the find command, can be combinations of REMOVE
 */
int32_t PIOS_FS_SPIFFS_Find(uintptr_t fs_id, const char *path, uint16_t pattern_size, uint32_t pattern_offset, uint32_t flags)
{
    int32_t filecount = 0;
    int16_t file_id;

    struct spiffs_state *fs_spiffs = (struct spiffs_state *)fs_id;

    if (!PIOS_FS_SPIFFS_Validate(fs_spiffs))
        return PIOS_FS_ERROR_FS_INVALID;

    if ((pattern_size > FS_FILENAME_LEN) || (pattern_offset > FS_FILENAME_LEN))
        return PIOS_FS_ERROR_PARAMETER;

    spiffs_DIR d;
    struct spiffs_dirent e;
    struct spiffs_dirent *pe = &e;

    SPIFFS_opendir(&fs_spiffs->fs, "/", &d);

    if ((strcmp(path, "*") == 0) && (pattern_size == 1)) {
        while ((pe = SPIFFS_readdir(&d, pe)))
            filecount++;
    }
    else
    {
        while ((pe = SPIFFS_readdir(&d, pe))) {
            if (strncmp(path, (char*)(pe->name + pattern_offset), pattern_size) == 0) {
                if (flags & PIOS_FS_REMOVE) {
                    file_id = SPIFFS_open_by_dirent(&fs_spiffs->fs, pe, SPIFFS_RDWR, 0);
                    SPIFFS_fremove(&fs_spiffs->fs, file_id);
                    fs_spiffs->files--;
                }
                filecount++;
            }
        }
    }

    SPIFFS_closedir(&d);
    return filecount;
}


/**
 * @brief Helper function: 'ls / | awk '{nr++; if( nr == file_number) print $0}'
 * @param[in] fs_id The filesystem to use for this action
 * @param[out] file name.
 * @param[out] file size.
 * @param[in] file number in the dir.
 * @param[in] flags the flags for the open command, can be combinations of TBD
 */
int32_t PIOS_FS_SPIFFS_Info(uintptr_t fs_id, char *path, uint32_t *size, uint32_t file_number, __attribute__((unused)) uint32_t flags)
{
    uint32_t filecount = 0;

    struct spiffs_state *fs_spiffs = (struct spiffs_state *)fs_id;

    if (!PIOS_FS_SPIFFS_Validate(fs_spiffs))
        return PIOS_FS_ERROR_FS_INVALID;

    spiffs_DIR d;
    struct spiffs_dirent e;
    struct spiffs_dirent *pe = &e;

    SPIFFS_opendir(&fs_spiffs->fs, "/", &d);

    while ((pe = SPIFFS_readdir(&d, pe))) {
        if (filecount == file_number) {
            strcpy(path, (char*)pe->name);
            *size = pe->size;
        }
        filecount++;
    }

    SPIFFS_closedir(&d);
    return filecount;
}


/**
 * @brief Erases all filesystem arenas and activate the first arena
 * @param[in] fs_id The filesystem to use for this action
 */
int32_t PIOS_FS_SPIFFS_Format(uintptr_t fs_id, uint32_t flags)
{
    int32_t rc = PIOS_FS_OK;
    bool previous_mount_status;

    struct spiffs_state *fs_spiffs = (struct spiffs_state *)fs_id;

    if (!PIOS_FS_SPIFFS_Validate(fs_spiffs))
        return PIOS_FS_ERROR_FS_INVALID;

    previous_mount_status = SPIFFS_mounted(&fs_spiffs->fs);

    if (previous_mount_status)
    {
        SPIFFS_unmount(&fs_spiffs->fs);
        fs_spiffs->mounted = false;
    }
#if 0
    else
    {
        /* For backward compatibility */
        if (PIOS_FS_SPIFFS_Mount((uintptr_t)fs_spiffs) == PIOS_FS_OK)
            SPIFFS_unmount(&fs_spiffs->fs);
    }
#endif

    if (flags & PIOS_FS_FORMAT_FLAG_CHIP_ERASE)
    fs_spiffs->driver->erase_chip(fs_spiffs->flash_id);
    SPIFFS_format(&fs_spiffs->fs);

    if (previous_mount_status)
    {
        rc = PIOS_FS_SPIFFS_Mount((uintptr_t)fs_spiffs);
        //PIOS_FS_SPIFFS_AddIDFile((uintptr_t)fs_spiffs);
    }

    fs_spiffs->files = 0;
    return rc;
}


/**
 * @brief Returs stats for the filesystems
 * @param[in] fs_id The filesystem to use for this action
 */
int32_t PIOS_FS_SPIFFS_GetStats(__attribute__((unused)) uintptr_t fs_id, pios_fs_Stats *stats)
{
    PIOS_Assert(stats);

    struct spiffs_state *fs_spiffs = (struct spiffs_state *)fs_id;

    if (!PIOS_FS_SPIFFS_Validate(fs_spiffs))
        return PIOS_FS_ERROR_FS_INVALID;

    stats->block_free = fs_spiffs->fs.free_blocks;
    stats->block_used = fs_spiffs->fs.block_count - stats->block_free;
#if SPIFFS_CACHE_STATS
    stats->cache_hits = fs_spiffs->fs.cache_hits;
    stats->cache_misses = fs_spiffs->fs.cache_misses;
#else
    stats->cache_hits = 0;
    stats->cache_misses = 0;
#endif
#if SPIFFS_GC_STATS
    stats->gc = fs_spiffs->fs.stats_gc_runs;
#else
    stats->gc = 0;
#endif
    stats->saved = fs_spiffs->files;
    return PIOS_FS_OK;
}


static fs_operations pios_fs_spiffs_fops __attribute__((__used__)) = {
    .lseek      = PIOS_FS_SPIFFS_Lseek,
    .read       = PIOS_FS_SPIFFS_Read,
    .write      = PIOS_FS_SPIFFS_Write,
    .open       = PIOS_FS_SPIFFS_Open,
    .format     = PIOS_FS_SPIFFS_Format,
    .stats      = PIOS_FS_SPIFFS_GetStats,
    .close      = PIOS_FS_SPIFFS_Close,
    .find       = PIOS_FS_SPIFFS_Find,
    .info       = PIOS_FS_SPIFFS_Info,
    .remove     = PIOS_FS_SPIFFS_Remove,
};


/**
 * @brief Initialize the file system
 */
int32_t PIOS_FS_SPIFFS_Init(__attribute__((unused)) uintptr_t *fs_id,
                            const struct pios_fs_spiffs_cfg *cfg,
                            const struct pios_flash_driver *driver,
                            uintptr_t flash_id)
{
    //int16_t fh;
    int16_t rc = PIOS_FS_OK;
//    spiffs_stat fstats;

    struct spiffs_state *fs_spiffs = (struct spiffs_state *)*fs_id;

    if (driver)
    {
        /* Make sure the underlying flash driver provides the minimal set of required methods */
        PIOS_Assert(driver->start_transaction);
        PIOS_Assert(driver->end_transaction);
        PIOS_Assert(driver->erase_sector);
        PIOS_Assert(driver->write_data);
        PIOS_Assert(driver->read_data);
    }

    /* We only support one instance of SPIFFS (SPIFFS_SINGLETON = 1). */

    /* This function can be called after a format. */
    /* (we won't re-allocate memory in that case) */
    if (!fs_spiffs)
    {
        fs_spiffs = (struct spiffs_state *)PIOS_FS_SPIFFS_alloc();

        if (!fs_spiffs)
            return PIOS_FS_ERROR_FS_ALLOC;

        fs_spiffs->work_buf = pios_malloc(cfg->work_buffer_size);
        fs_spiffs->cache_buf = pios_malloc(cfg->cache_buffer_size);
        fs_spiffs->copy_buf = pios_malloc(cfg->logical_page_size);

        /* Mutex is used in SPIFFS */
#if defined(PIOS_INCLUDE_FREERTOS)
        if (!fs_spiffs_mutex)
            fs_spiffs_mutex = xSemaphoreCreateRecursiveMutex();
#endif

        /* Bind configuration parameters to this filesystem instance */
        fs_spiffs->driver   = driver; /* lower-level flash driver */
        fs_spiffs->flash_id = flash_id; /* lower-level flash device id */
        fs_spiffs->mounted  = false;
        fs_spiffs->cfg      = cfg;

        /* HAL Callbacks */
        fs_spiffs->cfg_spiffs.hal_read_f = PIOS_FS_SPIFFS_spiffs_read;
        fs_spiffs->cfg_spiffs.hal_write_f = PIOS_FS_SPIFFS_spiffs_write;
        fs_spiffs->cfg_spiffs.hal_erase_f = PIOS_FS_SPIFFS_spiffs_erase;

        fs_spiffs->cfg_spiffs.phys_size = cfg->physical_size;
        fs_spiffs->cfg_spiffs.phys_addr = cfg->physical_addr;
        fs_spiffs->cfg_spiffs.phys_erase_block = cfg->physical_erase_block;
        fs_spiffs->cfg_spiffs.log_block_size = cfg->logical_block_size;
        fs_spiffs->cfg_spiffs.log_page_size = cfg->logical_page_size;

        fs_spiffs->cfg_spiffs.fs_id = (void *)fs_spiffs;

        /* Init buffers */
        fs_spiffs->cfg_spiffs.buffer = fs_spiffs->copy_buf;

        fs_spiffs->fops = &pios_fs_spiffs_fops;
    }

    if ((rc = PIOS_FS_SPIFFS_Mount((uintptr_t)fs_spiffs)) == PIOS_FS_OK) {
        *fs_id = (uintptr_t)fs_spiffs;
    } else {
        SPIFFS_format(&fs_spiffs->fs);
        rc = PIOS_FS_SPIFFS_Mount((uintptr_t)fs_spiffs);
    }

    PIOS_Assert(rc == PIOS_FS_OK);
    return rc;
}


#endif /* PIOS_INCLUDE_FS_SPIFFS */

/**
 * @}
 * @}
 */

/**
 ******************************************************************************
 * @file       pios_dosfs_logfs.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2013.
 * @addtogroup PIOS PIOS Core hardware abstraction layer
 * @{
 * @brief Log Structured Filesystem wrapper implemented using dosfs
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

#ifdef PIOS_USE_SETTINGS_ON_SDCARD

#include <openpilot.h>

#if defined(PIOS_INCLUDE_FREERTOS)
static xSemaphoreHandle mutex = 0;
#endif

struct flashfs_logfs_cfg;


/**
 * Get an 8 character (plus extension) filename for the object.
 * @param[in] obj The object handle.
 * @param[in] instId The instance ID
 * @param[in] file Filename string pointer -- must be 14 bytes long and allocated
 */
static void objectFilename(uint32_t obj_id, uint16_t obj_inst_id, uint8_t *filename)
{
    uint32_t prefix = obj_id + (obj_inst_id / 256) * 16; // put upper 8 bit of instance id into object id modification,
                                                         // skip least sig nibble since that is used for meta object id
    uint8_t suffix  = obj_inst_id & 0xff;

    snprintf((char *)filename, 13, "%08X.o%02X", prefix, suffix);
}


/**
 * @brief Initialize the flash object setting FS
 * @return 0 if success, -1 if failure
 */
int32_t PIOS_FLASHFS_Logfs_Init(__attribute__((unused)) uintptr_t *fs_id, __attribute__((unused)) const struct flashfs_logfs_cfg *cfg, __attribute__((unused)) const struct pios_flash_driver *driver, __attribute__((unused)) uintptr_t flash_id)
{
#if defined(PIOS_INCLUDE_FREERTOS)
    if (!mutex) {
        mutex = xSemaphoreCreateRecursiveMutex();
    }
#endif
    return 0;
}

int32_t PIOS_FLASHFS_Logfs_Destroy(__attribute__((unused)) uintptr_t fs_id)
{
    // stub, only wrapper for dosfs, does not need destroying
    return 0;
}

/**********************************
 *
 * Provide a PIOS_FLASHFS_* driver
 *
 *********************************/
#include "pios_flashfs.h" /* API for flash filesystem */

/**
 * @brief Saves one object instance to the filesystem
 * @param[in] fs_id The filesystem to use for this action
 * @param[in] obj UAVObject ID of the object to save
 * @param[in] obj_inst_id The instance number of the object being saved
 * @param[in] obj_data Contents of the object being saved
 * @param[in] obj_size Size of the object being saved
 * @return 0 if success or error code
 * @retval -1 if fs_id is not a valid filesystem instance
 * @retval -2 if failed to start transaction
 * @retval -3 if failure to delete any previous versions of the object
 * @retval -4 if filesystem is entirely full and garbage collection won't help
 * @retval -5 if garbage collection failed
 * @retval -6 if filesystem is full even after garbage collection should have freed space
 * @retval -7 if writing the new object to the filesystem failed
 */
int32_t PIOS_FLASHFS_ObjSave(__attribute__((unused)) uintptr_t fs_id, uint32_t obj_id, uint16_t obj_inst_id, uint8_t *obj_data, uint16_t obj_size)
{
    FILEINFO file;
    uint8_t filename[14];

    if (PIOS_SDCARD_IsMounted() == 0) {
        return -1;
    }

    // Lock

#if defined(PIOS_INCLUDE_FREERTOS)
    xSemaphoreTakeRecursive(mutex, portMAX_DELAY);
#endif

    // Get filename
    objectFilename(obj_id, obj_inst_id, filename);

    // Open file
    if (PIOS_FOPEN_WRITE(filename, file)) {
#if defined(PIOS_INCLUDE_FREERTOS)
        xSemaphoreGiveRecursive(mutex);
#endif
        return -2;
    }
    // Append object
    uint32_t bytes_written = 0;
    PIOS_FWRITE(&file, obj_data, obj_size, &bytes_written);

    // Done, close file and unlock
    PIOS_FCLOSE(file);
#if defined(PIOS_INCLUDE_FREERTOS)
    xSemaphoreGiveRecursive(mutex);
#endif

    if (bytes_written != obj_size) {
        return -7;
    }

    return 0;
}

/**
 * @brief Load one object instance from the filesystem
 * @param[in] fs_id The filesystem to use for this action
 * @param[in] obj UAVObject ID of the object to load
 * @param[in] obj_inst_id The instance of the object to load
 * @param[in] obj_data Buffer to hold the contents of the loaded object
 * @param[in] obj_size Size of the object to be loaded
 * @return 0 if success or error code
 * @retval -1 if fs_id is not a valid filesystem instance
 * @retval -2 if failed to start transaction
 * @retval -3 if object not found in filesystem
 * @retval -4 if object size in filesystem does not exactly match buffer size
 * @retval -5 if reading the object data from flash fails
 */
int32_t PIOS_FLASHFS_ObjLoad(__attribute__((unused)) uintptr_t fs_id, uint32_t obj_id, uint16_t obj_inst_id, uint8_t *obj_data, uint16_t obj_size)
{
    FILEINFO file;
    uint8_t filename[14];

    // Check for file system availability
    if (PIOS_SDCARD_IsMounted() == 0) {
        return -1;
    }

#if defined(PIOS_INCLUDE_FREERTOS)
    // Lock
    xSemaphoreTakeRecursive(mutex, portMAX_DELAY);
#endif
    // Get filename
    objectFilename(obj_id, obj_inst_id, filename);

    // Open file
    if (PIOS_FOPEN_READ(filename, file)) {
#if defined(PIOS_INCLUDE_FREERTOS)
        xSemaphoreGiveRecursive(mutex);
#endif
        return -1;
    }
    // Load object
    uint32_t bytes_read = 0;
    uint32_t result     = PIOS_FREAD(&file, obj_data, obj_size, &bytes_read);

    // Done, close file and unlock
    PIOS_FCLOSE(file);
#if defined(PIOS_INCLUDE_FREERTOS)
    xSemaphoreGiveRecursive(mutex);
#endif
    if (result != 0) {
        return -1;
    }
    return 0;
}

/**
 * @brief Delete one instance of an object from the filesystem
 * @param[in] fs_id The filesystem to use for this action
 * @param[in] obj UAVObject ID of the object to delete
 * @param[in] obj_inst_id The instance of the object to delete
 * @return 0 if success or error code
 * @retval -1 if fs_id is not a valid filesystem instance
 * @retval -2 if failed to start transaction
 * @retval -3 if failed to delete the object from the filesystem
 */
int32_t PIOS_FLASHFS_ObjDelete(__attribute__((unused)) uintptr_t fs_id, uint32_t obj_id, uint16_t obj_inst_id)
{
    uint8_t filename[14];

    // Check for file system availability
    if (PIOS_SDCARD_IsMounted() == 0) {
        return -1;
    }
#if defined(PIOS_INCLUDE_FREERTOS)
    // Lock
    xSemaphoreTakeRecursive(mutex, portMAX_DELAY);
#endif
    // Get filename
    objectFilename(obj_id, obj_inst_id, filename);

    // Delete file
    PIOS_FUNLINK(filename);

    // Done
#if defined(PIOS_INCLUDE_FREERTOS)
    xSemaphoreGiveRecursive(mutex);
#endif
    return 0;
}

/**
 * @brief Erases all filesystem arenas and activate the first arena
 * @param[in] fs_id The filesystem to use for this action
 * @return 0 if success or error code
 * @retval -1 if fs_id is not a valid filesystem instance
 * @retval -2 if failed to start transaction
 * @retval -3 if failed to erase all arenas
 * @retval -4 if failed to activate arena 0
 * @retval -5 if failed to mount arena 0
 */
int32_t PIOS_FLASHFS_Format(__attribute__((unused)) uintptr_t fs_id)
{
    /* stub - not implemented */
    return -1;
}

/**
 * @brief Returs stats for the filesystems
 * @param[in] fs_id The filesystem to use for this action
 * @return 0 if success or error code
 * @retval -1 if fs_id is not a valid filesystem instance
 */
int32_t PIOS_FLASHFS_GetStats(__attribute__((unused)) uintptr_t fs_id, __attribute__((unused)) struct PIOS_FLASHFS_Stats *stats)
{
    /* stub - not implemented */
    return 0;
}

#endif /* PIOS_USE_SETTINGS_ON_SDCARD */

/**
 * @}
 * @}
 */

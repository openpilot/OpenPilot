/**
 ******************************************************************************
 * @file       pios_logfs.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2013.
 * @addtogroup PIOS PIOS Core hardware abstraction layer
 * @{
 * @brief Log Structured Filesystem leverages yaffs
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
#include "pios_stdio.h"
#include "pios_trace.h"

#include <openpilot.h>

/**
 * Get an object file name and device name and path name
 * @param[in] fs_id flash device id
 * @param[in] obj The object handle.
 * @param[in] instId The instance ID
 * @param[in] file Filename string pointer -- must be 14 bytes long and allocated
 * @param[in] path string pointer -- must be 26 bytes long and allocated
 */
#define OBJECTPATHNAME_LEN 26
static void getObjectPathAndName(uintptr_t fs_id, uint32_t obj_id, uint16_t obj_inst_id, char *filename)
{
    uint32_t prefix = obj_id + (obj_inst_id / 256) * 16; // put upper 8 bit of instance id into object id modification,
                                                         // skip least sig nibble since that is used for meta object id
    uint8_t suffix  = obj_inst_id & 0xff;

    snprintf((char *)filename, OBJECTPATHNAME_LEN, "/dev%01u/logfs/%08X.o%02X", (unsigned)fs_id, (uint16_t)prefix, suffix);
}

/**
 * Get an devicename
 * @param[in] fs_id flash device id
 * @param[in] devicename string pointer -- must be 8 bytes long and allocated
 */
#define DEVICENAME_LEN 8
static void getDeviceName(uintptr_t fs_id, char *devicename)
{
    snprintf((char *)devicename, DEVICENAME_LEN, "/dev%01u", (unsigned) fs_id);
}

/**
 * Get an logfs path
 * @param[in] fs_id flash device id
 * @param[in] devicename string pointer -- must be 14 bytes long and allocated
 */
#define LOGFSPATH_LEN 14
static void getLogfsPath(uintptr_t fs_id, char *devicename)
{
    snprintf((char *)devicename, LOGFSPATH_LEN, "/dev%01u/logfs", (unsigned)fs_id);
}

//Simplistic API takes the fs_id and maps to a path /dev0 eg.  not really necessary
// but allows reuse existing API plus new API in parallel without too much conflict

int32_t PIOS_FLASHFS_Logfs_Destroy(
	__attribute__((unused)) uintptr_t fs_id)
{
    pios_DIR *dp; 
    struct pios_dirent *ep;
    char path[LOGFSPATH_LEN];
    getLogfsPath(fs_id, path);

    pios_trace(PIOS_TRACE_TEST, "PIOS_FLASHFS_Logfs_Destroy");


    dp = pios_opendir (path);
    if (dp != NULL)
    {
      while ((ep = pios_readdir(dp))) {
        pios_unlink (ep->d_name);
      }
      (void) pios_closedir (dp);
    }
    else
    {
      pios_trace(PIOS_TRACE_ERROR, "Couldn't open the directory %s.", path);
    }
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
int32_t PIOS_FLASHFS_ObjSave(
	__attribute__((unused)) uintptr_t fs_id, 
	uint32_t obj_id, 
	uint16_t obj_inst_id, 
	uint8_t *obj_data, 
	uint16_t obj_size)
{
    int fd;
    char filename[OBJECTPATHNAME_LEN];
    uint32_t bytes_written = 0;

    pios_trace(PIOS_TRACE_TEST, "PIOS_FLASHFS_ObjSave");

    // Get filename
    getObjectPathAndName(fs_id, obj_id, obj_inst_id, filename);

    // Open file
    fd = pios_open(filename, O_CREAT | O_RDWR | O_TRUNC, S_IREAD | S_IWRITE);
    pios_trace(PIOS_TRACE_TEST, "pios_open (%s) retval=%d.", filename, fd);

    if (fd < 0) {
	    pios_trace(PIOS_TRACE_ERROR, "Couldn't open %s", filename);
    }
    else {
        // Append object
        bytes_written = pios_write(fd, obj_data, obj_size);
        pios_trace(PIOS_TRACE_TEST, "pios_write bytes_written=%d obj_size=%d", bytes_written, obj_size);

        // Done, close file and unlock
        pios_close(fd);
    }

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
int32_t PIOS_FLASHFS_ObjLoad(
	__attribute__((unused)) uintptr_t fs_id, 
	uint32_t obj_id, 
	uint16_t obj_inst_id, 
	uint8_t *obj_data, 
	uint16_t obj_size)
{
    int fd;
    char filename[OBJECTPATHNAME_LEN];
    uint32_t bytes_read = 0;

    pios_trace(PIOS_TRACE_TEST, "PIOS_FLASHFS_ObjLoad");

    // Get filename
    getObjectPathAndName(fs_id, obj_id, obj_inst_id, filename);

    fd = pios_open(filename,O_RDONLY, S_IREAD | S_IWRITE);

    pios_trace(PIOS_TRACE_TEST, "pios_open (%s) retval=%d.", filename, fd);

    if (fd < 0) {
	pios_trace(PIOS_TRACE_ERROR, "Couldn't open %s", filename);
    }
    else {
        // Load object
        bytes_read  = pios_read(fd, (void *)obj_data, (uint32_t)obj_size);
        pios_trace(PIOS_TRACE_TEST, "pios_read (%d) expected=%d.", obj_size, bytes_read);

        // Done, close file and unlock
        pios_close(fd);
    }

    if (bytes_read != obj_size)
        return -1;

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
int32_t PIOS_FLASHFS_ObjDelete(
	__attribute__((unused)) uintptr_t fs_id, 
	uint32_t obj_id, 
	uint16_t obj_inst_id)
{
    char filename[OBJECTPATHNAME_LEN];
    int retval;
    pios_trace(PIOS_TRACE_TEST, "PIOS_FLASHFS_ObjDelete");

    // Get filename
    getObjectPathAndName(fs_id, obj_id, obj_inst_id, filename);

    // Delete file
    retval = pios_unlink((char *)filename);  // 0 for success, -1 for fail
    pios_trace(PIOS_TRACE_TEST, "pios_unlink(%s) retval=%d" , filename, retval);

    return retval;
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
int32_t PIOS_FLASHFS_Format(
	__attribute__((unused)) uintptr_t fs_id)
{
    int retval;
    pios_trace(PIOS_TRACE_TEST, "PIOS_FLASHFS_Format");

    // Convert fs_id into device name
    char devicename[DEVICENAME_LEN];
    getDeviceName(fs_id, devicename);

    retval = pios_format(devicename,
		       1,  // unmount flag
		       1,  // force unmount flag
		       1);  // remount

    pios_trace(PIOS_TRACE_TEST, "pios_format (%s) retval=%d.", devicename, retval);
    return retval; 
}

/**
 * @brief Returs stats for the filesystems
 * @param[in] fs_id The filesystem to use for this action
 * @return 0 if success or error code
 * @retval -1 if fs_id is not a valid filesystem instance
 */
int32_t PIOS_FLASHFS_GetStats(
	__attribute__((unused)) uintptr_t fs_id, 
	__attribute__((unused)) struct PIOS_FLASHFS_Stats *stats)
{
    // pios_trace(PIOS_TRACE_TEST, "PIOS_FLASHFS_GetStats");

    // Convert fs_id into device name
    char devicename[DEVICENAME_LEN];
    getDeviceName(fs_id, devicename);

    // Get yaffs statistics for that device
    stats->num_free_slots = yaffs_freespace(devicename);
    stats->num_active_slots = yaffs_totalspace(devicename) - stats->num_free_slots;
    //TODO add in bad block count/stats
    //TODO mount status?
    //TODO Error conditions???

    // Return device usage statistics
    return 0;
}


/**
 * @}
 * @}
 */

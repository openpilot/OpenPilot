/**
 ******************************************************************************
 *
 * @file       uavobjectpersistence.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2014.
 * @brief      handles uavo persistence
 *             --
 * @see        The GNU Public License (GPL) Version 3
 *
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

#include "openpilot.h"
#include "pios_struct_helper.h"
#include "inc/uavobjectprivate.h"

extern uintptr_t pios_uavo_settings_fs_id;

/* Generate file name */
static void UAVObjFilenameCreate(uintptr_t fs_id, uint32_t obj_id, uint16_t obj_inst_id, char *filename)
{
    uint32_t prefix = obj_id + (obj_inst_id / 256) * 16; // put upper 8 bit of instance id into object id modification,
                                                         // skip least sig nibble since that is used for meta object id
    uint8_t suffix  = obj_inst_id & 0xff;

    snprintf((char *)filename, FLASHFS_FILENAME_LEN, UAVO_PREFIX_STRING"%01u/%08X.o%02X", (unsigned)fs_id, (unsigned int)prefix, suffix);
}


/* Get prefix of all uavObj files */
static void UAVObjPrefixGet(uintptr_t fs_id, char *devicename)
{
    snprintf((char *)devicename, FLASHFS_FILENAME_LEN, UAVO_PREFIX_STRING"%01u", (unsigned)fs_id);
}


/**
 * Save the data of the specified object to the file system (SD card).
 * If the object contains multiple instances, all of them will be saved.
 * A new file with the name of the object will be created.
 * The object data can be restored using the UAVObjLoad function.
 * @param[in] obj The object handle.
 * @param[in] instId The instance ID
 * @return 0 if success or -1 if failure
 */
int32_t UAVObjSave(UAVObjHandle obj_handle, uint16_t instId)
{
    int32_t rc = 0;
    int16_t fh;
    uint8_t *obj_data;
    char filename[FLASHFS_FILENAME_LEN];

    PIOS_Assert(obj_handle);

    if (UAVObjIsMetaobject(obj_handle)) {

        if (instId != 0)
            return -1;

        obj_data = (uint8_t*)MetaDataPtr((struct UAVOMeta *)obj_handle);

    } else {
        InstanceHandle instEntry = getInstance((struct UAVOData *)obj_handle, instId);

        if (instEntry == NULL)
            return -1;

        if (InstanceData(instEntry) == NULL)
            return -1;

        obj_data = InstanceData(instEntry);

    }

    UAVObjFilenameCreate(pios_uavo_settings_fs_id, UAVObjGetID(obj_handle), instId, filename);

    fh = PIOS_FLASHFS_Open(pios_uavo_settings_fs_id, filename, PIOS_FLASHFS_CREAT | PIOS_FLASHFS_WRONLY | PIOS_FLASHFS_TRUNC);

    if (fh < 0)
        return -1;

    if (PIOS_FLASHFS_Write(pios_uavo_settings_fs_id, fh, obj_data, UAVObjGetNumBytes(obj_handle)) != 0)
        rc = -1;

    PIOS_FLASHFS_Close(pios_uavo_settings_fs_id, fh);

    return rc;
}


/**
 * Load an object from the file system (SD card).
 * A file with the name of the object will be opened.
 * The object data can be saved using the UAVObjSave function.
 * @param[in] obj The object handle.
 * @param[in] instId The object instance
 * @return 0 if success or -1 if failure
 */
int32_t UAVObjLoad(UAVObjHandle obj_handle, uint16_t instId)
{
    int32_t rc = 0;
    uint8_t *obj_data;
    int16_t fh;
    char filename[FLASHFS_FILENAME_LEN];

    PIOS_Assert(obj_handle);

    if (UAVObjIsMetaobject(obj_handle)) {

        if (instId != 0)
            return -1;

        obj_data = (uint8_t *)MetaDataPtr((struct UAVOMeta *)obj_handle);

    } else {

        InstanceHandle instEntry = getInstance((struct UAVOData *)obj_handle, instId);

        if (instEntry == NULL)
            return -1;

        obj_data = InstanceData(instEntry);
    }

    UAVObjFilenameCreate(pios_uavo_settings_fs_id, UAVObjGetID(obj_handle), instId, filename);

    fh = PIOS_FLASHFS_Open(pios_uavo_settings_fs_id, filename, PIOS_FLASHFS_RDONLY);

    if (fh < 0)
        return -1;

    if (PIOS_FLASHFS_Read(pios_uavo_settings_fs_id, fh, obj_data, UAVObjGetNumBytes(obj_handle)) != 0)
	    rc = -1;

    PIOS_FLASHFS_Close(pios_uavo_settings_fs_id, fh);

    if (!rc)
        sendEvent((struct UAVOBase *)obj_handle, instId, EV_UNPACKED);

    return rc;
}

/**
 * Delete an object from the file system (SD card).
 * @param[in] obj The object handle.
 * @param[in] instId The object instance
 * @return 0 if success or -1 if failure
 */
int32_t UAVObjDelete(UAVObjHandle obj_handle, uint16_t instId)
{
    char filename[FLASHFS_FILENAME_LEN];

    PIOS_Assert(obj_handle);

    UAVObjFilenameCreate(pios_uavo_settings_fs_id, UAVObjGetID(obj_handle), instId, filename);

    PIOS_FLASHFS_Remove(pios_uavo_settings_fs_id, filename);

    return 0;
}


/**
 * Delete all object from the file system.
 * @return 0 if success or -1 if failure
 */
int32_t UAVObjDeleteAll()
{
    char filename[FLASHFS_FILENAME_LEN];

    UAVObjPrefixGet(pios_uavo_settings_fs_id, filename);

    // Delete settings files from external flash (rm settings*)
    if (PIOS_FLASHFS_Find(pios_uavo_settings_fs_id, filename, UAVO_PREFIX_SIZE, PIOS_FLASHFS_REMOVE) < 0)
		return -1;

    return 0;
}

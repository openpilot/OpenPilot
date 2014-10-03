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
    PIOS_Assert(obj_handle);

    if (UAVObjIsMetaobject(obj_handle)) {
        if (instId != 0) {
            return -1;
        }

        if (PIOS_FLASHFS_ObjSave(pios_uavo_settings_fs_id, UAVObjGetID(obj_handle), instId, (uint8_t *)MetaDataPtr((struct UAVOMeta *)obj_handle), UAVObjGetNumBytes(obj_handle)) != 0) {
            return -1;
        }
    } else {
        InstanceHandle instEntry = getInstance((struct UAVOData *)obj_handle, instId);

        if (instEntry == NULL) {
            return -1;
        }

        if (InstanceData(instEntry) == NULL) {
            return -1;
        }

        if (PIOS_FLASHFS_ObjSave(pios_uavo_settings_fs_id, UAVObjGetID(obj_handle), instId, InstanceData(instEntry), UAVObjGetNumBytes(obj_handle)) != 0) {
            return -1;
        }
    }
    return 0;
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
    PIOS_Assert(obj_handle);

    if (UAVObjIsMetaobject(obj_handle)) {
        if (instId != 0) {
            return -1;
        }

        // Fire event on success
        if (PIOS_FLASHFS_ObjLoad(pios_uavo_settings_fs_id, UAVObjGetID(obj_handle), instId, (uint8_t *)MetaDataPtr((struct UAVOMeta *)obj_handle), UAVObjGetNumBytes(obj_handle)) == 0) {
            sendEvent((struct UAVOBase *)obj_handle, instId, EV_UNPACKED);
        } else {
            return -1;
        }
    } else {
        InstanceHandle instEntry = getInstance((struct UAVOData *)obj_handle, instId);

        if (instEntry == NULL) {
            return -1;
        }

        // Fire event on success
        if (PIOS_FLASHFS_ObjLoad(pios_uavo_settings_fs_id, UAVObjGetID(obj_handle), instId, InstanceData(instEntry), UAVObjGetNumBytes(obj_handle)) == 0) {
            sendEvent((struct UAVOBase *)obj_handle, instId, EV_UNPACKED);
        } else {
            return -1;
        }
    }


    return 0;
}

/**
 * Delete an object from the file system (SD card).
 * @param[in] obj The object handle.
 * @param[in] instId The object instance
 * @return 0 if success or -1 if failure
 */
int32_t UAVObjDelete(UAVObjHandle obj_handle, uint16_t instId)
{
    PIOS_Assert(obj_handle);
    PIOS_FLASHFS_ObjDelete(pios_uavo_settings_fs_id, UAVObjGetID(obj_handle), instId);
    return 0;
}

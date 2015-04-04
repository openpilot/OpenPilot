/**
 ******************************************************************************
 * @addtogroup OpenPilotModules OpenPilot Modules
 * @{
 * @addtogroup LoggingModule Logging Module
 * @brief Features for on board logging
 * @{
 *
 * @file       Logging.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2013.
 * @brief      Logging module, provides features for on board logging
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

// ****************

#include "openpilot.h"
#include "sync.h"
#include "flightstatus.h"

#define FILESYNC_READ_FLAG (PIOS_FS_RDONLY)
#define FILESYNC_WRITE_FLAG (PIOS_FS_CREAT | PIOS_FS_WRONLY | PIOS_FS_APPEND)

// List of the devices supported
extern uintptr_t pios_external_flash_fs_id;
extern uintptr_t pios_internal_flash_fs_id;

// private variables
static SyncData *filesync;

// private functions
static void FileSyncCb(UAVObjEvent *ev);


int32_t FileSyncInitialize(void)
{
    FlightStatusInitialize();
    SyncInitialize();

    filesync = pios_malloc(sizeof(SyncData));
    if (!filesync) {
        return -1;
    }

    return 0;
}

int32_t FileSyncStart(void)
{
    SyncConnectCallback(FileSyncCb);

    return 0;
}
MODULE_INITCALL(FileSyncInitialize, FileSyncStart);


/**
 * @brief File transfer state machine callback.
 * Only available when the following conditions are all met:
 *  - Enabled.
 *  - Not armed.
 */
static void FileSyncCb(__attribute__((unused)) UAVObjEvent *ev)
{
    uint8_t armed, previous_armed = 0;
    int16_t fh;
    int32_t i;
    uint32_t data_size;
    int32_t file_count;
    uint32_t flags = 0;
    uintptr_t pios_fs_filesync_id = 0;
    static int in_progress = 0;

    if (in_progress)
        return;

    in_progress = 1;
    SyncGet(filesync);

    if (filesync->Command == SYNC_COMMAND_IDLE){
        in_progress = 0;
        return;
    }

    filesync->Status = SYNC_STATUS_ERROR;

    if (filesync->Sync == SYNC_SYNC_ENABLED) {
        FlightStatusArmedGet(&armed);
        if (armed == FLIGHTSTATUS_ARMED_DISARMED) {

            /* Use the right device */
            if (filesync->Device == SYNC_DEVICE_EXTERNALFLASH)
                pios_fs_filesync_id = pios_external_flash_fs_id;
            if (filesync->Device == SYNC_DEVICE_INTERNALFLASH)
                pios_fs_filesync_id = pios_internal_flash_fs_id;

            if (pios_fs_filesync_id)
            switch (filesync->Command) {
                /* Download a file from board to host.*/
                case SYNC_COMMAND_DOWNLOAD:
                    fh = PIOS_FS_Open(pios_fs_filesync_id, (char*)filesync->Name, FILESYNC_READ_FLAG);
                    if (fh >= 0) {
                        if (filesync->Offset) {
                            if (PIOS_FS_Lseek(pios_fs_filesync_id, fh, filesync->Offset, PIOS_FS_SEEK_SET) == 0) {
                                if (PIOS_FS_Read(pios_fs_filesync_id, fh, filesync->Data, filesync->DataSize) == filesync->DataSize) {
                                    filesync->Status = SYNC_STATUS_OK;
                                }
                            }
                        }
                        else {
                            if (PIOS_FS_Read(pios_fs_filesync_id, fh, filesync->Data, filesync->DataSize) == filesync->DataSize) {
                                filesync->Status = SYNC_STATUS_OK;
                            }
                        }
                        PIOS_FS_Close(pios_fs_filesync_id, fh);
                    }
                    filesync->Command = SYNC_COMMAND_DOWNLOAD_RESPONSE;
                    break;
                /* UPload a file from host to board. */
                case SYNC_COMMAND_UPLOAD:
                    fh = PIOS_FS_Open(pios_fs_filesync_id, (char*)filesync->Name, FILESYNC_WRITE_FLAG);
                    if (fh >= 0) {
                        if (filesync->Offset) {
                            if (PIOS_FS_Lseek(pios_fs_filesync_id, fh, filesync->Offset, PIOS_FS_SEEK_SET) == 0) {
                                if (PIOS_FS_Write(pios_fs_filesync_id, fh, filesync->Data, filesync->DataSize) == 0) {
                                    filesync->Status = SYNC_STATUS_OK;
                                }
                            }
                        }
                        else {
                            if (PIOS_FS_Write(pios_fs_filesync_id, fh, filesync->Data, filesync->DataSize) == 0) {
                                filesync->Status = SYNC_STATUS_OK;
                            }
                        }
                        PIOS_FS_Close(pios_fs_filesync_id, fh);
                    }
                    filesync->Command = SYNC_COMMAND_UPLOAD_RESPONSE;
                    break;
                /* Get a list of files stored on the board's filesystem. */
                case SYNC_COMMAND_LIST:
                    /* Find out how many file there is on file system */
                    file_count = PIOS_FS_Find(pios_fs_filesync_id, "*", 1, 0, 0);
                    if (file_count >= 0)
                    {
                        /* how much data we have to send? */
                        /* Fill object's data. */
                        int file_size = FS_FILENAME_LEN;
                        int file_info_size = file_size + sizeof(filesync->TotalSize);
                        data_size = file_count * file_info_size;

                        /* we may have less data than requested (will fit into one packet). */
                        if (data_size <= filesync->DataSize) {

                            filesync->DataSize = data_size;
                            filesync->TotalSize = data_size;

                            /* Keep it simple: filter out the "out of sync" */
                            if (filesync->Offset)
                            {
                                filesync->DataSize = 0;
                            }
                        }
                        /* we may have more data than requested, split into multiple objects */
                        else if (data_size > filesync->DataSize)
                        {
                            /* find out if there has been a change on the file system (any writes) */
                            /* for now, simplify to a file count change */
                            if ((filesync->Offset) && (filesync->TotalSize != data_size)) {
                                filesync->DataSize = 0;
                            }
                            else
                            {
                                /* it is the first object */
                                if (filesync->Offset == 0)
                                    filesync->TotalSize = data_size;
                            }
                        }

                        memset(filesync->Data, 0, filesync->DataSize);

                        if (filesync->DataSize) {

                            for (i = (filesync->Offset / file_info_size);
                                 i < ((filesync->Offset + filesync->DataSize) / file_info_size);
                                 i++) {
                                 PIOS_FS_Info(pios_fs_filesync_id,
                                              (char*)&filesync->Data[i*file_info_size],
                                              (uint32_t*)&filesync->Data[i*file_info_size + file_size],
                                              i,
                                              0);
                            }
                        }
                    }
                    else
                    {
                        filesync->DataSize = 0;
                        filesync->TotalSize = 0;
                        filesync->Offset = 0;
                    }
                    filesync->Command = SYNC_COMMAND_LIST_RESPONSE;
                    break;
                /* Delete a file on the board. */
                case SYNC_COMMAND_DELETE:
                    if (!PIOS_FS_Remove(pios_fs_filesync_id, (char*)filesync->Name))
                        filesync->Status = SYNC_STATUS_OK;
                    filesync->Command = SYNC_COMMAND_DELETE_RESPONSE;
                    break;
                /* Format a file system on the board. */
                case SYNC_COMMAND_FORMAT:
                    if (filesync->Device == SYNC_DEVICE_EXTERNALFLASH)
                        flags = PIOS_FS_FORMAT_FLAG_CHIP_ERASE;
                    if (!PIOS_FS_Format(pios_fs_filesync_id, flags))
                        filesync->Status = SYNC_STATUS_OK;
                    filesync->Command = SYNC_COMMAND_FORMAT_RESPONSE;
                    break;
                default:
                    filesync->Command = SYNC_COMMAND_IDLE;
                    /* Command not supporte */
                    break;
            }

        }
        else
        {
            /* House cleaning: from disarmed to armed*/
            if (previous_armed != armed)
            {
                /* Terminate any outstanding commands */
                /* there should be no open file at this point */
                /* any cached file would have already been written back. */
            }
            /* GCS should know that this feature is only available when not armed */
            /* GCS should show a popup to inform user. */
        }
    }
    else
    {
        /* GCS should know that this feature is not enabled (it is in FileSync uavo).*/
        /* GCS should show a popup to inform user. */
    }
    SyncSet(filesync);
    in_progress = 0;
}


/**
 * @}
 * @}
 */

/**
 ******************************************************************************
 * @addtogroup PIOS PIOS Core hardware abstraction layer
 * @{
 * @defgroup   PIOS_DEBUGLOG Flash log debugging Functions
 * @brief Debugging functionality
 * @{
 *
 * @file       pios_debuglog.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2013.
 * @brief      Debugging Functions
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

/* Project Includes */
#include "pios.h"
#include "uavobjectmanager.h"
#include "debuglogentry.h"
#include <flashfsstats.h>

// global definitions


// Global variables
extern uintptr_t pios_user_fs_id; // flash filesystem for logging

#if defined(PIOS_INCLUDE_FREERTOS)
static xSemaphoreHandle mutex = 0;
#define mutexlock()   xSemaphoreTakeRecursive(mutex, portMAX_DELAY)
#define mutexunlock() xSemaphoreGiveRecursive(mutex)
#else
#define mutexlock()
#define mutexunlock()
#endif

static bool logging_enabled = false;
#define MAX_CONSECUTIVE_FAILS_COUNT 10
static bool log_is_full     = false;
static uint8_t fails_count  = 0;
static uint16_t flightnum   = 0;
static uint16_t lognum = 0;
static DebugLogEntryData *buffer = 0;
#if !defined(PIOS_INCLUDE_FREERTOS)
static DebugLogEntryData staticbuffer;
#endif

#define LOG_ENTRY_MAX_DATA_SIZE (sizeof(((DebugLogEntryData *)0)->Data))
#define LOG_ENTRY_HEADER_SIZE   (sizeof(DebugLogEntryData) - LOG_ENTRY_MAX_DATA_SIZE)
// build the obj_id as a DEBUGLOGENTRY ID with least significant byte zeroed and filled with flight number
#define LOG_GET_FLIGHT_OBJID(x) ((DEBUGLOGENTRY_OBJID & ~0xFF) | (x & 0xFF))

#define PIOS_DEBUGLOG_PREFIX_STRING "dlog"
#define PIOS_DEBUGLOG_PREFIX_SIZE 5

static uint32_t used_buffer_space = 0;

/* Private Function Prototypes */
static void enqueue_data(uint32_t objid, uint16_t instid, size_t size, uint8_t *data);
static bool write_current_buffer();

/* This is for testing right now: Need to find a better unique name generator with module as a prefix */
static void PIOS_DEBUGLOG_FilenameCreate(uintptr_t fs_id, uint32_t obj_id, uint16_t obj_inst_id, char *filename)
{
    uint32_t prefix = obj_id + (obj_inst_id / 256) * 16; // put upper 8 bit of instance id into object id modification,
                                                         // skip least sig nibble since that is used for meta object id
    uint8_t suffix  = obj_inst_id & 0xff;

    snprintf((char *)filename, FLASHFS_FILENAME_LEN, PIOS_DEBUGLOG_PREFIX_STRING"%01u/%08X.o%02X", (unsigned)fs_id, (unsigned int)prefix, suffix);
}


/* Get prefix of all uavObj files */
static void PIOS_DEBUGLOG_PrefixGet(uintptr_t fs_id, char *devicename)
{
    snprintf((char *)devicename, FLASHFS_FILENAME_LEN, PIOS_DEBUGLOG_PREFIX_STRING"%01u", (unsigned)fs_id);
}


/**
 * @brief Initialize the log facility
 */
void PIOS_DEBUGLOG_Initialize()
{
	char filename[FLASHFS_FILENAME_LEN];
    FlashFsStatsData flashfs;
    int32_t rc;

#if defined(PIOS_INCLUDE_FREERTOS)
    if (!mutex) {
        mutex  = xSemaphoreCreateRecursiveMutex();
        buffer = pios_malloc(sizeof(DebugLogEntryData));
    }
#else
    buffer = &staticbuffer;
#endif
    if (!buffer) {
        return;
    }
    mutexlock();
    lognum      = 0;
    flightnum   = 0;
    fails_count = 0;
    used_buffer_space = 0;
    log_is_full = false;

    /* Check number of file in the file system */
    PIOS_DEBUGLOG_PrefixGet(pios_user_fs_id, filename);

    /* Get the number of log files present in the file system ("find / -name prefix* | wc -l") */
    rc = PIOS_FLASHFS_Find(pios_user_fs_id, filename, PIOS_DEBUGLOG_PREFIX_SIZE, 0);

    if (rc > 0)
    flightnum = (uint16_t)rc;

	/* update stats */
	FlashFsStatsGet(&flashfs);
	flashfs.DBGlog = flightnum;
	FlashFsStatsSet(&flashfs);

    mutexunlock();
}


/**
 * @brief Enables or Disables logging globally
 * @param[in] enable or disable logging
 */
void PIOS_DEBUGLOG_Enable(uint8_t enabled)
{
    // increase the flight num as soon as logging is disabled
    if (logging_enabled && !enabled) {
        flightnum++;
        lognum = 0;
    }
    logging_enabled = enabled;
}

/**
 * @brief Write a debug log entry with a uavobject
 * @param[in] objectid
 * @param[in] instanceid
 * @param[in] instanceid
 * @param[in] size of object
 * @param[in] data buffer
 */
void PIOS_DEBUGLOG_UAVObject(uint32_t objid, uint16_t instid, size_t size, uint8_t *data)
{
    if (!logging_enabled || !buffer || log_is_full) {
        return;
    }
    mutexlock();

    enqueue_data(objid, instid, size, data);

    mutexunlock();
}
/**
 * @brief Write a debug log entry with text
 * @param[in] format - as in printf
 * @param[in] variable arguments for printf
 * @param...
 */
void PIOS_DEBUGLOG_Printf(char *format, ...)
{
    int16_t fh;
    char filename[FLASHFS_FILENAME_LEN];

    if (!logging_enabled || !buffer || log_is_full) {
        return;
    }

    va_list args;
    va_start(args, format);
    mutexlock();
    // flush any pending buffer before writing debug text
    if (used_buffer_space) {
        write_current_buffer();
    }
    memset(buffer->Data, 0xff, sizeof(buffer->Data));
    vsnprintf((char *)buffer->Data, sizeof(buffer->Data), (char *)format, args);
    buffer->Flight     = flightnum;

    buffer->FlightTime = PIOS_DELAY_GetuS();

    buffer->Entry      = lognum;
    buffer->Type       = DEBUGLOGENTRY_TYPE_TEXT;
    buffer->ObjectID   = 0;
    buffer->InstanceID = 0;
    buffer->Size       = strlen((const char *)buffer->Data);

    PIOS_DEBUGLOG_FilenameCreate(pios_user_fs_id, LOG_GET_FLIGHT_OBJID(flightnum), lognum, filename);

    fh = PIOS_FLASHFS_Open(pios_user_fs_id, filename, PIOS_FLASHFS_CREAT | PIOS_FLASHFS_WRONLY | PIOS_FLASHFS_TRUNC);

    if (fh < 0)
        goto done;

    if (PIOS_FLASHFS_Write(pios_user_fs_id, fh, (uint8_t *)buffer, sizeof(DebugLogEntryData)) != 0)
        goto done;

    if (PIOS_FLASHFS_Close(pios_user_fs_id, fh) != PIOS_FLASHFS_OK)
        goto done;

    lognum++;

done:
    mutexunlock();
}


/**
 * @brief Load one object instance from the filesystem
 * @param[out] buffer where to store the uavobject
 * @param[in] log entry from which flight
 * @param[in] log entry sequence number
 * @return 0 if success or error code
 * @retval -1 if fs_id is not a valid filesystem instance
 * @retval -2 if failed to start transaction
 * @retval -3 if object not found in filesystem
 * @retval -4 if object size in filesystem does not exactly match buffer size
 * @retval -5 if reading the object data from flash fails
 */
int32_t PIOS_DEBUGLOG_Read(void *mybuffer, uint16_t flight, uint16_t inst)
{
    int16_t fh;
    char filename[FLASHFS_FILENAME_LEN];

    PIOS_Assert(mybuffer);

    PIOS_DEBUGLOG_FilenameCreate(pios_user_fs_id, LOG_GET_FLIGHT_OBJID(flight), inst, filename);

    fh = PIOS_FLASHFS_Open(pios_user_fs_id, filename, PIOS_FLASHFS_RDONLY);

    if (fh < 0)
        return -1;

    if (PIOS_FLASHFS_Read(pios_user_fs_id, fh, (uint8_t *)mybuffer, sizeof(DebugLogEntryData)) != 0)
        return -1;

    if (PIOS_FLASHFS_Close(pios_user_fs_id, fh) != PIOS_FLASHFS_OK)
        return -1;

    return 0;
}

/**
 * @brief Retrieve run time info of logging system
 * @param[out] current flight number
 * @param[out] next entry number
 * @param[out] free slots in filesystem
 * @param[out] used slots in filesystem
 */
void PIOS_DEBUGLOG_Info(uint16_t *flight, uint16_t *entry, uint16_t *free, uint16_t *used)
{
    if (flight) {
        *flight = flightnum;
    }
    if (entry) {
        *entry = lognum;
    }

    struct PIOS_FLASHFS_Stats stats;
    PIOS_FLASHFS_GetStats(pios_user_fs_id, &stats);
    if (free) {
        *free = stats.block_free;
    }
    if (used) {
        *used = stats.block_used;
    }
}

/**
 * @brief Delete all debug log from the file system.
 */
void PIOS_DEBUGLOG_DeleteAll(void)
{
    mutexlock();

    char filename[FLASHFS_FILENAME_LEN];
    FlashFsStatsData flashfs;

    PIOS_DEBUGLOG_PrefixGet(pios_user_fs_id, filename);

    PIOS_FLASHFS_Find(pios_user_fs_id, filename, PIOS_DEBUGLOG_PREFIX_SIZE, PIOS_FLASHFS_REMOVE);

    FlashFsStatsGet(&flashfs);
    flashfs.DBGlog = 0;
	FlashFsStatsSet(&flashfs);

    lognum      = 0;
    flightnum   = 0;
    log_is_full = false;
    fails_count = 0;
    used_buffer_space = 0;
    mutexunlock();
}

// Mathieu TODO: We don't need this anymore, use file system cache as a buffer.
void enqueue_data(uint32_t objid, uint16_t instid, size_t size, uint8_t *data)
{
    DebugLogEntryData *entry;

    // start a new block
    if (!used_buffer_space) {
        entry = buffer;
        memset(buffer->Data, 0xff, sizeof(buffer->Data));
        used_buffer_space += size;
    } else {
        // if an instance is being filled and there is enough space, does enqueues new data.
        if (used_buffer_space + size + LOG_ENTRY_HEADER_SIZE > LOG_ENTRY_MAX_DATA_SIZE) {
            buffer->Type = DEBUGLOGENTRY_TYPE_MULTIPLEUAVOBJECTS;
            if (!write_current_buffer()) {
                return;
            }
            entry = buffer;
            memset(buffer->Data, 0xff, sizeof(buffer->Data));
            used_buffer_space += size;
        } else {
            entry = (DebugLogEntryData *)&buffer->Data[used_buffer_space];
            used_buffer_space += size + LOG_ENTRY_HEADER_SIZE;
        }
    }

    entry->Flight     = flightnum;
    entry->FlightTime = PIOS_DELAY_GetuS();
    entry->Entry = lognum;
    entry->Type = DEBUGLOGENTRY_TYPE_UAVOBJECT;
    entry->ObjectID   = objid;
    entry->InstanceID = instid;
    if (size > sizeof(buffer->Data)) {
        size = sizeof(buffer->Data);
    }
    entry->Size = size;

    memcpy(entry->Data, data, size);
}

bool write_current_buffer()
{
    int16_t fh;
    char filename[FLASHFS_FILENAME_LEN];
    FlashFsStatsData flashfs;

    PIOS_DEBUGLOG_FilenameCreate(pios_user_fs_id, LOG_GET_FLIGHT_OBJID(flightnum), lognum, filename);

    fh = PIOS_FLASHFS_Open(pios_user_fs_id, filename, PIOS_FLASHFS_CREAT | PIOS_FLASHFS_WRONLY | PIOS_FLASHFS_TRUNC);

    if (fh < 0)
        goto err;

    if (PIOS_FLASHFS_Write(pios_user_fs_id, fh, (uint8_t *)buffer, sizeof(DebugLogEntryData)) != 0)
        goto err;

    if (PIOS_FLASHFS_Close(pios_user_fs_id, fh) != PIOS_FLASHFS_OK)
        goto err;

    // not enough space, write the block and start a new one
	lognum++;
	fails_count = 0;
	used_buffer_space = 0;
	goto done;
err:
	if (fails_count++ > MAX_CONSECUTIVE_FAILS_COUNT) {
		log_is_full = true;
	}
    return false;
done:
	FlashFsStatsGet(&flashfs);
	flashfs.DBGlog++;
	FlashFsStatsSet(&flashfs);
    return true;
}
/**
 * @}
 * @}
 */

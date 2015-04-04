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

// global definitions


// Global variables
extern uintptr_t pios_log_fs_id; // flash filesystem for logging

#if defined(PIOS_INCLUDE_FREERTOS)
static xSemaphoreHandle mutex = 0;
#define mutexlock()   xSemaphoreTakeRecursive(mutex, portMAX_DELAY)
#define mutexunlock() xSemaphoreGiveRecursive(mutex)
#else
#define mutexlock()
#define mutexunlock()
#endif

static bool logging_enabled = false;

static uint16_t flightnum = 0;
static uint16_t lognum = 0;
static int16_t fh = -1;
static char filename[FS_FILENAME_LEN];
static DebugLogEntryData *buffer = 0;
#if !defined(PIOS_INCLUDE_FREERTOS)
static DebugLogEntryData staticbuffer;
#endif

#define LOG_ENTRY_MAX_DATA_SIZE (sizeof(((DebugLogEntryData *)0)->Data))
#define LOG_ENTRY_HEADER_SIZE   (sizeof(DebugLogEntryData) - LOG_ENTRY_MAX_DATA_SIZE)
// build the obj_id as a DEBUGLOGENTRY ID with least significant byte zeroed and filled with flight number
#define LOG_GET_FLIGHT_OBJID(x) ((DEBUGLOGENTRY_OBJID & ~0xFF) | (x & 0xFF))
#define PIOS_DEBUGLOG_EXT_STRING "dlog"
#define PIOS_DEBUGLOG_PATTERN_SIZE 4
#define PIOS_DEBUGLOG_PATTERN_OFFSET 9


/* This is for testing right now: Need to find a better unique name generator with module as a prefix */
static void PIOS_DEBUGLOG_FilenameCreate(uint32_t obj_id, uint16_t obj_inst_id, char *tmp_filename)
{
    snprintf((char *)tmp_filename, FS_FILENAME_LEN, "%08X-%02X."PIOS_DEBUGLOG_EXT_STRING, (unsigned)obj_id, obj_inst_id & 0xff);
}


/**
 * @brief Append one log entry to the log file.
 */
static void PIOS_DEBUGLOG_Add(uint32_t objid, uint16_t instid, size_t size, uint8_t *data, uint8_t type)
{
    buffer->Flight = flightnum;
    buffer->FlightTime = PIOS_DELAY_GetuS();
    buffer->Entry = lognum++;
    buffer->Type = type;
    buffer->ObjectID = objid;
    buffer->InstanceID = instid;
    if (size > sizeof(buffer->Data)) {
        size = sizeof(buffer->Data);
    }
    buffer->Size = size;

    if (type != DEBUGLOGENTRY_TYPE_TEXT)
        memcpy(buffer->Data, data, size);

    if (fh < 0)
        return;

    PIOS_FS_Write(pios_log_fs_id, fh, (uint8_t*)buffer, (uint16_t)(LOG_ENTRY_HEADER_SIZE + size));
}


/**
 * @brief Initialize the log facility
 */
void PIOS_DEBUGLOG_Initialize()
{

    int32_t rc;

#if defined(PIOS_INCLUDE_FREERTOS)
    if (!mutex) {
        mutex  = xSemaphoreCreateRecursiveMutex();
        buffer = pios_malloc(sizeof(DebugLogEntryData));
    }
#else
    buffer = &staticbuffer;
#endif
    if (!buffer)
        return;

    mutexlock();

    lognum = 0;
    flightnum = 0;

    /* Get the number of log files present in the file system ("find / -name prefix* | wc -l") */
    rc = PIOS_FS_Find(pios_log_fs_id, PIOS_DEBUGLOG_EXT_STRING, PIOS_DEBUGLOG_PATTERN_SIZE, PIOS_DEBUGLOG_PATTERN_OFFSET, 0);

    if (rc > 0)
        flightnum = (uint16_t)rc;

    mutexunlock();
}


/**
 * @brief Enables or Disables logging globally
 * @param[in] enable or disable logging
 */
void PIOS_DEBUGLOG_Enable(uint8_t enabled)
{
    mutexlock();

    // Stop log.
    if (logging_enabled && !enabled) {
        // Close log file, flush cached file chunks.
        PIOS_FS_Close(pios_log_fs_id, fh);
    }

    // Start log.
    if (!logging_enabled && enabled) {
        lognum = 0;
        flightnum++;

        PIOS_FS_Close(pios_log_fs_id, fh);

        // Create a new log file.
        PIOS_DEBUGLOG_FilenameCreate(LOG_GET_FLIGHT_OBJID(flightnum), lognum, filename);

        fh = PIOS_FS_Open(pios_log_fs_id, filename, PIOS_FS_CREAT | PIOS_FS_WRONLY | PIOS_FS_TRUNC);
    }

    // Update flag.
    logging_enabled = enabled;

    mutexunlock();
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
    if (!logging_enabled || !buffer) {
        return;
    }
    mutexlock();

    PIOS_DEBUGLOG_Add(objid, instid, size, data, DEBUGLOGENTRY_TYPE_UAVOBJECT);

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
    if (!logging_enabled || !buffer) {
        return;
    }

    va_list args;
    va_start(args, format);

    mutexlock();

    memset(buffer->Data, 0, sizeof(buffer->Data));
    vsnprintf((char *)buffer->Data, sizeof(buffer->Data) - 1, (char *)format, args);

    /* Add a character (NULL) for string termination. */
    PIOS_DEBUGLOG_Add(0, 0, strlen((const char *)buffer->Data) + 1, buffer->Data, DEBUGLOGENTRY_TYPE_TEXT);

    mutexunlock();
}


/**
 * @brief Read one entry of a log.
 * @param[out] buffer where to store the entry
 * @param[in] log entry from which flight
 * @param[in] log entry sequence number
 * @return 0 if success or error code
 * @retval -1 if the entry could not get retrieved

 */
int32_t PIOS_DEBUGLOG_Read(void *mybuffer, uint16_t flight, __attribute__((unused)) uint16_t inst)
{
    int32_t rc = 0;
	static uint16_t current_flight_opened = 0xFFFF;

    PIOS_Assert(mybuffer);

    // TODO: This only does incremental read, no random.

    mutexlock();

    if (current_flight_opened != flight)
    {


        PIOS_FS_Close(pios_log_fs_id, fh);

        PIOS_DEBUGLOG_FilenameCreate(LOG_GET_FLIGHT_OBJID(flight), 0, filename);

        if ((fh = PIOS_FS_Open(pios_log_fs_id, filename, PIOS_FS_RDONLY)) < 0) {
            mutexunlock();
            return -1;
        }

        current_flight_opened = flight;
    }

    if (PIOS_FS_Read(pios_log_fs_id, fh, (uint8_t *)mybuffer, LOG_ENTRY_HEADER_SIZE) != LOG_ENTRY_HEADER_SIZE) {
        rc = -1;
    }
    else
    {
        if (PIOS_FS_Read(pios_log_fs_id, fh, (uint8_t *)(mybuffer + LOG_ENTRY_HEADER_SIZE), ((DebugLogEntryData*)mybuffer)->Size) != ((DebugLogEntryData*)mybuffer)->Size) {
            rc = -1;
        }
    }

    if (rc)
        PIOS_FS_Close(pios_log_fs_id, fh);

    mutexunlock();
    return rc;
}


/**
 * @brief Retrieve run time info of logging system
 * @param[out] current flight number
 * @param[out] next entry number
 * @param[out] free slots in filesystem
 * @param[out] used slots in filesystem
 */
void PIOS_DEBUGLOG_Info(uint16_t *flight, uint16_t *entry)
{
    if (flight) {
        *flight = flightnum;
    }
    if (entry) {
        *entry = lognum;
    }
}


/**
 * @brief Delete all debug log from the file system.
 */
void PIOS_DEBUGLOG_DeleteAll(void)
{
    mutexlock();

    PIOS_FS_Find(pios_log_fs_id, PIOS_DEBUGLOG_EXT_STRING, PIOS_DEBUGLOG_PATTERN_SIZE, PIOS_DEBUGLOG_PATTERN_OFFSET, PIOS_FS_REMOVE);

    lognum = 0;
    flightnum = 0;

    mutexunlock();
}


/**
 * @}
 * @}
 */

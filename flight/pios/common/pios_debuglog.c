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
static uint16_t flightnum   = 0;
static uint16_t lognum = 0;
static DebugLogEntryData *buffer = 0;
#if !defined(PIOS_INCLUDE_FREERTOS)
static DebugLogEntryData staticbuffer;
#endif

/* Private Function Prototypes */

/**
 * @brief Initialize the log facility
 */
void PIOS_DEBUGLOG_Initialize()
{
#if defined(PIOS_INCLUDE_FREERTOS)
    if (!mutex) {
        mutex  = xSemaphoreCreateRecursiveMutex();
        buffer = pvPortMalloc(sizeof(DebugLogEntryData));
    }
#else
    buffer = &staticbuffer;
#endif
    if (!buffer) {
        return;
    }
    mutexlock();
    lognum    = 0;
    flightnum = 0;
    while (PIOS_FLASHFS_ObjLoad(pios_user_fs_id, flightnum * 256, lognum, (uint8_t *)buffer, sizeof(DebugLogEntryData)) == 0) {
        flightnum++;
    }
    mutexunlock();
}


/**
 * @brief Enables or Disables logging globally
 * @param[in] enable or disable logging
 */
void PIOS_DEBUGLOG_Enable(uint8_t enabled)
{
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
    if (!logging_enabled || !buffer) {
        return;
    }
    mutexlock();
    buffer->Flight     = flightnum;
#if defined(PIOS_INCLUDE_FREERTOS)
    buffer->FlightTime = xTaskGetTickCount() * portTICK_RATE_MS;
#else
    buffer->FlightTime = 0;
#endif
    buffer->Entry = lognum;
    buffer->Type = DEBUGLOGENTRY_TYPE_UAVOBJECT;
    buffer->ObjectID   = objid;
    buffer->InstanceID = instid;
    if (size > sizeof(buffer->Data)) {
        size = sizeof(buffer->Data);
    }
    buffer->Size = size;
    memset(buffer->Data, 0, sizeof(buffer->Data));
    memcpy(buffer->Data, data, size);

    if (PIOS_FLASHFS_ObjSave(pios_user_fs_id, flightnum * 256, lognum, (uint8_t *)buffer, sizeof(DebugLogEntryData)) == 0) {
        lognum++;
    }
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
    vsnprintf((char *)buffer->Data, sizeof(buffer->Data), (char *)format, args);
    buffer->Flight     = flightnum;
#if defined(PIOS_INCLUDE_FREERTOS)
    buffer->FlightTime = xTaskGetTickCount() * portTICK_RATE_MS;
#else
    buffer->FlightTime = 0;
#endif
    buffer->Entry      = lognum;
    buffer->Type       = DEBUGLOGENTRY_TYPE_TEXT;
    buffer->ObjectID   = 0;
    buffer->InstanceID = 0;
    buffer->Size       = strlen((const char *)buffer->Data);

    if (PIOS_FLASHFS_ObjSave(pios_user_fs_id, flightnum * 256, lognum, (uint8_t *)buffer, sizeof(DebugLogEntryData)) == 0) {
        lognum++;
    }
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
    PIOS_Assert(mybuffer);
    return PIOS_FLASHFS_ObjLoad(pios_user_fs_id, flight * 256, inst, (uint8_t *)mybuffer, sizeof(DebugLogEntryData));
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
    struct PIOS_FLASHFS_Stats stats = { 0, 0 };
    PIOS_FLASHFS_GetStats(pios_user_fs_id, &stats);
    if (free) {
        *free = stats.num_free_slots;
    }
    if (used) {
        *used = stats.num_active_slots;
    }
}

/**
 * @brief Format entire flash memory!!!
 */
void PIOS_DEBUGLOG_Format(void)
{
    mutexlock();
    PIOS_FLASHFS_Format(pios_user_fs_id);
    lognum    = 0;
    flightnum = 0;
    mutexunlock();
}

/**
 * @}
 * @}
 */

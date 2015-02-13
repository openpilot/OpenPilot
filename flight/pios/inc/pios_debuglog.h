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

#ifndef PIOS_DEBUGLOG_H
#define PIOS_DEBUGLOG_H


/**
 * @brief Initialize the log facility
 */
void PIOS_DEBUGLOG_Initialize();

/**
 * @brief Enables or Disables logging globally
 * @param[in] enable or disable logging
 */
void PIOS_DEBUGLOG_Enable(uint8_t enabled);

/**
 * @brief Write a debug log entry with a uavobject
 * @param[in] objectid
 * @param[in] instanceid
 * @param[in] instanceid
 * @param[in] size of object
 * @param[in] data buffer
 */
void PIOS_DEBUGLOG_UAVObject(uint32_t objid, uint16_t instid, size_t size, uint8_t *data);

/**
 * @brief Write a debug log entry with text
 * @param[in] format - as in printf
 * @param[in] variable arguments for printf
 * @param...
 */
void PIOS_DEBUGLOG_Printf(char *format, ...);

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
int32_t PIOS_DEBUGLOG_Read(void *buffer, uint16_t flight, uint16_t inst);

/**
 * @brief Retrieve run time info of logging system
 * @param[out] current flight number
 * @param[out] next entry number
 */
void PIOS_DEBUGLOG_Info(uint16_t *flight, uint16_t *entry);

/**
 * @brief Format entire flash memory!!!
 */
void PIOS_DEBUGLOG_DeleteAll(void);

#endif // ifndef PIOS_DEBUGLOG_H

/**
 * @}
 * @}
 */

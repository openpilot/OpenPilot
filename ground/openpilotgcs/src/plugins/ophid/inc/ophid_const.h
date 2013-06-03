/**
 ******************************************************************************
 *
 * @file       ophid_const.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup RawHIDPlugin Raw HID Plugin
 * @{
 * @brief Impliments a HID USB connection to the flight hardware as a QIODevice
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

#ifndef OPHID_CONST_H
#define OPHID_CONST_H

#define printf         qDebug
#define OPHID_DEBUG_ON 1
#ifdef OPHID_DEBUG_ON
#define OPHID_DEBUG(fmt, args ...)   qDebug("[DEBUG] "fmt,##args)
#define OPHID_TRACE(fmt, args ...)   qDebug("[TRACE] %s:%s:%d: "fmt, __FILE__, __func__, __LINE__,##args)
#define OPHID_ERROR(fmt, args ...)   qDebug("[ERROR] %s:%s:%d: "fmt, __FILE__, __func__, __LINE__,##args)
#define OPHID_WARNING(fmt, args ...) qDebug("[WARNING] "fmt,##args)
#else
#define OPHID_DEBUG(fmt, args ...)
#define OPHID_TRACE(fmt, args ...)
#define OPHID_ERROR(fmt, args ...)
#define OPHID_WARNING(fmt, args ...)
#endif


// USB
#define USB_MAX_DEVICES         10
#define USB_VID                 0x20A0
#define USB_PID                 0x4117
#define USB_USAGE_PAGE          0xFF9C
#define USB_USAGE               0x0001
#define USB_DEV_SERIAL_LEN      24
#define USB_PID_ANY             -1
#define USB_MAX_STRING_SIZE     255

// ERROR
#define OPHID_NO_ERROR          0
#define OPHID_ERROR_RET         -1
#define OPHID_ERROR_POINTER     -2
#define OPHID_ERROR_PARAMETER   -3
#define OPHID_ERROR_HANDLE      -4
#define OPHID_ERROR_INIT        -5
#define OPHID_ERROR_ENUMERATION -6

#endif // OPHID_CONST_H

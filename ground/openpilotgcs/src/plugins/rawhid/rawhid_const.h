/**
 ******************************************************************************
 *
 * @file       rawhid_const.h
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

#ifndef RAWHID_CONST_H
#define RAWHID_CONST_H

static const int USB_MAX_DEVICES    = 10;

static const int USB_VID            = 0x20A0;
static const int USB_PID            = 0x4117;

static const int USB_USAGE_PAGE     = 0xFF9C;
static const int USB_USAGE          = 0x0001;

static const int USB_DEV_SERIAL_LEN = 24;

#endif // RAWHID_CONST_H

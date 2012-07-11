/**
 ******************************************************************************
 * @addtogroup PIOS PIOS Core hardware abstraction layer
 * @{
 * @addtogroup PIOS_USB_BOARD Board specific USB definitions
 * @brief Board specific USB definitions
 * @{
 *
 * @file       pios_usb_board_data.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      Board specific USB definitions
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

#ifndef PIOS_USB_BOARD_DATA_H
#define PIOS_USB_BOARD_DATA_H

#define PIOS_USB_BOARD_HID_DATA_LENGTH 64

#define PIOS_USB_BOARD_EP_NUM 2

#include "pios_usb_defs.h" 	/* struct usb_* */

#define PIOS_USB_BOARD_PRODUCT_ID USB_PRODUCT_ID_PIPXTREME
#define PIOS_USB_BOARD_DEVICE_VER USB_OP_DEVICE_VER(USB_OP_BOARD_ID_PIPXTREME, USB_OP_BOARD_MODE_BL)
#define PIOS_USB_BOARD_SN_SUFFIX "+BL"

/*
 * The bootloader uses a simplified report structure
 *   BL: <REPORT_ID><DATA>...<DATA>
 *   FW: <REPORT_ID><LENGTH><DATA>...<DATA>
 * This define changes the behaviour in pios_usb_hid.c
 */
#define PIOS_USB_BOARD_BL_HID_HAS_NO_LENGTH_BYTE

#endif	/* PIOS_USB_BOARD_DATA_H */

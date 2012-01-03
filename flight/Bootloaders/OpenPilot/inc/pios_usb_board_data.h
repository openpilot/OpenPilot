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

struct usb_board_config {
	struct usb_configuration_desc         config;
	struct usb_interface_desc             hid_if;
	struct usb_hid_desc                   hid;
	struct usb_endpoint_desc              hid_in;
	struct usb_endpoint_desc              hid_out;
} __attribute__((packed));

extern const struct usb_device_desc PIOS_USB_BOARD_DeviceDescriptor;
extern const struct usb_board_config PIOS_USB_BOARD_Configuration;
extern const struct usb_string_langid PIOS_USB_BOARD_StringLangID;

/* NOTE NOTE NOTE
 *
 * Care must be taken to ensure that the _actual_ contents of
 * these arrays (in each board's pios_usb_board_data.c) is no
 * smaller than the stated sizes here or these descriptors 
 * will end up with trailing zeros on the wire.
 *
 * The compiler will catch any time that these definitions are
 * too small.
 */
extern const uint8_t PIOS_USB_BOARD_HidReportDescriptor[36];
extern const uint8_t PIOS_USB_BOARD_StringVendorID[28];
extern const uint8_t PIOS_USB_BOARD_StringProductID[20];
extern uint8_t PIOS_USB_BOARD_StringSerial[52];

#define PIOS_USB_BOARD_PRODUCT_ID USB_PRODUCT_ID_OPENPILOT_MAIN
#define PIOS_USB_BOARD_DEVICE_VER USB_OP_DEVICE_VER(USB_OP_BOARD_ID_OPENPILOT_MAIN, USB_OP_BOARD_MODE_BL)

/*
 * The bootloader uses a simplified report structure
 *   BL: <REPORT_ID><DATA>...<DATA>
 *   FW: <REPORT_ID><LENGTH><DATA>...<DATA>
 * This define changes the behaviour in pios_usb_hid.c
 */
#define PIOS_USB_BOARD_BL_HID_HAS_NO_LENGTH_BYTE

#endif	/* PIOS_USB_BOARD_DATA_H */

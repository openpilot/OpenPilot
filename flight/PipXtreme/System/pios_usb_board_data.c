/**
 ******************************************************************************
 * @addtogroup PIOS PIOS Core hardware abstraction layer
 * @{
 * @addtogroup PIOS_USB_BOARD Board specific USB definitions
 * @brief Board specific USB definitions
 * @{
 *
 * @file       pios_usb_board_data.c
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

#include "pios_usb_board_data.h" /* struct usb_*, USB_* */
#include "pios_sys.h"		 /* PIOS_SYS_SerialNumberGet */
#include "pios_usbhook.h"	 /* PIOS_USBHOOK_* */
#include "pios_usb_util.h"	 /* PIOS_USB_UTIL_AsciiToUtf8 */

static const uint8_t usb_product_id[20] = {
	sizeof(usb_product_id),
	USB_DESC_TYPE_STRING,
	'P', 0,
	'i', 0,
	'p', 0,
	'X', 0,
	't', 0,
	'r', 0,
	'e', 0,
	'm', 0,
	'e', 0,
};

static uint8_t usb_serial_number[2 + PIOS_SYS_SERIAL_NUM_ASCII_LEN*2 + (sizeof(PIOS_USB_BOARD_SN_SUFFIX)-1)*2] = {
	sizeof(usb_serial_number),
	USB_DESC_TYPE_STRING,
};

static const struct usb_string_langid usb_lang_id = {
	.bLength = sizeof(usb_lang_id),
	.bDescriptorType = USB_DESC_TYPE_STRING,
	.bLangID = htousbs(USB_LANGID_ENGLISH_US),
};

static const uint8_t usb_vendor_id[28] = {
	sizeof(usb_vendor_id),
	USB_DESC_TYPE_STRING,
	'o', 0,
	'p', 0,
	'e', 0,
	'n', 0,
	'p', 0,
	'i', 0,
	'l', 0,
	'o', 0,
	't', 0,
	'.', 0,
	'o', 0,
	'r', 0,
	'g', 0
};

int32_t PIOS_USB_BOARD_DATA_Init(void)
{
	/* Load device serial number into serial number string */
	uint8_t sn[PIOS_SYS_SERIAL_NUM_ASCII_LEN + 1];
	PIOS_SYS_SerialNumberGet((char *)sn);

	/* Concatenate the device serial number and the appropriate suffix ("+BL" or "+FW") into the USB serial number */
	uint8_t * utf8 = &(usb_serial_number[2]);
	utf8 = PIOS_USB_UTIL_AsciiToUtf8(utf8, sn, PIOS_SYS_SERIAL_NUM_ASCII_LEN);
	utf8 = PIOS_USB_UTIL_AsciiToUtf8(utf8, (uint8_t *)PIOS_USB_BOARD_SN_SUFFIX, sizeof(PIOS_USB_BOARD_SN_SUFFIX)-1);

	PIOS_USBHOOK_RegisterString(USB_STRING_DESC_PRODUCT, (uint8_t *)&usb_product_id, sizeof(usb_product_id));
	PIOS_USBHOOK_RegisterString(USB_STRING_DESC_SERIAL, (uint8_t *)&usb_serial_number, sizeof(usb_serial_number));

	PIOS_USBHOOK_RegisterString(USB_STRING_DESC_LANG, (uint8_t *)&usb_lang_id, sizeof(usb_lang_id));
	PIOS_USBHOOK_RegisterString(USB_STRING_DESC_VENDOR, (uint8_t *)&usb_vendor_id, sizeof(usb_vendor_id));

	return 0;
}

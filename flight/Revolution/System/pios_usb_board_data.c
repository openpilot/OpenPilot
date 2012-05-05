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

static const uint8_t usb_product_id[22] = {
	sizeof(usb_product_id),
	USB_DESC_TYPE_STRING,
	'R', 0,
	'e', 0,
	'v', 0,
	'o', 0,
	'l', 0,
	'u', 0,
	't', 0,
	'i', 0,
	'o', 0,
	'n', 0,
};

static uint8_t usb_serial_number[52] = {
	sizeof(usb_serial_number),
	USB_DESC_TYPE_STRING,
	0, 0,
	0, 0,
	0, 0,
	0, 0,
	0, 0,
	0, 0,
	0, 0,
	0, 0,
	0, 0,
	0, 0,
	0, 0,
	0, 0,
	0, 0,
	0, 0,
	0, 0,
	0, 0,
	0, 0,
	0, 0,
	0, 0,
	0, 0,
	0, 0,
	0, 0,
	0, 0,
	0, 0,
	0, 0
};

static const struct usb_string_langid usb_lang_id = {
	.bLength = sizeof(usb_lang_id),
	.bDescriptorType = USB_DESC_TYPE_STRING,
	.bLangID = htousbs(USB_LANGID_ENGLISH_UK),
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
	uint8_t sn[25];
	PIOS_SYS_SerialNumberGet((char *)sn);
	for (uint8_t i = 0; sn[i] != '\0' && (2 * i) < usb_serial_number[0]; i++) {
		usb_serial_number[2 + 2 * i] = sn[i];
	}

	PIOS_USBHOOK_RegisterString(USB_STRING_DESC_PRODUCT, (uint8_t *)&usb_product_id, sizeof(usb_product_id));
	PIOS_USBHOOK_RegisterString(USB_STRING_DESC_SERIAL, (uint8_t *)&usb_serial_number, sizeof(usb_serial_number));

	PIOS_USBHOOK_RegisterString(USB_STRING_DESC_LANG, (uint8_t *)&usb_lang_id, sizeof(usb_lang_id));
	PIOS_USBHOOK_RegisterString(USB_STRING_DESC_VENDOR, (uint8_t *)&usb_vendor_id, sizeof(usb_vendor_id));

	return 0;
}

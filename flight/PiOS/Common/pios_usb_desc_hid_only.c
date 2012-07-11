/**
 ******************************************************************************
 * @addtogroup PIOS PIOS Core hardware abstraction layer
 * @{
 * @addtogroup PIOS_USB_DESC USB Descriptor definitions
 * @brief USB Descriptor definitions for HID only
 * @{
 *
 * @file       pios_usb_desc_hid_only.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      USB Descriptor definitions for HID only
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

#include "pios_usb_desc_hid_only_priv.h" /* exported API */
#include "pios_usb_defs.h"		 /* struct usb_*, USB_* */
#include "pios_usb_board_data.h"	 /* PIOS_USB_BOARD_* */
#include "pios_usbhook.h"		 /* PIOS_USBHOOK_Register* */
#include "pios_usb_hid.h"		 /* PIOS_USB_HID_Register* */

static const struct usb_device_desc device_desc = {
	.bLength            = sizeof(struct usb_device_desc),
	.bDescriptorType    = USB_DESC_TYPE_DEVICE,
	.bcdUSB             = htousbs(0x0200),
	.bDeviceClass       = 0x00,
	.bDeviceSubClass    = 0x00,
	.bDeviceProtocol    = 0x00,
	.bMaxPacketSize0    = 64, /* Must be 64 for high-speed devices */
	.idVendor           = htousbs(USB_VENDOR_ID_OPENPILOT),
	.idProduct          = htousbs(PIOS_USB_BOARD_PRODUCT_ID),
	.bcdDevice          = htousbs(PIOS_USB_BOARD_DEVICE_VER),
	.iManufacturer      = 1,
	.iProduct           = 2,
	.iSerialNumber      = 3,
	.bNumConfigurations = 1,
};

static const uint8_t hid_report_desc[36] = {
	HID_GLOBAL_ITEM_2 (HID_TAG_GLOBAL_USAGE_PAGE),
	0x9C, 0xFF,		/* Usage Page 0xFF9C (Vendor Defined) */
	HID_LOCAL_ITEM_1  (HID_TAG_LOCAL_USAGE),
	0x01,			/* Usage ID 0x0001 (0x01-0x1F uaually for top-level collections) */

	HID_MAIN_ITEM_1   (HID_TAG_MAIN_COLLECTION),
	0x01,			/* Application */

	/* Device -> Host emulated serial channel */
	HID_GLOBAL_ITEM_1 (HID_TAG_GLOBAL_REPORT_ID),
	0x01,		        /* OpenPilot emulated serial channel (Device -> Host) */
	HID_LOCAL_ITEM_1  (HID_TAG_LOCAL_USAGE),
	0x02,
	HID_GLOBAL_ITEM_1 (HID_TAG_GLOBAL_LOGICAL_MIN),
	0x00,			/* Values range from min = 0x00 */
	HID_GLOBAL_ITEM_1 (HID_TAG_GLOBAL_LOGICAL_MAX),
	0xFF,			/* Values range to max = 0xFF */
	HID_GLOBAL_ITEM_1 (HID_TAG_GLOBAL_REPORT_SIZE),
	0x08,			/* 8 bits wide */
	HID_GLOBAL_ITEM_1 (HID_TAG_GLOBAL_REPORT_CNT),
	PIOS_USB_BOARD_HID_DATA_LENGTH-1, /* Need to leave room for a report ID */
	HID_MAIN_ITEM_1 (HID_TAG_MAIN_INPUT),
	0x03,			/* Variable, Constant (read-only) */

	/* Host -> Device emulated serial channel */
	HID_GLOBAL_ITEM_1 (HID_TAG_GLOBAL_REPORT_ID),
	0x02,                   /* OpenPilot emulated Serial Channel (Host -> Device) */
	HID_LOCAL_ITEM_1  (HID_TAG_LOCAL_USAGE),
	0x02,
	HID_GLOBAL_ITEM_1 (HID_TAG_GLOBAL_LOGICAL_MIN),
	0x00,			/* Values range from min = 0x00 */
	HID_GLOBAL_ITEM_1 (HID_TAG_GLOBAL_LOGICAL_MAX),
	0xFF,			/* Values range to max = 0xFF */
	HID_GLOBAL_ITEM_1 (HID_TAG_GLOBAL_REPORT_SIZE),
	0x08,			/* 8 bits wide */
	HID_GLOBAL_ITEM_1 (HID_TAG_GLOBAL_REPORT_CNT),
	PIOS_USB_BOARD_HID_DATA_LENGTH-1, /* Need to leave room for a report ID */
	HID_MAIN_ITEM_1 (HID_TAG_MAIN_OUTPUT),
	0x82,			/* Volatile, Variable */

	HID_MAIN_ITEM_0 (HID_TAG_MAIN_ENDCOLLECTION),
};

struct usb_config_hid_only {
	struct usb_configuration_desc         config;
	struct usb_interface_desc             hid_if;
	struct usb_hid_desc                   hid;
	struct usb_endpoint_desc              hid_in;
	struct usb_endpoint_desc              hid_out;
} __attribute__((packed));

const struct usb_config_hid_only config_hid_only = {
	.config = {
		.bLength              = sizeof(struct usb_configuration_desc),
		.bDescriptorType      = USB_DESC_TYPE_CONFIGURATION,
		.wTotalLength         = htousbs(sizeof(struct usb_config_hid_only)),
		.bNumInterfaces       = 1,
		.bConfigurationValue  = 1,
		.iConfiguration       = 0,
		.bmAttributes         = 0xC0,
		.bMaxPower            = 250/2, /* in units of 2ma */
	},
	.hid_if = {
		.bLength              = sizeof(struct usb_interface_desc),
		.bDescriptorType      = USB_DESC_TYPE_INTERFACE,
		.bInterfaceNumber     = 0,
		.bAlternateSetting    = 0,
		.bNumEndpoints        = 2,
		.bInterfaceClass      = USB_INTERFACE_CLASS_HID,
		.bInterfaceSubClass   = 0, /* no boot */
		.nInterfaceProtocol   = 0, /* none */
		.iInterface           = 0,
	},
	.hid = {
		.bLength = sizeof(struct usb_hid_desc),
		.bDescriptorType      = USB_DESC_TYPE_HID,
		.bcdHID               = htousbs(0x0110),
		.bCountryCode         = 0,
		.bNumDescriptors      = 1,
		.bClassDescriptorType = USB_DESC_TYPE_REPORT,
		.wItemLength          = htousbs(sizeof(hid_report_desc)),
	},
	.hid_in = {
		.bLength              = sizeof(struct usb_endpoint_desc),
		.bDescriptorType      = USB_DESC_TYPE_ENDPOINT,
		.bEndpointAddress     = USB_EP_IN(1),
		.bmAttributes         = USB_EP_ATTR_TT_INTERRUPT,
		.wMaxPacketSize       = htousbs(PIOS_USB_BOARD_HID_DATA_LENGTH),
		.bInterval            = 4, /* ms */
	},
	.hid_out = {
		.bLength              = sizeof(struct usb_endpoint_desc),
		.bDescriptorType      = USB_DESC_TYPE_ENDPOINT,
		.bEndpointAddress     = USB_EP_OUT(1),
		.bmAttributes         = USB_EP_ATTR_TT_INTERRUPT,
		.wMaxPacketSize       = htousbs(PIOS_USB_BOARD_HID_DATA_LENGTH),
		.bInterval            = 4, /* ms */
	},
};

int32_t PIOS_USB_DESC_HID_ONLY_Init(void)
{
	PIOS_USBHOOK_RegisterConfig(1, (uint8_t *)&config_hid_only, sizeof(config_hid_only));

	PIOS_USBHOOK_RegisterDevice((uint8_t *)&device_desc, sizeof(device_desc));

	PIOS_USB_HID_RegisterHidDescriptor((uint8_t *)&(config_hid_only.hid), sizeof(config_hid_only.hid));
	PIOS_USB_HID_RegisterHidReport((uint8_t *)hid_report_desc, sizeof(hid_report_desc));

	return 0;
}

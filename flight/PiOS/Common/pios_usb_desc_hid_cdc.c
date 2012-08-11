/**
 ******************************************************************************
 * @addtogroup PIOS PIOS Core hardware abstraction layer
 * @{
 * @addtogroup PIOS_USB_DESC USB Descriptor definitions
 * @brief USB Descriptor definitions for HID and CDC
 * @{
 *
 * @file       pios_usb_desc_hid_cdc.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      USB Descriptor definitions for HID and CDC
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

#include "pios_usb_desc_hid_cdc_priv.h" /* exported API */
#include "pios_usb_defs.h"		/* struct usb_*, USB_* */
#include "pios_usb_board_data.h"	/* PIOS_USB_BOARD_* */
#include "pios_usbhook.h"		/* PIOS_USBHOOK_Register* */
#include "pios_usb_hid.h"		/* PIOS_USB_HID_Register* */

static const struct usb_device_desc device_desc = {
	.bLength            = sizeof(struct usb_device_desc),
	.bDescriptorType    = USB_DESC_TYPE_DEVICE,
	.bcdUSB             = htousbs(0x0200),
	.bDeviceClass       = 0xef,
	.bDeviceSubClass    = 0x02,
	.bDeviceProtocol    = 0x01,
	.bMaxPacketSize0    = 64, /* Must be 64 for high-speed devices */
	.idVendor           = htousbs(USB_VENDOR_ID_OPENPILOT),
	.idProduct          = htousbs(PIOS_USB_BOARD_PRODUCT_ID),
	.bcdDevice          = htousbs(PIOS_USB_BOARD_DEVICE_VER),
	.iManufacturer      = USB_STRING_DESC_VENDOR,
	.iProduct           = USB_STRING_DESC_PRODUCT,
	.iSerialNumber      = USB_STRING_DESC_SERIAL,
	.bNumConfigurations = 1,
};

static const uint8_t hid_report_desc[89] = {
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

	/* Host -> Host emulated serial channel */
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

/* 36 bytes to here */

	/* Emulate a Joystick */
	HID_GLOBAL_ITEM_1 (HID_TAG_GLOBAL_USAGE_PAGE),
	0x01,	      /* Usage Page 0x01 (Generic Desktop Controls) */
	HID_LOCAL_ITEM_1  (HID_TAG_LOCAL_USAGE),
	0x04,			/* Usage ID 0x0004 (Joystick) */

	HID_GLOBAL_ITEM_1 (HID_TAG_GLOBAL_LOGICAL_MIN),
	0x00,			/* Values range from min = 0x00 */
	HID_GLOBAL_ITEM_4 (HID_TAG_GLOBAL_LOGICAL_MAX),
	0xFF, 0xFF, 0x00, 0x00,	/* Values range to max = 0x0000FFFF */

	HID_MAIN_ITEM_1   (HID_TAG_MAIN_COLLECTION),
	0x01,			/* Application */
	HID_MAIN_ITEM_1   (HID_TAG_MAIN_COLLECTION),
	0x00,			/* Physical */
	HID_GLOBAL_ITEM_1 (HID_TAG_GLOBAL_REPORT_ID),
	0x03,			/* OpenPilot Emulated joystick */

	/* X + Y controls */

	HID_LOCAL_ITEM_1  (HID_TAG_LOCAL_USAGE),
	0x30,			/* Usage ID 0x00010030 (Generic Desktop: X) */
	HID_LOCAL_ITEM_1  (HID_TAG_LOCAL_USAGE),
	0x31,			/* Usage ID 0x00010031 (Generic Desktop: Y) */
	HID_GLOBAL_ITEM_1 (HID_TAG_GLOBAL_REPORT_SIZE),
	0x10,			/* 16 bits wide */
	HID_GLOBAL_ITEM_1 (HID_TAG_GLOBAL_REPORT_CNT),
	2,
	HID_MAIN_ITEM_1 (HID_TAG_MAIN_INPUT),
	0x82,			/* Data, Var, Abs, Vol */

	/* Y + Rx controls */

	HID_LOCAL_ITEM_1  (HID_TAG_LOCAL_USAGE),
	0x32,			/* Usage ID 0x00010032 (Generic Desktop: Z) */
	HID_LOCAL_ITEM_1  (HID_TAG_LOCAL_USAGE),
	0x33,			/* Usage ID 0x00010031 (Generic Desktop: Rx) */
	HID_GLOBAL_ITEM_1 (HID_TAG_GLOBAL_REPORT_SIZE),
	0x10,			/* 16 bits wide */
	HID_GLOBAL_ITEM_1 (HID_TAG_GLOBAL_REPORT_CNT),
	2,
	HID_MAIN_ITEM_1 (HID_TAG_MAIN_INPUT),
	0x82,			/* Data, Var, Abs, Vol */

	/* Ry, Rz, Slider + Dial controls */

	HID_LOCAL_ITEM_1  (HID_TAG_LOCAL_USAGE),
	0x34,			/* Usage ID 0x00010034 (Generic Desktop: Ry) */
	HID_LOCAL_ITEM_1  (HID_TAG_LOCAL_USAGE),
	0x35,			/* Usage ID 0x00010035 (Generic Desktop: Rz) */
	HID_LOCAL_ITEM_1  (HID_TAG_LOCAL_USAGE),
	0x36,			/* Usage ID 0x00010036 (Generic Desktop: Slider) */
	HID_LOCAL_ITEM_1  (HID_TAG_LOCAL_USAGE),
	0x37,			/* Usage ID 0x00010037 (Generic Desktop: Dial) */
	HID_GLOBAL_ITEM_1 (HID_TAG_GLOBAL_REPORT_SIZE),
	0x10,			/* 16 bits wide */
	HID_GLOBAL_ITEM_1 (HID_TAG_GLOBAL_REPORT_CNT),
	4,
	HID_MAIN_ITEM_1 (HID_TAG_MAIN_INPUT),
	0x82,			/* Data, Var, Abs, Vol */

	HID_MAIN_ITEM_0 (HID_TAG_MAIN_ENDCOLLECTION),

	HID_MAIN_ITEM_0 (HID_TAG_MAIN_ENDCOLLECTION),

/* 89 bytes to here */
};

struct usb_config_hid_cdc {
	struct usb_configuration_desc         config;
	struct usb_interface_association_desc iad;
	struct usb_interface_desc             cdc_control_if;
	struct usb_cdc_header_func_desc       cdc_header;
	struct usb_cdc_callmgmt_func_desc     cdc_callmgmt;
	struct usb_cdc_acm_func_desc          cdc_acm;
	struct usb_cdc_union_func_desc        cdc_union;
	struct usb_endpoint_desc              cdc_mgmt_in;
	struct usb_interface_desc             cdc_data_if;
	struct usb_endpoint_desc              cdc_in;
	struct usb_endpoint_desc              cdc_out;
	struct usb_interface_desc             hid_if;
	struct usb_hid_desc                   hid;
	struct usb_endpoint_desc              hid_in;
	struct usb_endpoint_desc              hid_out;
} __attribute__((packed));

static const struct usb_config_hid_cdc config_hid_cdc = {
	.config = {
		.bLength              = sizeof(struct usb_configuration_desc),
		.bDescriptorType      = USB_DESC_TYPE_CONFIGURATION,
		.wTotalLength         = htousbs(sizeof(struct usb_config_hid_cdc)),
		.bNumInterfaces       = 3,
		.bConfigurationValue  = 1,
		.iConfiguration       = 0,
		.bmAttributes         = 0xC0,
		.bMaxPower            = 250/2, /* in units of 2ma */
	},
	.iad = {
		.bLength              = sizeof(struct usb_interface_association_desc),
		.bDescriptorType      = USB_DESC_TYPE_IAD,
		.bFirstInterface      = 0,
		.bInterfaceCount      = 2,
		.bFunctionClass       = USB_INTERFACE_CLASS_CDC, /* Communication */
		.bFunctionSubClass    = USB_CDC_DESC_SUBTYPE_ABSTRACT_CTRL, /* Abstract Control Model */
		.bFunctionProtocol    = 1, /* V.25ter, Common AT commands */
		.iInterface           = 0,
	},
	.cdc_control_if = {
		.bLength              = sizeof(struct usb_interface_desc),
		.bDescriptorType      = USB_DESC_TYPE_INTERFACE,
		.bInterfaceNumber     = 0,
		.bAlternateSetting    = 0,
		.bNumEndpoints        = 1,
		.bInterfaceClass      = USB_INTERFACE_CLASS_CDC,
		.bInterfaceSubClass   = USB_CDC_DESC_SUBTYPE_ABSTRACT_CTRL, /* Abstract Control Model */
		.nInterfaceProtocol   = 1,	 /* V.25ter, Common AT commands */
		.iInterface           = 0,
	},
	.cdc_header = {
		.bLength              = sizeof(struct usb_cdc_header_func_desc),
		.bDescriptorType      = USB_DESC_TYPE_CLASS_SPECIFIC,
		.bDescriptorSubType   = USB_CDC_DESC_SUBTYPE_HEADER,
		.bcdCDC               = htousbs(0x0110),
	},
	.cdc_callmgmt = {
		.bLength              = sizeof(struct usb_cdc_callmgmt_func_desc),
		.bDescriptorType      = USB_DESC_TYPE_CLASS_SPECIFIC,
		.bDescriptorSubType   = USB_CDC_DESC_SUBTYPE_CALLMGMT,
		.bmCapabilities       = 0x00, /* No call handling */
		.bDataInterface       = 1,
	},
	.cdc_acm = {
		.bLength              = sizeof(struct usb_cdc_acm_func_desc),
		.bDescriptorType      = USB_DESC_TYPE_CLASS_SPECIFIC,
		.bDescriptorSubType   = USB_CDC_DESC_SUBTYPE_ABSTRACT_CTRL,
		.bmCapabilities       = 0x00,
	},
	.cdc_union = {
		.bLength              = sizeof(struct usb_cdc_union_func_desc),
		.bDescriptorType      = USB_DESC_TYPE_CLASS_SPECIFIC,
		.bDescriptorSubType   = USB_CDC_DESC_SUBTYPE_UNION,
		.bMasterInterface     = 0,
		.bSlaveInterface      = 1,
	},
	.cdc_mgmt_in = {
		.bLength              = sizeof(struct usb_endpoint_desc),
		.bDescriptorType      = USB_DESC_TYPE_ENDPOINT,
		.bEndpointAddress     = USB_EP_IN(2),
		.bmAttributes         = USB_EP_ATTR_TT_INTERRUPT,
		.wMaxPacketSize       = htousbs(PIOS_USB_BOARD_CDC_MGMT_LENGTH),
		.bInterval            = 4, /* ms */
	},
	.cdc_data_if = {
		.bLength              = sizeof(struct usb_interface_desc),
		.bDescriptorType      = USB_DESC_TYPE_INTERFACE,
		.bInterfaceNumber     = 1,
		.bAlternateSetting    = 0,
		.bNumEndpoints        = 2,
		.bInterfaceClass      = USB_INTERFACE_CLASS_DATA,
		.bInterfaceSubClass   = 0,
		.nInterfaceProtocol   = 0, /* No class specific protocol */
		.iInterface           = 0,
	},
	.cdc_in = {
		.bLength              = sizeof(struct usb_endpoint_desc),
		.bDescriptorType      = USB_DESC_TYPE_ENDPOINT,
		.bEndpointAddress     = USB_EP_IN(3),
		.bmAttributes         = USB_EP_ATTR_TT_BULK,
		.wMaxPacketSize       = htousbs(PIOS_USB_BOARD_CDC_DATA_LENGTH),
		.bInterval            = 0, /* ms */
	},
	.cdc_out = {
		.bLength              = sizeof(struct usb_endpoint_desc),
		.bDescriptorType      = USB_DESC_TYPE_ENDPOINT,
		.bEndpointAddress     = USB_EP_OUT(3),
		.bmAttributes         = USB_EP_ATTR_TT_BULK,	  /* Bulk */
		.wMaxPacketSize       = htousbs(PIOS_USB_BOARD_CDC_DATA_LENGTH),
		.bInterval            = 0, /* ms */
	},
	.hid_if = {
		.bLength              = sizeof(struct usb_interface_desc),
		.bDescriptorType      = USB_DESC_TYPE_INTERFACE,
		.bInterfaceNumber     = 2,
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

int32_t PIOS_USB_DESC_HID_CDC_Init(void)
{
	PIOS_USBHOOK_RegisterConfig(1, (uint8_t *)&config_hid_cdc, sizeof(config_hid_cdc));

	PIOS_USBHOOK_RegisterDevice((uint8_t *)&device_desc, sizeof(device_desc));

	PIOS_USB_HID_RegisterHidDescriptor((uint8_t *)&(config_hid_cdc.hid), sizeof(config_hid_cdc.hid));
	PIOS_USB_HID_RegisterHidReport((uint8_t *)hid_report_desc, sizeof(hid_report_desc));

	return 0;
}

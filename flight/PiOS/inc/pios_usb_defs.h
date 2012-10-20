/**
 ******************************************************************************
 * @addtogroup PIOS PIOS Core hardware abstraction layer
 * @{
 * @addtogroup PIOS_USB_DEFS USB standard types and definitions
 * @brief USB standard types and definitions
 * @{
 *
 * @file       pios_usb_defs.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      USB Standard types and definitions
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

#ifndef PIOS_USB_DEFS_H
#define PIOS_USB_DEFS_H

#include <stdint.h>		/* uint*_t */

enum usb_desc_types {
	USB_DESC_TYPE_DEVICE         = 0x01,
	USB_DESC_TYPE_CONFIGURATION  = 0x02,
	USB_DESC_TYPE_STRING         = 0x03,
	USB_DESC_TYPE_INTERFACE      = 0x04,
	USB_DESC_TYPE_ENDPOINT       = 0x05,
	USB_DESC_TYPE_IAD            = 0x0B,
	USB_DESC_TYPE_HID            = 0x21,
	USB_DESC_TYPE_REPORT         = 0x22,
	USB_DESC_TYPE_CLASS_SPECIFIC = 0x24,
} __attribute__((packed));

enum usb_interface_class {
	USB_INTERFACE_CLASS_CDC  = 0x02,
	USB_INTERFACE_CLASS_HID  = 0x03,
	USB_INTERFACE_CLASS_DATA = 0x0A,
} __attribute__((packed));

enum usb_cdc_desc_subtypes {
	USB_CDC_DESC_SUBTYPE_HEADER        = 0x00,
	USB_CDC_DESC_SUBTYPE_CALLMGMT      = 0x01,
	USB_CDC_DESC_SUBTYPE_ABSTRACT_CTRL = 0x02,
	USB_CDC_DESC_SUBTYPE_UNION         = 0x06,
} __attribute__((packed));

enum usb_ep_attr {
	USB_EP_ATTR_TT_CONTROL     = 0x00,
	USB_EP_ATTR_TT_ISOCHRONOUS = 0x01,
	USB_EP_ATTR_TT_BULK        = 0x02,
	USB_EP_ATTR_TT_INTERRUPT   = 0x03,
} __attribute__((packed));

/* Standard macros to convert from host endian to USB endian (ie. little endian) */
#if __BIG_ENDIAN__
#define htousbs(v) ((uint16_t)(\
				((((v) >> 0) & 0xFF) << 8) |	\
				((((v) >> 8) & 0xFF) << 0)))
#define htousbl(v) ((uint32_t)(\
		((((v) >>  0) & 0xFF) << 24) |	\
		((((v) >>  8) & 0xFF) << 16) |	\
		((((v) >> 16) & 0xFF) <<  8) |	\
		((((v) >> 24) & 0xFF) <<  0)))
#else
#define htousbs(v) (v)
#define htousbl(v) (v)
#endif

#define USB_EP_IN(ep)  ((uint8_t) (0x80 | ((ep) & 0xF)))
#define USB_EP_OUT(ep) ((uint8_t) (0x00 | ((ep) & 0xF)))

#define HID_ITEM_TYPE_MAIN   0x0
#define HID_ITEM_TYPE_GLOBAL 0x1
#define HID_ITEM_TYPE_LOCAL  0x2
#define HID_ITEM_TYPE_RSVD   0x3

#define HID_TAG_GLOBAL_USAGE_PAGE  0x0 /* 0b0000 */
#define HID_TAG_GLOBAL_LOGICAL_MIN 0x1 /* 0b0001 */
#define HID_TAG_GLOBAL_LOGICAL_MAX 0x2 /* 0b0010 */
#define HID_TAG_GLOBAL_PHYS_MIN    0x3 /* 0b0011 */
#define HID_TAG_GLOBAL_PHYS_MAX    0x4 /* 0b0100 */
#define HID_TAG_GLOBAL_UNIT_EXP    0x5 /* 0b0101 */
#define HID_TAG_GLOBAL_UNIT        0x6 /* 0b0110 */
#define HID_TAG_GLOBAL_REPORT_SIZE 0x7 /* 0b0111 */
#define HID_TAG_GLOBAL_REPORT_ID   0x8 /* 0b1000 */
#define HID_TAG_GLOBAL_REPORT_CNT  0x9 /* 0b1001 */
#define HID_TAG_GLOBAL_PUSH        0xA /* 0b1010 */
#define HID_TAG_GLOBAL_POP         0xB /* 0b1011 */

#define HID_TAG_MAIN_INPUT         0x8 /* 0b1000 */
#define HID_TAG_MAIN_OUTPUT        0x9 /* 0b1001 */
#define HID_TAG_MAIN_COLLECTION    0xA /* 0b1010 */
#define HID_TAG_MAIN_FEATURE       0xB /* 0b1011 */
#define HID_TAG_MAIN_ENDCOLLECTION 0xC /* 0b1100 */

#define HID_TAG_LOCAL_USAGE        0x0 /* 0b0000 */
#define HID_TAG_LOCAL_USAGE_MIN    0x1 /* 0b0001 */
#define HID_TAG_LOCAL_USAGE_MAX    0x2 /* 0b0010 */
#define HID_TAG_LOCAL_DESIG_INDEX  0x3 /* 0b0011 */
#define HID_TAG_LOCAL_DESIG_MIN    0x4 /* 0b0100 */
#define HID_TAG_LOCAL_DESIG_MAX    0x5 /* 0b0101 */
/* There is no value defined for 0x6 */
#define HID_TAG_LOCAL_STRING_INDEX 0x7 /* 0b0111 */
#define HID_TAG_LOCAL_STRING_MIN   0x8 /* 0b1000 */
#define HID_TAG_LOCAL_STRING_MAX   0x9 /* 0b1001 */
#define HID_TAG_LOCAL_DELIMITER    0xA /* 0b1010 */

#define HID_TAG_RSVD               0xF /* 0b1111 */

#define HID_ITEM_SIZE_0 0
#define HID_ITEM_SIZE_1 1
#define HID_ITEM_SIZE_2 2
#define HID_ITEM_SIZE_4 3	/* Yes, 4 bytes is represented with a size field = 3 */

#define HID_SHORT_ITEM(tag,type,size) (\
	(((tag)  & 0xF) << 4) |	       \
	(((type) & 0x3) << 2) |	       \
	(((size) & 0x3) << 0))

/* Long items have a fixed prefix */
#define HID_LONG_ITEM HID_SHORT_ITEM(HID_TAG_RSVD, HID_ITEM_TYPE_RSVD, HID_ITEM_SIZE_2)

#define HID_MAIN_ITEM_0(tag) HID_SHORT_ITEM((tag), HID_ITEM_TYPE_MAIN, HID_ITEM_SIZE_0)
#define HID_MAIN_ITEM_1(tag) HID_SHORT_ITEM((tag), HID_ITEM_TYPE_MAIN, HID_ITEM_SIZE_1)
#define HID_MAIN_ITEM_2(tag) HID_SHORT_ITEM((tag), HID_ITEM_TYPE_MAIN, HID_ITEM_SIZE_2)
#define HID_MAIN_ITEM_4(tag) HID_SHORT_ITEM((tag), HID_ITEM_TYPE_MAIN, HID_ITEM_SIZE_4)

#define HID_GLOBAL_ITEM_0(tag) HID_SHORT_ITEM((tag), HID_ITEM_TYPE_GLOBAL, HID_ITEM_SIZE_0)
#define HID_GLOBAL_ITEM_1(tag) HID_SHORT_ITEM((tag), HID_ITEM_TYPE_GLOBAL, HID_ITEM_SIZE_1)
#define HID_GLOBAL_ITEM_2(tag) HID_SHORT_ITEM((tag), HID_ITEM_TYPE_GLOBAL, HID_ITEM_SIZE_2)
#define HID_GLOBAL_ITEM_4(tag) HID_SHORT_ITEM((tag), HID_ITEM_TYPE_GLOBAL, HID_ITEM_SIZE_4)

#define HID_LOCAL_ITEM_0(tag) HID_SHORT_ITEM((tag), HID_ITEM_TYPE_LOCAL, HID_ITEM_SIZE_0)
#define HID_LOCAL_ITEM_1(tag) HID_SHORT_ITEM((tag), HID_ITEM_TYPE_LOCAL, HID_ITEM_SIZE_1)
#define HID_LOCAL_ITEM_2(tag) HID_SHORT_ITEM((tag), HID_ITEM_TYPE_LOCAL, HID_ITEM_SIZE_2)
#define HID_LOCAL_ITEM_4(tag) HID_SHORT_ITEM((tag), HID_ITEM_TYPE_LOCAL, HID_ITEM_SIZE_4)

struct usb_device_desc {
	uint8_t  bLength;
	uint8_t  bDescriptorType;
	uint16_t bcdUSB;
	uint8_t  bDeviceClass;
	uint8_t  bDeviceSubClass;
	uint8_t  bDeviceProtocol;
	uint8_t  bMaxPacketSize0;
	uint16_t idVendor;
	uint16_t idProduct;
	uint16_t bcdDevice;
	uint8_t  iManufacturer;
	uint8_t  iProduct;
	uint8_t  iSerialNumber;
	uint8_t  bNumConfigurations;
} __attribute__((packed));

struct usb_configuration_desc {
	uint8_t  bLength;
	uint8_t  bDescriptorType;
	uint16_t wTotalLength;
	uint8_t  bNumInterfaces;
	uint8_t  bConfigurationValue;
	uint8_t  iConfiguration;
	uint8_t  bmAttributes;
	uint8_t  bMaxPower;
} __attribute__((packed));

struct usb_interface_association_desc {
	uint8_t  bLength;
	uint8_t  bDescriptorType;
	uint8_t  bFirstInterface;
	uint8_t  bInterfaceCount;
	uint8_t  bFunctionClass;
	uint8_t  bFunctionSubClass;
	uint8_t  bFunctionProtocol;
	uint8_t  iInterface;
} __attribute__((packed));

struct usb_interface_desc {
	uint8_t  bLength;
	uint8_t  bDescriptorType;
	uint8_t  bInterfaceNumber;
	uint8_t  bAlternateSetting;
	uint8_t  bNumEndpoints;
	uint8_t  bInterfaceClass;
	uint8_t  bInterfaceSubClass;
	uint8_t  nInterfaceProtocol;
	uint8_t  iInterface;
} __attribute__((packed));

struct usb_hid_desc {
	uint8_t  bLength;
	uint8_t  bDescriptorType;
	uint16_t bcdHID;
	uint8_t  bCountryCode;
	uint8_t  bNumDescriptors;
	uint8_t  bClassDescriptorType;
	uint16_t wItemLength;
} __attribute__((packed));

struct usb_endpoint_desc {
	uint8_t  bLength;
	uint8_t  bDescriptorType;
	uint8_t  bEndpointAddress;
	uint8_t  bmAttributes;
	uint16_t wMaxPacketSize;
	uint8_t  bInterval;
} __attribute__((packed));

struct usb_setup_request {
	uint8_t  bmRequestType;
	uint8_t  bRequest;
	uint16_t wValue;
	uint16_t wIndex;
	uint16_t wLength;
} __attribute__((packed));

#define USB_REQ_TYPE_STANDARD                          0x00
#define USB_REQ_TYPE_CLASS                             0x20
#define USB_REQ_TYPE_VENDOR                            0x40
#define USB_REQ_TYPE_MASK                              0x60

#define USB_REQ_RECIPIENT_DEVICE                       0x00
#define USB_REQ_RECIPIENT_INTERFACE                    0x01
#define USB_REQ_RECIPIENT_ENDPOINT                     0x02
#define USB_REQ_RECIPIENT_MASK                         0x03

enum usb_standard_requests {
	USB_REQ_GET_STATUS         = 0x00,
	USB_REQ_CLEAR_FEATURE      = 0x01,
	/* what is 0x02? */
	USB_REQ_SET_FEATURE        = 0x03,
	/* what is 0x04? */
	USB_REQ_SET_ADDRESS        = 0x05,
	USB_REQ_GET_DESCRIPTOR     = 0x06,
	USB_REQ_SET_DESCRIPTOR     = 0x07,
	USB_REQ_GET_CONFIGURATION  = 0x08,
	USB_REQ_SET_CONFIGURATION  = 0x09,
	USB_REQ_GET_INTERFACE      = 0x0A,
	USB_REQ_SET_INTERFACE      = 0x0B,
	USB_REQ_SYNCH_FRAME        = 0x0C,
};

enum usb_hid_requests {
	USB_HID_REQ_GET_REPORT     = 0x01,
	USB_HID_REQ_GET_IDLE       = 0x02,
	USB_HID_REQ_GET_PROTOCOL   = 0x03,
	/* 0x04-0x08 Reserved */
	USB_HID_REQ_SET_REPORT     = 0x09,
	USB_HID_REQ_SET_IDLE       = 0x0A,
	USB_HID_REQ_SET_PROTOCOL   = 0x0B,
};

enum usb_cdc_requests {
	USB_CDC_REQ_SET_LINE_CODING        = 0x20,
	USB_CDC_REQ_GET_LINE_CODING        = 0x21,

	USB_CDC_REQ_SET_CONTROL_LINE_STATE = 0x22,
};

struct usb_cdc_header_func_desc {
	uint8_t  bLength;
	uint8_t  bDescriptorType;
	uint8_t  bDescriptorSubType;
	uint16_t bcdCDC;
} __attribute__((packed));

struct usb_cdc_callmgmt_func_desc {
	uint8_t  bLength;
	uint8_t  bDescriptorType;
	uint8_t  bDescriptorSubType;
	uint8_t  bmCapabilities;
	uint8_t  bDataInterface;
} __attribute__((packed));

struct usb_cdc_acm_func_desc {
	uint8_t  bLength;
	uint8_t  bDescriptorType;
	uint8_t  bDescriptorSubType;
	uint8_t  bmCapabilities;
} __attribute__((packed));

struct usb_cdc_union_func_desc {
	uint8_t  bLength;
	uint8_t  bDescriptorType;
	uint8_t  bDescriptorSubType;
	uint8_t  bMasterInterface;
	uint8_t  bSlaveInterface;
} __attribute__((packed));

#define USB_LANGID_ENGLISH_US 0x0409

struct usb_string_langid {
	uint8_t  bLength;
	uint8_t  bDescriptorType;
	uint16_t bLangID;
} __attribute__((packed));

struct usb_cdc_line_coding {
	uint32_t dwDTERate;
	uint8_t  bCharFormat;
	uint8_t  bParityType;
	uint8_t  bDataBits;
} __attribute__((packed));

enum usb_cdc_line_coding_stop {
	USB_CDC_LINE_CODING_STOP_1   = 0,
	USB_CDC_LINE_CODING_STOP_1_5 = 1,
	USB_CDC_LINE_CODING_STOP_2   = 2,
} __attribute__((packed));

enum usb_cdc_line_coding_parity {
	USB_CDC_LINE_CODING_PARITY_NONE  = 0,
	USB_CDC_LINE_CODING_PARITY_ODD   = 1,
	USB_CDC_LINE_CODING_PARITY_EVEN  = 2,
	USB_CDC_LINE_CODING_PARITY_MARK  = 3,
	USB_CDC_LINE_CODING_PARITY_SPACE = 4,
} __attribute__((packed));

struct usb_cdc_serial_state_report {
	uint8_t  bmRequestType;
	uint8_t  bNotification;
	uint16_t wValue;
	uint16_t wIndex;
	uint16_t wLength;
	uint16_t bmUartState;
} __attribute__((packed));

enum usb_cdc_notification {
	USB_CDC_NOTIFICATION_SERIAL_STATE = 0x20,
} __attribute__((packed));

/*
 * OpenPilot Specific USB Definitions
 */

#define USB_VENDOR_ID_OPENPILOT 0x20A0

enum usb_product_ids {
	USB_PRODUCT_ID_OPENPILOT_MAIN = 0x415A,
	USB_PRODUCT_ID_COPTERCONTROL  = 0x415B,
	USB_PRODUCT_ID_PIPXTREME      = 0x415C,
	USB_PRODUCT_ID_CC3D           = 0x415D,
	USB_PRODUCT_ID_REVOLUTION     = 0x415E,
	USB_PRODUCT_ID_OSD            = 0x4194,
	USB_PRODUCT_ID_SPARE          = 0x4195,
} __attribute__((packed));

enum usb_op_board_ids {
	USB_OP_BOARD_ID_OPENPILOT_MAIN = 1,
	/* Board ID 2 may be unused or AHRS */
	USB_OP_BOARD_ID_PIPXTREME      = 3,
	USB_OP_BOARD_ID_COPTERCONTROL  = 4,
	USB_OP_BOARD_ID_REVOLUTION     = 5,
} __attribute__((packed));

enum usb_op_board_modes {
	USB_OP_BOARD_MODE_BL = 1,
	USB_OP_BOARD_MODE_FW = 2,
} __attribute__((packed));

#define USB_OP_DEVICE_VER(board_id, board_mode) (\
		((board_id   & 0xFF) << 8) |	 \
		((board_mode & 0xFF) << 0))

#endif /* PIOS_USB_DEFS_H */

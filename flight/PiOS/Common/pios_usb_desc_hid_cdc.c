#include "pios_usb_board_data.h" /* struct usb_*, USB_* */

const struct usb_device_desc PIOS_USB_BOARD_DeviceDescriptor = {
	.bLength            = sizeof(struct usb_device_desc),
	.bDescriptorType    = USB_DESC_TYPE_DEVICE,
	.bcdUSB             = htousbs(0x0200),
	.bDeviceClass       = 0x02,
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

const struct usb_board_config PIOS_USB_BOARD_Configuration = {
	.config = {
		.bLength              = sizeof(struct usb_configuration_desc),
		.bDescriptorType      = USB_DESC_TYPE_CONFIGURATION,
		.wTotalLength         = htousbs(sizeof(struct usb_board_config)),
		.bNumInterfaces       = 3,
		.bConfigurationValue  = 1,
		.iConfiguration       = 0,
		.bmAttributes         = 0xC0,
		.bMaxPower            = 250/2, /* in units of 2ma */
	},
	.iad = {
		.bLength              = sizeof(struct usb_interface_association_desc),
		.bDescriptorType      = USB_DESC_TYPE_IAD,
		.bFirstInterface      = 1,
		.bInterfaceCount      = 2,
		.bFunctionClass       = 2, /* Communication */
		.bFunctionSubClass    = 2, /* Abstract Control Model */
		.bFunctionProtocol    = 0, /* V.25ter, Common AT commands */
		.iInterface           = 0,
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
		.wItemLength          = htousbs(sizeof(PIOS_USB_BOARD_HidReportDescriptor)),
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
	.cdc_control_if = {
		.bLength              = sizeof(struct usb_interface_desc),
		.bDescriptorType      = USB_DESC_TYPE_INTERFACE,
		.bInterfaceNumber     = 1,
		.bAlternateSetting    = 0,
		.bNumEndpoints        = 1,
		.bInterfaceClass      = USB_INTERFACE_CLASS_CDC,
		.bInterfaceSubClass   = 2, /* Abstract Control Model */
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
		.bDataInterface       = 2,
	},
	.cdc_acm = {
		.bLength              = sizeof(struct usb_cdc_acm_func_desc),
		.bDescriptorType      = USB_DESC_TYPE_CLASS_SPECIFIC,
		.bDescriptorSubType   = USB_CDC_DESC_SUBTYPE_ABSTRACT_CTRL,
		.bmCapabilities       = 0,
	},
	.cdc_union = {
		.bLength              = sizeof(struct usb_cdc_union_func_desc),
		.bDescriptorType      = USB_DESC_TYPE_CLASS_SPECIFIC,
		.bDescriptorSubType   = USB_CDC_DESC_SUBTYPE_UNION,
		.bMasterInterface     = 1,
		.bSlaveInterface      = 2,
	},
	.cdc_mgmt_in = {
		.bLength              = sizeof(struct usb_endpoint_desc),
		.bDescriptorType      = USB_DESC_TYPE_ENDPOINT,
		.bEndpointAddress     = USB_EP_IN(2),
		.bmAttributes         = USB_EP_ATTR_TT_INTERRUPT,
		.wMaxPacketSize       = htousbs(16),
		.bInterval            = 4, /* ms */
	},
	.cdc_data_if = {
		.bLength              = sizeof(struct usb_interface_desc),
		.bDescriptorType      = USB_DESC_TYPE_INTERFACE,
		.bInterfaceNumber     = 2,
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
};

const uint8_t PIOS_USB_BOARD_HidReportDescriptor[] = {
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
};

const struct usb_string_langid PIOS_USB_BOARD_StringLangID = {
	.bLength = sizeof(PIOS_USB_BOARD_StringLangID),
	.bDescriptorType = USB_DESC_TYPE_STRING,
	.bLangID = htousbs(USB_LANGID_ENGLISH_UK),
};

const uint8_t PIOS_USB_BOARD_StringVendorID[] = {
	sizeof(PIOS_USB_BOARD_StringVendorID),
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

uint8_t PIOS_USB_BOARD_StringSerial[] = {
	sizeof(PIOS_USB_BOARD_StringSerial),
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

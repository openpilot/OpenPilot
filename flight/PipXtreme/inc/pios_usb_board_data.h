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

#define PIOS_USB_BOARD_PRODUCT_ID USB_PRODUCT_ID_PIPXTREME
#define PIOS_USB_BOARD_DEVICE_VER USB_OP_DEVICE_VER(USB_OP_BOARD_ID_PIPXTREME, USB_OP_BOARD_MODE_FW)

#endif	/* PIOS_USB_BOARD_DATA_H */

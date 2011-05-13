/**
 ******************************************************************************
 * @addtogroup PIOS PIOS Core hardware abstraction layer
 * @{
 * @addtogroup   PIOS_USB USB Functions
 * @brief PIOS USB interface code
 * @{
 *
 * @file       pios_usb.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * 	       Parts by Thorsten Klose (tk@midibox.org)
 * @brief      USB functions header.
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

#ifndef PIOS_USB_H
#define PIOS_USB_H

/* Local defines */
/* Following settings allow to customise the USB device descriptor */
#ifndef PIOS_USB_VENDOR_ID
#define PIOS_USB_VENDOR_ID		0x20A0
#endif

#ifndef PIOS_USB_VENDOR_STR
#define PIOS_USB_VENDOR_STR		"openpilot.org"
#endif

#ifndef PIOS_USB_PRODUCT_STR
#define PIOS_USB_PRODUCT_STR		"OpenPilot"
#endif

#ifndef PIOS_USB_PRODUCT_ID
#define PIOS_USB_PRODUCT_ID		0x415A
#endif

#ifndef PIOS_USB_VERSION_ID
#define PIOS_USB_VERSION_ID		0x0101	/* OpenPilot (01) Bootloader (01) */
#endif

/* Internal defines which are used by PIOS USB HID (don't touch) */
#define PIOS_USB_EP_NUM			2

/* Buffer table base address */
#define PIOS_USB_BTABLE_ADDRESS		0x000

/* EP0 rx/tx buffer base address */
#define PIOS_USB_ENDP0_RXADDR		0x040
#define PIOS_USB_ENDP0_TXADDR		0x080

/* EP1 Rx/Tx buffer base address for HID driver */
#define PIOS_USB_ENDP1_TXADDR		0x0C0
#define PIOS_USB_ENDP1_RXADDR		0x100

/* Global Variables */
extern void (*pEpInt_IN[7])(void);
extern void (*pEpInt_OUT[7])(void);

/* Public Functions */
extern int32_t PIOS_USB_Init(uint32_t mode);
extern int32_t PIOS_USB_IsInitialized(void);
extern int32_t PIOS_USB_CableConnected(void);

#endif /* PIOS_USB_H */

/**
 * @}
 * @}
 */

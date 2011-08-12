/**
 ******************************************************************************
 * @addtogroup PIOS PIOS Core hardware abstraction layer
 * @{
 * @addtogroup   PIOS_USB_HID USB HID Functions
 * @{
 *
 * @file       pios_usb_hid.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * 	       Parts by Thorsten Klose (tk@midibox.org)
 * @brief      USB HID layer functions header
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

#ifndef PIOS_USB_HID_H
#define PIOS_USB_HID_H

/* Global Definitions */
#define PIOS_USB_HID_SIZ_REPORT_DESC		32
#define PIOS_USB_HID_REPORT_DESCRIPTOR		0x22
#define PIOS_USB_HID_HID_DESCRIPTOR_TYPE	0x21
#define PIOS_USB_HID_OFF_HID_DESC		0x12
#define PIOS_USB_HID_SIZ_HID_DESC		0x09

#define PIOS_USB_HID_DATA_LENGTH		62

/* Global functions */
extern int32_t PIOS_USB_HID_Reenumerate();
extern int32_t PIOS_USB_HID_ChangeConnectionState(uint32_t Connected);
extern int32_t PIOS_USB_HID_CheckAvailable(uint8_t id);

extern int32_t PIOS_USB_HID_CB_Data_Setup(uint8_t RequestNo);
extern int32_t PIOS_USB_HID_CB_NoData_Setup(uint8_t RequestNo);
extern void PIOS_USB_HID_EP1_IN_Callback(void);
extern void PIOS_USB_HID_EP1_OUT_Callback(void);

#endif /* PIOS_USB_HID_H */

/**
  * @}
  * @}
  */

/**
 ******************************************************************************
 * @addtogroup PIOS PIOS Core hardware abstraction layer
 * @{
 * @addtogroup   PIOS_USB_HID USB HID Functions
 * @{
 *
 * @file       pios_usb_hid.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
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

/* Global functions */
extern int32_t PIOS_USB_HID_Reenumerate(void);
extern int32_t PIOS_USB_HID_ChangeConnectionState(uint32_t Connected);
extern bool PIOS_USB_HID_CheckAvailable(uint8_t id);

extern void PIOS_USB_HID_RegisterHidDescriptor(const uint8_t * desc, uint16_t length);
extern void PIOS_USB_HID_RegisterHidReport(const uint8_t * desc, uint16_t length);

#endif /* PIOS_USB_HID_H */

/**
  * @}
  * @}
  */

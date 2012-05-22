/**
 ******************************************************************************
 * @addtogroup PIOS PIOS Core hardware abstraction layer
 * @{
 * @addtogroup   PIOS_USB USB Functions
 * @{
 *
 * @file       pios_usb.h
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

#ifndef PIOS_USB_H
#define PIOS_USB_H

#include <stdbool.h>

/* Global functions */
extern int32_t PIOS_USB_Reenumerate();
extern int32_t PIOS_USB_ChangeConnectionState(bool connected);
extern bool PIOS_USB_CableConnected(uint8_t id);
extern bool PIOS_USB_CheckAvailable(uint8_t id);

#endif /* PIOS_USB_H */

/**
  * @}
  * @}
  */

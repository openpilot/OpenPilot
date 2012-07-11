/**
 ******************************************************************************
 * @addtogroup PIOS PIOS Core hardware abstraction layer
 * @{
 * @addtogroup PIOS_USBHOOK USB glue code
 * @brief Glue between PiOS and STM32 libs
 * @{
 *
 * @file       pios_usbhook.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      APIs for PIOS_USBHOOK layer
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

#ifndef PIOS_USBHOOK_H
#define PIOS_USBHOOK_H

#include <stdbool.h>
#include <stdint.h>
#include "pios_usb_defs.h"	/* usb_setup_request */

struct pios_usbhook_descriptor {
	const uint8_t * descriptor;
	uint16_t length;
};

enum usb_string_desc {
	USB_STRING_DESC_LANG    = 0,
	USB_STRING_DESC_VENDOR  = 1,
	USB_STRING_DESC_PRODUCT = 2,
	USB_STRING_DESC_SERIAL  = 3,
} __attribute__((packed));

extern void PIOS_USBHOOK_RegisterDevice(const uint8_t * desc, uint16_t desc_size);
extern void PIOS_USBHOOK_RegisterConfig(uint8_t config_id, const uint8_t * desc, uint16_t desc_size);
extern void PIOS_USBHOOK_RegisterString(enum usb_string_desc string_id, const uint8_t * desc, uint16_t desc_size);

struct pios_usb_ifops {
  void (*init)(uint32_t context);
  void (*deinit)(uint32_t context);
  bool (*setup)(uint32_t context, struct usb_setup_request * req);
  void (*ctrl_data_out)(uint32_t context, struct usb_setup_request * req);
};

extern void PIOS_USBHOOK_RegisterIfOps(uint8_t ifnum, struct pios_usb_ifops * ifops, uint32_t context);

typedef bool (*pios_usbhook_epcb)(uint32_t context, uint8_t epnum, uint16_t len);

extern void PIOS_USBHOOK_RegisterEpInCallback(uint8_t epnum, uint16_t max_len, pios_usbhook_epcb cb, uint32_t context);
extern void PIOS_USBHOOK_RegisterEpOutCallback(uint8_t epnum, uint16_t max_len, pios_usbhook_epcb cb, uint32_t context);
extern void PIOS_USBHOOK_DeRegisterEpInCallback(uint8_t epnum);
extern void PIOS_USBHOOK_DeRegisterEpOutCallback(uint8_t epnum);

extern void PIOS_USBHOOK_CtrlTx(const uint8_t *buf, uint16_t len);
extern void PIOS_USBHOOK_CtrlRx(uint8_t *buf, uint16_t len);
extern void PIOS_USBHOOK_EndpointTx(uint8_t epnum, const uint8_t *buf, uint16_t len);
extern void PIOS_USBHOOK_EndpointRx(uint8_t epnum, uint8_t *buf, uint16_t len);
extern void PIOS_USBHOOK_Activate(void);
extern void PIOS_USBHOOK_Deactivate(void);

#endif /* PIOS_USBHOOK_H */


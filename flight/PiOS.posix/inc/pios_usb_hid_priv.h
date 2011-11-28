 /**
 ******************************************************************************
 * @addtogroup PIOS PIOS Core hardware abstraction layer
 * @{
 * @addtogroup   PIOS_USB_HID USB_HID Functions
 * @brief PIOS interface for USB_HID port
 * @{
 *
 * @file       pios_usb_hid_priv.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      USB_HID private definitions.
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

#ifndef PIOS_USB_HID_PRIV_H
#define PIOS_USB_HID_PRIV_H

#include <pios.h>
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <usb.h>

struct pios_usb_hid_cfg {
	uint32_t vendor;
	uint32_t product;
};

typedef struct {
  const struct pios_usb_hid_cfg * cfg;
#if defined(PIOS_INCLUDE_FREERTOS)
  xTaskHandle rxThread;
  xTaskHandle txThread;
  volatile uint8_t txBytesAvailable;
#else
  pthread_t rxThread;
  pthread_t txThread;
#endif

  usb_dev_handle *device;
  uint32_t endpoint_in;
  uint32_t endpoint_out;

  pthread_cond_t cond;
  pthread_mutex_t mutex;

  pios_com_callback tx_out_cb;
  uint32_t tx_out_context;
  pios_com_callback rx_in_cb;
  uint32_t rx_in_context;

  uint8_t rx_buffer[PIOS_USB_RX_BUFFER_SIZE];
  uint8_t tx_buffer[PIOS_USB_RX_BUFFER_SIZE];
} pios_usb_dev;

extern int32_t PIOS_USB_HID_Init(uint32_t * usb_hid_id, const struct pios_usb_hid_cfg * cfg);

#endif /* PIOS_USB_HID_PRIV_H */

/**
  * @}
  * @}
  */


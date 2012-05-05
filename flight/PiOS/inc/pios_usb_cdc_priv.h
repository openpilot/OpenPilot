/**
 ******************************************************************************
 * @addtogroup PIOS PIOS Core hardware abstraction layer
 * @{
 * @addtogroup PIOS_USB_COM USB CDC COM layer functions
 * @brief Hardware communication layer
 * @{
 *
 * @file       pios_usb_cdc_priv.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      USB COM CDC private definitions.
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

#ifndef PIOS_USB_CDC_PRIV_H
#define PIOS_USB_CDC_PRIV_H

struct pios_usb_cdc_cfg {
	uint8_t ctrl_if;
	uint8_t ctrl_tx_ep;

	uint8_t data_if;
	uint8_t data_rx_ep;
	uint8_t data_tx_ep;
};

extern const struct pios_com_driver pios_usb_cdc_com_driver;

extern int32_t PIOS_USB_CDC_Init(uint32_t * usbcdc_id, const struct pios_usb_cdc_cfg * cfg, uint32_t lower_id);

#endif /* PIOS_USB_CDC_PRIV_H */

/**
  * @}
  * @}
  */

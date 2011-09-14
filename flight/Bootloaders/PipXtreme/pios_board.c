/**
 ******************************************************************************
 *
 * @file       pios_board.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      Defines board specific static initializers for hardware for the PipBee board.
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

#include <pios.h>

// ***********************************************************************************

#if defined(PIOS_INCLUDE_COM)

#include <pios_com_priv.h>

#define PIOS_COM_TELEM_USB_RX_BUF_LEN 192
#define PIOS_COM_TELEM_USB_TX_BUF_LEN 192

static uint8_t pios_com_telem_usb_rx_buffer[PIOS_COM_TELEM_USB_RX_BUF_LEN];
static uint8_t pios_com_telem_usb_tx_buffer[PIOS_COM_TELEM_USB_TX_BUF_LEN];

#endif /* PIOS_INCLUDE_COM */

// ***********************************************************************************

#if defined(PIOS_INCLUDE_USB_HID)
#include "pios_usb_hid_priv.h"

static const struct pios_usb_cfg pios_usb_main_cfg = {
  .irq = {
    .init    = {
      .NVIC_IRQChannel                   = USB_LP_CAN1_RX0_IRQn,
      .NVIC_IRQChannelPreemptionPriority = PIOS_IRQ_PRIO_LOW,
      .NVIC_IRQChannelSubPriority        = 0,
      .NVIC_IRQChannelCmd                = ENABLE,
    },
  },
};
#endif	/* PIOS_INCLUDE_USB_HID */

#if defined(PIOS_INCLUDE_USB_COM)
#include <pios_usb_com_priv.h>

const struct pios_usb_com_cfg pios_usb_com_hid_cfg = {
	.type = PIOS_USB_COM_HID,
	.data_if = 0,
	.data_rx_ep = 1,
	.data_tx_ep = 1,
};

#endif	/* PIOS_INCLUDE_USB_COM */

uint32_t pios_com_telem_usb_id;

/**
 * PIOS_Board_Init()
 * initializes all the core subsystems on this specific hardware
 * called from System/openpilot.c
 */
static bool board_init_complete = false;
void PIOS_Board_Init(void) {
	if (board_init_complete) {
		return;
	}

	/* Enable Prefetch Buffer */
	FLASH_PrefetchBufferCmd(FLASH_PrefetchBuffer_Enable);

	/* Flash 2 wait state */
	FLASH_SetLatency(FLASH_Latency_2);

	/* Delay system */
	PIOS_DELAY_Init();

	/* Initialize the PiOS library */
	PIOS_GPIO_Init();

#if defined(PIOS_INCLUDE_USB_HID)
	uint32_t pios_usb_id;
	if (PIOS_USB_Init(&pios_usb_id, &pios_usb_main_cfg)) {
		PIOS_Assert(0);
	}
#if defined(PIOS_INCLUDE_COM)
	uint32_t pios_usb_com_id;
	if (PIOS_USB_COM_Init(&pios_usb_com_id, &pios_usb_com_hid_cfg, pios_usb_id)) {
		PIOS_Assert(0);
	}
	if (PIOS_COM_Init(&pios_com_telem_usb_id, &pios_usb_hid_com_driver, pios_usb_com_id,
			  pios_com_telem_usb_rx_buffer, sizeof(pios_com_telem_usb_rx_buffer),
			  pios_com_telem_usb_tx_buffer, sizeof(pios_com_telem_usb_tx_buffer))) {
		PIOS_Assert(0);
	}
#endif	/* PIOS_INCLUDE_COM */
#endif	/* PIOS_INCLUDE_USB_HID */

	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_CRC, ENABLE);//TODO Tirar

	board_init_complete = true;
}

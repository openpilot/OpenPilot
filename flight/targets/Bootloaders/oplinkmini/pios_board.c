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

#include "board_hw_defs.c"
#include <pios.h>

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

#if defined(PIOS_INCLUDE_LED)
 	PIOS_LED_Init(&pios_led_cfg);
#endif	/* PIOS_INCLUDE_LED */

#if defined(PIOS_INCLUDE_USB)
	/* Initialize board specific USB data */
	PIOS_USB_BOARD_DATA_Init();

	/* Activate the HID-only USB configuration */
	PIOS_USB_DESC_HID_ONLY_Init();

	uint32_t pios_usb_id;
	if (PIOS_USB_Init(&pios_usb_id, &pios_usb_main_cfg)) {
		PIOS_Assert(0);
	}
#if defined(PIOS_INCLUDE_USB_HID) && defined(PIOS_INCLUDE_COM_MSG)
	uint32_t pios_usb_hid_id;
	if (PIOS_USB_HID_Init(&pios_usb_hid_id, &pios_usb_hid_cfg, pios_usb_id)) {
		PIOS_Assert(0);
	}
	if (PIOS_COM_MSG_Init(&pios_com_telem_usb_id, &pios_usb_hid_com_driver, pios_usb_hid_id)) {
		PIOS_Assert(0);
	}
#endif	/* PIOS_INCLUDE_USB_HID && PIOS_INCLUDE_COM_MSG */

#endif	/* PIOS_INCLUDE_USB */

	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_CRC, ENABLE);//TODO Tirar

	board_init_complete = true;
}

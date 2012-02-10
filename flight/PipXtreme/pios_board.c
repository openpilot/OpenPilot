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

/* Pull in the board-specific static HW definitions.
 * Including .c files is a bit ugly but this allows all of
 * the HW definitions to be const and static to limit their
 * scope.  
 *
 * NOTE: THIS IS THE ONLY PLACE THAT SHOULD EVER INCLUDE THIS FILE
 */
#include "board_hw_defs.c"

#include <pios.h>

#define PIOS_COM_TELEM_USB_RX_BUF_LEN 192
#define PIOS_COM_TELEM_USB_TX_BUF_LEN 192

static uint8_t pios_com_telem_usb_rx_buffer[PIOS_COM_TELEM_USB_RX_BUF_LEN];
static uint8_t pios_com_telem_usb_tx_buffer[PIOS_COM_TELEM_USB_TX_BUF_LEN];

#define PIOS_COM_SERIAL_RX_BUF_LEN 192
#define PIOS_COM_SERIAL_TX_BUF_LEN 192

static uint8_t pios_com_serial_rx_buffer[PIOS_COM_SERIAL_RX_BUF_LEN];
static uint8_t pios_com_serial_tx_buffer[PIOS_COM_SERIAL_TX_BUF_LEN];

uint32_t pios_com_serial_id;
uint32_t pios_com_telem_usb_id;

/**
 * PIOS_Board_Init()
 * initializes all the core subsystems on this specific hardware
 * called from System/openpilot.c
 */
void PIOS_Board_Init(void) {
	// Bring up System using CMSIS functions, enables the LEDs.
	PIOS_SYS_Init();

	// turn all the leds on
	USB_LED_ON;
	LINK_LED_ON;
	RX_LED_ON;
	TX_LED_ON;

	// Delay system
	PIOS_DELAY_Init();

	uint32_t pios_usart_serial_id;
	if (PIOS_USART_Init(&pios_usart_serial_id, &pios_usart_serial_cfg)) {
		PIOS_DEBUG_Assert(0);
	}
	if (PIOS_COM_Init(&pios_com_serial_id, &pios_usart_com_driver, pios_usart_serial_id,
			  pios_com_serial_rx_buffer, sizeof(pios_com_serial_rx_buffer),
			  pios_com_serial_tx_buffer, sizeof(pios_com_serial_tx_buffer))) {
		PIOS_DEBUG_Assert(0);
	}

#if defined(PIOS_INCLUDE_USB)
	/* Initialize board specific USB data */
	PIOS_USB_BOARD_DATA_Init();

	if (PIOS_USB_DESC_HID_ONLY_Init()) {
		PIOS_Assert(0);
	}

	uint32_t pios_usb_id;
	PIOS_USB_Init(&pios_usb_id, &pios_usb_main_cfg);

#if defined(PIOS_INCLUDE_USB_HID) && defined(PIOS_INCLUDE_COM)
	uint32_t pios_usb_hid_id;
	if (PIOS_USB_HID_Init(&pios_usb_hid_id, &pios_usb_hid_cfg, pios_usb_id)) {
		PIOS_Assert(0);
	}
	if (PIOS_COM_Init(&pios_com_telem_usb_id, &pios_usb_hid_com_driver, pios_usb_hid_id,
			  pios_com_telem_usb_rx_buffer, sizeof(pios_com_telem_usb_rx_buffer),
			  pios_com_telem_usb_tx_buffer, sizeof(pios_com_telem_usb_tx_buffer))) {
		PIOS_Assert(0);
	}
#endif	/* PIOS_INCLUDE_USB_HID && PIOS_INCLUDE_COM */

#endif	/* PIOS_INCLUDE_USB_HID */

	// ADC system
	// PIOS_ADC_Init();

	// SPI link to master
	if (PIOS_SPI_Init(&pios_spi_port_id, &pios_spi_port_cfg)) {
		PIOS_DEBUG_Assert(0);
	}
}

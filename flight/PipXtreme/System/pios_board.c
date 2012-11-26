/* -*- Mode: c; c-basic-offset: 2; tab-width: 2; indent-tabs-mode: t -*- */
/**
 ******************************************************************************
 * @addtogroup OpenPilotSystem OpenPilot System
 * @{
 * @addtogroup OpenPilotCore OpenPilot Core
 * @{
 *
 * @file       pios_board.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      Defines board specific static initializers for hardware for the OpenPilot board.
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
#include <openpilot.h>
#include <oplinksettings.h>
#include <board_hw_defs.c>

#define PIOS_COM_TELEM_RX_BUF_LEN 256
#define PIOS_COM_TELEM_TX_BUF_LEN 256

#define PIOS_COM_TELEM_USB_RX_BUF_LEN 256
#define PIOS_COM_TELEM_USB_TX_BUF_LEN 256

#define PIOS_COM_TELEM_VCP_RX_BUF_LEN 256
#define PIOS_COM_TELEM_VCP_TX_BUF_LEN 256

#define PIOS_COM_RFM22B_RF_RX_BUF_LEN 256
#define PIOS_COM_RFM22B_RF_TX_BUF_LEN 256

uint32_t pios_com_telem_usb_id = 0;
uint32_t pios_com_telem_vcp_id = 0;
uint32_t pios_com_telem_uart_telem_id = 0;
uint32_t pios_com_telem_uart_flexi_id = 0;
uint32_t pios_com_telemetry_id = 0;
#if defined(PIOS_INCLUDE_PPM)
uint32_t pios_ppm_rcvr_id = 0;
#endif
#if defined(PIOS_INCLUDE_RFM22B)
uint32_t pios_rfm22b_id = 0;
uint32_t pios_com_rfm22b_id = 0;
uint32_t pios_com_radio_id = 0;
uint32_t pios_packet_handler = 0;
#endif

/**
 * PIOS_Board_Init()
 * initializes all the core subsystems on this specific hardware
 * called from System/openpilot.c
 */
void PIOS_Board_Init(void) {

	/* Delay system */
	PIOS_DELAY_Init();

	/* Initialize UAVObject libraries */
	EventDispatcherInitialize();
	UAVObjInitialize();
	
	/* Set up the SPI interface to the rfm22b */
	if (PIOS_SPI_Init(&pios_spi_rfm22b_id, &pios_spi_rfm22b_cfg)) {
		PIOS_DEBUG_Assert(0);
	}

#ifdef PIOS_INCLUDE_WDG
	/* Initialize watchdog as early as possible to catch faults during init */
	PIOS_WDG_Init();
#endif /* PIOS_INCLUDE_WDG */

#if defined(PIOS_INCLUDE_RTC)
	/* Initialize the real-time clock and its associated tick */
	PIOS_RTC_Init(&pios_rtc_main_cfg);
#endif /* PIOS_INCLUDE_RTC */

	OPLinkSettingsInitialize();

#if defined(PIOS_INCLUDE_LED)
	PIOS_LED_Init(&pios_led_cfg);
#endif	/* PIOS_INCLUDE_LED */

	OPLinkSettingsData oplinkSettings;
#if defined(PIOS_INCLUDE_FLASH_EEPROM)
 	PIOS_EEPROM_Init(&pios_eeprom_cfg);

	/* Read the settings from flash. */
	/* NOTE: We probably need to save/restore the objID here incase the object changed but the size doesn't */
	if (PIOS_EEPROM_Load((uint8_t*)&oplinkSettings, sizeof(OPLinkSettingsData)) == 0)
		OPLinkSettingsSet(&oplinkSettings);
	else
		OPLinkSettingsGet(&oplinkSettings);
#else
	OPLinkSettingsGet(&oplinkSettings);
#endif /* PIOS_INCLUDE_FLASH_EEPROM */

	/* Initialize the task monitor library */
	TaskMonitorInitialize();

#if defined(PIOS_INCLUDE_TIM)
	/* Set up pulse timers */
	PIOS_TIM_InitClock(&tim_1_cfg);
	PIOS_TIM_InitClock(&tim_2_cfg);
	PIOS_TIM_InitClock(&tim_3_cfg);
	PIOS_TIM_InitClock(&tim_4_cfg);
#endif	/* PIOS_INCLUDE_TIM */

	/* Initialize board specific USB data */
	PIOS_USB_BOARD_DATA_Init();

	/* Flags to determine if various USB interfaces are advertised */
	bool usb_cdc_present = false;

#if defined(PIOS_INCLUDE_USB_CDC)
	if (PIOS_USB_DESC_HID_CDC_Init()) {
		PIOS_Assert(0);
	}
	usb_cdc_present = true;
#else
	if (PIOS_USB_DESC_HID_ONLY_Init()) {
		PIOS_Assert(0);
	}
#endif

	/*Initialize the USB device */
	uint32_t pios_usb_id;
	PIOS_USB_Init(&pios_usb_id, &pios_usb_main_cfg);

	/* Configure the USB HID port */
	{
		uint32_t pios_usb_hid_id;
		if (PIOS_USB_HID_Init(&pios_usb_hid_id, &pios_usb_hid_cfg, pios_usb_id)) {
			PIOS_Assert(0);
		}
		uint8_t *rx_buffer = (uint8_t *)pvPortMalloc(PIOS_COM_TELEM_USB_RX_BUF_LEN);
		uint8_t *tx_buffer = (uint8_t *)pvPortMalloc(PIOS_COM_TELEM_USB_TX_BUF_LEN);
		PIOS_Assert(rx_buffer);
		PIOS_Assert(tx_buffer);
		if (PIOS_COM_Init(&pios_com_telem_usb_id, &pios_usb_hid_com_driver, pios_usb_hid_id,
											rx_buffer, PIOS_COM_TELEM_USB_RX_BUF_LEN,
											tx_buffer, PIOS_COM_TELEM_USB_TX_BUF_LEN)) {
			PIOS_Assert(0);
		}
	}

	/* Configure the USB virtual com port (VCP) */
#if defined(PIOS_INCLUDE_USB_CDC)
	if (usb_cdc_present)
	{
		uint32_t pios_usb_cdc_id;
		if (PIOS_USB_CDC_Init(&pios_usb_cdc_id, &pios_usb_cdc_cfg, pios_usb_id)) {
			PIOS_Assert(0);
		}
		uint8_t *rx_buffer = (uint8_t *)pvPortMalloc(PIOS_COM_TELEM_VCP_RX_BUF_LEN);
		uint8_t *tx_buffer = (uint8_t *)pvPortMalloc(PIOS_COM_TELEM_VCP_TX_BUF_LEN);
		PIOS_Assert(rx_buffer);
		PIOS_Assert(tx_buffer);
		if (PIOS_COM_Init(&pios_com_telem_vcp_id, &pios_usb_cdc_com_driver, pios_usb_cdc_id,
											rx_buffer, PIOS_COM_TELEM_VCP_RX_BUF_LEN,
											tx_buffer, PIOS_COM_TELEM_VCP_TX_BUF_LEN)) {
			PIOS_Assert(0);
		}
	}
#endif

	/* Configure the telemetry serial port */
	{
		uint32_t pios_usart1_id;
		if (PIOS_USART_Init(&pios_usart1_id, &pios_usart_serial_cfg)) {
			PIOS_Assert(0);
		}
		uint8_t *rx_buffer = (uint8_t *)pvPortMalloc(PIOS_COM_TELEM_RX_BUF_LEN);
		uint8_t *tx_buffer = (uint8_t *)pvPortMalloc(PIOS_COM_TELEM_TX_BUF_LEN);
		PIOS_Assert(rx_buffer);
		PIOS_Assert(tx_buffer);
		if (PIOS_COM_Init(&pios_com_telem_uart_telem_id, &pios_usart_com_driver, pios_usart1_id,
											rx_buffer, PIOS_COM_TELEM_RX_BUF_LEN,
											tx_buffer, PIOS_COM_TELEM_TX_BUF_LEN)) {
			PIOS_Assert(0);
		}
	}

	/* Configure the flexi serial port */
	{
		uint32_t pios_usart3_id;
		if (PIOS_USART_Init(&pios_usart3_id, &pios_usart_telem_flexi_cfg)) {
			PIOS_Assert(0);
		}
		uint8_t * rx_buffer = (uint8_t *) pvPortMalloc(PIOS_COM_TELEM_RX_BUF_LEN);
		uint8_t * tx_buffer = (uint8_t *) pvPortMalloc(PIOS_COM_TELEM_TX_BUF_LEN);
		PIOS_Assert(rx_buffer);
		PIOS_Assert(tx_buffer);
		if (PIOS_COM_Init(&pios_com_telem_uart_flexi_id, &pios_usart_com_driver, pios_usart3_id,
											rx_buffer, PIOS_COM_TELEM_RX_BUF_LEN,
											tx_buffer, PIOS_COM_TELEM_TX_BUF_LEN)) {
			PIOS_Assert(0);
		}
	}

	/* Configure PPM input */
#if defined(PIOS_INCLUDE_PPM)
	if (oplinkSettings.PPM == OPLINKSETTINGS_PPM_TRUE)
	{
		uint32_t pios_ppm_id;
		PIOS_PPM_Init(&pios_ppm_id, &pios_ppm_cfg);

		if (PIOS_RCVR_Init(&pios_ppm_rcvr_id, &pios_ppm_rcvr_driver, pios_ppm_id))
			PIOS_Assert(0);
	}
#endif	/* PIOS_INCLUDE_PPM */

	/* Initalize the RFM22B radio COM device. */
#if defined(PIOS_INCLUDE_RFM22B)
	{
		extern const struct pios_rfm22b_cfg * PIOS_BOARD_HW_DEFS_GetRfm22Cfg (uint32_t board_revision);
		const struct pios_board_info * bdinfo = &pios_board_info_blob;
		const struct pios_rfm22b_cfg *pios_rfm22b_cfg = PIOS_BOARD_HW_DEFS_GetRfm22Cfg(bdinfo->board_rev);
		if (PIOS_RFM22B_Init(&pios_rfm22b_id, PIOS_RFM22_SPI_PORT, pios_rfm22b_cfg->slave_num, pios_rfm22b_cfg)) {
			PIOS_Assert(0);
		}

		uint8_t *rx_buffer = (uint8_t *) pvPortMalloc(PIOS_COM_RFM22B_RF_RX_BUF_LEN);
		uint8_t *tx_buffer = (uint8_t *) pvPortMalloc(PIOS_COM_RFM22B_RF_TX_BUF_LEN);
		PIOS_Assert(rx_buffer);
		PIOS_Assert(tx_buffer);
		if (PIOS_COM_Init(&pios_com_rfm22b_id, &pios_rfm22b_com_driver, pios_rfm22b_id,
											rx_buffer, PIOS_COM_RFM22B_RF_RX_BUF_LEN,
											tx_buffer, PIOS_COM_RFM22B_RF_TX_BUF_LEN)) {
			PIOS_Assert(0);
		}
	}

	// Initialize the packet handler
	PacketHandlerConfig pios_ph_cfg = {
		.default_destination_id = 0xffffffff, // Broadcast
		.source_id = PIOS_RFM22B_DeviceID(pios_rfm22b_id),
		.win_size = PIOS_PH_WIN_SIZE,
		.max_connections = PIOS_PH_MAX_CONNECTIONS,
	};
	pios_packet_handler = PHInitialize(&pios_ph_cfg);
#endif /* PIOS_INCLUDE_RFM22B */

	/* Remap AFIO pin */
	GPIO_PinRemapConfig( GPIO_Remap_SWJ_NoJTRST, ENABLE);

#ifdef PIOS_INCLUDE_ADC
	PIOS_ADC_Init();
#endif
 	PIOS_GPIO_Init();
}

/**
 * @}
 */

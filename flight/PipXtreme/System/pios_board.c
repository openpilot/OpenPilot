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
#include <board_hw_defs.c>

#define PIOS_COM_SERIAL_RX_BUF_LEN 192
#define PIOS_COM_SERIAL_TX_BUF_LEN 192

#define PIOS_COM_FLEXI_RX_BUF_LEN 192
#define PIOS_COM_FLEXI_TX_BUF_LEN 192

#define PIOS_COM_TELEM_USB_RX_BUF_LEN 192
#define PIOS_COM_TELEM_USB_TX_BUF_LEN 192

#define PIOS_COM_VCP_USB_RX_BUF_LEN 192
#define PIOS_COM_VCP_USB_TX_BUF_LEN 192

#define PIOS_COM_RFM22B_RF_RX_BUF_LEN 192
#define PIOS_COM_RFM22B_RF_TX_BUF_LEN 192

uint32_t pios_com_telem_usb_id;
uint32_t pios_com_vcp_usb_id;
uint32_t pios_com_usart1_id;
uint32_t pios_com_usart3_id;
uint32_t pios_com_rfm22b_id;

/**
 * PIOS_Board_Init()
 * initializes all the core subsystems on this specific hardware
 * called from System/openpilot.c
 */
void PIOS_Board_Init(void) {

	/* Delay system */
	PIOS_DELAY_Init();

	/* Set up the SPI interface to the serial flash */
	if (PIOS_SPI_Init(&pios_spi_port_id, &pios_spi_port_cfg)) {
		PIOS_Assert(0);
	}

	/* Initialize UAVObject libraries */
	EventDispatcherInitialize();
	UAVObjInitialize();

	/* Initialize watchdog as early as possible to catch faults during init */
	PIOS_WDG_Init();

	/* Initialize IAP */
	PIOS_IAP_Init();

#if defined(PIOS_INCLUDE_RTC)
	/* Initialize the real-time clock and its associated tick */
	PIOS_RTC_Init(&pios_rtc_main_cfg);
#endif

#if defined(PIOS_INCLUDE_LED)
	PIOS_LED_Init(&pios_led_cfg);
#endif	/* PIOS_INCLUDE_LED */

	/* Initialize the task monitor library */
	TaskMonitorInitialize();

#if defined(PIOS_INCLUDE_TIM)
	/* Set up pulse timers */
	PIOS_TIM_InitClock(&tim_1_cfg);
	PIOS_TIM_InitClock(&tim_2_cfg);
	PIOS_TIM_InitClock(&tim_3_cfg);
	PIOS_TIM_InitClock(&tim_4_cfg);
#endif	/* PIOS_INCLUDE_TIM */

#if defined(PIOS_INCLUDE_PACKET_HANDLER)
	pios_packet_handler = PHInitialize(&pios_ph_cfg);
#endif /* PIOS_INCLUDE_PACKET_HANDLER */

#if defined(PIOS_INCLUDE_USB)
	/* Initialize board specific USB data */
	PIOS_USB_BOARD_DATA_Init();

#if defined(PIOS_INCLUDE_USB_CDC)
	if (PIOS_USB_DESC_HID_CDC_Init()) {
		PIOS_Assert(0);
	}
#else
	if (PIOS_USB_DESC_HID_ONLY_Init()) {
		PIOS_Assert(0);
	}
#endif

	uint32_t pios_usb_id;
	PIOS_USB_Init(&pios_usb_id, &pios_usb_main_cfg);

#if defined(PIOS_INCLUDE_USB_CDC)

#if defined(PIOS_INCLUDE_COM)
	{
		uint32_t pios_usb_cdc_id;
		if (PIOS_USB_CDC_Init(&pios_usb_cdc_id, &pios_usb_cdc_cfg, pios_usb_id)) {
			PIOS_Assert(0);
		}
		uint8_t * rx_buffer = (uint8_t *) pvPortMalloc(PIOS_COM_VCP_USB_RX_BUF_LEN);
		uint8_t * tx_buffer = (uint8_t *) pvPortMalloc(PIOS_COM_VCP_USB_TX_BUF_LEN);
		PIOS_Assert(rx_buffer);
		PIOS_Assert(tx_buffer);
		if (PIOS_COM_Init(&pios_com_vcp_usb_id, &pios_usb_cdc_com_driver, pios_usb_cdc_id,
											rx_buffer, PIOS_COM_VCP_USB_RX_BUF_LEN,
											tx_buffer, PIOS_COM_VCP_USB_TX_BUF_LEN)) {
			PIOS_Assert(0);
		}
	}
#endif	/* PIOS_INCLUDE_COM */

#endif	/* PIOS_INCLUDE_USB_CDC */

#if defined(PIOS_INCLUDE_USB_HID)

	/* Configure the usb HID port */
#if defined(PIOS_INCLUDE_COM)
	{
		uint32_t pios_usb_hid_id;
		if (PIOS_USB_HID_Init(&pios_usb_hid_id, &pios_usb_hid_cfg, pios_usb_id)) {
			PIOS_Assert(0);
		}
		uint8_t * rx_buffer = (uint8_t *) pvPortMalloc(PIOS_COM_TELEM_USB_RX_BUF_LEN);
		uint8_t * tx_buffer = (uint8_t *) pvPortMalloc(PIOS_COM_TELEM_USB_TX_BUF_LEN);
		PIOS_Assert(rx_buffer);
		PIOS_Assert(tx_buffer);
		if (PIOS_COM_Init(&pios_com_telem_usb_id, &pios_usb_hid_com_driver, pios_usb_hid_id,
											rx_buffer, PIOS_COM_TELEM_USB_RX_BUF_LEN,
											tx_buffer, PIOS_COM_TELEM_USB_TX_BUF_LEN)) {
			PIOS_Assert(0);
		}
	}
#endif	/* PIOS_INCLUDE_COM */

#endif	/* PIOS_INCLUDE_USB_HID */

#endif	/* PIOS_INCLUDE_USB */

	/* Configure USART1 */
	{
		uint32_t pios_usart1_id;
		if (PIOS_USART_Init(&pios_usart1_id, &pios_usart_serial_cfg)) {
			PIOS_Assert(0);
		}

		uint8_t * rx_buffer = (uint8_t *) pvPortMalloc(PIOS_COM_SERIAL_RX_BUF_LEN);
		uint8_t * tx_buffer = (uint8_t *) pvPortMalloc(PIOS_COM_SERIAL_TX_BUF_LEN);
		PIOS_Assert(rx_buffer);
		PIOS_Assert(tx_buffer);
		if (PIOS_COM_Init(&pios_com_usart1_id, &pios_usart_com_driver, pios_usart1_id,
											rx_buffer, PIOS_COM_SERIAL_RX_BUF_LEN,
											tx_buffer, PIOS_COM_SERIAL_TX_BUF_LEN)) {
			PIOS_Assert(0);
		}
	}

	/* Configure USART3 */
	{
		uint32_t pios_usart3_id;
		if (PIOS_USART_Init(&pios_usart3_id, &pios_usart_telem_flexi_cfg)) {
			PIOS_Assert(0);
		}
		uint8_t * rx_buffer = (uint8_t *) pvPortMalloc(PIOS_COM_FLEXI_RX_BUF_LEN);
		uint8_t * tx_buffer = (uint8_t *) pvPortMalloc(PIOS_COM_FLEXI_TX_BUF_LEN);
		PIOS_Assert(rx_buffer);
		PIOS_Assert(tx_buffer);
		if (PIOS_COM_Init(&pios_com_usart3_id, &pios_usart_com_driver, pios_usart3_id,
											rx_buffer, PIOS_COM_FLEXI_RX_BUF_LEN,
											tx_buffer, PIOS_COM_FLEXI_TX_BUF_LEN)) {
			PIOS_Assert(0);
		}
	}
	PIOS_COM_SendString(PIOS_COM_DEBUG, "Hello DEBUG\n\r");
	PIOS_COM_SendString(PIOS_COM_FLEXI, "Hello Flexi\n\r");
	PIOS_COM_SendString(PIOS_COM_TELEM_SERIAL, "Hello Telem Serial\n\r");
	PIOS_COM_SendString(PIOS_COM_VCP_USB, "Hello VCP\n\r");

#if defined(PIOS_INCLUDE_RFM22B)
	/* Initalize the RFM22B radio COM device. */
	{
		uint32_t pios_rfm22b_id;
		if (PIOS_RFM22B_Init(&pios_rfm22b_id, &pios_rfm22b_cfg)) {
			PIOS_Assert(0);
		}
		uint8_t * rx_buffer = (uint8_t *) pvPortMalloc(PIOS_COM_RFM22B_RF_RX_BUF_LEN);
		uint8_t * tx_buffer = (uint8_t *) pvPortMalloc(PIOS_COM_RFM22B_RF_TX_BUF_LEN);
		PIOS_Assert(rx_buffer);
		PIOS_Assert(tx_buffer);
		if (PIOS_COM_Init(&pios_com_rfm22b_id, &pios_rfm22b_com_driver, pios_rfm22b_id,
											rx_buffer, PIOS_COM_RFM22B_RF_RX_BUF_LEN,
											tx_buffer, PIOS_COM_RFM22B_RF_TX_BUF_LEN)) {
			PIOS_Assert(0);
		}
	}
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

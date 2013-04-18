
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
#include <oplogsettings.h>
#include <board_hw_defs.c>

#define PIOS_COM_TELEM_RX_BUF_LEN 256
#define PIOS_COM_TELEM_TX_BUF_LEN 256

#define PIOS_COM_TELEM_USB_RX_BUF_LEN 256
#define PIOS_COM_TELEM_USB_TX_BUF_LEN 256

#define PIOS_COM_TELEM_VCP_RX_BUF_LEN 256
#define PIOS_COM_TELEM_VCP_TX_BUF_LEN 256

uint32_t pios_com_telem_usb_id = 0;
uint32_t pios_com_telem_vcp_id = 0;
uint32_t pios_com_telem_uart_telem_id = 0;
uint32_t pios_com_telem_uart_flexi_id = 0;
uint32_t pios_com_telemetry_id = 0;
uint32_t pios_i2c_flexi_adapter_id=0;
uint32_t pios_com_radio_id = 0;
#if defined(PIOS_INCLUDE_PPM)
uint32_t pios_ppm_rcvr_id = 0;
#endif
#if defined(PIOS_INCLUDE_PPM_OUT)
uint32_t pios_ppm_out_id = 0;
#endif


/**
 * PIOS_Board_Init()
 * initializes all the core subsystems on this specific hardware
 * called from System/openpilot.c
 */
void PIOS_Board_Init(void) {

	/* Delay system */
	PIOS_DELAY_Init();

#if defined(PIOS_INCLUDE_LED)
	PIOS_LED_Init(&pios_led_cfg);
	PIOS_LED_Toggle(PIOS_LED_LINK);
#endif	/* PIOS_INCLUDE_LED */

	/* Initialize UAVObject libraries */
	EventDispatcherInitialize();
	UAVObjInitialize();


#ifdef PIOS_INCLUDE_WDG
	/* Initialize watchdog as early as possible to catch faults during init */
	//PIOS_WDG_Init();
#endif /* PIOS_INCLUDE_WDG */

#if defined(PIOS_INCLUDE_RTC)
	/* Initialize the real-time clock and its associated tick */
	PIOS_RTC_Init(&pios_rtc_main_cfg);
#endif /* PIOS_INCLUDE_RTC */

	OPLogSettingsInitialize();

	OPLogSettingsData oplogSettings;
#if defined(PIOS_INCLUDE_FLASH_EEPROM)
 	PIOS_EEPROM_Init(&pios_eeprom_cfg);

	/* Read the settings from flash. */
	/* NOTE: We probably need to save/restore the objID here incase the object changed but the size doesn't */
	if (PIOS_EEPROM_Load((uint8_t*)&oplogSettings, sizeof(OPLogSettingsData)) == 0)
		OPLogSettingsSet(&oplogSettings);
	else
		OPLogSettingsGet(&oplogSettings);
#else
	OPLogSettingsGet(&oplogSettings);
#endif /* PIOS_INCLUDE_FLASH_EEPROM */

	/* Initialize the task monitor library */
	TaskMonitorInitialize();

#if defined(PIOS_INCLUDE_SDCARD2)
#if defined(PIOS_INCLUDE_SPI)
	/* Set up the SPI interface to the SD card */
	if (PIOS_SPI_Init(&pios_spi_sdcard_id, &pios_spi_sdcard_cfg)) {
		PIOS_Assert(0);
	}

	/* Enable and mount the SDCard */
	PIOS_SDCARD_Init(pios_spi_sdcard_id);
	if(PIOS_SDCARD_PowerOn())
	{
		PIOS_Assert(0);
	}
	PIOS_SDCARD_MountFS(1);
#endif /* PIOS_INCLUDE_SPI */
#endif

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
#ifndef PIOS_RFM22B_DEBUG_ON_TELEM
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
#endif

	/* Configure PPM input */
	switch (oplogSettings.PPM)	{
#if defined(PIOS_INCLUDE_PPM)
	case OPLOGSETTINGS_PPM_INPUT:
	{
		uint32_t pios_ppm_id;
		PIOS_PPM_Init(&pios_ppm_id, &pios_ppm_cfg);

		if (PIOS_RCVR_Init(&pios_ppm_rcvr_id, &pios_ppm_rcvr_driver, pios_ppm_id))
			PIOS_Assert(0);

		/*if (PIOS_I2C_Init(&pios_i2c_flexi_adapter_id, &pios_i2c_flexi_adapter_cfg)) {
			PIOS_Assert(0);
			}*/

		break;
	}
#endif	/* PIOS_INCLUDE_PPM */

#if defined(PIOS_INCLUDE_PPM_OUT)
	case OPLOGSETTINGS_PPM_OUTPUT:
		PIOS_PPM_Out_Init(&pios_ppm_out_id, &pios_ppm_out_cfg);
		break;
#endif	/* PIOS_INCLUDE_PPM_OUT */

	default:
	{
#if defined(PIOS_INCLUDE_I2C) && defined(PIOS_INCLUDE_MPU6050)
		{
			if (PIOS_I2C_Init(&pios_i2c_flexi_adapter_id, &pios_i2c_flexi_adapter_cfg)) {
				PIOS_Assert(0);
			}
		}
#else	
		/* Configure the flexi serial port if PPM not selected */
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
#endif
	}
	}

	/* Remap AFIO pin */
	GPIO_PinRemapConfig( GPIO_Remap_SWJ_NoJTRST, ENABLE);

#ifdef PIOS_INCLUDE_ADC
	PIOS_ADC_Init(&pios_adc_cfg);
#endif
 	PIOS_GPIO_Init();
}

/**
 * @}
 */


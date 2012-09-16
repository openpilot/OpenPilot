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
#include <pipxsettings.h>
#include <board_hw_defs.c>

#define PIOS_COM_SERIAL_RX_BUF_LEN 128
#define PIOS_COM_SERIAL_TX_BUF_LEN 128

#define PIOS_COM_FLEXI_RX_BUF_LEN 128
#define PIOS_COM_FLEXI_TX_BUF_LEN 128

#define PIOS_COM_TELEM_USB_RX_BUF_LEN 128
#define PIOS_COM_TELEM_USB_TX_BUF_LEN 128

#define PIOS_COM_VCP_USB_RX_BUF_LEN 128
#define PIOS_COM_VCP_USB_TX_BUF_LEN 128

#define PIOS_COM_RFM22B_RF_RX_BUF_LEN 128
#define PIOS_COM_RFM22B_RF_TX_BUF_LEN 128

uint32_t pios_com_telem_usb_id = 0;
uint32_t pios_com_telemetry_id;
uint32_t pios_com_flexi_id;
uint32_t pios_com_vcp_id;
uint32_t pios_com_uavtalk_com_id = 0;
uint32_t pios_com_gcs_com_id = 0;
uint32_t pios_com_trans_com_id = 0;
uint32_t pios_com_debug_id = 0;
uint32_t pios_com_rfm22b_id = 0;
uint32_t pios_rfm22b_id = 0;
uint32_t pios_ppm_rcvr_id = 0;

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

#ifdef PIOS_INCLUDE_WDG
	/* Initialize watchdog as early as possible to catch faults during init */
	PIOS_WDG_Init();
#endif /* PIOS_INCLUDE_WDG */

#if defined(PIOS_INCLUDE_RTC)
	/* Initialize the real-time clock and its associated tick */
	PIOS_RTC_Init(&pios_rtc_main_cfg);
#endif /* PIOS_INCLUDE_RTC */

	PipXSettingsInitialize();

#if defined(PIOS_INCLUDE_LED)
	PIOS_LED_Init(&pios_led_cfg);
#endif	/* PIOS_INCLUDE_LED */

	PipXSettingsData pipxSettings;
#if defined(PIOS_INCLUDE_FLASH_EEPROM)
 	PIOS_EEPROM_Init(&pios_eeprom_cfg);

	/* Read the settings from flash. */
	/* NOTE: We probably need to save/restore the objID here incase the object changed but the size doesn't */
	if (PIOS_EEPROM_Load((uint8_t*)&pipxSettings, sizeof(PipXSettingsData)) == 0)
		PipXSettingsSet(&pipxSettings);
	else
		PipXSettingsGet(&pipxSettings);
#else
	PipXSettingsGet(&pipxSettings);
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

#if defined(PIOS_INCLUDE_PACKET_HANDLER)
	pios_packet_handler = PHInitialize(&pios_ph_cfg);
#endif /* PIOS_INCLUDE_PACKET_HANDLER */

#if defined(PIOS_INCLUDE_USB)
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

	uint32_t pios_usb_id;
	PIOS_USB_Init(&pios_usb_id, &pios_usb_main_cfg);

#if defined(PIOS_INCLUDE_USB_CDC)
	if (!usb_cdc_present) {
		/* Force VCP port function to disabled if we haven't advertised VCP in our USB descriptor */
		pipxSettings.VCPConfig = PIPXSETTINGS_VCPCONFIG_DISABLED;
	}

	switch (pipxSettings.VCPConfig)
	{
	case PIPXSETTINGS_VCPCONFIG_SERIAL:
	case PIPXSETTINGS_VCPCONFIG_DEBUG:
	{
		uint32_t pios_usb_cdc_id;
		if (PIOS_USB_CDC_Init(&pios_usb_cdc_id, &pios_usb_cdc_cfg, pios_usb_id)) {
			PIOS_Assert(0);
		}
		uint8_t * rx_buffer = (uint8_t *) pvPortMalloc(PIOS_COM_VCP_USB_RX_BUF_LEN);
		uint8_t * tx_buffer = (uint8_t *) pvPortMalloc(PIOS_COM_VCP_USB_TX_BUF_LEN);
		PIOS_Assert(rx_buffer);
		PIOS_Assert(tx_buffer);
		if (PIOS_COM_Init(&pios_com_vcp_id, &pios_usb_cdc_com_driver, pios_usb_cdc_id,
											rx_buffer, PIOS_COM_VCP_USB_RX_BUF_LEN,
											tx_buffer, PIOS_COM_VCP_USB_TX_BUF_LEN)) {
			PIOS_Assert(0);
		}
		switch (pipxSettings.VCPConfig)
		{
		case PIPXSETTINGS_VCPCONFIG_SERIAL:
			pios_com_trans_com_id = pios_com_vcp_id;
			break;
		case PIPXSETTINGS_VCPCONFIG_DEBUG:
			pios_com_debug_id = pios_com_vcp_id;
			break;
		}
		break;
	}
	case PIPXSETTINGS_VCPCONFIG_DISABLED:
		break;
	}
#endif

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

	/* Configure USART1 (telemetry port) */
	switch (pipxSettings.TelemetryConfig)
	{
	case PIPXSETTINGS_TELEMETRYCONFIG_SERIAL:
	case PIPXSETTINGS_TELEMETRYCONFIG_UAVTALK:
	case PIPXSETTINGS_TELEMETRYCONFIG_GCS:
	case PIPXSETTINGS_TELEMETRYCONFIG_DEBUG:
	{
		uint32_t pios_usart1_id;
		if (PIOS_USART_Init(&pios_usart1_id, &pios_usart_serial_cfg)) {
			PIOS_Assert(0);
		}
		uint8_t * rx_buffer = (uint8_t *) pvPortMalloc(PIOS_COM_SERIAL_RX_BUF_LEN);
		uint8_t * tx_buffer = (uint8_t *) pvPortMalloc(PIOS_COM_SERIAL_TX_BUF_LEN);
		PIOS_Assert(rx_buffer);
		PIOS_Assert(tx_buffer);
		if (PIOS_COM_Init(&pios_com_telemetry_id, &pios_usart_com_driver, pios_usart1_id,
											rx_buffer, PIOS_COM_SERIAL_RX_BUF_LEN,
											tx_buffer, PIOS_COM_SERIAL_TX_BUF_LEN)) {
			PIOS_Assert(0);
		}
		switch (pipxSettings.TelemetryConfig)
		{
		case PIPXSETTINGS_TELEMETRYCONFIG_SERIAL:
			pios_com_trans_com_id = pios_com_telemetry_id;
			break;
		case PIPXSETTINGS_TELEMETRYCONFIG_UAVTALK:
			pios_com_uavtalk_com_id = pios_com_telemetry_id;
			break;
		case PIPXSETTINGS_TELEMETRYCONFIG_GCS:
			pios_com_gcs_com_id = pios_com_telemetry_id;
			break;
		case PIPXSETTINGS_TELEMETRYCONFIG_DEBUG:
			pios_com_debug_id = pios_com_telemetry_id;
			break;
		}
		break;
	}
	case PIPXSETTINGS_TELEMETRYCONFIG_DISABLED:
		break;
	}

	/* Configure USART3 */
	switch (pipxSettings.FlexiConfig)
	{
	case PIPXSETTINGS_FLEXICONFIG_SERIAL:
	case PIPXSETTINGS_FLEXICONFIG_UAVTALK:
	case PIPXSETTINGS_FLEXICONFIG_GCS:
	case PIPXSETTINGS_FLEXICONFIG_DEBUG:
	{
		uint32_t pios_usart3_id;
		if (PIOS_USART_Init(&pios_usart3_id, &pios_usart_telem_flexi_cfg)) {
			PIOS_Assert(0);
		}
		uint8_t * rx_buffer = (uint8_t *) pvPortMalloc(PIOS_COM_FLEXI_RX_BUF_LEN);
		uint8_t * tx_buffer = (uint8_t *) pvPortMalloc(PIOS_COM_FLEXI_TX_BUF_LEN);
		PIOS_Assert(rx_buffer);
		PIOS_Assert(tx_buffer);
		if (PIOS_COM_Init(&pios_com_flexi_id, &pios_usart_com_driver, pios_usart3_id,
											rx_buffer, PIOS_COM_FLEXI_RX_BUF_LEN,
											tx_buffer, PIOS_COM_FLEXI_TX_BUF_LEN)) {
			PIOS_Assert(0);
		}
		switch (pipxSettings.FlexiConfig)
		{
		case PIPXSETTINGS_FLEXICONFIG_SERIAL:
			pios_com_trans_com_id = pios_com_flexi_id;
			break;
		case PIPXSETTINGS_FLEXICONFIG_UAVTALK:
			pios_com_uavtalk_com_id = pios_com_flexi_id;
			break;
		case PIPXSETTINGS_FLEXICONFIG_GCS:
			pios_com_gcs_com_id = pios_com_flexi_id;
			break;
		case PIPXSETTINGS_FLEXICONFIG_DEBUG:
			pios_com_debug_id = pios_com_flexi_id;
			break;
		}
		break;
	}
	case PIPXSETTINGS_FLEXICONFIG_PPM_IN:
#if defined(PIOS_INCLUDE_PPM)
	{
			uint32_t pios_ppm_id;
			PIOS_PPM_Init(&pios_ppm_id, &pios_ppm_cfg);

			if (PIOS_RCVR_Init(&pios_ppm_rcvr_id, &pios_ppm_rcvr_driver, pios_ppm_id)) {
				PIOS_Assert(0);
			}
		}
#endif	/* PIOS_INCLUDE_PPM */
	break;
	case PIPXSETTINGS_FLEXICONFIG_PPM_OUT:
	case PIPXSETTINGS_FLEXICONFIG_RSSI:
	case PIPXSETTINGS_FLEXICONFIG_DISABLED:
		break;
	}

#if defined(PIOS_INCLUDE_RFM22B)
	struct pios_rfm22b_cfg pios_rfm22b_cfg = {
		.frequencyHz = 434000000,
		.minFrequencyHz = 434000000 - 2000000,
		.maxFrequencyHz = 434000000 + 2000000,
		.RFXtalCap = 0x7f,
		.maxRFBandwidth = 128000,
		.maxTxPower = RFM22_tx_pwr_txpow_7, // +20dBm .. 100mW
	};

	/* Retrieve hardware settings. */
	pios_rfm22b_cfg.frequencyHz = pipxSettings.Frequency;
	pios_rfm22b_cfg.RFXtalCap = pipxSettings.FrequencyCalibration;
	switch (pipxSettings.RFSpeed)
	{
	case PIPXSETTINGS_RFSPEED_2400:
		pios_rfm22b_cfg.maxRFBandwidth = 2000;
		break;
	case PIPXSETTINGS_RFSPEED_4800:
		pios_rfm22b_cfg.maxRFBandwidth = 4000;
		break;
	case PIPXSETTINGS_RFSPEED_9600:
		pios_rfm22b_cfg.maxRFBandwidth = 9600;
		break;
	case PIPXSETTINGS_RFSPEED_19200:
		pios_rfm22b_cfg.maxRFBandwidth = 19200;
		break;
	case PIPXSETTINGS_RFSPEED_38400:
		pios_rfm22b_cfg.maxRFBandwidth = 32000;
		break;
	case PIPXSETTINGS_RFSPEED_57600:
		pios_rfm22b_cfg.maxRFBandwidth = 64000;
		break;
	case PIPXSETTINGS_RFSPEED_115200:
		pios_rfm22b_cfg.maxRFBandwidth = 128000;
		break;
	}
	switch (pipxSettings.MaxRFPower)
	{
	case PIPXSETTINGS_MAXRFPOWER_125:
		pios_rfm22b_cfg.maxTxPower = RFM22_tx_pwr_txpow_0;
		break;
	case PIPXSETTINGS_MAXRFPOWER_16:
		pios_rfm22b_cfg.maxTxPower = RFM22_tx_pwr_txpow_1;
		break;
	case PIPXSETTINGS_MAXRFPOWER_316:
		pios_rfm22b_cfg.maxTxPower = RFM22_tx_pwr_txpow_2;
		break;
	case PIPXSETTINGS_MAXRFPOWER_63:
		pios_rfm22b_cfg.maxTxPower = RFM22_tx_pwr_txpow_3;
		break;
	case PIPXSETTINGS_MAXRFPOWER_126:
		pios_rfm22b_cfg.maxTxPower = RFM22_tx_pwr_txpow_4;
		break;
	case PIPXSETTINGS_MAXRFPOWER_25:
		pios_rfm22b_cfg.maxTxPower = RFM22_tx_pwr_txpow_5;
		break;
	case PIPXSETTINGS_MAXRFPOWER_50:
		pios_rfm22b_cfg.maxTxPower = RFM22_tx_pwr_txpow_6;
		break;
	case PIPXSETTINGS_MAXRFPOWER_100:
		pios_rfm22b_cfg.maxTxPower = RFM22_tx_pwr_txpow_7;
		break;
	}

	/* Initalize the RFM22B radio COM device. */
	{
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

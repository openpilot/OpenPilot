/**
 ******************************************************************************
 *
 * @file       pios_board.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      Defines board specific static initializers for hardware for the OPOSD board.
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
#include <fifo_buffer.h>
#include <openpilot.h>
#include <uavobjectsinit.h>
#include <hwsettings.h>
#include <manualcontrolsettings.h>
#include <gcsreceiver.h>

/* Private macro -------------------------------------------------------------*/
#define countof(a)   (sizeof(a) / sizeof(*(a)))

/* Private variables ---------------------------------------------------------*/

#if defined(PIOS_INCLUDE_ADC)
#include "pios_adc_priv.h"
void PIOS_ADC_DMC_irq_handler(void);
void DMA2_Stream4_IRQHandler(void) __attribute__((alias("PIOS_ADC_DMC_irq_handler")));
struct pios_adc_cfg pios_adc_cfg = {
	.adc_dev = ADC1,
	.dma = {
		.irq = {
			.flags   = (DMA_FLAG_TCIF4 | DMA_FLAG_TEIF4 | DMA_FLAG_HTIF4),
			.init    = {
				.NVIC_IRQChannel                   = DMA2_Stream4_IRQn,
				.NVIC_IRQChannelPreemptionPriority = PIOS_IRQ_PRIO_LOW,
				.NVIC_IRQChannelSubPriority        = 0,
				.NVIC_IRQChannelCmd                = ENABLE,
			},
		},
		.rx = {
			.channel = DMA2_Stream4,
			.init    = {
				.DMA_Channel                    = DMA_Channel_0,
				.DMA_PeripheralBaseAddr = (uint32_t) & ADC1->DR
			},
		}
	},
	.half_flag = DMA_IT_HTIF4,
	.full_flag = DMA_IT_TCIF4,

};
void PIOS_ADC_DMC_irq_handler(void)
{
	/* Call into the generic code to handle the IRQ for this specific device */
	PIOS_ADC_DMA_Handler();
}

#endif


static void Clock(uint32_t spektrum_id);


#define PIOS_COM_TELEM_RF_RX_BUF_LEN 512
#define PIOS_COM_TELEM_RF_TX_BUF_LEN 512

#define PIOS_COM_AUX_RX_BUF_LEN 512
#define PIOS_COM_AUX_TX_BUF_LEN 512

#define PIOS_COM_GPS_RX_BUF_LEN 32

#define PIOS_COM_TELEM_USB_RX_BUF_LEN 65
#define PIOS_COM_TELEM_USB_TX_BUF_LEN 65

#define PIOS_COM_BRIDGE_RX_BUF_LEN 65
#define PIOS_COM_BRIDGE_TX_BUF_LEN 12

uint32_t pios_com_aux_id;
uint32_t pios_com_gps_id;
uint32_t pios_com_telem_usb_id;
uint32_t pios_com_telem_rf_id;



void PIOS_Board_Init(void) {

	// Delay system
	PIOS_DELAY_Init();

	PIOS_LED_Init(&pios_led_cfg);

#if defined(PIOS_INCLUDE_SPI)
	/* Set up the SPI interface to the SD card */
	if (PIOS_SPI_Init(&pios_spi_sdcard_id, &pios_spi_sdcard_cfg)) {
		PIOS_Assert(0);
	}

	/* Enable and mount the SDCard */
	PIOS_SDCARD_Init(pios_spi_sdcard_id);
	PIOS_SDCARD_MountFS(0);
#endif /* PIOS_INCLUDE_SPI */

	/* Initialize UAVObject libraries */
	EventDispatcherInitialize();
	UAVObjInitialize();

	HwSettingsInitialize();

	/* Initialize the alarms library */
	AlarmsInitialize();

	/* Initialize the task monitor library */
	TaskMonitorInitialize();

	/* IAP System Setup */
	PIOS_IAP_Init();
	uint16_t boot_count = PIOS_IAP_ReadBootCount();
	if (boot_count < 3) {
		PIOS_IAP_WriteBootCount(++boot_count);
		AlarmsClear(SYSTEMALARMS_ALARM_BOOTFAULT);
	} else {
		/* Too many failed boot attempts, force hwsettings to defaults */
		HwSettingsSetDefaults(HwSettingsHandle(), 0);
		AlarmsSet(SYSTEMALARMS_ALARM_BOOTFAULT, SYSTEMALARMS_ALARM_CRITICAL);
	}

#if defined(PIOS_INCLUDE_RTC)
	/* Initialize the real-time clock and its associated tick */
	PIOS_RTC_Init(&pios_rtc_main_cfg);
	if (!PIOS_RTC_RegisterTickCallback(Clock, 0)) {
		PIOS_DEBUG_Assert(0);
	}
#endif

#if defined(PIOS_INCLUDE_USB)
	/* Initialize board specific USB data */
	PIOS_USB_BOARD_DATA_Init();

	/* Flags to determine if various USB interfaces are advertised */
	bool usb_hid_present = false;
	bool usb_cdc_present = false;

#if defined(PIOS_INCLUDE_USB_CDC)
	if (PIOS_USB_DESC_HID_CDC_Init()) {
		PIOS_Assert(0);
	}
	usb_hid_present = true;
	usb_cdc_present = true;
#else
	if (PIOS_USB_DESC_HID_ONLY_Init()) {
		PIOS_Assert(0);
	}
	usb_hid_present = true;
#endif

	uint32_t pios_usb_id;
	PIOS_USB_Init(&pios_usb_id, &pios_usb_main_cfg);

#if defined(PIOS_INCLUDE_USB_CDC)

	uint8_t hwsettings_usb_vcpport;
	/* Configure the USB VCP port */
	HwSettingsUSB_VCPPortGet(&hwsettings_usb_vcpport);

	if (!usb_cdc_present) {
		/* Force VCP port function to disabled if we haven't advertised VCP in our USB descriptor */
		hwsettings_usb_vcpport = HWSETTINGS_USB_VCPPORT_DISABLED;
	}

	switch (hwsettings_usb_vcpport) {
	case HWSETTINGS_USB_VCPPORT_DISABLED:
		break;
	case HWSETTINGS_USB_VCPPORT_USBTELEMETRY:
#if defined(PIOS_INCLUDE_COM)
		{
			uint32_t pios_usb_cdc_id;
			if (PIOS_USB_CDC_Init(&pios_usb_cdc_id, &pios_usb_cdc_cfg, pios_usb_id)) {
				PIOS_Assert(0);
			}
			uint8_t * rx_buffer = (uint8_t *) pvPortMalloc(PIOS_COM_TELEM_USB_RX_BUF_LEN);
			uint8_t * tx_buffer = (uint8_t *) pvPortMalloc(PIOS_COM_TELEM_USB_TX_BUF_LEN);
			PIOS_Assert(rx_buffer);
			PIOS_Assert(tx_buffer);
			if (PIOS_COM_Init(&pios_com_telem_usb_id, &pios_usb_cdc_com_driver, pios_usb_cdc_id,
						rx_buffer, PIOS_COM_TELEM_USB_RX_BUF_LEN,
						tx_buffer, PIOS_COM_TELEM_USB_TX_BUF_LEN)) {
				PIOS_Assert(0);
			}
		}
#endif	/* PIOS_INCLUDE_COM */
		break;
	case HWSETTINGS_USB_VCPPORT_COMBRIDGE:
#if defined(PIOS_INCLUDE_COM)
		{
			uint32_t pios_usb_cdc_id;
			if (PIOS_USB_CDC_Init(&pios_usb_cdc_id, &pios_usb_cdc_cfg, pios_usb_id)) {
				PIOS_Assert(0);
			}
			uint8_t * rx_buffer = (uint8_t *) pvPortMalloc(PIOS_COM_BRIDGE_RX_BUF_LEN);
			uint8_t * tx_buffer = (uint8_t *) pvPortMalloc(PIOS_COM_BRIDGE_TX_BUF_LEN);
			PIOS_Assert(rx_buffer);
			PIOS_Assert(tx_buffer);
			if (PIOS_COM_Init(&pios_com_vcp_id, &pios_usb_cdc_com_driver, pios_usb_cdc_id,
						rx_buffer, PIOS_COM_BRIDGE_RX_BUF_LEN,
						tx_buffer, PIOS_COM_BRIDGE_TX_BUF_LEN)) {
				PIOS_Assert(0);
			}
		}
#endif	/* PIOS_INCLUDE_COM */
		break;
	}
#endif	/* PIOS_INCLUDE_USB_CDC */

#if defined(PIOS_INCLUDE_USB_HID)
	/* Configure the usb HID port */
	uint8_t hwsettings_usb_hidport;
	HwSettingsUSB_HIDPortGet(&hwsettings_usb_hidport);

	if (!usb_hid_present) {
		/* Force HID port function to disabled if we haven't advertised HID in our USB descriptor */
		hwsettings_usb_hidport = HWSETTINGS_USB_HIDPORT_DISABLED;
	}

	switch (hwsettings_usb_hidport) {
	case HWSETTINGS_USB_HIDPORT_DISABLED:
		break;
	case HWSETTINGS_USB_HIDPORT_USBTELEMETRY:
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
		break;
	}

#endif	/* PIOS_INCLUDE_USB_HID */

	if (usb_hid_present || usb_cdc_present) {
		PIOS_USBHOOK_Activate();
	}
#endif	/* PIOS_INCLUDE_USB */

#if defined(PIOS_INCLUDE_COM)
#if defined(PIOS_INCLUDE_GPS)

	uint32_t pios_usart_gps_id;
	if (PIOS_USART_Init(&pios_usart_gps_id, &pios_usart_gps_cfg)) {
		PIOS_Assert(0);
	}

	uint8_t * rx_buffer = (uint8_t *) pvPortMalloc(PIOS_COM_GPS_RX_BUF_LEN);
	PIOS_Assert(rx_buffer);
	if (PIOS_COM_Init(&pios_com_gps_id, &pios_usart_com_driver, pios_usart_gps_id,
					  rx_buffer, PIOS_COM_GPS_RX_BUF_LEN,
					  NULL, 0)) {
		PIOS_Assert(0);
	}

#endif	/* PIOS_INCLUDE_GPS */

#if defined(PIOS_INCLUDE_COM_AUX)
	{
		uint32_t pios_usart_aux_id;

		if (PIOS_USART_Init(&pios_usart_aux_id, &pios_usart_aux_cfg)) {
			PIOS_DEBUG_Assert(0);
		}

		uint8_t * rx_buffer = (uint8_t *) pvPortMalloc(PIOS_COM_AUX_RX_BUF_LEN);
		uint8_t * tx_buffer = (uint8_t *) pvPortMalloc(PIOS_COM_AUX_TX_BUF_LEN);
		PIOS_Assert(rx_buffer);
		PIOS_Assert(tx_buffer);

		if (PIOS_COM_Init(&pios_com_aux_id, &pios_usart_com_driver, pios_usart_aux_id,
						  rx_buffer, PIOS_COM_AUX_RX_BUF_LEN,
						  tx_buffer, PIOS_COM_AUX_TX_BUF_LEN)) {
			PIOS_DEBUG_Assert(0);
		}
	}
#else
	pios_com_aux_id = 0;
#endif  /* PIOS_INCLUDE_COM_AUX */

#if defined(PIOS_INCLUDE_COM_TELEM)
	{ /* Eventually add switch for this port function */
		uint32_t pios_usart_telem_rf_id;
		if (PIOS_USART_Init(&pios_usart_telem_rf_id, &pios_usart_telem_main_cfg)) {
			PIOS_Assert(0);
		}

		uint8_t * rx_buffer = (uint8_t *) pvPortMalloc(PIOS_COM_TELEM_RF_RX_BUF_LEN);
		uint8_t * tx_buffer = (uint8_t *) pvPortMalloc(PIOS_COM_TELEM_RF_TX_BUF_LEN);
		PIOS_Assert(rx_buffer);
		PIOS_Assert(tx_buffer);
		if (PIOS_COM_Init(&pios_com_telem_rf_id, &pios_usart_com_driver, pios_usart_telem_rf_id,
						  rx_buffer, PIOS_COM_TELEM_RF_RX_BUF_LEN,
						  tx_buffer, PIOS_COM_TELEM_RF_TX_BUF_LEN)) {
			PIOS_Assert(0);
		}
	}
#else
	pios_com_telem_rf_id = 0;
#endif	/* PIOS_INCLUDE_COM_TELEM */

#endif	/* PIOS_INCLUDE_COM */


	/* Configure FlexiPort */

/*	uint8_t hwsettings_rv_flexiport;
	HwSettingsRV_FlexiPortGet(&hwsettings_rv_flexiport);

	switch (hwsettings_rv_flexiport) {
		case HWSETTINGS_RV_FLEXIPORT_DISABLED:
			break;
		case HWSETTINGS_RV_FLEXIPORT_I2C:*/
#if defined(PIOS_INCLUDE_I2C)
		{
			if (PIOS_I2C_Init(&pios_i2c_flexiport_adapter_id, &pios_i2c_flexiport_adapter_cfg)) {
				PIOS_Assert(0);
			}
		}
#endif	/* PIOS_INCLUDE_I2C */
/*			break;

		case HWSETTINGS_RV_FLEXIPORT_DSM2:
		case HWSETTINGS_RV_FLEXIPORT_DSMX10BIT:
		case HWSETTINGS_RV_FLEXIPORT_DSMX11BIT:
		{
			enum pios_dsm_proto proto;
			switch (hwsettings_rv_flexiport) {
				case HWSETTINGS_RV_FLEXIPORT_DSM2:
					proto = PIOS_DSM_PROTO_DSM2;
					break;
				case HWSETTINGS_RV_FLEXIPORT_DSMX10BIT:
					proto = PIOS_DSM_PROTO_DSMX10BIT;
					break;
				case HWSETTINGS_RV_FLEXIPORT_DSMX11BIT:
					proto = PIOS_DSM_PROTO_DSMX11BIT;
					break;
				default:
					PIOS_Assert(0);
					break;
			}
			//TODO: Define the various Channelgroup for Revo dsm inputs and handle here
			PIOS_Board_configure_dsm(&pios_usart_dsm_flexi_cfg, &pios_dsm_flexi_cfg,
											 &pios_usart_com_driver, &proto, MANUALCONTROLSETTINGS_CHANNELGROUPS_DSMMAINPORT,&hwsettings_DSMxBind);
		}
			break;
		case HWSETTINGS_RV_FLEXIPORT_COMAUX:
			PIOS_Board_configure_com(&pios_usart_flexi_cfg, PIOS_COM_AUX_RX_BUF_LEN, PIOS_COM_AUX_TX_BUF_LEN, &pios_usart_com_driver, &pios_com_aux_id);
			break;
		case HWSETTINGS_RV_FLEXIPORT_COMBRIDGE:
			PIOS_Board_configure_com(&pios_usart_flexi_cfg, PIOS_COM_BRIDGE_RX_BUF_LEN, PIOS_COM_BRIDGE_TX_BUF_LEN, &pios_usart_com_driver, &pios_com_bridge_id);
			break;
	}*/
		/* hwsettings_rv_flexiport */


#if defined(PIOS_INCLUDE_WAVE)
	PIOS_WavPlay_Init(&pios_dac_cfg);
#endif

	// ADC system
#if defined(PIOS_INCLUDE_ADC)
	PIOS_ADC_Init(&pios_adc_cfg);
#endif

#if defined(PIOS_INCLUDE_VIDEO)
	PIOS_Video_Init(&pios_video_cfg);
#endif
}

uint16_t supv_timer=0;

static void Clock(uint32_t spektrum_id) {
	/* 125hz */
	++supv_timer;
	if(supv_timer >= 625) {
		supv_timer = 0;
		timex.sec++;
	}
	if (timex.sec >= 60) {
		timex.sec = 0;
		timex.min++;
	}
	if (timex.min >= 60) {
		timex.min = 0;
		timex.hour++;
	}
	if (timex.hour >= 99) {
		timex.hour = 0;
	}
}

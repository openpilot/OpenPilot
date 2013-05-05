/**
 ******************************************************************************
 * @file       pios_board.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2011.
 * @author     PhoenixPilot, http://github.com/PhoenixPilot, Copyright (C) 2012
 * @addtogroup OpenPilotSystem OpenPilot System
 * @{
 * @addtogroup OpenPilotCore OpenPilot Core
 * @{
 * @brief Defines board specific static initializers for hardware for the revomini board.
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

#include "inc/openpilot.h"
#include <pios_board_info.h>
#include <uavobjectsinit.h>
#include <hwsettings.h>
#include <manualcontrolsettings.h>

/*
 * Pull in the board-specific static HW definitions.
 * Including .c files is a bit ugly but this allows all of
 * the HW definitions to be const and static to limit their
 * scope.
 *
 * NOTE: THIS IS THE ONLY PLACE THAT SHOULD EVER INCLUDE THIS FILE
 */
#include "../board_hw_defs.c"

#if defined(PIOS_INCLUDE_RFM22B)
// Forward declarations
static void configureComCallback(OPLinkSettingsRemoteMainPortOptions main_port, OPLinkSettingsRemoteFlexiPortOptions flexi_port,
				 OPLinkSettingsRemoteVCPPortOptions vcp_port, OPLinkSettingsComSpeedOptions com_speed,
				 uint32_t min_frequency, uint32_t max_frequency, uint32_t channel_spacing);
#endif

/**
 * Sensor configurations 
 */

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

#if defined(PIOS_INCLUDE_HMC5883)
#include "pios_hmc5883.h"
static const struct pios_exti_cfg pios_exti_hmc5883_cfg __exti_config = {
	.vector = PIOS_HMC5883_IRQHandler,
	.line = EXTI_Line7,
	.pin = {
		.gpio = GPIOB,
		.init = {
			.GPIO_Pin = GPIO_Pin_7,
			.GPIO_Speed = GPIO_Speed_100MHz,
			.GPIO_Mode = GPIO_Mode_IN,
			.GPIO_OType = GPIO_OType_OD,
			.GPIO_PuPd = GPIO_PuPd_NOPULL,
		},
	},
	.irq = {
		.init = {
			.NVIC_IRQChannel = EXTI9_5_IRQn,
			.NVIC_IRQChannelPreemptionPriority = PIOS_IRQ_PRIO_LOW,
			.NVIC_IRQChannelSubPriority = 0,
			.NVIC_IRQChannelCmd = ENABLE,
		},
	},
	.exti = {
		.init = {
			.EXTI_Line = EXTI_Line7, // matches above GPIO pin
			.EXTI_Mode = EXTI_Mode_Interrupt,
			.EXTI_Trigger = EXTI_Trigger_Rising,
			.EXTI_LineCmd = ENABLE,
		},
	},
};

static const struct pios_hmc5883_cfg pios_hmc5883_cfg = {
	.exti_cfg = &pios_exti_hmc5883_cfg,
	.M_ODR = PIOS_HMC5883_ODR_75,
	.Meas_Conf = PIOS_HMC5883_MEASCONF_NORMAL,
	.Gain = PIOS_HMC5883_GAIN_1_9,
	.Mode = PIOS_HMC5883_MODE_CONTINUOUS,

};
#endif /* PIOS_INCLUDE_HMC5883 */

/**
 * Configuration for the MS5611 chip
 */
#if defined(PIOS_INCLUDE_MS5611)
#include "pios_ms5611.h"
static const struct pios_ms5611_cfg pios_ms5611_cfg = {
	.oversampling = MS5611_OSR_512,
};
#endif /* PIOS_INCLUDE_MS5611 */


/**
 * Configuration for the MPU6000 chip
 */
#if defined(PIOS_INCLUDE_MPU6000)
#include "pios_mpu6000.h"
#include "pios_mpu6000_config.h"
static const struct pios_exti_cfg pios_exti_mpu6000_cfg __exti_config = {
	.vector = PIOS_MPU6000_IRQHandler,
	.line = EXTI_Line4,
	.pin = {
		.gpio = GPIOC,
		.init = {
			.GPIO_Pin = GPIO_Pin_4,
			.GPIO_Speed = GPIO_Speed_100MHz,
			.GPIO_Mode = GPIO_Mode_IN,
			.GPIO_OType = GPIO_OType_OD,
			.GPIO_PuPd = GPIO_PuPd_NOPULL,
		},
	},
	.irq = {
		.init = {
			.NVIC_IRQChannel = EXTI4_IRQn,
			.NVIC_IRQChannelPreemptionPriority = PIOS_IRQ_PRIO_HIGH,
			.NVIC_IRQChannelSubPriority = 0,
			.NVIC_IRQChannelCmd = ENABLE,
		},
	},
	.exti = {
		.init = {
			.EXTI_Line = EXTI_Line4, // matches above GPIO pin
			.EXTI_Mode = EXTI_Mode_Interrupt,
			.EXTI_Trigger = EXTI_Trigger_Rising,
			.EXTI_LineCmd = ENABLE,
		},
	},
};

static const struct pios_mpu6000_cfg pios_mpu6000_cfg = {
	.exti_cfg = &pios_exti_mpu6000_cfg,
	.Fifo_store = PIOS_MPU6000_FIFO_TEMP_OUT | PIOS_MPU6000_FIFO_GYRO_X_OUT | PIOS_MPU6000_FIFO_GYRO_Y_OUT | PIOS_MPU6000_FIFO_GYRO_Z_OUT,
	// Clock at 8 khz, downsampled by 12 for 666Hz
	.Smpl_rate_div_no_dlp = 11,
	// with dlp on output rate is 500Hz
	.Smpl_rate_div_dlp = 1,
	.interrupt_cfg = PIOS_MPU6000_INT_CLR_ANYRD,
	.interrupt_en = PIOS_MPU6000_INTEN_DATA_RDY,
	.User_ctl = PIOS_MPU6000_USERCTL_FIFO_EN | PIOS_MPU6000_USERCTL_DIS_I2C,
	.Pwr_mgmt_clk = PIOS_MPU6000_PWRMGMT_PLL_X_CLK,
	.accel_range = PIOS_MPU6000_ACCEL_8G,
	.gyro_range = PIOS_MPU6000_SCALE_2000_DEG,
	.filter = PIOS_MPU6000_LOWPASS_256_HZ,
	.orientation = PIOS_MPU6000_TOP_180DEG
};
#endif /* PIOS_INCLUDE_MPU6000 */

/* One slot per selectable receiver group.
 *  eg. PWM, PPM, GCS, SPEKTRUM1, SPEKTRUM2, SBUS
 * NOTE: No slot in this map for NONE.
 */
uint32_t pios_rcvr_group_map[MANUALCONTROLSETTINGS_CHANNELGROUPS_NONE];

#define PIOS_COM_TELEM_RF_RX_BUF_LEN 512
#define PIOS_COM_TELEM_RF_TX_BUF_LEN 512

#define PIOS_COM_GPS_RX_BUF_LEN 32

#define PIOS_COM_TELEM_USB_RX_BUF_LEN 65
#define PIOS_COM_TELEM_USB_TX_BUF_LEN 65

#define PIOS_COM_BRIDGE_RX_BUF_LEN 65
#define PIOS_COM_BRIDGE_TX_BUF_LEN 12

#define PIOS_COM_RFM22B_RF_RX_BUF_LEN 512
#define PIOS_COM_RFM22B_RF_TX_BUF_LEN 512

#define PIOS_COM_HKOSD_RX_BUF_LEN 22
#define PIOS_COM_HKOSD_TX_BUF_LEN 22

#if defined(PIOS_INCLUDE_DEBUG_CONSOLE)
#define PIOS_COM_DEBUGCONSOLE_TX_BUF_LEN 40
uint32_t pios_com_debug_id;
#endif	/* PIOS_INCLUDE_DEBUG_CONSOLE */

uint32_t pios_com_gps_id = 0;
uint32_t pios_com_telem_usb_id = 0;
uint32_t pios_com_telem_rf_id = 0;
uint32_t pios_com_bridge_id = 0;
uint32_t pios_com_overo_id = 0;
uint32_t pios_com_hkosd_id = 0;
#if defined(PIOS_INCLUDE_RFM22B)
uint32_t pios_rfm22b_id = 0;
#endif

/* 
 * Setup a com port based on the passed cfg, driver and buffer sizes. tx size of -1 make the port rx only
 */
static void PIOS_Board_configure_com(const struct pios_usart_cfg *usart_port_cfg, size_t rx_buf_len, size_t tx_buf_len,
		const struct pios_com_driver *com_driver, uint32_t *pios_com_id) 
{
	uint32_t pios_usart_id;
	if (PIOS_USART_Init(&pios_usart_id, usart_port_cfg)) {
		PIOS_Assert(0);
	}
	
	uint8_t * rx_buffer = (uint8_t *) pvPortMalloc(rx_buf_len);
	PIOS_Assert(rx_buffer);
	if(tx_buf_len!= (size_t)-1){ // this is the case for rx/tx ports
		uint8_t * tx_buffer = (uint8_t *) pvPortMalloc(tx_buf_len);
		PIOS_Assert(tx_buffer);
		
		if (PIOS_COM_Init(pios_com_id, com_driver, pios_usart_id,
				rx_buffer, rx_buf_len,
				tx_buffer, tx_buf_len)) {
			PIOS_Assert(0);
		}
	}
	else{ //rx only port
		if (PIOS_COM_Init(pios_com_id, com_driver, pios_usart_id,
				rx_buffer, rx_buf_len,
				NULL, 0)) {
			PIOS_Assert(0);
		}
	}
}

static void PIOS_Board_configure_dsm(const struct pios_usart_cfg *pios_usart_dsm_cfg, const struct pios_dsm_cfg *pios_dsm_cfg, 
		const struct pios_com_driver *pios_usart_com_driver,enum pios_dsm_proto *proto, 
		ManualControlSettingsChannelGroupsOptions channelgroup,uint8_t *bind)
{
	uint32_t pios_usart_dsm_id;
	if (PIOS_USART_Init(&pios_usart_dsm_id, pios_usart_dsm_cfg)) {
		PIOS_Assert(0);
	}
	
	uint32_t pios_dsm_id;
	if (PIOS_DSM_Init(&pios_dsm_id, pios_dsm_cfg, pios_usart_com_driver,
			pios_usart_dsm_id, *proto, *bind)) {
		PIOS_Assert(0);
	}
	
	uint32_t pios_dsm_rcvr_id;
	if (PIOS_RCVR_Init(&pios_dsm_rcvr_id, &pios_dsm_rcvr_driver, pios_dsm_id)) {
		PIOS_Assert(0);
	}
	pios_rcvr_group_map[channelgroup] = pios_dsm_rcvr_id;
}

static void PIOS_Board_configure_pwm(const struct pios_pwm_cfg *pios_pwm_cfg)
{
	/* Set up the receiver port.  Later this should be optional */
	uint32_t pios_pwm_id;
	PIOS_PWM_Init(&pios_pwm_id, pios_pwm_cfg);

	uint32_t pios_pwm_rcvr_id;
	if (PIOS_RCVR_Init(&pios_pwm_rcvr_id, &pios_pwm_rcvr_driver, pios_pwm_id)) {
		PIOS_Assert(0);
	}
	pios_rcvr_group_map[MANUALCONTROLSETTINGS_CHANNELGROUPS_PWM] = pios_pwm_rcvr_id;
}

static void PIOS_Board_configure_ppm(const struct pios_ppm_cfg *pios_ppm_cfg)
{
	uint32_t pios_ppm_id;
	PIOS_PPM_Init(&pios_ppm_id, pios_ppm_cfg);

	uint32_t pios_ppm_rcvr_id;
	if (PIOS_RCVR_Init(&pios_ppm_rcvr_id, &pios_ppm_rcvr_driver, pios_ppm_id)) {
		PIOS_Assert(0);
	}
	pios_rcvr_group_map[MANUALCONTROLSETTINGS_CHANNELGROUPS_PPM] = pios_ppm_rcvr_id;
}

/**
 * PIOS_Board_Init()
 * initializes all the core subsystems on this specific hardware
 * called from System/openpilot.c
 */

#include <pios_board_info.h>

void PIOS_Board_Init(void) {

	/* Delay system */
	PIOS_DELAY_Init();

	const struct pios_board_info * bdinfo = &pios_board_info_blob;
	
#if defined(PIOS_INCLUDE_LED)
	const struct pios_led_cfg * led_cfg = PIOS_BOARD_HW_DEFS_GetLedCfg(bdinfo->board_rev);
	PIOS_Assert(led_cfg);
	PIOS_LED_Init(led_cfg);
#endif	/* PIOS_INCLUDE_LED */

	/* Set up the SPI interface to the gyro/acelerometer */
	if (PIOS_SPI_Init(&pios_spi_gyro_id, &pios_spi_gyro_cfg)) {
		PIOS_DEBUG_Assert(0);
	}
	
	/* Set up the SPI interface to the flash and rfm22b */
	if (PIOS_SPI_Init(&pios_spi_telem_flash_id, &pios_spi_telem_flash_cfg)) {
		PIOS_DEBUG_Assert(0);
	}

#if defined(PIOS_INCLUDE_FLASH)
	/* Connect flash to the appropriate interface and configure it */
	uintptr_t flash_id;
	if (PIOS_Flash_Jedec_Init(&flash_id, pios_spi_telem_flash_id, 1)) {
		PIOS_DEBUG_Assert(0);
	}

	uintptr_t fs_id;
	if (PIOS_FLASHFS_Logfs_Init(&fs_id, &flashfs_m25p_cfg, &pios_jedec_flash_driver, flash_id)) {
		PIOS_DEBUG_Assert(0);
	}

#endif
	
#if defined(PIOS_INCLUDE_RTC)
	PIOS_RTC_Init(&pios_rtc_main_cfg);
#endif
	/* IAP System Setup */
	PIOS_IAP_Init();
	// check for safe mode commands from gcs
	if(PIOS_IAP_ReadBootCmd(0) == PIOS_IAP_CLEAR_FLASH_CMD_0 &&
	   PIOS_IAP_ReadBootCmd(1) == PIOS_IAP_CLEAR_FLASH_CMD_1 &&
	   PIOS_IAP_ReadBootCmd(2) == PIOS_IAP_CLEAR_FLASH_CMD_2)
	{
		 PIOS_FLASHFS_Format(fs_id);
		 PIOS_IAP_WriteBootCmd(0,0);
		 PIOS_IAP_WriteBootCmd(1,0);
		 PIOS_IAP_WriteBootCmd(2,0);
	}

	/* Initialize UAVObject libraries */
	EventDispatcherInitialize();
	UAVObjInitialize();
	
	HwSettingsInitialize();
	/* Initialize the alarms library */
	AlarmsInitialize();

	/* Initialize the task monitor library */
	TaskMonitorInitialize();

	/* Initialize the delayed callback library */
	CallbackSchedulerInitialize();

	/* Set up pulse timers */
	PIOS_TIM_InitClock(&tim_1_cfg);
	PIOS_TIM_InitClock(&tim_3_cfg);
	PIOS_TIM_InitClock(&tim_4_cfg);
	PIOS_TIM_InitClock(&tim_5_cfg);
	PIOS_TIM_InitClock(&tim_8_cfg);
	PIOS_TIM_InitClock(&tim_9_cfg);
	PIOS_TIM_InitClock(&tim_10_cfg);
	PIOS_TIM_InitClock(&tim_11_cfg);
	PIOS_TIM_InitClock(&tim_12_cfg);
	
	uint16_t boot_count = PIOS_IAP_ReadBootCount();
	if (boot_count < 3) {
		PIOS_IAP_WriteBootCount(++boot_count);
		AlarmsClear(SYSTEMALARMS_ALARM_BOOTFAULT);
	} else {
		/* Too many failed boot attempts, force hwsettings to defaults */
		HwSettingsSetDefaults(HwSettingsHandle(), 0);
		AlarmsSet(SYSTEMALARMS_ALARM_BOOTFAULT, SYSTEMALARMS_ALARM_CRITICAL);
	}


	//PIOS_IAP_Init();

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
	PIOS_USB_Init(&pios_usb_id, PIOS_BOARD_HW_DEFS_GetUsbCfg(bdinfo->board_rev));

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
			PIOS_Board_configure_com(&pios_usb_cdc_cfg, PIOS_COM_TELEM_USB_RX_BUF_LEN, PIOS_COM_TELEM_USB_TX_BUF_LEN, &pios_usb_cdc_com_driver, &pios_com_telem_usb_id);
#endif	/* PIOS_INCLUDE_COM */
		break;
	case HWSETTINGS_USB_VCPPORT_COMBRIDGE:
#if defined(PIOS_INCLUDE_COM)
		PIOS_Board_configure_com(&pios_usb_cdc_cfg, PIOS_COM_BRIDGE_RX_BUF_LEN, PIOS_COM_BRIDGE_TX_BUF_LEN, &pios_usb_cdc_com_driver, &pios_com_vcp_id);
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

	/* Configure IO ports */
	uint8_t hwsettings_DSMxBind;
	HwSettingsDSMxBindGet(&hwsettings_DSMxBind);
	
	/* Configure main USART port */
	uint8_t hwsettings_mainport;
	HwSettingsRM_MainPortGet(&hwsettings_mainport);
	switch (hwsettings_mainport) {
		case HWSETTINGS_RM_MAINPORT_DISABLED:
			break;
		case HWSETTINGS_RM_MAINPORT_TELEMETRY:
			PIOS_Board_configure_com(&pios_usart_main_cfg, PIOS_COM_TELEM_RF_RX_BUF_LEN, PIOS_COM_TELEM_RF_TX_BUF_LEN, &pios_usart_com_driver, &pios_com_telem_rf_id);
			break;
		case HWSETTINGS_RM_MAINPORT_GPS:
			PIOS_Board_configure_com(&pios_usart_main_cfg, PIOS_COM_GPS_RX_BUF_LEN, -1, &pios_usart_com_driver, &pios_com_gps_id);
			break;
		case HWSETTINGS_RM_MAINPORT_SBUS:
#if defined(PIOS_INCLUDE_SBUS)
                        {
                                uint32_t pios_usart_sbus_id;
                                if (PIOS_USART_Init(&pios_usart_sbus_id, &pios_usart_sbus_main_cfg)) {
                                        PIOS_Assert(0);
                                }

                                uint32_t pios_sbus_id;
                                if (PIOS_SBus_Init(&pios_sbus_id, &pios_sbus_cfg, &pios_usart_com_driver, pios_usart_sbus_id)) {
                                        PIOS_Assert(0);
                                }

                                uint32_t pios_sbus_rcvr_id;
                                if (PIOS_RCVR_Init(&pios_sbus_rcvr_id, &pios_sbus_rcvr_driver, pios_sbus_id)) {
                                        PIOS_Assert(0);
                                }
                                pios_rcvr_group_map[MANUALCONTROLSETTINGS_CHANNELGROUPS_SBUS] = pios_sbus_rcvr_id;
                        }
#endif
                        break;
		case HWSETTINGS_RM_MAINPORT_DSM2:
		case HWSETTINGS_RM_MAINPORT_DSMX10BIT:
		case HWSETTINGS_RM_MAINPORT_DSMX11BIT:
			{
				enum pios_dsm_proto proto;
				switch (hwsettings_mainport) {
				case HWSETTINGS_RM_MAINPORT_DSM2:
					proto = PIOS_DSM_PROTO_DSM2;
					break;
				case HWSETTINGS_RM_MAINPORT_DSMX10BIT:
					proto = PIOS_DSM_PROTO_DSMX10BIT;
					break;
				case HWSETTINGS_RM_MAINPORT_DSMX11BIT:
					proto = PIOS_DSM_PROTO_DSMX11BIT;
					break;
				default:
					PIOS_Assert(0);
					break;
				}

				// Force binding to zero on the main port
				hwsettings_DSMxBind = 0;

				//TODO: Define the various Channelgroup for Revo dsm inputs and handle here
				PIOS_Board_configure_dsm(&pios_usart_dsm_main_cfg, &pios_dsm_main_cfg, 
							&pios_usart_com_driver, &proto, MANUALCONTROLSETTINGS_CHANNELGROUPS_DSMMAINPORT,&hwsettings_DSMxBind);
			}
			break;
		case HWSETTINGS_RM_MAINPORT_DEBUGCONSOLE:
#if defined(PIOS_INCLUDE_DEBUG_CONSOLE)
			{
				PIOS_Board_configure_com(&pios_usart_main_cfg, 0, PIOS_COM_DEBUGCONSOLE_TX_BUF_LEN, &pios_usart_com_driver, &pios_com_debug_id);
			}
#endif	/* PIOS_INCLUDE_DEBUG_CONSOLE */
			break;
		case HWSETTINGS_RM_MAINPORT_COMBRIDGE:
			PIOS_Board_configure_com(&pios_usart_main_cfg, PIOS_COM_BRIDGE_RX_BUF_LEN, PIOS_COM_BRIDGE_TX_BUF_LEN, &pios_usart_com_driver, &pios_com_bridge_id);
			break;
		case HWSETTINGS_RM_MAINPORT_OSDHK:
			PIOS_Board_configure_com(&pios_usart_hkosd_main_cfg, PIOS_COM_HKOSD_RX_BUF_LEN, PIOS_COM_HKOSD_TX_BUF_LEN, &pios_usart_com_driver, &pios_com_hkosd_id);
			break;
	} /* 	hwsettings_rm_mainport */

	if (hwsettings_mainport != HWSETTINGS_RM_MAINPORT_SBUS) {
		GPIO_Init(pios_sbus_cfg.inv.gpio, &pios_sbus_cfg.inv.init);
		GPIO_WriteBit(pios_sbus_cfg.inv.gpio, pios_sbus_cfg.inv.init.GPIO_Pin, pios_sbus_cfg.gpio_inv_disable);
	}

	/* Configure FlexiPort */
	uint8_t hwsettings_flexiport;
	HwSettingsRM_FlexiPortGet(&hwsettings_flexiport);
	switch (hwsettings_flexiport) {
		case HWSETTINGS_RM_FLEXIPORT_DISABLED:
			break;
                case HWSETTINGS_RM_FLEXIPORT_TELEMETRY:
                        PIOS_Board_configure_com(&pios_usart_flexi_cfg, PIOS_COM_TELEM_RF_RX_BUF_LEN, PIOS_COM_TELEM_RF_TX_BUF_LEN, &pios_usart_com_driver, &pios_com_telem_rf_id);
			break;
		case HWSETTINGS_RM_FLEXIPORT_I2C:
#if defined(PIOS_INCLUDE_I2C)
			{
				if (PIOS_I2C_Init(&pios_i2c_flexiport_adapter_id, &pios_i2c_flexiport_adapter_cfg)) {
					PIOS_Assert(0);
				}
			}
#endif	/* PIOS_INCLUDE_I2C */
			break;
		case HWSETTINGS_RM_FLEXIPORT_GPS:
			PIOS_Board_configure_com(&pios_usart_flexi_cfg, PIOS_COM_GPS_RX_BUF_LEN, -1, &pios_usart_com_driver, &pios_com_gps_id);
			break;
		case HWSETTINGS_RM_FLEXIPORT_DSM2:
		case HWSETTINGS_RM_FLEXIPORT_DSMX10BIT:
		case HWSETTINGS_RM_FLEXIPORT_DSMX11BIT:
			{
				enum pios_dsm_proto proto;
				switch (hwsettings_flexiport) {
				case HWSETTINGS_RM_FLEXIPORT_DSM2:
					proto = PIOS_DSM_PROTO_DSM2;
					break;
				case HWSETTINGS_RM_FLEXIPORT_DSMX10BIT:
					proto = PIOS_DSM_PROTO_DSMX10BIT;
					break;
				case HWSETTINGS_RM_FLEXIPORT_DSMX11BIT:
					proto = PIOS_DSM_PROTO_DSMX11BIT;
					break;
				default:
					PIOS_Assert(0);
					break;
				}
				//TODO: Define the various Channelgroup for Revo dsm inputs and handle here
				PIOS_Board_configure_dsm(&pios_usart_dsm_flexi_cfg, &pios_dsm_flexi_cfg, 
							&pios_usart_com_driver, &proto, MANUALCONTROLSETTINGS_CHANNELGROUPS_DSMFLEXIPORT,&hwsettings_DSMxBind);
			}
			break;
		case HWSETTINGS_RM_FLEXIPORT_DEBUGCONSOLE:
#if defined(PIOS_INCLUDE_DEBUG_CONSOLE)
			{
				PIOS_Board_configure_com(&pios_usart_main_cfg, 0, PIOS_COM_DEBUGCONSOLE_TX_BUF_LEN, &pios_usart_com_driver, &pios_com_debug_id);
			}
#endif	/* PIOS_INCLUDE_DEBUG_CONSOLE */
			break;
		case HWSETTINGS_RM_FLEXIPORT_COMBRIDGE:
			PIOS_Board_configure_com(&pios_usart_flexi_cfg, PIOS_COM_BRIDGE_RX_BUF_LEN, PIOS_COM_BRIDGE_TX_BUF_LEN, &pios_usart_com_driver, &pios_com_bridge_id);
			break;
		case HWSETTINGS_RM_FLEXIPORT_OSDHK:
			PIOS_Board_configure_com(&pios_usart_hkosd_flexi_cfg, PIOS_COM_HKOSD_RX_BUF_LEN, PIOS_COM_HKOSD_TX_BUF_LEN, &pios_usart_com_driver, &pios_com_hkosd_id);
			break;
	} /* hwsettings_rm_flexiport */


	/* Initalize the RFM22B radio COM device. */
#if defined(PIOS_INCLUDE_RFM22B)
	uint8_t hwsettings_radioport;
	HwSettingsRadioPortGet(&hwsettings_radioport);
	uint8_t hwsettings_maxrfpower;
	HwSettingsMaxRFPowerGet(&hwsettings_maxrfpower);
	uint32_t hwsettings_deffreq;
	HwSettingsDefaultFrequencyGet(&hwsettings_deffreq);
	switch (hwsettings_radioport) {
		case HWSETTINGS_RADIOPORT_DISABLED:
			break;
		case HWSETTINGS_RADIOPORT_TELEMETRY:
		{
			extern const struct pios_rfm22b_cfg * PIOS_BOARD_HW_DEFS_GetRfm22Cfg (uint32_t board_revision);
			const struct pios_board_info * bdinfo = &pios_board_info_blob;
			const struct pios_rfm22b_cfg *pios_rfm22b_cfg = PIOS_BOARD_HW_DEFS_GetRfm22Cfg(bdinfo->board_rev);
			if (PIOS_RFM22B_Init(&pios_rfm22b_id, PIOS_RFM22_SPI_PORT, pios_rfm22b_cfg->slave_num, pios_rfm22b_cfg)) {
				PIOS_Assert(0);
			}

			// Set the modem parameters and reinitilize the modem.
			PIOS_RFM22B_SetInitialFrequency(pios_rfm22b_id, hwsettings_deffreq);
			switch (hwsettings_maxrfpower)
			{
			case OPLINKSETTINGS_MAXRFPOWER_125:
				PIOS_RFM22B_SetTxPower(pios_rfm22b_id, RFM22_tx_pwr_txpow_0);
				break;
			case OPLINKSETTINGS_MAXRFPOWER_16:
				PIOS_RFM22B_SetTxPower(pios_rfm22b_id, RFM22_tx_pwr_txpow_1);
				break;
			case OPLINKSETTINGS_MAXRFPOWER_316:
				PIOS_RFM22B_SetTxPower(pios_rfm22b_id, RFM22_tx_pwr_txpow_2);
				break;
			case OPLINKSETTINGS_MAXRFPOWER_63:
				PIOS_RFM22B_SetTxPower(pios_rfm22b_id, RFM22_tx_pwr_txpow_3);
				break;
			case OPLINKSETTINGS_MAXRFPOWER_126:
				PIOS_RFM22B_SetTxPower(pios_rfm22b_id, RFM22_tx_pwr_txpow_4);
				break;
			case OPLINKSETTINGS_MAXRFPOWER_25:
				PIOS_RFM22B_SetTxPower(pios_rfm22b_id, RFM22_tx_pwr_txpow_5);
				break;
			case OPLINKSETTINGS_MAXRFPOWER_50:
				PIOS_RFM22B_SetTxPower(pios_rfm22b_id, RFM22_tx_pwr_txpow_6);
				break;
			case OPLINKSETTINGS_MAXRFPOWER_100:
				PIOS_RFM22B_SetTxPower(pios_rfm22b_id, RFM22_tx_pwr_txpow_7);
				break;
			default:
				// do nothing
				break;
			}
			PIOS_RFM22B_Reinit(pios_rfm22b_id);

#ifdef PIOS_INCLUDE_RFM22B_COM
			uint8_t *rx_buffer = (uint8_t *) pvPortMalloc(PIOS_COM_RFM22B_RF_RX_BUF_LEN);
			uint8_t *tx_buffer = (uint8_t *) pvPortMalloc(PIOS_COM_RFM22B_RF_TX_BUF_LEN);
			PIOS_Assert(rx_buffer);
			PIOS_Assert(tx_buffer);
			if (PIOS_COM_Init(&pios_com_telem_rf_id, &pios_rfm22b_com_driver, pios_rfm22b_id,
					  rx_buffer, PIOS_COM_RFM22B_RF_RX_BUF_LEN,
					  tx_buffer, PIOS_COM_RFM22B_RF_TX_BUF_LEN))
				PIOS_Assert(0);
#endif
#ifdef PIOS_INCLUDE_RFM22B_RCVR
			if (PIOS_RFM22B_RCVR_Init(pios_rfm22b_id) != 0)
				PIOS_Assert(0);
			uint32_t pios_rfm22b_rcvr_id;
			if (PIOS_RCVR_Init(&pios_rfm22b_rcvr_id, &pios_rfm22b_rcvr_driver, pios_rfm22b_id))
				PIOS_Assert(0);
			pios_rcvr_group_map[MANUALCONTROLSETTINGS_CHANNELGROUPS_OPLINK] = pios_rfm22b_rcvr_id;
#endif

                        // Set the com port configuration callback.
                        PIOS_RFM22B_SetComConfigCallback(pios_rfm22b_id, &configureComCallback);

			break;
		}
	}
#endif /* PIOS_INCLUDE_RFM22B */

#if defined(PIOS_INCLUDE_PWM) ||  defined(PIOS_INCLUDE_PWM)
	
	const struct pios_servo_cfg *pios_servo_cfg;
	// default to servo outputs only
	pios_servo_cfg = &pios_servo_cfg_out;
#endif
	
	/* Configure the receiver port*/
	uint8_t hwsettings_rcvrport;
	HwSettingsRM_RcvrPortGet(&hwsettings_rcvrport);
	//   
	switch (hwsettings_rcvrport){
		case HWSETTINGS_RM_RCVRPORT_DISABLED:
			break;
		case HWSETTINGS_RM_RCVRPORT_PWM:
#if defined(PIOS_INCLUDE_PWM)
			/* Set up the receiver port.  Later this should be optional */
			PIOS_Board_configure_pwm(&pios_pwm_cfg);
#endif	/* PIOS_INCLUDE_PWM */
			break;
		case HWSETTINGS_RM_RCVRPORT_PPM:
		case HWSETTINGS_RM_RCVRPORT_PPMOUTPUTS:
		case HWSETTINGS_RM_RCVRPORT_PPMPWM:
#if defined(PIOS_INCLUDE_PPM)
			if(hwsettings_rcvrport == HWSETTINGS_RM_RCVRPORT_PPMOUTPUTS)
			{
				// configure servo outputs and the remaining 5 inputs as outputs
				pios_servo_cfg = &pios_servo_cfg_out_in_ppm;
			}

			PIOS_Board_configure_ppm(&pios_ppm_cfg);

			// enable pwm on the remaining channels
			if(hwsettings_rcvrport == HWSETTINGS_RM_RCVRPORT_PPMPWM)
			{
				PIOS_Board_configure_pwm(&pios_pwm_ppm_cfg);
			}

			break;
#endif	/* PIOS_INCLUDE_PPM */
		case HWSETTINGS_RM_RCVRPORT_OUTPUTS:
			// configure only the servo outputs
			pios_servo_cfg = &pios_servo_cfg_out_in;
			break;
	}


#if defined(PIOS_INCLUDE_GCSRCVR)
	GCSReceiverInitialize();
	uint32_t pios_gcsrcvr_id;
	PIOS_GCSRCVR_Init(&pios_gcsrcvr_id);
	uint32_t pios_gcsrcvr_rcvr_id;
	if (PIOS_RCVR_Init(&pios_gcsrcvr_rcvr_id, &pios_gcsrcvr_rcvr_driver, pios_gcsrcvr_id)) {
		PIOS_Assert(0);
	}
	pios_rcvr_group_map[MANUALCONTROLSETTINGS_CHANNELGROUPS_GCS] = pios_gcsrcvr_rcvr_id;
#endif	/* PIOS_INCLUDE_GCSRCVR */

#ifndef PIOS_ENABLE_DEBUG_PINS
	// pios_servo_cfg points to the correct configuration based on input port settings
	PIOS_Servo_Init(pios_servo_cfg);
#else
	PIOS_DEBUG_Init(pios_tim_servoport_all_pins, NELEMENTS(pios_tim_servoport_all_pins));
#endif
	
	if (PIOS_I2C_Init(&pios_i2c_mag_pressure_adapter_id, &pios_i2c_mag_pressure_adapter_cfg)) {
		PIOS_DEBUG_Assert(0);
	}
	
	PIOS_DELAY_WaitmS(50);

#if defined(PIOS_INCLUDE_ADC)
	PIOS_ADC_Init(&pios_adc_cfg);
#endif

#if defined(PIOS_INCLUDE_HMC5883)
	PIOS_HMC5883_Init(&pios_hmc5883_cfg);
#endif
	
#if defined(PIOS_INCLUDE_MS5611)
	PIOS_MS5611_Init(&pios_ms5611_cfg, pios_i2c_mag_pressure_adapter_id);
#endif

#if defined(PIOS_INCLUDE_MPU6000)
	PIOS_MPU6000_Init(pios_spi_gyro_id,0, &pios_mpu6000_cfg);
	PIOS_MPU6000_CONFIG_Configure();
#endif

}

#if defined(PIOS_INCLUDE_RFM22B)
/**
 * Configure the radio com port based on a configuration event from the remote coordinator.
 * \param[in] main_port  The main com port options
 * \param[in] flexi_port The flexi com port options
 * \param[in] vcp_port   The USB virtual com port options
 * \param[in] com_speed  The com port speed
 */
static void configureComCallback(__attribute__((unused)) OPLinkSettingsRemoteMainPortOptions main_port,
								  __attribute__((unused)) OPLinkSettingsRemoteFlexiPortOptions flexi_port,
								  __attribute__((unused)) OPLinkSettingsRemoteVCPPortOptions vcp_port,
								  OPLinkSettingsComSpeedOptions com_speed,
								  uint32_t min_frequency, uint32_t max_frequency, uint32_t channel_spacing)
{
    uint32_t comBaud = 9600;
    switch (com_speed) {
    case OPLINKSETTINGS_COMSPEED_2400:
        comBaud = 2400;
        break;
    case OPLINKSETTINGS_COMSPEED_4800:
        comBaud = 4800;
        break;
    case OPLINKSETTINGS_COMSPEED_9600:
        comBaud = 9600;
        break;
    case OPLINKSETTINGS_COMSPEED_19200:
        comBaud = 19200;
        break;
    case OPLINKSETTINGS_COMSPEED_38400:
        comBaud = 38400;
        break;
    case OPLINKSETTINGS_COMSPEED_57600:
        comBaud = 57600;
        break;
    case OPLINKSETTINGS_COMSPEED_115200:
        comBaud = 115200;
        break;
    }
    if (PIOS_COM_TELEM_RF) {
        PIOS_COM_ChangeBaud(PIOS_COM_TELEM_RF, comBaud);
    }

    // Set the frequency range.
    PIOS_RFM22B_SetFrequencyRange(pios_rfm22b_id, min_frequency, max_frequency, channel_spacing);
}

#endif

/**
 * @}
 * @}
 */


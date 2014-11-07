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
#include <oplinksettings.h>
#include <oplinkstatus.h>
#include <oplinkreceiver.h>
#include <pios_oplinkrcvr_priv.h>
#include <taskinfo.h>
#include <pios_ws2811.h>


#ifdef PIOS_INCLUDE_INSTRUMENTATION
#include <pios_instrumentation.h>
#endif

/*
 * Pull in the board-specific static HW definitions.
 * Including .c files is a bit ugly but this allows all of
 * the HW definitions to be const and static to limit their
 * scope.
 *
 * NOTE: THIS IS THE ONLY PLACE THAT SHOULD EVER INCLUDE THIS FILE
 */
#include "../board_hw_defs.c"

/**
 * Sensor configurations
 */

#if defined(PIOS_INCLUDE_ADC)

#include "pios_adc_priv.h"
void PIOS_ADC_DMC_irq_handler(void);
void DMA2_Stream4_IRQHandler(void) __attribute__((alias("PIOS_ADC_DMC_irq_handler")));
struct pios_adc_cfg pios_adc_cfg = {
    .adc_dev = ADC1,
    .dma     = {
        .irq                                       = {
            .flags = (DMA_FLAG_TCIF4 | DMA_FLAG_TEIF4 | DMA_FLAG_HTIF4),
            .init  = {
                .NVIC_IRQChannel    = DMA2_Stream4_IRQn,
                .NVIC_IRQChannelPreemptionPriority = PIOS_IRQ_PRIO_LOW,
                .NVIC_IRQChannelSubPriority        = 0,
                .NVIC_IRQChannelCmd = ENABLE,
            },
        },
        .rx                                        = {
            .channel = DMA2_Stream4,
            .init    = {
                .DMA_Channel                       = DMA_Channel_0,
                .DMA_PeripheralBaseAddr            = (uint32_t)&ADC1->DR
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

#endif /* if defined(PIOS_INCLUDE_ADC) */

#if defined(PIOS_INCLUDE_HMC5X83)
#include "pios_hmc5x83.h"
pios_hmc5x83_dev_t onboard_mag = 0;

bool pios_board_internal_mag_handler()
{
    return PIOS_HMC5x83_IRQHandler(onboard_mag);
}
static const struct pios_exti_cfg pios_exti_hmc5x83_cfg __exti_config = {
    .vector = pios_board_internal_mag_handler,
    .line   = EXTI_Line7,
    .pin    = {
        .gpio = GPIOB,
        .init = {
            .GPIO_Pin   = GPIO_Pin_7,
            .GPIO_Speed = GPIO_Speed_100MHz,
            .GPIO_Mode  = GPIO_Mode_IN,
            .GPIO_OType = GPIO_OType_OD,
            .GPIO_PuPd  = GPIO_PuPd_NOPULL,
        },
    },
    .irq                                       = {
        .init                                  = {
            .NVIC_IRQChannel    = EXTI9_5_IRQn,
            .NVIC_IRQChannelPreemptionPriority = PIOS_IRQ_PRIO_LOW,
            .NVIC_IRQChannelSubPriority        = 0,
            .NVIC_IRQChannelCmd = ENABLE,
        },
    },
    .exti                                      = {
        .init                                  = {
            .EXTI_Line    = EXTI_Line7, // matches above GPIO pin
            .EXTI_Mode    = EXTI_Mode_Interrupt,
            .EXTI_Trigger = EXTI_Trigger_Rising,
            .EXTI_LineCmd = ENABLE,
        },
    },
};

static const struct pios_hmc5x83_cfg pios_hmc5x83_cfg = {
    .exti_cfg  = &pios_exti_hmc5x83_cfg,
    .M_ODR     = PIOS_HMC5x83_ODR_75,
    .Meas_Conf = PIOS_HMC5x83_MEASCONF_NORMAL,
    .Gain      = PIOS_HMC5x83_GAIN_1_9,
    .Mode      = PIOS_HMC5x83_MODE_CONTINUOUS,
    .Driver    = &PIOS_HMC5x83_I2C_DRIVER,
};
#endif /* PIOS_INCLUDE_HMC5X83 */

/**
 * Configuration for the MS5611 chip
 */
#if defined(PIOS_INCLUDE_MS5611)
#include "pios_ms5611.h"
static const struct pios_ms5611_cfg pios_ms5611_cfg = {
    .oversampling = MS5611_OSR_4096,
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
    .line   = EXTI_Line4,
    .pin    = {
        .gpio = GPIOC,
        .init = {
            .GPIO_Pin   = GPIO_Pin_4,
            .GPIO_Speed = GPIO_Speed_100MHz,
            .GPIO_Mode  = GPIO_Mode_IN,
            .GPIO_OType = GPIO_OType_OD,
            .GPIO_PuPd  = GPIO_PuPd_NOPULL,
        },
    },
    .irq                                       = {
        .init                                  = {
            .NVIC_IRQChannel    = EXTI4_IRQn,
            .NVIC_IRQChannelPreemptionPriority = PIOS_IRQ_PRIO_HIGH,
            .NVIC_IRQChannelSubPriority        = 0,
            .NVIC_IRQChannelCmd = ENABLE,
        },
    },
    .exti                                      = {
        .init                                  = {
            .EXTI_Line    = EXTI_Line4, // matches above GPIO pin
            .EXTI_Mode    = EXTI_Mode_Interrupt,
            .EXTI_Trigger = EXTI_Trigger_Rising,
            .EXTI_LineCmd = ENABLE,
        },
    },
};

static const struct pios_mpu6000_cfg pios_mpu6000_cfg = {
    .exti_cfg   = &pios_exti_mpu6000_cfg,
    .Fifo_store = PIOS_MPU6000_FIFO_TEMP_OUT | PIOS_MPU6000_FIFO_GYRO_X_OUT | PIOS_MPU6000_FIFO_GYRO_Y_OUT | PIOS_MPU6000_FIFO_GYRO_Z_OUT,
    // Clock at 8 khz
    .Smpl_rate_div_no_dlp = 0,
    // with dlp on output rate is 1000Hz
    .Smpl_rate_div_dlp    = 0,
    .interrupt_cfg  = PIOS_MPU6000_INT_CLR_ANYRD,
    .interrupt_en   = PIOS_MPU6000_INTEN_DATA_RDY,
    .User_ctl             = PIOS_MPU6000_USERCTL_DIS_I2C,
    .Pwr_mgmt_clk   = PIOS_MPU6000_PWRMGMT_PLL_X_CLK,
    .accel_range    = PIOS_MPU6000_ACCEL_8G,
    .gyro_range     = PIOS_MPU6000_SCALE_2000_DEG,
    .filter               = PIOS_MPU6000_LOWPASS_256_HZ,
    .orientation    = PIOS_MPU6000_TOP_180DEG,
    .fast_prescaler = PIOS_SPI_PRESCALER_4,
    .std_prescaler  = PIOS_SPI_PRESCALER_64,
    .max_downsample = 16,
};
#endif /* PIOS_INCLUDE_MPU6000 */

/* One slot per selectable receiver group.
 *  eg. PWM, PPM, GCS, SPEKTRUM1, SPEKTRUM2, SBUS
 * NOTE: No slot in this map for NONE.
 */
uint32_t pios_rcvr_group_map[MANUALCONTROLSETTINGS_CHANNELGROUPS_NONE];

#define PIOS_COM_TELEM_RF_RX_BUF_LEN     512
#define PIOS_COM_TELEM_RF_TX_BUF_LEN     512

#define PIOS_COM_GPS_RX_BUF_LEN          128
#define PIOS_COM_GPS_TX_BUF_LEN          32

#define PIOS_COM_TELEM_USB_RX_BUF_LEN    65
#define PIOS_COM_TELEM_USB_TX_BUF_LEN    65

#define PIOS_COM_BRIDGE_RX_BUF_LEN       65
#define PIOS_COM_BRIDGE_TX_BUF_LEN       12

#define PIOS_COM_RFM22B_RF_RX_BUF_LEN    512
#define PIOS_COM_RFM22B_RF_TX_BUF_LEN    512

#define PIOS_COM_HKOSD_RX_BUF_LEN        22
#define PIOS_COM_HKOSD_TX_BUF_LEN        22

#if defined(PIOS_INCLUDE_DEBUG_CONSOLE)
#define PIOS_COM_DEBUGCONSOLE_TX_BUF_LEN 40
uint32_t pios_com_debug_id;
#endif /* PIOS_INCLUDE_DEBUG_CONSOLE */

uint32_t pios_com_gps_id       = 0;
uint32_t pios_com_telem_usb_id = 0;
uint32_t pios_com_telem_rf_id  = 0;
uint32_t pios_com_rf_id        = 0;
uint32_t pios_com_bridge_id    = 0;
uint32_t pios_com_overo_id     = 0;
uint32_t pios_com_hkosd_id     = 0;

uint32_t pios_com_vcp_id       = 0;

#if defined(PIOS_INCLUDE_RFM22B)
uint32_t pios_rfm22b_id        = 0;
#endif

uintptr_t pios_uavo_settings_fs_id;
uintptr_t pios_user_fs_id;

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

    uint8_t *rx_buffer = (uint8_t *)pios_malloc(rx_buf_len);
    PIOS_Assert(rx_buffer);
    if (tx_buf_len != (size_t)-1) { // this is the case for rx/tx ports
        uint8_t *tx_buffer = (uint8_t *)pios_malloc(tx_buf_len);
        PIOS_Assert(tx_buffer);

        if (PIOS_COM_Init(pios_com_id, com_driver, pios_usart_id,
                          rx_buffer, rx_buf_len,
                          tx_buffer, tx_buf_len)) {
            PIOS_Assert(0);
        }
    } else { // rx only port
        if (PIOS_COM_Init(pios_com_id, com_driver, pios_usart_id,
                          rx_buffer, rx_buf_len,
                          NULL, 0)) {
            PIOS_Assert(0);
        }
    }
}

static void PIOS_Board_configure_dsm(const struct pios_usart_cfg *pios_usart_dsm_cfg, const struct pios_dsm_cfg *pios_dsm_cfg,
                                     const struct pios_com_driver *usart_com_driver,
                                     ManualControlSettingsChannelGroupsOptions channelgroup, uint8_t *bind)
{
    uint32_t pios_usart_dsm_id;

    if (PIOS_USART_Init(&pios_usart_dsm_id, pios_usart_dsm_cfg)) {
        PIOS_Assert(0);
    }

    uint32_t pios_dsm_id;
    if (PIOS_DSM_Init(&pios_dsm_id, pios_dsm_cfg, usart_com_driver,
                      pios_usart_dsm_id, *bind)) {
        PIOS_Assert(0);
    }

    uint32_t pios_dsm_rcvr_id;
    if (PIOS_RCVR_Init(&pios_dsm_rcvr_id, &pios_dsm_rcvr_driver, pios_dsm_id)) {
        PIOS_Assert(0);
    }
    pios_rcvr_group_map[channelgroup] = pios_dsm_rcvr_id;
}

static void PIOS_Board_configure_pwm(const struct pios_pwm_cfg *pwm_cfg)
{
    /* Set up the receiver port.  Later this should be optional */
    uint32_t pios_pwm_id;

    PIOS_PWM_Init(&pios_pwm_id, pwm_cfg);

    uint32_t pios_pwm_rcvr_id;
    if (PIOS_RCVR_Init(&pios_pwm_rcvr_id, &pios_pwm_rcvr_driver, pios_pwm_id)) {
        PIOS_Assert(0);
    }
    pios_rcvr_group_map[MANUALCONTROLSETTINGS_CHANNELGROUPS_PWM] = pios_pwm_rcvr_id;
}

static void PIOS_Board_configure_ppm(const struct pios_ppm_cfg *ppm_cfg)
{
    uint32_t pios_ppm_id;

    PIOS_PPM_Init(&pios_ppm_id, ppm_cfg);

    uint32_t pios_ppm_rcvr_id;
    if (PIOS_RCVR_Init(&pios_ppm_rcvr_id, &pios_ppm_rcvr_driver, pios_ppm_id)) {
        PIOS_Assert(0);
    }
    pios_rcvr_group_map[MANUALCONTROLSETTINGS_CHANNELGROUPS_PPM] = pios_ppm_rcvr_id;
}

static void PIOS_Board_PPM_callback(const int16_t *channels)
{
    uint8_t max_chan = (RFM22B_PPM_NUM_CHANNELS < OPLINKRECEIVER_CHANNEL_NUMELEM) ? RFM22B_PPM_NUM_CHANNELS : OPLINKRECEIVER_CHANNEL_NUMELEM;
    OPLinkReceiverData opl_rcvr;

    for (uint8_t i = 0; i < max_chan; ++i) {
        opl_rcvr.Channel[i] = channels[i];
    }
    OPLinkReceiverSet(&opl_rcvr);
}

/**
 * PIOS_Board_Init()
 * initializes all the core subsystems on this specific hardware
 * called from System/openpilot.c
 */

#include <pios_board_info.h>

void PIOS_Board_Init(void)
{
    /* Delay system */
    PIOS_DELAY_Init();

    const struct pios_board_info *bdinfo = &pios_board_info_blob;

#if defined(PIOS_INCLUDE_LED)
    const struct pios_gpio_cfg *led_cfg  = PIOS_BOARD_HW_DEFS_GetLedCfg(bdinfo->board_rev);
    PIOS_Assert(led_cfg);
    PIOS_LED_Init(led_cfg);
#endif /* PIOS_INCLUDE_LED */


#ifdef PIOS_INCLUDE_INSTRUMENTATION
    PIOS_Instrumentation_Init(PIOS_INSTRUMENTATION_MAX_COUNTERS);
#endif

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

    // Initialize the external USER flash
    if (PIOS_Flash_Jedec_Init(&flash_id, pios_spi_telem_flash_id, 1)) {
        PIOS_DEBUG_Assert(0);
    }

    if (PIOS_FLASHFS_Logfs_Init(&pios_uavo_settings_fs_id, &flashfs_external_system_cfg, &pios_jedec_flash_driver, flash_id)) {
        PIOS_DEBUG_Assert(0);
    }

    if (PIOS_FLASHFS_Logfs_Init(&pios_user_fs_id, &flashfs_external_user_cfg, &pios_jedec_flash_driver, flash_id)) {
        PIOS_DEBUG_Assert(0);
    }

#endif /* if defined(PIOS_INCLUDE_FLASH) */

#if defined(PIOS_INCLUDE_RTC)
    PIOS_RTC_Init(&pios_rtc_main_cfg);
#endif
    /* IAP System Setup */
    PIOS_IAP_Init();
    // check for safe mode commands from gcs
    if (PIOS_IAP_ReadBootCmd(0) == PIOS_IAP_CLEAR_FLASH_CMD_0 &&
        PIOS_IAP_ReadBootCmd(1) == PIOS_IAP_CLEAR_FLASH_CMD_1 &&
        PIOS_IAP_ReadBootCmd(2) == PIOS_IAP_CLEAR_FLASH_CMD_2) {
        PIOS_FLASHFS_Format(pios_uavo_settings_fs_id);
        PIOS_IAP_WriteBootCmd(0, 0);
        PIOS_IAP_WriteBootCmd(1, 0);
        PIOS_IAP_WriteBootCmd(2, 0);
    }
#ifdef PIOS_INCLUDE_WDG
    PIOS_WDG_Init();
#endif

    /* Initialize the task monitor */
    if (PIOS_TASK_MONITOR_Initialize(TASKINFO_RUNNING_NUMELEM)) {
        PIOS_Assert(0);
    }

    /* Initialize the delayed callback library */
    PIOS_CALLBACKSCHEDULER_Initialize();

    /* Initialize UAVObject libraries */
    EventDispatcherInitialize();
    UAVObjInitialize();
    HwSettingsInitialize();
#if defined(PIOS_INCLUDE_RFM22B)
    OPLinkSettingsInitialize();
    OPLinkStatusInitialize();
#endif /* PIOS_INCLUDE_RFM22B */

    /* Initialize the alarms library */
    AlarmsInitialize();

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


    // PIOS_IAP_Init();

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
    uint32_t pios_usb_cdc_id;
    if (PIOS_USB_CDC_Init(&pios_usb_cdc_id, &pios_usb_cdc_cfg, pios_usb_id)) {
        PIOS_Assert(0);
    }

    uint32_t pios_usb_hid_id;
    if (PIOS_USB_HID_Init(&pios_usb_hid_id, &pios_usb_hid_cfg, pios_usb_id)) {
        PIOS_Assert(0);
    }

    switch (hwsettings_usb_vcpport) {
    case HWSETTINGS_USB_VCPPORT_DISABLED:
        break;
    case HWSETTINGS_USB_VCPPORT_USBTELEMETRY:
#if defined(PIOS_INCLUDE_COM)
        {
            uint8_t *rx_buffer = (uint8_t *)pios_malloc(PIOS_COM_TELEM_USB_RX_BUF_LEN);
            uint8_t *tx_buffer = (uint8_t *)pios_malloc(PIOS_COM_TELEM_USB_TX_BUF_LEN);
            PIOS_Assert(rx_buffer);
            PIOS_Assert(tx_buffer);
            if (PIOS_COM_Init(&pios_com_telem_usb_id, &pios_usb_cdc_com_driver, pios_usb_cdc_id,
                              rx_buffer, PIOS_COM_TELEM_USB_RX_BUF_LEN,
                              tx_buffer, PIOS_COM_TELEM_USB_TX_BUF_LEN)) {
                PIOS_Assert(0);
            }
        }
#endif /* PIOS_INCLUDE_COM */
        break;
    case HWSETTINGS_USB_VCPPORT_COMBRIDGE:
#if defined(PIOS_INCLUDE_COM)
        {
            uint8_t *rx_buffer = (uint8_t *)pios_malloc(PIOS_COM_BRIDGE_RX_BUF_LEN);
            uint8_t *tx_buffer = (uint8_t *)pios_malloc(PIOS_COM_BRIDGE_TX_BUF_LEN);
            PIOS_Assert(rx_buffer);
            PIOS_Assert(tx_buffer);
            if (PIOS_COM_Init(&pios_com_vcp_id, &pios_usb_cdc_com_driver, pios_usb_cdc_id,
                              rx_buffer, PIOS_COM_BRIDGE_RX_BUF_LEN,
                              tx_buffer, PIOS_COM_BRIDGE_TX_BUF_LEN)) {
                PIOS_Assert(0);
            }
        }
#endif /* PIOS_INCLUDE_COM */
        break;
    case HWSETTINGS_USB_VCPPORT_DEBUGCONSOLE:
#if defined(PIOS_INCLUDE_COM)
#if defined(PIOS_INCLUDE_DEBUG_CONSOLE)
        {
            uint8_t *tx_buffer = (uint8_t *)pios_malloc(PIOS_COM_DEBUGCONSOLE_TX_BUF_LEN);
            PIOS_Assert(tx_buffer);
            if (PIOS_COM_Init(&pios_com_debug_id, &pios_usb_cdc_com_driver, pios_usb_cdc_id,
                              NULL, 0,
                              tx_buffer, PIOS_COM_DEBUGCONSOLE_TX_BUF_LEN)) {
                PIOS_Assert(0);
            }
        }
#endif /* PIOS_INCLUDE_DEBUG_CONSOLE */
#endif /* PIOS_INCLUDE_COM */

        break;
    }
#endif /* PIOS_INCLUDE_USB_CDC */

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
            uint8_t *rx_buffer = (uint8_t *)pios_malloc(PIOS_COM_TELEM_USB_RX_BUF_LEN);
            uint8_t *tx_buffer = (uint8_t *)pios_malloc(PIOS_COM_TELEM_USB_TX_BUF_LEN);
            PIOS_Assert(rx_buffer);
            PIOS_Assert(tx_buffer);
            if (PIOS_COM_Init(&pios_com_telem_usb_id, &pios_usb_hid_com_driver, pios_usb_hid_id,
                              rx_buffer, PIOS_COM_TELEM_USB_RX_BUF_LEN,
                              tx_buffer, PIOS_COM_TELEM_USB_TX_BUF_LEN)) {
                PIOS_Assert(0);
            }
        }
#endif /* PIOS_INCLUDE_COM */
        break;
    }

#endif /* PIOS_INCLUDE_USB_HID */

    if (usb_hid_present || usb_cdc_present) {
        PIOS_USBHOOK_Activate();
    }
#endif /* PIOS_INCLUDE_USB */

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
        PIOS_Board_configure_com(&pios_usart_main_cfg, PIOS_COM_GPS_RX_BUF_LEN, PIOS_COM_GPS_TX_BUF_LEN, &pios_usart_com_driver, &pios_com_gps_id);
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
    case HWSETTINGS_RM_MAINPORT_DSM:
        // Force binding to zero on the main port
        hwsettings_DSMxBind = 0;

        // TODO: Define the various Channelgroup for Revo dsm inputs and handle here
        PIOS_Board_configure_dsm(&pios_usart_dsm_main_cfg, &pios_dsm_main_cfg,
                                 &pios_usart_com_driver, MANUALCONTROLSETTINGS_CHANNELGROUPS_DSMMAINPORT, &hwsettings_DSMxBind);
    break;
    case HWSETTINGS_RM_MAINPORT_DEBUGCONSOLE:
#if defined(PIOS_INCLUDE_DEBUG_CONSOLE)
        {
            PIOS_Board_configure_com(&pios_usart_main_cfg, 0, PIOS_COM_DEBUGCONSOLE_TX_BUF_LEN, &pios_usart_com_driver, &pios_com_debug_id);
        }
#endif /* PIOS_INCLUDE_DEBUG_CONSOLE */
        break;
    case HWSETTINGS_RM_MAINPORT_COMBRIDGE:
        PIOS_Board_configure_com(&pios_usart_main_cfg, PIOS_COM_BRIDGE_RX_BUF_LEN, PIOS_COM_BRIDGE_TX_BUF_LEN, &pios_usart_com_driver, &pios_com_bridge_id);
        break;
    case HWSETTINGS_RM_MAINPORT_OSDHK:
        PIOS_Board_configure_com(&pios_usart_hkosd_main_cfg, PIOS_COM_HKOSD_RX_BUF_LEN, PIOS_COM_HKOSD_TX_BUF_LEN, &pios_usart_com_driver, &pios_com_hkosd_id);
        break;
    } /*        hwsettings_rm_mainport */

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
#endif /* PIOS_INCLUDE_I2C */
        break;
    case HWSETTINGS_RM_FLEXIPORT_GPS:
        PIOS_Board_configure_com(&pios_usart_flexi_cfg, PIOS_COM_GPS_RX_BUF_LEN, PIOS_COM_GPS_TX_BUF_LEN, &pios_usart_com_driver, &pios_com_gps_id);
        break;
    case HWSETTINGS_RM_FLEXIPORT_DSM:
        // TODO: Define the various Channelgroup for Revo dsm inputs and handle here
        PIOS_Board_configure_dsm(&pios_usart_dsm_flexi_cfg, &pios_dsm_flexi_cfg,
                                 &pios_usart_com_driver, MANUALCONTROLSETTINGS_CHANNELGROUPS_DSMFLEXIPORT, &hwsettings_DSMxBind);
        break;
    case HWSETTINGS_RM_FLEXIPORT_DEBUGCONSOLE:
#if defined(PIOS_INCLUDE_DEBUG_CONSOLE)
        {
            PIOS_Board_configure_com(&pios_usart_main_cfg, 0, PIOS_COM_DEBUGCONSOLE_TX_BUF_LEN, &pios_usart_com_driver, &pios_com_debug_id);
        }
#endif /* PIOS_INCLUDE_DEBUG_CONSOLE */
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

    /* Fetch the OPinkSettings object. */
    OPLinkSettingsData oplinkSettings;
    OPLinkSettingsGet(&oplinkSettings);

    // Initialize out status object.
    OPLinkStatusData oplinkStatus;
    OPLinkStatusGet(&oplinkStatus);
    oplinkStatus.BoardType     = bdinfo->board_type;
    PIOS_BL_HELPER_FLASH_Read_Description(oplinkStatus.Description, OPLINKSTATUS_DESCRIPTION_NUMELEM);
    PIOS_SYS_SerialNumberGetBinary(oplinkStatus.CPUSerial);
    oplinkStatus.BoardRevision = bdinfo->board_rev;

    /* Is the radio turned on? */
    bool is_coordinator = (oplinkSettings.Coordinator == OPLINKSETTINGS_COORDINATOR_TRUE);
    bool is_oneway = (oplinkSettings.OneWay == OPLINKSETTINGS_ONEWAY_TRUE);
    bool ppm_mode  = (oplinkSettings.PPM == OPLINKSETTINGS_PPM_TRUE);
    bool ppm_only  = (oplinkSettings.PPMOnly == OPLINKSETTINGS_PPMONLY_TRUE);
    if (oplinkSettings.MaxRFPower != OPLINKSETTINGS_MAXRFPOWER_0) {
        /* Configure the RFM22B device. */
        const struct pios_rfm22b_cfg *rfm22b_cfg = PIOS_BOARD_HW_DEFS_GetRfm22Cfg(bdinfo->board_rev);
        if (PIOS_RFM22B_Init(&pios_rfm22b_id, PIOS_RFM22_SPI_PORT, rfm22b_cfg->slave_num, rfm22b_cfg)) {
            PIOS_Assert(0);
        }

        /* Configure the radio com interface */
        uint8_t *rx_buffer = (uint8_t *)pios_malloc(PIOS_COM_RFM22B_RF_RX_BUF_LEN);
        uint8_t *tx_buffer = (uint8_t *)pios_malloc(PIOS_COM_RFM22B_RF_TX_BUF_LEN);
        PIOS_Assert(rx_buffer);
        PIOS_Assert(tx_buffer);
        if (PIOS_COM_Init(&pios_com_rf_id, &pios_rfm22b_com_driver, pios_rfm22b_id,
                          rx_buffer, PIOS_COM_RFM22B_RF_RX_BUF_LEN,
                          tx_buffer, PIOS_COM_RFM22B_RF_TX_BUF_LEN)) {
            PIOS_Assert(0);
        }
        /* Set Telemetry to use OPLinkMini if no other telemetry is configured (USB always overrides anyway) */
        if (!pios_com_telem_rf_id) {
            pios_com_telem_rf_id = pios_com_rf_id;
        }
        oplinkStatus.LinkState = OPLINKSTATUS_LINKSTATE_ENABLED;

        // Set the RF data rate on the modem to ~2X the selected buad rate because the modem is half duplex.
        enum rfm22b_datarate datarate = RFM22_datarate_64000;
        switch (oplinkSettings.ComSpeed) {
        case OPLINKSETTINGS_COMSPEED_4800:
            datarate = RFM22_datarate_9600;
            break;
        case OPLINKSETTINGS_COMSPEED_9600:
            datarate = RFM22_datarate_19200;
            break;
        case OPLINKSETTINGS_COMSPEED_19200:
            datarate = RFM22_datarate_32000;
            break;
        case OPLINKSETTINGS_COMSPEED_38400:
            datarate = RFM22_datarate_64000;
            break;
        case OPLINKSETTINGS_COMSPEED_57600:
            datarate = RFM22_datarate_100000;
            break;
        case OPLINKSETTINGS_COMSPEED_115200:
            datarate = RFM22_datarate_192000;
            break;
        }

        /* Set the radio configuration parameters. */
        PIOS_RFM22B_SetCoordinatorID(pios_rfm22b_id, oplinkSettings.CoordID);
        PIOS_RFM22B_SetChannelConfig(pios_rfm22b_id, datarate, oplinkSettings.MinChannel, oplinkSettings.MaxChannel, oplinkSettings.ChannelSet, is_coordinator, is_oneway, ppm_mode, ppm_only);

        /* Set the PPM callback if we should be receiving PPM. */
        if (ppm_mode) {
            PIOS_RFM22B_SetPPMCallback(pios_rfm22b_id, PIOS_Board_PPM_callback);
        }

        /* Set the modem Tx poer level */
        switch (oplinkSettings.MaxRFPower) {
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

        /* Reinitialize the modem. */
        PIOS_RFM22B_Reinit(pios_rfm22b_id);
    } else {
        oplinkStatus.LinkState = OPLINKSTATUS_LINKSTATE_DISABLED;
    }

    OPLinkStatusSet(&oplinkStatus);
#endif /* PIOS_INCLUDE_RFM22B */

#if defined(PIOS_INCLUDE_PWM) || defined(PIOS_INCLUDE_PWM)

    const struct pios_servo_cfg *pios_servo_cfg;
    // default to servo outputs only
    pios_servo_cfg = &pios_servo_cfg_out;
#endif

    /* Configure the receiver port*/
    uint8_t hwsettings_rcvrport;
    HwSettingsRM_RcvrPortGet(&hwsettings_rcvrport);
    //
    switch (hwsettings_rcvrport) {
    case HWSETTINGS_RM_RCVRPORT_DISABLED:
        break;
    case HWSETTINGS_RM_RCVRPORT_PWM:
#if defined(PIOS_INCLUDE_PWM)
        /* Set up the receiver port.  Later this should be optional */
        PIOS_Board_configure_pwm(&pios_pwm_cfg);
#endif /* PIOS_INCLUDE_PWM */
        break;
    case HWSETTINGS_RM_RCVRPORT_PPM:
    case HWSETTINGS_RM_RCVRPORT_PPMOUTPUTS:
    case HWSETTINGS_RM_RCVRPORT_PPMPWM:
    case HWSETTINGS_RM_RCVRPORT_PPMTELEMETRY:
#if defined(PIOS_INCLUDE_PPM)
        PIOS_Board_configure_ppm(&pios_ppm_cfg);

        if (hwsettings_rcvrport == HWSETTINGS_RM_RCVRPORT_PPMOUTPUTS) {
            // configure servo outputs and the remaining 5 inputs as outputs
            pios_servo_cfg = &pios_servo_cfg_out_in_ppm;
        }

        // enable pwm on the remaining channels
        if (hwsettings_rcvrport == HWSETTINGS_RM_RCVRPORT_PPMPWM) {
            PIOS_Board_configure_pwm(&pios_pwm_ppm_cfg);
        }

        if (hwsettings_rcvrport == HWSETTINGS_RM_RCVRPORT_PPMTELEMETRY) {
            PIOS_Board_configure_com(&pios_usart_rcvrport_cfg, PIOS_COM_TELEM_RF_RX_BUF_LEN, PIOS_COM_TELEM_RF_TX_BUF_LEN, &pios_usart_com_driver, &pios_com_telem_rf_id);
        }

        break;
#endif /* PIOS_INCLUDE_PPM */
    case HWSETTINGS_RM_RCVRPORT_OUTPUTS:
        // configure only the servo outputs
        pios_servo_cfg = &pios_servo_cfg_out_in;
        break;
    case HWSETTINGS_RM_RCVRPORT_TELEMETRY:
        PIOS_Board_configure_com(&pios_usart_rcvrport_cfg, PIOS_COM_TELEM_RF_RX_BUF_LEN, PIOS_COM_TELEM_RF_TX_BUF_LEN, &pios_usart_com_driver, &pios_com_telem_rf_id);
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
#endif /* PIOS_INCLUDE_GCSRCVR */

#if defined(PIOS_INCLUDE_OPLINKRCVR)
    {
        OPLinkReceiverInitialize();
        uint32_t pios_oplinkrcvr_id;
        PIOS_OPLinkRCVR_Init(&pios_oplinkrcvr_id);
        uint32_t pios_oplinkrcvr_rcvr_id;
        if (PIOS_RCVR_Init(&pios_oplinkrcvr_rcvr_id, &pios_oplinkrcvr_rcvr_driver, pios_oplinkrcvr_id)) {
            PIOS_Assert(0);
        }
        pios_rcvr_group_map[MANUALCONTROLSETTINGS_CHANNELGROUPS_OPLINK] = pios_oplinkrcvr_rcvr_id;
    }
#endif /* PIOS_INCLUDE_OPLINKRCVR */

#ifndef PIOS_ENABLE_DEBUG_PINS
    // pios_servo_cfg points to the correct configuration based on input port settings
    PIOS_Servo_Init(pios_servo_cfg);
#else
    PIOS_DEBUG_Init(pios_tim_servoport_all_pins, NELEMENTS(pios_tim_servoport_all_pins));
#endif

    // Disable GPIO_A8 Pullup to prevent wrong results on battery voltage readout
    GPIO_InitTypeDef gpioA8 = {
        .GPIO_Speed = GPIO_Speed_2MHz,
        .GPIO_Mode  = GPIO_Mode_IN,
        .GPIO_PuPd  = GPIO_PuPd_NOPULL,
        .GPIO_Pin   = GPIO_Pin_8,
        .GPIO_OType = GPIO_OType_OD,
    };
    GPIO_Init(GPIOA, &gpioA8);

    if (PIOS_I2C_Init(&pios_i2c_mag_pressure_adapter_id, &pios_i2c_mag_pressure_adapter_cfg)) {
        PIOS_DEBUG_Assert(0);
    }

    PIOS_DELAY_WaitmS(50);

#if defined(PIOS_INCLUDE_ADC)
    PIOS_ADC_Init(&pios_adc_cfg);
#endif

#if defined(PIOS_INCLUDE_HMC5X83)
    onboard_mag = PIOS_HMC5x83_Init(&pios_hmc5x83_cfg, pios_i2c_mag_pressure_adapter_id, 0);
#endif

#if defined(PIOS_INCLUDE_MS5611)
    PIOS_MS5611_Init(&pios_ms5611_cfg, pios_i2c_mag_pressure_adapter_id);
#endif

#if defined(PIOS_INCLUDE_MPU6000)
    PIOS_MPU6000_Init(pios_spi_gyro_id, 0, &pios_mpu6000_cfg);
    PIOS_MPU6000_CONFIG_Configure();
#endif

#ifdef PIOS_INCLUDE_WS2811
#include <pios_ws2811.h>
    HwSettingsWS2811LED_OutOptions ws2811_pin_settings;
    HwSettingsWS2811LED_OutGet(&ws2811_pin_settings);

    if (ws2811_pin_settings != HWSETTINGS_WS2811LED_OUT_DISABLED && ws2811_pin_settings < NELEMENTS(pios_ws2811_pin_cfg)) {
        PIOS_WS2811_Init(&pios_ws2811_cfg, &pios_ws2811_pin_cfg[ws2811_pin_settings]);
    }

#endif // PIOS_INCLUDE_WS2811
}

/**
 * @}
 * @}
 */

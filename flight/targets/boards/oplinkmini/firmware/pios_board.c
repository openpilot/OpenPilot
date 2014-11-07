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

#include "inc/openpilot.h"
#include <pios_board_info.h>
#include <pios_ppm_out.h>
#include <oplinksettings.h>
#include <taskinfo.h>
#ifdef PIOS_INCLUDE_SERVO
#include <pios_servo.h>
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

#define PIOS_COM_TELEM_USB_RX_BUF_LEN 256
#define PIOS_COM_TELEM_USB_TX_BUF_LEN 256

#define PIOS_COM_TELEM_VCP_RX_BUF_LEN 256
#define PIOS_COM_TELEM_VCP_TX_BUF_LEN 256

#define PIOS_COM_RFM22B_RF_RX_BUF_LEN 256
#define PIOS_COM_RFM22B_RF_TX_BUF_LEN 256

#define PIOS_COM_TELEM_RX_BUF_LEN     256
#define PIOS_COM_TELEM_TX_BUF_LEN     256

uint32_t pios_com_telem_usb_id = 0;
uint32_t pios_com_telem_vcp_id = 0;
uint32_t pios_com_telem_uart_main_id = 0;
uint32_t pios_com_telem_uart_flexi_id = 0;
uint32_t pios_com_telemetry_id = 0;
#if defined(PIOS_INCLUDE_PPM)
uint32_t pios_ppm_rcvr_id   = 0;
#endif
#if defined(PIOS_INCLUDE_PPM_OUT)
uint32_t pios_ppm_out_id    = 0;
#endif
#if defined(PIOS_INCLUDE_RFM22B)
uint32_t pios_rfm22b_id     = 0;
uint32_t pios_com_rfm22b_id = 0;
uint32_t pios_com_radio_id  = 0;
#endif
uint8_t *pios_uart_rx_buffer;
uint8_t *pios_uart_tx_buffer;

uintptr_t pios_uavo_settings_fs_id;
uintptr_t pios_user_fs_id = 0;

uint8_t servo_count = 0;

// Forward definitions
static void PIOS_Board_PPM_callback(const int16_t *channels);

/**
 * PIOS_Board_Init()
 * initializes all the core subsystems on this specific hardware
 * called from System/openpilot.c
 */
void PIOS_Board_Init(void)
{
    /* Delay system */
    PIOS_DELAY_Init();

#ifdef PIOS_INCLUDE_FLASH_LOGFS_SETTINGS
    uintptr_t flash_id;
    PIOS_Flash_Internal_Init(&flash_id, &flash_internal_cfg);
    PIOS_FLASHFS_Logfs_Init(&pios_uavo_settings_fs_id, &flashfs_internal_cfg, &pios_internal_flash_driver, flash_id);
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

#if defined(PIOS_INCLUDE_LED)
    PIOS_LED_Init(&pios_led_cfg);
#endif /* PIOS_INCLUDE_LED */

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

#if defined(PIOS_INCLUDE_RFM22B)
    OPLinkSettingsInitialize();
    OPLinkStatusInitialize();
#endif /* PIOS_INCLUDE_RFM22B */


#if defined(PIOS_INCLUDE_TIM)
    /* Set up pulse timers */
    PIOS_TIM_InitClock(&tim_1_cfg);
    PIOS_TIM_InitClock(&tim_2_cfg);
    PIOS_TIM_InitClock(&tim_3_cfg);
    PIOS_TIM_InitClock(&tim_4_cfg);
#endif /* PIOS_INCLUDE_TIM */

    /* Initialize board specific USB data */
    PIOS_USB_BOARD_DATA_Init();


#if defined(PIOS_INCLUDE_USB_CDC)
    /* Flags to determine if various USB interfaces are advertised */
    bool usb_cdc_present = false;

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

    /* Configure the USB virtual com port (VCP) */
#if defined(PIOS_INCLUDE_USB_CDC)
    if (usb_cdc_present) {
        uint32_t pios_usb_cdc_id;
        if (PIOS_USB_CDC_Init(&pios_usb_cdc_id, &pios_usb_cdc_cfg, pios_usb_id)) {
            PIOS_Assert(0);
        }
        uint8_t *rx_buffer = (uint8_t *)pios_malloc(PIOS_COM_TELEM_VCP_RX_BUF_LEN);
        uint8_t *tx_buffer = (uint8_t *)pios_malloc(PIOS_COM_TELEM_VCP_TX_BUF_LEN);
        PIOS_Assert(rx_buffer);
        PIOS_Assert(tx_buffer);
        if (PIOS_COM_Init(&pios_com_telem_vcp_id, &pios_usb_cdc_com_driver, pios_usb_cdc_id,
                          rx_buffer, PIOS_COM_TELEM_VCP_RX_BUF_LEN,
                          tx_buffer, PIOS_COM_TELEM_VCP_TX_BUF_LEN)) {
            PIOS_Assert(0);
        }
    }
#endif

    /* Allocate the uart buffers. */
    pios_uart_rx_buffer = (uint8_t *)pios_malloc(PIOS_COM_TELEM_RX_BUF_LEN);
    pios_uart_tx_buffer = (uint8_t *)pios_malloc(PIOS_COM_TELEM_TX_BUF_LEN);

    // Configure the main port
    OPLinkSettingsData oplinkSettings;
    OPLinkSettingsGet(&oplinkSettings);
    bool is_coordinator = (oplinkSettings.Coordinator == OPLINKSETTINGS_COORDINATOR_TRUE);
    bool is_oneway   = (oplinkSettings.OneWay == OPLINKSETTINGS_ONEWAY_TRUE);
    bool ppm_only    = (oplinkSettings.PPMOnly == OPLINKSETTINGS_PPMONLY_TRUE);
    bool ppm_mode    = false;
    bool servo_main  = false;
    bool servo_flexi = false;
    switch (oplinkSettings.MainPort) {
    case OPLINKSETTINGS_MAINPORT_TELEMETRY:
    case OPLINKSETTINGS_MAINPORT_SERIAL:
    {
        /* Configure the main port for uart serial */
#ifndef PIOS_RFM22B_DEBUG_ON_TELEM
        uint32_t pios_usart1_id;
        if (PIOS_USART_Init(&pios_usart1_id, &pios_usart_serial_cfg)) {
            PIOS_Assert(0);
        }
        PIOS_Assert(pios_uart_rx_buffer);
        PIOS_Assert(pios_uart_tx_buffer);
        if (PIOS_COM_Init(&pios_com_telem_uart_main_id, &pios_usart_com_driver, pios_usart1_id,
                          pios_uart_rx_buffer, PIOS_COM_TELEM_RX_BUF_LEN,
                          pios_uart_tx_buffer, PIOS_COM_TELEM_TX_BUF_LEN)) {
            PIOS_Assert(0);
        }
        PIOS_COM_TELEMETRY = PIOS_COM_TELEM_UART_MAIN;
#endif
        break;
    }
    case OPLINKSETTINGS_MAINPORT_PPM:
    {
#if defined(PIOS_INCLUDE_PPM)
        /* PPM input is configured on the coordinator modem and output on the remote modem. */
        if (is_coordinator) {
            uint32_t pios_ppm_id;
            PIOS_PPM_Init(&pios_ppm_id, &pios_ppm_main_cfg);

            if (PIOS_RCVR_Init(&pios_ppm_rcvr_id, &pios_ppm_rcvr_driver, pios_ppm_id)) {
                PIOS_Assert(0);
            }
        }
        // For some reason, PPM output on the main port doesn't work.
#if defined(PIOS_INCLUDE_PPM_OUT)
        else {
            PIOS_PPM_Out_Init(&pios_ppm_out_id, &pios_main_ppm_out_cfg);
        }
#endif /* PIOS_INCLUDE_PPM_OUT */
        ppm_mode = true;
#endif /* PIOS_INCLUDE_PPM */
        break;
    }
    case OPLINKSETTINGS_MAINPORT_PWM:
        servo_main = true;
        break;
    case OPLINKSETTINGS_MAINPORT_DISABLED:
        break;
    }

    // Configure the flexi port
    switch (oplinkSettings.FlexiPort) {
    case OPLINKSETTINGS_FLEXIPORT_TELEMETRY:
    case OPLINKSETTINGS_FLEXIPORT_SERIAL:
    {
        /* Configure the flexi port as uart serial */
        uint32_t pios_usart3_id;

        if (PIOS_USART_Init(&pios_usart3_id, &pios_usart_telem_flexi_cfg)) {
            PIOS_Assert(0);
        }
        PIOS_Assert(pios_uart_rx_buffer);
        PIOS_Assert(pios_uart_tx_buffer);
        if (PIOS_COM_Init(&pios_com_telem_uart_flexi_id, &pios_usart_com_driver, pios_usart3_id,
                          pios_uart_rx_buffer, PIOS_COM_TELEM_RX_BUF_LEN,
                          pios_uart_tx_buffer, PIOS_COM_TELEM_TX_BUF_LEN)) {
            PIOS_Assert(0);
        }
        PIOS_COM_TELEMETRY = PIOS_COM_TELEM_UART_FLEXI;
        break;
    }
    case OPLINKSETTINGS_FLEXIPORT_PPM:
    {
#if defined(PIOS_INCLUDE_PPM)
        /* PPM input is configured on the coordinator modem and output on the remote modem. */
        if (is_coordinator) {
            uint32_t pios_ppm_id;
            PIOS_PPM_Init(&pios_ppm_id, &pios_ppm_flexi_cfg);

            if (PIOS_RCVR_Init(&pios_ppm_rcvr_id, &pios_ppm_rcvr_driver, pios_ppm_id)) {
                PIOS_Assert(0);
            }
        } else {
            PIOS_PPM_Out_Init(&pios_ppm_out_id, &pios_flexi_ppm_out_cfg);
        }
#endif /* PIOS_INCLUDE_PPM */
        ppm_mode = true;
        break;
    }
    case OPLINKSETTINGS_FLEXIPORT_PWM:
        servo_flexi = true;
        break;
    case OPLINKSETTINGS_FLEXIPORT_DISABLED:
        break;
    }

    // Configure the USB VCP port
    switch (oplinkSettings.VCPPort) {
    case OPLINKSETTINGS_VCPPORT_SERIAL:
        PIOS_COM_TELEMETRY = PIOS_COM_TELEM_USB_VCP;
        break;
    case OPLINKSETTINGS_VCPPORT_DISABLED:
        break;
    }

#if defined(PIOS_INCLUDE_SERVO)
    if (servo_main) {
        if (servo_flexi) {
            servo_count = 4;
            PIOS_Servo_Init(&pios_servo_main_flexi_cfg);
        } else {
            servo_count = 2;
            PIOS_Servo_Init(&pios_servo_main_cfg);
        }
    } else if (servo_flexi) {
        servo_count = 2;
        PIOS_Servo_Init(&pios_servo_flexi_cfg);
    }
    ppm_mode = ppm_mode || (servo_count > 0);
#endif

    // Initialize out status object.
    OPLinkStatusData oplinkStatus;
    OPLinkStatusGet(&oplinkStatus);

    // Get our hardware information.
    const struct pios_board_info *bdinfo = &pios_board_info_blob;

    oplinkStatus.BoardType     = bdinfo->board_type;
    PIOS_BL_HELPER_FLASH_Read_Description(oplinkStatus.Description, OPLINKSTATUS_DESCRIPTION_NUMELEM);
    PIOS_SYS_SerialNumberGetBinary(oplinkStatus.CPUSerial);
    oplinkStatus.BoardRevision = bdinfo->board_rev;

    /* Initalize the RFM22B radio COM device. */
    if (oplinkSettings.MaxRFPower != OPLINKSETTINGS_MAXRFPOWER_0) {
        oplinkStatus.LinkState = OPLINKSTATUS_LINKSTATE_ENABLED;

        // Configure the RFM22B device
        const struct pios_rfm22b_cfg *rfm22b_cfg = PIOS_BOARD_HW_DEFS_GetRfm22Cfg(bdinfo->board_rev);
        if (PIOS_RFM22B_Init(&pios_rfm22b_id, PIOS_RFM22_SPI_PORT, rfm22b_cfg->slave_num, rfm22b_cfg)) {
            PIOS_Assert(0);
        }

        // Configure the radio com interface
        uint8_t *rx_buffer = (uint8_t *)pios_malloc(PIOS_COM_RFM22B_RF_RX_BUF_LEN);
        uint8_t *tx_buffer = (uint8_t *)pios_malloc(PIOS_COM_RFM22B_RF_TX_BUF_LEN);
        PIOS_Assert(rx_buffer);
        PIOS_Assert(tx_buffer);
        if (PIOS_COM_Init(&pios_com_rfm22b_id, &pios_rfm22b_com_driver, pios_rfm22b_id,
                          rx_buffer, PIOS_COM_RFM22B_RF_RX_BUF_LEN,
                          tx_buffer, PIOS_COM_RFM22B_RF_TX_BUF_LEN)) {
            PIOS_Assert(0);
        }

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

        // Set the radio configuration parameters.
        PIOS_RFM22B_SetCoordinatorID(pios_rfm22b_id, oplinkSettings.CoordID);
        PIOS_RFM22B_SetChannelConfig(pios_rfm22b_id, datarate, oplinkSettings.MinChannel, oplinkSettings.MaxChannel, oplinkSettings.ChannelSet, is_coordinator, is_oneway, ppm_mode, ppm_only);

        /* Set the PPM callback if we should be receiving PPM. */
        if (ppm_mode) {
            PIOS_RFM22B_SetPPMCallback(pios_rfm22b_id, PIOS_Board_PPM_callback);
        }

        // Reinitilize the modem to affect te changes.
        PIOS_RFM22B_Reinit(pios_rfm22b_id);
    } else {
        oplinkStatus.LinkState = OPLINKSTATUS_LINKSTATE_DISABLED;
    }

    // Update the object
    OPLinkStatusSet(&oplinkStatus);

    // Update the com baud rate.
    uint32_t comBaud = 9600;
    switch (oplinkSettings.ComSpeed) {
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
    if (PIOS_COM_TELEMETRY) {
        PIOS_COM_ChangeBaud(PIOS_COM_TELEMETRY, comBaud);
    }

    /* Remap AFIO pin */
    GPIO_PinRemapConfig(GPIO_Remap_SWJ_NoJTRST, ENABLE);

#ifdef PIOS_INCLUDE_ADC
    PIOS_ADC_Init();
#endif
}

static void PIOS_Board_PPM_callback(const int16_t *channels)
{
#if defined(PIOS_INCLUDE_PPM) && defined(PIOS_INCLUDE_PPM_OUT)
    if (pios_ppm_out_id) {
        for (uint8_t i = 0; i < RFM22B_PPM_NUM_CHANNELS; ++i) {
            if (channels[i] != PIOS_RCVR_INVALID) {
                PIOS_PPM_OUT_Set(PIOS_PPM_OUTPUT, i, channels[i]);
            }
        }
    }
#if defined(PIOS_INCLUDE_SERVO)
    for (uint8_t i = 0; i < servo_count; ++i) {
        uint16_t val = (channels[i] == PIOS_RCVR_INVALID) ? 0 : channels[i];
        PIOS_Servo_Set(i, val);
    }
#endif /* PIOS_INCLUDE_SERVO */
#endif /* PIOS_INCLUDE_PPM && PIOS_INCLUDE_PPM_OUT */
}

/**
 * @}
 */

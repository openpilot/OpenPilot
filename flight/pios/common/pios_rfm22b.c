/**
 ******************************************************************************
 * @addtogroup PIOS PIOS Core hardware abstraction layer
 * @{
 * @addtogroup   PIOS_RFM22B Radio Functions
 * @brief PIOS interface for for the RFM22B radio
 * @{
 *
 * @file       pios_rfm22b.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2012.
 * @brief      Implements a driver the the RFM22B driver
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

// *****************************************************************
// RFM22B hardware layer
//
// This module uses the RFM22B's internal packet handling hardware to
// encapsulate our own packet data.
//
// The RFM22B internal hardware packet handler configuration is as follows ..
//
// 4-byte (32-bit) preamble .. alternating 0's & 1's
// 4-byte (32-bit) sync
// 1-byte packet length (number of data bytes to follow)
// 0 to 255 user data bytes
//
// Our own packet data will also contain it's own header and 32-bit CRC
// as a single 16-bit CRC is not sufficient for wireless comms.
//
// *****************************************************************

#include "pios.h"

#ifdef PIOS_INCLUDE_RFM22B

#include <pios_spi_priv.h>
#include <pios_rfm22b_priv.h>
#include <pios_ppm_out.h>
#include <ecc.h>

/* Local Defines */
#define STACK_SIZE_BYTES                 200
#define TASK_PRIORITY                    (tskIDLE_PRIORITY + 2)
#define ISR_TIMEOUT                      1 // ms
#define EVENT_QUEUE_SIZE                 5
#define RFM22B_DEFAULT_RX_DATARATE       RFM22_datarate_9600
#define RFM22B_DEFAULT_TX_POWER          RFM22_tx_pwr_txpow_0
#define RFM22B_NOMINAL_CARRIER_FREQUENCY 430000000
#define RFM22B_LINK_QUALITY_THRESHOLD    20
#define RFM22B_DEFAULT_NUM_CHANNELS      1
#define RFM22B_DEFAULT_MIN_CHANNEL       0
#define RFM22B_DEFAULT_MAX_CHANNEL       250
#define RFM22B_DEFAULT_CHANNEL_SET       24
#define RFM22B_DEFAULT_PACKET_PERIOD     15

// The maximum amount of time without activity before initiating a reset.
#define PIOS_RFM22B_SUPERVISOR_TIMEOUT   100  // ms

// this is too adjust the RF module so that it is on frequency
#define OSC_LOAD_CAP                     0x7F  // cap = 12.5pf .. default

#define TX_PREAMBLE_NIBBLES              12  // 7 to 511 (number of nibbles)
#define RX_PREAMBLE_NIBBLES              6   // 5 to 31 (number of nibbles)
#define SYNC_BYTES                       4
#define HEADER_BYTES                     4

// the size of the rf modules internal FIFO buffers
#define FIFO_SIZE                        64

#define TX_FIFO_HI_WATERMARK             62  // 0-63
#define TX_FIFO_LO_WATERMARK             32  // 0-63

#define RX_FIFO_HI_WATERMARK             32 // 0-63

// preamble byte (preceeds SYNC_BYTE's)
#define PREAMBLE_BYTE                    0x55

// RF sync bytes (32-bit in all)
#define SYNC_BYTE_1                      0x2D
#define SYNC_BYTE_2                      0xD4
#define SYNC_BYTE_3                      0x4B
#define SYNC_BYTE_4                      0x59

#ifndef RX_LED_ON
#define RX_LED_ON
#define RX_LED_OFF
#define TX_LED_ON
#define TX_LED_OFF
#define LINK_LED_ON
#define LINK_LED_OFF
#define USB_LED_ON
#define USB_LED_OFF
#endif

/* Local type definitions */

struct pios_rfm22b_transition {
    enum pios_radio_event (*entry_fn)(struct pios_rfm22b_dev *rfm22b_dev);
    enum pios_radio_state next_state[RADIO_EVENT_NUM_EVENTS];
};

// Must ensure these prefilled arrays match the define sizes
static const uint8_t FULL_PREAMBLE[FIFO_SIZE] = {
    PREAMBLE_BYTE, PREAMBLE_BYTE, PREAMBLE_BYTE, PREAMBLE_BYTE, PREAMBLE_BYTE,
    PREAMBLE_BYTE, PREAMBLE_BYTE, PREAMBLE_BYTE, PREAMBLE_BYTE, PREAMBLE_BYTE,
    PREAMBLE_BYTE, PREAMBLE_BYTE, PREAMBLE_BYTE, PREAMBLE_BYTE, PREAMBLE_BYTE,
    PREAMBLE_BYTE, PREAMBLE_BYTE, PREAMBLE_BYTE, PREAMBLE_BYTE, PREAMBLE_BYTE,
    PREAMBLE_BYTE, PREAMBLE_BYTE, PREAMBLE_BYTE, PREAMBLE_BYTE, PREAMBLE_BYTE,
    PREAMBLE_BYTE, PREAMBLE_BYTE, PREAMBLE_BYTE, PREAMBLE_BYTE, PREAMBLE_BYTE,
    PREAMBLE_BYTE, PREAMBLE_BYTE, PREAMBLE_BYTE, PREAMBLE_BYTE, PREAMBLE_BYTE,
    PREAMBLE_BYTE, PREAMBLE_BYTE, PREAMBLE_BYTE, PREAMBLE_BYTE, PREAMBLE_BYTE,
    PREAMBLE_BYTE, PREAMBLE_BYTE, PREAMBLE_BYTE, PREAMBLE_BYTE, PREAMBLE_BYTE,
    PREAMBLE_BYTE, PREAMBLE_BYTE, PREAMBLE_BYTE, PREAMBLE_BYTE, PREAMBLE_BYTE,
    PREAMBLE_BYTE, PREAMBLE_BYTE, PREAMBLE_BYTE, PREAMBLE_BYTE, PREAMBLE_BYTE,
    PREAMBLE_BYTE, PREAMBLE_BYTE, PREAMBLE_BYTE, PREAMBLE_BYTE, PREAMBLE_BYTE,
    PREAMBLE_BYTE, PREAMBLE_BYTE, PREAMBLE_BYTE, PREAMBLE_BYTE
}; // 64 bytes
static const uint8_t HEADER[(TX_PREAMBLE_NIBBLES + 1) / 2 + 2] = { PREAMBLE_BYTE, PREAMBLE_BYTE, PREAMBLE_BYTE, PREAMBLE_BYTE, PREAMBLE_BYTE, PREAMBLE_BYTE, SYNC_BYTE_1, SYNC_BYTE_2 };
static const uint8_t OUT_FF[64] = {
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
};
// The randomized channel list.
static const uint8_t channel_list[] = { 68, 34, 2, 184, 166, 94, 204, 18, 47, 118, 239, 176, 5, 213, 218, 186, 104, 160, 199, 209, 231, 197, 92, 191, 88, 129, 40, 19, 93, 200, 156, 14, 247, 182, 193, 194, 208, 210, 248, 76, 244, 48, 179, 105, 25, 74, 155, 203, 39, 97, 195, 81, 83, 180, 134, 172, 235, 132, 198, 119, 207, 154, 0, 61, 140, 171, 245, 26, 95, 3, 22, 62, 169, 55, 127, 144, 45, 33, 170, 91, 158, 167, 63, 201, 41, 21, 190, 51, 103, 49, 189, 205, 240, 89, 181, 149, 6, 157, 249, 230, 115, 72, 163, 17, 29, 99, 28, 117, 219, 73, 78, 53, 69, 216, 161, 124, 110, 242, 214, 145, 13, 11, 220, 113, 138, 58, 54, 162, 237, 37, 152, 187, 232, 77, 126, 85, 38, 238, 173, 23, 188, 100, 131, 226, 31, 9, 114, 106, 221, 42, 233, 139, 4, 241, 96, 211, 8, 98, 121, 147, 24, 217, 27, 87, 122, 125, 135, 148, 178, 71, 206, 57, 141, 35, 30, 246, 159, 16, 32, 15, 229, 20, 12, 223, 150, 101, 79, 56, 102, 111, 174, 236, 137, 143, 52, 225, 64, 224, 112, 168, 243, 130, 108, 202, 123, 146, 228, 75, 46, 153, 7, 192, 175, 151, 222, 59, 82, 90, 1, 65, 109, 44, 165, 84, 43, 36, 128, 196, 67, 80, 136, 86, 70, 234, 66, 185, 10, 164, 177, 116, 50, 107, 183, 215, 212, 60, 227, 133, 120, 142 };

/* Local function forwared declarations */
static void pios_rfm22_task(void *parameters);
static bool pios_rfm22_readStatus(struct pios_rfm22b_dev *rfm22b_dev);
static void pios_rfm22_setDatarate(struct pios_rfm22b_dev *rfm22b_dev);
static void rfm22_rxFailure(struct pios_rfm22b_dev *rfm22b_dev);
static void pios_rfm22_inject_event(struct pios_rfm22b_dev *rfm22b_dev, enum pios_radio_event event, bool inISR);
static enum pios_radio_event rfm22_init(struct pios_rfm22b_dev *rfm22b_dev);
static enum pios_radio_event radio_setRxMode(struct pios_rfm22b_dev *rfm22b_dev);
static enum pios_radio_event radio_rxData(struct pios_rfm22b_dev *rfm22b_dev);
static enum pios_radio_event radio_receivePacket(struct pios_rfm22b_dev *rfm22b_dev, uint8_t *p, uint16_t rx_len);
static enum pios_radio_event radio_txStart(struct pios_rfm22b_dev *rfm22b_dev);
static enum pios_radio_event radio_txData(struct pios_rfm22b_dev *rfm22b_dev);
static enum pios_radio_event rfm22_txFailure(struct pios_rfm22b_dev *rfm22b_dev);
static enum pios_radio_event rfm22_process_state_transition(struct pios_rfm22b_dev *rfm22b_dev, enum pios_radio_event event);
static void rfm22_process_event(struct pios_rfm22b_dev *rfm22b_dev, enum pios_radio_event event);
static enum pios_radio_event rfm22_timeout(struct pios_rfm22b_dev *rfm22b_dev);
static enum pios_radio_event rfm22_error(struct pios_rfm22b_dev *rfm22b_dev);
static enum pios_radio_event rfm22_fatal_error(struct pios_rfm22b_dev *rfm22b_dev);
static void rfm22b_add_rx_status(struct pios_rfm22b_dev *rfm22b_dev, enum pios_rfm22b_rx_packet_status status);
static void rfm22_setNominalCarrierFrequency(struct pios_rfm22b_dev *rfm22b_dev, uint8_t init_chan);
static bool rfm22_setFreqHopChannel(struct pios_rfm22b_dev *rfm22b_dev, uint8_t channel);
static void rfm22_updatePairStatus(struct pios_rfm22b_dev *radio_dev);
static void rfm22_calculateLinkQuality(struct pios_rfm22b_dev *rfm22b_dev);
static bool rfm22_isConnected(struct pios_rfm22b_dev *rfm22b_dev);
static bool rfm22_isCoordinator(struct pios_rfm22b_dev *rfm22b_dev);
static uint32_t rfm22_destinationID(struct pios_rfm22b_dev *rfm22b_dev);
static bool rfm22_timeToSend(struct pios_rfm22b_dev *rfm22b_dev);
static void rfm22_synchronizeClock(struct pios_rfm22b_dev *rfm22b_dev);
static portTickType rfm22_coordinatorTime(struct pios_rfm22b_dev *rfm22b_dev, portTickType ticks);
static uint8_t rfm22_calcChannel(struct pios_rfm22b_dev *rfm22b_dev, uint8_t index);
static uint8_t rfm22_calcChannelFromClock(struct pios_rfm22b_dev *rfm22b_dev);
static bool rfm22_changeChannel(struct pios_rfm22b_dev *rfm22b_dev);
static void rfm22_clearLEDs();

// Utility functions.
static uint32_t pios_rfm22_time_difference_ms(portTickType start_time, portTickType end_time);
static struct pios_rfm22b_dev *pios_rfm22_alloc(void);

// SPI read/write functions
static void rfm22_assertCs(struct pios_rfm22b_dev *rfm22b_dev);
static void rfm22_deassertCs(struct pios_rfm22b_dev *rfm22b_dev);
static void rfm22_claimBus(struct pios_rfm22b_dev *rfm22b_dev);
static void rfm22_releaseBus(struct pios_rfm22b_dev *rfm22b_dev);
static void rfm22_write_claim(struct pios_rfm22b_dev *rfm22b_dev, uint8_t addr, uint8_t data);
static void rfm22_write(struct pios_rfm22b_dev *rfm22b_dev, uint8_t addr, uint8_t data);
static uint8_t rfm22_read(struct pios_rfm22b_dev *rfm22b_dev, uint8_t addr);


/* The state transition table */
static const struct pios_rfm22b_transition rfm22b_transitions[RADIO_STATE_NUM_STATES] = {
    // Initialization thread
    [RADIO_STATE_UNINITIALIZED] = {
        .entry_fn   = 0,
        .next_state =             {
            [RADIO_EVENT_INITIALIZE] = RADIO_STATE_INITIALIZING,
            [RADIO_EVENT_ERROR] = RADIO_STATE_ERROR,
        },
    },
    [RADIO_STATE_INITIALIZING] =  {
        .entry_fn   = rfm22_init,
        .next_state =             {
            [RADIO_EVENT_INITIALIZED] = RADIO_STATE_RX_MODE,
            [RADIO_EVENT_ERROR] = RADIO_STATE_ERROR,
            [RADIO_EVENT_INITIALIZE]  = RADIO_STATE_INITIALIZING,
            [RADIO_EVENT_FATAL_ERROR] = RADIO_STATE_FATAL_ERROR,
        },
    },

    [RADIO_STATE_RX_MODE] =       {
        .entry_fn   = radio_setRxMode,
        .next_state =             {
            [RADIO_EVENT_TX_START]     = RADIO_STATE_TX_START,
            [RADIO_EVENT_INT_RECEIVED] = RADIO_STATE_RX_DATA,
            [RADIO_EVENT_TIMEOUT] = RADIO_STATE_TIMEOUT,
            [RADIO_EVENT_ERROR] = RADIO_STATE_ERROR,
            [RADIO_EVENT_INITIALIZE]   = RADIO_STATE_INITIALIZING,
            [RADIO_EVENT_FATAL_ERROR]  = RADIO_STATE_FATAL_ERROR,
        },
    },
    [RADIO_STATE_RX_DATA] =       {
        .entry_fn   = radio_rxData,
        .next_state =             {
            [RADIO_EVENT_INT_RECEIVED] = RADIO_STATE_RX_DATA,
            [RADIO_EVENT_TX_START]     = RADIO_STATE_TX_START,
            [RADIO_EVENT_RX_COMPLETE]  = RADIO_STATE_TX_START,
            [RADIO_EVENT_RX_MODE]     = RADIO_STATE_RX_MODE,
            [RADIO_EVENT_TIMEOUT]     = RADIO_STATE_TIMEOUT,
            [RADIO_EVENT_ERROR]       = RADIO_STATE_ERROR,
            [RADIO_EVENT_INITIALIZE]  = RADIO_STATE_INITIALIZING,
            [RADIO_EVENT_FATAL_ERROR] = RADIO_STATE_FATAL_ERROR,
        },
    },
    [RADIO_STATE_TX_START] =      {
        .entry_fn   = radio_txStart,
        .next_state =             {
            [RADIO_EVENT_INT_RECEIVED] = RADIO_STATE_TX_DATA,
            [RADIO_EVENT_RX_MODE]     = RADIO_STATE_RX_MODE,
            [RADIO_EVENT_TIMEOUT]     = RADIO_STATE_TIMEOUT,
            [RADIO_EVENT_ERROR]       = RADIO_STATE_ERROR,
            [RADIO_EVENT_INITIALIZE]  = RADIO_STATE_INITIALIZING,
            [RADIO_EVENT_FATAL_ERROR] = RADIO_STATE_FATAL_ERROR,
        },
    },
    [RADIO_STATE_TX_DATA] =       {
        .entry_fn   = radio_txData,
        .next_state =             {
            [RADIO_EVENT_INT_RECEIVED] = RADIO_STATE_TX_DATA,
            [RADIO_EVENT_RX_MODE]     = RADIO_STATE_RX_MODE,
            [RADIO_EVENT_TIMEOUT]     = RADIO_STATE_TIMEOUT,
            [RADIO_EVENT_ERROR]       = RADIO_STATE_ERROR,
            [RADIO_EVENT_INITIALIZE]  = RADIO_STATE_INITIALIZING,
            [RADIO_EVENT_FATAL_ERROR] = RADIO_STATE_FATAL_ERROR,
        },
    },
    [RADIO_STATE_TX_FAILURE] =    {
        .entry_fn   = rfm22_txFailure,
        .next_state =             {
            [RADIO_EVENT_TX_START]    = RADIO_STATE_TX_START,
            [RADIO_EVENT_TIMEOUT]     = RADIO_STATE_TIMEOUT,
            [RADIO_EVENT_ERROR]       = RADIO_STATE_ERROR,
            [RADIO_EVENT_INITIALIZE]  = RADIO_STATE_INITIALIZING,
            [RADIO_EVENT_FATAL_ERROR] = RADIO_STATE_FATAL_ERROR,
        },
    },
    [RADIO_STATE_TIMEOUT] =       {
        .entry_fn   = rfm22_timeout,
        .next_state =             {
            [RADIO_EVENT_TX_START]    = RADIO_STATE_TX_START,
            [RADIO_EVENT_RX_MODE]     = RADIO_STATE_RX_MODE,
            [RADIO_EVENT_ERROR]       = RADIO_STATE_ERROR,
            [RADIO_EVENT_INITIALIZE]  = RADIO_STATE_INITIALIZING,
            [RADIO_EVENT_FATAL_ERROR] = RADIO_STATE_FATAL_ERROR,
        },
    },
    [RADIO_STATE_ERROR] =         {
        .entry_fn   = rfm22_error,
        .next_state =             {
            [RADIO_EVENT_INITIALIZE]  = RADIO_STATE_INITIALIZING,
            [RADIO_EVENT_FATAL_ERROR] = RADIO_STATE_FATAL_ERROR,
        },
    },
    [RADIO_STATE_FATAL_ERROR] =   {
        .entry_fn   = rfm22_fatal_error,
        .next_state =             {},
    },
};

// xtal 10 ppm, 434MHz
static const uint32_t data_rate[] = { 9600, 19200, 32000, 57600, 64000, 128000, 192000, 256000 };
static const uint8_t modulation_index[] = { 1, 1, 1, 1, 1, 1, 1, 1 };

static const uint8_t reg_1C[] = { 0x01, 0x28, 0x16, 0x06, 0x07, 0x83, 0x8A, 0x8C }; // rfm22_if_filter_bandwidth

static const uint8_t reg_1D[] = { 0x40, 0x44, 0x44, 0x40, 0x44, 0x44, 0x44, 0x44 }; // rfm22_afc_loop_gearshift_override
static const uint8_t reg_1E[] = { 0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x02 }; // rfm22_afc_timing_control

static const uint8_t reg_1F[] = { 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03 }; // rfm22_clk_recovery_gearshift_override
static const uint8_t reg_20[] = { 0xA1, 0x34, 0x3F, 0x45, 0x3F, 0x5E, 0x3F, 0x2F }; // rfm22_clk_recovery_oversampling_ratio
static const uint8_t reg_21[] = { 0x20, 0x02, 0x02, 0x01, 0x02, 0x01, 0x02, 0x02 }; // rfm22_clk_recovery_offset2
static const uint8_t reg_22[] = { 0x4E, 0x75, 0x0C, 0xD7, 0x0c, 0x5D, 0x0C, 0xBB }; // rfm22_clk_recovery_offset1
static const uint8_t reg_23[] = { 0xA5, 0x25, 0x4A, 0xDC, 0x4A, 0x86, 0x4A, 0x0D }; // rfm22_clk_recovery_offset0
static const uint8_t reg_24[] = { 0x00, 0x07, 0x07, 0x07, 0x07, 0x05, 0x07, 0x07 }; // rfm22_clk_recovery_timing_loop_gain1
static const uint8_t reg_25[] = { 0x34, 0xFF, 0xFF, 0x6E, 0xFF, 0x74, 0xFF, 0xFF }; // rfm22_clk_recovery_timing_loop_gain0

static const uint8_t reg_2A[] = { 0x1E, 0x0E, 0x17, 0x2D, 0x31, 0x50, 0x50, 0x50 }; // rfm22_afc_limiter .. AFC_pull_in_range = �AFCLimiter[7:0] x (hbsel+1) x 625 Hz

static const uint8_t reg_58[] = { 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80 }; // rfm22_cpcuu
static const uint8_t reg_69[] = { 0x60, 0x20, 0x20, 0x60, 0x20, 0x20, 0x20, 0x20 }; // rfm22_agc_override1
static const uint8_t reg_6E[] = { 0x4E, 0x9D, 0x08, 0x0E, 0x10, 0x20, 0x31, 0x41 }; // rfm22_tx_data_rate1
static const uint8_t reg_6F[] = { 0xA5, 0x49, 0x31, 0xBF, 0x62, 0xC5, 0x27, 0x89 }; // rfm22_tx_data_rate0

static const uint8_t reg_70[] = { 0x2C, 0x2D, 0x0D, 0x0C, 0x0C, 0x0D, 0x0D, 0x0D }; // rfm22_modulation_mode_control1
static const uint8_t reg_71[] = { 0x23, 0x23, 0x23, 0x23, 0x23, 0x23, 0x23, 0x23 }; // rfm22_modulation_mode_control2

static const uint8_t reg_72[] = { 0x30, 0x0F, 0x1A, 0x2E, 0x33, 0x66, 0x9A, 0xCD }; // rfm22_frequency_deviation

static struct pios_rfm22b_dev *g_rfm22b_dev = NULL;


/*****************************************************************************
* External Interface Functions
*****************************************************************************/

/**
 * Initialise an RFM22B device
 *
 * @param[out] rfm22b_id  A pointer to store the device ID in.
 * @param[in] spi_id  The SPI bus index.
 * @param[in] slave_num  The SPI bus slave number.
 * @param[in] cfg  The device configuration.
 */
int32_t PIOS_RFM22B_Init(uint32_t *rfm22b_id, uint32_t spi_id, uint32_t slave_num, const struct pios_rfm22b_cfg *cfg)
{
    PIOS_DEBUG_Assert(rfm22b_id);
    PIOS_DEBUG_Assert(cfg);

    // Allocate the device structure.
    struct pios_rfm22b_dev *rfm22b_dev = (struct pios_rfm22b_dev *)pios_rfm22_alloc();
    if (!rfm22b_dev) {
        return -1;
    }
    *rfm22b_id   = (uint32_t)rfm22b_dev;
    g_rfm22b_dev = rfm22b_dev;

    // Store the SPI handle
    rfm22b_dev->slave_num     = slave_num;
    rfm22b_dev->spi_id        = spi_id;

    // Initialize our configuration parameters
    rfm22b_dev->datarate      = RFM22B_DEFAULT_RX_DATARATE;
    rfm22b_dev->tx_power      = RFM22B_DEFAULT_TX_POWER;
    rfm22b_dev->coordinator   = false;
    rfm22b_dev->coordinatorID = 0;

    // Initialize the com callbacks.
    rfm22b_dev->rx_in_cb      = NULL;
    rfm22b_dev->tx_out_cb     = NULL;

    // Initialize the stats.
    rfm22b_dev->stats.packets_per_sec = 0;
    rfm22b_dev->stats.rx_good = 0;
    rfm22b_dev->stats.rx_corrected    = 0;
    rfm22b_dev->stats.rx_error     = 0;
    rfm22b_dev->stats.rx_missed    = 0;
    rfm22b_dev->stats.tx_dropped   = 0;
    rfm22b_dev->stats.tx_resent    = 0;
    rfm22b_dev->stats.resets       = 0;
    rfm22b_dev->stats.timeouts     = 0;
    rfm22b_dev->stats.link_quality = 0;
    rfm22b_dev->stats.rssi = 0;
    rfm22b_dev->stats.tx_seq       = 0;
    rfm22b_dev->stats.rx_seq       = 0;
    rfm22b_dev->stats.tx_failure   = 0;

    // Initialize the channels.
    PIOS_RFM22B_SetChannelConfig(*rfm22b_id, RFM22B_DEFAULT_NUM_CHANNELS, RFM22B_DEFAULT_MIN_CHANNEL, RFM22B_DEFAULT_MAX_CHANNEL, RFM22B_DEFAULT_CHANNEL_SET, RFM22B_DEFAULT_PACKET_PERIOD, false);

    // Create the event queue
    rfm22b_dev->eventQueue = xQueueCreate(EVENT_QUEUE_SIZE, sizeof(enum pios_radio_event));

    // Bind the configuration to the device instance
    rfm22b_dev->cfg = *cfg;

    // Create a semaphore to know if an ISR needs responding to
    vSemaphoreCreateBinary(rfm22b_dev->isrPending);

    // Create our (hopefully) unique 32 bit id from the processor serial number.
    uint8_t crcs[] = { 0, 0, 0, 0 };
    {
        char serial_no_str[33];
        PIOS_SYS_SerialNumberGet(serial_no_str);
        // Create a 32 bit value using 4 8 bit CRC values.
        for (uint8_t i = 0; serial_no_str[i] != 0; ++i) {
            crcs[i % 4] = PIOS_CRC_updateByte(crcs[i % 4], serial_no_str[i]);
        }
    }
    rfm22b_dev->deviceID = crcs[0] | crcs[1] << 8 | crcs[2] << 16 | crcs[3] << 24;
    DEBUG_PRINTF(2, "RF device ID: %x\n\r", rfm22b_dev->deviceID);

    // Initialize the external interrupt.
    PIOS_EXTI_Init(cfg->exti_cfg);

    // Register the watchdog timer for the radio driver task
#if defined(PIOS_INCLUDE_WDG) && defined(PIOS_WDG_RFM22B)
    PIOS_WDG_RegisterFlag(PIOS_WDG_RFM22B);
#endif /* PIOS_WDG_RFM22B */

    // Initialize the ECC library.
    initialize_ecc();

    // Set the state to initializing.
    rfm22b_dev->state = RADIO_STATE_UNINITIALIZED;

    // Initialize the radio device.
    pios_rfm22_inject_event(rfm22b_dev, RADIO_EVENT_INITIALIZE, false);

    // Start the driver task.  This task controls the radio state machine and removed all of the IO from the IRQ handler.
    xTaskCreate(pios_rfm22_task, (signed char *)"PIOS_RFM22B_Task", STACK_SIZE_BYTES, (void *)rfm22b_dev, TASK_PRIORITY, &(rfm22b_dev->taskHandle));

    return 0;
}

/**
 * Re-initialize the modem after a configuration change.
 *
 * @param[in] rbm22b_id  The RFM22B device ID.
 */
void PIOS_RFM22B_Reinit(uint32_t rfm22b_id)
{
    struct pios_rfm22b_dev *rfm22b_dev = (struct pios_rfm22b_dev *)rfm22b_id;

    if (PIOS_RFM22B_Validate(rfm22b_dev)) {
        pios_rfm22_inject_event(rfm22b_dev, RADIO_EVENT_INITIALIZE, false);
    }
}

/**
 * The RFM22B external interrupt routine.
 */
bool PIOS_RFM22_EXT_Int(void)
{
    if (!PIOS_RFM22B_Validate(g_rfm22b_dev)) {
        return false;
    }

    // Inject an interrupt event into the state machine.
    pios_rfm22_inject_event(g_rfm22b_dev, RADIO_EVENT_INT_RECEIVED, true);
    return false;
}

/**
 * Returns the unique device ID for the RFM22B device.
 *
 * @param[in] rfm22b_id The RFM22B device index.
 * @return The unique device ID
 */
uint32_t PIOS_RFM22B_DeviceID(uint32_t rfm22b_id)
{
    struct pios_rfm22b_dev *rfm22b_dev = (struct pios_rfm22b_dev *)rfm22b_id;

    if (PIOS_RFM22B_Validate(rfm22b_dev)) {
        return rfm22b_dev->deviceID;
    }
    return 0;
}

/**
 * Are we connected to the remote modem?
 *
 * @param[in] rfm22b_dev  The device structure
 */
static bool rfm22_isConnected(struct pios_rfm22b_dev *rfm22b_dev)
{
    return (rfm22b_dev->stats.link_state == OPLINKSTATUS_LINKSTATE_CONNECTED) || (rfm22b_dev->stats.link_state == OPLINKSTATUS_LINKSTATE_CONNECTING);
}

/**
 * Returns true if the modem is not actively sending or receiving a packet.
 *
 * @param[in] rfm22b_id The RFM22B device index.
 * @return True if the modem is not actively sending or receiving a packet.
 */
bool PIOS_RFM22B_InRxWait(uint32_t rfm22b_id)
{
    struct pios_rfm22b_dev *rfm22b_dev = (struct pios_rfm22b_dev *)rfm22b_id;

    if (PIOS_RFM22B_Validate(rfm22b_dev)) {
        return (rfm22b_dev->rfm22b_state == RFM22B_STATE_RX_WAIT) || (rfm22b_dev->rfm22b_state == RFM22B_STATE_TRANSITION);
    }
    return false;
}

/**
 * Sets the radio device transmit power.
 *
 * @param[in] rfm22b_id The RFM22B device index.
 * @param[in] tx_pwr The transmit power.
 */
void PIOS_RFM22B_SetTxPower(uint32_t rfm22b_id, enum rfm22b_tx_power tx_pwr)
{
    struct pios_rfm22b_dev *rfm22b_dev = (struct pios_rfm22b_dev *)rfm22b_id;

    if (!PIOS_RFM22B_Validate(rfm22b_dev)) {
        return;
    }
    rfm22b_dev->tx_power = tx_pwr;
}

/**
 * Sets the range and number of channels to use for the radio.
 * The channels are 0 to 255 divided across the 430-440 MHz range.
 * The number of channels configured will be spread across the selected channel range.
 * The channel spacing is 10MHz / 250 = 40kHz
 *
 * @param[in] rfm22b_id  The RFM22B device index.
 * @param[in] num_channels  The number of channels to use for frequency hopping.
 * @param[in] min_chan  The minimum channel.
 * @param[in] max_chan  The maximum channel.
 * @param[in] chan_set  The "seed" for selecting a channel sequence.
 * @param[in] packet_period  The fixed time alloted to sending a single packet
 * @param[in] oneway Only the coordinator can send packets if true.
 */
void PIOS_RFM22B_SetChannelConfig(uint32_t rfm22b_id, uint8_t num_chan, uint8_t min_chan, uint8_t max_chan, uint8_t chan_set, uint8_t packet_period, bool oneway)
{
    struct pios_rfm22b_dev *rfm22b_dev = (struct pios_rfm22b_dev *)rfm22b_id;

    if (!PIOS_RFM22B_Validate(rfm22b_dev)) {
        return;
    }
    rfm22b_dev->packet_period = packet_period;
    rfm22b_dev->num_channels  = num_chan;
    rfm22b_dev->one_way_link  = oneway;

    // Find the first N channels that meet the min/max criteria out of the random channel list.
    uint8_t num_found = 0;
    for (uint16_t i = 0; (i < RFM22B_NUM_CHANNELS) && (num_found < num_chan); ++i) {
        uint8_t idx  = (i + chan_set) % RFM22B_NUM_CHANNELS;
        uint8_t chan = channel_list[idx];
        if ((chan >= min_chan) && (chan <= max_chan)) {
            rfm22b_dev->channels[num_found++] = chan;
        }
    }
}

/**
 * Set a modem to be a coordinator or not.
 *
 * @param[in] rfm22b_id The RFM22B device index.
 * @param[in] coordinator If true, this modem will be configured as a coordinator.
 */
extern void PIOS_RFM22B_SetCoordinator(uint32_t rfm22b_id, bool coordinator)
{
    struct pios_rfm22b_dev *rfm22b_dev = (struct pios_rfm22b_dev *)rfm22b_id;

    if (PIOS_RFM22B_Validate(rfm22b_dev)) {
        rfm22b_dev->coordinator = coordinator;
    }
}

/**
 * Sets the device coordinator ID.
 *
 * @param[in] rfm22b_id The RFM22B device index.
 * @param[in] coord_id The coordinator ID.
 */
void PIOS_RFM22B_SetCoordinatorID(uint32_t rfm22b_id, uint32_t coord_id)
{
    struct pios_rfm22b_dev *rfm22b_dev = (struct pios_rfm22b_dev *)rfm22b_id;

    if (PIOS_RFM22B_Validate(rfm22b_dev)) {
        rfm22b_dev->coordinatorID = coord_id;
    }
}

/**
 * Returns the device statistics RFM22B device.
 *
 * @param[in] rfm22b_id The RFM22B device index.
 * @param[out] stats The stats are returned in this structure
 */
void PIOS_RFM22B_GetStats(uint32_t rfm22b_id, struct rfm22b_stats *stats)
{
    struct pios_rfm22b_dev *rfm22b_dev = (struct pios_rfm22b_dev *)rfm22b_id;

    if (!PIOS_RFM22B_Validate(rfm22b_dev)) {
        return;
    }

    // Calculate the current link quality
    rfm22_calculateLinkQuality(rfm22b_dev);

    // Return the stats.
    *stats = rfm22b_dev->stats;
}

/**
 * Get the stats of the oter radio devices that are in range.
 *
 * @param[out] device_ids  A pointer to the array to store the device IDs.
 * @param[out] RSSIs  A pointer to the array to store the RSSI values in.
 * @param[in] mx_pairs  The length of the pdevice_ids and RSSIs arrays.
 * @return  The number of pair stats returned.
 */
uint8_t PIOS_RFM2B_GetPairStats(uint32_t rfm22b_id, uint32_t *device_ids, int8_t *RSSIs, uint8_t max_pairs)
{
    struct pios_rfm22b_dev *rfm22b_dev = (struct pios_rfm22b_dev *)rfm22b_id;

    if (!PIOS_RFM22B_Validate(rfm22b_dev)) {
        return 0;
    }

    uint8_t mp = (max_pairs >= OPLINKSTATUS_PAIRIDS_NUMELEM) ? max_pairs : OPLINKSTATUS_PAIRIDS_NUMELEM;
    for (uint8_t i = 0; i < mp; ++i) {
        device_ids[i] = rfm22b_dev->pair_stats[i].pairID;
        RSSIs[i] = rfm22b_dev->pair_stats[i].rssi;
    }

    return mp;
}

/**
 * Check the radio device for a valid connection
 *
 * @param[in] rfm22b_id  The rfm22b device.
 * @return true if there is a valid connection to paired radio, false otherwise.
 */
bool PIOS_RFM22B_LinkStatus(uint32_t rfm22b_id)
{
    struct pios_rfm22b_dev *rfm22b_dev = (struct pios_rfm22b_dev *)rfm22b_id;

    if (!PIOS_RFM22B_Validate(rfm22b_dev)) {
        return false;
    }
    return rfm22b_dev->stats.link_quality > RFM22B_LINK_QUALITY_THRESHOLD;
}

/**
 * Put the RFM22B device into receive mode.
 *
 * @param[in] rfm22b_id  The rfm22b device.
 * @param[in] p  The packet to receive into.
 * @return true if Rx mode was entered sucessfully.
 */
bool PIOS_RFM22B_ReceivePacket(uint32_t rfm22b_id, uint8_t *p)
{
    struct pios_rfm22b_dev *rfm22b_dev = (struct pios_rfm22b_dev *)rfm22b_id;

    if (!PIOS_RFM22B_Validate(rfm22b_dev)) {
        return false;
    }

    // Are we already in Rx mode?
    if ((rfm22b_dev->rfm22b_state == RFM22B_STATE_RX_MODE) || (rfm22b_dev->rfm22b_state == RFM22B_STATE_RX_WAIT)) {
        return false;
    }
    rfm22b_dev->rx_packet_handle = p;

    // Claim the SPI bus.
    rfm22_claimBus(rfm22b_dev);

    // disable interrupts
    rfm22_write(rfm22b_dev, RFM22_interrupt_enable1, 0x00);
    rfm22_write(rfm22b_dev, RFM22_interrupt_enable2, 0x00);

    // Switch to TUNE mode
    rfm22_write(rfm22b_dev, RFM22_op_and_func_ctrl1, RFM22_opfc1_pllon);

#ifdef PIOS_RFM22B_DEBUG_ON_TELEM
    D2_LED_OFF;
#endif // PIOS_RFM22B_DEBUG_ON_TELEM
    RX_LED_OFF;
    TX_LED_OFF;

    // empty the rx buffer
    rfm22b_dev->rx_buffer_wr = 0;

    // Clear the TX buffer.
    rfm22b_dev->tx_data_rd   = rfm22b_dev->tx_data_wr = 0;

    // clear FIFOs
    rfm22_write(rfm22b_dev, RFM22_op_and_func_ctrl2, RFM22_opfc2_ffclrrx | RFM22_opfc2_ffclrtx);
    rfm22_write(rfm22b_dev, RFM22_op_and_func_ctrl2, 0x00);

    // enable RX interrupts
    rfm22_write(rfm22b_dev, RFM22_interrupt_enable1, RFM22_ie1_encrcerror | RFM22_ie1_enpkvalid |
                RFM22_ie1_enrxffafull | RFM22_ie1_enfferr);
    rfm22_write(rfm22b_dev, RFM22_interrupt_enable2, RFM22_ie2_enpreainval | RFM22_ie2_enpreaval |
                RFM22_ie2_enswdet);

    // enable the receiver
    rfm22_write(rfm22b_dev, RFM22_op_and_func_ctrl1, RFM22_opfc1_pllon | RFM22_opfc1_rxon);

    // Release the SPI bus.
    rfm22_releaseBus(rfm22b_dev);

    // Indicate that we're in RX wait mode.
    rfm22b_dev->rfm22b_state = RFM22B_STATE_RX_WAIT;

    return true;
}

/**
 * Transmit a packet via the RFM22B device.
 *
 * @param[in] rfm22b_id  The rfm22b device.
 * @param[in] p  The packet to transmit.
 * @return true if there if the packet was queued for transmission.
 */
bool PIOS_RFM22B_TransmitPacket(uint32_t rfm22b_id, uint8_t *p, uint8_t len)
{
    struct pios_rfm22b_dev *rfm22b_dev = (struct pios_rfm22b_dev *)rfm22b_id;

    if (!PIOS_RFM22B_Validate(rfm22b_dev)) {
        return false;
    }

    rfm22b_dev->tx_packet_handle     = p;
    rfm22b_dev->stats.tx_byte_count += len;
    rfm22b_dev->packet_start_ticks   = xTaskGetTickCount();
    if (rfm22b_dev->packet_start_ticks == 0) {
        rfm22b_dev->packet_start_ticks = 1;
    }

    // Claim the SPI bus.
    rfm22_claimBus(rfm22b_dev);

    // Disable interrupts
    rfm22_write(rfm22b_dev, RFM22_interrupt_enable1, 0x00);
    rfm22_write(rfm22b_dev, RFM22_interrupt_enable2, 0x00);

    // set the tx power
    rfm22b_dev->tx_power = 0x7;
    rfm22_write(rfm22b_dev, RFM22_tx_power, RFM22_tx_pwr_lna_sw | rfm22b_dev->tx_power);

    // TUNE mode
    rfm22_write(rfm22b_dev, RFM22_op_and_func_ctrl1, RFM22_opfc1_pllon);

    // Queue the data up for sending
    rfm22b_dev->tx_data_wr = len;

    RX_LED_OFF;

    // Set the destination address in the transmit header.
    uint32_t id = rfm22_destinationID(rfm22b_dev);
    rfm22_write(rfm22b_dev, RFM22_transmit_header0, id & 0xff);
    rfm22_write(rfm22b_dev, RFM22_transmit_header1, (id >> 8) & 0xff);
    rfm22_write(rfm22b_dev, RFM22_transmit_header2, (id >> 16) & 0xff);
    rfm22_write(rfm22b_dev, RFM22_transmit_header3, (id >> 24) & 0xff);

    // FIFO mode, GFSK modulation
    uint8_t fd_bit = rfm22_read(rfm22b_dev, RFM22_modulation_mode_control2) & RFM22_mmc2_fd;
    rfm22_write(rfm22b_dev, RFM22_modulation_mode_control2, fd_bit | RFM22_mmc2_dtmod_fifo | RFM22_mmc2_modtyp_gfsk);

    // Clear the FIFOs.
    rfm22_write(rfm22b_dev, RFM22_op_and_func_ctrl2, RFM22_opfc2_ffclrrx | RFM22_opfc2_ffclrtx);
    rfm22_write(rfm22b_dev, RFM22_op_and_func_ctrl2, 0x00);

    // Set the total number of data bytes we are going to transmit.
    rfm22_write(rfm22b_dev, RFM22_transmit_packet_length, len);

    // Add some data to the chips TX FIFO before enabling the transmitter
    uint8_t *tx_buffer = rfm22b_dev->tx_packet_handle;
    rfm22_assertCs(rfm22b_dev);
    PIOS_SPI_TransferByte(rfm22b_dev->spi_id, RFM22_fifo_access | 0x80);
    int bytes_to_write = (rfm22b_dev->tx_data_wr - rfm22b_dev->tx_data_rd);
    bytes_to_write = (bytes_to_write > FIFO_SIZE) ? FIFO_SIZE : bytes_to_write;
    PIOS_SPI_TransferBlock(rfm22b_dev->spi_id, &tx_buffer[rfm22b_dev->tx_data_rd], NULL, bytes_to_write, NULL);
    rfm22b_dev->tx_data_rd += bytes_to_write;
    rfm22_deassertCs(rfm22b_dev);

    // Enable TX interrupts.
    rfm22_write(rfm22b_dev, RFM22_interrupt_enable1, RFM22_ie1_enpksent | RFM22_ie1_entxffaem);

    // Enable the transmitter.
    rfm22_write(rfm22b_dev, RFM22_op_and_func_ctrl1, RFM22_opfc1_pllon | RFM22_opfc1_txon);

    // Release the SPI bus.
    rfm22_releaseBus(rfm22b_dev);

    // We're in Tx mode.
    rfm22b_dev->rfm22b_state = RFM22B_STATE_TX_MODE;

    TX_LED_ON;

#ifdef PIOS_RFM22B_DEBUG_ON_TELEM
    D1_LED_ON;
#endif

    return true;
}

/**
 * Process a Tx interrupt from the RFM22B device.
 *
 * @param[in] rfm22b_id  The rfm22b device.
 * @return PIOS_RFM22B_TX_COMPLETE on completed Tx, or PIOS_RFM22B_INT_SUCCESS/PIOS_RFM22B_INT_FAILURE.
 */
pios_rfm22b_int_result PIOS_RFM22B_ProcessTx(uint32_t rfm22b_id)
{
    struct pios_rfm22b_dev *rfm22b_dev = (struct pios_rfm22b_dev *)rfm22b_id;

    if (!PIOS_RFM22B_Validate(rfm22b_dev)) {
        return PIOS_RFM22B_INT_FAILURE;
    }

    // Read the device status registers
    if (!pios_rfm22_readStatus(rfm22b_dev)) {
        return PIOS_RFM22B_INT_FAILURE;
    }

    // TX FIFO almost empty, it needs filling up
    if (rfm22b_dev->status_regs.int_status_1.tx_fifo_almost_empty) {
        // Add data to the TX FIFO buffer
        uint8_t *tx_buffer = rfm22b_dev->tx_packet_handle;
        uint16_t max_bytes = FIFO_SIZE - TX_FIFO_LO_WATERMARK - 1;
        rfm22_claimBus(rfm22b_dev);
        rfm22_assertCs(rfm22b_dev);
        PIOS_SPI_TransferByte(rfm22b_dev->spi_id, RFM22_fifo_access | 0x80);
        int bytes_to_write = (rfm22b_dev->tx_data_wr - rfm22b_dev->tx_data_rd);
        bytes_to_write = (bytes_to_write > max_bytes) ? max_bytes : bytes_to_write;
        PIOS_SPI_TransferBlock(rfm22b_dev->spi_id, &tx_buffer[rfm22b_dev->tx_data_rd], NULL, bytes_to_write, NULL);
        rfm22b_dev->tx_data_rd += bytes_to_write;
        rfm22_deassertCs(rfm22b_dev);
        rfm22_releaseBus(rfm22b_dev);

        return PIOS_RFM22B_INT_SUCCESS;
    } else if (rfm22b_dev->status_regs.int_status_1.packet_sent_interrupt) {
        // Transition out of Tx mode.
        rfm22b_dev->rfm22b_state = RFM22B_STATE_TRANSITION;
        return PIOS_RFM22B_TX_COMPLETE;
    }

    return 0;
}

/**
 * Process a Rx interrupt from the RFM22B device.
 *
 * @param[in] rfm22b_id  The rfm22b device.
 * @return PIOS_RFM22B_RX_COMPLETE on completed Rx, or PIOS_RFM22B_INT_SUCCESS/PIOS_RFM22B_INT_FAILURE.
 */
pios_rfm22b_int_result PIOS_RFM22B_ProcessRx(uint32_t rfm22b_id)
{
    struct pios_rfm22b_dev *rfm22b_dev = (struct pios_rfm22b_dev *)rfm22b_id;

    if (!PIOS_RFM22B_Validate(rfm22b_dev)) {
        return PIOS_RFM22B_INT_FAILURE;
    }
    uint8_t *rx_buffer = rfm22b_dev->rx_packet_handle;
    pios_rfm22b_int_result ret = PIOS_RFM22B_INT_SUCCESS;

    // Read the device status registers
    if (!pios_rfm22_readStatus(rfm22b_dev)) {
        rfm22_rxFailure(rfm22b_dev);
        return PIOS_RFM22B_INT_FAILURE;
    }

    // FIFO under/over flow error.  Restart RX mode.
    if (rfm22b_dev->status_regs.int_status_1.fifo_underoverflow_error ||
        rfm22b_dev->status_regs.int_status_1.crc_error) {
        rfm22_rxFailure(rfm22b_dev);
        return PIOS_RFM22B_INT_FAILURE;
    }

    // Valid packet received
    if (rfm22b_dev->status_regs.int_status_1.valid_packet_received) {
        // Claim the SPI bus.
        rfm22_claimBus(rfm22b_dev);

        // read the total length of the packet data
        uint32_t len = rfm22_read(rfm22b_dev, RFM22_received_packet_length);

        // The received packet is going to be larger than the receive buffer
        if (len > rfm22b_dev->max_packet_len) {
            rfm22_releaseBus(rfm22b_dev);
            rfm22_rxFailure(rfm22b_dev);
            return PIOS_RFM22B_INT_FAILURE;
        }

        // there must still be data in the RX FIFO we need to get
        if (rfm22b_dev->rx_buffer_wr < len) {
            int32_t bytes_to_read = len - rfm22b_dev->rx_buffer_wr;
            // Fetch the data from the RX FIFO
            rfm22_assertCs(rfm22b_dev);
            PIOS_SPI_TransferByte(rfm22b_dev->spi_id, RFM22_fifo_access & 0x7F);
            rfm22b_dev->rx_buffer_wr += (PIOS_SPI_TransferBlock(rfm22b_dev->spi_id, OUT_FF, (uint8_t *)&rx_buffer[rfm22b_dev->rx_buffer_wr],
                                                                bytes_to_read, NULL) == 0) ? bytes_to_read : 0;
            rfm22_deassertCs(rfm22b_dev);
        }

        // Read the packet header (destination ID)
        rfm22b_dev->rx_destination_id  = rfm22_read(rfm22b_dev, RFM22_received_header0);
        rfm22b_dev->rx_destination_id |= (rfm22_read(rfm22b_dev, RFM22_received_header1) << 8);
        rfm22b_dev->rx_destination_id |= (rfm22_read(rfm22b_dev, RFM22_received_header2) << 16);
        rfm22b_dev->rx_destination_id |= (rfm22_read(rfm22b_dev, RFM22_received_header3) << 24);

        // Release the SPI bus.
        rfm22_releaseBus(rfm22b_dev);

        // Is there a length error?
        if (rfm22b_dev->rx_buffer_wr != len) {
            rfm22_rxFailure(rfm22b_dev);
            return PIOS_RFM22B_INT_FAILURE;
        }

        // Increment the total byte received count.
        rfm22b_dev->stats.rx_byte_count += rfm22b_dev->rx_buffer_wr;

        // Update the pair status with this packet.
        rfm22_updatePairStatus(rfm22b_dev);

        // We're finished with Rx mode
        rfm22b_dev->rfm22b_state = RFM22B_STATE_TRANSITION;

        ret = PIOS_RFM22B_RX_COMPLETE;
    } else if (rfm22b_dev->status_regs.int_status_1.rx_fifo_almost_full) {
        // RX FIFO almost full, it needs emptying
        // read data from the rf chips FIFO buffer

        // Claim the SPI bus.
        rfm22_claimBus(rfm22b_dev);

        // Read the total length of the packet data
        uint16_t len = rfm22_read(rfm22b_dev, RFM22_received_packet_length);

        // The received packet is going to be larger than the specified length
        if ((rfm22b_dev->rx_buffer_wr + RX_FIFO_HI_WATERMARK) > len) {
            rfm22_releaseBus(rfm22b_dev);
            rfm22_rxFailure(rfm22b_dev);
            return PIOS_RFM22B_INT_FAILURE;
        }

        // The received packet is going to be larger than the receive buffer
        if ((rfm22b_dev->rx_buffer_wr + RX_FIFO_HI_WATERMARK) > rfm22b_dev->max_packet_len) {
            rfm22_releaseBus(rfm22b_dev);
            rfm22_rxFailure(rfm22b_dev);
            return PIOS_RFM22B_INT_FAILURE;
        }

        // Fetch the data from the RX FIFO
        rfm22_assertCs(rfm22b_dev);
        PIOS_SPI_TransferByte(rfm22b_dev->spi_id, RFM22_fifo_access & 0x7F);
        rfm22b_dev->rx_buffer_wr += (PIOS_SPI_TransferBlock(rfm22b_dev->spi_id, OUT_FF, (uint8_t *)&rx_buffer[rfm22b_dev->rx_buffer_wr],
                                                            RX_FIFO_HI_WATERMARK, NULL) == 0) ? RX_FIFO_HI_WATERMARK : 0;
        rfm22_deassertCs(rfm22b_dev);

        // Release the SPI bus.
        rfm22_releaseBus(rfm22b_dev);

        // Make sure that we're in RX mode.
        rfm22b_dev->rfm22b_state = RFM22B_STATE_RX_MODE;
    } else if (rfm22b_dev->status_regs.int_status_2.valid_preamble_detected) {
        // Valid preamble detected
        RX_LED_ON;

        // Sync word detected
#ifdef PIOS_RFM22B_DEBUG_ON_TELEM
        D2_LED_ON;
#endif // PIOS_RFM22B_DEBUG_ON_TELEM
        rfm22b_dev->packet_start_ticks = xTaskGetTickCount();
        if (rfm22b_dev->packet_start_ticks == 0) {
            rfm22b_dev->packet_start_ticks = 1;
        }

        // We detected the preamble, now wait for sync.
        rfm22b_dev->rfm22b_state = RFM22B_STATE_RX_WAIT_SYNC;
    } else if (rfm22b_dev->status_regs.int_status_2.sync_word_detected) {
        // Claim the SPI bus.
        rfm22_claimBus(rfm22b_dev);

        // read the 10-bit signed afc correction value
        // bits 9 to 2
        uint16_t afc_correction = (uint16_t)rfm22_read(rfm22b_dev, RFM22_afc_correction_read) << 8;
        // bits 1 & 0
        afc_correction  |= (uint16_t)rfm22_read(rfm22b_dev, RFM22_ook_counter_value1) & 0x00c0;
        afc_correction >>= 6;
        // convert the afc value to Hz
        int32_t afc_corr = (int32_t)(rfm22b_dev->frequency_step_size * afc_correction + 0.5f);
        rfm22b_dev->afc_correction_Hz = (afc_corr < -127) ? -127 : ((afc_corr > 127) ? 127 : afc_corr);

        // read rx signal strength .. 45 = -100dBm, 205 = -20dBm
        uint8_t rssi = rfm22_read(rfm22b_dev, RFM22_rssi);
        // convert to dBm
        rfm22b_dev->rssi_dBm = (int8_t)(rssi >> 1) - 122;

        // Release the SPI bus.
        rfm22_releaseBus(rfm22b_dev);

        // Indicate that we're in RX mode.
        rfm22b_dev->rfm22b_state = RFM22B_STATE_RX_MODE;
    } else if ((rfm22b_dev->rfm22b_state == RFM22B_STATE_RX_WAIT_SYNC) && !rfm22b_dev->status_regs.int_status_2.valid_preamble_detected) {
        // Waiting for the preamble timed out.
        rfm22_rxFailure(rfm22b_dev);
        return PIOS_RFM22B_INT_FAILURE;
    }

    return ret;
}

/**
 * Validate that the device structure is valid.
 *
 * @param[in] rfm22b_dev  The RFM22B device structure pointer.
 */
inline bool PIOS_RFM22B_Validate(struct pios_rfm22b_dev *rfm22b_dev)
{
    return rfm22b_dev != NULL && rfm22b_dev->magic == PIOS_RFM22B_DEV_MAGIC;
}


/*****************************************************************************
* The Device Control Thread
*****************************************************************************/

/**
 * The task that controls the radio state machine.
 *
 * @param[in] paramters  The task parameters.
 */
static void pios_rfm22_task(void *parameters)
{
    struct pios_rfm22b_dev *rfm22b_dev = (struct pios_rfm22b_dev *)parameters;

    if (!PIOS_RFM22B_Validate(rfm22b_dev)) {
        return;
    }
    portTickType lastEventTicks = xTaskGetTickCount();

    while (1) {
#if defined(PIOS_INCLUDE_WDG) && defined(PIOS_WDG_RFM22B)
        // Update the watchdog timer
        PIOS_WDG_UpdateFlag(PIOS_WDG_RFM22B);
#endif /* PIOS_WDG_RFM22B */

        // Wait for a signal indicating an external interrupt or a pending send/receive request.
        if (xSemaphoreTake(rfm22b_dev->isrPending, ISR_TIMEOUT / portTICK_RATE_MS) == pdTRUE) {
            lastEventTicks = xTaskGetTickCount();

            // Process events through the state machine.
            enum pios_radio_event event;
            while (xQueueReceive(rfm22b_dev->eventQueue, &event, 0) == pdTRUE) {
                if ((event == RADIO_EVENT_INT_RECEIVED) &&
                    ((rfm22b_dev->state == RADIO_STATE_UNINITIALIZED) || (rfm22b_dev->state == RADIO_STATE_INITIALIZING))) {
                    continue;
                }
                rfm22_process_event(rfm22b_dev, event);
            }
        } else {
            // Has it been too long since the last event?
            portTickType curTicks = xTaskGetTickCount();
            if (pios_rfm22_time_difference_ms(lastEventTicks, curTicks) > PIOS_RFM22B_SUPERVISOR_TIMEOUT) {
                // Transsition through an error event.
                rfm22_process_event(rfm22b_dev, RADIO_EVENT_ERROR);

                // Clear the event queue.
                enum pios_radio_event event;
                while (xQueueReceive(rfm22b_dev->eventQueue, &event, 0) == pdTRUE) {
                    // Do nothing;
                }
                lastEventTicks = xTaskGetTickCount();
            }
        }

        // Change channels if necessary.
        rfm22_changeChannel(rfm22b_dev);

        portTickType curTicks = xTaskGetTickCount();
        // Have we been sending / receiving this packet too long?
        if ((rfm22b_dev->packet_start_ticks > 0) &&
            (pios_rfm22_time_difference_ms(rfm22b_dev->packet_start_ticks, curTicks) > (rfm22b_dev->packet_period * 3))) {
            rfm22_process_event(rfm22b_dev, RADIO_EVENT_TIMEOUT);
        }

        // Send a packet if it's our time slice
        bool time_to_send = rfm22_timeToSend(rfm22b_dev);
#ifdef PIOS_RFM22B_DEBUG_ON_TELEM
        if (time_to_send) {
            D4_LED_ON;
        } else {
            D4_LED_OFF;
        }
#endif

        // Start transmitting a packet if it's time.
        if (time_to_send && PIOS_RFM22B_InRxWait((uint32_t)rfm22b_dev)) {
            // If the on_sync_channel flag is set, it means that we were on the sync channel, but no packet was received, so transition to a non-connected state.
            if (rfm22b_dev->on_sync_channel) {
                rfm22b_dev->on_sync_channel = false;
                if (rfm22b_dev->stats.link_state == OPLINKSTATUS_LINKSTATE_CONNECTED) {
                    rfm22b_dev->stats.link_state = OPLINKSTATUS_LINKSTATE_CONNECTING;
                } else {
                    rfm22b_dev->stats.link_state = OPLINKSTATUS_LINKSTATE_DISCONNECTED;
                }
            }

            rfm22_process_event(rfm22b_dev, RADIO_EVENT_TX_START);
        }
    }
}


/*****************************************************************************
* The State Machine Functions
*****************************************************************************/

/**
 * Inject an event into the RFM22B state machine.
 *
 * @param[in] rfm22b_dev The device structure
 * @param[in] event The event to inject
 * @param[in] inISR Is this being called from an interrrup service routine?
 */
static void pios_rfm22_inject_event(struct pios_rfm22b_dev *rfm22b_dev, enum pios_radio_event event, bool inISR)
{
    if (inISR) {
        // Store the event.
        portBASE_TYPE pxHigherPriorityTaskWoken1;
        if (xQueueSendFromISR(rfm22b_dev->eventQueue, &event, &pxHigherPriorityTaskWoken1) != pdTRUE) {
            return;
        }
        // Signal the semaphore to wake up the handler thread.
        portBASE_TYPE pxHigherPriorityTaskWoken2;
        if (xSemaphoreGiveFromISR(rfm22b_dev->isrPending, &pxHigherPriorityTaskWoken2) != pdTRUE) {
            // Something went fairly seriously wrong
            rfm22b_dev->errors++;
        }
        portEND_SWITCHING_ISR((pxHigherPriorityTaskWoken2 == pdTRUE) || (pxHigherPriorityTaskWoken2 == pdTRUE));
    } else {
        // Store the event.
        if (xQueueSend(rfm22b_dev->eventQueue, &event, portMAX_DELAY) != pdTRUE) {
            return;
        }
        // Signal the semaphore to wake up the handler thread.
        if (xSemaphoreGive(rfm22b_dev->isrPending) != pdTRUE) {
            // Something went fairly seriously wrong
            rfm22b_dev->errors++;
        }
    }
}

/**
 * Process the next state transition from the given event.
 *
 * @param[in] rfm22b_dev The device structure
 * @param[in] event The event to process
 * @return enum pios_radio_event  The next event to inject
 */
static enum pios_radio_event rfm22_process_state_transition(struct pios_rfm22b_dev *rfm22b_dev, enum pios_radio_event event)
{
    // No event
    if (event >= RADIO_EVENT_NUM_EVENTS) {
        return RADIO_EVENT_NUM_EVENTS;
    }

    // Don't transition if there is no transition defined
    enum pios_radio_state next_state = rfm22b_transitions[rfm22b_dev->state].next_state[event];
    if (!next_state) {
        return RADIO_EVENT_NUM_EVENTS;
    }

    /*
     * Move to the next state
     *
     * This is done prior to calling the new state's entry function to
     * guarantee that the entry function never depends on the previous
     * state.  This way, it cannot ever know what the previous state was.
     */
    rfm22b_dev->state = next_state;

    /* Call the entry function (if any) for the next state. */
    if (rfm22b_transitions[rfm22b_dev->state].entry_fn) {
        return rfm22b_transitions[rfm22b_dev->state].entry_fn(rfm22b_dev);
    }

    return RADIO_EVENT_NUM_EVENTS;
}

/**
 * Process the given event through the state transition table.
 * This could cause a series of events and transitions to take place.
 *
 * @param[in] rfm22b_dev The device structure
 * @param[in] event The event to process
 */
static void rfm22_process_event(struct pios_rfm22b_dev *rfm22b_dev, enum pios_radio_event event)
{
    // Process all state transitions.
    while (event != RADIO_EVENT_NUM_EVENTS) {
        event = rfm22_process_state_transition(rfm22b_dev, event);
    }
}


/*****************************************************************************
* The Device Initialization / Configuration Functions
*****************************************************************************/

/**
 * Initialize (or re-initialize) the RFM22B radio device.
 *
 * @param[in] rfm22b_dev The device structure
 * @return enum pios_radio_event  The next event to inject
 */
static enum pios_radio_event rfm22_init(struct pios_rfm22b_dev *rfm22b_dev)
{
    // Initialize the register values.
    rfm22b_dev->status_regs.int_status_1.raw  = 0;
    rfm22b_dev->status_regs.int_status_2.raw  = 0;
    rfm22b_dev->status_regs.device_status.raw = 0;
    rfm22b_dev->status_regs.ezmac_status.raw  = 0;

    // Clean the LEDs
    rfm22_clearLEDs();

    // Initialize the detected device statistics.
    for (uint8_t i = 0; i < OPLINKSTATUS_PAIRIDS_NUMELEM; ++i) {
        rfm22b_dev->pair_stats[i].pairID = 0;
        rfm22b_dev->pair_stats[i].rssi   = -127;
        rfm22b_dev->pair_stats[i].afc_correction = 0;
        rfm22b_dev->pair_stats[i].lastContact = 0;
    }

    // Initlize the link stats.
    for (uint8_t i = 0; i < RFM22B_RX_PACKET_STATS_LEN; ++i) {
        rfm22b_dev->rx_packet_stats[i] = 0;
    }

    // Initialize the state
    rfm22b_dev->stats.link_state  = OPLINKSTATUS_LINKSTATE_DISCONNECTED;

    // Initialize the packets.
    rfm22b_dev->rx_packet_len     = 0;
    rfm22b_dev->rx_destination_id = 0;
    rfm22b_dev->tx_packet_handle  = NULL;

    // Initialize the devide state
    rfm22b_dev->rx_buffer_wr       = 0;
    rfm22b_dev->tx_data_rd         = rfm22b_dev->tx_data_wr = 0;
    rfm22b_dev->channel = 0;
    rfm22b_dev->channel_index      = 0;
    rfm22b_dev->afc_correction_Hz  = 0;
    rfm22b_dev->packet_start_ticks = 0;
    rfm22b_dev->tx_complete_ticks  = 0;
    rfm22b_dev->rfm22b_state       = RFM22B_STATE_INITIALIZING;
    rfm22b_dev->on_sync_channel    = false;

    // software reset the RF chip .. following procedure according to Si4x3x Errata (rev. B)
    rfm22_write_claim(rfm22b_dev, RFM22_op_and_func_ctrl1, RFM22_opfc1_swres);

    for (uint8_t i = 0; i < 50; ++i) {
        // read the status registers
        pios_rfm22_readStatus(rfm22b_dev);

        // Is the chip ready?
        if (rfm22b_dev->status_regs.int_status_2.chip_ready) {
            break;
        }

        // Wait 1ms if not.
        PIOS_DELAY_WaitmS(1);
    }

    // ****************

    // read status - clears interrupt
    pios_rfm22_readStatus(rfm22b_dev);

    // Claim the SPI bus.
    rfm22_claimBus(rfm22b_dev);

    // disable all interrupts
    rfm22_write(rfm22b_dev, RFM22_interrupt_enable1, 0x00);
    rfm22_write(rfm22b_dev, RFM22_interrupt_enable2, 0x00);

    // read the RF chip ID bytes

    // read the device type
    uint8_t device_type    = rfm22_read(rfm22b_dev, RFM22_DEVICE_TYPE) & RFM22_DT_MASK;
    // read the device version
    uint8_t device_version = rfm22_read(rfm22b_dev, RFM22_DEVICE_VERSION) & RFM22_DV_MASK;

#if defined(RFM22_DEBUG)
    DEBUG_PRINTF(2, "rf device type: %d\n\r", device_type);
    DEBUG_PRINTF(2, "rf device version: %d\n\r", device_version);
#endif

    if (device_type != 0x08) {
#if defined(RFM22_DEBUG)
        DEBUG_PRINTF(2, "rf device type: INCORRECT - should be 0x08\n\r");
#endif

        // incorrect RF module type
        return RADIO_EVENT_FATAL_ERROR;
    }
    if (device_version != RFM22_DEVICE_VERSION_B1) {
#if defined(RFM22_DEBUG)
        DEBUG_PRINTF(2, "rf device version: INCORRECT\n\r");
#endif
        // incorrect RF module version
        return RADIO_EVENT_FATAL_ERROR;
    }

    // calibrate our RF module to be exactly on frequency .. different for every module
    rfm22_write(rfm22b_dev, RFM22_xtal_osc_load_cap, OSC_LOAD_CAP);

    // disable Low Duty Cycle Mode
    rfm22_write(rfm22b_dev, RFM22_op_and_func_ctrl2, 0x00);

    // 1MHz clock output
    rfm22_write(rfm22b_dev, RFM22_cpu_output_clk, RFM22_coc_1MHz);

    // READY mode
    rfm22_write(rfm22b_dev, RFM22_op_and_func_ctrl1, RFM22_opfc1_xton);

    // choose the 3 GPIO pin functions
    // GPIO port use default value
    rfm22_write(rfm22b_dev, RFM22_io_port_config, RFM22_io_port_default);
    if (rfm22b_dev->cfg.gpio_direction == GPIO0_TX_GPIO1_RX) {
        // GPIO0 = TX State (to control RF Switch)
        rfm22_write(rfm22b_dev, RFM22_gpio0_config, RFM22_gpio0_config_drv3 | RFM22_gpio0_config_txstate);
        // GPIO1 = RX State (to control RF Switch)
        rfm22_write(rfm22b_dev, RFM22_gpio1_config, RFM22_gpio1_config_drv3 | RFM22_gpio1_config_rxstate);
    } else {
        // GPIO0 = TX State (to control RF Switch)
        rfm22_write(rfm22b_dev, RFM22_gpio0_config, RFM22_gpio0_config_drv3 | RFM22_gpio0_config_rxstate);
        // GPIO1 = RX State (to control RF Switch)
        rfm22_write(rfm22b_dev, RFM22_gpio1_config, RFM22_gpio1_config_drv3 | RFM22_gpio1_config_txstate);
    }
    // GPIO2 = Clear Channel Assessment
    rfm22_write(rfm22b_dev, RFM22_gpio2_config, RFM22_gpio2_config_drv3 | RFM22_gpio2_config_cca);

    // FIFO mode, GFSK modulation
    uint8_t fd_bit = rfm22_read(rfm22b_dev, RFM22_modulation_mode_control2) & RFM22_mmc2_fd;
    rfm22_write(rfm22b_dev, RFM22_modulation_mode_control2, RFM22_mmc2_trclk_clk_none | RFM22_mmc2_dtmod_fifo | fd_bit | RFM22_mmc2_modtyp_gfsk);

    // setup to read the internal temperature sensor

    // ADC used to sample the temperature sensor
    uint8_t adc_config = RFM22_ac_adcsel_temp_sensor | RFM22_ac_adcref_bg;
    rfm22_write(rfm22b_dev, RFM22_adc_config, adc_config);

    // adc offset
    rfm22_write(rfm22b_dev, RFM22_adc_sensor_amp_offset, 0);

    // temp sensor calibration .. �40C to +64C 0.5C resolution
    rfm22_write(rfm22b_dev, RFM22_temp_sensor_calib, RFM22_tsc_tsrange0 | RFM22_tsc_entsoffs);

    // temp sensor offset
    rfm22_write(rfm22b_dev, RFM22_temp_value_offset, 0);

    // start an ADC conversion
    rfm22_write(rfm22b_dev, RFM22_adc_config, adc_config | RFM22_ac_adcstartbusy);

    // set the RSSI threshold interrupt to about -90dBm
    rfm22_write(rfm22b_dev, RFM22_rssi_threshold_clear_chan_indicator, (-90 + 122) * 2);

    // enable the internal Tx & Rx packet handlers (without CRC)
    rfm22_write(rfm22b_dev, RFM22_data_access_control, RFM22_dac_enpacrx | RFM22_dac_enpactx);

    // x-nibbles tx preamble
    rfm22_write(rfm22b_dev, RFM22_preamble_length, TX_PREAMBLE_NIBBLES);
    // x-nibbles rx preamble detection
    rfm22_write(rfm22b_dev, RFM22_preamble_detection_ctrl1, RX_PREAMBLE_NIBBLES << 3);

    // header control - using a 4 by header with broadcast of 0xffffffff
    rfm22_write(rfm22b_dev, RFM22_header_control1,
                RFM22_header_cntl1_bcen_0 |
                RFM22_header_cntl1_bcen_1 |
                RFM22_header_cntl1_bcen_2 |
                RFM22_header_cntl1_bcen_3 |
                RFM22_header_cntl1_hdch_0 |
                RFM22_header_cntl1_hdch_1 |
                RFM22_header_cntl1_hdch_2 |
                RFM22_header_cntl1_hdch_3);
    // Check all bit of all bytes of the header, unless we're an unbound modem.
    uint8_t header_mask = (rfm22_destinationID(rfm22b_dev) == 0xffffffff) ? 0 : 0xff;
    rfm22_write(rfm22b_dev, RFM22_header_enable0, header_mask);
    rfm22_write(rfm22b_dev, RFM22_header_enable1, header_mask);
    rfm22_write(rfm22b_dev, RFM22_header_enable2, header_mask);
    rfm22_write(rfm22b_dev, RFM22_header_enable3, header_mask);
    // The destination ID and receive ID should be the same.
    uint32_t id = rfm22_destinationID(rfm22b_dev);
    rfm22_write(rfm22b_dev, RFM22_check_header0, id & 0xff);
    rfm22_write(rfm22b_dev, RFM22_check_header1, (id >> 8) & 0xff);
    rfm22_write(rfm22b_dev, RFM22_check_header2, (id >> 16) & 0xff);
    rfm22_write(rfm22b_dev, RFM22_check_header3, (id >> 24) & 0xff);
    // 4 header bytes, synchronization word length 3, 2, 1 & 0 used, packet length included in header.
    rfm22_write(rfm22b_dev, RFM22_header_control2,
                RFM22_header_cntl2_hdlen_3210 |
                RFM22_header_cntl2_synclen_3210 |
                ((TX_PREAMBLE_NIBBLES >> 8) & 0x01));

    // sync word
    rfm22_write(rfm22b_dev, RFM22_sync_word3, SYNC_BYTE_1);
    rfm22_write(rfm22b_dev, RFM22_sync_word2, SYNC_BYTE_2);
    rfm22_write(rfm22b_dev, RFM22_sync_word1, SYNC_BYTE_3);
    rfm22_write(rfm22b_dev, RFM22_sync_word0, SYNC_BYTE_4);

    // TX FIFO Almost Full Threshold (0 - 63)
    rfm22_write(rfm22b_dev, RFM22_tx_fifo_control1, TX_FIFO_HI_WATERMARK);

    // TX FIFO Almost Empty Threshold (0 - 63)
    rfm22_write(rfm22b_dev, RFM22_tx_fifo_control2, TX_FIFO_LO_WATERMARK);

    // RX FIFO Almost Full Threshold (0 - 63)
    rfm22_write(rfm22b_dev, RFM22_rx_fifo_control, RX_FIFO_HI_WATERMARK);

    // Set the frequency calibration
    rfm22_write(rfm22b_dev, RFM22_xtal_osc_load_cap, rfm22b_dev->cfg.RFXtalCap);

    // Release the bus
    rfm22_releaseBus(rfm22b_dev);

    // Initialize the frequency and datarate to te default.
    rfm22_setNominalCarrierFrequency(rfm22b_dev, 0);
    pios_rfm22_setDatarate(rfm22b_dev);

    return RADIO_EVENT_INITIALIZED;
}

/**
 * Set the air datarate for the RFM22B device.
 *
 * Carson's rule:
 *  The signal bandwidth is about 2(Delta-f + fm) ..
 *
 * Delta-f = frequency deviation
 * fm = maximum frequency of the signal
 *
 * @param[in] rfm33b_dev  The device structure pointer.
 * @param[in] datarate  The air datarate.
 * @param[in] data_whitening  Is data whitening desired?
 */
static void pios_rfm22_setDatarate(struct pios_rfm22b_dev *rfm22b_dev)
{
    enum rfm22b_datarate datarate = rfm22b_dev->datarate;
    bool data_whitening    = true;
    uint32_t datarate_bps  = data_rate[datarate];

    // Calculate the maximum packet length from the datarate.
    float bytes_per_period = (float)datarate_bps * (float)(rfm22b_dev->packet_period) / 9000;

    rfm22b_dev->max_packet_len = bytes_per_period - TX_PREAMBLE_NIBBLES / 2 - SYNC_BYTES - HEADER_BYTES - 5;
    if (rfm22b_dev->max_packet_len > RFM22B_MAX_PACKET_LEN) {
        rfm22b_dev->max_packet_len = RFM22B_MAX_PACKET_LEN;
    }

    // Claim the SPI bus.
    rfm22_claimBus(rfm22b_dev);

    // rfm22_if_filter_bandwidth
    rfm22_write(rfm22b_dev, 0x1C, reg_1C[datarate]);

    // rfm22_afc_loop_gearshift_override
    rfm22_write(rfm22b_dev, 0x1D, reg_1D[datarate]);
    // RFM22_afc_timing_control
    rfm22_write(rfm22b_dev, 0x1E, reg_1E[datarate]);

    // RFM22_clk_recovery_gearshift_override
    rfm22_write(rfm22b_dev, 0x1F, reg_1F[datarate]);
    // rfm22_clk_recovery_oversampling_ratio
    rfm22_write(rfm22b_dev, 0x20, reg_20[datarate]);
    // rfm22_clk_recovery_offset2
    rfm22_write(rfm22b_dev, 0x21, reg_21[datarate]);
    // rfm22_clk_recovery_offset1
    rfm22_write(rfm22b_dev, 0x22, reg_22[datarate]);
    // rfm22_clk_recovery_offset0
    rfm22_write(rfm22b_dev, 0x23, reg_23[datarate]);
    // rfm22_clk_recovery_timing_loop_gain1
    rfm22_write(rfm22b_dev, 0x24, reg_24[datarate]);
    // rfm22_clk_recovery_timing_loop_gain0
    rfm22_write(rfm22b_dev, 0x25, reg_25[datarate]);
    // rfm22_agc_override1
    rfm22_write(rfm22b_dev, RFM22_agc_override1, reg_69[datarate]);

    // rfm22_afc_limiter
    rfm22_write(rfm22b_dev, 0x2A, reg_2A[datarate]);

    // rfm22_tx_data_rate1
    rfm22_write(rfm22b_dev, 0x6E, reg_6E[datarate]);
    // rfm22_tx_data_rate0
    rfm22_write(rfm22b_dev, 0x6F, reg_6F[datarate]);

    if (!data_whitening) {
        // rfm22_modulation_mode_control1
        rfm22_write(rfm22b_dev, 0x70, reg_70[datarate] & ~RFM22_mmc1_enwhite);
    } else {
        // rfm22_modulation_mode_control1
        rfm22_write(rfm22b_dev, 0x70, reg_70[datarate] | RFM22_mmc1_enwhite);
    }

    // rfm22_modulation_mode_control2
    rfm22_write(rfm22b_dev, 0x71, reg_71[datarate]);

    // rfm22_frequency_deviation
    rfm22_write(rfm22b_dev, 0x72, reg_72[datarate]);

    // rfm22_cpcuu
    rfm22_write(rfm22b_dev, 0x58, reg_58[datarate]);

    rfm22_write(rfm22b_dev, RFM22_ook_counter_value1, 0x00);
    rfm22_write(rfm22b_dev, RFM22_ook_counter_value2, 0x00);

    // Release the bus
    rfm22_releaseBus(rfm22b_dev);
}

/**
 * Set the nominal carrier frequency, channel step size, and initial channel
 *
 * @param[in] rfm33b_dev  The device structure pointer.
 * @param[in] init_chan  The initial channel to tune to.
 */
static void rfm22_setNominalCarrierFrequency(struct pios_rfm22b_dev *rfm22b_dev, uint8_t init_chan)
{
    // Set the frequency channels to start at 430MHz
    uint32_t frequency_hz = RFM22B_NOMINAL_CARRIER_FREQUENCY;
    // The step size is 10MHz / 250 channels = 40khz, and the step size is specified in 10khz increments.
    uint8_t freq_hop_step_size = 4;

    // holds the hbsel (1 or 2)
    uint8_t hbsel;

    if (frequency_hz < 480000000) {
        hbsel = 0;
    } else {
        hbsel = 1;
    }
    float freq_mhz = (float)(frequency_hz) / 1000000.0f;
    float xtal_freq_khz = 30000.0f;
    float sfreq    = freq_mhz / (10.0f * (xtal_freq_khz / 30000.0f) * (1 + hbsel));
    uint32_t fb    = (uint32_t)sfreq - 24 + (64 + 32 * hbsel);
    uint32_t fc    = (uint32_t)((sfreq - (uint32_t)sfreq) * 64000.0f);
    uint8_t fch    = (fc >> 8) & 0xff;
    uint8_t fcl    = fc & 0xff;

    // Claim the SPI bus.
    rfm22_claimBus(rfm22b_dev);

    // Setthe frequency hopping step size.
    rfm22_write(rfm22b_dev, RFM22_frequency_hopping_step_size, freq_hop_step_size);

    // frequency hopping channel (0-255)
    rfm22b_dev->frequency_step_size = 156.25f * hbsel;

    // frequency hopping channel (0-255)
    rfm22b_dev->channel = init_chan;
    rfm22_write(rfm22b_dev, RFM22_frequency_hopping_channel_select, init_chan);

    // no frequency offset
    rfm22_write(rfm22b_dev, RFM22_frequency_offset1, 0);
    rfm22_write(rfm22b_dev, RFM22_frequency_offset2, 0);

    // set the carrier frequency
    rfm22_write(rfm22b_dev, RFM22_frequency_band_select, fb & 0xff);
    rfm22_write(rfm22b_dev, RFM22_nominal_carrier_frequency1, fch);
    rfm22_write(rfm22b_dev, RFM22_nominal_carrier_frequency0, fcl);

    // Release the bus
    rfm22_releaseBus(rfm22b_dev);
}


/**
 * Set the frequency hopping channel.
 *
 * @param[in] rfm33b_dev  The device structure pointer.
 */
static bool rfm22_setFreqHopChannel(struct pios_rfm22b_dev *rfm22b_dev, uint8_t channel)
{
    // set the frequency hopping channel
    if (rfm22b_dev->channel == channel) {
        return false;
    }
#ifdef PIOS_RFM22B_DEBUG_ON_TELEM
    D3_LED_TOGGLE;
#endif // PIOS_RFM22B_DEBUG_ON_TELEM
    rfm22b_dev->channel = channel;
    rfm22_write(rfm22b_dev, RFM22_frequency_hopping_channel_select, channel);
    return true;
}

/**
 * Read the RFM22B interrupt and device status registers
 *
 * @param[in] rfm22b_dev  The device structure
 */
static bool pios_rfm22_readStatus(struct pios_rfm22b_dev *rfm22b_dev)
{
    // 1. Read the interrupt statuses with burst read
    rfm22_claimBus(rfm22b_dev); // Set RC and the semaphore
    uint8_t write_buf[3] = { RFM22_interrupt_status1 &0x7f, 0xFF, 0xFF };
    uint8_t read_buf[3];
    rfm22_assertCs(rfm22b_dev);
    PIOS_SPI_TransferBlock(rfm22b_dev->spi_id, write_buf, read_buf, sizeof(write_buf), NULL);
    rfm22_deassertCs(rfm22b_dev);
    rfm22b_dev->status_regs.int_status_1.raw  = read_buf[1];
    rfm22b_dev->status_regs.int_status_2.raw  = read_buf[2];

    // Device status
    rfm22b_dev->status_regs.device_status.raw = rfm22_read(rfm22b_dev, RFM22_device_status);

    // EzMAC status
    rfm22b_dev->status_regs.ezmac_status.raw  = rfm22_read(rfm22b_dev, RFM22_ezmac_status);

    // Release the bus
    rfm22_releaseBus(rfm22b_dev);

    // the RF module has gone and done a reset - we need to re-initialize the rf module
    if (rfm22b_dev->status_regs.int_status_2.poweron_reset) {
        return false;
    }

    return true;
}

/**
 * Recover from a failure in receiving a packet.
 *
 * @param[in] rfm22b_dev  The device structure
 * @return enum pios_radio_event  The next event to inject
 */
static void rfm22_rxFailure(struct pios_rfm22b_dev *rfm22b_dev)
{
    rfm22b_dev->stats.rx_failure++;
    rfm22b_dev->rx_buffer_wr = 0;
    rfm22b_dev->packet_start_ticks = 0;
    rfm22b_dev->rfm22b_state = RFM22B_STATE_TRANSITION;
}


/*****************************************************************************
* Radio Transmit and Receive functions.
*****************************************************************************/

/**
 * Start a transmit if possible
 *
 * @param[in] radio_dev The device structure
 * @return enum pios_radio_event  The next event to inject
 */
static enum pios_radio_event radio_txStart(struct pios_rfm22b_dev *radio_dev)
{
    uint8_t *p  = radio_dev->rx_packet;
    uint8_t len = 0;
    uint8_t max_data_len = radio_dev->max_packet_len - RS_ECC_NPARITY;

    // Don't send if it's not our turn, or if we're receiving a packet.
    if (!rfm22_timeToSend(radio_dev) || !PIOS_RFM22B_InRxWait((uint32_t)radio_dev)) {
        return RADIO_EVENT_RX_MODE;
    }

    // Don't send anything if we're bound to a coordinator and not yet connected.
    if (!rfm22_isCoordinator(radio_dev) && !rfm22_isConnected(radio_dev)) {
        return RADIO_EVENT_RX_MODE;
    }

    // Try to get some data to send
    if (radio_dev->tx_out_cb) {
        bool need_yield = false;
        len = (radio_dev->tx_out_cb)(radio_dev->tx_out_context, p, max_data_len, NULL, &need_yield);
    }

    // Always send a packet on the sync channel if this modem is a coordinator.
    if ((len == 0) && ((radio_dev->channel_index != 0) || !rfm22_isCoordinator(radio_dev))) {
        return RADIO_EVENT_RX_MODE;
    }

    // Increment the packet sequence number.
    radio_dev->stats.tx_seq++;

    // Change the channel.
    rfm22_changeChannel(radio_dev);

    // Add the error correcting code.
    if (len != 0) {
        encode_data((unsigned char *)p, len, (unsigned char *)p);
    }
    len += RS_ECC_NPARITY;

    // Transmit the packet.
    PIOS_RFM22B_TransmitPacket((uint32_t)radio_dev, p, len);

    return RADIO_EVENT_NUM_EVENTS;
}

/**
 * Receive packet data.
 *
 * @param[in] rfm22b_dev The device structure
 * @return enum pios_radio_event  The next event to inject
 */
static enum pios_radio_event radio_txData(struct pios_rfm22b_dev *radio_dev)
{
    enum pios_radio_event ret_event = RADIO_EVENT_NUM_EVENTS;
    pios_rfm22b_int_result res = PIOS_RFM22B_ProcessTx((uint32_t)radio_dev);

    // Is the transmition complete
    if (res == PIOS_RFM22B_TX_COMPLETE) {
        radio_dev->tx_complete_ticks = xTaskGetTickCount();

        // Is this an ACK?
        ret_event = RADIO_EVENT_RX_MODE;
        radio_dev->tx_packet_handle   = 0;
        radio_dev->tx_data_wr = radio_dev->tx_data_rd = 0;
        // Start a new transaction
        radio_dev->packet_start_ticks = 0;

#ifdef PIOS_RFM22B_DEBUG_ON_TELEM
        D1_LED_OFF;
#endif
    }

    return ret_event;
}

/**
 * Switch the radio into receive mode.
 *
 * @param[in] rfm22b_dev The device structure
 * @return enum pios_radio_event  The next event to inject
 */
static enum pios_radio_event radio_setRxMode(struct pios_rfm22b_dev *rfm22b_dev)
{
    if (!PIOS_RFM22B_ReceivePacket((uint32_t)rfm22b_dev, rfm22b_dev->rx_packet)) {
        return RADIO_EVENT_NUM_EVENTS;
    }
    rfm22b_dev->packet_start_ticks = 0;

    // No event generated
    return RADIO_EVENT_NUM_EVENTS;
}

/**
 * Complete the receipt of a packet.
 *
 * @param[in] radio_dev  The device structure
 * @param[in] p  The packet handle of the received packet.
 * @param[in] rc_len  The number of bytes received.
 * @return enum pios_radio_event  The next event to inject
 */
static enum pios_radio_event radio_receivePacket(struct pios_rfm22b_dev *radio_dev, uint8_t *p, uint16_t rx_len)
{
    uint8_t data_len = rx_len - RS_ECC_NPARITY;
    // Attempt to correct any errors in the packet.
    bool good_packet = true;
    bool corrected_packet = false;

    if (data_len > 0) {
        decode_data((unsigned char *)p, rx_len);

        good_packet = check_syndrome() == 0;
        // We have an error.  Try to correct it.
        if (!good_packet && (correct_errors_erasures((unsigned char *)p, rx_len, 0, 0) != 0)) {
            // We corrected it
            corrected_packet = true;
        }
    }

    // Set the packet status
    if (good_packet) {
        rfm22b_add_rx_status(radio_dev, RADIO_GOOD_RX_PACKET);
    } else if (corrected_packet) {
        // We corrected the error.
        rfm22b_add_rx_status(radio_dev, RADIO_CORRECTED_RX_PACKET);
    } else {
        // We couldn't correct the error, so drop the packet.
        rfm22b_add_rx_status(radio_dev, RADIO_ERROR_RX_PACKET);
    }

    enum pios_radio_event ret_event = RADIO_EVENT_RX_COMPLETE;
    if (good_packet || corrected_packet) {
        // Send the data to the com port
        bool rx_need_yield;
        if (radio_dev->rx_in_cb) {
            (radio_dev->rx_in_cb)(radio_dev->rx_in_context, p, data_len, NULL, &rx_need_yield);
        }

        // We only synchronize the clock on packets from our coordinator on the sync channel.
        if (!rfm22_isCoordinator(radio_dev) && (radio_dev->rx_destination_id == rfm22_destinationID(radio_dev)) && (radio_dev->channel_index == 0)) {
            rfm22_synchronizeClock(radio_dev);
            radio_dev->stats.link_state = OPLINKSTATUS_LINKSTATE_CONNECTED;
            radio_dev->on_sync_channel  = false;
        }
    } else {
        ret_event = RADIO_EVENT_RX_COMPLETE;
    }

    return ret_event;
}

/**
 * Receive the packet data.
 *
 * @param[in] rfm22b_dev The device structure
 * @return enum pios_radio_event  The next event to inject
 */
static enum pios_radio_event radio_rxData(struct pios_rfm22b_dev *radio_dev)
{
    enum pios_radio_event ret_event = RADIO_EVENT_NUM_EVENTS;
    pios_rfm22b_int_result res = PIOS_RFM22B_ProcessRx((uint32_t)radio_dev);

    switch (res) {
    case PIOS_RFM22B_RX_COMPLETE:

        // Receive the packet.
        ret_event = radio_receivePacket(radio_dev, radio_dev->rx_packet_handle, radio_dev->rx_buffer_wr);
        radio_dev->rx_buffer_wr = 0;
#ifdef PIOS_RFM22B_DEBUG_ON_TELEM
        D2_LED_OFF;
#endif

        // Start a new transaction
        radio_dev->packet_start_ticks = 0;
        break;

    case PIOS_RFM22B_INT_FAILURE:

        ret_event = RADIO_EVENT_RX_MODE;
        break;

    default:
        // do nothing.
        break;
    }

    return ret_event;
}

/*****************************************************************************
* Link Statistics Functions
*****************************************************************************/

/**
 * Update the modem pair status.
 *
 * @param[in] rfm22b_dev  The device structure
 */
static void rfm22_updatePairStatus(struct pios_rfm22b_dev *radio_dev)
{
    int8_t rssi    = radio_dev->rssi_dBm;
    int8_t afc     = radio_dev->afc_correction_Hz;
    uint32_t id    = radio_dev->rx_destination_id;

    // Have we seen this device recently?
    bool found     = false;
    uint8_t id_idx = 0;

    for (; id_idx < OPLINKSTATUS_PAIRIDS_NUMELEM; ++id_idx) {
        if (radio_dev->pair_stats[id_idx].pairID == id) {
            found = true;
            break;
        }
    }

    // If we have seen it, update the RSSI and reset the last contact counter
    if (found) {
        radio_dev->pair_stats[id_idx].rssi = rssi;
        radio_dev->pair_stats[id_idx].afc_correction = afc;
        radio_dev->pair_stats[id_idx].lastContact = 0;
    } else {
        // If we haven't seen it, find a slot to put it in.
        uint8_t min_idx = 0;
        int8_t min_rssi = radio_dev->pair_stats[0].rssi;
        for (id_idx = 1; id_idx < OPLINKSTATUS_PAIRIDS_NUMELEM; ++id_idx) {
            if (radio_dev->pair_stats[id_idx].rssi < min_rssi) {
                min_rssi = radio_dev->pair_stats[id_idx].rssi;
                min_idx  = id_idx;
            }
        }
        radio_dev->pair_stats[min_idx].pairID = id;
        radio_dev->pair_stats[min_idx].rssi   = rssi;
        radio_dev->pair_stats[min_idx].afc_correction = afc;
        radio_dev->pair_stats[min_idx].lastContact = 0;
    }
}

/**
 * Calculate the link quality from the packet receipt, tranmittion statistics.
 *
 * @param[in] rfm22b_dev  The device structure
 */
static void rfm22_calculateLinkQuality(struct pios_rfm22b_dev *rfm22b_dev)
{
    // Add the RX packet statistics
    rfm22b_dev->stats.rx_good      = 0;
    rfm22b_dev->stats.rx_corrected = 0;
    rfm22b_dev->stats.rx_error     = 0;
    rfm22b_dev->stats.tx_resent    = 0;
    for (uint8_t i = 0; i < RFM22B_RX_PACKET_STATS_LEN; ++i) {
        uint32_t val = rfm22b_dev->rx_packet_stats[i];
        for (uint8_t j = 0; j < 16; ++j) {
            switch ((val >> (j * 2)) & 0x3) {
            case RADIO_GOOD_RX_PACKET:
                rfm22b_dev->stats.rx_good++;
                break;
            case RADIO_CORRECTED_RX_PACKET:
                rfm22b_dev->stats.rx_corrected++;
                break;
            case RADIO_ERROR_RX_PACKET:
                rfm22b_dev->stats.rx_error++;
                break;
            case RADIO_RESENT_TX_PACKET:
                rfm22b_dev->stats.tx_resent++;
                break;
            }
        }
    }

    // Calculate the link quality metric, which is related to the number of good packets in relation to the number of bad packets.
    // Note: This assumes that the number of packets sampled for the stats is 64.
    // Using this equation, error and resent packets are counted as -2, and corrected packets are counted as -1.
    // The range is 0 (all error or resent packets) to 128 (all good packets).
    rfm22b_dev->stats.link_quality = 64 + rfm22b_dev->stats.rx_good - rfm22b_dev->stats.rx_error - rfm22b_dev->stats.tx_resent;
}

/**
 * Add a status value to the RX packet status array.
 *
 * @param[in] rfm22b_dev  The device structure
 * @param[in] status  The packet status value
 */
static void rfm22b_add_rx_status(struct pios_rfm22b_dev *rfm22b_dev, enum pios_rfm22b_rx_packet_status status)
{
    // Shift the status registers
    for (uint8_t i = RFM22B_RX_PACKET_STATS_LEN - 1; i > 0; --i) {
        rfm22b_dev->rx_packet_stats[i] = (rfm22b_dev->rx_packet_stats[i] << 2) | (rfm22b_dev->rx_packet_stats[i - 1] >> 30);
    }
    rfm22b_dev->rx_packet_stats[0] = (rfm22b_dev->rx_packet_stats[0] << 2) | status;
}


/*****************************************************************************
* Connection Handling Functions
*****************************************************************************/

/**
 * Are we a coordinator modem?
 *
 * @param[in] rfm22b_dev  The device structure
 */
static bool rfm22_isCoordinator(struct pios_rfm22b_dev *rfm22b_dev)
{
    return rfm22b_dev->coordinator;
}

/**
 * Returns the destination ID to send packets to.
 *
 * @param[in] rfm22b_id The RFM22B device index.
 * @return The destination ID
 */
uint32_t rfm22_destinationID(struct pios_rfm22b_dev *rfm22b_dev)
{
    if (rfm22_isCoordinator(rfm22b_dev)) {
        return rfm22b_dev->deviceID;
    } else if (rfm22b_dev->coordinatorID) {
        return rfm22b_dev->coordinatorID;
    } else {
        return 0xffffffff;
    }
}


/*****************************************************************************
* Frequency Hopping Functions
*****************************************************************************/

/**
 * Synchronize the clock after a packet receive from our coordinator on the syncronization channel.
 * This function should be called when a packet is received on the synchronization channel.
 *
 * @param[in] rfm22b_dev  The device structure
 */
static void rfm22_synchronizeClock(struct pios_rfm22b_dev *rfm22b_dev)
{
    portTickType start_time = rfm22b_dev->packet_start_ticks;

    // This packet was transmitted on channel 0, calculate the time delta that will force us to transmit on channel 0 at the time this packet started.
    uint16_t frequency_hop_cycle_time = rfm22b_dev->packet_period * rfm22b_dev->num_channels;
    uint16_t time_delta = start_time % frequency_hop_cycle_time;

    rfm22b_dev->time_delta = frequency_hop_cycle_time - time_delta + 1;
}

/**
 * Return the extimated current clock ticks count on the coordinator modem.
 * This is the master clock used for all synchronization.
 *
 * @param[in] rfm22b_dev  The device structure
 */
static portTickType rfm22_coordinatorTime(struct pios_rfm22b_dev *rfm22b_dev, portTickType ticks)
{
    if (rfm22_isCoordinator(rfm22b_dev)) {
        return ticks;
    }
    return ticks + rfm22b_dev->time_delta;
}

/**
 * Return true if this modem is in the send interval, which allows the modem to initiate a transmit.
 *
 * @param[in] rfm22b_dev  The device structure
 */
static bool rfm22_timeToSend(struct pios_rfm22b_dev *rfm22b_dev)
{
    portTickType time = rfm22_coordinatorTime(rfm22b_dev, xTaskGetTickCount());
    bool is_coordinator = rfm22_isCoordinator(rfm22b_dev);

    // If this is a one-way link, only the coordinator can send.
    if (rfm22b_dev->one_way_link) {
        if (is_coordinator) {
            return ((time -1) % (rfm22b_dev->packet_period)) == 0;
        } else {
            return false;
        }
    }

    if (!is_coordinator) {
        time += rfm22b_dev->packet_period - 1;
    } else {
        time -= 1;
    }
    return (time % (rfm22b_dev->packet_period * 2)) == 0;
}

/**
 * Calculate the nth channel index.
 *
 * @param[in] rfm22b_dev  The device structure
 * @param[in] index  The channel index to calculate
 */
static uint8_t rfm22_calcChannel(struct pios_rfm22b_dev *rfm22b_dev, uint8_t index)
{
    // Make sure we don't index outside of the range.
    uint8_t idx = index % rfm22b_dev->num_channels;

    rfm22b_dev->channel_index = idx;
    if (idx == 0) {
        rfm22b_dev->on_sync_channel = true;
    }
    return rfm22b_dev->channels[idx];
}

/**
 * Calculate what the current channel shold be.
 *
 * @param[in] rfm22b_dev  The device structure
 */
static uint8_t rfm22_calcChannelFromClock(struct pios_rfm22b_dev *rfm22b_dev)
{
    portTickType time = rfm22_coordinatorTime(rfm22b_dev, xTaskGetTickCount());
    // Divide time into 8ms blocks.  Coordinator sends in first 2 ms, and remote send in 5th and 6th ms.
    // Channel changes occur in the last 2 ms.
    uint8_t n = (time / rfm22b_dev->packet_period) % rfm22b_dev->num_channels;

    return rfm22_calcChannel(rfm22b_dev, n);
}

/**
 * Change channels to the calculated current channel.
 *
 * @param[in] rfm22b_dev  The device structure
 */
static bool rfm22_changeChannel(struct pios_rfm22b_dev *rfm22b_dev)
{
    // A disconnected non-coordinator modem should sit on the sync channel until connected.
    if (!rfm22_isCoordinator(rfm22b_dev) && !rfm22_isConnected(rfm22b_dev)) {
        return rfm22_setFreqHopChannel(rfm22b_dev, rfm22_calcChannel(rfm22b_dev, 0));
    } else {
        return rfm22_setFreqHopChannel(rfm22b_dev, rfm22_calcChannelFromClock(rfm22b_dev));
    }
}


/*****************************************************************************
* Error Handling Functions
*****************************************************************************/

/**
 * Recover from a transmit failure.
 *
 * @param[in] rfm22b_dev The device structure
 * @return enum pios_radio_event  The next event to inject
 */
static enum pios_radio_event rfm22_txFailure(struct pios_rfm22b_dev *rfm22b_dev)
{
    rfm22b_dev->stats.tx_failure++;
    rfm22b_dev->packet_start_ticks = 0;
    rfm22b_dev->tx_data_wr = rfm22b_dev->tx_data_rd = 0;
    return RADIO_EVENT_TX_START;
}

/**
 * Recover from a timeout event.
 *
 * @param[in] rfm22b_dev  The device structure
 * @return enum pios_radio_event  The next event to inject
 */
static enum pios_radio_event rfm22_timeout(struct pios_rfm22b_dev *rfm22b_dev)
{
    rfm22b_dev->stats.timeouts++;
    rfm22b_dev->packet_start_ticks = 0;
    // Release the Tx packet if it's set.
    if (rfm22b_dev->tx_packet_handle != 0) {
        rfm22b_dev->tx_data_rd = rfm22b_dev->tx_data_wr = 0;
    }
    rfm22b_dev->rfm22b_state = RFM22B_STATE_TRANSITION;
    rfm22b_dev->rx_buffer_wr = 0;
    TX_LED_OFF;
    RX_LED_OFF;
#ifdef PIOS_RFM22B_DEBUG_ON_TELEM
    D1_LED_OFF;
    D2_LED_OFF;
    D3_LED_OFF;
    D4_LED_OFF;
#endif
    return RADIO_EVENT_RX_MODE;
}

/**
 * Recover from a severe error.
 *
 * @param[in] rfm22b_dev  The device structure
 * @return enum pios_radio_event  The next event to inject
 */
static enum pios_radio_event rfm22_error(struct pios_rfm22b_dev *rfm22b_dev)
{
    rfm22b_dev->stats.resets++;
    rfm22_clearLEDs();
    return RADIO_EVENT_INITIALIZE;
}

/**
 * A fatal error has occured in the state machine.
 * this should not happen.
 *
 * @parem [in] rfm22b_dev  The device structure
 * @return enum pios_radio_event  The next event to inject
 */
static enum pios_radio_event rfm22_fatal_error(__attribute__((unused)) struct pios_rfm22b_dev *rfm22b_dev)
{
    // RF module error .. flash the LED's
    rfm22_clearLEDs();
    for (unsigned int j = 0; j < 16; j++) {
        USB_LED_ON;
        LINK_LED_ON;
        RX_LED_OFF;
        TX_LED_OFF;

        PIOS_DELAY_WaitmS(200);

        USB_LED_OFF;
        LINK_LED_OFF;
        RX_LED_ON;
        TX_LED_ON;

        PIOS_DELAY_WaitmS(200);
    }

    PIOS_DELAY_WaitmS(1000);

    PIOS_Assert(0);

    return RADIO_EVENT_FATAL_ERROR;
}


/*****************************************************************************
* Utility Functions
*****************************************************************************/

/**
 * Calculate the time difference between the start time and end time.
 * Times are in ticks.  Also handles rollover.
 *
 * @param[in] start_time  The start time in ticks.
 * @param[in] end_time  The end time in ticks.
 */
static uint32_t pios_rfm22_time_difference_ms(portTickType start_time, portTickType end_time)
{
    if (end_time >= start_time) {
        return (end_time - start_time) * portTICK_RATE_MS;
    }
    // Rollover
    return ((portMAX_DELAY - start_time) + end_time) * portTICK_RATE_MS;
}

/**
 * Allocate the device structure
 */
#if defined(PIOS_INCLUDE_FREERTOS)
static struct pios_rfm22b_dev *pios_rfm22_alloc(void)
{
    struct pios_rfm22b_dev *rfm22b_dev;

    rfm22b_dev = (struct pios_rfm22b_dev *)pvPortMalloc(sizeof(*rfm22b_dev));
    rfm22b_dev->spi_id = 0;
    if (!rfm22b_dev) {
        return NULL;
    }

    rfm22b_dev->magic = PIOS_RFM22B_DEV_MAGIC;
    return rfm22b_dev;
}
#else
static struct pios_rfm22b_dev pios_rfm22b_devs[PIOS_RFM22B_MAX_DEVS];
static uint8_t pios_rfm22b_num_devs;
static struct pios_rfm22b_dev *pios_rfm22_alloc(void)
{
    struct pios_rfm22b_dev *rfm22b_dev;

    if (pios_rfm22b_num_devs >= PIOS_RFM22B_MAX_DEVS) {
        return NULL;
    }

    rfm22b_dev = &pios_rfm22b_devs[pios_rfm22b_num_devs++];
    rfm22b_dev->magic = PIOS_RFM22B_DEV_MAGIC;

    return rfm22b_dev;
}
#endif /* if defined(PIOS_INCLUDE_FREERTOS) */

/**
 * Turn off all of the LEDs
 */
static void rfm22_clearLEDs(void)
{
    LINK_LED_OFF;
    RX_LED_OFF;
    TX_LED_OFF;
#ifdef PIOS_RFM22B_DEBUG_ON_TELEM
    D1_LED_OFF;
    D2_LED_OFF;
    D3_LED_OFF;
    D4_LED_OFF;
#endif
}


/*****************************************************************************
* SPI Read/Write Functions
*****************************************************************************/

/**
 * Assert the chip select line.
 *
 * @param[in] rfm22b_dev  The RFM22B device.
 */
static void rfm22_assertCs(struct pios_rfm22b_dev *rfm22b_dev)
{
    PIOS_DELAY_WaituS(1);
    if (rfm22b_dev->spi_id != 0) {
        PIOS_SPI_RC_PinSet(rfm22b_dev->spi_id, rfm22b_dev->slave_num, 0);
    }
}

/**
 * Deassert the chip select line.
 *
 * @param[in] rfm22b_dev  The RFM22B device structure pointer.
 */
static void rfm22_deassertCs(struct pios_rfm22b_dev *rfm22b_dev)
{
    if (rfm22b_dev->spi_id != 0) {
        PIOS_SPI_RC_PinSet(rfm22b_dev->spi_id, rfm22b_dev->slave_num, 1);
    }
}

/**
 * Claim the SPI bus.
 *
 * @param[in] rfm22b_dev  The RFM22B device structure pointer.
 */
static void rfm22_claimBus(struct pios_rfm22b_dev *rfm22b_dev)
{
    if (rfm22b_dev->spi_id != 0) {
        PIOS_SPI_ClaimBus(rfm22b_dev->spi_id);
    }
}

/**
 * Release the SPI bus.
 *
 * @param[in] rfm22b_dev  The RFM22B device structure pointer.
 */
static void rfm22_releaseBus(struct pios_rfm22b_dev *rfm22b_dev)
{
    if (rfm22b_dev->spi_id != 0) {
        PIOS_SPI_ReleaseBus(rfm22b_dev->spi_id);
    }
}

/**
 * Claim the semaphore and write a byte to a register
 *
 * @param[in] rfm22b_dev  The RFM22B device.
 * @param[in] addr The address to write to
 * @param[in] data The datat to write to that address
 */
static void rfm22_write_claim(struct pios_rfm22b_dev *rfm22b_dev, uint8_t addr, uint8_t data)
{
    rfm22_claimBus(rfm22b_dev);
    rfm22_assertCs(rfm22b_dev);
    uint8_t buf[2] = { addr | 0x80, data };
    PIOS_SPI_TransferBlock(rfm22b_dev->spi_id, buf, NULL, sizeof(buf), NULL);
    rfm22_deassertCs(rfm22b_dev);
    rfm22_releaseBus(rfm22b_dev);
}

/**
 * Write a byte to a register without claiming the semaphore
 *
 * @param[in] rfm22b_dev  The RFM22B device.
 * @param[in] addr The address to write to
 * @param[in] data The datat to write to that address
 */
static void rfm22_write(struct pios_rfm22b_dev *rfm22b_dev, uint8_t addr, uint8_t data)
{
    rfm22_assertCs(rfm22b_dev);
    uint8_t buf[2] = { addr | 0x80, data };
    PIOS_SPI_TransferBlock(rfm22b_dev->spi_id, buf, NULL, sizeof(buf), NULL);
    rfm22_deassertCs(rfm22b_dev);
}

/**
 * Read a byte from an RFM22b register without claiming the bus
 *
 * @param[in] rfm22b_dev  The RFM22B device structure pointer.
 * @param[in] addr The address to read from
 * @return Returns the result of the register read
 */
static uint8_t rfm22_read(struct pios_rfm22b_dev *rfm22b_dev, uint8_t addr)
{
    uint8_t out[2] = { addr &0x7F, 0xFF };
    uint8_t in[2];

    rfm22_assertCs(rfm22b_dev);
    PIOS_SPI_TransferBlock(rfm22b_dev->spi_id, out, in, sizeof(out), NULL);
    rfm22_deassertCs(rfm22b_dev);
    return in[1];
}

#endif /* PIOS_INCLUDE_RFM22B */

/**
 * @}
 * @}
 */

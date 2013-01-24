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

/* Project Includes */
#include "pios.h"

#if defined(PIOS_INCLUDE_RFM22B)

#include <pios_spi_priv.h>
#include <packet_handler.h>
#include <pios_rfm22b_priv.h>
#include <ecc.h>

/* Local Defines */
#define STACK_SIZE_BYTES 200
#define TASK_PRIORITY (tskIDLE_PRIORITY + 2)
#define ISR_TIMEOUT 5 // ms
#define EVENT_QUEUE_SIZE 5
#define PACKET_QUEUE_SIZE 3
#define RFM22B_DEFAULT_RX_DATARATE RFM22_datarate_64000

// The maximum amount of time without activity before initiating a reset.
#define PIOS_RFM22B_SUPERVISOR_TIMEOUT 100  // ms

// The time between updates over the radio link.
#define RADIOSTATS_UPDATE_PERIOD_MS 250

// this is too adjust the RF module so that it is on frequency
#define OSC_LOAD_CAP				0x7F	// cap = 12.5pf .. default
#define OSC_LOAD_CAP_1				0x7D	// board 1
#define OSC_LOAD_CAP_2				0x7B	// board 2
#define OSC_LOAD_CAP_3				0x7E	// board 3
#define OSC_LOAD_CAP_4				0x7F	// board 4

// ************************************

#define TX_PREAMBLE_NIBBLES				12		// 7 to 511 (number of nibbles)
#define RX_PREAMBLE_NIBBLES				6		// 5 to 31 (number of nibbles)

// the size of the rf modules internal FIFO buffers
#define FIFO_SIZE	64

#define TX_FIFO_HI_WATERMARK			62		// 0-63
#define TX_FIFO_LO_WATERMARK			32		// 0-63

#define RX_FIFO_HI_WATERMARK			32		// 0-63

#define PREAMBLE_BYTE					0x55	// preamble byte (preceeds SYNC_BYTE's)

#define SYNC_BYTE_1						0x2D    // RF sync bytes (32-bit in all)
#define SYNC_BYTE_2						0xD4    //
#define SYNC_BYTE_3						0x4B    //
#define SYNC_BYTE_4						0x59    //

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

// ************************************
// Normal data streaming
// GFSK modulation
// no manchester encoding
// data whitening
// FIFO mode
//  5-nibble rx preamble length detection
// 10-nibble tx preamble length
// AFC enabled

/* Local type definitions */
enum pios_rfm22b_dev_magic {
	PIOS_RFM22B_DEV_MAGIC = 0x68e971b6,
};

enum pios_rfm22b_state {
	RFM22B_STATE_UNINITIALIZED,
	RFM22B_STATE_INITIALIZING,
	RFM22B_STATE_RX_MODE,
	RFM22B_STATE_WAIT_PREAMBLE,
	RFM22B_STATE_WAIT_SYNC,
	RFM22B_STATE_RX_DATA,
	RFM22B_STATE_TX_START,
	RFM22B_STATE_TX_DATA,
	RFM22B_STATE_TIMEOUT,
	RFM22B_STATE_ERROR,
	RFM22B_STATE_FATAL_ERROR,

	RFM22B_STATE_NUM_STATES // Must be last
};

enum pios_rfm22b_event {
	RFM22B_EVENT_INITIALIZE,
	RFM22B_EVENT_INITIALIZED,
	RFM22B_EVENT_INT_RECEIVED,
	RFM22B_EVENT_RX_MODE,
	RFM22B_EVENT_PREAMBLE_DETECTED,
	RFM22B_EVENT_SYNC_DETECTED,
	RFM22B_EVENT_RX_COMPLETE,
	RFM22B_EVENT_SEND_PACKET,
	RFM22B_EVENT_TX_START,
	RFM22B_EVENT_TX_STARTED,
	RFM22B_EVENT_TX_COMPLETE,
	RFM22B_EVENT_TIMEOUT,
	RFM22B_EVENT_ERROR,
	RFM22B_EVENT_FATAL_ERROR,

	RFM22B_EVENT_NUM_EVENTS  // Must be last
};

struct pios_rfm22b_dev {
	enum pios_rfm22b_dev_magic magic;
	struct pios_rfm22b_cfg cfg;

	uint32_t spi_id;
	uint32_t slave_num;

	// The device ID
	uint32_t deviceID;

	// The destination ID
	uint32_t destination_id;

	// The task handle
	xTaskHandle taskHandle;

	// ISR pending
	xSemaphoreHandle isrPending;

	// Receive packet complete
	xSemaphoreHandle rxsem;

	// The COM callback functions.
	pios_com_callback rx_in_cb;
	uint32_t rx_in_context;
	pios_com_callback tx_out_cb;
	uint32_t tx_out_context;

	// the transmit power to use for data transmissions
	uint8_t	tx_power;

	// The RF datarate lookup index.
	uint8_t datarate;

	// The state machine state and the current event
	enum pios_rfm22b_state state;
	// The event queue handle
	xQueueHandle eventQueue;

	// device status register
	uint8_t device_status;
	// interrupt status register 1
	uint8_t int_status1;
	// interrupt status register 2
	uint8_t int_status2;
	// ezmac status register
	uint8_t ezmac_status;

	// The packet transmission counts
	uint32_t tx_packet_count;
	uint32_t rx_packet_count;

	// The dropped packet counters
	uint8_t slow_block;
	uint8_t fast_block;
	uint8_t slow_good_packets;
	uint8_t fast_good_packets;
	uint8_t slow_corrected_packets;
	uint8_t fast_corrected_packets;
	uint8_t slow_error_packets;
	uint8_t fast_error_packets;
	uint8_t slow_link_quality;
	uint8_t fast_link_quality;

	// Stats
	uint16_t resets;
	uint16_t timeouts;
	uint16_t errors;
	// the current RSSI (register value)
	uint8_t rssi;
	// RSSI in dBm
	int8_t rssi_dBm;

	// The packet queue handle
	xQueueHandle packetQueue;

	// The current tx packet
	PHPacketHandle tx_packet;
	// the tx data read index
	uint16_t tx_data_rd;
	// the tx data write index
	uint16_t tx_data_wr;

	// The current rx packet
	PHPacketHandle rx_packet;
	// The previous rx packet
	PHPacketHandle rx_packet_prev;
	// The next rx packet
	PHPacketHandle rx_packet_next;
	// the receive buffer write index
	uint16_t rx_buffer_wr;
	// the receive buffer write index
	uint16_t rx_packet_len;
	
	// The frequency hopping step size
	float frequency_step_size;
	// current frequency hop channel
	uint8_t	frequency_hop_channel;
	// the frequency hop step size
	uint8_t frequency_hop_step_size_reg;
	// afc correction reading (in Hz)
	int32_t afc_correction_Hz;
	int8_t rx_packet_start_afc_Hz;

	// The status packet
	PHStatusPacket status_packet;

	// The maximum time (ms) that it should take to transmit / receive a packet.
	uint32_t max_packet_time;
	portTickType packet_start_ticks;
};

struct pios_rfm22b_transition {
	enum pios_rfm22b_event (*entry_fn) (struct pios_rfm22b_dev *rfm22b_dev);
	enum pios_rfm22b_state next_state[RFM22B_EVENT_NUM_EVENTS];
};

// Must ensure these prefilled arrays match the define sizes
static const uint8_t FULL_PREAMBLE[FIFO_SIZE] = 
	{PREAMBLE_BYTE,PREAMBLE_BYTE,PREAMBLE_BYTE,PREAMBLE_BYTE,PREAMBLE_BYTE,
	PREAMBLE_BYTE,PREAMBLE_BYTE,PREAMBLE_BYTE,PREAMBLE_BYTE,PREAMBLE_BYTE,
	PREAMBLE_BYTE,PREAMBLE_BYTE,PREAMBLE_BYTE,PREAMBLE_BYTE,PREAMBLE_BYTE,
	PREAMBLE_BYTE,PREAMBLE_BYTE,PREAMBLE_BYTE,PREAMBLE_BYTE,PREAMBLE_BYTE,
	PREAMBLE_BYTE,PREAMBLE_BYTE,PREAMBLE_BYTE,PREAMBLE_BYTE,PREAMBLE_BYTE,
	PREAMBLE_BYTE,PREAMBLE_BYTE,PREAMBLE_BYTE,PREAMBLE_BYTE,PREAMBLE_BYTE,
	PREAMBLE_BYTE,PREAMBLE_BYTE,PREAMBLE_BYTE,PREAMBLE_BYTE,PREAMBLE_BYTE,
	PREAMBLE_BYTE,PREAMBLE_BYTE,PREAMBLE_BYTE,PREAMBLE_BYTE,PREAMBLE_BYTE,
	PREAMBLE_BYTE,PREAMBLE_BYTE,PREAMBLE_BYTE,PREAMBLE_BYTE,PREAMBLE_BYTE,
	PREAMBLE_BYTE,PREAMBLE_BYTE,PREAMBLE_BYTE,PREAMBLE_BYTE,PREAMBLE_BYTE,
	PREAMBLE_BYTE,PREAMBLE_BYTE,PREAMBLE_BYTE,PREAMBLE_BYTE,PREAMBLE_BYTE,
	PREAMBLE_BYTE,PREAMBLE_BYTE,PREAMBLE_BYTE,PREAMBLE_BYTE,PREAMBLE_BYTE,
	PREAMBLE_BYTE,PREAMBLE_BYTE,PREAMBLE_BYTE,PREAMBLE_BYTE}; // 64 bytes
static const uint8_t HEADER[(TX_PREAMBLE_NIBBLES + 1)/2 + 2] = {PREAMBLE_BYTE, PREAMBLE_BYTE, PREAMBLE_BYTE, PREAMBLE_BYTE,PREAMBLE_BYTE, PREAMBLE_BYTE, SYNC_BYTE_1, SYNC_BYTE_2};
static const uint8_t OUT_FF[64] = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
	0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
	0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
	0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
	0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
	0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
	0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
	0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};

/* Local function forwared declarations */
static void PIOS_RFM22B_Task(void *parameters);
static void PIOS_RFM22B_InjectEvent(struct pios_rfm22b_dev *rfm22b_dev, enum pios_rfm22b_event event, bool inISR);
static bool rfm22_readStatus(struct pios_rfm22b_dev *rfm22b_dev);
static enum pios_rfm22b_event rfm22_setRxMode(struct pios_rfm22b_dev *rfm22b_dev);
static enum pios_rfm22b_event rfm22_detectPreamble(struct pios_rfm22b_dev *rfm22b_dev);
static enum pios_rfm22b_event rfm22_detectSync(struct pios_rfm22b_dev *rfm22b_dev);
static enum pios_rfm22b_event rfm22_rxData(struct pios_rfm22b_dev *rfm22b_dev);
static enum pios_rfm22b_event rfm22_init(struct pios_rfm22b_dev *rfm22b_dev);
static enum pios_rfm22b_event rfm22_txStart(struct pios_rfm22b_dev *rfm22b_dev);
static enum pios_rfm22b_event rfm22_txData(struct pios_rfm22b_dev *rfm22b_dev);
static enum pios_rfm22b_event rfm22_process_state_transition(struct pios_rfm22b_dev *rfm22b_dev, enum pios_rfm22b_event event);
static enum pios_rfm22b_event rfm22_timeout(struct pios_rfm22b_dev *rfm22b_dev);
static enum pios_rfm22b_event rfm22_error(struct pios_rfm22b_dev *rfm22b_dev);
static enum pios_rfm22b_event rfm22_fatal_error(struct pios_rfm22b_dev *rfm22b_dev);
static bool rfm22_sendStatus(struct pios_rfm22b_dev *rfm22b_dev);

// SPI read/write functions
static void rfm22_assertCs();
static void rfm22_deassertCs();
static void rfm22_claimBus();
static void rfm22_releaseBus();
static void rfm22_write(uint8_t addr, uint8_t data);
static uint8_t rfm22_read(uint8_t addr);
static uint8_t rfm22_read_noclaim(uint8_t addr);

/* Provide a COM driver */
static void PIOS_RFM22B_ChangeBaud(uint32_t rfm22b_id, uint32_t baud);
static void PIOS_RFM22B_RegisterRxCallback(uint32_t rfm22b_id, pios_com_callback rx_in_cb, uint32_t context);
static void PIOS_RFM22B_RegisterTxCallback(uint32_t rfm22b_id, pios_com_callback tx_out_cb, uint32_t context);
static void PIOS_RFM22B_TxStart(uint32_t rfm22b_id, uint16_t tx_bytes_avail);
static void PIOS_RFM22B_RxStart(uint32_t rfm22b_id, uint16_t rx_bytes_avail);

/* Local variables */
const struct pios_com_driver pios_rfm22b_com_driver = {
	.set_baud   = PIOS_RFM22B_ChangeBaud,
	.tx_start   = PIOS_RFM22B_TxStart,
	.rx_start   = PIOS_RFM22B_RxStart,
	.bind_tx_cb = PIOS_RFM22B_RegisterTxCallback,
	.bind_rx_cb = PIOS_RFM22B_RegisterRxCallback,
};

/* Te state transition table */
const static struct pios_rfm22b_transition rfm22b_transitions[RFM22B_STATE_NUM_STATES] = {
	[RFM22B_STATE_UNINITIALIZED] = {
		.entry_fn = 0,
		.next_state = {
			[RFM22B_EVENT_INITIALIZE] = RFM22B_STATE_INITIALIZING,
			[RFM22B_EVENT_ERROR] = RFM22B_STATE_ERROR,
		},
	},
	[RFM22B_STATE_INITIALIZING] = {
		.entry_fn = rfm22_init,
		.next_state = {
			[RFM22B_EVENT_INITIALIZED] = RFM22B_STATE_TX_START,
			[RFM22B_EVENT_ERROR] = RFM22B_STATE_ERROR,
			[RFM22B_EVENT_FATAL_ERROR] = RFM22B_STATE_FATAL_ERROR,
		},
	},
	[RFM22B_STATE_RX_MODE] = {
		.entry_fn = rfm22_setRxMode,
		.next_state = {
			[RFM22B_EVENT_INT_RECEIVED] = RFM22B_STATE_WAIT_PREAMBLE,
			[RFM22B_EVENT_SEND_PACKET] = RFM22B_STATE_TX_START,
			[RFM22B_EVENT_TX_START] = RFM22B_STATE_TX_START,
			[RFM22B_EVENT_TIMEOUT] = RFM22B_STATE_TIMEOUT,
			[RFM22B_EVENT_ERROR] = RFM22B_STATE_ERROR,
			[RFM22B_EVENT_FATAL_ERROR] = RFM22B_STATE_FATAL_ERROR,
		},
	},
	[RFM22B_STATE_WAIT_PREAMBLE] = {
		.entry_fn = rfm22_detectPreamble,
		.next_state = {
			[RFM22B_EVENT_INT_RECEIVED] = RFM22B_STATE_WAIT_PREAMBLE,
			[RFM22B_EVENT_PREAMBLE_DETECTED] = RFM22B_STATE_WAIT_SYNC,
			[RFM22B_EVENT_SEND_PACKET] = RFM22B_STATE_TX_START,
			[RFM22B_EVENT_TX_START] = RFM22B_STATE_TX_START,
			[RFM22B_EVENT_TIMEOUT] = RFM22B_STATE_TIMEOUT,
			[RFM22B_EVENT_ERROR] = RFM22B_STATE_ERROR,
			[RFM22B_EVENT_FATAL_ERROR] = RFM22B_STATE_FATAL_ERROR,
		},
	},
	[RFM22B_STATE_WAIT_SYNC] = {
		.entry_fn = rfm22_detectSync,
		.next_state = {
			[RFM22B_EVENT_INT_RECEIVED] = RFM22B_STATE_WAIT_SYNC,
			[RFM22B_EVENT_SYNC_DETECTED] = RFM22B_STATE_RX_DATA,
			[RFM22B_EVENT_TX_START] = RFM22B_STATE_TX_START,
			[RFM22B_EVENT_TIMEOUT] = RFM22B_STATE_TIMEOUT,
			[RFM22B_EVENT_ERROR] = RFM22B_STATE_ERROR,
			[RFM22B_EVENT_FATAL_ERROR] = RFM22B_STATE_FATAL_ERROR,
		},
	},
	[RFM22B_STATE_RX_DATA] = {
		.entry_fn = rfm22_rxData,
		.next_state = {
			[RFM22B_EVENT_INT_RECEIVED] = RFM22B_STATE_RX_DATA,
			[RFM22B_EVENT_RX_COMPLETE] = RFM22B_STATE_TX_START,
			[RFM22B_EVENT_TIMEOUT] = RFM22B_STATE_TIMEOUT,
			[RFM22B_EVENT_ERROR] = RFM22B_STATE_ERROR,
			[RFM22B_EVENT_FATAL_ERROR] = RFM22B_STATE_FATAL_ERROR,
		},
	},
	[RFM22B_STATE_TX_START] = {
		.entry_fn = rfm22_txStart,
		.next_state = {
			[RFM22B_EVENT_INT_RECEIVED] = RFM22B_STATE_TX_DATA,
			[RFM22B_EVENT_RX_MODE] = RFM22B_STATE_RX_MODE,
			[RFM22B_EVENT_TIMEOUT] = RFM22B_STATE_TIMEOUT,
			[RFM22B_EVENT_ERROR] = RFM22B_STATE_ERROR,
			[RFM22B_EVENT_FATAL_ERROR] = RFM22B_STATE_FATAL_ERROR,
		},
	},
	[RFM22B_STATE_TX_DATA] = {
		.entry_fn = rfm22_txData,
		.next_state = {
			[RFM22B_EVENT_INT_RECEIVED] = RFM22B_STATE_TX_DATA,
			[RFM22B_EVENT_TX_COMPLETE] = RFM22B_STATE_TX_START,
			[RFM22B_EVENT_TIMEOUT] = RFM22B_STATE_TIMEOUT,
			[RFM22B_EVENT_ERROR] = RFM22B_STATE_ERROR,
			[RFM22B_EVENT_FATAL_ERROR] = RFM22B_STATE_FATAL_ERROR,
		},
	},
	[RFM22B_STATE_TIMEOUT] = {
		.entry_fn = rfm22_timeout,
		.next_state = {
			[RFM22B_EVENT_TX_START] = RFM22B_STATE_TX_START,
			[RFM22B_EVENT_INITIALIZE] = RFM22B_STATE_INITIALIZING,
			[RFM22B_EVENT_ERROR] = RFM22B_STATE_ERROR,
			[RFM22B_EVENT_FATAL_ERROR] = RFM22B_STATE_FATAL_ERROR,
		},
	},
	[RFM22B_STATE_ERROR] = {
		.entry_fn = rfm22_error,
		.next_state = {
			[RFM22B_EVENT_INITIALIZE] = RFM22B_STATE_INITIALIZING,
			[RFM22B_EVENT_ERROR] = RFM22B_STATE_ERROR,
			[RFM22B_EVENT_FATAL_ERROR] = RFM22B_STATE_FATAL_ERROR,
		},
	},
	[RFM22B_STATE_FATAL_ERROR] = {
		.entry_fn = rfm22_fatal_error,
		.next_state = {
			[RFM22B_EVENT_ERROR] = RFM22B_STATE_ERROR,
			[RFM22B_EVENT_FATAL_ERROR] = RFM22B_STATE_FATAL_ERROR,
		},
	},
};

// xtal 10 ppm, 434MHz
#define LOOKUP_SIZE	 14
static const uint32_t            data_rate[] = {   500,  1000,  2000,  4000,  8000,  9600, 16000, 19200, 24000,  32000,  64000, 128000, 192000, 256000};
static const uint8_t      modulation_index[] = {    16,     8,     4,     2,     1,     1,     1,     1,     1,      1,      1,      1,      1,      1};
static const uint32_t       freq_deviation[] = {  4000,  4000,  4000,  4000,  4000,  4800,  8000,  9600, 12000,  16000,  32000,  64000,  96000, 128000};
static const uint32_t         rx_bandwidth[] = { 17500, 17500, 17500, 17500, 17500, 19400, 32200, 38600, 51200,  64100, 137900, 269300, 420200, 518800};
static const int8_t        est_rx_sens_dBm[] = {  -118,  -118,  -117,  -116,  -115,  -115,  -112,  -112,  -110,   -109,   -106,   -103,   -101,   -100}; // estimated receiver sensitivity for BER = 1E-3

static const uint8_t                reg_1C[] = {  0x37,  0x37,  0x37,  0x37,  0x3A,  0x3B,  0x26,  0x28,  0x2E,   0x16,   0x07,   0x83,   0x8A,   0x8C}; // rfm22_if_filter_bandwidth

static const uint8_t                reg_1D[] = {  0x44,  0x44,  0x44,  0x44,  0x44,  0x44,  0x44,  0x44,  0x44,   0x44,   0x44,   0x44,   0x44,   0x44}; // rfm22_afc_loop_gearshift_override
static const uint8_t                reg_1E[] = {  0x0A,  0x0A,  0x0A,  0x0A,  0x0A,  0x0A,  0x0A,  0x0A,  0x0A,   0x0A,   0x0A,   0x0A,   0x0A,   0x02}; // rfm22_afc_timing_control

static const uint8_t                reg_1F[] = {  0x03,  0x03,  0x03,  0x03,  0x03,  0x03,  0x03,  0x03,  0x03,   0x03,   0x03,   0x03,   0x03,   0x03}; // rfm22_clk_recovery_gearshift_override
static const uint8_t                reg_20[] = {  0xE8,  0xF4,  0xFA,  0x70,  0x3F,  0x34,  0x3F,  0x34,  0x2A,   0x3F,   0x3F,   0x5E,   0x3F,   0x2F}; // rfm22_clk_recovery_oversampling_ratio
static const uint8_t                reg_21[] = {  0x60,  0x20,  0x00,  0x01,  0x02,  0x02,  0x02,  0x02,  0x03,   0x02,   0x02,   0x01,   0x02,   0x02}; // rfm22_clk_recovery_offset2
static const uint8_t                reg_22[] = {  0x20,  0x41,  0x83,  0x06,  0x0C,  0x75,  0x0C,  0x75,  0x12,   0x0C,   0x0C,   0x5D,   0x0C,   0xBB}; // rfm22_clk_recovery_offset1
static const uint8_t                reg_23[] = {  0xC5,  0x89,  0x12,  0x25,  0x4A,  0x25,  0x4A,  0x25,  0x6F,   0x4A,   0x4A,   0x86,   0x4A,   0x0D}; // rfm22_clk_recovery_offset0
static const uint8_t                reg_24[] = {  0x00,  0x00,  0x00,  0x02,  0x07,  0x07,  0x07,  0x07,  0x07,   0x07,   0x07,   0x05,   0x07,   0x07}; // rfm22_clk_recovery_timing_loop_gain1
static const uint8_t                reg_25[] = {  0x0A,  0x23,  0x85,  0x0E,  0xFF,  0xFF,  0xFF,  0xFF,  0xFF,   0xFF,   0xFF,   0x74,   0xFF,   0xFF}; // rfm22_clk_recovery_timing_loop_gain0

static const uint8_t                reg_2A[] = {  0x0E,  0x0E,  0x0E,  0x0E,  0x0E,  0x0D,  0x0D,  0x0E,  0x12,   0x17,   0x31,   0x50,   0x50,   0x50}; // rfm22_afc_limiter .. AFC_pull_in_range = �AFCLimiter[7:0] x (hbsel+1) x 625 Hz

static const uint8_t                reg_6E[] = {  0x04,  0x08,  0x10,  0x20,  0x41,  0x4E,  0x83,  0x9D,  0xC4,   0x08,   0x10,   0x20,   0x31,   0x41}; // rfm22_tx_data_rate1
static const uint8_t                reg_6F[] = {  0x19,  0x31,  0x62,  0xC5,  0x89,  0xA5,  0x12,  0x49,  0x9C,   0x31,   0x62,   0xC5,   0x27,   0x89}; // rfm22_tx_data_rate0

static const uint8_t                reg_70[] = {  0x2D,  0x2D,  0x2D,  0x2D,  0x2D,  0x2D,  0x2D,  0x2D,  0x2D,   0x0D,   0x0D,   0x0D,   0x0D,   0x0D}; // rfm22_modulation_mode_control1
static const uint8_t                reg_71[] = {  0x23,  0x23,  0x23,  0x23,  0x23,  0x23,  0x23,  0x23,  0x23,   0x23,   0x23,   0x23,   0x23,   0x23}; // rfm22_modulation_mode_control2

static const uint8_t                reg_72[] = {  0x06,  0x06,  0x06,  0x06,  0x06,  0x08,  0x0D,  0x0F,  0x13,   0x1A,   0x33,   0x66,   0x9A,   0xCD}; // rfm22_frequency_deviation

// ************************************
// Scan Spectrum settings
// GFSK modulation
// no manchester encoding
// data whitening
// FIFO mode
//  5-nibble rx preamble length detection
// 10-nibble tx preamble length
#define SS_LOOKUP_SIZE	 2

// xtal 1 ppm, 434MHz
static const uint32_t ss_rx_bandwidth[] = {  2600, 10600};

static const uint8_t ss_reg_1C[] = {  0x51, 0x32}; // rfm22_if_filter_bandwidth
static const uint8_t ss_reg_1D[] = {  0x00, 0x00}; // rfm22_afc_loop_gearshift_override

static const uint8_t ss_reg_20[] = {  0xE8, 0x38}; // rfm22_clk_recovery_oversampling_ratio
static const uint8_t ss_reg_21[] = {  0x60, 0x02}; // rfm22_clk_recovery_offset2
static const uint8_t ss_reg_22[] = {  0x20, 0x4D}; // rfm22_clk_recovery_offset1
static const uint8_t ss_reg_23[] = {  0xC5, 0xD3}; // rfm22_clk_recovery_offset0
static const uint8_t ss_reg_24[] = {  0x00, 0x07}; // rfm22_clk_recovery_timing_loop_gain1
static const uint8_t ss_reg_25[] = {  0x0F, 0xFF}; // rfm22_clk_recovery_timing_loop_gain0

static const uint8_t ss_reg_2A[] = {  0xFF, 0xFF}; // rfm22_afc_limiter .. AFC_pull_in_range = �AFCLimiter[7:0] x (hbsel+1) x 625 Hz

static const uint8_t ss_reg_70[] = {  0x24, 0x2D}; // rfm22_modulation_mode_control1
static const uint8_t ss_reg_71[] = {  0x2B, 0x23}; // rfm22_modulation_mode_control2


static bool PIOS_RFM22B_validate(struct pios_rfm22b_dev * rfm22b_dev)
{
	return (rfm22b_dev != NULL && rfm22b_dev->magic == PIOS_RFM22B_DEV_MAGIC);
}

#if defined(PIOS_INCLUDE_FREERTOS)
static struct pios_rfm22b_dev * PIOS_RFM22B_alloc(void)
{
	struct pios_rfm22b_dev * rfm22b_dev;

	rfm22b_dev = (struct pios_rfm22b_dev *)pvPortMalloc(sizeof(*rfm22b_dev));
	rfm22b_dev->spi_id = 0;
	if (!rfm22b_dev) return(NULL);

	rfm22b_dev->magic = PIOS_RFM22B_DEV_MAGIC;
	return(rfm22b_dev);
}
#else
static struct pios_rfm22b_dev pios_rfm22b_devs[PIOS_RFM22B_MAX_DEVS];
static uint8_t pios_rfm22b_num_devs;
static struct pios_rfm22b_dev * PIOS_RFM22B_alloc(void)
{
	struct pios_rfm22b_dev * rfm22b_dev;

	if (pios_rfm22b_num_devs >= PIOS_RFM22B_MAX_DEVS)
		return NULL;

	rfm22b_dev = &pios_rfm22b_devs[pios_rfm22b_num_devs++];
	rfm22b_dev->magic = PIOS_RFM22B_DEV_MAGIC;

	return (rfm22b_dev);
}
#endif

static struct pios_rfm22b_dev * g_rfm22b_dev =  NULL;

/**
 * Initialise an RFM22B device
 */
int32_t PIOS_RFM22B_Init(uint32_t *rfm22b_id, uint32_t spi_id, uint32_t slave_num, const struct pios_rfm22b_cfg *cfg)
{
	PIOS_DEBUG_Assert(rfm22b_id);
	PIOS_DEBUG_Assert(cfg);

	// Allocate the device structure.
	struct pios_rfm22b_dev * rfm22b_dev = (struct pios_rfm22b_dev *) PIOS_RFM22B_alloc();
	if (!rfm22b_dev)
		return(-1);

	// Store the SPI handle
	rfm22b_dev->slave_num = slave_num;
	rfm22b_dev->spi_id = spi_id;

	// Set the state to initializing.
	rfm22b_dev->state = RFM22B_STATE_UNINITIALIZED;
	// Create the event queue
	rfm22b_dev->eventQueue = xQueueCreate(EVENT_QUEUE_SIZE, sizeof(enum pios_rfm22b_event));

	// Initialize the register values.
	rfm22b_dev->device_status = 0;
	rfm22b_dev->int_status1 = 0;
	rfm22b_dev->int_status2 = 0;
	rfm22b_dev->ezmac_status = 0;

	// Initlize the link stats.
	rfm22b_dev->slow_block = 0;
	rfm22b_dev->fast_block = 0;
	rfm22b_dev->slow_good_packets = 0;
	rfm22b_dev->fast_good_packets = 0;
	rfm22b_dev->slow_corrected_packets = 0;
	rfm22b_dev->fast_corrected_packets = 0;
	rfm22b_dev->slow_error_packets = 0;
	rfm22b_dev->fast_error_packets = 0;
	rfm22b_dev->slow_link_quality = 255;
	rfm22b_dev->fast_link_quality = 255;

	// Initialize the stats.
	rfm22b_dev->resets = 0;
	rfm22b_dev->timeouts = 0;
	rfm22b_dev->errors = 0;
	rfm22b_dev->tx_packet_count = 0;
	rfm22b_dev->rx_packet_count = 0;
	rfm22b_dev->rssi = 0;
	rfm22b_dev->rssi_dBm = -127;

	// Bind the configuration to the device instance
	rfm22b_dev->cfg = *cfg;
	rfm22b_dev->datarate = RFM22B_DEFAULT_RX_DATARATE;

	// Initialize the packets.
	rfm22b_dev->rx_packet = NULL;
	rfm22b_dev->rx_packet_next = NULL;
	rfm22b_dev->rx_packet_prev = NULL;
	rfm22b_dev->rx_packet_len = 0;
	rfm22b_dev->tx_packet = NULL;

	*rfm22b_id = (uint32_t)rfm22b_dev;
	g_rfm22b_dev = rfm22b_dev;

	// Calculate the (approximate) maximum amount of time that it should take to transmit / receive a packet.
	rfm22b_dev->packet_start_ticks = 0;

	// Create a semaphore to know if an ISR needs responding to
	vSemaphoreCreateBinary( rfm22b_dev->isrPending );

	// Create a semaphore to know when an rx packet is available
	vSemaphoreCreateBinary( rfm22b_dev->rxsem );

	// Create the packet queue.
	rfm22b_dev->packetQueue = xQueueCreate(PACKET_QUEUE_SIZE, sizeof(PHPacketHandle));

	// Initialize the max tx power level.
	PIOS_RFM22B_SetTxPower(*rfm22b_id, cfg->maxTxPower);

	// Create our (hopefully) unique 32 bit id from the processor serial number.
	uint8_t crcs[] = { 0, 0, 0, 0 };
	{
		char serial_no_str[33];
		PIOS_SYS_SerialNumberGet(serial_no_str);
		// Create a 32 bit value using 4 8 bit CRC values.
		for (uint8_t i = 0; serial_no_str[i] != 0; ++i)
			crcs[i % 4] = PIOS_CRC_updateByte(crcs[i % 4], serial_no_str[i]);
	}
	rfm22b_dev->deviceID = crcs[0] | crcs[1] << 8 | crcs[2] << 16 | crcs[3] << 24;
	DEBUG_PRINTF(2, "RF device ID: %x\n\r", rfm22b_dev->deviceID);

	// Initialize the external interrupt.
	PIOS_EXTI_Init(cfg->exti_cfg);

	// Register the watchdog timer for the radio driver task
#ifdef PIOS_WDG_RFM22B
	PIOS_WDG_RegisterFlag(PIOS_WDG_RFM22B);
#endif /* PIOS_WDG_RFM22B */

	// Start the driver task.  This task controls the radio state machine and removed all of the IO from the IRQ handler.
	xTaskCreate(PIOS_RFM22B_Task, (signed char *)"PIOS_RFM22B_Task", STACK_SIZE_BYTES, (void*)rfm22b_dev, TASK_PRIORITY, &(rfm22b_dev->taskHandle));

	// Initialize the radio device.
	PIOS_RFM22B_InjectEvent(rfm22b_dev, RFM22B_EVENT_INITIALIZE, false);

	return(0);
}

/**
 * The RFM22B external interrupt routine.
 */
bool PIOS_RFM22_EXT_Int(void)
{
	if (!PIOS_RFM22B_validate(g_rfm22b_dev))
	    return false;

	// Inject an interrupt event into the state machine.
	PIOS_RFM22B_InjectEvent(g_rfm22b_dev, RFM22B_EVENT_INT_RECEIVED, true);
        return false;
}

/**
 * Inject an event into the RFM22B state machine.
 * \param[in] rfm22b_dev The device structure
 * \param[in] event The event to inject
 * \param[in] inISR Is this being called from an interrrup service routine?
 */
static void PIOS_RFM22B_InjectEvent(struct pios_rfm22b_dev *rfm22b_dev, enum pios_rfm22b_event event, bool inISR)
{

	// Store the event.
	if (xQueueSend(rfm22b_dev->eventQueue, &event, portMAX_DELAY) != pdTRUE)
		return;

	// Signal the semaphore to wake up the handler thread.
	if (inISR) {
		portBASE_TYPE pxHigherPriorityTaskWoken;
		if (xSemaphoreGiveFromISR(rfm22b_dev->isrPending, &pxHigherPriorityTaskWoken) != pdTRUE) {
			// Something went fairly seriously wrong
			rfm22b_dev->errors++;
		}
		portEND_SWITCHING_ISR(pxHigherPriorityTaskWoken);
	}
	else
	{
		if (xSemaphoreGive(rfm22b_dev->isrPending) != pdTRUE) {
			// Something went fairly seriously wrong
			rfm22b_dev->errors++;
		}
	}
}

/**
 * Returns the unique device ID for th RFM22B device.
 * \param[in] rfm22b_id The RFM22B device index.
 * \return The unique device ID
 */
uint32_t PIOS_RFM22B_DeviceID(uint32_t rfm22b_id)
{
	struct pios_rfm22b_dev *rfm22b_dev = (struct pios_rfm22b_dev *)rfm22b_id;
	if(PIOS_RFM22B_validate(rfm22b_dev))
		return rfm22b_dev->deviceID;
	else
		return 0;
}

void PIOS_RFM22B_SetTxPower(uint32_t rfm22b_id, enum rfm22b_tx_power tx_pwr)
{
	struct pios_rfm22b_dev *rfm22b_dev = (struct pios_rfm22b_dev *)rfm22b_id;
	if(!PIOS_RFM22B_validate(rfm22b_dev))
		return;
	rfm22b_dev->tx_power = tx_pwr;
}

void PIOS_RFM22B_SetDestinationId(uint32_t rfm22b_id, uint32_t dest_id)
{
	struct pios_rfm22b_dev *rfm22b_dev = (struct pios_rfm22b_dev *)rfm22b_id;
	if(!PIOS_RFM22B_validate(rfm22b_dev))
		return;
	rfm22b_dev->destination_id = (dest_id == 0) ? 0xffffffff : dest_id;
}

uint16_t PIOS_RFM22B_Resets(uint32_t rfm22b_id)
{
	struct pios_rfm22b_dev *rfm22b_dev = (struct pios_rfm22b_dev *)rfm22b_id;
	if(!PIOS_RFM22B_validate(rfm22b_dev))
		return 0;
	return rfm22b_dev->resets;
}

uint16_t PIOS_RFM22B_Timeouts(uint32_t rfm22b_id)
{
	struct pios_rfm22b_dev *rfm22b_dev = (struct pios_rfm22b_dev *)rfm22b_id;
	if(!PIOS_RFM22B_validate(rfm22b_dev))
		return 0;
	return rfm22b_dev->timeouts;
}

uint8_t PIOS_RFM22B_LinkQuality(uint32_t rfm22b_id)
{
	struct pios_rfm22b_dev *rfm22b_dev = (struct pios_rfm22b_dev *)rfm22b_id;
	if(!PIOS_RFM22B_validate(rfm22b_dev))
		return 0;
	return rfm22b_dev->slow_link_quality;
}

int8_t PIOS_RFM22B_RSSI(uint32_t rfm22b_id)
{
	struct pios_rfm22b_dev *rfm22b_dev = (struct pios_rfm22b_dev *)rfm22b_id;
	if(!PIOS_RFM22B_validate(rfm22b_dev))
		return 0;
	return rfm22b_dev->rssi_dBm;
}

static void PIOS_RFM22B_RxStart(uint32_t rfm22b_id, uint16_t rx_bytes_avail)
{
	struct pios_rfm22b_dev * rfm22b_dev = (struct pios_rfm22b_dev *)rfm22b_id;

	bool valid = PIOS_RFM22B_validate(rfm22b_dev);
	PIOS_Assert(valid);

}

/**
 * Insert a packet on the packet queue for sending.
 * Note: If this finction succedds, the packet will be released by the driver, so no release is necessary.
 *       If this function doesn't success, the caller is still responsible for the packet.
 * \param[in] rfm22b_id  The rfm22b device.
 * \param[in] p  The packet handle.
 * \param[in] max_delay  The maximum time to delay waiting to queue the packet.
 * \return  true on success, false on failue to queue the packet.
 */
bool PIOS_RFM22B_Send_Packet(uint32_t rfm22b_id, PHPacketHandle p, uint32_t max_delay)
{
 	struct pios_rfm22b_dev *rfm22b_dev = (struct pios_rfm22b_dev *)rfm22b_id;
	if(!PIOS_RFM22B_validate(rfm22b_dev))
		return false;

	// Store the packet handle in the packet queue
	if (xQueueSend(rfm22b_dev->packetQueue, &p, max_delay) != pdTRUE)
		return false;
 
	// Inject a send packet event
	PIOS_RFM22B_InjectEvent(g_rfm22b_dev, RFM22B_EVENT_SEND_PACKET, false);

	// Success
	return true;
}

/**
 * Receive a packet from the radio.
 * \param[in] rfm22b_id  The rfm22b device.
 * \param[in] pret  A pointer to the packet handle.
 * \param[in] max_delay  The maximum time to delay waiting for a packet.
 * \return  The number of bytes received.
 */
uint32_t PIOS_RFM22B_Receive_Packet(uint32_t rfm22b_id, PHPacketHandle *pret, uint32_t max_delay)
{
	struct pios_rfm22b_dev * rfm22b_dev = (struct pios_rfm22b_dev *)rfm22b_id;
	if (!PIOS_RFM22B_validate(rfm22b_dev))
		return 0;

	// Allocate the next Rx packet
	if (rfm22b_dev->rx_packet_next == NULL)
		rfm22b_dev->rx_packet_next = PHGetRXPacket(pios_packet_handler);

	// Block on the semephore until the a packet has been received.
	if (xSemaphoreTake(rfm22b_dev->rxsem,  max_delay / portTICK_RATE_MS) != pdTRUE)
		return 0;

	// Return the Rx packet if it's available.
	uint32_t rx_len = 0;
	if (rfm22b_dev->rx_packet_prev)
	{
		PHPacketHandle p = rfm22b_dev->rx_packet_prev;
		uint16_t len = PHPacketSizeECC(p);

		// Attempt to correct any errors in the packet.
		decode_data((unsigned char*)p, len);

		// Check if there were any errors
		bool rx_error = check_syndrome() != 0;
		if(rx_error)
		{

			// We have an error.  Try to correct it.
			if (correct_errors_erasures((unsigned char*)p, len, 0, 0) == 0)
			{
				// We couldn't correct the error, so drop the packet.
				rfm22b_dev->fast_error_packets++;
				PHReleaseRXPacket(pios_packet_handler, p);
			}
			else
			{
				// We corrected the error.
				rfm22b_dev->fast_corrected_packets++;
				rx_error = false;
			}

		}

		// Return the packet if there were not uncorrectable errors.
		if (!rx_error)
		{
			rfm22b_dev->fast_good_packets++;
			*pret = p;
			rx_len = rfm22b_dev->rx_packet_len;

			// Update the link statistics if necessary.
			uint8_t fast_block = (p->header.seq_num >> 2) & 0xff;
			uint8_t slow_block = (p->header.seq_num >> 4) & 0xff;
			if (rfm22b_dev->fast_block != fast_block)
			{
				rfm22b_dev->fast_link_quality = (uint8_t)(((4 + (uint16_t)rfm22b_dev->fast_good_packets - rfm22b_dev->fast_error_packets) << 5) - 1);
				rfm22b_dev->slow_good_packets += rfm22b_dev->fast_good_packets;
				rfm22b_dev->slow_corrected_packets += rfm22b_dev->fast_corrected_packets;
				rfm22b_dev->slow_error_packets += rfm22b_dev->fast_error_packets;
				rfm22b_dev->fast_good_packets = rfm22b_dev->fast_corrected_packets = rfm22b_dev->fast_error_packets = 0;
				rfm22b_dev->fast_block = fast_block;
			}
			if (rfm22b_dev->slow_block != slow_block)
			{
				rfm22b_dev->slow_link_quality = (uint8_t)(((16 + (uint16_t)rfm22b_dev->slow_good_packets - rfm22b_dev->slow_error_packets) << 3) - 1);
				rfm22b_dev->slow_good_packets = rfm22b_dev->slow_corrected_packets = rfm22b_dev->slow_error_packets = 0;
				rfm22b_dev->slow_block = slow_block;
			}
		}
		rfm22b_dev->rx_packet_prev = NULL;
	}

	return rx_len;
}

/**
 * The task that controls the radio state machine.
 */
static void PIOS_RFM22B_Task(void *parameters)
{
	struct pios_rfm22b_dev *rfm22b_dev = (struct pios_rfm22b_dev *)parameters;
	if (!PIOS_RFM22B_validate(rfm22b_dev))
	    return;
	portTickType lastEventTicks = xTaskGetTickCount();
	portTickType lastStatusTicks = lastEventTicks;

	while(1)
	{
#ifdef PIOS_WDG_RFM22B
		// Update the watchdog timer
		PIOS_WDG_UpdateFlag(PIOS_WDG_RFM22B);
#endif /* PIOS_WDG_RFM22B */

		// Wait for a signal indicating an external interrupt or a pending send/receive request.
		if ( xSemaphoreTake(g_rfm22b_dev->isrPending,  ISR_TIMEOUT / portTICK_RATE_MS) == pdTRUE ) {
			lastEventTicks = xTaskGetTickCount();

			// Process events through the state machine.
			enum pios_rfm22b_event event;
			while (xQueueReceive(rfm22b_dev->eventQueue, &event, 0) == pdTRUE)
			{
				if ((event == RFM22B_EVENT_INT_RECEIVED) &&
				    ((rfm22b_dev->state == RFM22B_STATE_UNINITIALIZED) || (rfm22b_dev->state == RFM22B_STATE_INITIALIZING)))
					continue;

				// Process all state transitions.
				while(event != RFM22B_EVENT_NUM_EVENTS)
					event = rfm22_process_state_transition(rfm22b_dev, event);
			}
		}
		else
		{
			// Has it been too long since the last event?
			portTickType curTicks = xTaskGetTickCount();
			if (curTicks < lastEventTicks)
				lastEventTicks = curTicks;
			portTickType ticksSinceEvent = curTicks - lastEventTicks;
			if ((ticksSinceEvent / portTICK_RATE_MS) > PIOS_RFM22B_SUPERVISOR_TIMEOUT)
			{
 				// Transsition through an error event.
				enum pios_rfm22b_event event = RFM22B_EVENT_ERROR;
				while(event != RFM22B_EVENT_NUM_EVENTS)
					event = rfm22_process_state_transition(rfm22b_dev, event);

				// Clear the event queue.
				while (xQueueReceive(rfm22b_dev->eventQueue, &event, 0) == pdTRUE)
					;
				lastEventTicks = xTaskGetTickCount();
			}
		}

		// Have we locked up sending / receiving a packet?
		if (rfm22b_dev->packet_start_ticks > 0)
		{
			portTickType cur_ticks = xTaskGetTickCount();

			// Did the clock wrap around?
			if (cur_ticks < rfm22b_dev->packet_start_ticks)
				rfm22b_dev->packet_start_ticks = (cur_ticks > 0) ? cur_ticks : 1;

			// Have we been sending this packet too long?
			if (((cur_ticks - rfm22b_dev->packet_start_ticks) / portTICK_RATE_MS) > (rfm22b_dev->max_packet_time * 3))
			{
				enum pios_rfm22b_event event = RFM22B_EVENT_TIMEOUT;
				while(event != RFM22B_EVENT_NUM_EVENTS)
					event = rfm22_process_state_transition(rfm22b_dev, event);
			}
		}

		// Queue up a status packet if it's time.
		portTickType curTicks = xTaskGetTickCount();
		// Rollover
		if (curTicks < lastStatusTicks)
			lastStatusTicks = curTicks;
		if (((curTicks - lastStatusTicks) / portTICK_RATE_MS) > RADIOSTATS_UPDATE_PERIOD_MS)
			if (rfm22_sendStatus(rfm22b_dev))
				lastStatusTicks = curTicks;
	}
}

static void PIOS_RFM22B_TxStart(uint32_t rfm22b_id, uint16_t tx_bytes_avail)
{
	struct pios_rfm22b_dev * rfm22b_dev = (struct pios_rfm22b_dev *)rfm22b_id;
	bool valid = PIOS_RFM22B_validate(rfm22b_dev);
	PIOS_Assert(valid);

#ifdef NEVER
	// Get some data to send
	bool need_yield = false;
	if(tx_pre_buffer_size == 0)
		tx_pre_buffer_size = (rfm22b_dev->tx_out_cb)(rfm22b_dev->tx_out_context, tx_pre_buffer,
							     TX_BUFFER_SIZE, NULL, &need_yield);

	// Inject a send packet event
	PIOS_RFM22B_InjectEvent(g_rfm22b_dev, RFM22B_EVENT_TX_START, false);
#endif
}

/**
 * Changes the baud rate of the RFM22B peripheral without re-initialising.
 * \param[in] rfm22b_id RFM22B name (GPS, TELEM, AUX)
 * \param[in] baud Requested baud rate
 */
static void PIOS_RFM22B_ChangeBaud(uint32_t rfm22b_id, uint32_t baud)
{
	struct pios_rfm22b_dev * rfm22b_dev = (struct pios_rfm22b_dev *)rfm22b_id;

	bool valid = PIOS_RFM22B_validate(rfm22b_dev);
	PIOS_Assert(valid);

}

static void PIOS_RFM22B_RegisterRxCallback(uint32_t rfm22b_id, pios_com_callback rx_in_cb, uint32_t context)
{
	struct pios_rfm22b_dev * rfm22b_dev = (struct pios_rfm22b_dev *)rfm22b_id;

	bool valid = PIOS_RFM22B_validate(rfm22b_dev);
	PIOS_Assert(valid);

	/* 
	 * Order is important in these assignments since ISR uses _cb
	 * field to determine if it's ok to dereference _cb and _context
	 */
	rfm22b_dev->rx_in_context = context;
	rfm22b_dev->rx_in_cb = rx_in_cb;
}

static void PIOS_RFM22B_RegisterTxCallback(uint32_t rfm22b_id, pios_com_callback tx_out_cb, uint32_t context)
{
	struct pios_rfm22b_dev * rfm22b_dev = (struct pios_rfm22b_dev *)rfm22b_id;

	bool valid = PIOS_RFM22B_validate(rfm22b_dev);
	PIOS_Assert(valid);

	/* 
	 * Order is important in these assignments since ISR uses _cb
	 * field to determine if it's ok to dereference _cb and _context
	 */
	rfm22b_dev->tx_out_context = context;
	rfm22b_dev->tx_out_cb = tx_out_cb;
}

// ************************************
// SPI read/write

//! Assert the CS line
static void rfm22_assertCs()
{
	PIOS_DELAY_WaituS(1);
	if(PIOS_RFM22B_validate(g_rfm22b_dev) && g_rfm22b_dev->spi_id != 0)
		PIOS_SPI_RC_PinSet(g_rfm22b_dev->spi_id, g_rfm22b_dev->slave_num, 0);
}

//! Deassert the CS line
static void rfm22_deassertCs()
{
	if(PIOS_RFM22B_validate(g_rfm22b_dev) && g_rfm22b_dev->spi_id != 0)
		PIOS_SPI_RC_PinSet(g_rfm22b_dev->spi_id, g_rfm22b_dev->slave_num, 1);
}

//! Claim the SPI bus semaphore
static void rfm22_claimBus()
{
	if(PIOS_RFM22B_validate(g_rfm22b_dev) && g_rfm22b_dev->spi_id != 0)
		PIOS_SPI_ClaimBus(g_rfm22b_dev->spi_id);
}

//! Release the SPI bus semaphore
static void rfm22_releaseBus()
{
	if(PIOS_RFM22B_validate(g_rfm22b_dev) && g_rfm22b_dev->spi_id != 0)
		PIOS_SPI_ReleaseBus(g_rfm22b_dev->spi_id);
}

/**
 * Claim the semaphore and write a byte to a register
 * @param[in] addr The address to write to
 * @param[in] data The datat to write to that address
 */
static void rfm22_write(uint8_t addr, uint8_t data)
{
	if(PIOS_RFM22B_validate(g_rfm22b_dev)) {
		rfm22_claimBus();
		rfm22_assertCs();
		uint8_t buf[2] = {addr | 0x80, data};
		PIOS_SPI_TransferBlock(g_rfm22b_dev->spi_id, buf, NULL, sizeof(buf), NULL);
		rfm22_deassertCs();
		rfm22_releaseBus();
	}
}

/**
 * Write a byte to a register without claiming the bus.  Also
 * toggle the NSS line
 * @param[in] addr The address of the RFM22b register to write
 * @param[in] data The data to write to that register
static void rfm22_write_noclaim(uint8_t addr, uint8_t data)
{
	uint8_t buf[2] = {addr | 0x80, data};
	if(PIOS_RFM22B_validate(g_rfm22b_dev)) {
		rfm22_assertCs();
		PIOS_SPI_TransferBlock(g_rfm22b_dev->spi_id, buf, NULL, sizeof(buf), NULL);
		rfm22_deassertCs();
	}	
}
*/

/**

 * Read a byte from an RFM22b register
 * @param[in] addr The address to read from
 * @return Returns the result of the register read
 */
static uint8_t rfm22_read(uint8_t addr)
{
	uint8_t in[2];	
	uint8_t out[2] = {addr & 0x7f, 0xFF};
	if(PIOS_RFM22B_validate(g_rfm22b_dev)) {
		rfm22_claimBus();
		rfm22_assertCs();
		PIOS_SPI_TransferBlock(g_rfm22b_dev->spi_id, out, in, sizeof(out), NULL);
		rfm22_deassertCs();
		rfm22_releaseBus();
	}
	return in[1];
}

/**
 * Read a byte from an RFM22b register without claiming the bus
 * @param[in] addr The address to read from
 * @return Returns the result of the register read
 */
static uint8_t rfm22_read_noclaim(uint8_t addr)
{
	uint8_t out[2] = {addr & 0x7F, 0xFF};
	uint8_t in[2];
	if (PIOS_RFM22B_validate(g_rfm22b_dev)) {
		rfm22_assertCs();
		PIOS_SPI_TransferBlock(g_rfm22b_dev->spi_id, out, in, sizeof(out), NULL);
		rfm22_deassertCs();
	}
	return in[1];
}

// ************************************

static enum pios_rfm22b_event rfm22_process_state_transition(struct pios_rfm22b_dev *rfm22b_dev, enum pios_rfm22b_event event)
{

	// No event
	if (event == RFM22B_EVENT_NUM_EVENTS)
		return RFM22B_EVENT_NUM_EVENTS;

	// Don't transition if there is no transition defined
	enum pios_rfm22b_state next_state = rfm22b_transitions[rfm22b_dev->state].next_state[event];
	if (!next_state)
		return RFM22B_EVENT_NUM_EVENTS;

	/*
	 * Move to the next state
	 *
	 * This is done prior to calling the new state's entry function to 
	 * guarantee that the entry function never depends on the previous
	 * state.  This way, it cannot ever know what the previous state was.
	 */
	enum pios_rfm22b_state prev_state = rfm22b_dev->state;
	if (prev_state) ;

	rfm22b_dev->state = next_state;

	/* Call the entry function (if any) for the next state. */
	if (rfm22b_transitions[rfm22b_dev->state].entry_fn)
		return rfm22b_transitions[rfm22b_dev->state].entry_fn(rfm22b_dev);

	return RFM22B_EVENT_NUM_EVENTS;
}

// ************************************

static void rfm22_setNominalCarrierFrequency(struct pios_rfm22b_dev *rfm22b_dev, uint32_t frequency_hz)
{
	uint32_t min_frequency_hz = rfm22b_dev->cfg.minFrequencyHz;
	uint32_t max_frequency_hz = rfm22b_dev->cfg.maxFrequencyHz;
	if (frequency_hz < min_frequency_hz)
		frequency_hz = min_frequency_hz;
	else if (frequency_hz > max_frequency_hz)
		frequency_hz = max_frequency_hz;

	// holds the hbsel (1 or 2)
	uint8_t	hbsel;
	if (frequency_hz < 480000000)
		hbsel = 1;
	else
		hbsel = 2;
	uint8_t fb = (uint8_t)(frequency_hz / (10000000 * hbsel));

	uint32_t fc = (uint32_t)(frequency_hz - (10000000 * hbsel * fb));

	fc = (fc * 64u) / (10000ul * hbsel);
	fb -= 24;

	if (hbsel > 1)
		fb |= RFM22_fbs_hbsel;

	fb |= RFM22_fbs_sbse;	// is this the RX LO polarity?

	// frequency hopping channel (0-255)
	rfm22b_dev->frequency_step_size = 156.25f * hbsel;

	// frequency hopping channel (0-255)
	rfm22_write(RFM22_frequency_hopping_channel_select, rfm22b_dev->frequency_hop_channel);

	// no frequency offset
	rfm22_write(RFM22_frequency_offset1, 0);
	// no frequency offset
	rfm22_write(RFM22_frequency_offset2, 0);

	// set the carrier frequency
	rfm22_write(RFM22_frequency_band_select, fb);
	rfm22_write(RFM22_nominal_carrier_frequency1, fc >> 8);
	rfm22_write(RFM22_nominal_carrier_frequency0, fc & 0xff);
}

void rfm22_setFreqHopChannel(uint8_t channel)
{	// set the frequency hopping channel
	g_rfm22b_dev->frequency_hop_channel = channel;
	rfm22_write(RFM22_frequency_hopping_channel_select, channel);
}

uint32_t rfm22_freqHopSize(void)
{	// return the frequency hopping step size
	return ((uint32_t)g_rfm22b_dev->frequency_hop_step_size_reg * 10000);
}

// ************************************
// radio datarate about 19200 Baud
// radio frequency deviation 45kHz
// radio receiver bandwidth 67kHz.
//
// Carson's rule:
//  The signal bandwidth is about 2(Delta-f + fm) ..
//
// Delta-f = frequency deviation
// fm = maximum frequency of the signal
//
// This gives 2(45 + 9.6) = 109.2kHz.

void RFM22_SetDatarate(uint32_t rfm22b_id, enum rfm22b_datarate datarate, bool data_whitening)
{
	struct pios_rfm22b_dev * rfm22b_dev = (struct pios_rfm22b_dev *)rfm22b_id;
	if(!PIOS_RFM22B_validate(rfm22b_dev))
		return;

	uint32_t datarate_bps = data_rate[datarate];
	rfm22b_dev->datarate = datarate;
	rfm22b_dev->max_packet_time = (uint16_t)((float)(PIOS_PH_MAX_PACKET * 8 * 1000) / (float)(datarate_bps) + 0.5);

	// rfm22_if_filter_bandwidth
	rfm22_write(0x1C, reg_1C[datarate]);

	// rfm22_afc_loop_gearshift_override
	rfm22_write(0x1D, reg_1D[datarate]);
	// RFM22_afc_timing_control
	rfm22_write(0x1E, reg_1E[datarate]);

	// RFM22_clk_recovery_gearshift_override
	rfm22_write(0x1F, reg_1F[datarate]);
	// rfm22_clk_recovery_oversampling_ratio
	rfm22_write(0x20, reg_20[datarate]);
	// rfm22_clk_recovery_offset2
	rfm22_write(0x21, reg_21[datarate]);
	// rfm22_clk_recovery_offset1
	rfm22_write(0x22, reg_22[datarate]);
	// rfm22_clk_recovery_offset0
	rfm22_write(0x23, reg_23[datarate]);
	// rfm22_clk_recovery_timing_loop_gain1
	rfm22_write(0x24, reg_24[datarate]);
	// rfm22_clk_recovery_timing_loop_gain0
	rfm22_write(0x25, reg_25[datarate]);

	// rfm22_afc_limiter
	rfm22_write(0x2A, reg_2A[datarate]);

/* This breaks all bit rates < 100000!
	if (datarate_bps < 100000)
		// rfm22_chargepump_current_trimming_override
		rfm22_write(0x58, 0x80);
	else
		// rfm22_chargepump_current_trimming_override
		rfm22_write(0x58, 0xC0);
*/

	// rfm22_tx_data_rate1
	rfm22_write(0x6E, reg_6E[datarate]);
	// rfm22_tx_data_rate0
	rfm22_write(0x6F, reg_6F[datarate]);

	// Enable data whitening
	//	uint8_t txdtrtscale_bit = rfm22_read(RFM22_modulation_mode_control1) & RFM22_mmc1_txdtrtscale;
	//	rfm22_write(RFM22_modulation_mode_control1, txdtrtscale_bit | RFM22_mmc1_enwhite);

	if (!data_whitening)
		// rfm22_modulation_mode_control1
		rfm22_write(0x70, reg_70[datarate] & ~RFM22_mmc1_enwhite);
	else
		// rfm22_modulation_mode_control1
		rfm22_write(0x70, reg_70[datarate] |  RFM22_mmc1_enwhite);

	// rfm22_modulation_mode_control2
	rfm22_write(0x71, reg_71[datarate]);

	// rfm22_frequency_deviation
	rfm22_write(0x72, reg_72[datarate]);

	rfm22_write(RFM22_ook_counter_value1, 0x00);
	rfm22_write(RFM22_ook_counter_value2, 0x00);
}

// ************************************

static enum pios_rfm22b_event rfm22_setRxMode(struct pios_rfm22b_dev *rfm22b_dev)
{
	rfm22b_dev->packet_start_ticks = 0;

	// disable interrupts
	rfm22_write(RFM22_interrupt_enable1, 0x00);
	rfm22_write(RFM22_interrupt_enable2, 0x00);

	// Switch to TUNE mode
	rfm22_write(RFM22_op_and_func_ctrl1, RFM22_opfc1_pllon);

	RX_LED_OFF;
	TX_LED_OFF;

	// empty the rx buffer
	rfm22b_dev->rx_buffer_wr = 0;

	// Clear the TX buffer.
	rfm22b_dev->tx_data_rd = rfm22b_dev->tx_data_wr = 0;

	// clear FIFOs
	rfm22_write(RFM22_op_and_func_ctrl2, RFM22_opfc2_ffclrrx | RFM22_opfc2_ffclrtx);
	rfm22_write(RFM22_op_and_func_ctrl2, 0x00);

	// enable RX interrupts
	rfm22_write(RFM22_interrupt_enable1, RFM22_ie1_encrcerror | RFM22_ie1_enpkvalid |
		    RFM22_ie1_enrxffafull | RFM22_ie1_enfferr);
	rfm22_write(RFM22_interrupt_enable2, RFM22_ie2_enpreainval | RFM22_ie2_enpreaval |
		    RFM22_ie2_enswdet);

	// enable the receiver
	rfm22_write(RFM22_op_and_func_ctrl1, RFM22_opfc1_pllon | RFM22_opfc1_rxon);

	// No event generated
	return RFM22B_EVENT_NUM_EVENTS;
}

// ************************************

static enum pios_rfm22b_event rfm22_txStart(struct pios_rfm22b_dev *rfm22b_dev)
{
	// See if there's a packet on the packet queue.
	PHPacketHandle p;
	if (xQueueReceive(rfm22b_dev->packetQueue, &p, 0) != pdTRUE)
	{
		// Clear the TX buffer.
		rfm22b_dev->tx_data_rd = rfm22b_dev->tx_data_wr = 0;
		return RFM22B_EVENT_RX_MODE;
	}

	// Add the error correcting code.
	p->header.source_id = rfm22b_dev->deviceID;
	p->header.seq_num = rfm22b_dev->tx_packet_count++;
	encode_data((unsigned char*)p, PHPacketSize(p), (unsigned char*)p);

	rfm22b_dev->tx_packet = p;
	rfm22b_dev->packet_start_ticks = xTaskGetTickCount();
	if (rfm22b_dev->packet_start_ticks == 0)
		rfm22b_dev->packet_start_ticks = 1;

	// disable interrupts
	rfm22_write(RFM22_interrupt_enable1, 0x00);
	rfm22_write(RFM22_interrupt_enable2, 0x00);

	// TUNE mode
	rfm22_write(RFM22_op_and_func_ctrl1, RFM22_opfc1_pllon);

	// Queue the data up for sending
	rfm22b_dev->tx_data_wr = PH_PACKET_SIZE(rfm22b_dev->tx_packet);

	RX_LED_OFF;

	// Set the destination address in the transmit header.
	// The destination address is the first 4 bytes of the message.
	uint8_t *tx_buffer = (uint8_t*)(rfm22b_dev->tx_packet);
	rfm22_write(RFM22_transmit_header0, tx_buffer[0]);
	rfm22_write(RFM22_transmit_header1, tx_buffer[1]);
	rfm22_write(RFM22_transmit_header2, tx_buffer[2]);
	rfm22_write(RFM22_transmit_header3, tx_buffer[3]);

	// FIFO mode, GFSK modulation
	uint8_t fd_bit = rfm22_read(RFM22_modulation_mode_control2) & RFM22_mmc2_fd;
	rfm22_write(RFM22_modulation_mode_control2, fd_bit | RFM22_mmc2_dtmod_fifo |
		    RFM22_mmc2_modtyp_gfsk);

	// set the tx power
	rfm22_write(RFM22_tx_power, RFM22_tx_pwr_papeaken | RFM22_tx_pwr_papeaklvl_1 |
		    RFM22_tx_pwr_papeaklvl_0 | RFM22_tx_pwr_lna_sw | g_rfm22b_dev->tx_power);

	// clear FIFOs
	rfm22_write(RFM22_op_and_func_ctrl2, RFM22_opfc2_ffclrrx | RFM22_opfc2_ffclrtx);
	rfm22_write(RFM22_op_and_func_ctrl2, 0x00);

	// *******************
	// add some data to the chips TX FIFO before enabling the transmitter

	// set the total number of data bytes we are going to transmit
	rfm22_write(RFM22_transmit_packet_length, rfm22b_dev->tx_data_wr);

	// add some data
	rfm22_claimBus();
	rfm22_assertCs();
	PIOS_SPI_TransferByte(g_rfm22b_dev->spi_id, RFM22_fifo_access | 0x80);
	int bytes_to_write = (rfm22b_dev->tx_data_wr - rfm22b_dev->tx_data_rd);
	bytes_to_write = (bytes_to_write > FIFO_SIZE) ? FIFO_SIZE:  bytes_to_write;
	PIOS_SPI_TransferBlock(g_rfm22b_dev->spi_id, &tx_buffer[rfm22b_dev->tx_data_rd], NULL, bytes_to_write, NULL);
	rfm22b_dev->tx_data_rd += bytes_to_write;
	rfm22_deassertCs();
	rfm22_releaseBus();

	// enable TX interrupts
	rfm22_write(RFM22_interrupt_enable1, RFM22_ie1_enpksent | RFM22_ie1_entxffaem);

	// enable the transmitter
	rfm22_write(RFM22_op_and_func_ctrl1, RFM22_opfc1_pllon | RFM22_opfc1_txon);

	TX_LED_ON;

	return RFM22B_EVENT_TX_STARTED;
}

static bool rfm22_sendStatus(struct pios_rfm22b_dev *rfm22b_dev)
{
	PHPacketHandle sph = (PHPacketHandle)&(rfm22b_dev->status_packet);

	// Queue the status message
	rfm22b_dev->status_packet.header.destination_id = 0xffffffff; // Broadcast
	rfm22b_dev->status_packet.header.type = PACKET_TYPE_STATUS;
	rfm22b_dev->status_packet.header.data_size = PH_STATUS_DATA_SIZE(&(rfm22b_dev->status_packet));
	rfm22b_dev->status_packet.errors = rfm22b_dev->errors;
	rfm22b_dev->status_packet.resets = rfm22b_dev->resets;
	rfm22b_dev->status_packet.retries = 0;
	rfm22b_dev->status_packet.uavtalk_errors = 0;
	rfm22b_dev->status_packet.dropped = 0;
	if (xQueueSend(rfm22b_dev->packetQueue, &sph, 0) != pdTRUE)
		return false;

	// Process a SEND_PACKT event.
	enum pios_rfm22b_event event = RFM22B_EVENT_SEND_PACKET;
	while(event != RFM22B_EVENT_NUM_EVENTS)
		event = rfm22_process_state_transition(rfm22b_dev, event);

	return true;
}

// ************************************

/**
 * Read the RFM22B interrupt and device status registers
 * \param[in] rfm22b_dev  The device structure
 */
static bool rfm22_readStatus(struct pios_rfm22b_dev *rfm22b_dev)
{

	// 1. Read the interrupt statuses with burst read
	rfm22_claimBus();  // Set RC and the semaphore
	uint8_t write_buf[3] = {RFM22_interrupt_status1 & 0x7f, 0xFF, 0xFF};
	uint8_t read_buf[3];
	rfm22_assertCs();
	PIOS_SPI_TransferBlock(g_rfm22b_dev->spi_id, write_buf, read_buf, sizeof(write_buf), NULL);
	rfm22_deassertCs();
	rfm22b_dev->int_status1 = read_buf[1];
	rfm22b_dev->int_status2 = read_buf[2];
	
	// Device status
	rfm22b_dev->device_status = rfm22_read_noclaim(RFM22_device_status);

	// EzMAC status
	rfm22b_dev->ezmac_status = rfm22_read_noclaim(RFM22_ezmac_status);

	// Release the bus
	rfm22_releaseBus();

	// the RF module has gone and done a reset - we need to re-initialize the rf module
	if (rfm22b_dev->int_status2 & RFM22_is2_ipor)
		return false;

	return true;
}

static enum pios_rfm22b_event rfm22_detectPreamble(struct pios_rfm22b_dev *rfm22b_dev)
{

	// Read the device status registers
	if (!rfm22_readStatus(rfm22b_dev))
		return RFM22B_EVENT_ERROR;

	// Valid preamble detected
	if (rfm22b_dev->int_status2 & RFM22_is2_ipreaval)
	{
		rfm22b_dev->packet_start_ticks = xTaskGetTickCount();
		if (rfm22b_dev->packet_start_ticks == 0)
			rfm22b_dev->packet_start_ticks = 1;
		RX_LED_ON;
		return RFM22B_EVENT_PREAMBLE_DETECTED;
	}

	return RFM22B_EVENT_NUM_EVENTS;
}

static enum pios_rfm22b_event rfm22_detectSync(struct pios_rfm22b_dev *rfm22b_dev)
{

	// Read the device status registers
	if (!rfm22_readStatus(rfm22b_dev))
		return RFM22B_EVENT_ERROR;

	// Sync word detected
	if (rfm22b_dev->int_status2 & RFM22_is2_iswdet)
	{
		RX_LED_ON;

		// read the 10-bit signed afc correction value
		// bits 9 to 2
		uint16_t afc_correction = (uint16_t)rfm22_read(RFM22_afc_correction_read) << 8;
		// bits 1 & 0
		afc_correction |= (uint16_t)rfm22_read(RFM22_ook_counter_value1) & 0x00c0;
		afc_correction >>= 6;
		// convert the afc value to Hz
		rfm22b_dev->afc_correction_Hz = (int32_t)(rfm22b_dev->frequency_step_size * afc_correction + 0.5f);

		// read rx signal strength .. 45 = -100dBm, 205 = -20dBm
		rfm22b_dev->rssi = rfm22_read(RFM22_rssi);
		// convert to dBm
		rfm22b_dev->rssi_dBm = (int8_t)(rfm22b_dev->rssi >> 1) - 122;

		// remember the afc value for this packet
		rfm22b_dev->rx_packet_start_afc_Hz = rfm22b_dev->afc_correction_Hz;

		return RFM22B_EVENT_SYNC_DETECTED;
	}
	else if (rfm22b_dev->int_status2 & !RFM22_is2_ipreaval)
	{
		// Waiting for sync timed out.
		return RFM22B_EVENT_TX_START;
	}

	return RFM22B_EVENT_NUM_EVENTS;
}

static enum pios_rfm22b_event rfm22_rxData(struct pios_rfm22b_dev *rfm22b_dev)
{
	// Swap in the next packet buffer if required.
	if (rfm22b_dev->rx_packet == NULL)
	{
		if (rfm22b_dev->rx_packet_next != NULL)
			rfm22b_dev->rx_packet = rfm22b_dev->rx_packet_next;
		else
			return RFM22B_EVENT_ERROR;
	}
	uint8_t *rx_buffer = (uint8_t*)(rfm22b_dev->rx_packet);

	// Read the device status registers
	if (!rfm22_readStatus(rfm22b_dev))
		return RFM22B_EVENT_ERROR;

	// FIFO under/over flow error.  Restart RX mode.
	if (rfm22b_dev->device_status & (RFM22_ds_ffunfl | RFM22_ds_ffovfl))
		return RFM22B_EVENT_ERROR;

	// RX FIFO almost full, it needs emptying
	if (rfm22b_dev->int_status1 & RFM22_is1_irxffafull)
	{
		// read data from the rf chips FIFO buffer
		// read the total length of the packet data
		uint16_t len = rfm22_read(RFM22_received_packet_length);

		// The received packet is going to be larger than the specified length
		if ((rfm22b_dev->rx_buffer_wr + RX_FIFO_HI_WATERMARK) > len)
			return RFM22B_EVENT_ERROR;

		// Another packet length error.
		if (((rfm22b_dev->rx_buffer_wr + RX_FIFO_HI_WATERMARK) >= len) && !(rfm22b_dev->int_status1 & RFM22_is1_ipkvalid))
			return RFM22B_EVENT_ERROR;

		// Fetch the data from the RX FIFO
		rfm22_claimBus();
		rfm22_assertCs();
		PIOS_SPI_TransferByte(rfm22b_dev->spi_id,RFM22_fifo_access & 0x7F);
		rfm22b_dev->rx_buffer_wr += (PIOS_SPI_TransferBlock(rfm22b_dev->spi_id ,OUT_FF, (uint8_t *)&rx_buffer[rfm22b_dev->rx_buffer_wr], RX_FIFO_HI_WATERMARK, NULL) == 0) ? RX_FIFO_HI_WATERMARK : 0;
		rfm22_deassertCs();
		rfm22_releaseBus();
	}

	// CRC error .. discard the received data
	if (rfm22b_dev->int_status1 & RFM22_is1_icrerror)
		return RFM22B_EVENT_ERROR;

	// Valid packet received
	if (rfm22b_dev->int_status1 & RFM22_is1_ipkvalid)
	{

		// read the total length of the packet data
		uint32_t len = rfm22_read(RFM22_received_packet_length);

		// their must still be data in the RX FIFO we need to get
		if (rfm22b_dev->rx_buffer_wr < len)
		{
			int32_t bytes_to_read = len - rfm22b_dev->rx_buffer_wr;
			// Fetch the data from the RX FIFO
			rfm22_claimBus();
			rfm22_assertCs();
			PIOS_SPI_TransferByte(rfm22b_dev->spi_id,RFM22_fifo_access & 0x7F);
			rfm22b_dev->rx_buffer_wr += (PIOS_SPI_TransferBlock(rfm22b_dev->spi_id,OUT_FF, (uint8_t *)&rx_buffer[rfm22b_dev->rx_buffer_wr], bytes_to_read, NULL) == 0) ? bytes_to_read : 0;
			rfm22_deassertCs();
			rfm22_releaseBus();
		}
	
		if (rfm22b_dev->rx_buffer_wr != len)
			return RFM22B_EVENT_ERROR;

		// we have a valid received packet

		if (rfm22b_dev->rx_buffer_wr > 0)
		{
			// Add the rssi and afc to the end of the packet.
			rx_buffer[rfm22b_dev->rx_buffer_wr++] = rfm22b_dev->rssi_dBm;
			rx_buffer[rfm22b_dev->rx_buffer_wr++] = rfm22b_dev->rx_packet_start_afc_Hz;
			// Swap the Rx packets.
			if (rfm22b_dev->rx_packet_prev == NULL)
			{
				rfm22b_dev->rx_packet_prev = rfm22b_dev->rx_packet;
				rfm22b_dev->rx_packet = rfm22b_dev->rx_packet_next;
				rfm22b_dev->rx_packet_len = rfm22b_dev->rx_buffer_wr;
				// Signal the receive thread.
				xSemaphoreGive(rfm22b_dev->rxsem);
			}
			rfm22b_dev->rx_buffer_wr = 0;
		}

		// Start a new transaction
		rfm22b_dev->packet_start_ticks = 0;
		return RFM22B_EVENT_RX_COMPLETE;
	}

	return RFM22B_EVENT_NUM_EVENTS;
}

static enum pios_rfm22b_event rfm22_txData(struct pios_rfm22b_dev *rfm22b_dev)
{

	// Read the device status registers
	if (!rfm22_readStatus(rfm22b_dev))
	{
		// Free the tx packet
		PHReleaseTXPacket(pios_packet_handler, rfm22b_dev->tx_packet);
		rfm22b_dev->tx_packet = 0;
		rfm22b_dev->tx_data_wr = rfm22b_dev->tx_data_rd = 0;
		return RFM22B_EVENT_ERROR;
	}

	// FIFO under/over flow error.  Back to RX mode.
	if (rfm22b_dev->device_status & (RFM22_ds_ffunfl | RFM22_ds_ffovfl))
	{
		// Free the tx packet
		PHReleaseTXPacket(pios_packet_handler, rfm22b_dev->tx_packet);
		rfm22b_dev->tx_packet = 0;
		rfm22b_dev->tx_data_wr = rfm22b_dev->tx_data_rd = 0;
		return RFM22B_EVENT_ERROR;
	}

	// TX FIFO almost empty, it needs filling up
	if (rfm22b_dev->int_status1 & RFM22_is1_ixtffaem)
	{
		// top-up the rf chips TX FIFO buffer
		uint8_t *tx_buffer = (uint8_t*)(rfm22b_dev->tx_packet);
		uint16_t max_bytes = FIFO_SIZE - TX_FIFO_LO_WATERMARK - 1;
		rfm22_claimBus();
		rfm22_assertCs();
		PIOS_SPI_TransferByte(g_rfm22b_dev->spi_id, RFM22_fifo_access | 0x80);
		int bytes_to_write = (rfm22b_dev->tx_data_wr - rfm22b_dev->tx_data_rd);
		bytes_to_write = (bytes_to_write > max_bytes) ? max_bytes:  bytes_to_write;
		PIOS_SPI_TransferBlock(g_rfm22b_dev->spi_id, &tx_buffer[rfm22b_dev->tx_data_rd], NULL, bytes_to_write, NULL);
		rfm22b_dev->tx_data_rd += bytes_to_write;
		rfm22_deassertCs();
		rfm22_releaseBus();
	}

	// Packet has been sent
	else if (rfm22b_dev->int_status1 & RFM22_is1_ipksent)
	{
		// Free the tx packet
		PHReleaseTXPacket(pios_packet_handler, rfm22b_dev->tx_packet);
		rfm22b_dev->tx_packet = 0;
		rfm22b_dev->tx_data_wr = rfm22b_dev->tx_data_rd = 0;
		// Start a new transaction
		rfm22b_dev->packet_start_ticks = 0;
		return RFM22B_EVENT_TX_COMPLETE;
	}

	return RFM22B_EVENT_NUM_EVENTS;
}

// ************************************

// return the current mode
int8_t rfm22_currentMode(void)
{
	return g_rfm22b_dev->state;
}

// return true if we are transmitting
bool rfm22_transmitting(void)
{
	return (g_rfm22b_dev->state == RFM22B_STATE_TX_DATA);
}

// return true if the channel is clear to transmit on
bool rfm22_channelIsClear(void)
{
	if (g_rfm22b_dev->state != RFM22B_STATE_RX_MODE && g_rfm22b_dev->state != RFM22B_STATE_WAIT_PREAMBLE && g_rfm22b_dev->state != RFM22B_STATE_WAIT_SYNC)
		// we are receiving something or we are transmitting or we are scanning the spectrum
		return false;

	return true;
}

// ************************************
// set/get the frequency calibration value

void rfm22_setFreqCalibration(uint8_t value)
{
	rfm22_write(RFM22_xtal_osc_load_cap, value);
}

// ************************************
// Initialise this hardware layer module and the rf module

static enum pios_rfm22b_event rfm22_init(struct pios_rfm22b_dev *rfm22b_dev)
{
	uint32_t id = rfm22b_dev->deviceID;
	uint32_t min_frequency_hz = rfm22b_dev->cfg.minFrequencyHz;
	uint32_t max_frequency_hz = rfm22b_dev->cfg.maxFrequencyHz;
	uint32_t freq_hop_step_size = 50000;

	// software reset the RF chip .. following procedure according to Si4x3x Errata (rev. B)
	rfm22_write(RFM22_op_and_func_ctrl1, RFM22_opfc1_swres);

	// wait 26ms
	PIOS_DELAY_WaitmS(26);

	for (int i = 50; i > 0; i--)
	{
		// wait 1ms
		PIOS_DELAY_WaitmS(1);

		// read the status registers
		rfm22b_dev->int_status1 = rfm22_read(RFM22_interrupt_status1);
		rfm22b_dev->int_status2 = rfm22_read(RFM22_interrupt_status2);
		if (rfm22b_dev->int_status2 & RFM22_is2_ichiprdy) break;
	}

	// ****************

	// read status - clears interrupt
	rfm22b_dev->device_status = rfm22_read(RFM22_device_status);
	rfm22b_dev->int_status1 = rfm22_read(RFM22_interrupt_status1);
	rfm22b_dev->int_status2 = rfm22_read(RFM22_interrupt_status2);
	rfm22b_dev->ezmac_status = rfm22_read(RFM22_ezmac_status);

	// disable all interrupts
	rfm22_write(RFM22_interrupt_enable1, 0x00);
	rfm22_write(RFM22_interrupt_enable2, 0x00);

	// ****************

	rfm22b_dev->device_status = rfm22b_dev->int_status1 = rfm22b_dev->int_status2 = rfm22b_dev->ezmac_status = 0;

	rfm22b_dev->rx_buffer_wr = 0;

	rfm22b_dev->tx_data_rd = rfm22b_dev->tx_data_wr = 0;

	rfm22b_dev->frequency_hop_channel = 0;

	rfm22b_dev->afc_correction_Hz = 0;

	rfm22b_dev->packet_start_ticks = 0;

	// ****************
	// read the RF chip ID bytes

	// read the device type
	uint8_t device_type = rfm22_read(RFM22_DEVICE_TYPE) & RFM22_DT_MASK;
	// read the device version
	uint8_t device_version = rfm22_read(RFM22_DEVICE_VERSION) & RFM22_DV_MASK;

#if defined(RFM22_DEBUG)
	DEBUG_PRINTF(2, "rf device type: %d\n\r", device_type);
	DEBUG_PRINTF(2, "rf device version: %d\n\r", device_version);
#endif

	if (device_type != 0x08)
	{
#if defined(RFM22_DEBUG)
		DEBUG_PRINTF(2, "rf device type: INCORRECT - should be 0x08\n\r");
#endif
		// incorrect RF module type
		return RFM22B_EVENT_FATAL_ERROR;
	}
	if (device_version != RFM22_DEVICE_VERSION_B1)
	{
#if defined(RFM22_DEBUG)
		DEBUG_PRINTF(2, "rf device version: INCORRECT\n\r");
#endif
		// incorrect RF module version
		return RFM22B_EVENT_FATAL_ERROR;
	}

	// ****************
	// set the minimum and maximum carrier frequency allowed

	if (min_frequency_hz < RFM22_MIN_CARRIER_FREQUENCY_HZ) min_frequency_hz = RFM22_MIN_CARRIER_FREQUENCY_HZ;
	else
		if (min_frequency_hz > RFM22_MAX_CARRIER_FREQUENCY_HZ) min_frequency_hz = RFM22_MAX_CARRIER_FREQUENCY_HZ;

	if (max_frequency_hz < RFM22_MIN_CARRIER_FREQUENCY_HZ) max_frequency_hz = RFM22_MIN_CARRIER_FREQUENCY_HZ;
	else
		if (max_frequency_hz > RFM22_MAX_CARRIER_FREQUENCY_HZ) max_frequency_hz = RFM22_MAX_CARRIER_FREQUENCY_HZ;

	if (min_frequency_hz > max_frequency_hz)
	{	// swap them over
		uint32_t tmp = min_frequency_hz;
		min_frequency_hz = max_frequency_hz;
		max_frequency_hz = tmp;
	}

	// ****************
	// calibrate our RF module to be exactly on frequency .. different for every module
	rfm22_write(RFM22_xtal_osc_load_cap, OSC_LOAD_CAP);

	// ****************

	// disable Low Duty Cycle Mode
	rfm22_write(RFM22_op_and_func_ctrl2, 0x00);

	// 1MHz clock output
	rfm22_write(RFM22_cpu_output_clk, RFM22_coc_1MHz);

	// READY mode
	rfm22_write(RFM22_op_and_func_ctrl1, RFM22_opfc1_xton);

	// choose the 3 GPIO pin functions
	// GPIO port use default value
	rfm22_write(RFM22_io_port_config, RFM22_io_port_default);
	if (rfm22b_dev->cfg.gpio_direction == GPIO0_TX_GPIO1_RX) {
		rfm22_write(RFM22_gpio0_config, RFM22_gpio0_config_drv3 | RFM22_gpio0_config_txstate);	// GPIO0 = TX State (to control RF Switch)
		rfm22_write(RFM22_gpio1_config, RFM22_gpio1_config_drv3 | RFM22_gpio1_config_rxstate);	// GPIO1 = RX State (to control RF Switch)
	} else {
		rfm22_write(RFM22_gpio0_config, RFM22_gpio0_config_drv3 | RFM22_gpio0_config_rxstate);	// GPIO0 = TX State (to control RF Switch)
		rfm22_write(RFM22_gpio1_config, RFM22_gpio1_config_drv3 | RFM22_gpio1_config_txstate);	// GPIO1 = RX State (to control RF Switch)		
	}
	rfm22_write(RFM22_gpio2_config, RFM22_gpio2_config_drv3 | RFM22_gpio2_config_cca);		// GPIO2 = Clear Channel Assessment

	// ****************

	// initialize the frequency hopping step size
	freq_hop_step_size /= 10000;	// in 10kHz increments
	if (freq_hop_step_size > 255) freq_hop_step_size = 255;
	rfm22b_dev->frequency_hop_step_size_reg = freq_hop_step_size;

	// FIFO mode, GFSK modulation
	uint8_t fd_bit = rfm22_read(RFM22_modulation_mode_control2) & RFM22_mmc2_fd;
	rfm22_write(RFM22_modulation_mode_control2, RFM22_mmc2_trclk_clk_none | RFM22_mmc2_dtmod_fifo | fd_bit | RFM22_mmc2_modtyp_gfsk);

	// setup to read the internal temperature sensor

	// ADC used to sample the temperature sensor
	uint8_t adc_config = RFM22_ac_adcsel_temp_sensor | RFM22_ac_adcref_bg;
	rfm22_write(RFM22_adc_config, adc_config);

	// adc offset
	rfm22_write(RFM22_adc_sensor_amp_offset, 0);

	// temp sensor calibration .. �40C to +64C 0.5C resolution
	rfm22_write(RFM22_temp_sensor_calib, RFM22_tsc_tsrange0 | RFM22_tsc_entsoffs);

	// temp sensor offset
	rfm22_write(RFM22_temp_value_offset, 0);

	// start an ADC conversion
	rfm22_write(RFM22_adc_config, adc_config | RFM22_ac_adcstartbusy);

	// set the RSSI threshold interrupt to about -90dBm
	rfm22_write(RFM22_rssi_threshold_clear_chan_indicator, (-90 + 122) * 2);

	// enable the internal Tx & Rx packet handlers (without CRC)
	rfm22_write(RFM22_data_access_control, RFM22_dac_enpacrx | RFM22_dac_enpactx);

	// x-nibbles tx preamble
	rfm22_write(RFM22_preamble_length, TX_PREAMBLE_NIBBLES);
	// x-nibbles rx preamble detection
	rfm22_write(RFM22_preamble_detection_ctrl1, RX_PREAMBLE_NIBBLES << 3);

#ifdef PIOS_RFM22_NO_HEADER
	// header control - we are not using the header
	rfm22_write(RFM22_header_control1, RFM22_header_cntl1_bcen_none | RFM22_header_cntl1_hdch_none);

	// no header bytes, synchronization word length 3, 2, 1 & 0 used, packet length included in header.
	rfm22_write(RFM22_header_control2, RFM22_header_cntl2_hdlen_none |
							RFM22_header_cntl2_synclen_3210 | ((TX_PREAMBLE_NIBBLES >> 8) & 0x01));
#else
	// header control - using a 4 by header with broadcast of 0xffffffff
	rfm22_write(RFM22_header_control1,
							RFM22_header_cntl1_bcen_0 |
							RFM22_header_cntl1_bcen_1 |
							RFM22_header_cntl1_bcen_2 |
							RFM22_header_cntl1_bcen_3 |
							RFM22_header_cntl1_hdch_0 |
							RFM22_header_cntl1_hdch_1 |
							RFM22_header_cntl1_hdch_2 |
							RFM22_header_cntl1_hdch_3);
	// Check all bit of all bytes of the header
	rfm22_write(RFM22_header_enable0, 0xff);
	rfm22_write(RFM22_header_enable1, 0xff);
	rfm22_write(RFM22_header_enable2, 0xff);
	rfm22_write(RFM22_header_enable3, 0xff);
	// Set the ID to be checked
	rfm22_write(RFM22_check_header0, id & 0xff);
	rfm22_write(RFM22_check_header1, (id >> 8) & 0xff);
	rfm22_write(RFM22_check_header2, (id >> 16) & 0xff);
	rfm22_write(RFM22_check_header3, (id >> 24) & 0xff);
	// 4 header bytes, synchronization word length 3, 2, 1 & 0 used, packet length included in header.
	rfm22_write(RFM22_header_control2,
							RFM22_header_cntl2_hdlen_3210 |
							RFM22_header_cntl2_synclen_3210 |
							((TX_PREAMBLE_NIBBLES >> 8) & 0x01));
#endif

	// sync word
	rfm22_write(RFM22_sync_word3, SYNC_BYTE_1);
	rfm22_write(RFM22_sync_word2, SYNC_BYTE_2);
	rfm22_write(RFM22_sync_word1, SYNC_BYTE_3);
	rfm22_write(RFM22_sync_word0, SYNC_BYTE_4);

	rfm22_write(RFM22_agc_override1, RFM22_agc_ovr1_agcen);

	// set frequency hopping channel step size (multiples of 10kHz)
	rfm22_write(RFM22_frequency_hopping_step_size, rfm22b_dev->frequency_hop_step_size_reg);

	// set our nominal carrier frequency
	rfm22_setNominalCarrierFrequency(rfm22b_dev, (min_frequency_hz + max_frequency_hz) / 2);

	// set the tx power
	rfm22_write(RFM22_tx_power, RFM22_tx_pwr_papeaken | RFM22_tx_pwr_papeaklvl_0 | RFM22_tx_pwr_lna_sw | rfm22b_dev->tx_power);

	// TX FIFO Almost Full Threshold (0 - 63)
	rfm22_write(RFM22_tx_fifo_control1, TX_FIFO_HI_WATERMARK);

	// TX FIFO Almost Empty Threshold (0 - 63)
	rfm22_write(RFM22_tx_fifo_control2, TX_FIFO_LO_WATERMARK);

	// RX FIFO Almost Full Threshold (0 - 63)
	rfm22_write(RFM22_rx_fifo_control, RX_FIFO_HI_WATERMARK);

	rfm22_setFreqCalibration(rfm22b_dev->cfg.RFXtalCap);
	rfm22_setNominalCarrierFrequency(rfm22b_dev, rfm22b_dev->cfg.frequencyHz);
	RFM22_SetDatarate((uint32_t)rfm22b_dev, rfm22b_dev->datarate, true);

	return RFM22B_EVENT_INITIALIZED;
}

static enum pios_rfm22b_event rfm22_timeout(struct pios_rfm22b_dev *rfm22b_dev)
{
	rfm22b_dev->resets++;
	rfm22b_dev->packet_start_ticks = 0;
	return RFM22B_EVENT_TX_START;
}

static enum pios_rfm22b_event rfm22_error(struct pios_rfm22b_dev *rfm22b_dev)
{
	rfm22b_dev->resets++;
	rfm22b_dev->packet_start_ticks = 0;
	return RFM22B_EVENT_INITIALIZE;
}

/**
 * A fatal error has occured in the state machine.
 * this should not happen.
 * \parem [in] rfm22b_dev  The device structure
 * \return enum pios_rfm22b_event  The next event to inject
 */
static enum pios_rfm22b_event rfm22_fatal_error(struct pios_rfm22b_dev *rfm22b_dev)
{

	// RF module error .. flash the LED's
	for(unsigned int j = 0; j < 16; j++)
	{
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

	return RFM22B_EVENT_FATAL_ERROR;
}

// ************************************

#endif

/**
 * @}
 * @}
 */

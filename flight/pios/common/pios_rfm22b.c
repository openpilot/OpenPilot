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
#include <packet_handler.h>
#if defined(PIOS_INCLUDE_GCSRCVR)
#include <gcsreceiver.h>
#endif
#include <pios_rfm22b_priv.h>
#include <pios_ppm_out_priv.h>
#include <ecc.h>

/* Local Defines */
#define STACK_SIZE_BYTES 200
#define TASK_PRIORITY (tskIDLE_PRIORITY + 2)
#define ISR_TIMEOUT 2 // ms
#define EVENT_QUEUE_SIZE 5
#define RFM22B_DEFAULT_RX_DATARATE RFM22_datarate_9600
#define RFM22B_DEFAULT_TX_POWER RFM22_tx_pwr_txpow_0
#define RFM22B_LINK_QUALITY_THRESHOLD 20
#define RFM22B_NOMINAL_CARRIER_FREQUENCY 430000000
#define RFM22B_MAXIMUM_FREQUENCY 440000000
#define RFM22B_DEFAULT_FREQUENCY 433000000
#define RFM22B_FREQUENCY_HOP_STEP_SIZE 75000
//#define RFM22B_TEST_DROPPED_PACKETS 1

// The maximum amount of time since the last message received to consider the connection broken.
#define DISCONNECT_TIMEOUT_MS 1000 // ms

// The maximum amount of time without activity before initiating a reset.
#define PIOS_RFM22B_SUPERVISOR_TIMEOUT 100  // ms

// The time between connection attempts when not connected
#define CONNECT_ATTEMPT_PERIOD_MS 250 // ms

// The time between updates for sending stats the radio link.
#define RADIOSTATS_UPDATE_PERIOD_MS 250

// The number of stats updates that a modem can miss before it's considered disconnected
#define MAX_RADIOSTATS_MISS_COUNT 3

// The time between PPM updates
#define PPM_UPDATE_PERIOD_MS 20

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

struct pios_rfm22b_transition {
	enum pios_rfm22b_event (*entry_fn) (struct pios_rfm22b_dev *rfm22b_dev);
	enum pios_rfm22b_state next_state[RFM22B_EVENT_NUM_EVENTS];
};

// Must ensure these prefilled arrays match the define sizes
static const uint8_t FULL_PREAMBLE[FIFO_SIZE] = {
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
	PREAMBLE_BYTE,PREAMBLE_BYTE,PREAMBLE_BYTE,PREAMBLE_BYTE,PREAMBLE_BYTE,
	PREAMBLE_BYTE,PREAMBLE_BYTE,PREAMBLE_BYTE,PREAMBLE_BYTE}; // 64 bytes
static const uint8_t HEADER[(TX_PREAMBLE_NIBBLES + 1)/2 + 2] = {PREAMBLE_BYTE, PREAMBLE_BYTE, PREAMBLE_BYTE, PREAMBLE_BYTE,PREAMBLE_BYTE, PREAMBLE_BYTE, SYNC_BYTE_1, SYNC_BYTE_2};
static const uint8_t OUT_FF[64] = {
	0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
	0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
	0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
	0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
	0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
	0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
	0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
	0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};

/* Local function forwared declarations */
static void PIOS_RFM22B_Task(void *parameters);
static bool rfm22_readStatus(struct pios_rfm22b_dev *rfm22b_dev);
static void rfm22_setDatarate(struct pios_rfm22b_dev * rfm22b_dev, enum rfm22b_datarate datarate, bool data_whitening);
static enum pios_rfm22b_event rfm22_init(struct pios_rfm22b_dev *rfm22b_dev);
static enum pios_rfm22b_event rfm22_setRxMode(struct pios_rfm22b_dev *rfm22b_dev);
static enum pios_rfm22b_event rfm22_detectPreamble(struct pios_rfm22b_dev *rfm22b_dev);
static enum pios_rfm22b_event rfm22_detectSync(struct pios_rfm22b_dev *rfm22b_dev);
static enum pios_rfm22b_event rfm22_rxData(struct pios_rfm22b_dev *rfm22b_dev);
static enum pios_rfm22b_event rfm22_rxFailure(struct pios_rfm22b_dev *rfm22b_dev);
static enum pios_rfm22b_event rfm22_receiveStatus(struct pios_rfm22b_dev *rfm22b_dev);
static enum pios_rfm22b_event rfm22_receiveAck(struct pios_rfm22b_dev *rfm22b_dev);
static enum pios_rfm22b_event rfm22_receiveNack(struct pios_rfm22b_dev *rfm22b_dev);
static enum pios_rfm22b_event rfm22_sendAck(struct pios_rfm22b_dev *rfm22b_dev);
static enum pios_rfm22b_event rfm22_sendNack(struct pios_rfm22b_dev *rfm22b_dev);
static enum pios_rfm22b_event rfm22_requestConnection(struct pios_rfm22b_dev *rfm22b_dev);
static enum pios_rfm22b_event rfm22_acceptConnection(struct pios_rfm22b_dev *rfm22b_dev);
static enum pios_rfm22b_event rfm22_txStart(struct pios_rfm22b_dev *rfm22b_dev);
static enum pios_rfm22b_event rfm22_txData(struct pios_rfm22b_dev *rfm22b_dev);
static enum pios_rfm22b_event rfm22_txFailure(struct pios_rfm22b_dev *rfm22b_dev);
static enum pios_rfm22b_event rfm22_process_state_transition(struct pios_rfm22b_dev *rfm22b_dev, enum pios_rfm22b_event event);
static void rfm22_process_event(struct pios_rfm22b_dev *rfm22b_dev, enum pios_rfm22b_event event);
static enum pios_rfm22b_event rfm22_timeout(struct pios_rfm22b_dev *rfm22b_dev);
static enum pios_rfm22b_event rfm22_error(struct pios_rfm22b_dev *rfm22b_dev);
static enum pios_rfm22b_event rfm22_fatal_error(struct pios_rfm22b_dev *rfm22b_dev);
static void rfm22_sendStatus(struct pios_rfm22b_dev *rfm22b_dev);
static void rfm22_sendPPM(struct pios_rfm22b_dev *rfm22b_dev);
static void rfm22b_add_rx_status(struct pios_rfm22b_dev *rfm22b_dev, enum pios_rfm22b_rx_packet_status status);
static bool rfm22_receivePacket(struct pios_rfm22b_dev *rfm22b_dev, PHPacketHandle p, uint16_t rx_len);
static void rfm22_setNominalCarrierFrequency(struct pios_rfm22b_dev *rfm22b_dev, uint32_t min_frequency, uint32_t max_frequency, uint32_t step_size);
static bool rfm22_setFreqHopChannel(struct pios_rfm22b_dev *rfm22b_dev, uint8_t channel);
static void rfm22_calculateLinkQuality(struct pios_rfm22b_dev *rfm22b_dev);
static bool rfm22_ready_to_send(struct pios_rfm22b_dev *rfm22b_dev);
static bool rfm22_isConnected(struct pios_rfm22b_dev *rfm22b_dev);
static void rfm22_setConnectionParameters(struct pios_rfm22b_dev *rfm22b_dev);
static portTickType rfm22_coordinatorTime(struct pios_rfm22b_dev *rfm22b_dev, portTickType ticks);
static bool rfm22_inChannelBuffer(struct pios_rfm22b_dev *rfm22b_dev);
static uint8_t rfm22_calcChannel(struct pios_rfm22b_dev *rfm22b_dev);
static bool rfm22_changeChannel(struct pios_rfm22b_dev *rfm22b_dev);
static void rfm22_clearLEDs();

// SPI read/write functions
static void rfm22_assertCs(struct pios_rfm22b_dev *rfm22b_dev);
static void rfm22_deassertCs(struct pios_rfm22b_dev *rfm22b_dev);
static void rfm22_claimBus(struct pios_rfm22b_dev *rfm22b_dev);
static void rfm22_releaseBus(struct pios_rfm22b_dev *rfm22b_dev);
static void rfm22_write(struct pios_rfm22b_dev *rfm22b_dev, uint8_t addr, uint8_t data);
static uint8_t rfm22_read(struct pios_rfm22b_dev *rfm22b_dev, uint8_t addr);
static uint8_t rfm22_read_noclaim(struct pios_rfm22b_dev *rfm22b_dev, uint8_t addr);

/* Te state transition table */
const static struct pios_rfm22b_transition rfm22b_transitions[RFM22B_STATE_NUM_STATES] = {

	// Initialization thread
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
			[RFM22B_EVENT_INITIALIZED] = RFM22B_STATE_RX_MODE,
			[RFM22B_EVENT_ERROR] = RFM22B_STATE_ERROR,
			[RFM22B_EVENT_INITIALIZE] = RFM22B_STATE_INITIALIZING,
			[RFM22B_EVENT_FATAL_ERROR] = RFM22B_STATE_FATAL_ERROR,
		},
	},
	[RFM22B_STATE_REQUESTING_CONNECTION] = {
		.entry_fn = rfm22_requestConnection,
		.next_state = {
			[RFM22B_EVENT_TX_START] = RFM22B_STATE_TX_START,
			[RFM22B_EVENT_TIMEOUT] = RFM22B_STATE_TIMEOUT,
			[RFM22B_EVENT_ERROR] = RFM22B_STATE_ERROR,
			[RFM22B_EVENT_INITIALIZE] = RFM22B_STATE_INITIALIZING,
			[RFM22B_EVENT_FATAL_ERROR] = RFM22B_STATE_FATAL_ERROR
		},
	},
	[RFM22B_STATE_ACCEPTING_CONNECTION] = {
		.entry_fn = rfm22_acceptConnection,
		.next_state = {
			[RFM22B_EVENT_DEFAULT] = RFM22B_STATE_SENDING_ACK,
			[RFM22B_EVENT_TIMEOUT] = RFM22B_STATE_TIMEOUT,
			[RFM22B_EVENT_ERROR] = RFM22B_STATE_ERROR,
			[RFM22B_EVENT_INITIALIZE] = RFM22B_STATE_INITIALIZING,
			[RFM22B_EVENT_FATAL_ERROR] = RFM22B_STATE_FATAL_ERROR,
		},
	},

	[RFM22B_STATE_RX_MODE] = {
		.entry_fn = rfm22_setRxMode,
		.next_state = {
			[RFM22B_EVENT_INT_RECEIVED] = RFM22B_STATE_WAIT_PREAMBLE,
			[RFM22B_EVENT_TX_START] = RFM22B_STATE_TX_START,
			[RFM22B_EVENT_ACK_TIMEOUT] = RFM22B_STATE_RECEIVING_NACK,
			[RFM22B_EVENT_FAILURE] = RFM22B_STATE_RX_FAILURE,
			[RFM22B_EVENT_TIMEOUT] = RFM22B_STATE_TIMEOUT,
			[RFM22B_EVENT_ERROR] = RFM22B_STATE_ERROR,
			[RFM22B_EVENT_INITIALIZE] = RFM22B_STATE_INITIALIZING,
			[RFM22B_EVENT_FATAL_ERROR] = RFM22B_STATE_FATAL_ERROR,
		},
	},
	[RFM22B_STATE_WAIT_PREAMBLE] = {
		.entry_fn = rfm22_detectPreamble,
		.next_state = {
			[RFM22B_EVENT_PREAMBLE_DETECTED] = RFM22B_STATE_WAIT_SYNC,
			[RFM22B_EVENT_TX_START] = RFM22B_STATE_TX_START,
			[RFM22B_EVENT_ACK_TIMEOUT] = RFM22B_STATE_RECEIVING_NACK,
			[RFM22B_EVENT_INT_RECEIVED] = RFM22B_STATE_WAIT_PREAMBLE,
			[RFM22B_EVENT_FAILURE] = RFM22B_STATE_RX_FAILURE,
			[RFM22B_EVENT_TIMEOUT] = RFM22B_STATE_TIMEOUT,
			[RFM22B_EVENT_ERROR] = RFM22B_STATE_ERROR,
			[RFM22B_EVENT_INITIALIZE] = RFM22B_STATE_INITIALIZING,
			[RFM22B_EVENT_FATAL_ERROR] = RFM22B_STATE_FATAL_ERROR,
		},
	},
	[RFM22B_STATE_WAIT_SYNC] = {
		.entry_fn = rfm22_detectSync,
		.next_state = {
			[RFM22B_EVENT_INT_RECEIVED] = RFM22B_STATE_WAIT_SYNC,
			[RFM22B_EVENT_SYNC_DETECTED] = RFM22B_STATE_RX_DATA,
			[RFM22B_EVENT_FAILURE] = RFM22B_STATE_RX_FAILURE,
			[RFM22B_EVENT_TIMEOUT] = RFM22B_STATE_TIMEOUT,
			[RFM22B_EVENT_ERROR] = RFM22B_STATE_ERROR,
			[RFM22B_EVENT_INITIALIZE] = RFM22B_STATE_INITIALIZING,
			[RFM22B_EVENT_FATAL_ERROR] = RFM22B_STATE_FATAL_ERROR,
		},
	},
	[RFM22B_STATE_RX_DATA] = {
		.entry_fn = rfm22_rxData,
		.next_state = {
			[RFM22B_EVENT_INT_RECEIVED] = RFM22B_STATE_RX_DATA,
			[RFM22B_EVENT_RX_COMPLETE] = RFM22B_STATE_SENDING_ACK,
			[RFM22B_EVENT_RX_MODE] = RFM22B_STATE_RX_MODE,
			[RFM22B_EVENT_RX_ERROR] = RFM22B_STATE_SENDING_NACK,
			[RFM22B_EVENT_STATUS_RECEIVED] = RFM22B_STATE_RECEIVING_STATUS,
			[RFM22B_EVENT_CONNECTION_REQUESTED] = RFM22B_STATE_ACCEPTING_CONNECTION,
			[RFM22B_EVENT_PACKET_ACKED] = RFM22B_STATE_RECEIVING_ACK,
			[RFM22B_EVENT_PACKET_NACKED] = RFM22B_STATE_RECEIVING_NACK,
			[RFM22B_EVENT_FAILURE] = RFM22B_STATE_RX_FAILURE,
			[RFM22B_EVENT_TIMEOUT] = RFM22B_STATE_TIMEOUT,
			[RFM22B_EVENT_ERROR] = RFM22B_STATE_ERROR,
			[RFM22B_EVENT_INITIALIZE] = RFM22B_STATE_INITIALIZING,
			[RFM22B_EVENT_FATAL_ERROR] = RFM22B_STATE_FATAL_ERROR,
		},
	},
	[RFM22B_STATE_RX_FAILURE] = {
		.entry_fn = rfm22_rxFailure,
		.next_state = {
			[RFM22B_EVENT_RX_MODE] = RFM22B_STATE_RX_MODE,
			[RFM22B_EVENT_TIMEOUT] = RFM22B_STATE_TIMEOUT,
			[RFM22B_EVENT_ERROR] = RFM22B_STATE_ERROR,
			[RFM22B_EVENT_INITIALIZE] = RFM22B_STATE_INITIALIZING,
			[RFM22B_EVENT_FATAL_ERROR] = RFM22B_STATE_FATAL_ERROR,
		},
	},
	[RFM22B_STATE_RECEIVING_ACK] = {
		.entry_fn = rfm22_receiveAck,
		.next_state = {
			[RFM22B_EVENT_TX_START] = RFM22B_STATE_TX_START,
			[RFM22B_EVENT_RX_MODE] = RFM22B_STATE_RX_MODE,
			[RFM22B_EVENT_TIMEOUT] = RFM22B_STATE_TIMEOUT,
			[RFM22B_EVENT_ERROR] = RFM22B_STATE_ERROR,
			[RFM22B_EVENT_INITIALIZE] = RFM22B_STATE_INITIALIZING,
			[RFM22B_EVENT_FATAL_ERROR] = RFM22B_STATE_FATAL_ERROR,
		},
	},
	[RFM22B_STATE_RECEIVING_NACK] = {
		.entry_fn = rfm22_receiveNack,
		.next_state = {
			[RFM22B_EVENT_TX_START] = RFM22B_STATE_TX_START,
			[RFM22B_EVENT_TIMEOUT] = RFM22B_STATE_TIMEOUT,
			[RFM22B_EVENT_ERROR] = RFM22B_STATE_ERROR,
			[RFM22B_EVENT_INITIALIZE] = RFM22B_STATE_INITIALIZING,
			[RFM22B_EVENT_FATAL_ERROR] = RFM22B_STATE_FATAL_ERROR,
		},
	},
	[RFM22B_STATE_RECEIVING_STATUS] = {
		.entry_fn = rfm22_receiveStatus,
		.next_state = {
			[RFM22B_EVENT_RX_COMPLETE] = RFM22B_STATE_SENDING_ACK,
			[RFM22B_EVENT_TIMEOUT] = RFM22B_STATE_TIMEOUT,
			[RFM22B_EVENT_ERROR] = RFM22B_STATE_ERROR,
			[RFM22B_EVENT_INITIALIZE] = RFM22B_STATE_INITIALIZING,
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
			[RFM22B_EVENT_INITIALIZE] = RFM22B_STATE_INITIALIZING,
			[RFM22B_EVENT_FATAL_ERROR] = RFM22B_STATE_FATAL_ERROR,
		},
	},
	[RFM22B_STATE_TX_DATA] = {
		.entry_fn = rfm22_txData,
		.next_state = {
			[RFM22B_EVENT_INT_RECEIVED] = RFM22B_STATE_TX_DATA,
			[RFM22B_EVENT_REQUEST_CONNECTION] = RFM22B_STATE_REQUESTING_CONNECTION,
			[RFM22B_EVENT_RX_MODE] = RFM22B_STATE_RX_MODE,
			[RFM22B_EVENT_FAILURE] = RFM22B_STATE_TX_FAILURE,
			[RFM22B_EVENT_TIMEOUT] = RFM22B_STATE_TIMEOUT,
			[RFM22B_EVENT_ERROR] = RFM22B_STATE_ERROR,
			[RFM22B_EVENT_INITIALIZE] = RFM22B_STATE_INITIALIZING,
			[RFM22B_EVENT_FATAL_ERROR] = RFM22B_STATE_FATAL_ERROR,
		},
	},
	[RFM22B_STATE_TX_FAILURE] = {
		.entry_fn = rfm22_txFailure,
		.next_state = {
			[RFM22B_EVENT_TX_START] = RFM22B_STATE_TX_START,
			[RFM22B_EVENT_TIMEOUT] = RFM22B_STATE_TIMEOUT,
			[RFM22B_EVENT_ERROR] = RFM22B_STATE_ERROR,
			[RFM22B_EVENT_INITIALIZE] = RFM22B_STATE_INITIALIZING,
			[RFM22B_EVENT_FATAL_ERROR] = RFM22B_STATE_FATAL_ERROR,
		},
	},
	[RFM22B_STATE_SENDING_ACK] = {
		.entry_fn = rfm22_sendAck,
		.next_state = {
			[RFM22B_EVENT_TX_START] = RFM22B_STATE_TX_START,
			[RFM22B_EVENT_ERROR] = RFM22B_STATE_ERROR,
			[RFM22B_EVENT_INITIALIZE] = RFM22B_STATE_INITIALIZING,
			[RFM22B_EVENT_FATAL_ERROR] = RFM22B_STATE_FATAL_ERROR,
		},
	},
	[RFM22B_STATE_SENDING_NACK] = {
		.entry_fn = rfm22_sendNack,
		.next_state = {
			[RFM22B_EVENT_TX_START] = RFM22B_STATE_TX_START,
			[RFM22B_EVENT_ERROR] = RFM22B_STATE_ERROR,
			[RFM22B_EVENT_INITIALIZE] = RFM22B_STATE_INITIALIZING,
			[RFM22B_EVENT_FATAL_ERROR] = RFM22B_STATE_FATAL_ERROR,
		},
	},
	[RFM22B_STATE_TIMEOUT] = {
		.entry_fn = rfm22_timeout,
		.next_state = {
			[RFM22B_EVENT_TX_START] = RFM22B_STATE_TX_START,
			[RFM22B_EVENT_ERROR] = RFM22B_STATE_ERROR,
			[RFM22B_EVENT_INITIALIZE] = RFM22B_STATE_INITIALIZING,
			[RFM22B_EVENT_FATAL_ERROR] = RFM22B_STATE_FATAL_ERROR,
		},
	},
	[RFM22B_STATE_ERROR] = {
		.entry_fn = rfm22_error,
		.next_state = {
			[RFM22B_EVENT_ERROR] = RFM22B_STATE_ERROR,
			[RFM22B_EVENT_INITIALIZE] = RFM22B_STATE_INITIALIZING,
			[RFM22B_EVENT_FATAL_ERROR] = RFM22B_STATE_FATAL_ERROR,
		},
	},
	[RFM22B_STATE_FATAL_ERROR] = {
		.entry_fn = rfm22_fatal_error,
		.next_state = {
		},
	},
};

// xtal 10 ppm, 434MHz
#define LOOKUP_SIZE	 15
static const uint32_t            data_rate[] = {   500,  1000,  2000,  4000,  8000,  9600, 16000, 19200, 24000,  32000,  57600, 64000, 128000, 192000, 256000};
static const uint8_t      modulation_index[] = {    16,     8,     4,     2,     1,     1,     1,     1,     1,      1,      1,     1,      1,      1,      1};

static const uint8_t                reg_1C[] = {  0x37,  0x37,  0x37,  0x37,  0x3A,  0x3B,  0x26,  0x28,  0x2E,   0x16,   0x06,   0x07,   0x83,   0x8A,   0x8C}; // rfm22_if_filter_bandwidth

static const uint8_t                reg_1D[] = {  0x44,  0x44,  0x44,  0x44,  0x44,  0x44,  0x44,  0x44,  0x44,   0x44,   0x40,   0x44,   0x44,   0x44,   0x44}; // rfm22_afc_loop_gearshift_override
static const uint8_t                reg_1E[] = {  0x0A,  0x0A,  0x0A,  0x0A,  0x0A,  0x0A,  0x0A,  0x0A,  0x0A,   0x0A,   0x0A,   0x0A,   0x0A,   0x0A,   0x02}; // rfm22_afc_timing_control

static const uint8_t                reg_1F[] = {  0x03,  0x03,  0x03,  0x03,  0x03,  0x03,  0x03,  0x03,  0x03,   0x03,   0x03,   0x03,   0x03,   0x03,   0x03}; // rfm22_clk_recovery_gearshift_override
static const uint8_t                reg_20[] = {  0xE8,  0xF4,  0xFA,  0x70,  0x3F,  0x34,  0x3F,  0x34,  0x2A,   0x3F,   0x45,   0x3F,   0x5E,   0x3F,   0x2F}; // rfm22_clk_recovery_oversampling_ratio
static const uint8_t                reg_21[] = {  0x60,  0x20,  0x00,  0x01,  0x02,  0x02,  0x02,  0x02,  0x03,   0x02,   0x01,   0x02,   0x01,   0x02,   0x02}; // rfm22_clk_recovery_offset2
static const uint8_t                reg_22[] = {  0x20,  0x41,  0x83,  0x06,  0x0C,  0x75,  0x0C,  0x75,  0x12,   0x0C,   0xD7,   0x0c,   0x5D,   0x0C,   0xBB}; // rfm22_clk_recovery_offset1
static const uint8_t                reg_23[] = {  0xC5,  0x89,  0x12,  0x25,  0x4A,  0x25,  0x4A,  0x25,  0x6F,   0x4A,   0xDC,   0x4A,   0x86,   0x4A,   0x0D}; // rfm22_clk_recovery_offset0
static const uint8_t                reg_24[] = {  0x00,  0x00,  0x00,  0x02,  0x07,  0x07,  0x07,  0x07,  0x07,   0x07,   0x07,   0x07,   0x05,   0x07,   0x07}; // rfm22_clk_recovery_timing_loop_gain1
static const uint8_t                reg_25[] = {  0x0A,  0x23,  0x85,  0x0E,  0xFF,  0xFF,  0xFF,  0xFF,  0xFF,   0xFF,   0x6E,   0xFF,   0x74,   0xFF,   0xFF}; // rfm22_clk_recovery_timing_loop_gain0

static const uint8_t                reg_2A[] = {  0x0E,  0x0E,  0x0E,  0x0E,  0x0E,  0x0D,  0x0D,  0x0E,  0x12,   0x17,   0x2D,   0x31,   0x50,   0x50,   0x50}; // rfm22_afc_limiter .. AFC_pull_in_range = �AFCLimiter[7:0] x (hbsel+1) x 625 Hz

static const uint8_t                reg_58[] = {  0x80,  0x80,  0x80,  0x80,  0x80,  0x80,  0x80,  0x80,  0x80,   0x80,   0x80,   0x80,   0x80,   0x80,   0x80}; // rfm22_cpcuu
static const uint8_t                reg_69[] = {  0x20,  0x20,  0x20,  0x20,  0x20,  0x20,  0x20,  0x20,  0x20,   0x20,   0x60,   0x20,   0x20,   0x20,   0x20}; // rfm22_agc_override1
static const uint8_t                reg_6E[] = {  0x04,  0x08,  0x10,  0x20,  0x41,  0x4E,  0x83,  0x9D,  0xC4,   0x08,   0x0E,   0x10,   0x20,   0x31,   0x41}; // rfm22_tx_data_rate1
static const uint8_t                reg_6F[] = {  0x19,  0x31,  0x62,  0xC5,  0x89,  0xA5,  0x12,  0x49,  0x9C,   0x31,   0xBF,   0x62,   0xC5,   0x27,   0x89}; // rfm22_tx_data_rate0

static const uint8_t                reg_70[] = {  0x2D,  0x2D,  0x2D,  0x2D,  0x2D,  0x2D,  0x2D,  0x2D,  0x2D,   0x0D,   0x0C,   0x0C,   0x0D,   0x0D,   0x0D}; // rfm22_modulation_mode_control1
static const uint8_t                reg_71[] = {  0x23,  0x23,  0x23,  0x23,  0x23,  0x23,  0x23,  0x23,  0x23,   0x23,   0x23,   0x23,   0x23,   0x23,   0x23}; // rfm22_modulation_mode_control2

static const uint8_t                reg_72[] = {  0x06,  0x06,  0x06,  0x06,  0x06,  0x08,  0x0D,  0x0F,  0x13,   0x1A,   0x2E,   0x33,   0x66,   0x9A,   0xCD}; // rfm22_frequency_deviation


static inline uint32_t timeDifferenceMs(portTickType start_time, portTickType end_time)
{
	if(end_time >= start_time)
		return (end_time - start_time) * portTICK_RATE_MS;
	// Rollover
	return ((portMAX_DELAY - start_time) + end_time) * portTICK_RATE_MS;
}

bool PIOS_RFM22B_validate(struct pios_rfm22b_dev * rfm22b_dev)
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
	*rfm22b_id = (uint32_t)rfm22b_dev;
	g_rfm22b_dev = rfm22b_dev;

	// Store the SPI handle
	rfm22b_dev->slave_num = slave_num;
	rfm22b_dev->spi_id = spi_id;

	// Initialize our configuration parameters
	rfm22b_dev->send_ppm = false;
	rfm22b_dev->datarate = RFM22B_DEFAULT_RX_DATARATE;
	rfm22b_dev->tx_power = RFM22B_DEFAULT_TX_POWER;

	// Initialize the com callbacks.
	rfm22b_dev->com_config_cb = NULL;
	rfm22b_dev->rx_in_cb = NULL;
	rfm22b_dev->tx_out_cb = NULL;

	// Initialize the stats.
	rfm22b_dev->stats.packets_per_sec = 0;
	rfm22b_dev->stats.rx_good = 0;
	rfm22b_dev->stats.rx_corrected = 0;
	rfm22b_dev->stats.rx_error = 0;
	rfm22b_dev->stats.rx_missed = 0;
	rfm22b_dev->stats.tx_dropped = 0;
	rfm22b_dev->stats.tx_resent = 0;
	rfm22b_dev->stats.resets = 0;
	rfm22b_dev->stats.timeouts = 0;
	rfm22b_dev->stats.link_quality = 0;
	rfm22b_dev->stats.rssi = 0;
	rfm22b_dev->stats.tx_seq = 0;
	rfm22b_dev->stats.rx_seq = 0;

	// Initialize the frequencies.
	PIOS_RFM22B_SetInitialFrequency(*rfm22b_id, RFM22B_DEFAULT_FREQUENCY);
	PIOS_RFM22B_SetFrequencyRange(*rfm22b_id, RFM22B_DEFAULT_FREQUENCY, RFM22B_DEFAULT_FREQUENCY, RFM22B_FREQUENCY_HOP_STEP_SIZE);

	// Initialize the bindings.
	for (uint32_t i = 0; i < OPLINKSETTINGS_BINDINGS_NUMELEM; ++i) {
		rfm22b_dev->bindings[i].pairID = 0;
	}
	rfm22b_dev->coordinator = false;

	// Create the event queue
	rfm22b_dev->eventQueue = xQueueCreate(EVENT_QUEUE_SIZE, sizeof(enum pios_rfm22b_event));

	// Bind the configuration to the device instance
	rfm22b_dev->cfg = *cfg;

	// Create a semaphore to know if an ISR needs responding to
	vSemaphoreCreateBinary( rfm22b_dev->isrPending );

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

#if defined(PIOS_INCLUDE_GCSRCVR)
        // Initialize the GCSReceive object
	GCSReceiverInitialize();
#endif

	// Initialize the external interrupt.
	PIOS_EXTI_Init(cfg->exti_cfg);

	// Register the watchdog timer for the radio driver task
#ifdef PIOS_WDG_RFM22B
	PIOS_WDG_RegisterFlag(PIOS_WDG_RFM22B);
#endif /* PIOS_WDG_RFM22B */

	// Initialize the ECC library.
	initialize_ecc();

	// Set the state to initializing.
	rfm22b_dev->state = RFM22B_STATE_UNINITIALIZED;

	// Initialize the radio device.
	PIOS_RFM22B_InjectEvent(rfm22b_dev, RFM22B_EVENT_INITIALIZE, false);

	// Start the driver task.  This task controls the radio state machine and removed all of the IO from the IRQ handler.
	xTaskCreate(PIOS_RFM22B_Task, (signed char *)"PIOS_RFM22B_Task", STACK_SIZE_BYTES, (void*)rfm22b_dev, TASK_PRIORITY, &(rfm22b_dev->taskHandle));

	return(0);
}

/**
 * Re-initialize the modem after a configuration change.
 */
void PIOS_RFM22B_Reinit(uint32_t rfm22b_id)
{
	struct pios_rfm22b_dev *rfm22b_dev = (struct pios_rfm22b_dev *)rfm22b_id;
	if (PIOS_RFM22B_validate(rfm22b_dev))
		PIOS_RFM22B_InjectEvent(rfm22b_dev, RFM22B_EVENT_INITIALIZE, false);
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
void PIOS_RFM22B_InjectEvent(struct pios_rfm22b_dev *rfm22b_dev, enum pios_rfm22b_event event, bool inISR)
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
 * Returns the unique device ID for the RFM22B device.
 * \param[in] rfm22b_id The RFM22B device index.
 * \return The unique device ID
 */
uint32_t PIOS_RFM22B_DeviceID(uint32_t rfm22b_id)
{
	struct pios_rfm22b_dev *rfm22b_dev = (struct pios_rfm22b_dev *)rfm22b_id;
	if (PIOS_RFM22B_validate(rfm22b_dev)) {
                return rfm22b_dev->deviceID;
	} else {
		return 0;
	}
}

/**
 * Returns true if the modem is configured as a coordinator.
 * \param[in] rfm22b_id The RFM22B device index.
 * \return True if the modem is configured as a coordinator.
 */
bool PIOS_RFM22B_IsCoordinator(uint32_t rfm22b_id)
{
	struct pios_rfm22b_dev *rfm22b_dev = (struct pios_rfm22b_dev *)rfm22b_id;
	if (PIOS_RFM22B_validate(rfm22b_dev)) {
		return rfm22b_dev->coordinator;
	} else {
		return false;
	}
}

/**
 * Sets the radio device transmit power.
 * \param[in] rfm22b_id The RFM22B device index.
 * \param[in] tx_pwr The transmit power.
 */
void PIOS_RFM22B_SetTxPower(uint32_t rfm22b_id, enum rfm22b_tx_power tx_pwr)
{
	struct pios_rfm22b_dev *rfm22b_dev = (struct pios_rfm22b_dev *)rfm22b_id;
	if (!PIOS_RFM22B_validate(rfm22b_dev)) {
		return;
	}
	rfm22b_dev->tx_power = tx_pwr;
}

/**
 * Sets the radio frequency range and initial frequency
 * \param[in] rfm22b_id  The RFM22B device index.
 * \param[in] min_freq  The minimum frequency
 * \param[in] max_freq  The maximum frequency
 * \param[in] step_size  The channel step size
 */
void PIOS_RFM22B_SetFrequencyRange(uint32_t rfm22b_id, uint32_t min_freq, uint32_t max_freq, uint32_t step_size)
{
	struct pios_rfm22b_dev *rfm22b_dev = (struct pios_rfm22b_dev *)rfm22b_id;
	if (!PIOS_RFM22B_validate(rfm22b_dev)) {
		return;
	}
	rfm22b_dev->con_packet.min_frequency = min_freq;
	rfm22b_dev->con_packet.max_frequency = max_freq;
	rfm22b_dev->con_packet.channel_spacing = step_size;
}

/**
 * Sets the initial radio frequency range
 * \param[in] rfm22b_id  The RFM22B device index.
 * \param[in] init_freq  The initial frequency
 */
void PIOS_RFM22B_SetInitialFrequency(uint32_t rfm22b_id, uint32_t init_freq)
{
	struct pios_rfm22b_dev *rfm22b_dev = (struct pios_rfm22b_dev *)rfm22b_id;
	if (!PIOS_RFM22B_validate(rfm22b_dev)) {
		return;
	}
	rfm22b_dev->init_frequency = init_freq;
}

/**
 * Set the com port configuration callback (to receive com configuration over the air)
 * \param[in] rfm22b_id  The rfm22b device.
 * \param[in] cb  A pointer to the callback function
 */
void PIOS_RFM22B_SetComConfigCallback(uint32_t rfm22b_id, PIOS_RFM22B_ComConfigCallback cb)
{
 	struct pios_rfm22b_dev *rfm22b_dev = (struct pios_rfm22b_dev *)rfm22b_id;
	if(!PIOS_RFM22B_validate(rfm22b_dev)) {
		return;
	}
	rfm22b_dev->com_config_cb = cb;
}

/**
 * Set the list of modems that this modem will bind with.
 * \param[in] rfm22b_id  The rfm22b device.
 * \param[in] bindings  The array of bindings.
 */
void PIOS_RFM22B_SetBindings(uint32_t rfm22b_id, const uint32_t bindingPairIDs[], const uint8_t mainPortSettings[],
			     const uint8_t flexiPortSettings[], const uint8_t vcpPortSettings[], const uint8_t comSpeeds[])
{
 	struct pios_rfm22b_dev *rfm22b_dev = (struct pios_rfm22b_dev *)rfm22b_id;
	if(!PIOS_RFM22B_validate(rfm22b_dev))
		return;
	// This modem will be considered a coordinator if any bindings have been set.
	rfm22b_dev->coordinator = false;
	for (uint32_t i = 0; i < OPLINKSETTINGS_BINDINGS_NUMELEM; ++i) {
		rfm22b_dev->bindings[i].pairID = bindingPairIDs[i];
		rfm22b_dev->bindings[i].main_port = mainPortSettings[i];
		rfm22b_dev->bindings[i].flexi_port = flexiPortSettings[i];
		rfm22b_dev->bindings[i].vcp_port = vcpPortSettings[i];
		rfm22b_dev->bindings[i].com_speed = comSpeeds[i];
		rfm22b_dev->coordinator |= (rfm22b_dev->bindings[i].pairID != 0);
	}
}

/**
 * Returns the device statistics RFM22B device.
 * \param[in] rfm22b_id The RFM22B device index.
 * \param[out] stats The stats are returned in this structure
 */
void PIOS_RFM22B_GetStats(uint32_t rfm22b_id, struct rfm22b_stats *stats) {
	struct pios_rfm22b_dev *rfm22b_dev = (struct pios_rfm22b_dev *)rfm22b_id;
	if(!PIOS_RFM22B_validate(rfm22b_dev))
		return;

	// Calculate the current link quality
	rfm22_calculateLinkQuality(rfm22b_dev);

	// We are connected if our destination ID is in the pair stats.
	if (rfm22b_dev->destination_id != 0xffffffff)
		for (uint8_t i = 0; i < OPLINKSTATUS_PAIRIDS_NUMELEM; ++i)
		{
			if ((rfm22b_dev->pair_stats[i].pairID == rfm22b_dev->destination_id) &&
			    (rfm22b_dev->pair_stats[i].rssi > -127))
			{
				rfm22b_dev->stats.rssi = rfm22b_dev->pair_stats[i].rssi;
				rfm22b_dev->stats.afc_correction = rfm22b_dev->pair_stats[i].afc_correction;
				break;
			}
		}
	*stats = rfm22b_dev->stats;
}

/**
 * Get the stats of the oter radio devices that are in range.
 * \param[out] device_ids  A pointer to the array to store the device IDs.
 * \param[out] RSSIs  A pointer to the array to store the RSSI values in.
 * \param[in] mx_pairs  The length of the pdevice_ids and RSSIs arrays.
 * \return  The number of pair stats returned.
 */
uint8_t PIOS_RFM2B_GetPairStats(uint32_t rfm22b_id, uint32_t *device_ids, int8_t *RSSIs, uint8_t max_pairs)
{
	struct pios_rfm22b_dev *rfm22b_dev = (struct pios_rfm22b_dev *)rfm22b_id;
 	if (!PIOS_RFM22B_validate(rfm22b_dev))
 		return 0;

	uint8_t mp = (max_pairs >= OPLINKSTATUS_PAIRIDS_NUMELEM) ? max_pairs : OPLINKSTATUS_PAIRIDS_NUMELEM;
	for (uint8_t i = 0; i < mp; ++i)
 	{
		device_ids[i] = rfm22b_dev->pair_stats[i].pairID;
		RSSIs[i] = rfm22b_dev->pair_stats[i].rssi;
 	}

	return mp;
}

/**
 * Check the radio device for a valid connection
 * \param[in] rfm22b_id  The rfm22b device.
 * Returns true if there is a valid connection to paired radio, false otherwise.
 */
bool PIOS_RFM22B_LinkStatus(uint32_t rfm22b_id)
{
 	struct pios_rfm22b_dev *rfm22b_dev = (struct pios_rfm22b_dev *)rfm22b_id;
	if(!PIOS_RFM22B_validate(rfm22b_dev))
		return false;
	return (rfm22_isConnected(rfm22b_dev) && (rfm22b_dev->stats.link_quality > RFM22B_LINK_QUALITY_THRESHOLD));
}

/**
 * Send a PPM packet with the given channel values.
 * \param[in] rfm22b_id  The rfm22b device.
 * \param[in] channels  The channel values.
 * \param[in] nchannels  The number of channels.
 * Returns true if there is a valid connection to paired radio, false otherwise.
 */
void PIOS_RFM22B_SendPPM(uint32_t rfm22b_id, const uint16_t *channels, uint8_t nchannels)
{
#ifdef PIOS_PPM_RECEIVER
	struct pios_rfm22b_dev *rfm22b_dev = (struct pios_rfm22b_dev *)rfm22b_id;
 	if (!PIOS_RFM22B_validate(rfm22b_dev)) {
 		return;
	}

	// Only send PPM if we're connected
	if (!rfm22_isConnected(rfm22b_dev)) {
		return;
	}

	// See if we have any valid channels.
	uint8_t nchan = (nchannels <= PIOS_PPM_NUM_INPUTS) ? nchannels : PIOS_PPM_NUM_INPUTS;
	for (uint8_t i = 0; i < nchan; ++i) {
		rfm22b_dev->ppm_packet.channels[i] = channels[i];
	}

	// Send the PPM packet.
	rfm22b_dev->ppm_packet.header.destination_id = rfm22b_dev->destination_id;
	rfm22b_dev->ppm_packet.header.type = PACKET_TYPE_PPM;
	rfm22b_dev->ppm_packet.header.data_size = PH_PPM_DATA_SIZE(&(rfm22b_dev->ppm_packet));
	rfm22b_dev->send_ppm = true;
#endif
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
	portTickType lastPPMTicks = lastEventTicks;

	while(1)
	{
#ifdef PIOS_WDG_RFM22B
		// Update the watchdog timer
		PIOS_WDG_UpdateFlag(PIOS_WDG_RFM22B);
#endif /* PIOS_WDG_RFM22B */

		// Wait for a signal indicating an external interrupt or a pending send/receive request.
		if (xSemaphoreTake(rfm22b_dev->isrPending,  ISR_TIMEOUT / portTICK_RATE_MS) == pdTRUE) {
			lastEventTicks = xTaskGetTickCount();

			// Process events through the state machine.
			enum pios_rfm22b_event event;
			while (xQueueReceive(rfm22b_dev->eventQueue, &event, 0) == pdTRUE)
			{
				if ((event == RFM22B_EVENT_INT_RECEIVED) &&
				    ((rfm22b_dev->state == RFM22B_STATE_UNINITIALIZED) || (rfm22b_dev->state == RFM22B_STATE_INITIALIZING)))
					continue;
				rfm22_process_event(rfm22b_dev, event);
			}
		}
		else
		{
			// Has it been too long since the last event?
			portTickType curTicks = xTaskGetTickCount();
			if (timeDifferenceMs(lastEventTicks, curTicks) > PIOS_RFM22B_SUPERVISOR_TIMEOUT)
			{
 				// Transsition through an error event.
				rfm22_process_event(rfm22b_dev, RFM22B_EVENT_ERROR);

				// Clear the event queue.
				enum pios_rfm22b_event event;
				while (xQueueReceive(rfm22b_dev->eventQueue, &event, 0) == pdTRUE)
					;
				lastEventTicks = xTaskGetTickCount();
			}
		}

		// Change channels if necessary.
		if ((rfm22b_dev->state == RFM22B_STATE_RX_MODE) || (rfm22b_dev->state == RFM22B_STATE_WAIT_PREAMBLE)) {
			rfm22_changeChannel(rfm22b_dev);
		}

		portTickType curTicks = xTaskGetTickCount();
		uint32_t last_rec_ms = (rfm22b_dev->rx_complete_ticks == 0) ? 0 : timeDifferenceMs(rfm22b_dev->rx_complete_ticks, curTicks);
		// Have we been sending this packet too long?
		if ((rfm22b_dev->packet_start_ticks > 0) && (timeDifferenceMs(rfm22b_dev->packet_start_ticks, curTicks) > (rfm22b_dev->max_packet_time * 3))) {
			rfm22_process_event(rfm22b_dev, RFM22B_EVENT_TIMEOUT);

		// Has it been too long since we received a packet
		} else if (last_rec_ms > DISCONNECT_TIMEOUT_MS) {
			rfm22_process_event(rfm22b_dev, RFM22B_EVENT_ERROR);
		} else {
				
			// Are we waiting for an ACK?
			if (rfm22b_dev->prev_tx_packet)
			{

				// Should we resend the packet?
				if (timeDifferenceMs(rfm22b_dev->tx_complete_ticks, curTicks) > rfm22b_dev->max_ack_delay)
				{
					rfm22b_dev->tx_complete_ticks = curTicks;
					rfm22_process_event(rfm22b_dev, RFM22B_EVENT_ACK_TIMEOUT);
				}
			}
			else
			{

				// Queue up a PPM packet if it's time.
				if (timeDifferenceMs(lastPPMTicks, curTicks) > PPM_UPDATE_PERIOD_MS)
				{
					rfm22_sendPPM(rfm22b_dev);
					lastPPMTicks = curTicks;
				}

				// Queue up a status packet if it's time.
				if ((timeDifferenceMs(lastStatusTicks, curTicks) > RADIOSTATS_UPDATE_PERIOD_MS) || (last_rec_ms > rfm22b_dev->max_packet_time * 4))
				{
					rfm22_sendStatus(rfm22b_dev);
					lastStatusTicks = curTicks;
				}
			}

		}

		// Send a packet if it's our time slice
		rfm22b_dev->time_to_send = (((curTicks - rfm22b_dev->time_to_send_offset) & 0x6) == 0);
#if defined(PIOS_RFM22B_DEBUG_ON_TELEM) || defined(PIOS_RFM22B_DEBUG_ON_RCVR)
		if (rfm22b_dev->time_to_send) {
			D4_LED_ON;
		} else {
			D4_LED_OFF;
		}
		if (rfm22_inChannelBuffer(rfm22b_dev)) {
			D3_LED_ON;
		} else {
			D3_LED_OFF;
		}
#endif
		if (rfm22b_dev->time_to_send)
			rfm22_process_event(rfm22b_dev, RFM22B_EVENT_TX_START);
	}
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

static void rfm22_setDatarate(struct pios_rfm22b_dev * rfm22b_dev, enum rfm22b_datarate datarate, bool data_whitening)
{
	uint32_t datarate_bps = data_rate[datarate];
	rfm22b_dev->max_packet_time = (uint16_t)((float)(PIOS_PH_MAX_PACKET * 8 * 1000) / (float)(datarate_bps) + 0.5f);
	if (rfm22_isConnected(rfm22b_dev))
	{
		// Generate a pseudo-random number from 0-8 to add to the delay
		uint8_t random = PIOS_CRC_updateByte(0, (uint8_t)(xTaskGetTickCount() & 0xff)) & 0x03;
		rfm22b_dev->max_ack_delay = (uint16_t)((float)((sizeof(PHAckNackPacket) * 8 + TX_PREAMBLE_NIBBLES * 4) * 1000) / (float)(datarate_bps) + 0.5f) * 4 + 4 + random;
	}
	else
		rfm22b_dev->max_ack_delay = CONNECT_ATTEMPT_PERIOD_MS;

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

	if (!data_whitening)
		// rfm22_modulation_mode_control1
		rfm22_write(rfm22b_dev, 0x70, reg_70[datarate] & ~RFM22_mmc1_enwhite);
	else
		// rfm22_modulation_mode_control1
		rfm22_write(rfm22b_dev, 0x70, reg_70[datarate] |  RFM22_mmc1_enwhite);

	// rfm22_modulation_mode_control2
	rfm22_write(rfm22b_dev, 0x71, reg_71[datarate]);

	// rfm22_frequency_deviation
	rfm22_write(rfm22b_dev, 0x72, reg_72[datarate]);

	// rfm22_cpcuu
	rfm22_write(rfm22b_dev, 0x58, reg_58[datarate]);

	rfm22_write(rfm22b_dev, RFM22_ook_counter_value1, 0x00);
	rfm22_write(rfm22b_dev, RFM22_ook_counter_value2, 0x00);
}

// ************************************
// SPI read/write

//! Assert the CS line
static void rfm22_assertCs(struct pios_rfm22b_dev *rfm22b_dev)
{
	PIOS_DELAY_WaituS(1);
	if(rfm22b_dev->spi_id != 0)
		PIOS_SPI_RC_PinSet(rfm22b_dev->spi_id, rfm22b_dev->slave_num, 0);
}

//! Deassert the CS line
static void rfm22_deassertCs(struct pios_rfm22b_dev *rfm22b_dev)
{
	if(rfm22b_dev->spi_id != 0)
		PIOS_SPI_RC_PinSet(rfm22b_dev->spi_id, rfm22b_dev->slave_num, 1);
}

//! Claim the SPI bus semaphore
static void rfm22_claimBus(struct pios_rfm22b_dev *rfm22b_dev)
{
	if(rfm22b_dev->spi_id != 0)
		PIOS_SPI_ClaimBus(rfm22b_dev->spi_id);
}

//! Release the SPI bus semaphore
static void rfm22_releaseBus(struct pios_rfm22b_dev *rfm22b_dev)
{
	if(rfm22b_dev->spi_id != 0)
		PIOS_SPI_ReleaseBus(rfm22b_dev->spi_id);
}

/**
 * Claim the semaphore and write a byte to a register
 * @param[in] addr The address to write to
 * @param[in] data The datat to write to that address
 */
static void rfm22_write(struct pios_rfm22b_dev *rfm22b_dev, uint8_t addr, uint8_t data)
{
	rfm22_claimBus(rfm22b_dev);
	rfm22_assertCs(rfm22b_dev);
	uint8_t buf[2] = {addr | 0x80, data};
	PIOS_SPI_TransferBlock(rfm22b_dev->spi_id, buf, NULL, sizeof(buf), NULL);
	rfm22_deassertCs(rfm22b_dev);
	rfm22_releaseBus(rfm22b_dev);
}

/**
 * Write a byte to a register without claiming the bus.  Also
 * toggle the NSS line
 * @param[in] addr The address of the RFM22b register to write
 * @param[in] data The data to write to that register
 static void rfm22_write_noclaim(struct pios_rfm22b_dev *rfm22b_dev, uint8_t addr, uint8_t data)
 {
	 uint8_t buf[2] = {addr | 0x80, data};
	 rfm22_assertCs(rfm22b_dev);
	 PIOS_SPI_TransferBlock(rfm22b_dev->spi_id, buf, NULL, sizeof(buf), NULL);
	 rfm22_deassertCs(rfm22b_dev);
 }
*/

/**

 * Read a byte from an RFM22b register
 * @param[in] addr The address to read from
 * @return Returns the result of the register read
 */
static uint8_t rfm22_read(struct pios_rfm22b_dev *rfm22b_dev, uint8_t addr)
{
	uint8_t in[2];	
	uint8_t out[2] = {addr & 0x7f, 0xFF};
	rfm22_claimBus(rfm22b_dev);
	rfm22_assertCs(rfm22b_dev);
	PIOS_SPI_TransferBlock(rfm22b_dev->spi_id, out, in, sizeof(out), NULL);
	rfm22_deassertCs(rfm22b_dev);
	rfm22_releaseBus(rfm22b_dev);
	return in[1];
}

/**
 * Read a byte from an RFM22b register without claiming the bus
 * @param[in] addr The address to read from
 * @return Returns the result of the register read
 */
static uint8_t rfm22_read_noclaim(struct pios_rfm22b_dev *rfm22b_dev, uint8_t addr)
{
	uint8_t out[2] = {addr & 0x7F, 0xFF};
	uint8_t in[2];
	rfm22_assertCs(rfm22b_dev);
	PIOS_SPI_TransferBlock(rfm22b_dev->spi_id, out, in, sizeof(out), NULL);
	rfm22_deassertCs(rfm22b_dev);
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

static void rfm22_process_event(struct pios_rfm22b_dev *rfm22b_dev, enum pios_rfm22b_event event)
{
	// Process all state transitions.
	while(event != RFM22B_EVENT_NUM_EVENTS)
		event = rfm22_process_state_transition(rfm22b_dev, event);
}

// ************************************

static void rfm22_setNominalCarrierFrequency(struct pios_rfm22b_dev *rfm22b_dev, uint32_t min_frequency, uint32_t max_frequency, uint32_t step_size)
{
	uint32_t frequency_hz = min_frequency;

	// holds the hbsel (1 or 2)
	uint8_t	hbsel;
	if (frequency_hz < 480000000)
		hbsel = 0;
	else
		hbsel = 1;
	float freq_mhz = (float)(frequency_hz) / 1000000.0;
	float xtal_freq_khz = 30000;
	float sfreq = freq_mhz / (10.0 * (xtal_freq_khz / 30000.0) * (1 + hbsel));
	uint32_t fb = (uint32_t)sfreq - 24 + (64 + 32 * hbsel);
	uint32_t fc = (uint32_t)((sfreq - (uint32_t)sfreq) * 64000.0);
	uint8_t fch = (fc >> 8) & 0xff;
	uint8_t fcl = fc & 0xff;

	// Calculate the number of frequency hopping channels.
	rfm22b_dev->num_channels = (step_size == 0) ? 1 : (uint16_t)((max_frequency - min_frequency) / step_size);

	// initialize the frequency hopping step size (specified in 10khz increments).
	uint32_t freq_hop_step_size = step_size / 10000;
	if (freq_hop_step_size > 255) {
		freq_hop_step_size = 255;
        }
	rfm22_write(rfm22b_dev, RFM22_frequency_hopping_step_size, (uint8_t)freq_hop_step_size);

	// frequency hopping channel (0-255)
	rfm22b_dev->frequency_step_size = 156.25f * hbsel;

	// frequency hopping channel (0-255)
	rfm22b_dev->frequency_hop_channel = 0;
	rfm22_write(rfm22b_dev, RFM22_frequency_hopping_channel_select, 0);

	// no frequency offset
	rfm22_write(rfm22b_dev, RFM22_frequency_offset1, 0);
	rfm22_write(rfm22b_dev, RFM22_frequency_offset2, 0);

	// set the carrier frequency
	rfm22_write(rfm22b_dev, RFM22_frequency_band_select, fb & 0xff);
	rfm22_write(rfm22b_dev, RFM22_nominal_carrier_frequency1, fch);
	rfm22_write(rfm22b_dev, RFM22_nominal_carrier_frequency0, fcl);
}

static bool rfm22_setFreqHopChannel(struct pios_rfm22b_dev *rfm22b_dev, uint8_t channel)
{
	// set the frequency hopping channel
	if (rfm22b_dev->frequency_hop_channel == channel)
		return false;
	rfm22b_dev->frequency_hop_channel = channel;
	rfm22_write(rfm22b_dev, RFM22_frequency_hopping_channel_select, channel);
	return true;
}

static void rfm22_calculateLinkQuality(struct pios_rfm22b_dev *rfm22b_dev)
{
	// Add the RX packet statistics
	rfm22b_dev->stats.rx_good = 0;
	rfm22b_dev->stats.rx_corrected = 0;
	rfm22b_dev->stats.rx_error = 0;
	rfm22b_dev->stats.tx_resent = 0;
	for (uint8_t i = 0; i < RFM22B_RX_PACKET_STATS_LEN; ++i)
	{
		uint32_t val = rfm22b_dev->rx_packet_stats[i];
		for (uint8_t j = 0; j < 16; ++j)
		{
			switch ((val >> (j * 2)) & 0x3)
			{
			case RFM22B_GOOD_RX_PACKET:
				rfm22b_dev->stats.rx_good++;
				break;
			case RFM22B_CORRECTED_RX_PACKET:
				rfm22b_dev->stats.rx_corrected++;
				break;
			case RFM22B_ERROR_RX_PACKET:
				rfm22b_dev->stats.rx_error++;
				break;
			case RFM22B_RESENT_TX_PACKET:
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

// ************************************

static enum pios_rfm22b_event rfm22_setRxMode(struct pios_rfm22b_dev *rfm22b_dev)
{
	// Are we already in Rx mode?
	if (rfm22b_dev->in_rx_mode)
 		return RFM22B_EVENT_NUM_EVENTS;
	rfm22b_dev->packet_start_ticks = 0;
#if defined(PIOS_RFM22B_DEBUG_ON_TELEM) || defined(PIOS_RFM22B_DEBUG_ON_RCVR)
	D2_LED_ON;
#endif // PIOS_RFM22B_DEBUG_ON_TELEM

	// disable interrupts
	rfm22_write(rfm22b_dev, RFM22_interrupt_enable1, 0x00);
	rfm22_write(rfm22b_dev, RFM22_interrupt_enable2, 0x00);

	// Switch to TUNE mode
	rfm22_write(rfm22b_dev, RFM22_op_and_func_ctrl1, RFM22_opfc1_pllon);

	RX_LED_OFF;
	TX_LED_OFF;

	// empty the rx buffer
	rfm22b_dev->rx_buffer_wr = 0;

	// Clear the TX buffer.
	rfm22b_dev->tx_data_rd = rfm22b_dev->tx_data_wr = 0;

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

	// Indicate that we're in RX mode.
	rfm22b_dev->in_rx_mode = true;

	// No event generated
	return RFM22B_EVENT_NUM_EVENTS;
}

// ************************************

static bool rfm22_ready_to_send(struct pios_rfm22b_dev *rfm22b_dev)
{
	// Is there a status of PPM packet ready to send?
	if (rfm22b_dev->prev_tx_packet || rfm22b_dev->send_ppm || rfm22b_dev->send_status)
		return true;

	// Are we not connected yet?
	if (!rfm22_isConnected(rfm22b_dev))
		return true;

	// Is there some data ready to sent?
	PHPacketHandle dp = &rfm22b_dev->data_packet;
	if (dp->header.data_size > 0)
		return true;
	bool need_yield = false;
	if (rfm22b_dev->tx_out_cb)
		dp->header.data_size = (rfm22b_dev->tx_out_cb)(rfm22b_dev->tx_out_context, dp->data, PH_MAX_DATA, NULL, &need_yield);
	if (dp->header.data_size > 0)
		return true;

	return false;
}

static bool rfm22_isConnected(struct pios_rfm22b_dev *rfm22b_dev)
{
	return (rfm22b_dev->stats.link_state == OPLINKSTATUS_LINKSTATE_CONNECTED);
}

static enum pios_rfm22b_event rfm22_txStart(struct pios_rfm22b_dev *rfm22b_dev)
{
	PHPacketHandle p = NULL;

	// Don't send if it's not our turn.
	if (!rfm22b_dev->time_to_send || (rfm22_inChannelBuffer(rfm22b_dev) && rfm22_isConnected(rfm22b_dev)))
		return RFM22B_EVENT_RX_MODE;

	// See if there's a packet ready to send.
	if (rfm22b_dev->tx_packet)
		p = rfm22b_dev->tx_packet;

	else {

		// Don't send a packet if we're waiting for an ACK
		if (rfm22b_dev->prev_tx_packet)
			return RFM22B_EVENT_RX_MODE;

		// Send a connection request?
		if (!p && rfm22b_dev->send_connection_request) {
			p = (PHPacketHandle)&(rfm22b_dev->con_packet);
			rfm22b_dev->send_connection_request = false;
		}

#ifdef PIOS_PPM_RECEIVER
		// Send a PPM packet?
		if (!p && rfm22b_dev->send_ppm)	{
			p = (PHPacketHandle)&(rfm22b_dev->ppm_packet);
			rfm22b_dev->send_ppm = false;
		}
#endif

		// Send status?
		if (!p && rfm22b_dev->send_status) {
			p = (PHPacketHandle)&(rfm22b_dev->status_packet);
			rfm22b_dev->send_status = false;
		}

		// Try to get some data to send
		if (!p)	{
			bool need_yield = false;
			p = &rfm22b_dev->data_packet;
			p->header.type = PACKET_TYPE_DATA;
			p->header.destination_id = rfm22b_dev->destination_id;
			if (rfm22b_dev->tx_out_cb && (p->header.data_size == 0))
				p->header.data_size = (rfm22b_dev->tx_out_cb)(rfm22b_dev->tx_out_context, p->data, PH_MAX_DATA, NULL, &need_yield);

			// Don't send any data until we're connected.
			if (!rfm22_isConnected(rfm22b_dev))
				p->header.data_size = 0;
			if (p->header.data_size == 0)
				p = NULL;
		}

		if (p)
			p->header.seq_num = rfm22b_dev->stats.tx_seq++;
	}
	if (!p)
		return RFM22B_EVENT_RX_MODE;

	// We're transitioning out of Rx mode.
	rfm22b_dev->in_rx_mode = false;

#if defined(PIOS_RFM22B_DEBUG_ON_TELEM) || defined(PIOS_RFM22B_DEBUG_ON_RCVR)
	D1_LED_ON;
	D2_LED_OFF;
#endif

	// Change the channel if necessary.
	if (((p->header.type != PACKET_TYPE_ACK) && (p->header.type != PACKET_TYPE_ACK_RTS)) ||
	    (rfm22b_dev->rx_packet.header.type != PACKET_TYPE_CON_REQUEST))
		rfm22_changeChannel(rfm22b_dev);

	// Add the error correcting code.
	encode_data((unsigned char*)p, PHPacketSize(p), (unsigned char*)p);

	rfm22b_dev->tx_packet = p;
	rfm22b_dev->packet_start_ticks = xTaskGetTickCount();
	if (rfm22b_dev->packet_start_ticks == 0)
		rfm22b_dev->packet_start_ticks = 1;

	// disable interrupts
	rfm22_write(rfm22b_dev, RFM22_interrupt_enable1, 0x00);
	rfm22_write(rfm22b_dev, RFM22_interrupt_enable2, 0x00);

	// TUNE mode
	rfm22_write(rfm22b_dev, RFM22_op_and_func_ctrl1, RFM22_opfc1_pllon);

	// Queue the data up for sending
	rfm22b_dev->tx_data_wr = PH_PACKET_SIZE(rfm22b_dev->tx_packet);

	RX_LED_OFF;

	// Set the destination address in the transmit header.
	// The destination address is the first 4 bytes of the message.
	uint8_t *tx_buffer = (uint8_t*)(rfm22b_dev->tx_packet);
	rfm22_write(rfm22b_dev, RFM22_transmit_header0, tx_buffer[0]);
	rfm22_write(rfm22b_dev, RFM22_transmit_header1, tx_buffer[1]);
	rfm22_write(rfm22b_dev, RFM22_transmit_header2, tx_buffer[2]);
	rfm22_write(rfm22b_dev, RFM22_transmit_header3, tx_buffer[3]);

	// FIFO mode, GFSK modulation
	uint8_t fd_bit = rfm22_read(rfm22b_dev, RFM22_modulation_mode_control2) & RFM22_mmc2_fd;
	rfm22_write(rfm22b_dev, RFM22_modulation_mode_control2, fd_bit | RFM22_mmc2_dtmod_fifo |
		    RFM22_mmc2_modtyp_gfsk);

	// clear FIFOs
	rfm22_write(rfm22b_dev, RFM22_op_and_func_ctrl2, RFM22_opfc2_ffclrrx | RFM22_opfc2_ffclrtx);
	rfm22_write(rfm22b_dev, RFM22_op_and_func_ctrl2, 0x00);

	// *******************
	// add some data to the chips TX FIFO before enabling the transmitter

	// set the total number of data bytes we are going to transmit
	rfm22_write(rfm22b_dev, RFM22_transmit_packet_length, rfm22b_dev->tx_data_wr);

	// add some data
	rfm22_claimBus(rfm22b_dev);
	rfm22_assertCs(rfm22b_dev);
	PIOS_SPI_TransferByte(rfm22b_dev->spi_id, RFM22_fifo_access | 0x80);
	int bytes_to_write = (rfm22b_dev->tx_data_wr - rfm22b_dev->tx_data_rd);
	bytes_to_write = (bytes_to_write > FIFO_SIZE) ? FIFO_SIZE:  bytes_to_write;
	PIOS_SPI_TransferBlock(rfm22b_dev->spi_id, &tx_buffer[rfm22b_dev->tx_data_rd], NULL, bytes_to_write, NULL);
	rfm22b_dev->tx_data_rd += bytes_to_write;
	rfm22_deassertCs(rfm22b_dev);
	rfm22_releaseBus(rfm22b_dev);

	// enable TX interrupts
	rfm22_write(rfm22b_dev, RFM22_interrupt_enable1, RFM22_ie1_enpksent | RFM22_ie1_entxffaem);

	// enable the transmitter
	rfm22_write(rfm22b_dev, RFM22_op_and_func_ctrl1, RFM22_opfc1_pllon | RFM22_opfc1_txon);

	TX_LED_ON;

	return RFM22B_EVENT_NUM_EVENTS;
}

static void rfm22_sendStatus(struct pios_rfm22b_dev *rfm22b_dev)
{
	// The coordinator doesn't send status.
	if (rfm22b_dev->coordinator)
		return;

	// Update the link quality metric.
	rfm22_calculateLinkQuality(rfm22b_dev);

	// Queue the status message
	if (rfm22_isConnected(rfm22b_dev))
		rfm22b_dev->status_packet.header.destination_id = rfm22b_dev->destination_id;
	else if (rfm22b_dev->coordinator)
		return;
	else
		rfm22b_dev->status_packet.header.destination_id = 0xffffffff; // Broadcast
	rfm22b_dev->status_packet.header.type = PACKET_TYPE_STATUS;
	rfm22b_dev->status_packet.header.data_size = PH_STATUS_DATA_SIZE(&(rfm22b_dev->status_packet));
        rfm22b_dev->status_packet.source_id = rfm22b_dev->deviceID;
	rfm22b_dev->status_packet.link_quality = rfm22b_dev->stats.link_quality;
	rfm22b_dev->status_packet.received_rssi = rfm22b_dev->rssi_dBm;
	rfm22b_dev->send_status = true;

	return;
}

static void rfm22_sendPPM(struct pios_rfm22b_dev *rfm22b_dev)
{
#ifdef PIOS_PPM_RECEIVER
	// Only send PPM if we're connected
	if (!rfm22_isConnected(rfm22b_dev)) {
		return;
	}

	// Just return if the PPM receiver is not configured.
	if (PIOS_PPM_RECEIVER == 0) {
		return;
	}

	// See if we have any valid channels.
	bool valid_input_detected = false;
	for (uint8_t i = 1; i <= PIOS_PPM_NUM_INPUTS; ++i)
	{
		rfm22b_dev->ppm_packet.channels[i - 1] = PIOS_RCVR_Read(PIOS_PPM_RECEIVER, i);
		if(rfm22b_dev->ppm_packet.channels[i - 1] != PIOS_RCVR_TIMEOUT)
			valid_input_detected = true;
	}

	// Send the PPM packet if it's valid
	if (valid_input_detected)
	{
		rfm22b_dev->ppm_packet.header.destination_id = rfm22b_dev->destination_id;
		rfm22b_dev->ppm_packet.header.type = PACKET_TYPE_PPM;
		rfm22b_dev->ppm_packet.header.data_size = PH_PPM_DATA_SIZE(&(rfm22b_dev->ppm_packet));
		rfm22b_dev->send_ppm = true;
	}
#endif
}

/**
 * Read the RFM22B interrupt and device status registers
 * \param[in] rfm22b_dev  The device structure
 */
static bool rfm22_readStatus(struct pios_rfm22b_dev *rfm22b_dev)
{

	// 1. Read the interrupt statuses with burst read
	rfm22_claimBus(rfm22b_dev);  // Set RC and the semaphore
	uint8_t write_buf[3] = {RFM22_interrupt_status1 & 0x7f, 0xFF, 0xFF};
	uint8_t read_buf[3];
	rfm22_assertCs(rfm22b_dev);
	PIOS_SPI_TransferBlock(rfm22b_dev->spi_id, write_buf, read_buf, sizeof(write_buf), NULL);
	rfm22_deassertCs(rfm22b_dev);
	rfm22b_dev->int_status1 = read_buf[1];
	rfm22b_dev->int_status2 = read_buf[2];
	
	// Device status
	rfm22b_dev->device_status = rfm22_read_noclaim(rfm22b_dev, RFM22_device_status);

	// EzMAC status
	rfm22b_dev->ezmac_status = rfm22_read_noclaim(rfm22b_dev, RFM22_ezmac_status);

	// Release the bus
	rfm22_releaseBus(rfm22b_dev);

	// the RF module has gone and done a reset - we need to re-initialize the rf module
	if (rfm22b_dev->int_status2 & RFM22_is2_ipor)
		return false;

	return true;
}

/**
 * Add a status value to the RX packet status array.
 * \param[in] rfm22b_dev  The device structure
 * \param[in] status  The packet status value
 */
static void rfm22b_add_rx_status(struct pios_rfm22b_dev *rfm22b_dev, enum pios_rfm22b_rx_packet_status status)
{
	// Shift the status registers
	for (uint8_t i = RFM22B_RX_PACKET_STATS_LEN - 1; i > 0; --i)
	{
		rfm22b_dev->rx_packet_stats[i] = (rfm22b_dev->rx_packet_stats[i] << 2) | (rfm22b_dev->rx_packet_stats[i - 1] >> 30);
	}
	rfm22b_dev->rx_packet_stats[0] = (rfm22b_dev->rx_packet_stats[0] << 2) | status;
}

static enum pios_rfm22b_event rfm22_detectPreamble(struct pios_rfm22b_dev *rfm22b_dev)
{
	// Read the device status registers
	if (!rfm22_readStatus(rfm22b_dev))
		return RFM22B_EVENT_FAILURE;

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
		return RFM22B_EVENT_FAILURE;

	// Sync word detected
	if (rfm22b_dev->int_status2 & RFM22_is2_iswdet)
	{
		RX_LED_ON;

		// read the 10-bit signed afc correction value
		// bits 9 to 2
		uint16_t afc_correction = (uint16_t)rfm22_read(rfm22b_dev, RFM22_afc_correction_read) << 8;
		// bits 1 & 0
		afc_correction |= (uint16_t)rfm22_read(rfm22b_dev, RFM22_ook_counter_value1) & 0x00c0;
		afc_correction >>= 6;
		// convert the afc value to Hz
		int32_t afc_corr = (int32_t)(rfm22b_dev->frequency_step_size * afc_correction + 0.5f);
		rfm22b_dev->afc_correction_Hz = (afc_corr < -127) ? -127 : ((afc_corr > 127) ? 127 : afc_corr);

		// read rx signal strength .. 45 = -100dBm, 205 = -20dBm
		uint8_t rssi = rfm22_read(rfm22b_dev, RFM22_rssi);
		// convert to dBm
		rfm22b_dev->rssi_dBm = (int8_t)(rssi >> 1) - 122;

		return RFM22B_EVENT_SYNC_DETECTED;
	}
	else if (rfm22b_dev->int_status2 & !RFM22_is2_ipreaval)
		// Waiting for sync timed out.
		return RFM22B_EVENT_FAILURE;

	return RFM22B_EVENT_NUM_EVENTS;
}

static bool rfm22_receivePacket(struct pios_rfm22b_dev *rfm22b_dev, PHPacketHandle p, uint16_t rx_len)
{

	// Attempt to correct any errors in the packet.
	decode_data((unsigned char*)p, rx_len);

	bool good_packet = check_syndrome() == 0;
	bool corrected_packet = false;
	// We have an error.  Try to correct it.
	if(!good_packet && (correct_errors_erasures((unsigned char*)p, rx_len, 0, 0) != 0))
		// We corrected it
		corrected_packet = true;

	// Add any missed packets into the stats.
	bool ack_nack_packet = ((p->header.type == PACKET_TYPE_ACK) || (p->header.type == PACKET_TYPE_ACK_RTS) || (p->header.type == PACKET_TYPE_NACK));
	if (!ack_nack_packet && (good_packet || corrected_packet))
	{
		uint16_t seq_num = p->header.seq_num;
		if (rfm22_isConnected(rfm22b_dev)) {
			static bool first_time = true;
			uint16_t missed_packets = 0;
			if (first_time)
				first_time = false;
			else
			{
				uint16_t prev_seq_num = rfm22b_dev->stats.rx_seq;
				if (seq_num > prev_seq_num)
					missed_packets = seq_num - prev_seq_num - 1;
				else if((seq_num == prev_seq_num) && (p->header.type == PACKET_TYPE_DATA))
					p->header.type = PACKET_TYPE_DUPLICATE_DATA;
			}
			rfm22b_dev->stats.rx_missed += missed_packets;
		}
		rfm22b_dev->stats.rx_seq = seq_num;
	}

	// Set the packet status
	if (good_packet)
 		rfm22b_add_rx_status(rfm22b_dev, RFM22B_GOOD_RX_PACKET);
	else if(corrected_packet)
		// We corrected the error.
		rfm22b_add_rx_status(rfm22b_dev, RFM22B_CORRECTED_RX_PACKET);
	else
		// We couldn't correct the error, so drop the packet.
		rfm22b_add_rx_status(rfm22b_dev, RFM22B_ERROR_RX_PACKET);

	return (good_packet || corrected_packet);
}

static enum pios_rfm22b_event rfm22_rxData(struct pios_rfm22b_dev *rfm22b_dev)
{
	// Swap in the next packet buffer if required.
	uint8_t *rx_buffer = (uint8_t*)&(rfm22b_dev->rx_packet);

	// Read the device status registers
	if (!rfm22_readStatus(rfm22b_dev))
		return RFM22B_EVENT_FAILURE;

	// FIFO under/over flow error.  Restart RX mode.
	if (rfm22b_dev->int_status1 & RFM22_is1_ifferr)
		return RFM22B_EVENT_FAILURE;

	// RX FIFO almost full, it needs emptying
	if (rfm22b_dev->int_status1 & RFM22_is1_irxffafull)
	{
		// read data from the rf chips FIFO buffer
		// read the total length of the packet data
		uint16_t len = rfm22_read(rfm22b_dev, RFM22_received_packet_length);

		// The received packet is going to be larger than the specified length
		if ((rfm22b_dev->rx_buffer_wr + RX_FIFO_HI_WATERMARK) > len)
			return RFM22B_EVENT_FAILURE;

		// Another packet length error.
		if (((rfm22b_dev->rx_buffer_wr + RX_FIFO_HI_WATERMARK) >= len) && !(rfm22b_dev->int_status1 & RFM22_is1_ipkvalid))
			return RFM22B_EVENT_FAILURE;

		// Fetch the data from the RX FIFO
		rfm22_claimBus(rfm22b_dev);
		rfm22_assertCs(rfm22b_dev);
		PIOS_SPI_TransferByte(rfm22b_dev->spi_id,RFM22_fifo_access & 0x7F);
		rfm22b_dev->rx_buffer_wr += (PIOS_SPI_TransferBlock(rfm22b_dev->spi_id ,OUT_FF, (uint8_t *)&rx_buffer[rfm22b_dev->rx_buffer_wr], RX_FIFO_HI_WATERMARK, NULL) == 0) ? RX_FIFO_HI_WATERMARK : 0;
		rfm22_deassertCs(rfm22b_dev);
		rfm22_releaseBus(rfm22b_dev);
	}

	// CRC error .. discard the received data
	if (rfm22b_dev->int_status1 & RFM22_is1_icrerror)
		return RFM22B_EVENT_FAILURE;

	// Valid packet received
	if (rfm22b_dev->int_status1 & RFM22_is1_ipkvalid)
	{

		// read the total length of the packet data
		uint32_t len = rfm22_read(rfm22b_dev, RFM22_received_packet_length);

		// their must still be data in the RX FIFO we need to get
		if (rfm22b_dev->rx_buffer_wr < len)
		{
			int32_t bytes_to_read = len - rfm22b_dev->rx_buffer_wr;
			// Fetch the data from the RX FIFO
			rfm22_claimBus(rfm22b_dev);
			rfm22_assertCs(rfm22b_dev);
			PIOS_SPI_TransferByte(rfm22b_dev->spi_id,RFM22_fifo_access & 0x7F);
			rfm22b_dev->rx_buffer_wr += (PIOS_SPI_TransferBlock(rfm22b_dev->spi_id,OUT_FF, (uint8_t *)&rx_buffer[rfm22b_dev->rx_buffer_wr], bytes_to_read, NULL) == 0) ? bytes_to_read : 0;
			rfm22_deassertCs(rfm22b_dev);
			rfm22_releaseBus(rfm22b_dev);
		}
	
		if (rfm22b_dev->rx_buffer_wr != len)
			return RFM22B_EVENT_FAILURE;

		// we have a valid received packet
		enum pios_rfm22b_event ret_event = RFM22B_EVENT_RX_COMPLETE;
		if (rfm22b_dev->rx_buffer_wr > 0)
		{
			rfm22b_dev->stats.rx_byte_count += rfm22b_dev->rx_buffer_wr;
			// Check the packet for errors.
			if (rfm22_receivePacket(rfm22b_dev, &(rfm22b_dev->rx_packet), rfm22b_dev->rx_buffer_wr))
			{
				switch (rfm22b_dev->rx_packet.header.type)
				{
				case PACKET_TYPE_STATUS:
					ret_event = RFM22B_EVENT_STATUS_RECEIVED;
					break;
				case PACKET_TYPE_CON_REQUEST:
					ret_event = RFM22B_EVENT_CONNECTION_REQUESTED;
					break;
				case PACKET_TYPE_DATA:
				{
					// Send the data to the com port
					bool rx_need_yield;
					if (rfm22b_dev->rx_in_cb)
						(rfm22b_dev->rx_in_cb)(rfm22b_dev->rx_in_context, rfm22b_dev->rx_packet.data, rfm22b_dev->rx_packet.header.data_size, NULL, &rx_need_yield);
#ifdef RFM22B_TEST_DROPPED_PACKETS
					// Inject radnom missed ACKs
					{
						static uint8_t crc = 0;
						static uint8_t cntr = 0;
						crc = PIOS_CRC_updateByte(crc, cntr++);
					        if ((crc & 0x7) == 0)
							ret_event = RFM22B_EVENT_RX_MODE;
					}
#endif
					break;
				}
				case PACKET_TYPE_DUPLICATE_DATA:
					break;
				case PACKET_TYPE_ACK:
				case PACKET_TYPE_ACK_RTS:
					ret_event = RFM22B_EVENT_PACKET_ACKED;
					break;
				case PACKET_TYPE_NACK:
					ret_event = RFM22B_EVENT_PACKET_NACKED;
					break;
				case PACKET_TYPE_PPM:
				{
#if defined(PIOS_INCLUDE_GCSRCVR) || (defined(PIOS_INCLUDE_PPM_OUT) && defined(PIOS_PPM_OUTPUT)) || defined(PIOS_INCLUDE_RFM22B_RCVR)
					PHPpmPacketHandle ppmp = (PHPpmPacketHandle)&(rfm22b_dev->rx_packet);
#if defined(PIOS_INCLUDE_GCSRCVR) || (defined(PIOS_INCLUDE_PPM_OUT) && defined(PIOS_PPM_OUTPUT))
					bool ppm_output = false;
#endif
#endif
#if defined(PIOS_INCLUDE_RFM22B_RCVR)
					ppm_output = true;
					for (uint8_t i = 0; i < PIOS_RFM22B_RCVR_MAX_CHANNELS; ++i) {
						rfm22b_dev->ppm_channel[i] = ppmp->channels[i];
					}
#endif					
#if defined(PIOS_INCLUDE_PPM_OUT) && defined(PIOS_PPM_OUTPUT)
					if (PIOS_PPM_OUTPUT) {
						ppm_output = true;
						for (uint8_t i = 0; i < PIOS_RFM22B_RCVR_MAX_CHANNELS; ++i) {
							PIOS_PPM_OUT_Set(PIOS_PPM_OUTPUT, i, ppmp->channels[i]);
						}
					}
#endif
#if defined(PIOS_INCLUDE_GCSRCVR)
					if (!ppm_output) {
						GCSReceiverData gcsRcvr;
						for (uint8_t i = 0; (i < PIOS_RFM22B_RCVR_MAX_CHANNELS) && (i < GCSRECEIVER_CHANNEL_NUMELEM); ++i) {
							gcsRcvr.Channel[i] = ppmp->channels[i];
						}
						GCSReceiverSet(&gcsRcvr);                                        
					}
#endif
					break;
				}
				default:
					break;
				}

			}
			else
				ret_event = RFM22B_EVENT_RX_ERROR;
			rfm22b_dev->rx_buffer_wr = 0;
			rfm22b_dev->rx_complete_ticks = xTaskGetTickCount();
			if (rfm22b_dev->rx_complete_ticks == 0)
				rfm22b_dev->rx_complete_ticks = 1;
#if defined(PIOS_RFM22B_DEBUG_ON_TELEM) || defined(PIOS_RFM22B_DEBUG_ON_RCVR)
			D2_LED_OFF;
#endif
		}

		// We're finished with Rx mode
		rfm22b_dev->in_rx_mode = false;

		// Start a new transaction
		rfm22b_dev->packet_start_ticks = 0;
		return ret_event;
	}

	return RFM22B_EVENT_NUM_EVENTS;
}

static enum pios_rfm22b_event rfm22_rxFailure(struct pios_rfm22b_dev *rfm22b_dev)
{
	rfm22b_dev->stats.rx_failure++;
	rfm22b_dev->rx_buffer_wr = 0;
	rfm22b_dev->rx_complete_ticks = xTaskGetTickCount();
	rfm22b_dev->in_rx_mode = false;
	if (rfm22b_dev->rx_complete_ticks == 0)
		rfm22b_dev->rx_complete_ticks = 1;
	return RFM22B_EVENT_RX_MODE;
}

static enum pios_rfm22b_event rfm22_txData(struct pios_rfm22b_dev *rfm22b_dev)
{
	enum pios_rfm22b_event ret_event = RFM22B_EVENT_NUM_EVENTS;

	// Read the device status registers
	if (!rfm22_readStatus(rfm22b_dev))
		return RFM22B_EVENT_FAILURE;

	// TX FIFO almost empty, it needs filling up
	if (rfm22b_dev->int_status1 & RFM22_is1_ixtffaem)
	{
		// top-up the rf chips TX FIFO buffer
		uint8_t *tx_buffer = (uint8_t*)(rfm22b_dev->tx_packet);
		uint16_t max_bytes = FIFO_SIZE - TX_FIFO_LO_WATERMARK - 1;
		rfm22_claimBus(rfm22b_dev);
		rfm22_assertCs(rfm22b_dev);
		PIOS_SPI_TransferByte(rfm22b_dev->spi_id, RFM22_fifo_access | 0x80);
		int bytes_to_write = (rfm22b_dev->tx_data_wr - rfm22b_dev->tx_data_rd);
		bytes_to_write = (bytes_to_write > max_bytes) ? max_bytes:  bytes_to_write;
		PIOS_SPI_TransferBlock(rfm22b_dev->spi_id, &tx_buffer[rfm22b_dev->tx_data_rd], NULL, bytes_to_write, NULL);
		rfm22b_dev->tx_data_rd += bytes_to_write;
		rfm22_deassertCs(rfm22b_dev);
		rfm22_releaseBus(rfm22b_dev);
	}

	// Packet has been sent
	else if (rfm22b_dev->int_status1 & RFM22_is1_ipksent)
	{
		portTickType curTicks = xTaskGetTickCount();
		rfm22b_dev->stats.tx_byte_count += PH_PACKET_SIZE(rfm22b_dev->tx_packet);

		// Is this an ACK?
		bool is_ack = ((rfm22b_dev->tx_packet->header.type == PACKET_TYPE_ACK) || (rfm22b_dev->tx_packet->header.type == PACKET_TYPE_ACK_RTS));
		ret_event = RFM22B_EVENT_RX_MODE;
		if (is_ack) {

			// If this is an ACK for a connection request message we need to
			// configure this modem from the connection request message.
			if (rfm22b_dev->rx_packet.header.type == PACKET_TYPE_CON_REQUEST) {

				rfm22_setConnectionParameters(rfm22b_dev);

			} else if (rfm22b_dev->coordinator && !rfm22_isConnected(rfm22b_dev) && (rfm22b_dev->rx_packet.header.type == PACKET_TYPE_STATUS)) {

				// Send a connection request message if we're not connected, and this is a status message from a modem that we're bound to.
				PHStatusPacketHandle status = (PHStatusPacketHandle)&(rfm22b_dev->rx_packet);
				uint32_t source_id = status->source_id;
				for (uint8_t i = 0; OPLINKSETTINGS_BINDINGS_NUMELEM; ++i) {
					if (rfm22b_dev->bindings[i].pairID == source_id) {
						rfm22b_dev->cur_binding = i;
						ret_event = RFM22B_EVENT_REQUEST_CONNECTION;
						break;
					}
				}
			}

			// Change the channel
			// On the remote side, we initialize the time delta when we finish sending the ACK for the connection request message.
			if (rfm22b_dev->rx_packet.header.type == PACKET_TYPE_CON_REQUEST) {
				rfm22b_dev->time_delta = portMAX_DELAY - curTicks;
			}

		} else if (rfm22b_dev->tx_packet->header.type != PACKET_TYPE_NACK) {

			// We need to wait for an ACK if this packet it not an ACK or NACK.
			rfm22b_dev->prev_tx_packet = rfm22b_dev->tx_packet;
			rfm22b_dev->tx_complete_ticks = xTaskGetTickCount();
		}
		// Set the Tx period
		if (rfm22b_dev->tx_packet->header.type == PACKET_TYPE_ACK)
			rfm22b_dev->time_to_send_offset = curTicks + 0x4;
		else if (rfm22b_dev->tx_packet->header.type == PACKET_TYPE_ACK_RTS)
			rfm22b_dev->time_to_send_offset = curTicks;
		rfm22b_dev->tx_packet = 0;
		rfm22b_dev->tx_data_wr = rfm22b_dev->tx_data_rd = 0;
		// Start a new transaction
		rfm22b_dev->packet_start_ticks = 0;

#if defined(PIOS_RFM22B_DEBUG_ON_TELEM) || defined(PIOS_RFM22B_DEBUG_ON_RCVR)
		D1_LED_OFF;
#endif
	}

	return ret_event;
}

static enum pios_rfm22b_event rfm22_txFailure(struct pios_rfm22b_dev *rfm22b_dev)
{
	rfm22b_dev->stats.tx_failure++;
	rfm22b_dev->tx_data_wr = rfm22b_dev->tx_data_rd = 0;
	return RFM22B_EVENT_TX_START;
}

/**
 * Send an ACK to a received packet.
 * \param[in] rfm22b_dev  The device structure
 */
static enum pios_rfm22b_event rfm22_sendAck(struct pios_rfm22b_dev *rfm22b_dev)
{
	PHAckNackPacketHandle aph = (PHAckNackPacketHandle)(&(rfm22b_dev->ack_nack_packet));
	aph->header.destination_id = rfm22b_dev->destination_id;
	aph->header.type = rfm22_ready_to_send(rfm22b_dev) ? PACKET_TYPE_ACK_RTS : PACKET_TYPE_ACK;
	aph->header.data_size = PH_ACK_NACK_DATA_SIZE(aph);
	aph->header.seq_num = rfm22b_dev->rx_packet.header.seq_num;
	aph->packet_recv_time = rfm22_coordinatorTime(rfm22b_dev, rfm22b_dev->rx_complete_ticks);
	rfm22b_dev->tx_packet = (PHPacketHandle)aph;
	rfm22b_dev->time_to_send = true;
	return RFM22B_EVENT_TX_START;
}

/**
 * Send an NACK to a received packet.
 * \param[in] rfm22b_dev  The device structure
 */
static enum pios_rfm22b_event rfm22_sendNack(struct pios_rfm22b_dev *rfm22b_dev)
{
	PHAckNackPacketHandle aph = (PHAckNackPacketHandle)(&(rfm22b_dev->ack_nack_packet));
	aph->header.destination_id = rfm22b_dev->destination_id;
	aph->header.type = PACKET_TYPE_NACK;
	aph->header.data_size = PH_ACK_NACK_DATA_SIZE(aph);
	aph->header.seq_num = rfm22b_dev->rx_packet.header.seq_num;
	rfm22b_dev->tx_packet = (PHPacketHandle)aph;
	rfm22b_dev->time_to_send = true;
	return RFM22B_EVENT_TX_START;
}

/**
 * Receive an ACK.
 * \param[in] rfm22b_dev  The device structure
 */
static enum pios_rfm22b_event rfm22_receiveAck(struct pios_rfm22b_dev *rfm22b_dev)
{
	PHPacketHandle prev = rfm22b_dev->prev_tx_packet;
	portTickType curTicks = xTaskGetTickCount();

	// Clear the previous TX packet.
	rfm22b_dev->prev_tx_packet = NULL;

	// Was this a connection request?
	switch (prev->header.type) {
	case PACKET_TYPE_CON_REQUEST:
		rfm22_setConnectionParameters(rfm22b_dev);
		break;
	case PACKET_TYPE_DATA:
		rfm22b_dev->data_packet.header.data_size = 0;
		break;
	}

	// On the coordinator side, we initialize the time delta when we receive the ACK for the connection request message.
	if (prev->header.type == PACKET_TYPE_CON_REQUEST) {
		rfm22b_dev->time_delta = portMAX_DELAY - rfm22b_dev->rx_complete_ticks;
	} else if (!rfm22b_dev->coordinator) {
		PHAckNackPacketHandle aph = (PHAckNackPacketHandle)(&(rfm22b_dev->rx_packet));
		portTickType local_tx_time = rfm22_coordinatorTime(rfm22b_dev, rfm22b_dev->tx_complete_ticks);
		portTickType remote_rx_time = aph->packet_recv_time;
		// Adjust the time delta based on the difference between our estimated time offset and the coordinator offset.
		// This is not working yet
		rfm22b_dev->time_delta += remote_rx_time - local_tx_time;
	}

	// Reset the resend count
	rfm22b_dev->cur_resent_count = 0;

	// Should we try to start another TX?
	if (rfm22b_dev->rx_packet.header.type == PACKET_TYPE_ACK) {
		rfm22b_dev->time_to_send_offset = curTicks;
		rfm22b_dev->time_to_send = true;
		return RFM22B_EVENT_TX_START;
	} else {
		rfm22b_dev->time_to_send_offset = curTicks + 0x4;
		return RFM22B_EVENT_RX_MODE;
	}
}

/**
 * Receive an MACK.
 * \param[in] rfm22b_dev  The device structure
 */
static enum pios_rfm22b_event rfm22_receiveNack(struct pios_rfm22b_dev *rfm22b_dev)
{

	// Resend the previous TX packet.
	rfm22b_dev->tx_packet = rfm22b_dev->prev_tx_packet;
	rfm22b_dev->prev_tx_packet = NULL;

	// Increment the reset packet counter if we're connected.
	if (rfm22_isConnected(rfm22b_dev)) {
		rfm22b_add_rx_status(rfm22b_dev, RFM22B_RESENT_TX_PACKET);
	}
	rfm22b_dev->time_to_send = true;
	return RFM22B_EVENT_TX_START;
}

/**
 * Receive a status packet
 * \param[in] rfm22b_dev  The device structure
 */
static enum pios_rfm22b_event rfm22_receiveStatus(struct pios_rfm22b_dev *rfm22b_dev)
{
	PHStatusPacketHandle status = (PHStatusPacketHandle)&(rfm22b_dev->rx_packet);
	int8_t rssi = rfm22b_dev->rssi_dBm;
	int8_t afc = rfm22b_dev->afc_correction_Hz;
	uint32_t id = status->source_id;

	// Have we seen this device recently?
	bool found = false;
	uint8_t id_idx = 0;
	for ( ; id_idx < OPLINKSTATUS_PAIRIDS_NUMELEM; ++id_idx)
		if(rfm22b_dev->pair_stats[id_idx].pairID == id)
		{
			found = true;
			break;
		}

	// If we have seen it, update the RSSI and reset the last contact couter
	if(found)
	{
		rfm22b_dev->pair_stats[id_idx].rssi = rssi;
		rfm22b_dev->pair_stats[id_idx].afc_correction = afc;
		rfm22b_dev->pair_stats[id_idx].lastContact = 0;
	}

	// If we haven't seen it, find a slot to put it in.
	else
	{
		uint8_t min_idx = 0;
		int8_t min_rssi = rfm22b_dev->pair_stats[0].rssi;
		for (id_idx = 1; id_idx < OPLINKSTATUS_PAIRIDS_NUMELEM; ++id_idx)
		{
			if(rfm22b_dev->pair_stats[id_idx].rssi < min_rssi)
			{
				min_rssi = rfm22b_dev->pair_stats[id_idx].rssi;
				min_idx = id_idx;
			}
		}
		rfm22b_dev->pair_stats[min_idx].pairID = id;
		rfm22b_dev->pair_stats[min_idx].rssi = rssi;
		rfm22b_dev->pair_stats[min_idx].afc_correction = afc;
		rfm22b_dev->pair_stats[min_idx].lastContact = 0;
	}

	return RFM22B_EVENT_RX_COMPLETE;
}

static enum pios_rfm22b_event rfm22_requestConnection(struct pios_rfm22b_dev *rfm22b_dev)
{
	PHConnectionPacketHandle cph = &(rfm22b_dev->con_packet);

	// Set our connection state to requesting connection.
	rfm22b_dev->stats.link_state = OPLINKSTATUS_LINKSTATE_CONNECTING;

	// Fill in the connection request
	rfm22b_dev->destination_id = rfm22b_dev->bindings[rfm22b_dev->cur_binding].pairID;
	cph->header.destination_id = rfm22b_dev->destination_id;
	cph->header.type = PACKET_TYPE_CON_REQUEST;
	cph->header.data_size = PH_CONNECTION_DATA_SIZE(&(rfm22b_dev->con_packet));
        cph->source_id = rfm22b_dev->deviceID;
	cph->main_port = rfm22b_dev->bindings[rfm22b_dev->cur_binding].main_port;
	cph->flexi_port = rfm22b_dev->bindings[rfm22b_dev->cur_binding].flexi_port;
	cph->vcp_port = rfm22b_dev->bindings[rfm22b_dev->cur_binding].vcp_port;
	cph->com_speed = rfm22b_dev->bindings[rfm22b_dev->cur_binding].com_speed;
	rfm22b_dev->time_to_send = true;
	rfm22b_dev->send_connection_request = true;
	rfm22b_dev->prev_tx_packet = NULL;

	return RFM22B_EVENT_TX_START;
}

static void rfm22_setConnectionParameters(struct pios_rfm22b_dev *rfm22b_dev)
{
	PHConnectionPacketHandle cph = &(rfm22b_dev->con_packet);

	// Set our connection state to connected
	rfm22b_dev->stats.link_state = OPLINKSTATUS_LINKSTATE_CONNECTED;

	// Call the com port configuration function
	if (rfm22b_dev->com_config_cb)
		rfm22b_dev->com_config_cb(cph->main_port, cph->flexi_port, cph->vcp_port, cph->com_speed,
					  cph->min_frequency, cph->max_frequency, cph->channel_spacing);
 
 	// Configure this modem from the connection request message.
	rfm22_setNominalCarrierFrequency(rfm22b_dev, cph->min_frequency, cph->max_frequency, cph->channel_spacing);
	rfm22_setDatarate(rfm22b_dev, rfm22b_dev->datarate, true);
}

static portTickType rfm22_coordinatorTime(struct pios_rfm22b_dev *rfm22b_dev, portTickType ticks)
{
	return ticks + rfm22b_dev->time_delta;
}

static bool rfm22_inChannelBuffer(struct pios_rfm22b_dev *rfm22b_dev)
{
	portTickType time = rfm22_coordinatorTime(rfm22b_dev, xTaskGetTickCount());
	uint8_t window = (uint8_t)(time & 0x7e);
	return ((window == 0x7f) || (window == 0));
}

static uint8_t rfm22_calcChannel(struct pios_rfm22b_dev *rfm22b_dev)
{
	portTickType time = rfm22_coordinatorTime(rfm22b_dev, xTaskGetTickCount());
	// We change channels every 128 ms.
	uint16_t n = (time >> 7) & 0xffff;
	// The channel is calculated using the 16 bit CRC as the pseudo random number generator.
	n = PIOS_CRC16_updateByte(n, 0);
	float num_channels = rfm22b_dev->num_channels;
	return (uint8_t)(num_channels * (float)n / (float)0xffff);
}

static bool rfm22_changeChannel(struct pios_rfm22b_dev *rfm22b_dev)
{
	if (rfm22_isConnected(rfm22b_dev)) {
		return rfm22_setFreqHopChannel(rfm22b_dev, rfm22_calcChannel(rfm22b_dev));
	}
	return false;
}

static enum pios_rfm22b_event rfm22_acceptConnection(struct pios_rfm22b_dev *rfm22b_dev)
{
	// Set our connection state to connected
	rfm22b_dev->stats.link_state = OPLINKSTATUS_LINKSTATE_CONNECTED;

	// Copy the connection packet
	PHConnectionPacketHandle cph = (PHConnectionPacketHandle)(&(rfm22b_dev->rx_packet));
	PHConnectionPacketHandle lcph = (PHConnectionPacketHandle)(&(rfm22b_dev->con_packet));
	memcpy((uint8_t*)lcph, (uint8_t*)cph, PH_PACKET_SIZE((PHPacketHandle)cph));

	// Set the destination ID to the source of the connection request message.
	rfm22b_dev->destination_id = cph->source_id;

	return RFM22B_EVENT_DEFAULT;
}

// ************************************
// Initialise this hardware layer module and the rf module

static enum pios_rfm22b_event rfm22_init(struct pios_rfm22b_dev *rfm22b_dev)
{

	// Initialize the register values.
	rfm22b_dev->device_status = 0;
	rfm22b_dev->int_status1 = 0;
	rfm22b_dev->int_status2 = 0;
	rfm22b_dev->ezmac_status = 0;

	// Clean the LEDs
	rfm22_clearLEDs();

	// Initialize the detected device statistics.
	for (uint8_t i = 0; i < OPLINKSTATUS_PAIRIDS_NUMELEM; ++i) {
		rfm22b_dev->pair_stats[i].pairID = 0;
		rfm22b_dev->pair_stats[i].rssi = -127;
		rfm22b_dev->pair_stats[i].afc_correction = 0;
		rfm22b_dev->pair_stats[i].lastContact = 0;
	}

	// Initlize the link stats.
	for (uint8_t i = 0; i < RFM22B_RX_PACKET_STATS_LEN; ++i)
		rfm22b_dev->rx_packet_stats[i] = 0;

	// Initialize the state
	rfm22b_dev->stats.link_state = OPLINKSTATUS_LINKSTATE_DISCONNECTED;
	rfm22b_dev->destination_id = 0xffffffff;
	rfm22b_dev->time_to_send = false;
	rfm22b_dev->time_to_send_offset = 0;
	rfm22b_dev->send_status = false;
	rfm22b_dev->send_connection_request = false;
	rfm22b_dev->cur_resent_count = 0;

	// Initialize the packets.
	rfm22b_dev->rx_packet_len = 0;
	rfm22b_dev->tx_packet = NULL;
	rfm22b_dev->prev_tx_packet = NULL;
	rfm22b_dev->data_packet.header.data_size = 0;
	rfm22b_dev->in_rx_mode = false;

	// Initialize the devide state
	rfm22b_dev->device_status = rfm22b_dev->int_status1 = rfm22b_dev->int_status2 = rfm22b_dev->ezmac_status = 0;
	rfm22b_dev->rx_buffer_wr = 0;
	rfm22b_dev->tx_data_rd = rfm22b_dev->tx_data_wr = 0;
	rfm22b_dev->frequency_hop_channel = 0;
	rfm22b_dev->afc_correction_Hz = 0;
	rfm22b_dev->packet_start_ticks = 0;
	rfm22b_dev->tx_complete_ticks = 0;
	rfm22b_dev->rx_complete_ticks = 0;

	// software reset the RF chip .. following procedure according to Si4x3x Errata (rev. B)
	rfm22_write(rfm22b_dev, RFM22_op_and_func_ctrl1, RFM22_opfc1_swres);

	for (int i = 50; i > 0; i--) {
		// read the status registers
		rfm22b_dev->int_status1 = rfm22_read(rfm22b_dev, RFM22_interrupt_status1);
		rfm22b_dev->int_status2 = rfm22_read(rfm22b_dev, RFM22_interrupt_status2);
		if (rfm22b_dev->int_status2 & RFM22_is2_ichiprdy) break;

		// wait 1ms
		PIOS_DELAY_WaitmS(1);
	}

	// ****************

	// read status - clears interrupt
	rfm22b_dev->device_status = rfm22_read(rfm22b_dev, RFM22_device_status);
	rfm22b_dev->int_status1 = rfm22_read(rfm22b_dev, RFM22_interrupt_status1);
	rfm22b_dev->int_status2 = rfm22_read(rfm22b_dev, RFM22_interrupt_status2);
	rfm22b_dev->ezmac_status = rfm22_read(rfm22b_dev, RFM22_ezmac_status);

	// disable all interrupts
	rfm22_write(rfm22b_dev, RFM22_interrupt_enable1, 0x00);
	rfm22_write(rfm22b_dev, RFM22_interrupt_enable2, 0x00);

	// read the RF chip ID bytes

	// read the device type
	uint8_t device_type = rfm22_read(rfm22b_dev, RFM22_DEVICE_TYPE) & RFM22_DT_MASK;
	// read the device version
	uint8_t device_version = rfm22_read(rfm22b_dev, RFM22_DEVICE_VERSION) & RFM22_DV_MASK;

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

#ifdef PIOS_RFM22_NO_HEADER
	// header control - we are not using the header
	rfm22_write(rfm22b_dev, RFM22_header_control1, RFM22_header_cntl1_bcen_none | RFM22_header_cntl1_hdch_none);

	// no header bytes, synchronization word length 3, 2, 1 & 0 used, packet length included in header.
	rfm22_write(rfm22b_dev, RFM22_header_control2, RFM22_header_cntl2_hdlen_none |
		    RFM22_header_cntl2_synclen_3210 | ((TX_PREAMBLE_NIBBLES >> 8) & 0x01));
#else
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
	// Check all bit of all bytes of the header
	rfm22_write(rfm22b_dev, RFM22_header_enable0, 0xff);
	rfm22_write(rfm22b_dev, RFM22_header_enable1, 0xff);
	rfm22_write(rfm22b_dev, RFM22_header_enable2, 0xff);
	rfm22_write(rfm22b_dev, RFM22_header_enable3, 0xff);
	// Set the ID to be checked
	uint32_t id = rfm22b_dev->deviceID;
	rfm22_write(rfm22b_dev, RFM22_check_header0, id & 0xff);
	rfm22_write(rfm22b_dev, RFM22_check_header1, (id >> 8) & 0xff);
	rfm22_write(rfm22b_dev, RFM22_check_header2, (id >> 16) & 0xff);
	rfm22_write(rfm22b_dev, RFM22_check_header3, (id >> 24) & 0xff);
	// 4 header bytes, synchronization word length 3, 2, 1 & 0 used, packet length included in header.
	rfm22_write(rfm22b_dev, RFM22_header_control2,
		    RFM22_header_cntl2_hdlen_3210 |
		    RFM22_header_cntl2_synclen_3210 |
		    ((TX_PREAMBLE_NIBBLES >> 8) & 0x01));
#endif

	// sync word
	rfm22_write(rfm22b_dev, RFM22_sync_word3, SYNC_BYTE_1);
	rfm22_write(rfm22b_dev, RFM22_sync_word2, SYNC_BYTE_2);
	rfm22_write(rfm22b_dev, RFM22_sync_word1, SYNC_BYTE_3);
	rfm22_write(rfm22b_dev, RFM22_sync_word0, SYNC_BYTE_4);

	// set the tx power
	rfm22_write(rfm22b_dev, RFM22_tx_power, RFM22_tx_pwr_lna_sw | rfm22b_dev->tx_power);

	// TX FIFO Almost Full Threshold (0 - 63)
	rfm22_write(rfm22b_dev, RFM22_tx_fifo_control1, TX_FIFO_HI_WATERMARK);

	// TX FIFO Almost Empty Threshold (0 - 63)
	rfm22_write(rfm22b_dev, RFM22_tx_fifo_control2, TX_FIFO_LO_WATERMARK);

	// RX FIFO Almost Full Threshold (0 - 63)
	rfm22_write(rfm22b_dev, RFM22_rx_fifo_control, RX_FIFO_HI_WATERMARK);

	// Set the frequency calibration
	rfm22_write(rfm22b_dev, RFM22_xtal_osc_load_cap, rfm22b_dev->cfg.RFXtalCap);

	// Initialize the frequency and datarate to te default.
	rfm22_setNominalCarrierFrequency(rfm22b_dev, rfm22b_dev->init_frequency, rfm22b_dev->init_frequency, RFM22B_FREQUENCY_HOP_STEP_SIZE);
	rfm22_setDatarate(rfm22b_dev, RFM22B_DEFAULT_RX_DATARATE, true);

	return RFM22B_EVENT_INITIALIZED;
}

static void rfm22_clearLEDs() {
	LINK_LED_OFF;
	RX_LED_OFF;
	TX_LED_OFF;
#if defined(PIOS_RFM22B_DEBUG_ON_TELEM) || defined(PIOS_RFM22B_DEBUG_ON_RCVR)
	D1_LED_OFF;
	D2_LED_OFF;
	D3_LED_OFF;
	D4_LED_OFF;
#endif
}

static enum pios_rfm22b_event rfm22_timeout(struct pios_rfm22b_dev *rfm22b_dev)
{
	rfm22b_dev->stats.timeouts++;
	rfm22b_dev->packet_start_ticks = 0;
	// Release the Tx packet if it's set.
	if (rfm22b_dev->tx_packet != 0)
	{
		rfm22b_dev->tx_data_rd = rfm22b_dev->tx_data_wr = 0;
	}
	rfm22b_dev->rx_buffer_wr = 0;
	TX_LED_OFF;
	RX_LED_OFF;
#if defined(PIOS_RFM22B_DEBUG_ON_TELEM) || defined(PIOS_RFM22B_DEBUG_ON_RCVR)
	D1_LED_OFF;
	D2_LED_OFF;
	D3_LED_OFF;
	D4_LED_OFF;
#endif
	return RFM22B_EVENT_TX_START;
}

static enum pios_rfm22b_event rfm22_error(struct pios_rfm22b_dev *rfm22b_dev)
{
	rfm22b_dev->stats.resets++;
	rfm22_clearLEDs();
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
	rfm22_clearLEDs();
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

#endif /* PIOS_INCLUDE_RFM22B */

/**
 * @}
 * @}
 */

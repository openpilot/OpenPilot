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

#include <string.h>		// memmove

#include <stm32f10x.h>
#include <stopwatch.h>

#include <packet_handler.h>

#include <pios_rfm22b_priv.h>

/* Local Defines */
#define STACK_SIZE_BYTES  200

// RTC timer is running at 625Hz (1.6ms or 5 ticks == 8ms).
// A 256 byte message at 56kbps should take less than 40ms
// Note: This timeout should be rate dependent.
#define PIOS_RFM22B_SUPERVISOR_TIMEOUT 65  // ~100ms

// this is too adjust the RF module so that it is on frequency
#define OSC_LOAD_CAP					0x7F	// cap = 12.5pf .. default
#define OSC_LOAD_CAP_1				0x7D	// board 1
#define OSC_LOAD_CAP_2				0x7B	// board 2
#define OSC_LOAD_CAP_3				0x7E	// board 3
#define OSC_LOAD_CAP_4				0x7F	// board 4

// ************************************

#define TX_TEST_MODE_TIMELIMIT_MS		30000	// TX test modes time limit (in ms)

//#define TX_PREAMBLE_NIBBLES				8		// 7 to 511 (number of nibbles)
//#define RX_PREAMBLE_NIBBLES				5		// 5 to 31 (number of nibbles)
#define TX_PREAMBLE_NIBBLES				12		// 7 to 511 (number of nibbles)
#define RX_PREAMBLE_NIBBLES				6		// 5 to 31 (number of nibbles)

// the size of the rf modules internal transmit buffers.
#define TX_BUFFER_SIZE 256
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

// ************************************
// the default TX power level

#define RFM22_DEFAULT_RF_POWER			RFM22_tx_pwr_txpow_0    // +1dBm ... 1.25mW
//#define RFM22_DEFAULT_RF_POWER		RFM22_tx_pwr_txpow_1    // +2dBm ... 1.6mW
//#define RFM22_DEFAULT_RF_POWER		RFM22_tx_pwr_txpow_2    // +5dBm ... 3.16mW
//#define RFM22_DEFAULT_RF_POWER		RFM22_tx_pwr_txpow_3    // +8dBm ... 6.3mW
//#define RFM22_DEFAULT_RF_POWER		RFM22_tx_pwr_txpow_4    // +11dBm .. 12.6mW
//#define RFM22_DEFAULT_RF_POWER		RFM22_tx_pwr_txpow_5    // +14dBm .. 25mW
//#define RFM22_DEFAULT_RF_POWER		RFM22_tx_pwr_txpow_6    // +17dBm .. 50mW
//#define RFM22_DEFAULT_RF_POWER		RFM22_tx_pwr_txpow_7    // +20dBm .. 100mW

// ************************************
// the default RF datarate

//#define RFM22_DEFAULT_RF_DATARATE       500          // 500 bits per sec
//#define RFM22_DEFAULT_RF_DATARATE       1000         // 1k bits per sec
//#define RFM22_DEFAULT_RF_DATARATE       2000         // 2k bits per sec
//#define RFM22_DEFAULT_RF_DATARATE       4000         // 4k bits per sec
//#define RFM22_DEFAULT_RF_DATARATE       8000         // 8k bits per sec
//#define RFM22_DEFAULT_RF_DATARATE       9600         // 9.6k bits per sec
//#define RFM22_DEFAULT_RF_DATARATE       16000        // 16k bits per sec
//#define RFM22_DEFAULT_RF_DATARATE       19200        // 19k2 bits per sec
//#define RFM22_DEFAULT_RF_DATARATE       24000        // 24k bits per sec
//#define RFM22_DEFAULT_RF_DATARATE       32000        // 32k bits per sec
//#define RFM22_DEFAULT_RF_DATARATE       64000        // 64k bits per sec
#define RFM22_DEFAULT_RF_DATARATE       128000       // 128k bits per sec
//#define RFM22_DEFAULT_RF_DATARATE       192000       // 192k bits per sec
//#define RFM22_DEFAULT_RF_DATARATE       256000       // 256k bits per sec .. NOT YET WORKING

// ************************************

#define RFM22_DEFAULT_SS_RF_DATARATE       125			// 128bps

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

struct pios_rfm22b_dev {
	enum pios_rfm22b_dev_magic magic;
	struct pios_rfm22b_cfg cfg;

	uint32_t deviceID;

	// The COM callback functions.
	pios_com_callback rx_in_cb;
	uint32_t rx_in_context;
	pios_com_callback tx_out_cb;
	uint32_t tx_out_context;

	// The supervisor countdown timer.
	uint16_t supv_timer;
	uint16_t resets;
};

uint32_t random32 = 0x459ab8d8;

/* Local function forwared declarations */
static void PIOS_RFM22B_Supervisor(uint32_t ppm_id);
static void rfm22_processInt(void);
static bool PIOS_RFM22_EXT_Int(void);
static void rfm22_setTxMode(uint8_t mode);

// SPI read/write functions
void rfm22_startBurstWrite(uint8_t addr);
inline void rfm22_burstWrite(uint8_t data)
{
	PIOS_SPI_TransferByte(RFM22_PIOS_SPI, data);
}
void rfm22_endBurstWrite(void);
void rfm22_write(uint8_t addr, uint8_t data);
void rfm22_startBurstRead(uint8_t addr);
inline uint8_t rfm22_burstRead(void)
{
	return PIOS_SPI_TransferByte(RFM22_PIOS_SPI, 0xff);
}
void rfm22_endBurstRead(void);
uint8_t rfm22_read(uint8_t addr);
uint8_t rfm22_txStart();

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

// External interrupt configuration
static const struct pios_exti_cfg pios_exti_rfm22b_cfg __exti_config = {
	.vector = PIOS_RFM22_EXT_Int,
	.line = PIOS_RFM22_EXTI_LINE,
	.pin = {
		.gpio = PIOS_RFM22_EXTI_GPIO_PORT,
		.init = {
			.GPIO_Pin = PIOS_RFM22_EXTI_GPIO_PIN,
			.GPIO_Mode = GPIO_Mode_IN_FLOATING,
		},
	},
	.irq = {
		.init = {
			.NVIC_IRQChannel = PIOS_RFM22_EXTI_IRQn,
			.NVIC_IRQChannelPreemptionPriority = PIOS_RFM22_EXTI_PRIO,
			.NVIC_IRQChannelSubPriority = 0,
			.NVIC_IRQChannelCmd = ENABLE,
		},
	},
	.exti = {
		.init = {
			.EXTI_Line = PIOS_RFM22_EXTI_LINE,
			.EXTI_Mode = EXTI_Mode_Interrupt,
			.EXTI_Trigger = EXTI_Trigger_Falling,
			.EXTI_LineCmd = ENABLE,
		},
	},
};

// xtal 10 ppm, 434MHz
#define LOOKUP_SIZE	 14
const uint32_t            data_rate[] = {   500,  1000,  2000,  4000,  8000,  9600, 16000, 19200, 24000,  32000,  64000, 128000, 192000, 256000};
const uint8_t      modulation_index[] = {    16,     8,     4,     2,     1,     1,     1,     1,     1,      1,      1,      1,      1,      1};
const uint32_t       freq_deviation[] = {  4000,  4000,  4000,  4000,  4000,  4800,  8000,  9600, 12000,  16000,  32000,  64000,  96000, 128000};
const uint32_t         rx_bandwidth[] = { 17500, 17500, 17500, 17500, 17500, 19400, 32200, 38600, 51200,  64100, 137900, 269300, 420200, 518800};
const int8_t        est_rx_sens_dBm[] = {  -118,  -118,  -117,  -116,  -115,  -115,  -112,  -112,  -110,   -109,   -106,   -103,   -101,   -100}; // estimated receiver sensitivity for BER = 1E-3

const uint8_t                reg_1C[] = {  0x37,  0x37,  0x37,  0x37,  0x3A,  0x3B,  0x26,  0x28,  0x2E,   0x16,   0x07,   0x83,   0x8A,   0x8C}; // rfm22_if_filter_bandwidth

const uint8_t                reg_1D[] = {  0x44,  0x44,  0x44,  0x44,  0x44,  0x44,  0x44,  0x44,  0x44,   0x44,   0x44,   0x44,   0x44,   0x44}; // rfm22_afc_loop_gearshift_override
const uint8_t                reg_1E[] = {  0x0A,  0x0A,  0x0A,  0x0A,  0x0A,  0x0A,  0x0A,  0x0A,  0x0A,   0x0A,   0x0A,   0x0A,   0x0A,   0x02}; // rfm22_afc_timing_control

const uint8_t                reg_1F[] = {  0x03,  0x03,  0x03,  0x03,  0x03,  0x03,  0x03,  0x03,  0x03,   0x03,   0x03,   0x03,   0x03,   0x03}; // rfm22_clk_recovery_gearshift_override
const uint8_t                reg_20[] = {  0xE8,  0xF4,  0xFA,  0x70,  0x3F,  0x34,  0x3F,  0x34,  0x2A,   0x3F,   0x3F,   0x5E,   0x3F,   0x2F}; // rfm22_clk_recovery_oversampling_ratio
const uint8_t                reg_21[] = {  0x60,  0x20,  0x00,  0x01,  0x02,  0x02,  0x02,  0x02,  0x03,   0x02,   0x02,   0x01,   0x02,   0x02}; // rfm22_clk_recovery_offset2
const uint8_t                reg_22[] = {  0x20,  0x41,  0x83,  0x06,  0x0C,  0x75,  0x0C,  0x75,  0x12,   0x0C,   0x0C,   0x5D,   0x0C,   0xBB}; // rfm22_clk_recovery_offset1
const uint8_t                reg_23[] = {  0xC5,  0x89,  0x12,  0x25,  0x4A,  0x25,  0x4A,  0x25,  0x6F,   0x4A,   0x4A,   0x86,   0x4A,   0x0D}; // rfm22_clk_recovery_offset0
const uint8_t                reg_24[] = {  0x00,  0x00,  0x00,  0x02,  0x07,  0x07,  0x07,  0x07,  0x07,   0x07,   0x07,   0x05,   0x07,   0x07}; // rfm22_clk_recovery_timing_loop_gain1
const uint8_t                reg_25[] = {  0x0A,  0x23,  0x85,  0x0E,  0xFF,  0xFF,  0xFF,  0xFF,  0xFF,   0xFF,   0xFF,   0x74,   0xFF,   0xFF}; // rfm22_clk_recovery_timing_loop_gain0

const uint8_t                reg_2A[] = {  0x0E,  0x0E,  0x0E,  0x0E,  0x0E,  0x0D,  0x0D,  0x0E,  0x12,   0x17,   0x31,   0x50,   0x50,   0x50}; // rfm22_afc_limiter .. AFC_pull_in_range = ±AFCLimiter[7:0] x (hbsel+1) x 625 Hz

const uint8_t                reg_6E[] = {  0x04,  0x08,  0x10,  0x20,  0x41,  0x4E,  0x83,  0x9D,  0xC4,   0x08,   0x10,   0x20,   0x31,   0x41}; // rfm22_tx_data_rate1
const uint8_t                reg_6F[] = {  0x19,  0x31,  0x62,  0xC5,  0x89,  0xA5,  0x12,  0x49,  0x9C,   0x31,   0x62,   0xC5,   0x27,   0x89}; // rfm22_tx_data_rate0

const uint8_t                reg_70[] = {  0x2D,  0x2D,  0x2D,  0x2D,  0x2D,  0x2D,  0x2D,  0x2D,  0x2D,   0x0D,   0x0D,   0x0D,   0x0D,   0x0D}; // rfm22_modulation_mode_control1
const uint8_t                reg_71[] = {  0x23,  0x23,  0x23,  0x23,  0x23,  0x23,  0x23,  0x23,  0x23,   0x23,   0x23,   0x23,   0x23,   0x23}; // rfm22_modulation_mode_control2

const uint8_t                reg_72[] = {  0x06,  0x06,  0x06,  0x06,  0x06,  0x08,  0x0D,  0x0F,  0x13,   0x1A,   0x33,   0x66,   0x9A,   0xCD}; // rfm22_frequency_deviation

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
const uint32_t ss_rx_bandwidth[] = {  2600, 10600};

const uint8_t ss_reg_1C[] = {  0x51, 0x32}; // rfm22_if_filter_bandwidth
const uint8_t ss_reg_1D[] = {  0x00, 0x00}; // rfm22_afc_loop_gearshift_override

const uint8_t ss_reg_20[] = {  0xE8, 0x38}; // rfm22_clk_recovery_oversampling_ratio
const uint8_t ss_reg_21[] = {  0x60, 0x02}; // rfm22_clk_recovery_offset2
const uint8_t ss_reg_22[] = {  0x20, 0x4D}; // rfm22_clk_recovery_offset1
const uint8_t ss_reg_23[] = {  0xC5, 0xD3}; // rfm22_clk_recovery_offset0
const uint8_t ss_reg_24[] = {  0x00, 0x07}; // rfm22_clk_recovery_timing_loop_gain1
const uint8_t ss_reg_25[] = {  0x0F, 0xFF}; // rfm22_clk_recovery_timing_loop_gain0

const uint8_t ss_reg_2A[] = {  0xFF, 0xFF}; // rfm22_afc_limiter .. AFC_pull_in_range = ±AFCLimiter[7:0] x (hbsel+1) x 625 Hz

const uint8_t ss_reg_70[] = {  0x24, 0x2D}; // rfm22_modulation_mode_control1
const uint8_t ss_reg_71[] = {  0x2B, 0x23}; // rfm22_modulation_mode_control2

// ************************************

volatile bool		initialized = false;

#if defined(RFM22_EXT_INT_USE)
volatile bool		exec_using_spi;					// set this if you want to access the SPI bus outside of the interrupt
#endif

uint8_t				device_type;						// the RF chips device ID number
uint8_t				device_version;						// the RF chips revision number

volatile uint8_t	rf_mode;							// holds our current RF mode

uint32_t			lower_carrier_frequency_limit_Hz;	// the minimum RF frequency we can use
uint32_t			upper_carrier_frequency_limit_Hz;	// the maximum RF frequency we can use
uint32_t			carrier_frequency_hz;				// the current RF frequency we are on

uint32_t			carrier_datarate_bps;				// the RF data rate we are using

uint32_t			rf_bandwidth_used;					// the RF bandwidth currently used
uint32_t			ss_rf_bandwidth_used;					// the RF bandwidth currently used

uint8_t				hbsel;								// holds the hbsel (1 or 2)
float				frequency_step_size;				// holds the minimum frequency step size

uint8_t				frequency_hop_channel;				// current frequency hop channel

uint8_t				frequency_hop_step_size_reg;		//

uint8_t				adc_config;							// holds the adc config reg value

volatile uint8_t	device_status;						// device status register
volatile uint8_t	int_status1;						// interrupt status register 1
volatile uint8_t	int_status2;						// interrupt status register 2
volatile uint8_t	ezmac_status;						// ezmac status register

volatile int16_t	afc_correction;						// afc correction reading
volatile int32_t	afc_correction_Hz;					// afc correction reading (in Hz)

volatile int16_t	temperature_reg;					// the temperature sensor reading

#if defined(RFM22_DEBUG)
volatile uint8_t	prev_device_status;				// just for debugging
volatile uint8_t	prev_int_status1;				//  "          "
volatile uint8_t	prev_int_status2;				//  "          "
volatile uint8_t	prev_ezmac_status;				//  "          "

const char *debug_msg = "";
const char *error_msg = "";
static uint32_t debug_val = 0;
#endif

volatile uint8_t	osc_load_cap;						// xtal frequency calibration value

volatile uint8_t	rssi;								// the current RSSI (register value)
volatile int8_t	rssi_dBm;							// dBm value

// the transmit power to use for data transmissions
uint8_t				tx_power;
// the tx power register read back
volatile uint8_t	tx_pwr;

// The transmit buffer. Holds data that is being transmitted.
uint8_t tx_buffer[TX_BUFFER_SIZE] __attribute__ ((aligned(4)));
// The transmit buffer.  Hosts data that is wating to be transmitted.
uint8_t tx_pre_buffer[TX_BUFFER_SIZE] __attribute__ ((aligned(4)));
// The tx pre-buffer write index.
uint16_t tx_pre_buffer_size;
// the tx data read index
volatile uint16_t	tx_data_rd;
// the tx data write index
volatile uint16_t	tx_data_wr;

// the current receive buffer in use (double buffer)
volatile uint8_t rx_buffer_current;
// the receive buffer .. received packet data is saved here
volatile uint8_t rx_buffer[258] __attribute__ ((aligned(4)));
// the receive buffer write index
volatile uint16_t	rx_buffer_wr;

// the received packet
volatile int8_t	rx_packet_start_rssi_dBm;							//
volatile int8_t	rx_packet_start_afc_Hz;								//
volatile int8_t	rx_packet_rssi_dBm;									// the received packet signal strength
volatile int8_t	rx_packet_afc_Hz;									// the receive packet frequency offset

int					lookup_index;
int					ss_lookup_index;

volatile bool		power_on_reset;						// set if the RF module has reset itself

volatile uint16_t	rfm22_int_timer;					// used to detect if the RF module stops responding. thus act accordingly if it does stop responding.
volatile uint16_t	rfm22_int_time_outs;				// counter
volatile uint16_t	prev_rfm22_int_time_outs;			//

uint16_t			timeout_ms = 20000;					//
uint16_t			timeout_sync_ms = 3;				//
uint16_t			timeout_data_ms = 20;				//

struct pios_rfm22b_dev * rfm22b_dev_g;


static bool PIOS_RFM22B_validate(struct pios_rfm22b_dev * rfm22b_dev)
{
	return (rfm22b_dev->magic == PIOS_RFM22B_DEV_MAGIC);
}

#if defined(PIOS_INCLUDE_FREERTOS)
static struct pios_rfm22b_dev * PIOS_RFM22B_alloc(void)
{
	struct pios_rfm22b_dev * rfm22b_dev;

	rfm22b_dev = (struct pios_rfm22b_dev *)pvPortMalloc(sizeof(*rfm22b_dev));
	if (!rfm22b_dev) return(NULL);
	rfm22b_dev_g = rfm22b_dev;

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

/**
 * Initialise an RFM22B device
 */
int32_t PIOS_RFM22B_Init(uint32_t *rfm22b_id, const struct pios_rfm22b_cfg *cfg)
{
	PIOS_DEBUG_Assert(rfm22b_id);
	PIOS_DEBUG_Assert(cfg);

	// Allocate the device structure.
	struct pios_rfm22b_dev * rfm22b_dev = (struct pios_rfm22b_dev *) PIOS_RFM22B_alloc();
	if (!rfm22b_dev)
		return(-1);

	// Bind the configuration to the device instance
	rfm22b_dev->cfg = *cfg;
  
	*rfm22b_id = (uint32_t)rfm22b_dev;

	// Initialize the TX pre-buffer pointer.
	tx_pre_buffer_size = 0;

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

	// Initialize the supervisor timer.
	rfm22b_dev->supv_timer = PIOS_RFM22B_SUPERVISOR_TIMEOUT;
	rfm22b_dev->resets = 0;

	// Initialize the radio device.
	int initval = rfm22_init_normal(rfm22b_dev->deviceID, cfg->minFrequencyHz, cfg->maxFrequencyHz, 50000);

	if (initval < 0)
	{

		// RF module error .. flash the LED's
#if defined(PIOS_COM_DEBUG)
		DEBUG_PRINTF(2, "RF ERROR res: %d\n\r\n\r", initval);
#endif

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

		return initval;
	}

	rfm22_setFreqCalibration(cfg->RFXtalCap);
	rfm22_setNominalCarrierFrequency(cfg->frequencyHz);
	rfm22_setDatarate(cfg->maxRFBandwidth, TRUE);
	rfm22_setTxPower(cfg->maxTxPower);

	DEBUG_PRINTF(2, "\n\r");
	DEBUG_PRINTF(2, "RF device ID: %x\n\r", rfm22b_dev->deviceID);
	DEBUG_PRINTF(2, "RF datarate: %dbps\n\r", rfm22_getDatarate());
	DEBUG_PRINTF(2, "RF frequency: %dHz\n\r", rfm22_getNominalCarrierFrequency());
	DEBUG_PRINTF(2, "RF TX power: %d\n\r", rfm22_getTxPower());

	// Setup a real-time clock callback to kickstart the radio if a transfer lock sup.
	if (!PIOS_RTC_RegisterTickCallback(PIOS_RFM22B_Supervisor, *rfm22b_id)) {
		PIOS_DEBUG_Assert(0);
	}

	return(0);
}

uint32_t PIOS_RFM22B_DeviceID(uint32_t rfm22b_id)
{
	struct pios_rfm22b_dev *rfm22b_dev = (struct pios_rfm22b_dev *)rfm22b_id;

	return rfm22b_dev->deviceID;
}

int8_t PIOS_RFM22B_RSSI(uint32_t rfm22b_id)
{
	return rfm22_receivedRSSI();
}

int16_t PIOS_RFM22B_Resets(uint32_t rfm22b_id)
{
	struct pios_rfm22b_dev *rfm22b_dev = (struct pios_rfm22b_dev *)rfm22b_id;

	return rfm22b_dev->resets;
}

static void PIOS_RFM22B_RxStart(uint32_t rfm22b_id, uint16_t rx_bytes_avail)
{
	struct pios_rfm22b_dev * rfm22b_dev = (struct pios_rfm22b_dev *)rfm22b_id;

	bool valid = PIOS_RFM22B_validate(rfm22b_dev);
	PIOS_Assert(valid);

}

static void PIOS_RFM22B_TxStart(uint32_t rfm22b_id, uint16_t tx_bytes_avail)
{
	struct pios_rfm22b_dev * rfm22b_dev = (struct pios_rfm22b_dev *)rfm22b_id;
	bool valid = PIOS_RFM22B_validate(rfm22b_dev);
	PIOS_Assert(valid);

	// Get some data to send
	bool need_yield = false;
	if(tx_pre_buffer_size == 0)
		tx_pre_buffer_size = (rfm22b_dev->tx_out_cb)(rfm22b_dev->tx_out_context, tx_pre_buffer,
							     TX_BUFFER_SIZE, NULL, &need_yield);

	if(tx_pre_buffer_size > 0)
	{
		// already have data to be sent
		if (tx_data_wr > 0)
			return;

		// we are currently transmitting or scanning the spectrum
		if (rf_mode == TX_DATA_MODE || rf_mode == TX_STREAM_MODE || rf_mode == TX_CARRIER_MODE ||
		    rf_mode == TX_PN_MODE || rf_mode == RX_SCAN_SPECTRUM)
			return;

		// is the channel clear to transmit on?
		if (!rfm22_channelIsClear())
			return;

		// Start the transmit
		rfm22_txStart();
	}
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

static void PIOS_RFM22B_Supervisor(uint32_t rfm22b_id)
{
	/* Recover our device context */
	struct pios_rfm22b_dev * rfm22b_dev = (struct pios_rfm22b_dev *)rfm22b_id;

	if (!PIOS_RFM22B_validate(rfm22b_dev)) {
		/* Invalid device specified */
		return;
	}

	/* If we're waiting for a receive, we just need to make sure that there are no packets waiting to be transmitted. */
	if(rf_mode == RX_WAIT_PREAMBLE_MODE)
	{
		/* Start a packet transfer if one is available. */
		PIOS_RFM22B_TxStart(rfm22b_id, 0);
		return;
	}

	/* The radio must be locked up if the timer reaches 0 */
	if(--(rfm22b_dev->supv_timer) != 0)
		return;
	++(rfm22b_dev->resets);

	TX_LED_OFF;
	TX_LED_OFF;

	/* Clear the TX buffer in case we locked up in a transmit */
	tx_data_wr = 0;

	rfm22_init_normal(rfm22b_dev->deviceID, rfm22b_dev->cfg.minFrequencyHz, rfm22b_dev->cfg.maxFrequencyHz, 50000);

	/* Start a packet transfer if one is available. */
	rf_mode = RX_WAIT_PREAMBLE_MODE;
	PIOS_RFM22B_TxStart(rfm22b_id, 0);
	if(rf_mode == RX_WAIT_PREAMBLE_MODE)
	{
		/* Switch to RX mode */
		rfm22_setRxMode(RX_WAIT_PREAMBLE_MODE, false);
	}
}

static void rfm22_setDebug(const char* msg)
{
	debug_msg = msg;
}

static void rfm22_setError(const char* msg)
{
	error_msg = msg;
}

// ************************************
// SPI read/write

void rfm22_startBurstWrite(uint8_t addr)
{
	// wait 1us .. so we don't toggle the CS line to quickly
	PIOS_DELAY_WaituS(1);

	// chip select line LOW
	PIOS_SPI_RC_PinSet(RFM22_PIOS_SPI, 0, 0);

	PIOS_SPI_TransferByte(RFM22_PIOS_SPI, 0x80 | addr);
}

void rfm22_endBurstWrite(void)
{
	// chip select line HIGH
	PIOS_SPI_RC_PinSet(RFM22_PIOS_SPI, 0, 1);
}

void rfm22_write(uint8_t addr, uint8_t data)
{
	// wait 1us .. so we don't toggle the CS line to quickly
	PIOS_DELAY_WaituS(1);

	// chip select line LOW
	PIOS_SPI_RC_PinSet(RFM22_PIOS_SPI, 0, 0);

	PIOS_SPI_TransferByte(RFM22_PIOS_SPI, 0x80 | addr);
	PIOS_SPI_TransferByte(RFM22_PIOS_SPI, data);

	// chip select line HIGH
	PIOS_SPI_RC_PinSet(RFM22_PIOS_SPI, 0, 1);
}

void rfm22_startBurstRead(uint8_t addr)
{
	// wait 1us .. so we don't toggle the CS line to quickly
	PIOS_DELAY_WaituS(1);

	// chip select line LOW
	PIOS_SPI_RC_PinSet(RFM22_PIOS_SPI, 0, 0);

	PIOS_SPI_TransferByte(RFM22_PIOS_SPI, addr & 0x7f);
}

void rfm22_endBurstRead(void)
{
	// chip select line HIGH
	PIOS_SPI_RC_PinSet(RFM22_PIOS_SPI, 0, 1);
}

uint8_t rfm22_read(uint8_t addr)
{
	uint8_t rdata;

	// wait 1us .. so we don't toggle the CS line to quickly
	PIOS_DELAY_WaituS(1);

	// chip select line LOW
	PIOS_SPI_RC_PinSet(RFM22_PIOS_SPI, 0, 0);

	PIOS_SPI_TransferByte(RFM22_PIOS_SPI, addr & 0x7f);
	rdata = PIOS_SPI_TransferByte(RFM22_PIOS_SPI, 0xff);

	// chip select line HIGH
	PIOS_SPI_RC_PinSet(RFM22_PIOS_SPI, 0, 1);

	return rdata;
}

// ************************************
// external interrupt


static bool PIOS_RFM22_EXT_Int(void)
{
	rfm22_setDebug("Ext Int");
	if (!exec_using_spi)
		rfm22_processInt();
	rfm22_setDebug("Ext Done");
	return false;
}

void rfm22_disableExtInt(void)
{
#if defined(RFM22_EXT_INT_USE)
	rfm22_setDebug("Disable Int");
	// Configure the external interrupt
	GPIO_EXTILineConfig(PIOS_RFM22_EXTI_PORT_SOURCE, PIOS_RFM22_EXTI_PIN_SOURCE);
	EXTI_InitTypeDef EXTI_InitStructure = pios_exti_rfm22b_cfg.exti.init;
	EXTI_InitStructure.EXTI_LineCmd = DISABLE;
	EXTI_Init(&EXTI_InitStructure);

	EXTI_ClearFlag(PIOS_RFM22_EXTI_LINE);
	rfm22_setDebug("Disable Int done");
#endif
}

void rfm22_enableExtInt(void)
{
#if defined(RFM22_EXT_INT_USE)
	rfm22_setDebug("Ensable Int");
	if (PIOS_EXTI_Init(&pios_exti_rfm22b_cfg))
		PIOS_Assert(0);
	rfm22_setDebug("Ensable Int done");
#endif
}


// ************************************
// set/get the current tx power setting

void rfm22_setTxPower(uint8_t tx_pwr)
{
	switch (tx_pwr)
	{
	case 0: tx_power = RFM22_tx_pwr_txpow_0; break;    // +1dBm ... 1.25mW
	case 1: tx_power = RFM22_tx_pwr_txpow_1; break;    // +2dBm ... 1.6mW
	case 2: tx_power = RFM22_tx_pwr_txpow_2; break;    // +5dBm ... 3.16mW
	case 3: tx_power = RFM22_tx_pwr_txpow_3; break;    // +8dBm ... 6.3mW
	case 4: tx_power = RFM22_tx_pwr_txpow_4; break;    // +11dBm .. 12.6mW
	case 5: tx_power = RFM22_tx_pwr_txpow_5; break;    // +14dBm .. 25mW
	case 6: tx_power = RFM22_tx_pwr_txpow_6; break;    // +17dBm .. 50mW
	case 7: tx_power = RFM22_tx_pwr_txpow_7; break;    // +20dBm .. 100mW
	default: break;
	}
}

uint8_t rfm22_getTxPower(void)
{
	return tx_power;
}

// ************************************

uint32_t rfm22_minFrequency(void)
{
	return lower_carrier_frequency_limit_Hz;
}
uint32_t rfm22_maxFrequency(void)
{
	return upper_carrier_frequency_limit_Hz;
}

void rfm22_setNominalCarrierFrequency(uint32_t frequency_hz)
{

	exec_using_spi = TRUE;

	// *******

	if (frequency_hz < lower_carrier_frequency_limit_Hz)
		frequency_hz = lower_carrier_frequency_limit_Hz;
	else if (frequency_hz > upper_carrier_frequency_limit_Hz)
		frequency_hz = upper_carrier_frequency_limit_Hz;

	if (frequency_hz < 480000000)
		hbsel = 1;
	else
		hbsel = 2;
	uint8_t fb = (uint8_t)(frequency_hz / (10000000 * hbsel));

	uint32_t fc = (uint32_t)(frequency_hz - (10000000 * hbsel * fb));

	fc = (fc * 64u) / (10000ul * hbsel);
	fb -= 24;

	//	carrier_frequency_hz = frequency_hz;
	carrier_frequency_hz = ((uint32_t)fb + 24 + ((float)fc / 64000)) * 10000000 * hbsel;

	if (hbsel > 1)
		fb |= RFM22_fbs_hbsel;

	fb |= RFM22_fbs_sbse;	// is this the RX LO polarity?

	frequency_step_size = 156.25f * hbsel;

	rfm22_write(RFM22_frequency_hopping_channel_select, frequency_hop_channel);	// frequency hopping channel (0-255)

	rfm22_write(RFM22_frequency_offset1, 0);						// no frequency offset
	rfm22_write(RFM22_frequency_offset2, 0);						// no frequency offset

	rfm22_write(RFM22_frequency_band_select, fb);					// set the carrier frequency
	rfm22_write(RFM22_nominal_carrier_frequency1, fc >> 8);			//    "            "
	rfm22_write(RFM22_nominal_carrier_frequency0, fc & 0xff);		//    "            "

	// *******

#if defined(RFM22_DEBUG)
	//DEBUG_PRINTF(2, "rf setFreq: %u\n\r", carrier_frequency_hz);
	//	DEBUG_PRINTF(2, "rf setFreq frequency_step_size: %0.2f\n\r", frequency_step_size);
#endif

	exec_using_spi = FALSE;
}

uint32_t rfm22_getNominalCarrierFrequency(void)
{
	return carrier_frequency_hz;
}

float rfm22_getFrequencyStepSize(void)
{
	return frequency_step_size;
}

void rfm22_setFreqHopChannel(uint8_t channel)
{	// set the frequency hopping channel
	frequency_hop_channel = channel;
	rfm22_write(RFM22_frequency_hopping_channel_select, frequency_hop_channel);
}

uint8_t rfm22_freqHopChannel(void)
{	// return the current frequency hopping channel
	return frequency_hop_channel;
}

uint32_t rfm22_freqHopSize(void)
{	// return the frequency hopping step size
	return ((uint32_t)frequency_hop_step_size_reg * 10000);
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

void rfm22_setDatarate(uint32_t datarate_bps, bool data_whitening)
{

	exec_using_spi = TRUE;

	lookup_index = 0;
	while (lookup_index < (LOOKUP_SIZE - 1) && data_rate[lookup_index] < datarate_bps)
		lookup_index++;

	carrier_datarate_bps = datarate_bps = data_rate[lookup_index];

	rf_bandwidth_used = rx_bandwidth[lookup_index];

	// rfm22_if_filter_bandwidth
	rfm22_write(0x1C, reg_1C[lookup_index]);

	// rfm22_afc_loop_gearshift_override
	rfm22_write(0x1D, reg_1D[lookup_index]);
	// RFM22_afc_timing_control
	rfm22_write(0x1E, reg_1E[lookup_index]);

	// RFM22_clk_recovery_gearshift_override
	rfm22_write(0x1F, reg_1F[lookup_index]);
	// rfm22_clk_recovery_oversampling_ratio
	rfm22_write(0x20, reg_20[lookup_index]);
	// rfm22_clk_recovery_offset2
	rfm22_write(0x21, reg_21[lookup_index]);
	// rfm22_clk_recovery_offset1
	rfm22_write(0x22, reg_22[lookup_index]);
	// rfm22_clk_recovery_offset0
	rfm22_write(0x23, reg_23[lookup_index]);
	// rfm22_clk_recovery_timing_loop_gain1
	rfm22_write(0x24, reg_24[lookup_index]);
	// rfm22_clk_recovery_timing_loop_gain0
	rfm22_write(0x25, reg_25[lookup_index]);

	// rfm22_afc_limiter
	rfm22_write(0x2A, reg_2A[lookup_index]);

	if (carrier_datarate_bps < 100000)
		// rfm22_chargepump_current_trimming_override
		rfm22_write(0x58, 0x80);
	else
		// rfm22_chargepump_current_trimming_override
		rfm22_write(0x58, 0xC0);

	// rfm22_tx_data_rate1
	rfm22_write(0x6E, reg_6E[lookup_index]);
	// rfm22_tx_data_rate0
	rfm22_write(0x6F, reg_6F[lookup_index]);

	// Enable data whitening
	//	uint8_t txdtrtscale_bit = rfm22_read(RFM22_modulation_mode_control1) & RFM22_mmc1_txdtrtscale;
	//	rfm22_write(RFM22_modulation_mode_control1, txdtrtscale_bit | RFM22_mmc1_enwhite);

	if (!data_whitening)
		// rfm22_modulation_mode_control1
		rfm22_write(0x70, reg_70[lookup_index] & ~RFM22_mmc1_enwhite);
	else
		// rfm22_modulation_mode_control1
		rfm22_write(0x70, reg_70[lookup_index] |  RFM22_mmc1_enwhite);

	// rfm22_modulation_mode_control2
	rfm22_write(0x71, reg_71[lookup_index]);

	// rfm22_frequency_deviation
	rfm22_write(0x72, reg_72[lookup_index]);

	rfm22_write(RFM22_ook_counter_value1, 0x00);
	rfm22_write(RFM22_ook_counter_value2, 0x00);

	// ********************************
	// calculate the TX register values
	/*
		uint16_t fd = frequency_deviation / 625;

		uint8_t mmc1 = RFM22_mmc1_enphpwdn | RFM22_mmc1_manppol;
		uint16_t txdr;
		if (datarate_bps < 30000)
		{
		txdr = (datarate_bps * 20972) / 10000;
		mmc1 |= RFM22_mmc1_txdtrtscale;
		}
		else
		txdr = (datarate_bps * 6553) / 100000;

		uint8_t mmc2 = RFM22_mmc2_dtmod_fifo | RFM22_mmc2_modtyp_gfsk;		// FIFO mode, GFSK
		//	uint8_t mmc2 = RFM22_mmc2_dtmod_pn9 | RFM22_mmc2_modtyp_gfsk;		// PN9 mode, GFSK .. TX TEST MODE
		if (fd & 0x100) mmc2 |= RFM22_mmc2_fd;

		rfm22_write(RFM22_frequency_deviation, fd);							// set the TX peak frequency deviation

		rfm22_write(RFM22_modulation_mode_control1, mmc1);
		rfm22_write(RFM22_modulation_mode_control2, mmc2);

		rfm22_write(RFM22_tx_data_rate1, txdr >> 8);						// set the TX data rate
		rfm22_write(RFM22_tx_data_rate0, txdr);								//   "         "
	*/
	// ********************************
	// determine a clear channel time

	// initialise the stopwatch with a suitable resolution for the datarate
	//STOPWATCH_init(4000000ul / carrier_datarate_bps);					// set resolution to the time for 1 nibble (4-bits) at rf datarate

	// ********************************
	// determine suitable time-out periods

	// milliseconds
	timeout_sync_ms = (8000ul * 16) / carrier_datarate_bps;
	if (timeout_sync_ms < 3)
		// because out timer resolution is only 1ms
		timeout_sync_ms = 3;

	// milliseconds
	timeout_data_ms = (8000ul * 100) / carrier_datarate_bps;
	if (timeout_data_ms < 3)
		// because out timer resolution is only 1ms
		timeout_data_ms = 3;

	// ********************************

#if defined(RFM22_DEBUG)
/*
	DEBUG_PRINTF(2, "rf datarate_bps: %d\n\r", datarate_bps);
	DEBUG_PRINTF(2, "rf frequency_deviation: %d\n\r", frequency_deviation);
	uint32_t frequency_deviation = freq_deviation[lookup_index];	// Hz
	uint32_t modulation_bandwidth = datarate_bps + (2 * frequency_deviation);
	DEBUG_PRINTF(2, "rf modulation_bandwidth: %u\n\r", modulation_bandwidth);
	DEBUG_PRINTF(2, "rf_rx_bandwidth[%u]: %u\n\r", lookup_index, rx_bandwidth[lookup_index]);
	DEBUG_PRINTF(2, "rf est rx sensitivity[%u]: %ddBm\n\r", lookup_index, est_rx_sens_dBm[lookup_index]);
*/
#endif

	// *******

	exec_using_spi = FALSE;
}

uint32_t rfm22_getDatarate(void)
{
	return carrier_datarate_bps;
}

// ************************************

void rfm22_setSSBandwidth(uint32_t bandwidth_index)
{

	exec_using_spi = TRUE;

	ss_lookup_index = bandwidth_index;

	ss_rf_bandwidth_used = ss_rx_bandwidth[lookup_index];

	// ********************************

	rfm22_write(0x1C, ss_reg_1C[ss_lookup_index]);	// rfm22_if_filter_bandwidth
	rfm22_write(0x1D, ss_reg_1D[ss_lookup_index]);	// rfm22_afc_loop_gearshift_override

	rfm22_write(0x20, ss_reg_20[ss_lookup_index]);	// rfm22_clk_recovery_oversampling_ratio
	rfm22_write(0x21, ss_reg_21[ss_lookup_index]);	// rfm22_clk_recovery_offset2
	rfm22_write(0x22, ss_reg_22[ss_lookup_index]);	// rfm22_clk_recovery_offset1
	rfm22_write(0x23, ss_reg_23[ss_lookup_index]);	// rfm22_clk_recovery_offset0
	rfm22_write(0x24, ss_reg_24[ss_lookup_index]);	// rfm22_clk_recovery_timing_loop_gain1
	rfm22_write(0x25, ss_reg_25[ss_lookup_index]);	// rfm22_clk_recovery_timing_loop_gain0

	rfm22_write(0x2A, ss_reg_2A[ss_lookup_index]);	// rfm22_afc_limiter

	rfm22_write(0x58, 0x80);						// rfm22_chargepump_current_trimming_override

	rfm22_write(0x70, ss_reg_70[ss_lookup_index]);	// rfm22_modulation_mode_control1
	rfm22_write(0x71, ss_reg_71[ss_lookup_index]);	// rfm22_modulation_mode_control2

	rfm22_write(RFM22_ook_counter_value1, 0x00);
	rfm22_write(RFM22_ook_counter_value2, 0x00);

	// ********************************

#if defined(RFM22_DEBUG)
	DEBUG_PRINTF(2, "ss_rf_rx_bandwidth[%u]: %u\n\r", ss_lookup_index, ss_rx_bandwidth[ss_lookup_index]);
#endif

	// *******

	exec_using_spi = FALSE;
}

// ************************************

void rfm22_setRxMode(uint8_t mode, bool multi_packet_mode)
{
	exec_using_spi = TRUE;

	// disable interrupts
	rfm22_write(RFM22_interrupt_enable1, 0x00);
	rfm22_write(RFM22_interrupt_enable2, 0x00);

	// Switch to TUNE mode
	rfm22_write(RFM22_op_and_func_ctrl1, RFM22_opfc1_pllon);

	RX_LED_OFF;
	TX_LED_OFF;

	if (rf_mode == TX_CARRIER_MODE || rf_mode == TX_PN_MODE)
	{
		// FIFO mode, GFSK modulation
		uint8_t fd_bit = rfm22_read(RFM22_modulation_mode_control2) & RFM22_mmc2_fd;
		rfm22_write(RFM22_modulation_mode_control2, fd_bit | RFM22_mmc2_dtmod_fifo |
			    RFM22_mmc2_modtyp_gfsk);
	}

	// empty the rx buffer
	rx_buffer_wr = 0;
	// reset the timer
	rfm22_int_timer = 0;
	rf_mode = mode;

	// Clear the TX buffer.
	tx_data_rd = tx_data_wr = 0;

	// clear FIFOs
	if (!multi_packet_mode)
	{
		rfm22_write(RFM22_op_and_func_ctrl2, RFM22_opfc2_ffclrrx | RFM22_opfc2_ffclrtx);
		rfm22_write(RFM22_op_and_func_ctrl2, 0x00);
	} else {
		rfm22_write(RFM22_op_and_func_ctrl2, RFM22_opfc2_rxmpk | RFM22_opfc2_ffclrrx |
								RFM22_opfc2_ffclrtx);
		rfm22_write(RFM22_op_and_func_ctrl2, RFM22_opfc2_rxmpk);
	}

	// enable RX interrupts
	rfm22_write(RFM22_interrupt_enable1, RFM22_ie1_encrcerror | RFM22_ie1_enpkvalid |
		    RFM22_ie1_enrxffafull | RFM22_ie1_enfferr);
	rfm22_write(RFM22_interrupt_enable2, RFM22_ie2_enpreainval | RFM22_ie2_enpreaval |
		    RFM22_ie2_enswdet);

	// enable the receiver
	rfm22_write(RFM22_op_and_func_ctrl1, RFM22_opfc1_pllon | RFM22_opfc1_rxon);

	exec_using_spi = FALSE;
}

// ************************************

uint16_t rfm22_addHeader()
{
	uint16_t i = 0;

	for (uint16_t j = (TX_PREAMBLE_NIBBLES + 1) / 2; j > 0; j--)
	{
		rfm22_burstWrite(PREAMBLE_BYTE);
		i++;
	}
	rfm22_burstWrite(SYNC_BYTE_1); i++;
	rfm22_burstWrite(SYNC_BYTE_2); i++;

	return i;
}

// ************************************

uint8_t rfm22_txStart()
{
	if((tx_pre_buffer_size == 0) || (exec_using_spi == TRUE))
	{
		// Clear the TX buffer.
		tx_data_rd = tx_data_wr = 0;
		return 0;
	}

	exec_using_spi = TRUE;

	// Disable interrrupts.
	PIOS_IRQ_Disable();

	// Initialize the supervisor timer.
	rfm22b_dev_g->supv_timer = PIOS_RFM22B_SUPERVISOR_TIMEOUT;

	// disable interrupts
	rfm22_write(RFM22_interrupt_enable1, 0x00);
	rfm22_write(RFM22_interrupt_enable2, 0x00);

	// TUNE mode
	rfm22_write(RFM22_op_and_func_ctrl1, RFM22_opfc1_pllon);

	// Queue the data up for sending
	memcpy(tx_buffer, tx_pre_buffer, tx_pre_buffer_size);
	tx_data_rd = 0;
	tx_data_wr = tx_pre_buffer_size;
	tx_pre_buffer_size = 0;

	RX_LED_OFF;

	// Set the destination address in the transmit header.
	// The destination address is the first 4 bytes of the message.
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
		    RFM22_tx_pwr_papeaklvl_0 | RFM22_tx_pwr_lna_sw | tx_power);

	// clear FIFOs
	rfm22_write(RFM22_op_and_func_ctrl2, RFM22_opfc2_ffclrrx | RFM22_opfc2_ffclrtx);
	rfm22_write(RFM22_op_and_func_ctrl2, 0x00);

	// *******************
	// add some data to the chips TX FIFO before enabling the transmitter

	// set the total number of data bytes we are going to transmit
	rfm22_write(RFM22_transmit_packet_length, tx_data_wr);

	// add some data
	rfm22_startBurstWrite(RFM22_fifo_access);
	for (uint16_t i = 0; (tx_data_rd < tx_data_wr) && (i < FIFO_SIZE); ++tx_data_rd, ++i)
		rfm22_burstWrite(tx_buffer[tx_data_rd]);
	rfm22_endBurstWrite();

	// *******************

	// reset the timer
	rfm22_int_timer = 0;

	rf_mode = TX_DATA_MODE;

	// enable TX interrupts
	rfm22_write(RFM22_interrupt_enable1, RFM22_ie1_enpksent | RFM22_ie1_entxffaem);

	// enable the transmitter
	rfm22_write(RFM22_op_and_func_ctrl1, RFM22_opfc1_pllon | RFM22_opfc1_txon);

	// Re-ensable interrrupts.
	PIOS_IRQ_Enable();

	TX_LED_ON;

	exec_using_spi = FALSE;
	return 1;
}


static void rfm22_setTxMode(uint8_t mode)
{
	rfm22_setDebug("setTxMode");
	if (mode != TX_DATA_MODE && mode != TX_STREAM_MODE && mode != TX_CARRIER_MODE && mode != TX_PN_MODE)
		return;		// invalid mode

	exec_using_spi = TRUE;

	// disable interrupts
	rfm22_write(RFM22_interrupt_enable1, 0x00);
	rfm22_write(RFM22_interrupt_enable2, 0x00);

	// TUNE mode
	rfm22_write(RFM22_op_and_func_ctrl1, RFM22_opfc1_pllon);

	RX_LED_OFF;

	// set the tx power
	rfm22_write(RFM22_tx_power, RFM22_tx_pwr_papeaken | RFM22_tx_pwr_papeaklvl_1 |
		    RFM22_tx_pwr_papeaklvl_0 | RFM22_tx_pwr_lna_sw | tx_power);

	uint8_t fd_bit = rfm22_read(RFM22_modulation_mode_control2) & RFM22_mmc2_fd;
	if (mode == TX_CARRIER_MODE)
		// blank carrier mode -  for testing
		rfm22_write(RFM22_modulation_mode_control2, fd_bit | RFM22_mmc2_dtmod_pn9 |
								RFM22_mmc2_modtyp_none);	// FIFO mode, Blank carrier
	else if (mode == TX_PN_MODE)
		// psuedo random data carrier mode - for testing
		rfm22_write(RFM22_modulation_mode_control2, fd_bit | RFM22_mmc2_dtmod_pn9 |
								RFM22_mmc2_modtyp_gfsk);	// FIFO mode, PN9 carrier
	else
		// data transmission
		// FIFO mode, GFSK modulation
		rfm22_write(RFM22_modulation_mode_control2, fd_bit | RFM22_mmc2_dtmod_fifo |
								RFM22_mmc2_modtyp_gfsk);

	// clear FIFOs
	rfm22_write(RFM22_op_and_func_ctrl2, RFM22_opfc2_ffclrrx | RFM22_opfc2_ffclrtx);
	rfm22_write(RFM22_op_and_func_ctrl2, 0x00);

	// add some data to the chips TX FIFO before enabling the transmitter
	{
		uint16_t rd = 0;
		uint16_t wr = tx_data_wr;

		if (mode == TX_DATA_MODE)
			// set the total number of data bytes we are going to transmit
			rfm22_write(RFM22_transmit_packet_length, wr);

		uint16_t max_bytes = FIFO_SIZE - 1;
		uint16_t i = 0;
		rfm22_startBurstWrite(RFM22_fifo_access);
		if (mode == TX_STREAM_MODE)	{
			if (rd >= wr)	{
				// no data to send - yet .. just send preamble pattern
				while (true) {
					rfm22_burstWrite(PREAMBLE_BYTE);
					if (++i >= max_bytes) break;
				}
			}	else	// add the RF heaader
				i += rfm22_addHeader();
		}

		// add some data
		for (uint16_t j = wr - rd; j > 0; j--) {
			rfm22_burstWrite(tx_buffer[rd++]);
			if (++i >= max_bytes)
				break;
		}

		rfm22_endBurstWrite();

		tx_data_rd = rd;
	}

	// reset the timer
	rfm22_int_timer = 0;

	rf_mode = mode;

	// enable TX interrupts
	//	rfm22_write(RFM22_interrupt_enable1, RFM22_ie1_enpksent | RFM22_ie1_entxffaem | RFM22_ie1_enfferr);
	rfm22_write(RFM22_interrupt_enable1, RFM22_ie1_enpksent | RFM22_ie1_entxffaem);

	// read interrupt status - clear interrupts
	rfm22_read(RFM22_interrupt_status1);
	rfm22_read(RFM22_interrupt_status2);

	// enable the transmitter
	//	rfm22_write(RFM22_op_and_func_ctrl1, RFM22_opfc1_xton | RFM22_opfc1_txon);
	rfm22_write(RFM22_op_and_func_ctrl1, RFM22_opfc1_pllon | RFM22_opfc1_txon);

	TX_LED_ON;

	// *******************

	exec_using_spi = FALSE;

	rfm22_setDebug("setTxMode end");
}

// ************************************
// external interrupt line triggered (or polled) from the rf chip

void rfm22_processRxInt(void)
{
	register uint8_t int_stat1 = int_status1;
	register uint8_t int_stat2 = int_status2;
	rfm22_setDebug("processRxInt");

	// FIFO under/over flow error.  Restart RX mode.
	if (device_status & (RFM22_ds_ffunfl | RFM22_ds_ffovfl))
	{
		rfm22_setError("R_UNDER/OVERRUN");
		rfm22_setRxMode(RX_WAIT_PREAMBLE_MODE, false);
		return;
	}

	// Valid preamble detected
	if (int_stat2 & RFM22_is2_ipreaval && (rf_mode == RX_WAIT_PREAMBLE_MODE))
	{
		rf_mode = RX_WAIT_SYNC_MODE;
		RX_LED_ON;
		rfm22_setDebug("pream_det");
	}

	// Sync word detected
	if (int_stat2 & RFM22_is2_iswdet && ((rf_mode == RX_WAIT_PREAMBLE_MODE || rf_mode == RX_WAIT_SYNC_MODE)))
	{
		rf_mode = RX_DATA_MODE;
		RX_LED_ON;
		rfm22_setDebug("sync_det");

		// read the 10-bit signed afc correction value
		// bits 9 to 2
		afc_correction = (uint16_t)rfm22_read(RFM22_afc_correction_read) << 8;
		// bits 1 & 0
		afc_correction |= (uint16_t)rfm22_read(RFM22_ook_counter_value1) & 0x00c0;
		afc_correction >>= 6;
		// convert the afc value to Hz
		afc_correction_Hz = (int32_t)(frequency_step_size * afc_correction + 0.5f);

		// remember the rssi for this packet
		rx_packet_start_rssi_dBm = rssi_dBm;
		// remember the afc value for this packet
		rx_packet_start_afc_Hz = afc_correction_Hz;
	}

	// RX FIFO almost full, it needs emptying
	if (int_stat1 & RFM22_is1_irxffafull)
	{
		if (rf_mode == RX_DATA_MODE)
		{
			// read data from the rf chips FIFO buffer
			// read the total length of the packet data
			uint16_t len = rfm22_read(RFM22_received_packet_length);

			// The received packet is going to be larger than the specified length
			if ((rx_buffer_wr + RX_FIFO_HI_WATERMARK) > len)
			{
				rfm22_setError("r_size_error1");
				rfm22_setRxMode(RX_WAIT_PREAMBLE_MODE, false);
				return;
			}

			// Another packet length error.
			if (((rx_buffer_wr + RX_FIFO_HI_WATERMARK) >= len) && !(int_stat1 & RFM22_is1_ipkvalid))
			{
				rfm22_setError("r_size_error2");
				rfm22_setRxMode(RX_WAIT_PREAMBLE_MODE, false);
				return;
			}

			// Fetch the data from the RX FIFO
			rfm22_startBurstRead(RFM22_fifo_access);
			for (uint8_t i = 0; i < RX_FIFO_HI_WATERMARK; ++i)
				rx_buffer[rx_buffer_wr++] = rfm22_burstRead();
			rfm22_endBurstRead();
		}
		else
		{	// just clear the RX FIFO
			rfm22_startBurstRead(RFM22_fifo_access);
			for (register uint16_t i = RX_FIFO_HI_WATERMARK; i > 0; i--)
				rfm22_burstRead();	// read a byte from the rf modules RX FIFO buffer
			rfm22_endBurstRead();
		}
	}

	// CRC error .. discard the received data
	if (int_stat1 & RFM22_is1_icrerror)
	{
		rfm22_int_timer = 0;	// reset the timer
		rfm22_setError("CRC_ERR");
		rfm22_setRxMode(RX_WAIT_PREAMBLE_MODE, false);
		return;
	}

	// Valid packet received
	if (int_stat1 & RFM22_is1_ipkvalid)
	{

		// read the total length of the packet data
		register uint16_t len = rfm22_read(RFM22_received_packet_length);

		// their must still be data in the RX FIFO we need to get
		if (rx_buffer_wr < len)
		{
			// Fetch the data from the RX FIFO
			rfm22_startBurstRead(RFM22_fifo_access);
			while (rx_buffer_wr < len)
				rx_buffer[rx_buffer_wr++] = rfm22_burstRead();
			rfm22_endBurstRead();
		}

		if (rx_buffer_wr != len)
		{
			// we have a packet length error .. discard the packet
			rfm22_setError("r_pack_len_error");
			debug_val = len;
			rfm22_setRxMode(RX_WAIT_PREAMBLE_MODE, false);
			return;
		}

		// we have a valid received packet
		rfm22_setDebug("VALID_R_PACKET");

		if (rx_buffer_wr > 0)
		{
			// remember the rssi for this packet
			rx_packet_rssi_dBm = rx_packet_start_rssi_dBm;
			// remember the afc offset for this packet
			rx_packet_afc_Hz = rx_packet_start_afc_Hz;
			// Add the rssi and afc to the end of the packet.
			rx_buffer[rx_buffer_wr++] = rx_packet_start_rssi_dBm;
			rx_buffer[rx_buffer_wr++] = rx_packet_start_afc_Hz;
			// Pass this packet on
			bool need_yield = false;
			if (rfm22b_dev_g->rx_in_cb)
				(rfm22b_dev_g->rx_in_cb)(rfm22b_dev_g->rx_in_context, (uint8_t*)rx_buffer,
							 rx_buffer_wr, NULL, &need_yield);
			rx_buffer_wr = 0;
		}

		// Send a packet if it's available.
		if(!rfm22_txStart())
		{
			// Switch to RX mode
			rfm22_setDebug(" Set RX");
			rfm22_setRxMode(RX_WAIT_PREAMBLE_MODE, false);
		}
	}

	rfm22_setDebug("processRxInt end");
}

void rfm22_processTxInt(void)
{
	register uint8_t int_stat1 = int_status1;

	// reset the timer
	rfm22_int_timer = 0;

	// FIFO under/over flow error.  Back to RX mode.
	if (device_status & (RFM22_ds_ffunfl | RFM22_ds_ffovfl))
	{
		rfm22_setError("T_UNDER/OVERRUN");
		rfm22_setRxMode(RX_WAIT_PREAMBLE_MODE, false);
		return;
	}

	// Transmit timeout.  Abort the transmit.
	if (rfm22_int_timer >= timeout_data_ms)
	{
		rfm22_int_time_outs++;
		rfm22_setRxMode(RX_WAIT_PREAMBLE_MODE, false);
		return;
	}

	// the rf module is not in tx mode
	if ((device_status & RFM22_ds_cps_mask) != RFM22_ds_cps_tx)
	{
		if (rfm22_int_timer >= 100)
		{
			rfm22_int_time_outs++;
			rfm22_setError("T_TIMEOUT");
			rfm22_setRxMode(RX_WAIT_PREAMBLE_MODE, false);				// back to rx mode
			return;
		}
	}

	// TX FIFO almost empty, it needs filling up
	if (int_stat1 & RFM22_is1_ixtffaem)
	{
		// top-up the rf chips TX FIFO buffer
		uint16_t max_bytes = FIFO_SIZE - TX_FIFO_LO_WATERMARK - 1;
		rfm22_startBurstWrite(RFM22_fifo_access);
		for (uint16_t i = 0; (tx_data_rd < tx_data_wr) && (i < max_bytes); ++i, ++tx_data_rd)
			rfm22_burstWrite(tx_buffer[tx_data_rd]);
		rfm22_endBurstWrite();
	}

	// Packet has been sent
	if (int_stat1 & RFM22_is1_ipksent)
	{
		rfm22_setDebug(" T_Sent");

		// Send another packet if it's available.
		if(!rfm22_txStart())
		{
			// Switch to RX mode
			rfm22_setDebug(" Set RX");
			rfm22_setRxMode(RX_WAIT_PREAMBLE_MODE, false);
			return;
		}
	}

	rfm22_setDebug("ProcessTxInt done");
}

static void rfm22_processInt(void)
{
	rfm22_setDebug("ProcessInt");
	// this is called from the external interrupt handler

	if (!initialized || power_on_reset)
		// we haven't yet been initialized
		return;

	exec_using_spi = TRUE;

	// Reset the supervisor timer.
	rfm22b_dev_g->supv_timer = PIOS_RFM22B_SUPERVISOR_TIMEOUT;

	// read interrupt status registers - clears the interrupt line
	int_status1 = rfm22_read(RFM22_interrupt_status1);
	int_status2 = rfm22_read(RFM22_interrupt_status2);

	// read device status register
	device_status = rfm22_read(RFM22_device_status);

	// read ezmac status register
	ezmac_status = rfm22_read(RFM22_ezmac_status);

	// Read the RSSI if we're in RX mode
	if (rf_mode != TX_DATA_MODE && rf_mode != TX_STREAM_MODE &&
	    rf_mode != TX_CARRIER_MODE && rf_mode != TX_PN_MODE)
	{
		// read rx signal strength .. 45 = -100dBm, 205 = -20dBm
		rssi = rfm22_read(RFM22_rssi);
		// convert to dBm
		rssi_dBm = (int8_t)(rssi >> 1) - 122;
	}
	else
		// read the tx power register
		tx_pwr = rfm22_read(RFM22_tx_power);

	// the RF module has gone and done a reset - we need to re-initialize the rf module
	if (int_status2 & RFM22_is2_ipor)
	{
		initialized = FALSE;
		power_on_reset = TRUE;
		rfm22_setError("Reset");
		// Need to do something here!
		return;
	}

	switch (rf_mode)
	{
	case RX_SCAN_SPECTRUM:
		break;

	case RX_WAIT_PREAMBLE_MODE:
	case RX_WAIT_SYNC_MODE:
	case RX_DATA_MODE:

		rfm22_processRxInt();
		break;

	case TX_DATA_MODE:
	case TX_STREAM_MODE:

		rfm22_processTxInt();
		break;

	case TX_CARRIER_MODE:
	case TX_PN_MODE:

		//			if (rfm22_int_timer >= TX_TEST_MODE_TIMELIMIT_MS)			// 'nn'ms limit
		//			{
		//				rfm22_setRxMode(RX_WAIT_PREAMBLE_MODE, false);					// back to rx mode
		//				tx_data_rd = tx_data_wr = 0;					// wipe TX buffer
		//				break;
		//			}

		break;

	default:	// unknown mode - this should NEVER happen, maybe we should do a complete CPU reset here
		rfm22_setRxMode(RX_WAIT_PREAMBLE_MODE, false);					// to rx mode
		break;
	}

	exec_using_spi = FALSE;

	rfm22_setDebug("ProcessInt done");
}

// ************************************

int8_t rfm22_getRSSI(void)
{
	exec_using_spi = TRUE;

	rssi = rfm22_read(RFM22_rssi);			// read rx signal strength .. 45 = -100dBm, 205 = -20dBm
	rssi_dBm = (int8_t)(rssi >> 1) - 122;	// convert to dBm

	exec_using_spi = FALSE;
	return rssi_dBm;
}

int8_t rfm22_receivedRSSI(void)
{	// return the packets signal strength
	if (!initialized)
		return -127;
	else
		return rx_packet_rssi_dBm;
}

int32_t rfm22_receivedAFCHz(void)
{	// return the packets offset frequency
	if (!initialized)
		return 0;
	else
		return rx_packet_afc_Hz;
}

// ************************************

// ************************************

void rfm22_setTxStream(void)	// TEST ONLY
{
	if (!initialized)
		return;

	tx_data_rd = tx_data_wr = 0;

	rfm22_setTxMode(TX_STREAM_MODE);
}

// ************************************

void rfm22_setTxNormal(void)
{
	if (!initialized)
		return;

	//	if (rf_mode == TX_CARRIER_MODE || rf_mode == TX_PN_MODE)
	if (rf_mode != RX_SCAN_SPECTRUM)
	{
		rfm22_setRxMode(RX_WAIT_PREAMBLE_MODE, false);
		tx_data_rd = tx_data_wr = 0;

		rx_packet_start_rssi_dBm = 0;
		rx_packet_start_afc_Hz = 0;
		rx_packet_rssi_dBm = 0;
		rx_packet_afc_Hz = 0;
	}
}

// enable a blank tx carrier (for frequency alignment)
void rfm22_setTxCarrierMode(void)
{
	if (!initialized)
		return;

	if (rf_mode != TX_CARRIER_MODE && rf_mode != RX_SCAN_SPECTRUM)
		rfm22_setTxMode(TX_CARRIER_MODE);
}

// enable a psuedo random data tx carrier (for spectrum inspection)
void rfm22_setTxPNMode(void)
{
	if (!initialized)
		return;

	if (rf_mode != TX_PN_MODE && rf_mode != RX_SCAN_SPECTRUM)
		rfm22_setTxMode(TX_PN_MODE);
}

// ************************************

// return the current mode
int8_t rfm22_currentMode(void)
{
	return rf_mode;
}

// return TRUE if we are transmitting
bool rfm22_transmitting(void)
{
	return (rf_mode == TX_DATA_MODE || rf_mode == TX_STREAM_MODE || rf_mode == TX_CARRIER_MODE || rf_mode == TX_PN_MODE);
}

// return TRUE if the channel is clear to transmit on
bool rfm22_channelIsClear(void)
{
	if (!initialized)
		// we haven't yet been initialized
		return FALSE;

	if (rf_mode != RX_WAIT_PREAMBLE_MODE && rf_mode != RX_WAIT_SYNC_MODE)
		// we are receiving something or we are transmitting or we are scanning the spectrum
		return FALSE;

	return TRUE;
}

// return TRUE if the transmiter is ready for use
bool rfm22_txReady(void)
{
	if (!initialized)
		// we haven't yet been initialized
		return FALSE;

	return (tx_data_rd == 0 && tx_data_wr == 0 && rf_mode != TX_DATA_MODE &&
		rf_mode != TX_STREAM_MODE && rf_mode != TX_CARRIER_MODE && rf_mode != TX_PN_MODE &&
		rf_mode != RX_SCAN_SPECTRUM);
}

// ************************************
// set/get the frequency calibration value

void rfm22_setFreqCalibration(uint8_t value)
{
	osc_load_cap = value;

	if (!initialized || power_on_reset)
		return;				// we haven't yet been initialized

	uint8_t prev_rf_mode = rf_mode;

	if (rf_mode == TX_CARRIER_MODE || rf_mode == TX_PN_MODE)
	{
		rfm22_setRxMode(RX_WAIT_PREAMBLE_MODE, false);
		tx_data_rd = tx_data_wr = 0;
	}

	exec_using_spi = TRUE;

	rfm22_write(RFM22_xtal_osc_load_cap, osc_load_cap);

	exec_using_spi = FALSE;

	if (prev_rf_mode == TX_CARRIER_MODE || prev_rf_mode == TX_PN_MODE)
		rfm22_setTxMode(prev_rf_mode);
}

uint8_t rfm22_getFreqCalibration(void)
{
	return osc_load_cap;
}

// ************************************
// can be called from an interrupt if you wish

void rfm22_1ms_tick(void)
{	// call this once every ms
	if (!initialized) return;		// we haven't yet been initialized

	if (rf_mode != RX_SCAN_SPECTRUM)
	{
		if (rfm22_int_timer < 0xffff) rfm22_int_timer++;
	}
}

// ************************************
// reset the RF module

int rfm22_resetModule(uint8_t mode, uint32_t min_frequency_hz, uint32_t max_frequency_hz)
{
	initialized = false;

#if defined(RFM22_EXT_INT_USE)
	rfm22_disableExtInt();
#endif

	power_on_reset = false;

	// ****************

	exec_using_spi = TRUE;

	// ****************
	// setup the SPI port

	// chip select line HIGH
	PIOS_SPI_RC_PinSet(RFM22_PIOS_SPI, 0, 1);

	// set SPI port SCLK frequency .. 4.5MHz
	PIOS_SPI_SetClockSpeed(RFM22_PIOS_SPI, PIOS_SPI_PRESCALER_16);
	// set SPI port SCLK frequency .. 2.25MHz
	//		PIOS_SPI_SetClockSpeed(RFM22_PIOS_SPI, PIOS_SPI_PRESCALER_32);

	// set SPI port SCLK frequency .. 285kHz .. purely for hardware fault finding
	//		PIOS_SPI_SetClockSpeed(RFM22_PIOS_SPI, PIOS_SPI_PRESCALER_256);

	// ****************
	// software reset the RF chip .. following procedure according to Si4x3x Errata (rev. B)

	rfm22_write(RFM22_op_and_func_ctrl1, RFM22_opfc1_swres);			// software reset the radio

	PIOS_DELAY_WaitmS(26);												// wait 26ms

	for (int i = 50; i > 0; i--)
	{
		PIOS_DELAY_WaitmS(1);											// wait 1ms

		// read the status registers
		int_status1 = rfm22_read(RFM22_interrupt_status1);
		int_status2 = rfm22_read(RFM22_interrupt_status2);
		if (int_status2 & RFM22_is2_ichiprdy) break;
	}

	// ****************

	// read status - clears interrupt
	device_status = rfm22_read(RFM22_device_status);
	int_status1 = rfm22_read(RFM22_interrupt_status1);
	int_status2 = rfm22_read(RFM22_interrupt_status2);
	ezmac_status = rfm22_read(RFM22_ezmac_status);

	// disable all interrupts
	rfm22_write(RFM22_interrupt_enable1, 0x00);
	rfm22_write(RFM22_interrupt_enable2, 0x00);

	// ****************

	exec_using_spi = FALSE;

	// ****************

	rf_mode = mode;

	device_status = int_status1 = int_status2 = ezmac_status = 0;

	rssi = 0;
	rssi_dBm = -127;

	rx_buffer_current = 0;
	rx_buffer_wr = 0;
	rx_packet_rssi_dBm = -127;
	rx_packet_afc_Hz = 0;

	tx_data_rd = tx_data_wr = 0;

	lookup_index = 0;
	ss_lookup_index = 0;

	rf_bandwidth_used = 0;
	ss_rf_bandwidth_used = 0;

	rfm22_int_timer = 0;
	rfm22_int_time_outs = 0;
	prev_rfm22_int_time_outs = 0;

	hbsel = 0;
	frequency_step_size = 0.0f;

	frequency_hop_channel = 0;

	afc_correction = 0;
	afc_correction_Hz = 0;

	temperature_reg = 0;

	// set the TX power
	tx_power = RFM22_DEFAULT_RF_POWER;

	tx_pwr = 0;

	// ****************
	// read the RF chip ID bytes

	device_type = rfm22_read(RFM22_DEVICE_TYPE) & RFM22_DT_MASK;		// read the device type
	device_version = rfm22_read(RFM22_DEVICE_VERSION) & RFM22_DV_MASK;	// read the device version

#if defined(RFM22_DEBUG)
	DEBUG_PRINTF(2, "rf device type: %d\n\r", device_type);
	DEBUG_PRINTF(2, "rf device version: %d\n\r", device_version);
#endif

	if (device_type != 0x08)
	{
#if defined(RFM22_DEBUG)
		DEBUG_PRINTF(2, "rf device type: INCORRECT - should be 0x08\n\r");
#endif
		return -1;	// incorrect RF module type
	}

	//	if (device_version != RFM22_DEVICE_VERSION_V2)	// V2
	//		return -2;	// incorrect RF module version
	//	if (device_version != RFM22_DEVICE_VERSION_A0)	// A0
	//		return -2;	// incorrect RF module version
	if (device_version != RFM22_DEVICE_VERSION_B1)	// B1
	{
#if defined(RFM22_DEBUG)
		DEBUG_PRINTF(2, "rf device version: INCORRECT\n\r");
#endif
		return -2;	// incorrect RF module version
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

	lower_carrier_frequency_limit_Hz = min_frequency_hz;
	upper_carrier_frequency_limit_Hz = max_frequency_hz;

	// ****************
	// calibrate our RF module to be exactly on frequency .. different for every module

	osc_load_cap = OSC_LOAD_CAP;	// default
	rfm22_write(RFM22_xtal_osc_load_cap, osc_load_cap);

	// ****************

	// disable Low Duty Cycle Mode
	rfm22_write(RFM22_op_and_func_ctrl2, 0x00);

	rfm22_write(RFM22_cpu_output_clk, RFM22_coc_1MHz);						// 1MHz clock output

	rfm22_write(RFM22_op_and_func_ctrl1, RFM22_opfc1_xton);					// READY mode
	//	rfm22_write(RFM22_op_and_func_ctrl1, RFM22_opfc1_pllon);				// TUNE mode

	// choose the 3 GPIO pin functions
	rfm22_write(RFM22_io_port_config, RFM22_io_port_default);								// GPIO port use default value
	rfm22_write(RFM22_gpio0_config, RFM22_gpio0_config_drv3 | RFM22_gpio0_config_txstate);	// GPIO0 = TX State (to control RF Switch)
	rfm22_write(RFM22_gpio1_config, RFM22_gpio1_config_drv3 | RFM22_gpio1_config_rxstate);	// GPIO1 = RX State (to control RF Switch)
	rfm22_write(RFM22_gpio2_config, RFM22_gpio2_config_drv3 | RFM22_gpio2_config_cca);		// GPIO2 = Clear Channel Assessment

	// ****************

	return 0;	// OK
}

// ************************************

int rfm22_init_scan_spectrum(uint32_t min_frequency_hz, uint32_t max_frequency_hz)
{
#if defined(RFM22_DEBUG)
	DEBUG_PRINTF(2, "\n\rRF init scan spectrum\n\r");
#endif

	int res = rfm22_resetModule(RX_SCAN_SPECTRUM, min_frequency_hz, max_frequency_hz);
	if (res < 0)
		return res;

	//	rfm22_setSSBandwidth(0);
	rfm22_setSSBandwidth(1);

	// FIFO mode, GFSK modulation
	uint8_t fd_bit = rfm22_read(RFM22_modulation_mode_control2) & RFM22_mmc2_fd;
	rfm22_write(RFM22_modulation_mode_control2, RFM22_mmc2_trclk_clk_none | RFM22_mmc2_dtmod_fifo | fd_bit | RFM22_mmc2_modtyp_gfsk);

	rfm22_write(RFM22_cpu_output_clk, RFM22_coc_1MHz);								// 1MHz clock output

	rfm22_write(RFM22_rssi_threshold_clear_chan_indicator, 0);

	rfm22_write(RFM22_preamble_detection_ctrl1, 31 << 3);					// 31-nibbles rx preamble detection

	// avoid packet detection
	rfm22_write(RFM22_data_access_control, RFM22_dac_enpacrx | RFM22_dac_encrc);
	rfm22_write(RFM22_header_control1, 0x0f);
	rfm22_write(RFM22_header_control2, 0x77);

	rfm22_write(RFM22_sync_word3, SYNC_BYTE_1);
	rfm22_write(RFM22_sync_word2, SYNC_BYTE_2);
	rfm22_write(RFM22_sync_word1, SYNC_BYTE_3 ^ 0xff);
	rfm22_write(RFM22_sync_word0, SYNC_BYTE_4 ^ 0xff);

	// all the bits to be checked
	rfm22_write(RFM22_header_enable3, 0xff);
	rfm22_write(RFM22_header_enable2, 0xff);
	rfm22_write(RFM22_header_enable1, 0xff);
	rfm22_write(RFM22_header_enable0, 0xff);

	// set frequency hopping channel step size (multiples of 10kHz)
	//	rfm22_write(RFM22_frequency_hopping_step_size, 0);

	// set our nominal carrier frequency
	rfm22_setNominalCarrierFrequency(min_frequency_hz);

	// set minimum tx power
	rfm22_write(RFM22_tx_power, RFM22_tx_pwr_lna_sw | 0);

	rfm22_write(RFM22_agc_override1, RFM22_agc_ovr1_sgi | RFM22_agc_ovr1_agcen);

	//	rfm22_write(RFM22_vco_current_trimming, 0x7f);
	//	rfm22_write(RFM22_vco_calibration_override, 0x40);
	//	rfm22_write(RFM22_chargepump_current_trimming_override, 0x80);

	// Enable RF module external interrupt
	rfm22_enableExtInt();

	rfm22_setRxMode(RX_SCAN_SPECTRUM, true);

	initialized = true;

	return 0;	// OK
}

// ************************************

int rfm22_init_tx_stream(uint32_t min_frequency_hz, uint32_t max_frequency_hz)
{
#if defined(RFM22_DEBUG)
	DEBUG_PRINTF(2, "\n\rRF init TX stream\n\r");
#endif

	int res = rfm22_resetModule(TX_STREAM_MODE, min_frequency_hz, max_frequency_hz);
	if (res < 0)
		return res;

	frequency_hop_step_size_reg = 0;

	// set the RF datarate
	rfm22_setDatarate(RFM22_DEFAULT_RF_DATARATE, FALSE);

	// FIFO mode, GFSK modulation
	uint8_t fd_bit = rfm22_read(RFM22_modulation_mode_control2) & RFM22_mmc2_fd;
	rfm22_write(RFM22_modulation_mode_control2, RFM22_mmc2_trclk_clk_none | RFM22_mmc2_dtmod_fifo | fd_bit | RFM22_mmc2_modtyp_gfsk);

	// disable the internal Tx & Rx packet handlers (without CRC)
	rfm22_write(RFM22_data_access_control, 0);

	rfm22_write(RFM22_preamble_length, TX_PREAMBLE_NIBBLES);				// x-nibbles tx preamble
	rfm22_write(RFM22_preamble_detection_ctrl1, RX_PREAMBLE_NIBBLES << 3);	// x-nibbles rx preamble detection

	rfm22_write(RFM22_header_control1, RFM22_header_cntl1_bcen_none | RFM22_header_cntl1_hdch_none);	// header control - we are not using the header
	rfm22_write(RFM22_header_control2, RFM22_header_cntl2_fixpklen | RFM22_header_cntl2_hdlen_none | RFM22_header_cntl2_synclen_32 | ((TX_PREAMBLE_NIBBLES >> 8) & 0x01));	// no header bytes, synchronization word length 3, 2 used, packet length not included in header (fixed packet length).

	rfm22_write(RFM22_sync_word3, SYNC_BYTE_1);								// sync word
	rfm22_write(RFM22_sync_word2, SYNC_BYTE_2);								//

	//	rfm22_write(RFM22_modem_test, 0x01);

	rfm22_write(RFM22_agc_override1, RFM22_agc_ovr1_agcen);
	//	rfm22_write(RFM22_agc_override1, RFM22_agc_ovr1_sgi | RFM22_agc_ovr1_agcen);

	rfm22_write(RFM22_frequency_hopping_step_size, frequency_hop_step_size_reg);	// set frequency hopping channel step size (multiples of 10kHz)

	rfm22_setNominalCarrierFrequency((min_frequency_hz + max_frequency_hz) / 2);	// set our nominal carrier frequency

	rfm22_write(RFM22_tx_power, RFM22_tx_pwr_papeaken | RFM22_tx_pwr_papeaklvl_0 | RFM22_tx_pwr_lna_sw | tx_power);	// set the tx power
	//	rfm22_write(RFM22_tx_power, RFM22_tx_pwr_lna_sw | tx_power);	// set the tx power

	//	rfm22_write(RFM22_vco_current_trimming, 0x7f);
	//	rfm22_write(RFM22_vco_calibration_override, 0x40);
	//	rfm22_write(RFM22_chargepump_current_trimming_override, 0x80);

	rfm22_write(RFM22_tx_fifo_control1, TX_FIFO_HI_WATERMARK);				// TX FIFO Almost Full Threshold (0 - 63)
	rfm22_write(RFM22_tx_fifo_control2, TX_FIFO_LO_WATERMARK);				// TX FIFO Almost Empty Threshold (0 - 63)

	// Enable RF module external interrupt
	rfm22_enableExtInt();

	initialized = true;

	return 0;	// OK
}

// ************************************

int rfm22_init_rx_stream(uint32_t min_frequency_hz, uint32_t max_frequency_hz)
{
#if defined(RFM22_DEBUG)
	DEBUG_PRINTF(2, "\n\rRF init RX stream\n\r");
#endif

	int res = rfm22_resetModule(RX_WAIT_PREAMBLE_MODE, min_frequency_hz, max_frequency_hz);
	if (res < 0)
		return res;

	frequency_hop_step_size_reg = 0;

	// set the RF datarate
	rfm22_setDatarate(RFM22_DEFAULT_RF_DATARATE, FALSE);

	// FIFO mode, GFSK modulation
	uint8_t fd_bit = rfm22_read(RFM22_modulation_mode_control2) & RFM22_mmc2_fd;
	rfm22_write(RFM22_modulation_mode_control2, RFM22_mmc2_trclk_clk_none | RFM22_mmc2_dtmod_fifo | fd_bit | RFM22_mmc2_modtyp_gfsk);

	// disable the internal Tx & Rx packet handlers (without CRC)
	rfm22_write(RFM22_data_access_control, 0);

	rfm22_write(RFM22_preamble_length, TX_PREAMBLE_NIBBLES);				// x-nibbles tx preamble
	rfm22_write(RFM22_preamble_detection_ctrl1, RX_PREAMBLE_NIBBLES << 3);	// x-nibbles rx preamble detection

	rfm22_write(RFM22_header_control1, RFM22_header_cntl1_bcen_none | RFM22_header_cntl1_hdch_none);	// header control - we are not using the header
	rfm22_write(RFM22_header_control2, RFM22_header_cntl2_fixpklen | RFM22_header_cntl2_hdlen_none | RFM22_header_cntl2_synclen_32 | ((TX_PREAMBLE_NIBBLES >> 8) & 0x01));	// no header bytes, synchronization word length 3, 2 used, packet length not included in header (fixed packet length).

	rfm22_write(RFM22_sync_word3, SYNC_BYTE_1);								// sync word
	rfm22_write(RFM22_sync_word2, SYNC_BYTE_2);								//

	// no header bits to be checked
	rfm22_write(RFM22_header_enable3, 0x00);
	rfm22_write(RFM22_header_enable2, 0x00);
	rfm22_write(RFM22_header_enable1, 0x00);
	rfm22_write(RFM22_header_enable0, 0x00);

	//	rfm22_write(RFM22_modem_test, 0x01);

	rfm22_write(RFM22_agc_override1, RFM22_agc_ovr1_agcen);
	//	rfm22_write(RFM22_agc_override1, RFM22_agc_ovr1_sgi | RFM22_agc_ovr1_agcen);

	rfm22_write(RFM22_frequency_hopping_step_size, frequency_hop_step_size_reg);	// set frequency hopping channel step size (multiples of 10kHz)

	rfm22_setNominalCarrierFrequency((min_frequency_hz + max_frequency_hz) / 2);	// set our nominal carrier frequency

	//	rfm22_write(RFM22_vco_current_trimming, 0x7f);
	//	rfm22_write(RFM22_vco_calibration_override, 0x40);
	//	rfm22_write(RFM22_chargepump_current_trimming_override, 0x80);

	// RX FIFO Almost Full Threshold (0 - 63)
	rfm22_write(RFM22_rx_fifo_control, RX_FIFO_HI_WATERMARK);

	// Enable RF module external interrupt
	rfm22_enableExtInt();

	rfm22_setRxMode(RX_WAIT_PREAMBLE_MODE, false);

	initialized = true;

	return 0;	// OK
}

// ************************************
// Initialise this hardware layer module and the rf module

int rfm22_init_normal(uint32_t id, uint32_t min_frequency_hz, uint32_t max_frequency_hz, uint32_t freq_hop_step_size)
{
	int res = rfm22_resetModule(RX_WAIT_PREAMBLE_MODE, min_frequency_hz, max_frequency_hz);
	if (res < 0)
		return res;

	// initialize the frequency hopping step size
	freq_hop_step_size /= 10000;	// in 10kHz increments
	if (freq_hop_step_size > 255) freq_hop_step_size = 255;
	frequency_hop_step_size_reg = freq_hop_step_size;

	// set the RF datarate
	rfm22_setDatarate(RFM22_DEFAULT_RF_DATARATE, TRUE);

	// FIFO mode, GFSK modulation
	uint8_t fd_bit = rfm22_read(RFM22_modulation_mode_control2) & RFM22_mmc2_fd;
	rfm22_write(RFM22_modulation_mode_control2, RFM22_mmc2_trclk_clk_none | RFM22_mmc2_dtmod_fifo | fd_bit | RFM22_mmc2_modtyp_gfsk);

	// setup to read the internal temperature sensor

	// ADC used to sample the temperature sensor
	adc_config = RFM22_ac_adcsel_temp_sensor | RFM22_ac_adcref_bg;
	rfm22_write(RFM22_adc_config, adc_config);

	// adc offset
	rfm22_write(RFM22_adc_sensor_amp_offset, 0);

	// temp sensor calibration .. –40C to +64C 0.5C resolution
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
	rfm22_write(RFM22_frequency_hopping_step_size, frequency_hop_step_size_reg);

	// set our nominal carrier frequency
	rfm22_setNominalCarrierFrequency((min_frequency_hz + max_frequency_hz) / 2);

	// set the tx power
	rfm22_write(RFM22_tx_power, RFM22_tx_pwr_papeaken | RFM22_tx_pwr_papeaklvl_0 |
							RFM22_tx_pwr_lna_sw | tx_power);

	// TX FIFO Almost Full Threshold (0 - 63)
	rfm22_write(RFM22_tx_fifo_control1, TX_FIFO_HI_WATERMARK);

	// TX FIFO Almost Empty Threshold (0 - 63)
	rfm22_write(RFM22_tx_fifo_control2, TX_FIFO_LO_WATERMARK);

	// RX FIFO Almost Full Threshold (0 - 63)
	rfm22_write(RFM22_rx_fifo_control, RX_FIFO_HI_WATERMARK);

	// Enable RF module external interrupt
	rfm22_enableExtInt();

	rfm22_setRxMode(RX_WAIT_PREAMBLE_MODE, false);

	initialized = true;

	return 0;	// ok
}

// ************************************

#endif

/**
 * @}
 * @}
 */

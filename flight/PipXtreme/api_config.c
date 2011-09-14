/**
 ******************************************************************************
 *
 * @file       apiconfig_config.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      RF Module hardware layer
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

#include <string.h>

#include "stm32f10x.h"
#include "gpio_in.h"
#include "api_config.h"
#include "rfm22b.h"
#include "packet_handler.h"
#include "stream.h"
#include "ppm.h"
#include "saved_settings.h"
#include "crc.h"
#include "main.h"

#if defined(PIOS_COM_DEBUG)
//	#define APICONFIG_DEBUG
#endif

// *****************************************************************************
// modem configuration packets

#define PIPX_HEADER_MARKER					0x76b38a52

enum {
	PIPX_PACKET_TYPE_REQ_DETAILS = 0,
	PIPX_PACKET_TYPE_DETAILS,
	PIPX_PACKET_TYPE_REQ_SETTINGS,
	PIPX_PACKET_TYPE_SETTINGS,
	PIPX_PACKET_TYPE_REQ_STATE,
	PIPX_PACKET_TYPE_STATE,
	PIPX_PACKET_TYPE_SPECTRUM
};

typedef struct
{
    uint32_t    marker;
    uint32_t    serial_number;
    uint8_t     type;
    uint8_t     spare;
    uint16_t    data_size;
	uint32_t    header_crc;
	uint32_t    data_crc;
//	uint8_t     data[0];
} __attribute__((__packed__)) t_pipx_config_header;

typedef struct
{
	uint8_t		major_version;
	uint8_t		minor_version;
	uint32_t	serial_number;
	uint32_t    min_frequency_Hz;
	uint32_t    max_frequency_Hz;
	uint8_t     frequency_band;
	float	    frequency_step_size;
} __attribute__((__packed__)) t_pipx_config_details;

typedef struct
{
    uint8_t     mode;
    uint8_t     link_state;
    int16_t		rssi;
    int32_t		afc;
    uint16_t	retries;
} __attribute__((__packed__)) t_pipx_config_state;

typedef struct
{
    uint8_t     mode;
    uint32_t    serial_baudrate;    // serial usart baudrate
    uint32_t    destination_id;
    uint32_t    frequency_Hz;
    uint32_t    max_rf_bandwidth;
    uint8_t     max_tx_power;
    uint8_t     rf_xtal_cap;
    bool        aes_enable;
    uint8_t     aes_key[16];
    uint8_t		rts_time;
    uint8_t		spare[16];
} __attribute__((__packed__)) t_pipx_config_settings;

typedef struct
{
	uint32_t	start_frequency;
    uint16_t    frequency_step_size;
    uint16_t    magnitudes;
//    int8_t      magnitude[0];
} __attribute__((__packed__)) t_pipx_config_spectrum;

// *****************************************************************************
// local variables

uint32_t			apiconfig_previous_com_port = 0;

volatile uint16_t	apiconfig_rx_config_timer = 0;
volatile uint16_t	apiconfig_rx_timer = 0;
volatile uint16_t	apiconfig_tx_timer = 0;
volatile uint16_t	apiconfig_ss_timer = 0;

uint8_t				apiconfig_rx_buffer[256] __attribute__ ((aligned(4)));
uint16_t			apiconfig_rx_buffer_wr;

uint8_t				apiconfig_tx_buffer[256] __attribute__ ((aligned(4)));
uint16_t			apiconfig_tx_buffer_wr;

uint8_t				apiconfig_tx_config_buffer[128] __attribute__ ((aligned(4)));
uint16_t			apiconfig_tx_config_buffer_wr;

// for scanning the spectrum
int8_t				apiconfig_spec_buffer[64] __attribute__ ((aligned(4)));
uint16_t			apiconfig_spec_buffer_wr;
uint32_t			apiconfig_spec_start_freq;
uint32_t			apiconfig_spec_freq;

bool				apiconfig_usb_comms;
uint32_t			apiconfig_comm_port;

uint16_t			apiconfig_connection_index;		// the RF connection we are using

uint8_t				led_counter;

// *****************************************************************************

int apiconfig_sendDetailsPacket(void)
{
	if (sizeof(apiconfig_tx_config_buffer) - apiconfig_tx_config_buffer_wr < sizeof(t_pipx_config_header) + sizeof(t_pipx_config_details))
		return -1;	// not enough room in the tx buffer for the packet we were going to send

	t_pipx_config_header *header = (t_pipx_config_header *)(apiconfig_tx_config_buffer + apiconfig_tx_config_buffer_wr);
	t_pipx_config_details *details = (t_pipx_config_details *)((uint8_t *)header + sizeof(t_pipx_config_header));

	header->marker = PIPX_HEADER_MARKER;
	header->serial_number = serial_number_crc32;
	header->type = PIPX_PACKET_TYPE_DETAILS;
	header->spare = 0;
	header->data_size = sizeof(t_pipx_config_details);

	details->major_version = VERSION_MAJOR;
	details->minor_version = VERSION_MINOR;
	details->serial_number = serial_number_crc32;
	details->min_frequency_Hz = saved_settings.min_frequency_Hz;
	details->max_frequency_Hz = saved_settings.max_frequency_Hz;
	details->frequency_band = saved_settings.frequency_band;
	details->frequency_step_size = rfm22_getFrequencyStepSize();

	header->data_crc = updateCRC32Data(0xffffffff, details, header->data_size);
	header->header_crc = 0;
	header->header_crc = updateCRC32Data(0xffffffff, header, sizeof(t_pipx_config_header));

	int total_packet_size = sizeof(t_pipx_config_header) + header->data_size;

	apiconfig_tx_config_buffer_wr += total_packet_size;

	#if defined(APICONFIG_DEBUG)
		DEBUG_PRINTF("TX api config: details\r\n");
	#endif

	return total_packet_size;
}

int apiconfig_sendStatePacket(void)
{
	if (sizeof(apiconfig_tx_config_buffer) - apiconfig_tx_config_buffer_wr < sizeof(t_pipx_config_header) + sizeof(t_pipx_config_state))
		return -1;	// not enough room in the tx buffer for the packet we were going to send

	t_pipx_config_header *header = (t_pipx_config_header *)(apiconfig_tx_config_buffer + apiconfig_tx_config_buffer_wr);
	t_pipx_config_state *state = (t_pipx_config_state *)((uint8_t *)header + sizeof(t_pipx_config_header));

	header->marker = PIPX_HEADER_MARKER;
	header->serial_number = serial_number_crc32;
	header->type = PIPX_PACKET_TYPE_STATE;
	header->spare = 0;
	header->data_size = sizeof(t_pipx_config_state);

	state->mode = 0;
	state->link_state = ph_getCurrentLinkState(0);
	state->rssi = ph_getLastRSSI(0);
	state->afc = ph_getLastAFC(0);
	state->retries = ph_getRetries(0);

	header->data_crc = updateCRC32Data(0xffffffff, state, header->data_size);
	header->header_crc = 0;
	header->header_crc = updateCRC32Data(0xffffffff, header, sizeof(t_pipx_config_header));

	int total_packet_size = sizeof(t_pipx_config_header) + header->data_size;

	apiconfig_tx_config_buffer_wr += total_packet_size;

	#if defined(APICONFIG_DEBUG)
		DEBUG_PRINTF("TX api config: state\r\n");
	#endif

	return total_packet_size;
}

int apiconfig_sendSettingsPacket(void)
{
	if (sizeof(apiconfig_tx_config_buffer) - apiconfig_tx_config_buffer_wr < sizeof(t_pipx_config_header) + sizeof(t_pipx_config_settings))
		return -1;	// not enough room in the tx buffer for the packet we were going to send

	t_pipx_config_header *header = (t_pipx_config_header *)(apiconfig_tx_config_buffer + apiconfig_tx_config_buffer_wr);
	t_pipx_config_settings *settings = (t_pipx_config_settings *)((uint8_t *)header + sizeof(t_pipx_config_header));

	header->marker = PIPX_HEADER_MARKER;
	header->serial_number = serial_number_crc32;
	header->type = PIPX_PACKET_TYPE_SETTINGS;
	header->spare = 0;
	header->data_size = sizeof(t_pipx_config_settings);

	settings->mode = saved_settings.mode;
	settings->destination_id = saved_settings.destination_id;
	settings->frequency_Hz = saved_settings.frequency_Hz;
	settings->max_rf_bandwidth = saved_settings.max_rf_bandwidth;
	settings->max_tx_power = saved_settings.max_tx_power;
	settings->rf_xtal_cap = saved_settings.rf_xtal_cap;
	settings->serial_baudrate = saved_settings.serial_baudrate;
	settings->aes_enable = saved_settings.aes_enable;
	memcpy((char *)settings->aes_key, (char *)saved_settings.aes_key, sizeof(settings->aes_key));
	settings->rts_time = saved_settings.rts_time;
	memset(settings->spare, 0xff, sizeof(settings->spare));

	header->data_crc = updateCRC32Data(0xffffffff, settings, header->data_size);
	header->header_crc = 0;
	header->header_crc = updateCRC32Data(0xffffffff, header, sizeof(t_pipx_config_header));

	int total_packet_size = sizeof(t_pipx_config_header) + header->data_size;

	apiconfig_tx_config_buffer_wr += total_packet_size;

	#if defined(APICONFIG_DEBUG)
		DEBUG_PRINTF("TX api config: settings\r\n");
	#endif

	return total_packet_size;
}

int apiconfig_sendSpectrumPacket(uint32_t start_frequency, uint16_t freq_step_size, void *spectrum_data, uint32_t spectrum_data_size)
{
	if (sizeof(apiconfig_tx_config_buffer) - apiconfig_tx_config_buffer_wr < sizeof(t_pipx_config_header) + sizeof(t_pipx_config_spectrum) + spectrum_data_size)
		return -1;	// not enough room in the tx buffer for the packet we were going to send

	if (!spectrum_data || spectrum_data_size <= 0)
		return -2;	// nothing to send

	t_pipx_config_header *header = (t_pipx_config_header *)(apiconfig_tx_config_buffer + apiconfig_tx_config_buffer_wr);
	t_pipx_config_spectrum *spectrum = (t_pipx_config_spectrum *)((uint8_t *)header + sizeof(t_pipx_config_header));
	uint8_t *data = (uint8_t *)spectrum + sizeof(t_pipx_config_spectrum);

	header->marker = PIPX_HEADER_MARKER;
	header->serial_number = serial_number_crc32;
	header->type = PIPX_PACKET_TYPE_SPECTRUM;
	header->spare = 0;
	header->data_size = sizeof(t_pipx_config_spectrum) + spectrum_data_size;

	spectrum->start_frequency = start_frequency;
    spectrum->frequency_step_size = freq_step_size;
    spectrum->magnitudes = spectrum_data_size;
    memmove(data, spectrum_data, spectrum_data_size);

	header->data_crc = updateCRC32Data(0xffffffff, spectrum, header->data_size);
	header->header_crc = 0;
	header->header_crc = updateCRC32Data(0xffffffff, header, sizeof(t_pipx_config_header));

	int total_packet_size = sizeof(t_pipx_config_header) + header->data_size;

	apiconfig_tx_config_buffer_wr += total_packet_size;

	#if defined(APICONFIG_DEBUG)
		DEBUG_PRINTF("TX api config: spectrum\r\n");
	#endif

	return total_packet_size;
}

void apiconfig_processInputPacket(void *buf, uint16_t len)
{
	if (len <= 0)
		return;

	t_pipx_config_header *header = (t_pipx_config_header *)buf;
	uint8_t *data = (uint8_t *)header + sizeof(t_pipx_config_header);

	switch (header->type)
	{
		case PIPX_PACKET_TYPE_REQ_DETAILS:

			#if defined(APICONFIG_DEBUG)
				DEBUG_PRINTF("RX api config: req_details\r\n");
			#endif

			if (header->serial_number == 0 || header->serial_number == 0xffffffff || header->serial_number == serial_number_crc32)
				apiconfig_sendDetailsPacket();

			break;

		case PIPX_PACKET_TYPE_REQ_STATE:

			#if defined(APICONFIG_DEBUG)
				DEBUG_PRINTF("RX api config: req_state\r\n");
			#endif

			if (header->serial_number == serial_number_crc32)
				apiconfig_sendStatePacket();

			break;

		case PIPX_PACKET_TYPE_REQ_SETTINGS:	// they are requesting our configuration

			#if defined(APICONFIG_DEBUG)
				DEBUG_PRINTF("RX api config: req_settings\r\n");
			#endif

			if (header->serial_number == serial_number_crc32)
				apiconfig_sendSettingsPacket();

			break;

		case PIPX_PACKET_TYPE_SETTINGS:		// they have sent us new configuration settings
			#if defined(APICONFIG_DEBUG)
				DEBUG_PRINTF("RX api config: settings\r\n");
			#endif

			if (header->serial_number == serial_number_crc32)
			{	// the packet is meant for us

				booting = true;

			    ph_set_remote_serial_number(0, 0);	// stop the packet handler trying to communicate

			    // ******
			    // save the new settings

			    t_pipx_config_settings *settings = (t_pipx_config_settings *)data;

				saved_settings.mode = settings->mode;
				saved_settings.destination_id = settings->destination_id;
				saved_settings.frequency_Hz = settings->frequency_Hz;
				saved_settings.max_tx_power = settings->max_tx_power;
				saved_settings.max_rf_bandwidth = settings->max_rf_bandwidth;
				saved_settings.rf_xtal_cap = settings->rf_xtal_cap;
				saved_settings.serial_baudrate = settings->serial_baudrate;
				saved_settings.aes_enable = settings->aes_enable;
				memcpy((char *)saved_settings.aes_key, (char *)settings->aes_key, sizeof(saved_settings.aes_key));
				saved_settings.rts_time = settings->rts_time;

			    saved_settings_save();	// save them into flash

			    // ******

			    ph_deinit();
			    stream_deinit();
			    ppm_deinit();

			    PIOS_COM_ChangeBaud(PIOS_COM_SERIAL, saved_settings.serial_baudrate);

			    switch (saved_settings.mode)
			    {
			    	case MODE_NORMAL:					// normal 2-way packet mode
			    	case MODE_STREAM_TX:				// 1-way continuous tx packet mode
			    	case MODE_STREAM_RX:				// 1-way continuous rx packet mode
			    	case MODE_PPM_TX:					// PPM tx mode
			    	case MODE_PPM_RX:					// PPM rx mode
			    	case MODE_SCAN_SPECTRUM:			// scan the receiver over the whole band
			    	case MODE_TX_BLANK_CARRIER_TEST:	// blank carrier Tx mode (for calibrating the carrier frequency say)
			    	case MODE_TX_SPECTRUM_TEST:			// pseudo random Tx data mode (for checking the Tx carrier spectrum)
			    		break;
			    	default:							// unknown mode
			    		saved_settings.mode = MODE_NORMAL;
			    		break;
			    }

			    switch (saved_settings.mode)
			    {
			    	case MODE_NORMAL:						// normal 2-way packet mode
			   			ph_init(serial_number_crc32);		// initialise the packet handler
			    		break;

			    	case MODE_STREAM_TX:					// 1-way continuous tx packet mode
			    	case MODE_STREAM_RX:					// 1-way continuous rx packet mode
			   			stream_init(serial_number_crc32);	// initialise the continuous packet stream module
		    			break;

			    	case MODE_PPM_TX:						// PPM tx mode
			    	case MODE_PPM_RX:						// PPM rx mode
			   			ppm_init(serial_number_crc32);		// initialise the PPM module
		    			break;

			    	case MODE_SCAN_SPECTRUM:			// scan the receiver over the whole band
						rfm22_init_scan_spectrum(saved_settings.min_frequency_Hz, saved_settings.max_frequency_Hz);
					    rfm22_setFreqCalibration(saved_settings.rf_xtal_cap);
				    	apiconfig_spec_start_freq = saved_settings.min_frequency_Hz;
				    	apiconfig_spec_freq = apiconfig_spec_start_freq;
				    	apiconfig_spec_buffer_wr = 0;
				    	apiconfig_ss_timer = 0;
			    		break;

			    	case MODE_TX_BLANK_CARRIER_TEST:	// blank carrier Tx mode (for calibrating the carrier frequency say)
			    		rfm22_init_normal(saved_settings.min_frequency_Hz, saved_settings.max_frequency_Hz, rfm22_freqHopSize());
			    	    rfm22_setFreqCalibration(saved_settings.rf_xtal_cap);
			    	    rfm22_setNominalCarrierFrequency(saved_settings.frequency_Hz);
			    	    rfm22_setDatarate(saved_settings.max_rf_bandwidth, TRUE);
			    	    rfm22_setTxPower(saved_settings.max_tx_power);
			    		rfm22_setTxCarrierMode();
			    		break;

			    	case MODE_TX_SPECTRUM_TEST:			// pseudo random Tx data mode (for checking the Tx carrier spectrum)
			    		rfm22_init_normal(saved_settings.min_frequency_Hz, saved_settings.max_frequency_Hz, rfm22_freqHopSize());
			    	    rfm22_setFreqCalibration(saved_settings.rf_xtal_cap);
			    	    rfm22_setNominalCarrierFrequency(saved_settings.frequency_Hz);
			    	    rfm22_setDatarate(saved_settings.max_rf_bandwidth, TRUE);
			    	    rfm22_setTxPower(saved_settings.max_tx_power);
			    		rfm22_setTxPNMode();
			    		break;
			    }

			    // ******

			    booting = false;
			}

			break;

		default:
			#if defined(APICONFIG_DEBUG)
				DEBUG_PRINTF("RX api config: unknown type [%u]\r\n", header->type);
			#endif
			break;
	}
}

uint16_t apiconfig_scanForConfigPacket(void *buf, uint16_t *len, bool rf_packet)
{
	uint16_t length = *len;
	uint16_t i = 0;

	while (TRUE)
	{
		uint32_t crc1, crc2;

		t_pipx_config_header *header = (t_pipx_config_header *)buf + i;
		uint8_t *data = (uint8_t *)header + sizeof(t_pipx_config_header);

		if (i + sizeof(t_pipx_config_header) > length)
		{	// not enough data for a packet
			if (i >= sizeof(header->marker))
				i -= sizeof(header->marker);
			else
				i = 0;
			break;
		}

		if (header->marker != PIPX_HEADER_MARKER)
		{	// no packet marker found
			i++;
			continue;
		}

		// check the header is error free
		crc1 = header->header_crc;
		header->header_crc = 0;
		crc2 = updateCRC32Data(0xffffffff, header, sizeof(t_pipx_config_header));
		header->header_crc = crc1;
		if (crc2 != crc1)
		{	// faulty header or not really a header
			i++;
			continue;
		}

		// valid header found!

		#if defined(APICONFIG_DEBUG)
			DEBUG_PRINTF("RX api config: header found\r\n");
		#endif

		if (!rf_packet)
		{
			apiconfig_rx_config_timer = 0;			// reset timer
			apiconfig_rx_timer = 0;					// reset the timer
		}

		int total_packet_size = sizeof(t_pipx_config_header) + header->data_size;

		if (i + total_packet_size > length)
		{	// not yet got a full packet
			break;
		}

		if (header->data_size > 0)
		{	// check the data is error free
			crc1 = header->data_crc;
			crc2 = updateCRC32Data(0xffffffff, data, header->data_size);
			if (crc2 != crc1)
			{	// faulty data .. remove the entire packet
				length -= total_packet_size;
				if (length - i > 0)
					memmove(buf + i, buf + i + total_packet_size, length - i);
				continue;
			}
		}

		if (!rf_packet)
			apiconfig_processInputPacket(buf + i, length - i);

		// remove the packet from the buffer
		length -= total_packet_size;
		if (length - i > 0)
			memmove(buf + i, buf + i + total_packet_size, length - i);

		break;
	}

	*len = length;

	return i;
}

// *****************************************************************************
// send the data received from the local comm-port to the RF packet handler TX buffer

void apiconfig_processTx(void)
{
	while (TRUE)
	{
		// scan for a configuration packet in the rx buffer
		uint16_t data_size = apiconfig_scanForConfigPacket(apiconfig_rx_buffer, &apiconfig_rx_buffer_wr, false);

		uint16_t time_out = 5;	// ms
		if (!apiconfig_usb_comms) time_out = (15000 * sizeof(t_pipx_config_header)) / saved_settings.serial_baudrate;	// allow enough time to rx a config header
		if (time_out < 5) time_out = 5;

		if (data_size == 0 && apiconfig_rx_timer >= time_out)
			// no config packet found in the buffer within the timeout period, treat any data in the buffer as data to be sent over the air
			data_size = apiconfig_rx_buffer_wr;

		if (data_size == 0)
			break;	// no data to send over the air

		if (saved_settings.mode == MODE_NORMAL || saved_settings.mode == MODE_STREAM_TX)
		{
			// free space size in the RF packet handler tx buffer
			uint16_t ph_num = ph_putData_free(apiconfig_connection_index);

			// set the USART RTS handshaking line
			if (!apiconfig_usb_comms && ph_connected(apiconfig_connection_index))
			{
				if (ph_num < 32)
					SERIAL_RTS_CLEAR;						// lower the USART RTS line - we don't have space in the buffer for anymore bytes
				else
					SERIAL_RTS_SET;							// release the USART RTS line - we have space in the buffer for now bytes
			}
			else
				SERIAL_RTS_SET;								// release the USART RTS line

			#if defined(APICONFIG_DEBUG)
				DEBUG_PRINTF("RX api config: data size %u\r\n", data_size);
			#endif

			if (ph_connected(apiconfig_connection_index))
			{	// we have an RF link to a remote modem

				if (ph_num < data_size)
					break;	// not enough room in the RF packet handler TX buffer for the data

				// copy the data into the RF packet handler TX buffer for sending over the RF link
				data_size = ph_putData(apiconfig_connection_index, apiconfig_rx_buffer, data_size);
			}
		}
		else
		{
			SERIAL_RTS_SET;									// release the USART RTS line - we have space in the buffer for now bytes
		}

		// remove the data from our RX buffer
		apiconfig_rx_buffer_wr -= data_size;
		if (apiconfig_rx_buffer_wr > 0)
			memmove(apiconfig_rx_buffer, apiconfig_rx_buffer + data_size, apiconfig_rx_buffer_wr);
	}
}

// *****************************************************************************
// send data down the local comm port

void apiconfig_processRx(void)
{
	if (apiconfig_tx_config_buffer_wr > 0)
	{	// send any config packets in the config buffer

		while (TRUE)
		{
			uint16_t data_size = apiconfig_tx_config_buffer_wr;

//			if (data_size > 32)
//				data_size = 32;

			if (!apiconfig_usb_comms && !GPIO_IN(SERIAL_CTS_PIN))
				break;	// we can't yet send data down the comm-port

			// send the data out the comm-port
			int32_t res = PIOS_COM_SendBufferNonBlocking(apiconfig_comm_port, apiconfig_tx_config_buffer, data_size);
			if (res < 0)
			{	// failed to send the data out the comm-port

				#if defined(APICONFIG_DEBUG)
					DEBUG_PRINTF("PIOS_COM_SendBuffer %d %d\r\n", data_size, res);
				#endif

				if (apiconfig_tx_timer >= 5000)
				{	// seems we can't send our data for at least the last 5 seconds - delete it
					apiconfig_tx_config_buffer_wr = 0;
				}

				break;
			}

			// data was sent out the comm-port OK .. remove the sent data from our buffer

			#if defined(APICONFIG_DEBUG)
				DEBUG_PRINTF("TX api config: data [%u]\r\n", data_size);
			#endif

			apiconfig_tx_config_buffer_wr -= data_size;
			if (apiconfig_tx_config_buffer_wr > 0)
				memmove(apiconfig_tx_config_buffer, apiconfig_tx_config_buffer + data_size, apiconfig_tx_config_buffer_wr);

			apiconfig_tx_timer = 0;

			break;
		}

		return;
	}

	if (saved_settings.mode == MODE_NORMAL || saved_settings.mode == MODE_STREAM_RX)
	{
		// send the data received via the RF link out the comm-port
		while (TRUE)
		{
			// get number of data bytes received via the RF link
			uint16_t ph_num = ph_getData_used(apiconfig_connection_index);

			// limit to how much space we have in the temp TX buffer
			if (ph_num > sizeof(apiconfig_tx_buffer) - apiconfig_tx_buffer_wr)
				ph_num = sizeof(apiconfig_tx_buffer) - apiconfig_tx_buffer_wr;

			if (ph_num > 0)
			{	// fetch the data bytes received via the RF link and save into our temp buffer
				ph_num = ph_getData(apiconfig_connection_index, apiconfig_tx_buffer + apiconfig_tx_buffer_wr, ph_num);
				apiconfig_tx_buffer_wr += ph_num;
			}

			uint16_t data_size = apiconfig_tx_buffer_wr;
			if (data_size == 0)
				break;	// no data to send

			// we have data to send down the comm-port
/*
			#if (defined(PIOS_COM_DEBUG) && (PIOS_COM_DEBUG == PIOS_COM_SERIAL))
				if (!apiconfig_usb_comms)
				{	// the serial-port is being used for debugging - don't send data down it
					apiconfig_tx_buffer_wr = 0;
					apiconfig_tx_timer = 0;
					continue;
				}
			#endif
*/
			if (!apiconfig_usb_comms && !GPIO_IN(SERIAL_CTS_PIN))
				break;	// we can't yet send data down the comm-port

			// send the data out the comm-port
			int32_t res = PIOS_COM_SendBufferNonBlocking(apiconfig_comm_port, apiconfig_tx_buffer, data_size);
			if (res < 0)
			{	// failed to send the data out the comm-port

				#if defined(APICONFIG_DEBUG)
					DEBUG_PRINTF("PIOS_COM_SendBuffer %d %d\r\n", data_size, res);
				#endif

				if (apiconfig_tx_timer >= 5000)
				{	// seems we can't send our data for at least the last 5 seconds - delete it
					apiconfig_tx_buffer_wr = 0;
				}

				break;
			}

			// data was sent out the comm-port OK .. remove the sent data from our buffer

			apiconfig_tx_buffer_wr -= data_size;
			if (apiconfig_tx_buffer_wr > 0)
				memmove(apiconfig_tx_buffer, apiconfig_tx_buffer + data_size, apiconfig_tx_buffer_wr);

			apiconfig_tx_timer = 0;
		}
	}
}

// *************************************************************

void apiconfig_process_scan_spectrum(void)
{
	if (saved_settings.mode != MODE_SCAN_SPECTRUM)
		return;

	if (apiconfig_ss_timer < 1) return;	// 1ms
	apiconfig_ss_timer = 0;

	if (++led_counter >= 30)
	{
		led_counter = 0;
//		RX_LED_TOGGLE;
		LINK_LED_TOGGLE;
	}

//	uint16_t freq_step_size = (uint16_t)(rfm22_getFrequencyStepSize() * 4 + 0.5f);
	uint16_t freq_step_size = (uint16_t)(rfm22_getFrequencyStepSize() * 20 + 0.5f);

	// fetch the current rssi level
	int16_t rssi_dbm = rfm22_getRSSI();
	if (rssi_dbm < -128) rssi_dbm = -128;
	else
	if (rssi_dbm >  0) rssi_dbm =  0;

	// onto next frequency
	apiconfig_spec_freq += freq_step_size;
	if (apiconfig_spec_freq >= rfm22_maxFrequency()) apiconfig_spec_freq = rfm22_minFrequency();
	rfm22_setNominalCarrierFrequency(apiconfig_spec_freq);
//	rfm22_setRxMode(RX_SCAN_SPECTRUM, true);

	// add the RSSI value to the buffer
	apiconfig_spec_buffer[apiconfig_spec_buffer_wr] = rssi_dbm;
	if (++apiconfig_spec_buffer_wr >= sizeof(apiconfig_spec_buffer))
	{	// send the buffer
		apiconfig_sendSpectrumPacket(apiconfig_spec_start_freq, freq_step_size, apiconfig_spec_buffer, apiconfig_spec_buffer_wr);

		apiconfig_spec_buffer_wr = 0;
		apiconfig_spec_start_freq = apiconfig_spec_freq;
	}
}

// *****************************************************************************
// call this as often as possible - not from an interrupt

void apiconfig_process(void)
{	// copy data from comm-port RX buffer to RF packet handler TX buffer, and from RF packet handler RX buffer to comm-port TX buffer

   	apiconfig_process_scan_spectrum();

	// ********************

	// decide which comm-port we are using (usart or usb)
	apiconfig_usb_comms = false;					// TRUE if we are using the usb port for comms.
	apiconfig_comm_port = PIOS_COM_SERIAL;		// default to using the usart comm-port
	#if defined(PIOS_INCLUDE_USB_HID)
		if (PIOS_USB_HID_CheckAvailable(0))
		{	// USB comms is up, use the USB comm-port instead of the USART comm-port
			apiconfig_usb_comms = true;
			apiconfig_comm_port = PIOS_COM_TELEM_USB;
		}
	#endif

	// check to see if the local communication port has changed (usart/usb)
	if (apiconfig_previous_com_port == 0 && apiconfig_previous_com_port != apiconfig_comm_port)
	{	// the local communications port has changed .. remove any data in the buffers
		apiconfig_rx_buffer_wr = 0;
		apiconfig_tx_buffer_wr = 0;
		apiconfig_tx_config_buffer_wr = 0;
	}
	else
	if (apiconfig_usb_comms)
	{	// we're using the USB for comms - keep the USART rx buffer empty
		int32_t bytes = PIOS_COM_ReceiveBufferUsed(PIOS_COM_SERIAL);
		while (bytes > 0)
		{
			uint8_t c;
			PIOS_COM_ReceiveBuffer(PIOS_COM_SERIAL, &c, 1, 0);
			bytes--;
		}
	}
	apiconfig_previous_com_port = apiconfig_comm_port;		// remember the current comm-port we are using

	apiconfig_connection_index = 0;							// the RF connection we are using

	// *******************
	// fetch the data received via the comm-port

	// get the number of data bytes received down the comm-port
	int32_t com_num = PIOS_COM_ReceiveBufferUsed(apiconfig_comm_port);

	// limit number of bytes we will get to how much space we have in our RX buffer
	if (com_num > sizeof(apiconfig_rx_buffer) - apiconfig_rx_buffer_wr)
		com_num = sizeof(apiconfig_rx_buffer) - apiconfig_rx_buffer_wr;

	
	while (com_num > 0)
	{	// fetch a byte from the comm-port RX buffer and save it into our RX buffer
		uint8_t c;
		PIOS_COM_ReceiveBuffer(apiconfig_comm_port, &c, 1, 0);
		apiconfig_rx_buffer[apiconfig_rx_buffer_wr++] = c;
		com_num--;
	}

	apiconfig_processTx();
	apiconfig_processRx();

	// speed the pinging speed up if the GCS is connected and monitoring the signal level etc
	ph_setFastPing(apiconfig_rx_config_timer < 2000);
}

// *****************************************************************************
// can be called from an interrupt if you wish

void apiconfig_1ms_tick(void)
{	// call this once every 1ms

	if (apiconfig_rx_config_timer < 0xffff) apiconfig_rx_config_timer++;
	if (apiconfig_rx_timer < 0xffff) apiconfig_rx_timer++;
	if (apiconfig_tx_timer < 0xffff) apiconfig_tx_timer++;
	if (apiconfig_ss_timer < 0xffff) apiconfig_ss_timer++;
}

// *****************************************************************************

void apiconfig_init(void)
{
	apiconfig_previous_com_port = 0;

	apiconfig_rx_buffer_wr = 0;

	apiconfig_tx_buffer_wr = 0;

	apiconfig_tx_config_buffer_wr = 0;

	apiconfig_rx_config_timer = 0xffff;
	apiconfig_rx_timer = 0;
	apiconfig_tx_timer = 0;
	apiconfig_ss_timer = 0;

	apiconfig_spec_buffer_wr = 0;
	apiconfig_spec_start_freq = 0;

	led_counter = 0;
}

// *****************************************************************************

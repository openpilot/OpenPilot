/**
 ******************************************************************************
 *
 * @file       ppm.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      Sends or Receives the ppm values to/from the remote unit
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

//#include <string.h>	// memmove

#include "main.h"
#include "rfm22b.h"
#include "saved_settings.h"
#include "ppm.h"

#if defined(PIOS_COM_DEBUG)
	#define PPM_DEBUG
#endif

// *************************************************************
// can be called from an interrupt if you wish
// call this once every ms

void ppm_1ms_tick(void)
{
	if (booting) return;

	if (saved_settings.mode == MODE_PPM_TX)
	{
	}
	else
	if (saved_settings.mode == MODE_PPM_RX)
	{
	}
}

// *************************************************************
// return a byte for the tx packet transmission.
//
// return value < 0 if no more bytes available, otherwise return byte to be sent

int16_t ppm_TxDataByteCallback(void)
{
	return -1;
}

// *************************************************************
// we are being given a received byte
//
// return TRUE to continue current packet receive, otherwise return FALSe to halt current packet reception

bool ppm_RxDataByteCallback(uint8_t b)
{
	return true;
}

// *************************************************************
// call this from the main loop (not interrupt) as often as possible

void ppm_process(void)
{
	if (booting) return;

	if (saved_settings.mode == MODE_PPM_TX)
	{
	}
	else
	if (saved_settings.mode == MODE_PPM_RX)
	{
	}
}

// *************************************************************

void ppm_init(uint32_t our_sn)
{
	#if defined(PPM_DEBUG)
		DEBUG_PRINTF("\r\nPPM init\r\n");
	#endif

	if (saved_settings.mode == MODE_PPM_TX)
		rfm22_init_tx_stream(saved_settings.min_frequency_Hz, saved_settings.max_frequency_Hz);
	else
	if (saved_settings.mode == MODE_PPM_RX)
		rfm22_init_rx_stream(saved_settings.min_frequency_Hz, saved_settings.max_frequency_Hz);

	rfm22_TxDataByte_SetCallback(ppm_TxDataByteCallback);
	rfm22_RxDataByte_SetCallback(ppm_RxDataByteCallback);

    rfm22_setFreqCalibration(saved_settings.rf_xtal_cap);
	rfm22_setNominalCarrierFrequency(saved_settings.frequency_Hz);
	rfm22_setDatarate(saved_settings.max_rf_bandwidth, FALSE);
	rfm22_setTxPower(saved_settings.max_tx_power);

	rfm22_setTxStream();			// TEST ONLY
}

// *************************************************************

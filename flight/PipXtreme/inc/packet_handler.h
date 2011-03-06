/**
 ******************************************************************************
 *
 * @file       packet_handler.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      Modem packet handling routines
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

#ifndef __PACKET_HANDLER_H__
#define __PACKET_HANDLER_H__

#include "stdint.h"

// *****************************************************************************

#define PH_MAX_CONNECTIONS      1	// maximum number of remote connections

// *****************************************************************************

void ph_1ms_tick(void);
void ph_process(void);

bool ph_connected(const int connection_index);

uint16_t ph_putData_free(const int connection_index);
uint16_t ph_putData(const int connection_index, const void *data, uint16_t len);

uint16_t ph_getData_used(const int connection_index);
uint16_t ph_getData(const int connection_index, void *data, uint16_t len);

void ph_setFastPing(bool fast);

uint16_t ph_getRetries(const int connection_index);

uint8_t ph_getCurrentLinkState(const int connection_index);

int16_t ph_getLastRSSI(const int connection_index);
int32_t ph_getLastAFC(const int connection_index);

void ph_setNominalCarrierFrequency(uint32_t frequency_hz);
uint32_t ph_getNominalCarrierFrequency(void);

void ph_setDatarate(uint32_t datarate_bps);
uint32_t ph_getDatarate(void);

void ph_setTxPower(uint8_t tx_power);
uint8_t ph_getTxPower(void);

void ph_set_AES128_key(const void *key);

int ph_set_remote_serial_number(int connection_index, uint32_t sn);
void ph_set_remote_encryption(int connection_index, bool enabled, const void *key);

void ph_deinit(void);
void ph_init(uint32_t our_sn);

// *****************************************************************************

#endif

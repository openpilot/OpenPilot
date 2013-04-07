/**
  ******************************************************************************
  * @addtogroup PIOS PIOS Core hardware abstraction layer
  * @{
  * @addtogroup PIOS_ETASV3 ETASV3 Functions
  * @brief Hardware functions to deal with the Eagle Tree Airspeed MicroSensor V3
  * @{
  *
  * @file       pios_etasv3.c
  * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2012.
  * @brief      ETASV3 Airspeed Sensor Driver
  * @see        The GNU Public License (GPL) Version 3
  *
  ******************************************************************************/
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

#include "pios.h"

#ifdef PIOS_INCLUDE_ETASV3

static bool PIOS_ETASV3_Read(uint8_t * buffer, uint8_t len)
{
	const struct pios_i2c_txn txn_list[] = {
		{
		 .info = __func__,
		 .addr = ETASV3_I2C_ADDR,
		 .rw = PIOS_I2C_TXN_READ,
		 .len = len,
		 .buf = buffer,
		 }
	};

	return PIOS_I2C_Transfer(PIOS_I2C_ETASV3_ADAPTER, txn_list, NELEMENTS(txn_list));
}

int16_t PIOS_ETASV3_ReadAirspeed (void)
{
	uint8_t airspeed_raw[2];

	if (PIOS_ETASV3_Read(airspeed_raw, sizeof(airspeed_raw)) != 0) {
		/* Failed to read airspeed */
		return -1;
	}

	return (airspeed_raw[0] | (airspeed_raw[1]<<8));
}

#endif /* PIOS_INCLUDE_ETASV3 */

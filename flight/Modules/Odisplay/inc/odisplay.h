/**
 ******************************************************************************
 * @addtogroup OpenPilotModules OpenPilot Modules
 * @{ 
 * @addtogroup AirspeedModule Airspeed Module
 * @brief Communicate with the EagleTree V3 Airspeed sensor (ETASV3) and update @ref BaroAirspeed "BaroAirspeed UAV Object"
 * @{ 
 *
 * @file       airspeed.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2012.
 * @brief      Airspeed module, handles airspeed readings from ETASV3
 *
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
#ifndef ODISPLAY_H
#define ODISPLAY_H

enum Odisplay_UAVO {
	UAVO_TX  = 0x01,
	UAVO_RX  = 0x02,
	UAVO_RSSI = 0x03,
	UAVO_ALTITUDE = 0x04,
	UAVO_VARIOMETER = 0x05,
	UAVO_GALTITUDE = 0x06,
	UAVO_GSPEED = 0x07,
	UAVO_TEMPERATURE = 0x08,
	UAVO_BATTERY = 0x09,
	UAVO_NONE = 0x00
};

#endif // ODISPLAY_H

/**
 * @}
 * @}
 */

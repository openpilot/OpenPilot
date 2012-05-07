/**
 ******************************************************************************
 * @addtogroup OpenPilotModules OpenPilot Modules
 * @{
 * @addtogroup GSPModule GPS Module
 * @brief Process GPS information (UBX binary format)
 * @{
 *
 * @file       UBX.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2012.
 * @brief      GPS module, handles GPS and NMEA stream
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

#include "openpilot.h"
#include "pios.h"
#include "UBX.h"
#include "gpsvelocity.h"

bool checksum_ubx_message (UBXPacket *ubx)
{
	int i;
	uint8_t ck_a, ck_b;

	ck_a = ubx->header.class;
	ck_b = ck_a;

	ck_a += ubx->header.id;
	ck_b += ck_a;

	ck_a += ubx->header.len & 0xff;
	ck_b += ck_a;

	ck_a += ubx->header.len >> 8;
	ck_b += ck_a;



	for (i = 0; i < ubx->header.len; i++)
	{
		ck_a += ubx->payload.payload[i];
		ck_b += ck_a;
	}

	if (ubx->header.ck_a == ck_a &&
			ubx->header.ck_b == ck_b)
	{
		return true;
	}
	else
		return false;

}

void parse_ubx_nav_velned (UBXPayload payload)
{
#if defined(REVOLUTION)
	GPSVelocityData GpsVelocity;
	GPSVelocityGet(&GpsVelocity);

	GpsVelocity.North	= (float)payload.nav_velned.velN/100.0f;
	GpsVelocity.East	= (float)payload.nav_velned.velE/100.0f;
	GpsVelocity.Down	= (float)payload.nav_velned.velD/100.0f;

	GPSVelocitySet(&GpsVelocity);
#endif
}

void parse_ubx_message (UBXPacket *ubx)
{
	switch (ubx->header.class)
	{
		case UBX_CLASS_NAV:
			switch (ubx->header.id)
			{
				case UBX_ID_VELNED:
					parse_ubx_nav_velned (ubx->payload);
			}
			break;
	}
}



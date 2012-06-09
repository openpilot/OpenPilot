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
#include "gpsvelocity.h"
#include "gpssatellites.h"
#include "gpsposition.h"
#include "gpstime.h"
#include "UBX.h"


// Keep track of various GPS messages needed to make up a single UAVO update
// time-of-week timestamp is used to correlate matching messages

#define POSLLH_RECEIVED	(1 << 0)
#define STATUS_RECEIVED	(1 << 1)
#define DOP_RECEIVED	(1 << 2)
#define VELNED_RECEIVED	(1 << 3)
#define SOL_RECEIVED	(1 << 4)
#define	ALL_RECEIVED	(SOL_RECEIVED | VELNED_RECEIVED | DOP_RECEIVED | POSLLH_RECEIVED)
#define NONE_RECEIVED	0

static struct msgtracker{
		uint32_t	currentTOW;		// TOW of the message set currently in progress
		uint8_t		msg_received;	// Flag received messages
	} msgtracker;

// Check if a message belongs to the current data set and register it as 'received'
bool check_msgtracker (uint32_t tow, uint8_t msg_flag)
{
	if ((tow > msgtracker.currentTOW) || 								// start of a new message set
		(tow < 60000 && msgtracker.currentTOW > (7*24-1)*3600*1000))	// TOW wrap around
	{
		msgtracker.currentTOW = tow;
		msgtracker.msg_received = NONE_RECEIVED;
	}
	else
	{
		if (tow < msgtracker.currentTOW)	// message outdated (don't process)
				return false;
	}

	msgtracker.msg_received |= msg_flag;
	return true;
}

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

void parse_ubx_nav_posllh (UBXPayload payload, GPSPositionData *gpsposition)
{
	if (check_msgtracker(payload.nav_posllh.iTOW, POSLLH_RECEIVED))
	{
		gpsposition->Altitude = (float)payload.nav_posllh.hMSL*0.001f;
		gpsposition->GeoidSeparation = (float)(payload.nav_posllh.height -
				payload.nav_posllh.hMSL)*0.001f;
		gpsposition->Latitude = payload.nav_posllh.lat;
		gpsposition->Longitude = payload.nav_posllh.lon;
	}
}

void parse_ubx_nav_status (UBXPayload payload, GPSPositionData *gpsposition)
{
	if (check_msgtracker(payload.nav_status.iTOW, STATUS_RECEIVED))
	{
		switch (payload.nav_status.gpsFix)
		{
			case STATUS_GPSFIX_2DFIX:
				gpsposition->Status = GPSPOSITION_STATUS_FIX2D;
				break;
			case STATUS_GPSFIX_3DFIX:
				gpsposition->Status = GPSPOSITION_STATUS_FIX3D;
				break;
			default: gpsposition->Status = GPSPOSITION_STATUS_NOFIX;
		}
	}
}

void parse_ubx_nav_sol (UBXPayload payload, GPSPositionData *gpsposition)
{
	if (check_msgtracker(payload.nav_dop.iTOW, SOL_RECEIVED))
	{
		gpsposition->Satellites = (float)payload.nav_sol.numSV;
		switch (payload.nav_sol.gpsFix)
		{
			case STATUS_GPSFIX_2DFIX:
				gpsposition->Status = GPSPOSITION_STATUS_FIX2D;
				break;
			case STATUS_GPSFIX_3DFIX:
				gpsposition->Status = GPSPOSITION_STATUS_FIX3D;
				break;
			default: gpsposition->Status = GPSPOSITION_STATUS_NOFIX;
		}
	}
}

void parse_ubx_nav_dop (UBXPayload payload, GPSPositionData *gpsposition)
{
	if (check_msgtracker(payload.nav_dop.iTOW, DOP_RECEIVED))
	{
		gpsposition->HDOP = (float)payload.nav_dop.hDOP * 0.01f;
		gpsposition->VDOP = (float)payload.nav_dop.vDOP * 0.01f;
		gpsposition->PDOP = (float)payload.nav_dop.pDOP * 0.01f;
	}
}

void parse_ubx_nav_velned (UBXPayload payload, GPSPositionData *gpsposition)
{
	GPSVelocityData GpsVelocity;

	GpsVelocity.North	= (float)payload.nav_velned.velN/100.0f;
	GpsVelocity.East	= (float)payload.nav_velned.velE/100.0f;
	GpsVelocity.Down	= (float)payload.nav_velned.velD/100.0f;

	GPSVelocitySet(&GpsVelocity);

	if (check_msgtracker(payload.nav_velned.iTOW, VELNED_RECEIVED))
	{
		gpsposition->Groundspeed = (float)payload.nav_velned.gSpeed * 0.01f;
		gpsposition->Heading = (float)payload.nav_velned.heading * 1.0e-5f;
	}

}

void parse_ubx_nav_timeutc (UBXPayload payload)
{
	if (!(payload.nav_timeutc.valid & TIMEUTC_VALIDUTC))
		return;

	GPSTimeData gpst;

	gpst.Year = payload.nav_timeutc.year;
	gpst.Month = payload.nav_timeutc.month;
	gpst.Day = payload.nav_timeutc.day;
	gpst.Hour = payload.nav_timeutc.hour;
	gpst.Minute = payload.nav_timeutc.min;
	gpst.Second = payload.nav_timeutc.sec;

	GPSTimeSet(&gpst);
}

void parse_ubx_nav_svinfo (UBXPayload payload)
{
	GPSSatellitesData svdata;
	uint8_t chan;
	svdata.SatsInView = 0;
	for (chan = 0; chan < payload.nav_svinfo.numCh;	chan++)
	{
		if ((payload.nav_svinfo.sv[chan].elev > 0) &&	// some unhealthy SV report negative elevation
				(svdata.SatsInView < GPSSATELLITES_PRN_NUMELEM))
		{
			svdata.Azimuth[svdata.SatsInView] = (float)payload.nav_svinfo.sv[chan].azim;
			svdata.Elevation[svdata.SatsInView] = (float)payload.nav_svinfo.sv[chan].elev;
			svdata.PRN[svdata.SatsInView] = payload.nav_svinfo.sv[chan].svid;
			svdata.SNR[svdata.SatsInView] = payload.nav_svinfo.sv[chan].cno;
			svdata.SatsInView++;
		}

	}
	// fill remaining slots (if any)
	for (chan = svdata.SatsInView; chan < GPSSATELLITES_PRN_NUMELEM; chan++)
	{
		svdata.Azimuth[chan] = (float)0.0f;
		svdata.Elevation[chan] = (float)0.0f;
		svdata.PRN[chan] = 0;
		svdata.SNR[chan] = 0;
	}

	GPSSatellitesSet(&svdata);
}

// UBX message parser
// returns true on valid position updates

bool parse_ubx_message (UBXPacket *ubx, GPSPositionData *gpsposition)
{
#if defined(REVOLUTION)

	switch (ubx->header.class)
	{
		case UBX_CLASS_NAV:
			switch (ubx->header.id)
			{
				case UBX_ID_POSLLH:
					parse_ubx_nav_posllh (ubx->payload, gpsposition);
					break;
				case UBX_ID_STATUS:
					parse_ubx_nav_status (ubx->payload, gpsposition);
					break;
				case UBX_ID_DOP:
					parse_ubx_nav_dop (ubx->payload, gpsposition);
					break;
				case UBX_ID_SOL:
					parse_ubx_nav_sol (ubx->payload, gpsposition);
					break;
				case UBX_ID_VELNED:
					parse_ubx_nav_velned (ubx->payload, gpsposition);
					break;
				case UBX_ID_TIMEUTC:
					parse_ubx_nav_timeutc (ubx->payload);
					break;
				case UBX_ID_SVINFO:
					parse_ubx_nav_svinfo (ubx->payload);
					break;
			}
			break;
	}
	if (msgtracker.msg_received == ALL_RECEIVED)
	{
		GPSPositionSet(gpsposition);
		msgtracker.msg_received = NONE_RECEIVED;
		return true;
	}
	return false;
#endif
}



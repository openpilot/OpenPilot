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

#if defined(PIOS_INCLUDE_GPS_UBX_PARSER)

#include "UBX.h"
#include "GPS.h"

// parse incoming character stream for messages in UBX binary format

int parse_ubx_stream (uint8_t c, char *gps_rx_buffer, GPSPositionData *GpsData, struct GPS_RX_STATS *gpsRxStats)
{
	enum proto_states {
		START,
		UBX_SY2,
		UBX_CLASS,
		UBX_ID,
		UBX_LEN1,
		UBX_LEN2,
		UBX_PAYLOAD,
		UBX_CHK1,
		UBX_CHK2,
		FINISHED
	};

	static enum proto_states proto_state = START;
	static uint8_t rx_count = 0;
	struct UBXPacket *ubx = (struct UBXPacket *)gps_rx_buffer;

	switch (proto_state) {
		case START: // detect protocol
			if (c ==  UBX_SYNC1) // first UBX sync char found
				proto_state = UBX_SY2;
			break;
		case UBX_SY2:
			if (c == UBX_SYNC2) // second UBX sync char found
				proto_state = UBX_CLASS;
			else
				proto_state = START; // reset state
			break;
		case UBX_CLASS:
			ubx->header.class = c;
			proto_state = UBX_ID;
			break;
		case UBX_ID:
			ubx->header.id = c;
			proto_state = UBX_LEN1;
			break;
		case UBX_LEN1:
			ubx->header.len = c;
			proto_state = UBX_LEN2;
			break;
		case UBX_LEN2:
			ubx->header.len += (c << 8);
			if (ubx->header.len > sizeof(UBXPayload)) {
				gpsRxStats->gpsRxOverflow++;
				proto_state = START;
			} else {
				rx_count = 0;
				proto_state = UBX_PAYLOAD;
			}
			break;
		case UBX_PAYLOAD:
			if (rx_count < ubx->header.len) {
				ubx->payload.payload[rx_count] = c;
				if (++rx_count == ubx->header.len)
					proto_state = UBX_CHK1;
			} else {
				gpsRxStats->gpsRxOverflow++;
				proto_state = START;
			}
			break;
		case UBX_CHK1:
			ubx->header.ck_a = c;
			proto_state = UBX_CHK2;
			break;
		case UBX_CHK2:
			ubx->header.ck_b = c;
			if (checksum_ubx_message(ubx)) { // message complete and valid
				parse_ubx_message(ubx, GpsData);
				proto_state = FINISHED;
			} else {
				gpsRxStats->gpsRxChkSumError++;
				proto_state = START;
			}
			break;
		default: break;
	}

	if (proto_state == START)
		return PARSER_ERROR;	// parser couldn't use this byte
	else if (proto_state == FINISHED) {
		gpsRxStats->gpsRxReceived++;
		proto_state = START;
		return PARSER_COMPLETE;	// message complete & processed
	}

	return PARSER_INCOMPLETE; // message not (yet) complete
}


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
		uint8_t		msg_received;	// keep track of received message types
	} msgtracker;

// Check if a message belongs to the current data set and register it as 'received'
bool check_msgtracker (uint32_t tow, uint8_t msg_flag)
{

	if (tow > msgtracker.currentTOW ? true                  // start of a new message set
		: (msgtracker.currentTOW - tow > 6*24*3600*1000)) { // 6 days, TOW wrap around occured
		msgtracker.currentTOW = tow;
		msgtracker.msg_received = NONE_RECEIVED;
	} else if (tow < msgtracker.currentTOW)	// message outdated (don't process)
				return false;

	msgtracker.msg_received |= msg_flag; // register reception of this msg type
	return true;
}

bool checksum_ubx_message (struct UBXPacket *ubx)
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

	for (i = 0; i < ubx->header.len; i++) {
		ck_a += ubx->payload.payload[i];
		ck_b += ck_a;
	}

	if (ubx->header.ck_a == ck_a &&
			ubx->header.ck_b == ck_b)
		return true;
	else
		return false;

}

void parse_ubx_nav_posllh (struct UBX_NAV_POSLLH *posllh, GPSPositionData *GpsPosition)
{
	if (check_msgtracker(posllh->iTOW, POSLLH_RECEIVED)) {
		if (GpsPosition->Status != GPSPOSITION_STATUS_NOFIX) {
			GpsPosition->Altitude = (float)posllh->hMSL*0.001f;
			GpsPosition->GeoidSeparation = (float)(posllh->height - posllh->hMSL)*0.001f;
			GpsPosition->Latitude = posllh->lat;
			GpsPosition->Longitude = posllh->lon;
		}
	}
}

void parse_ubx_nav_sol (struct UBX_NAV_SOL *sol, GPSPositionData *GpsPosition)
{
	if (check_msgtracker(sol->iTOW, SOL_RECEIVED)) {
		GpsPosition->Satellites = sol->numSV;

		if (sol->flags & STATUS_FLAGS_GPSFIX_OK) {
			switch (sol->gpsFix) {
				case STATUS_GPSFIX_2DFIX:
					GpsPosition->Status = GPSPOSITION_STATUS_FIX2D;
					break;
				case STATUS_GPSFIX_3DFIX:
					GpsPosition->Status = GPSPOSITION_STATUS_FIX3D;
					break;
				default: GpsPosition->Status = GPSPOSITION_STATUS_NOFIX;
			}
		}
		else // fix is not valid so we make sure to treat is as NOFIX
			GpsPosition->Status = GPSPOSITION_STATUS_NOFIX;
	}
}

void parse_ubx_nav_dop (struct UBX_NAV_DOP *dop, GPSPositionData *GpsPosition)
{
	if (check_msgtracker(dop->iTOW, DOP_RECEIVED)) {
		GpsPosition->HDOP = (float)dop->hDOP * 0.01f;
		GpsPosition->VDOP = (float)dop->vDOP * 0.01f;
		GpsPosition->PDOP = (float)dop->pDOP * 0.01f;
	}
}

void parse_ubx_nav_velned (struct UBX_NAV_VELNED *velned, GPSPositionData *GpsPosition)
{
	GPSVelocityData GpsVelocity;

	if (check_msgtracker(velned->iTOW, VELNED_RECEIVED)) {
		if (GpsPosition->Status != GPSPOSITION_STATUS_NOFIX) {
			GpsVelocity.North	= (float)velned->velN/100.0f;
			GpsVelocity.East	= (float)velned->velE/100.0f;
			GpsVelocity.Down	= (float)velned->velD/100.0f;
			GPSVelocitySet(&GpsVelocity);
			GpsPosition->Groundspeed = (float)velned->gSpeed * 0.01f;
			GpsPosition->Heading = (float)velned->heading * 1.0e-5f;
		}
	}
}

#if !defined(PIOS_GPS_MINIMAL)
void parse_ubx_nav_timeutc (struct UBX_NAV_TIMEUTC *timeutc)
{
	if (!(timeutc->valid & TIMEUTC_VALIDUTC))
		return;

	GPSTimeData GpsTime;

	GpsTime.Year = timeutc->year;
	GpsTime.Month = timeutc->month;
	GpsTime.Day = timeutc->day;
	GpsTime.Hour = timeutc->hour;
	GpsTime.Minute = timeutc->min;
	GpsTime.Second = timeutc->sec;

	GPSTimeSet(&GpsTime);
}
#endif

#if !defined(PIOS_GPS_MINIMAL)
void parse_ubx_nav_svinfo (struct UBX_NAV_SVINFO *svinfo)
{
	uint8_t chan;
	GPSSatellitesData svdata;

	svdata.SatsInView = 0;
	for (chan = 0; chan < svinfo->numCh;	chan++) {
		if (svdata.SatsInView < GPSSATELLITES_PRN_NUMELEM) {
			svdata.Azimuth[svdata.SatsInView] = (float)svinfo->sv[chan].azim;
			svdata.Elevation[svdata.SatsInView] = (float)svinfo->sv[chan].elev;
			svdata.PRN[svdata.SatsInView] = svinfo->sv[chan].svid;
			svdata.SNR[svdata.SatsInView] = svinfo->sv[chan].cno;
			svdata.SatsInView++;
		}

	}
	// fill remaining slots (if any)
	for (chan = svdata.SatsInView; chan < GPSSATELLITES_PRN_NUMELEM; chan++) {
		svdata.Azimuth[chan] = (float)0.0f;
		svdata.Elevation[chan] = (float)0.0f;
		svdata.PRN[chan] = 0;
		svdata.SNR[chan] = 0;
	}

	GPSSatellitesSet(&svdata);
}
#endif

// UBX message parser
// returns UAVObjectID if a UAVObject structure is ready for further processing

uint32_t parse_ubx_message (struct UBXPacket *ubx, GPSPositionData *GpsPosition)
{
	uint32_t id = 0;

	switch (ubx->header.class) {
		case UBX_CLASS_NAV:
			switch (ubx->header.id) {
				case UBX_ID_POSLLH:
					parse_ubx_nav_posllh (&ubx->payload.nav_posllh, GpsPosition);
					break;
				case UBX_ID_DOP:
					parse_ubx_nav_dop (&ubx->payload.nav_dop, GpsPosition);
					break;
				case UBX_ID_SOL:
					parse_ubx_nav_sol (&ubx->payload.nav_sol, GpsPosition);
					break;
				case UBX_ID_VELNED:
					parse_ubx_nav_velned (&ubx->payload.nav_velned, GpsPosition);
					break;
#if !defined(PIOS_GPS_MINIMAL)
				case UBX_ID_TIMEUTC:
					parse_ubx_nav_timeutc (&ubx->payload.nav_timeutc);
					break;
				case UBX_ID_SVINFO:
					parse_ubx_nav_svinfo (&ubx->payload.nav_svinfo);
					break;
#endif
			}
			break;
	}
	if (msgtracker.msg_received == ALL_RECEIVED) {
		GPSPositionSet(GpsPosition);
		msgtracker.msg_received = NONE_RECEIVED;
		id = GPSPOSITION_OBJID;
	}
	return id;
}

#endif // PIOS_INCLUDE_GPS_UBX_PARSER


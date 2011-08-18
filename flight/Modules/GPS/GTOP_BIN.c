/**
 ******************************************************************************
 * @addtogroup OpenPilotModules OpenPilot Modules
 * @{
 * @addtogroup GSPModule GPS Module
 * @brief Process GPS information
 * @{
 *
 * @file       GTOP_BIN.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
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
#include "GTOP_BIN.h"
#include "gpsposition.h"
#include "gpstime.h"
#include "gpssatellites.h"

#include <string.h>	// memmove

#ifdef ENABLE_GPS_BINARY_GTOP

// ************
// the structure of the binary packet

typedef struct
{
	uint32_t  utc_time;

	int32_t   latitude;
	uint8_t   ns_indicator;

	int32_t   longitude;
	uint8_t   ew_indicator;

	uint8_t   fix_quality;

	uint8_t   satellites_used;

	uint16_t  hdop;

	int32_t   msl_altitude;

	int32_t   geoidal_seperation;

	uint8_t   fix_type;

	int32_t   course_over_ground;

	int32_t   speed_over_ground;

	uint8_t   day;
	uint8_t   month;
	uint16_t  year;
}  __attribute__((__packed__)) t_gps_bin_packet_data;

typedef struct
{
	uint16_t              header;
	t_gps_bin_packet_data data;
	uint8_t               asterisk;
	uint8_t               checksum;
	uint16_t              end_word;
}  __attribute__((__packed__)) t_gps_bin_packet;

// ************

// buffer that holds the incoming binary packet
static uint8_t gps_rx_buffer[sizeof(t_gps_bin_packet)] __attribute__ ((aligned(4)));

// number of bytes currently in the rx buffer
static int16_t gps_rx_buffer_wr = 0;

// ************
// endian swapping functions

static uint16_t swap2Bytes(uint16_t data)
{
	return (((data >> 8) & 0x00ff) |
			((data << 8) & 0xff00));
}

static uint32_t swap4Bytes(uint32_t data)
{
	return (((data >> 24) & 0x000000ff) |
			((data >>  8) & 0x0000ff00) |
			((data <<  8) & 0x00ff0000) |
			((data << 24) & 0xff000000));
}

// ************
/**
 * Parses a complete binary packet and update the GPSPosition and GPSTime UAVObjects
 *
 * param[in] .. b = a new received byte from the GPS
 *
 * return '0' if we have found a valid binary packet
 * return <0 if any errors were encountered with the packet or no packet found
 */

int GTOP_BIN_update_position(uint8_t b, volatile uint32_t *chksum_errors, volatile uint32_t *parsing_errors)
{
	if (gps_rx_buffer_wr >= sizeof(gps_rx_buffer))
	{	// make room for the new byte .. this will actually never get executed, just here as a safe guard really
		memmove(gps_rx_buffer, gps_rx_buffer + 1, sizeof(gps_rx_buffer) - 1);
		gps_rx_buffer_wr = sizeof(gps_rx_buffer) - 1;
	}

	// add the new byte into the buffer
	gps_rx_buffer[gps_rx_buffer_wr++] = b;

	int16_t i = 0;

	while (gps_rx_buffer_wr > 0)
	{
		t_gps_bin_packet *rx_packet = (t_gps_bin_packet *)(gps_rx_buffer + i);

		// scan for the start of a binary packet (the header bytes)
		while (gps_rx_buffer_wr - i >= sizeof(rx_packet->header))
		{
			if (rx_packet->header == 0x2404)
				break;   // found a valid header marker
			rx_packet = (t_gps_bin_packet *)(gps_rx_buffer + ++i);
		}

		// remove unwanted bytes before the start of the packet header
		if (i > 0)
		{
			gps_rx_buffer_wr -= i;
			if (gps_rx_buffer_wr > 0)
				memmove(gps_rx_buffer, gps_rx_buffer + i, gps_rx_buffer_wr);
			i = 0;
		}

		if (gps_rx_buffer_wr < sizeof(t_gps_bin_packet))
			break;   // not yet enough bytes for a complete binary packet

		// we have enough bytes for a complete binary packet

		// check to see if certain parameters in the binary packet are valid
		if (rx_packet->header != 0x2404 ||
			rx_packet->end_word != 0x0A0D ||
			rx_packet->asterisk != 0x2A ||
			(rx_packet->data.ns_indicator != 1 && rx_packet->data.ns_indicator != 2) ||
			(rx_packet->data.ew_indicator != 1 && rx_packet->data.ew_indicator != 2) ||
			(rx_packet->data.fix_quality > 2) ||
			(rx_packet->data.fix_type < 1 || rx_packet->data.fix_type > 3) )
		{   // invalid packet
			if (parsing_errors) *parsing_errors++;
			i++;
			continue;
		}

		{   // check the checksum is valid
			uint8_t *p = (uint8_t *)&rx_packet->data;
			uint8_t checksum = 0;
			for (int i = 0; i < sizeof(t_gps_bin_packet_data); i++)
				checksum ^= *p++;

			if (checksum != rx_packet->checksum)
			{	// checksum error
				if (chksum_errors) *chksum_errors++;
				i++;
				continue;
			}
		}

		// we now have a valid complete binary packet, update the GpsData and GpsTime objects

		// correct the endian order of the parameters
		rx_packet->data.utc_time = swap4Bytes(rx_packet->data.utc_time);
		rx_packet->data.latitude = swap4Bytes(rx_packet->data.latitude);
		rx_packet->data.longitude = swap4Bytes(rx_packet->data.longitude);
		rx_packet->data.hdop = swap2Bytes(rx_packet->data.hdop);
		rx_packet->data.msl_altitude = swap4Bytes(rx_packet->data.msl_altitude);
		rx_packet->data.geoidal_seperation = swap4Bytes(rx_packet->data.geoidal_seperation);
		rx_packet->data.course_over_ground = swap4Bytes(rx_packet->data.course_over_ground);
		rx_packet->data.speed_over_ground = swap4Bytes(rx_packet->data.speed_over_ground);
		rx_packet->data.year = swap2Bytes(rx_packet->data.year);

#if !defined(PIOS_GPS_PURISTIC)
		// set the gps time object
		GPSTimeData GpsTime;
//		GPSTimeGet(&GpsTime);
			uint32_t utc_time = rx_packet->data.utc_time / 1000;
			GpsTime.Second = utc_time % 100;          // seconds
			GpsTime.Minute = (utc_time / 100) % 100;  // minutes
			GpsTime.Hour = utc_time / 10000;          // hours
			GpsTime.Day = rx_packet->data.day;        // day
			GpsTime.Month = rx_packet->data.month;    // month
			GpsTime.Year = rx_packet->data.year;      // year
		GPSTimeSet(&GpsTime);
#endif //PIOS_GPS_PURISTIC

		// set the gps position object
		GPSPositionData	GpsData;
//		GPSPositionGet(&GpsData);
			switch (rx_packet->data.fix_type)
			{
				case 1: GpsData.Status = GPSPOSITION_STATUS_NOFIX; break;
				case 2: GpsData.Status = GPSPOSITION_STATUS_FIX2D; break;
				case 3: GpsData.Status = GPSPOSITION_STATUS_FIX3D; break;
				default: GpsData.Status = GPSPOSITION_STATUS_NOGPS; break;
			}
			GpsData.Latitude        = rx_packet->data.latitude  * (rx_packet->data.ns_indicator == 1 ? +1 : -1) * 10;   // degrees * 10e6
			GpsData.Longitude       = rx_packet->data.longitude * (rx_packet->data.ew_indicator == 1 ? +1 : -1) * 10;	// degrees * 10e6
			GpsData.Altitude        = (float)rx_packet->data.msl_altitude / 1000;                                       // meters
			GpsData.GeoidSeparation = (float)rx_packet->data.geoidal_seperation / 1000;                                 // meters
			GpsData.Heading         = (float)rx_packet->data.course_over_ground / 1000;                                 // degrees
			GpsData.Groundspeed     = (float)rx_packet->data.speed_over_ground / 3600;                                  // m/s
			GpsData.Satellites      = rx_packet->data.satellites_used;                                                  //
			GpsData.PDOP            = 99.99;                                                                            // not available in binary mode
			GpsData.HDOP            = (float)rx_packet->data.hdop / 100;                                                //
			GpsData.VDOP            = 99.99;                                                                            // not available in binary mode
		GPSPositionSet(&GpsData);

#if !defined(PIOS_GPS_PURISTIC)
		// set the number of satellites
//		GPSSatellitesData SattelliteData;
////		GPSSatellitesGet(&SattelliteData);
//			memset(&SattelliteData, 0, sizeof(SattelliteData));
//			SattelliteData.SatsInView = rx_packet->data.satellites_used;                                                    //
//		GPSSatellitesSet(&SattelliteData);
#endif //PIOS_GPS_PURISTIC

		// remove the spent binary packet from the buffer
		gps_rx_buffer_wr -= sizeof(t_gps_bin_packet);
		if (gps_rx_buffer_wr > 0)
			memmove(gps_rx_buffer, gps_rx_buffer + sizeof(t_gps_bin_packet), gps_rx_buffer_wr);

		return 0;  // found a valid packet
	}

	return -1;     // no valid packet found
}

// ************

void GTOP_BIN_init(void)
{
	gps_rx_buffer_wr = 0;
}

// ************

#endif // ENABLE_GPS_BINARY_GTOP


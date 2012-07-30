/**
 ******************************************************************************
 * @addtogroup OpenPilotModules OpenPilot Modules
 * @{
 * @addtogroup GSPModule GPS Module
 * @brief Process GPS information
 * @{
 *
 * @file       NMEA.h
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

#ifndef NMEA_H
#define NMEA_H

#include <stdbool.h>
#include <stdint.h>
#include "GPS.h"

#define NMEA_MAX_PACKET_LENGTH          96 // 82 max NMEA msg size plus 12 margin (because some vendors add custom crap) plus CR plus Linefeed

extern bool NMEA_update_position(char *nmea_sentence, GPSPositionData *GpsData);
extern bool NMEA_checksum(char *nmea_sentence);
extern int parse_nmea_stream(uint8_t, char *, GPSPositionData *, struct GPS_RX_STATS *);

#endif /* NMEA_H */

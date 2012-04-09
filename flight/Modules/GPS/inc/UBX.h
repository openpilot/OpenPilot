/**
 ******************************************************************************
 * @addtogroup OpenPilotModules OpenPilot Modules
 * @{
 * @addtogroup GSPModule GPS Module
 * @brief Process GPS information
 * @{
 *
 * @file       UBX.h
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

#ifndef UBX_H
#define UBX_H
#include "openpilot.h"

#define UBX_SYNC1						0xb5 // UBX protocol synchronization characters
#define UBX_SYNC2						0x62

// From u-blox6 receiver protocol specification

// Messages classes
#define UBX_CLASS_NAV	0x01

// Message IDs
#define	UBX_ID_VELNED	0x12

// private structures

typedef struct  {
	uint32_t	iTOW;	// ms GPS Millisecond Time of Week
	int32_t		velN;	// cm/s NED north velocity
	int32_t		velE;	// cm/s NED east velocity
	int32_t		velD;	// cm/s NED down velocity
	uint32_t	speed;	// cm/s Speed (3-D)
	uint32_t	gSpeed;	// cm/s Ground Speed (2-D)
	int32_t		heading;	// 1e-5 *deg Heading of motion 2-D
	uint32_t	sAcc;	// cm/s Speed Accuracy Estimate
	uint32_t	cAcc;	// 1e-5 *deg Course / Heading Accuracy Estimate
} UBX_NAV_VELNED;

typedef union {		// add more message types later
	uint8_t		payload[0];
	UBX_NAV_VELNED	nav_velned;
} UBXPayload;

typedef struct {
	uint8_t 	class;
	uint8_t 	id;
	uint16_t	len;
	uint8_t 	ck_a;
	uint8_t 	ck_b;
} UBXHeader;

typedef struct {
	UBXHeader	header;
	UBXPayload	payload;
} UBXPacket;

bool checksum_ubx_message(UBXPacket *);
void parse_ubx_message(UBXPacket *);
#endif /* UBX_H */

/**
 ******************************************************************************
 *
 * @file       gpsinfo.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      Include file for the GPS module.
 * 	       As with all modules only the initialize function is exposed all other
 * 	       interactions with the module take place through the event queue and
 *             objects.
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

#ifndef GPSINFO_H
#define GPSINFO_H

// constants/macros/typdefs
typedef union union_float_u32
{
	float f;
	unsigned long i;
	unsigned char b[4];
	unsigned char c[20];
} float_u32;

typedef union union_double_u64
{
	double f;
	unsigned long long i;
	unsigned char b[8];
} double_u64;

struct PositionLLA
{
	float_u32 lat;
	float_u32 lon;
	float_u32 alt;
	float_u32 TimeOfFix;
	uint16_t updates;
};

struct VelocityENU
{
	float_u32 east;
	float_u32 north;
	float_u32 up;
	float_u32 TimeOfFix;
	uint16_t updates;
};

struct VelocityHS
{
	float_u32 heading;
	float_u32 speed;
	float_u32 TimeOfFix;
	uint16_t updates;
};

struct PositionECEF
{
	float_u32 x;
	float_u32 y;
	float_u32 z;
	float_u32 TimeOfFix;
	uint16_t updates;
};

struct VelocityECEF
{
	float_u32 x;
	float_u32 y;
	float_u32 z;
	float_u32 TimeOfFix;
	uint16_t updates;
};

struct UTCtime
{
	uint16_t year;
	uint8_t month;
	uint8_t day;
	uint8_t hour;
	uint8_t min;
	uint8_t sec;
	uint8_t offset;
};

typedef struct struct_GpsInfo
{
	float_u32 TimeOfWeek;
	uint16_t WeekNum;
	float_u32 UtcOffset;
	uint8_t numSVs;


	struct PositionLLA PosLLA;
	struct PositionECEF PosECEF;
	struct VelocityECEF VelECEF;
	struct VelocityENU VelENU;
	struct VelocityHS VelHS;
	struct UTCtime UTC;

} GpsInfoType;

#endif // GPSINFO_H

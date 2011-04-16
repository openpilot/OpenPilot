/**
 ******************************************************************************
 * @addtogroup INS INS

 * @brief The main INS headers
 *
 * @{
 * @addtogroup INS_Main
 * @brief INS headers
 * @{
 *
 * @file       ins.h
 * @author     David "Buzz" Carlson (buzz@chebuzz.com)
 * 				The OpenPilot Team, http://www.openpilot.org Copyright (C) 2011.
 * @brief      INS Headers
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

#ifndef INS_H
#define INS_H

/* PIOS Includes */
#include <pios.h>

struct mag_sensor {
	uint8_t id[4];
	uint8_t updated;
	struct {
		int16_t axis[3];
	} raw;
	struct {
		float axis[3];
	} scaled;
	struct {
		float bias[3];
		float scale[3];
		float variance[3];
	} calibration;
};

//! Contains the data from the accelerometer
struct accel_sensor {
	struct {
		uint16_t x;
		uint16_t y;
		uint16_t z;
	} raw;
	struct {
		float x;
		float y;
		float z;
	} filtered;
	struct {
		float scale[3][4];
		float variance[3];
	} calibration;
};

//! Contains the data from the gyro
struct gyro_sensor {
	struct {
		uint16_t x;
		uint16_t y;
		uint16_t z;
	} raw;
	struct {
		float x;
		float y;
		float z;
	} filtered;
	struct {
		float bias[3];
		float scale[3];
		float variance[3];
		float tempcompfactor[3];
	} calibration;
	struct {
		uint16_t xy;
		uint16_t z;
	} temp;
};

//! Conains the current estimate of the attitude
struct attitude_solution {
	struct {
		float q1;
		float q2;
		float q3;
		float q4;
	} quaternion;
};

//! Contains data from the altitude sensor
struct altitude_sensor {
	float altitude;
	bool updated;
};

//! Contains data from the GPS (via the SPI link)
struct gps_sensor {
	float NED[3];
	float heading;
	float groundspeed;
	float quality;
	bool updated;
};

#endif /* INS_H */

/**
 * @}
 * @}
 */

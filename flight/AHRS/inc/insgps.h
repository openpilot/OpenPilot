/**
 ******************************************************************************
 *
 * @file       insgps.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      Include file of the INSGPS exposed functionality.
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

#ifndef INSGPS_H_
#define INSGPS_H_

//  Exposed Function Prototypes
void INSGPSInit();
void INSPrediction(float gyro_data[3], float accel_data[3], float dT);
void INSSetPosVelVar(float PosVar);
void INSSetGyroBias(float gyro_bias[3]);
void INSSetAccelVar(float accel_var[3]);
void INSSetGyroVar(float gyro_var[3]);
void INSSetMagNorth(float B[3]);
void INSSetMagVar(float scaled_mag_var[3]);
void MagCorrection(float mag_data[3]);
void FullCorrection(float mag_data[3], float Pos[3], float Vel[3], float BaroAlt);
void GndSpeedAndMagCorrection(float Speed, float Heading, float mag_data[3]);

//  Nav structure containing current solution
struct NavStruct {
    float Pos[3];           // Position in meters and relative to a local NED frame
    float Vel[3];           // Velocity in meters and in NED
    float q[4];             // unit quaternion rotation relative to NED
} Nav;

// constants
#define MagSensors 0x1C0
#define FullSensors 0x3FF
#define GndSpeedAndMagSensors 0x1D8

#endif /* EKF_H_ */

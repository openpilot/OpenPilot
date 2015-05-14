/**
 ******************************************************************************
 *
 * @file       CoordinateConverions.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      Header for Coordinate conversions library in CoordinateConversions.c
 *             - all angles in deg
 *             - distances in meters
 *             - altitude above WGS-84 elipsoid
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

#ifndef COORDINATECONVERSIONS_H_
#define COORDINATECONVERSIONS_H_

// ****** convert Lat,Lon,Alt to ECEF  ************
void LLA2ECEF(int32_t LLAi[3], double ECEF[3]);

// ****** convert ECEF to Lat,Lon,Alt (ITERATIVE!) *********
uint16_t ECEF2LLA(double ECEF[3], float LLA[3]);

void RneFromLLA(int32_t LLAi[3], float Rne[3][3]);

// ****** find rotation matrix from rotation vector
void Rv2Rot(float Rv[3], float R[3][3]);

// ****** find roll, pitch, yaw from quaternion ********
void Quaternion2RPY(const float q[4], float rpy[3]);

// ****** find quaternion from roll, pitch, yaw ********
void RPY2Quaternion(const float rpy[3], float q[4]);

// ** Find Rbe, that rotates a vector from earth fixed to body frame, from quaternion **
void Quaternion2R(float q[4], float Rbe[3][3]);

// ** Find first row of Rbe, that rotates a vector from earth fixed to body frame, from quaternion **
// ** This vector corresponds to the fuselage/roll vector xB **
void QuaternionC2xB(const float q0, const float q1, const float q2, const float q3, float x[3]);
void Quaternion2xB(const float q[4], float x[3]);

// ** Find second row of Rbe, that rotates a vector from earth fixed to body frame, from quaternion **
// ** This vector corresponds to the spanwise/pitch vector yB **
void QuaternionC2yB(const float q0, const float q1, const float q2, const float q3, float y[3]);
void Quaternion2yB(const float q[4], float y[3]);

// ** Find third row of Rbe, that rotates a vector from earth fixed to body frame, from quaternion **
// ** This vector corresponds to the vertical/yaw vector zB **
void QuaternionC2zB(const float q0, const float q1, const float q2, const float q3, float z[3]);
void Quaternion2zB(const float q[4], float z[3]);

// ****** Express LLA in a local NED Base Frame ********
void LLA2Base(int32_t LLAi[3], double BaseECEF[3], float Rne[3][3], float NED[3]);

// ****** Express ECEF in a local NED Base Frame ********
void ECEF2Base(double ECEF[3], double BaseECEF[3], float Rne[3][3], float NED[3]);

// ****** convert Rotation Matrix to Quaternion ********
// ****** if R converts from e to b, q is rotation from e to b ****
void R2Quaternion(float R[3][3], float q[4]);

// ****** Rotation Matrix from Two Vector Directions ********
// ****** given two vector directions (v1 and v2) known in two frames (b and e) find Rbe ***
// ****** solution is approximate if can't be exact ***
uint8_t RotFrom2Vectors(const float v1b[3], const float v1e[3], const float v2b[3], const float v2e[3], float Rbe[3][3]);

// ****** Vector Cross Product ********
void CrossProduct(const float v1[3], const float v2[3], float result[3]);

// ****** Vector Magnitude ********
float VectorMagnitude(const float v[3]);

void quat_inverse(float q[4]);
void quat_copy(const float q[4], float qnew[4]);
void quat_mult(const float q1[4], const float q2[4], float qout[4]);
void rot_mult(float R[3][3], const float vec[3], float vec_out[3]);
/**
 * matrix_mult_3x3f - perform a multiplication between two 3x3 float matrices
 * result = a*b
 * @param a
 * @param b
 * @param result
 */
static inline void matrix_mult_3x3f(float a[3][3], float b[3][3], float result[3][3])
{
    result[0][0] = a[0][0] * b[0][0] + a[1][0] * b[0][1] + a[2][0] * b[0][2];
    result[0][1] = a[0][1] * b[0][0] + a[1][1] * b[0][1] + a[2][1] * b[0][2];
    result[0][2] = a[0][2] * b[0][0] + a[1][2] * b[0][1] + a[2][2] * b[0][2];

    result[1][0] = a[0][0] * b[1][0] + a[1][0] * b[1][1] + a[2][0] * b[1][2];
    result[1][1] = a[0][1] * b[1][0] + a[1][1] * b[1][1] + a[2][1] * b[1][2];
    result[1][2] = a[0][2] * b[1][0] + a[1][2] * b[1][1] + a[2][2] * b[1][2];

    result[2][0] = a[0][0] * b[2][0] + a[1][0] * b[2][1] + a[2][0] * b[2][2];
    result[2][1] = a[0][1] * b[2][0] + a[1][1] * b[2][1] + a[2][1] * b[2][2];
    result[2][2] = a[0][2] * b[2][0] + a[1][2] * b[2][1] + a[2][2] * b[2][2];
}

static inline void matrix_inline_scale_3f(float a[3][3], float scale)
{
    a[0][0] *= scale;
    a[0][1] *= scale;
    a[0][2] *= scale;

    a[1][0] *= scale;
    a[1][1] *= scale;
    a[1][2] *= scale;

    a[2][0] *= scale;
    a[2][1] *= scale;
    a[2][2] *= scale;
}

static inline void rot_about_axis_x(const float rotation, float R[3][3])
{
    float s = sinf(rotation);
    float c = cosf(rotation);

    R[0][0] = 1;
    R[0][1] = 0;
    R[0][2] = 0;

    R[1][0] = 0;
    R[1][1] = c;
    R[1][2] = -s;

    R[2][0] = 0;
    R[2][1] = s;
    R[2][2] = c;
}

static inline void rot_about_axis_y(const float rotation, float R[3][3])
{
    float s = sinf(rotation);
    float c = cosf(rotation);

    R[0][0] = c;
    R[0][1] = 0;
    R[0][2] = s;

    R[1][0] = 0;
    R[1][1] = 1;
    R[1][2] = 0;

    R[2][0] = -s;
    R[2][1] = 0;
    R[2][2] = c;
}

static inline void rot_about_axis_z(const float rotation, float R[3][3])
{
    float s = sinf(rotation);
    float c = cosf(rotation);

    R[0][0] = c;
    R[0][1] = -s;
    R[0][2] = 0;

    R[1][0] = s;
    R[1][1] = c;
    R[1][2] = 0;

    R[2][0] = 0;
    R[2][1] = 0;
    R[2][2] = 1;
}

#endif // COORDINATECONVERSIONS_H_

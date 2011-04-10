/**
 ******************************************************************************
 *
 * @file       CoordinateConversions.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      General conversions with different coordinate systems.
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

#include <math.h>
#include <stdint.h>
#include "CoordinateConversions.h"

#define RAD2DEG (180.0/M_PI)
#define DEG2RAD (M_PI/180.0)

// ****** convert Lat,Lon,Alt to ECEF  ************
void LLA2ECEF(double LLA[3], double ECEF[3])
{
	const double a = 6378137.0;	// Equatorial Radius
	const double e = 8.1819190842622e-2;	// Eccentricity
	double sinLat, sinLon, cosLat, cosLon;
	double N;

	sinLat = sin(DEG2RAD * LLA[0]);
	sinLon = sin(DEG2RAD * LLA[1]);
	cosLat = cos(DEG2RAD * LLA[0]);
	cosLon = cos(DEG2RAD * LLA[1]);

	N = a / sqrt(1.0 - e * e * sinLat * sinLat);	//prime vertical radius of curvature

	ECEF[0] = (N + LLA[2]) * cosLat * cosLon;
	ECEF[1] = (N + LLA[2]) * cosLat * sinLon;
	ECEF[2] = ((1 - e * e) * N + LLA[2]) * sinLat;
}

// ****** convert ECEF to Lat,Lon,Alt (ITERATIVE!) *********
uint16_t ECEF2LLA(double ECEF[3], double LLA[3])
{
	/**
	 * LLA parameter is used to prime the iteration.
	 * A position within 1 meter of the specified LLA
	 * will be calculated within at most 3 iterations.
	 * If unknown: Call with any valid LLA coordinate
	 * will compute within at most 5 iterations.
	 * Suggestion: [0,0,0]
	 **/

	const double a = 6378137.0;	// Equatorial Radius
	const double e = 8.1819190842622e-2;	// Eccentricity
	double x = ECEF[0], y = ECEF[1], z = ECEF[2];
	double Lat, N, NplusH, delta, esLat;
	uint16_t iter;
#define MAX_ITER 10		// should not take more than 5 for valid coordinates
#define ACCURACY 1.0e-11	// used to be e-14, but we don't need sub micrometer exact calculations

	LLA[1] = RAD2DEG * atan2(y, x);
	Lat = DEG2RAD * LLA[0];
	esLat = e * sin(Lat);
	N = a / sqrt(1 - esLat * esLat);
	NplusH = N + LLA[2];
	delta = 1;
	iter = 0;

	while (((delta > ACCURACY) || (delta < -ACCURACY))
	       && (iter < MAX_ITER)) {
		delta = Lat - atan(z / (sqrt(x * x + y * y) * (1 - (N * e * e / NplusH))));
		Lat = Lat - delta;
		esLat = e * sin(Lat);
		N = a / sqrt(1 - esLat * esLat);
		NplusH = sqrt(x * x + y * y) / cos(Lat);
		iter += 1;
	}

	LLA[0] = RAD2DEG * Lat;
	LLA[2] = NplusH - N;

	return (iter < MAX_ITER);
}

// ****** find ECEF to NED rotation matrix ********
void RneFromLLA(double LLA[3], float Rne[3][3])
{
	float sinLat, sinLon, cosLat, cosLon;

	sinLat = (float)sin(DEG2RAD * LLA[0]);
	sinLon = (float)sin(DEG2RAD * LLA[1]);
	cosLat = (float)cos(DEG2RAD * LLA[0]);
	cosLon = (float)cos(DEG2RAD * LLA[1]);

	Rne[0][0] = -sinLat * cosLon;
	Rne[0][1] = -sinLat * sinLon;
	Rne[0][2] = cosLat;
	Rne[1][0] = -sinLon;
	Rne[1][1] = cosLon;
	Rne[1][2] = 0;
	Rne[2][0] = -cosLat * cosLon;
	Rne[2][1] = -cosLat * sinLon;
	Rne[2][2] = -sinLat;
}

// ****** find roll, pitch, yaw from quaternion ********
void Quaternion2RPY(const float q[4], float rpy[3])
{
	float R13, R11, R12, R23, R33;
	float q0s = q[0] * q[0];
	float q1s = q[1] * q[1];
	float q2s = q[2] * q[2];
	float q3s = q[3] * q[3];

	R13 = 2 * (q[1] * q[3] - q[0] * q[2]);
	R11 = q0s + q1s - q2s - q3s;
	R12 = 2 * (q[1] * q[2] + q[0] * q[3]);
	R23 = 2 * (q[2] * q[3] + q[0] * q[1]);
	R33 = q0s - q1s - q2s + q3s;

	rpy[1] = RAD2DEG * asinf(-R13);	// pitch always between -pi/2 to pi/2
	rpy[2] = RAD2DEG * atan2f(R12, R11);
	rpy[0] = RAD2DEG * atan2f(R23, R33);

	//TODO: consider the cases where |R13| ~= 1, |pitch| ~= pi/2
}

// ****** find quaternion from roll, pitch, yaw ********
void RPY2Quaternion(const float rpy[3], float q[4])
{
	float phi, theta, psi;
	float cphi, sphi, ctheta, stheta, cpsi, spsi;

	phi = DEG2RAD * rpy[0] / 2;
	theta = DEG2RAD * rpy[1] / 2;
	psi = DEG2RAD * rpy[2] / 2;
	cphi = cosf(phi);
	sphi = sinf(phi);
	ctheta = cosf(theta);
	stheta = sinf(theta);
	cpsi = cosf(psi);
	spsi = sinf(psi);

	q[0] = cphi * ctheta * cpsi + sphi * stheta * spsi;
	q[1] = sphi * ctheta * cpsi - cphi * stheta * spsi;
	q[2] = cphi * stheta * cpsi + sphi * ctheta * spsi;
	q[3] = cphi * ctheta * spsi - sphi * stheta * cpsi;

	if (q[0] < 0) {		// q0 always positive for uniqueness
		q[0] = -q[0];
		q[1] = -q[1];
		q[2] = -q[2];
		q[3] = -q[3];
	}
}

//** Find Rbe, that rotates a vector from earth fixed to body frame, from quaternion **
void Quaternion2R(float q[4], float Rbe[3][3])
{

	float q0s = q[0] * q[0], q1s = q[1] * q[1], q2s = q[2] * q[2], q3s = q[3] * q[3];

	Rbe[0][0] = q0s + q1s - q2s - q3s;
	Rbe[0][1] = 2 * (q[1] * q[2] + q[0] * q[3]);
	Rbe[0][2] = 2 * (q[1] * q[3] - q[0] * q[2]);
	Rbe[1][0] = 2 * (q[1] * q[2] - q[0] * q[3]);
	Rbe[1][1] = q0s - q1s + q2s - q3s;
	Rbe[1][2] = 2 * (q[2] * q[3] + q[0] * q[1]);
	Rbe[2][0] = 2 * (q[1] * q[3] + q[0] * q[2]);
	Rbe[2][1] = 2 * (q[2] * q[3] - q[0] * q[1]);
	Rbe[2][2] = q0s - q1s - q2s + q3s;
}

// ****** Express LLA in a local NED Base Frame ********
void LLA2Base(double LLA[3], double BaseECEF[3], float Rne[3][3], float NED[3])
{
	double ECEF[3];
	float diff[3];

	LLA2ECEF(LLA, ECEF);

	diff[0] = (float)(ECEF[0] - BaseECEF[0]);
	diff[1] = (float)(ECEF[1] - BaseECEF[1]);
	diff[2] = (float)(ECEF[2] - BaseECEF[2]);

	NED[0] = Rne[0][0] * diff[0] + Rne[0][1] * diff[1] + Rne[0][2] * diff[2];
	NED[1] = Rne[1][0] * diff[0] + Rne[1][1] * diff[1] + Rne[1][2] * diff[2];
	NED[2] = Rne[2][0] * diff[0] + Rne[2][1] * diff[1] + Rne[2][2] * diff[2];
}

// ****** Express ECEF in a local NED Base Frame ********
void ECEF2Base(double ECEF[3], double BaseECEF[3], float Rne[3][3], float NED[3])
{
	float diff[3];

	diff[0] = (float)(ECEF[0] - BaseECEF[0]);
	diff[1] = (float)(ECEF[1] - BaseECEF[1]);
	diff[2] = (float)(ECEF[2] - BaseECEF[2]);

	NED[0] = Rne[0][0] * diff[0] + Rne[0][1] * diff[1] + Rne[0][2] * diff[2];
	NED[1] = Rne[1][0] * diff[0] + Rne[1][1] * diff[1] + Rne[1][2] * diff[2];
	NED[2] = Rne[2][0] * diff[0] + Rne[2][1] * diff[1] + Rne[2][2] * diff[2];
}

// ****** convert Rotation Matrix to Quaternion ********
// ****** if R converts from e to b, q is rotation from e to b ****
void R2Quaternion(float R[3][3], float q[4])
{
	float m[4], mag;
	uint8_t index,i;

	m[0] = 1 + R[0][0] + R[1][1] + R[2][2];
	m[1] = 1 + R[0][0] - R[1][1] - R[2][2];
	m[2] = 1 - R[0][0] + R[1][1] - R[2][2];
	m[3] = 1 - R[0][0] - R[1][1] + R[2][2];

	// find maximum divisor
	index = 0;
	mag = m[0];
	for (i=1;i<4;i++){
		if (m[i] > mag){
			mag = m[i];
			index = i;
		}
	}
	mag = 2*sqrt(mag);

	if (index == 0) {
		q[0] = mag/4;
		q[1] = (R[1][2]-R[2][1])/mag;
		q[2] = (R[2][0]-R[0][2])/mag;
		q[3] = (R[0][1]-R[1][0])/mag;
	}
	else if (index == 1) {
		q[1] = mag/4;
		q[0] = (R[1][2]-R[2][1])/mag;
		q[2] = (R[0][1]+R[1][0])/mag;
		q[3] = (R[0][2]+R[2][0])/mag;
	}
	else if (index == 2) {
		q[2] = mag/4;
		q[0] = (R[2][0]-R[0][2])/mag;
		q[1] = (R[0][1]+R[1][0])/mag;
		q[3] = (R[1][2]+R[2][1])/mag;
	}
	else {
		q[3] = mag/4;
		q[0] = (R[0][1]-R[1][0])/mag;
		q[1] = (R[0][2]+R[2][0])/mag;
		q[2] = (R[1][2]+R[2][1])/mag;
	}

	// q0 positive, i.e. angle between pi and -pi
	if (q[0] < 0){
		q[0] = -q[0];
		q[1] = -q[1];
		q[2] = -q[2];
		q[3] = -q[3];
	}
}

// ****** Rotation Matrix from Two Vector Directions ********
// ****** given two vector directions (v1 and v2) known in two frames (b and e) find Rbe ***
// ****** solution is approximate if can't be exact ***
uint8_t RotFrom2Vectors(const float v1b[3], const float v1e[3], const float v2b[3], const float v2e[3], float Rbe[3][3])
{
	float Rib[3][3], Rie[3][3];
	float mag;
	uint8_t i,j,k;

	// identity rotation in case of error
	for (i=0;i<3;i++){
		for (j=0;j<3;j++)
			Rbe[i][j]=0;
		Rbe[i][i]=1;
	}

	// The first rows of rot matrices chosen in direction of v1
	mag = VectorMagnitude(v1b);
	if (fabs(mag) < 1e-30)
		return (-1);
	for (i=0;i<3;i++)
		Rib[0][i]=v1b[i]/mag;

	mag = VectorMagnitude(v1e);
	if (fabs(mag) < 1e-30)
		return (-1);
	for (i=0;i<3;i++)
		Rie[0][i]=v1e[i]/mag;

	// The second rows of rot matrices chosen in direction of v1xv2
	CrossProduct(v1b,v2b,&Rib[1][0]);
	mag = VectorMagnitude(&Rib[1][0]);
	if (fabs(mag) < 1e-30)
		return (-1);
	for (i=0;i<3;i++)
		Rib[1][i]=Rib[1][i]/mag;

	CrossProduct(v1e,v2e,&Rie[1][0]);
	mag = VectorMagnitude(&Rie[1][0]);
	if (fabs(mag) < 1e-30)
		return (-1);
	for (i=0;i<3;i++)
		Rie[1][i]=Rie[1][i]/mag;

	// The third rows of rot matrices are XxY (Row1xRow2)
	CrossProduct(&Rib[0][0],&Rib[1][0],&Rib[2][0]);
	CrossProduct(&Rie[0][0],&Rie[1][0],&Rie[2][0]);

	// Rbe = Rbi*Rie = Rib'*Rie
	for (i=0;i<3;i++)
		for(j=0;j<3;j++){
			Rbe[i][j]=0;
			for(k=0;k<3;k++)
				Rbe[i][j] += Rib[k][i]*Rie[k][j];
		}

	return 1;
}

void Rv2Rot(float Rv[3], float R[3][3])
{
	// Compute rotation matrix from a rotation vector
	// To save .text space, uses Quaternion2R()
	float q[4];

	float angle = VectorMagnitude(Rv);
	if (angle <= 0.00048828125f) {
		// angle < sqrt(2*machine_epsilon(float)), so flush cos(x) to 1.0f
		q[0] = 1.0f;

        // and flush sin(x/2)/x to 0.5
		q[1] = 0.5f*Rv[0];
		q[2] = 0.5f*Rv[1];
		q[3] = 0.5f*Rv[2];
		// This prevents division by zero, while retaining full accuracy
	}
	else {
		q[0] = cosf(angle*0.5f);
		float scale = sinf(angle*0.5f) / angle;
		q[1] = scale*Rv[0];
		q[2] = scale*Rv[1];
		q[3] = scale*Rv[2];
	}

	Quaternion2R(q, R);
}

// ****** Vector Cross Product ********
void CrossProduct(const float v1[3], const float v2[3], float result[3])
{
	result[0] = v1[1]*v2[2] - v2[1]*v1[2];
	result[1] = v2[0]*v1[2] - v1[0]*v2[2];
	result[2] = v1[0]*v2[1] - v2[0]*v1[1];
}

// ****** Vector Magnitude ********
float VectorMagnitude(const float v[3])
{
	return(sqrt(v[0]*v[0] + v[1]*v[1] + v[2]*v[2]));
}

void quat_inverse(float q[4]) 
{
	q[1] = -q[1];
	q[2] = -q[2];
	q[3] = -q[3];
}

void quat_copy(const float q[4], float qnew[4]) 
{
	qnew[0] = q[0];
	qnew[1] = q[1];
	qnew[2] = q[2];
	qnew[3] = q[3];
}

void quat_mult(const float q1[4], const float q2[4], float qout[4]) 
{
	qout[0] = q1[0]*q2[0] - q1[1]*q2[1] - q1[2]*q2[2] - q1[3]*q2[3];
	qout[1] = q1[0]*q2[1] + q1[1]*q2[0] + q1[2]*q2[3] - q1[3]*q2[2];
	qout[2] = q1[0]*q2[2] - q1[1]*q2[3] + q1[2]*q2[0] + q1[3]*q2[1];
	qout[3] = q1[0]*q2[3] + q1[1]*q2[2] - q1[2]*q2[1] + q1[3]*q2[0];
}

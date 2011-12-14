/**
 ******************************************************************************
 * @addtogroup AHRS 
 * @{
 * @addtogroup INSGPS
 * @{
 * @brief INSGPS is a joint attitude and position estimation EKF
 *
 * @file       insgps.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      An INS/GPS algorithm implemented with an EKF.
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

#include "insgps.h"
#include <math.h>
#include <stdint.h>

// constants/macros/typdefs
#define NUMX 13			// number of states, X is the state vector
#define NUMW 9			// number of plant noise inputs, w is disturbance noise vector
#define NUMV 10			// number of measurements, v is the measurement noise vector
#define NUMU 6			// number of deterministic inputs, U is the input vector

#if defined(GENERAL_COV)
// This might trick people so I have a note here.  There is a slower but bigger version of the 
// code here but won't fit when debugging disabled (requires -Os)
#define COVARIANCE_PREDICTION_GENERAL
#endif

// Private functions
void CovariancePrediction(float F[NUMX][NUMX], float G[NUMX][NUMW],
			  float Q[NUMW], float dT, float P[NUMX][NUMX]);
void SerialUpdate(float H[NUMV][NUMX], float R[NUMV], float Z[NUMV],
		  float Y[NUMV], float P[NUMX][NUMX], float X[NUMX],
		  uint16_t SensorsUsed);
void RungeKutta(float X[NUMX], float U[NUMU], float dT);
void StateEq(float X[NUMX], float U[NUMU], float Xdot[NUMX]);
void LinearizeFG(float X[NUMX], float U[NUMU], float F[NUMX][NUMX],
		 float G[NUMX][NUMW]);
void MeasurementEq(float X[NUMX], float Be[3], float Y[NUMV]);
void LinearizeH(float X[NUMX], float Be[3], float H[NUMV][NUMX]);

// Private variables
float F[NUMX][NUMX], G[NUMX][NUMW], H[NUMV][NUMX];	// linearized system matrices
													// global to init to zero and maintain zero elements
float Be[3];			// local magnetic unit vector in NED frame
float P[NUMX][NUMX], X[NUMX];	// covariance matrix and state vector
float Q[NUMW], R[NUMV];		// input noise and measurement noise variances
float K[NUMX][NUMV];		// feedback gain matrix

//  *************  Exposed Functions ****************
//  *************************************************

uint16_t ins_get_num_states() 
{
	return NUMX;
}

void INSGPSInit()		//pretty much just a place holder for now
{
	Be[0] = 1.0f;
	Be[1] = 0.0f;
	Be[2] = 0.0f;		// local magnetic unit vector

	for (int i = 0; i < NUMX; i++) {
		for (int j = 0; j < NUMX; j++) {
			P[i][j] = 0.0f; // zero all terms
			F[i][j] = 0.0f;
		}
		
		for (int j = 0; j < NUMW; j++)
			G[i][j] = 0.0f;
			
		for (int j = 0; j < NUMV; j++) {
			H[j][i] = 0.0f;
			K[i][j] = 0.0f;
		}
			
		X[i] = 0.0f;
	}
	for (int i = 0; i < NUMW; i++)
		Q[i] = 0.0f;
	for (int i = 0; i < NUMV; i++) 
		R[i] = 0.0f;

	
	P[0][0] = P[1][1] = P[2][2] = 25.0f;            // initial position variance (m^2)
	P[3][3] = P[4][4] = P[5][5] = 5.0f;             // initial velocity variance (m/s)^2
	P[6][6] = P[7][7] = P[8][8] = P[9][9] = 1e-5f;  // initial quaternion variance
	P[10][10] = P[11][11] = P[12][12] = 1e-9f;      // initial gyro bias variance (rad/s)^2

	X[0] = X[1] = X[2] = X[3] = X[4] = X[5] = 0.0f;	// initial pos and vel (m)
	X[6] = 1.0f;
	X[7] = X[8] = X[9] = 0.0f;	    // initial quaternion (level and North) (m/s)
	X[10] = X[11] = X[12] = 0.0f;	// initial gyro bias (rad/s)

	Q[0] = Q[1] = Q[2] = 50e-4f;	// gyro noise variance (rad/s)^2
	Q[3] = Q[4] = Q[5] = 0.00001f;	// accelerometer noise variance (m/s^2)^2
	Q[6] = Q[7] = Q[8] = 2e-8f;	    // gyro bias random walk variance (rad/s^2)^2

	R[0] = R[1] = 0.004f;	// High freq GPS horizontal position noise variance (m^2)
	R[2] = 0.036f;          // High freq GPS vertical position noise variance (m^2)
	R[3] = R[4] = 0.004f;   // High freq GPS horizontal velocity noise variance (m/s)^2
	R[5] = 100.0f;          // High freq GPS vertical velocity noise variance (m/s)^2
	R[6] = R[7] = R[8] = 0.005f;    // magnetometer unit vector noise variance
	R[9] = .05f;                    // High freq altimeter noise variance (m^2)
}

void INSResetP(float PDiag[NUMX])
{
	uint8_t i,j;

	// if PDiag[i] nonzero then clear row and column and set diagonal element
	for (i=0;i<NUMX;i++){
		if (PDiag != 0){
			for (j=0;j<NUMX;j++)
				P[i][j]=P[j][i]=0.0f;
			P[i][i]=PDiag[i];
		}
	}
}

void INSSetState(float pos[3], float vel[3], float q[4], float gyro_bias[3], float accel_bias[3])
{
	/* Note: accel_bias not used in 13 state INS */
	X[0] = pos[0];
	X[1] = pos[1];
	X[2] = pos[2];
	X[3] = vel[0];
	X[4] = vel[1];
	X[5] = vel[2];
	X[6] = q[0];
	X[7] = q[1];
	X[8] = q[2];
	X[9] = q[3];
	X[10] = gyro_bias[0];
	X[11] = gyro_bias[1];
	X[12] = gyro_bias[2];
}

void INSPosVelReset(float pos[3], float vel[3]) 
{
	for (int i = 0; i < 6; i++) {
		for(int j = i; j < NUMX; j++) {
			P[i][j] = 0;  // zero the first 6 rows and columns
			P[j][i] = 0; 
		}
	}
	
	P[0][0] = P[1][1] = P[2][2] = 25;	// initial position variance (m^2)
	P[3][3] = P[4][4] = P[5][5] = 5;	// initial velocity variance (m/s)^2
	
	X[0] = pos[0];
	X[1] = pos[1];
	X[2] = pos[2];
	X[3] = vel[0];
	X[4] = vel[1];
	X[5] = vel[2];	
}

void INSSetPosVelVar(float PosVar, float VelVar)
{
	R[0] = PosVar;
	R[1] = PosVar;
	R[2] = PosVar;
	R[3] = VelVar;
	R[4] = VelVar;
//    R[5] = PosVar;  // Don't change vertical velocity, not measured
}

void INSSetGyroBias(float gyro_bias[3])
{
	X[10] = gyro_bias[0];
	X[11] = gyro_bias[1];
	X[12] = gyro_bias[2];
}

void INSSetAccelVar(float accel_var[3])
{
	Q[3] = accel_var[0];
	Q[4] = accel_var[1];
	Q[5] = accel_var[2];
}

void INSSetGyroVar(float gyro_var[3])
{
	Q[0] = gyro_var[0];
	Q[1] = gyro_var[1];
	Q[2] = gyro_var[2];
}

void INSSetMagVar(float scaled_mag_var[3])
{
	R[6] = scaled_mag_var[0];
	R[7] = scaled_mag_var[1];
	R[8] = scaled_mag_var[2];
}

void INSSetMagNorth(float B[3])
{
	Be[0] = B[0];
	Be[1] = B[1];
	Be[2] = B[2];
}

void INSStatePrediction(float gyro_data[3], float accel_data[3], float dT)
{
	float U[6];
	float qmag;

	// rate gyro inputs in units of rad/s
	U[0] = gyro_data[0];
	U[1] = gyro_data[1];
	U[2] = gyro_data[2];

	// accelerometer inputs in units of m/s
	U[3] = accel_data[0];
	U[4] = accel_data[1];
	U[5] = accel_data[2];

	// EKF prediction step
	LinearizeFG(X, U, F, G);
	RungeKutta(X, U, dT);
	qmag = sqrtf(X[6] * X[6] + X[7] * X[7] + X[8] * X[8] + X[9] * X[9]);
	X[6] /= qmag;
	X[7] /= qmag;
	X[8] /= qmag;
	X[9] /= qmag;
	//CovariancePrediction(F,G,Q,dT,P);

	// Update Nav solution structure
	Nav.Pos[0] = X[0];
	Nav.Pos[1] = X[1];
	Nav.Pos[2] = X[2];
	Nav.Vel[0] = X[3];
	Nav.Vel[1] = X[4];
	Nav.Vel[2] = X[5];
	Nav.q[0] = X[6];
	Nav.q[1] = X[7];
	Nav.q[2] = X[8];
	Nav.q[3] = X[9];
	Nav.gyro_bias[0] = X[10];
	Nav.gyro_bias[1] = X[11];
	Nav.gyro_bias[2] = X[12];	
}

void INSCovariancePrediction(float dT)
{
	CovariancePrediction(F, G, Q, dT, P);
}

float zeros[3] = { 0, 0, 0 };

void MagCorrection(float mag_data[3])
{
	INSCorrection(mag_data, zeros, zeros, zeros[0], MAG_SENSORS);
}

void MagVelBaroCorrection(float mag_data[3], float Vel[3], float BaroAlt)
{
	INSCorrection(mag_data, zeros, Vel, BaroAlt,
		      MAG_SENSORS | HORIZ_SENSORS | VERT_SENSORS |
		      BARO_SENSOR);
}

void GpsBaroCorrection(float Pos[3], float Vel[3], float BaroAlt)
{
	INSCorrection(zeros, Pos, Vel, BaroAlt,
		      HORIZ_SENSORS | VERT_SENSORS | BARO_SENSOR);
}

void FullCorrection(float mag_data[3], float Pos[3], float Vel[3],
		    float BaroAlt)
{
	INSCorrection(mag_data, Pos, Vel, BaroAlt, FULL_SENSORS);
}

void GpsMagCorrection(float mag_data[3], float Pos[3], float Vel[3])
{
	INSCorrection(mag_data, Pos, Vel, zeros[0],
		      POS_SENSORS | HORIZ_SENSORS | MAG_SENSORS);
}

void VelBaroCorrection(float Vel[3], float BaroAlt)
{
	INSCorrection(zeros, zeros, Vel, BaroAlt,
		      HORIZ_SENSORS | VERT_SENSORS | BARO_SENSOR);
}

void INSCorrection(float mag_data[3], float Pos[3], float Vel[3],
		   float BaroAlt, uint16_t SensorsUsed)
{
	float Z[10], Y[10];
	float Bmag, qmag;

	// GPS Position in meters and in local NED frame
	Z[0] = Pos[0];
	Z[1] = Pos[1];
	Z[2] = Pos[2];

	// GPS Velocity in meters and in local NED frame
	Z[3] = Vel[0];
	Z[4] = Vel[1];
	Z[5] = Vel[2];

	// magnetometer data in any units (use unit vector) and in body frame
	Bmag =
	    sqrtf(mag_data[0] * mag_data[0] + mag_data[1] * mag_data[1] +
		 mag_data[2] * mag_data[2]);
	Z[6] = mag_data[0] / Bmag;
	Z[7] = mag_data[1] / Bmag;
	Z[8] = mag_data[2] / Bmag;

	// barometric altimeter in meters and in local NED frame
	Z[9] = BaroAlt;

	// EKF correction step
	LinearizeH(X, Be, H);
	MeasurementEq(X, Be, Y);
	SerialUpdate(H, R, Z, Y, P, X, SensorsUsed);
	qmag = sqrtf(X[6] * X[6] + X[7] * X[7] + X[8] * X[8] + X[9] * X[9]);
	X[6] /= qmag;
	X[7] /= qmag;
	X[8] /= qmag;
	X[9] /= qmag;

	// Update Nav solution structure
	Nav.Pos[0] = X[0];
	Nav.Pos[1] = X[1];
	Nav.Pos[2] = X[2];
	Nav.Vel[0] = X[3];
	Nav.Vel[1] = X[4];
	Nav.Vel[2] = X[5];
	Nav.q[0] = X[6];
	Nav.q[1] = X[7];
	Nav.q[2] = X[8];
	Nav.q[3] = X[9];
}

//  *************  CovariancePrediction *************
//  Does the prediction step of the Kalman filter for the covariance matrix
//  Output, Pnew, overwrites P, the input covariance
//  Pnew = (I+F*T)*P*(I+F*T)' + T^2*G*Q*G'
//  Q is the discrete time covariance of process noise
//  Q is vector of the diagonal for a square matrix with
//    dimensions equal to the number of disturbance noise variables
//  The General Method is very inefficient,not taking advantage of the sparse F and G
//  The first Method is very specific to this implementation
//  ************************************************

#ifdef COVARIANCE_PREDICTION_GENERAL

void CovariancePrediction(float F[NUMX][NUMX], float G[NUMX][NUMW],
			  float Q[NUMW], float dT, float P[NUMX][NUMX])
{
	float Dummy[NUMX][NUMX], dTsq;
	uint8_t i, j, k;

	//  Pnew = (I+F*T)*P*(I+F*T)' + T^2*G*Q*G' = T^2[(P/T + F*P)*(I/T + F') + G*Q*G')]

	dTsq = dT * dT;

	for (i = 0; i < NUMX; i++)	// Calculate Dummy = (P/T +F*P)
		for (j = 0; j < NUMX; j++) {
			Dummy[i][j] = P[i][j] / dT;
			for (k = 0; k < NUMX; k++)
				Dummy[i][j] += F[i][k] * P[k][j];
		}
	for (i = 0; i < NUMX; i++)	// Calculate Pnew = Dummy/T + Dummy*F' + G*Qw*G'
		for (j = i; j < NUMX; j++) {	// Use symmetry, ie only find upper triangular
			P[i][j] = Dummy[i][j] / dT;
			for (k = 0; k < NUMX; k++)
				P[i][j] += Dummy[i][k] * F[j][k];	// P = Dummy/T + Dummy*F'
			for (k = 0; k < NUMW; k++)
				P[i][j] += Q[k] * G[i][k] * G[j][k];	// P = Dummy/T + Dummy*F' + G*Q*G'
			P[j][i] = P[i][j] = P[i][j] * dTsq;	// Pnew = T^2*P and fill in lower triangular;
		}
}

#else

void CovariancePrediction(float F[NUMX][NUMX], float G[NUMX][NUMW],
			  float Q[NUMW], float dT, float P[NUMX][NUMX])
{
	float D[NUMX][NUMX], T, Tsq;
	uint8_t i, j;

	//  Pnew = (I+F*T)*P*(I+F*T)' + T^2*G*Q*G' = scalar expansion from symbolic manipulator

	T = dT;
	Tsq = dT * dT;

	for (i = 0; i < NUMX; i++)	// Create a copy of the upper triangular of P
		for (j = i; j < NUMX; j++)
			D[i][j] = P[i][j];

	// Brute force calculation of the elements of P
	P[0][0] = D[3][3] * Tsq + (2 * D[0][3]) * T + D[0][0];
	P[0][1] = P[1][0] =
	    D[3][4] * Tsq + (D[0][4] + D[1][3]) * T + D[0][1];
	P[0][2] = P[2][0] =
	    D[3][5] * Tsq + (D[0][5] + D[2][3]) * T + D[0][2];
	P[0][3] = P[3][0] =
	    (F[3][6] * D[3][6] + F[3][7] * D[3][7] + F[3][8] * D[3][8] +
	     F[3][9] * D[3][9]) * Tsq + (D[3][3] + F[3][6] * D[0][6] +
					 F[3][7] * D[0][7] +
					 F[3][8] * D[0][8] +
					 F[3][9] * D[0][9]) * T + D[0][3];
	P[0][4] = P[4][0] =
	    (F[4][6] * D[3][6] + F[4][7] * D[3][7] + F[4][8] * D[3][8] +
	     F[4][9] * D[3][9]) * Tsq + (D[3][4] + F[4][6] * D[0][6] +
					 F[4][7] * D[0][7] +
					 F[4][8] * D[0][8] +
					 F[4][9] * D[0][9]) * T + D[0][4];
	P[0][5] = P[5][0] =
	    (F[5][6] * D[3][6] + F[5][7] * D[3][7] + F[5][8] * D[3][8] +
	     F[5][9] * D[3][9]) * Tsq + (D[3][5] + F[5][6] * D[0][6] +
					 F[5][7] * D[0][7] +
					 F[5][8] * D[0][8] +
					 F[5][9] * D[0][9]) * T + D[0][5];
	P[0][6] = P[6][0] =
	    (F[6][7] * D[3][7] + F[6][8] * D[3][8] + F[6][9] * D[3][9] +
	     F[6][10] * D[3][10] + F[6][11] * D[3][11] +
	     F[6][12] * D[3][12]) * Tsq + (D[3][6] + F[6][7] * D[0][7] +
					   F[6][8] * D[0][8] +
					   F[6][9] * D[0][9] +
					   F[6][10] * D[0][10] +
					   F[6][11] * D[0][11] +
					   F[6][12] * D[0][12]) * T +
	    D[0][6];
	P[0][7] = P[7][0] =
	    (F[7][6] * D[3][6] + F[7][8] * D[3][8] + F[7][9] * D[3][9] +
	     F[7][10] * D[3][10] + F[7][11] * D[3][11] +
	     F[7][12] * D[3][12]) * Tsq + (D[3][7] + F[7][6] * D[0][6] +
					   F[7][8] * D[0][8] +
					   F[7][9] * D[0][9] +
					   F[7][10] * D[0][10] +
					   F[7][11] * D[0][11] +
					   F[7][12] * D[0][12]) * T +
	    D[0][7];
	P[0][8] = P[8][0] =
	    (F[8][6] * D[3][6] + F[8][7] * D[3][7] + F[8][9] * D[3][9] +
	     F[8][10] * D[3][10] + F[8][11] * D[3][11] +
	     F[8][12] * D[3][12]) * Tsq + (D[3][8] + F[8][6] * D[0][6] +
					   F[8][7] * D[0][7] +
					   F[8][9] * D[0][9] +
					   F[8][10] * D[0][10] +
					   F[8][11] * D[0][11] +
					   F[8][12] * D[0][12]) * T +
	    D[0][8];
	P[0][9] = P[9][0] =
	    (F[9][6] * D[3][6] + F[9][7] * D[3][7] + F[9][8] * D[3][8] +
	     F[9][10] * D[3][10] + F[9][11] * D[3][11] +
	     F[9][12] * D[3][12]) * Tsq + (D[3][9] + F[9][6] * D[0][6] +
					   F[9][7] * D[0][7] +
					   F[9][8] * D[0][8] +
					   F[9][10] * D[0][10] +
					   F[9][11] * D[0][11] +
					   F[9][12] * D[0][12]) * T +
	    D[0][9];
	P[0][10] = P[10][0] = D[3][10] * T + D[0][10];
	P[0][11] = P[11][0] = D[3][11] * T + D[0][11];
	P[0][12] = P[12][0] = D[3][12] * T + D[0][12];
	P[1][1] = D[4][4] * Tsq + (2 * D[1][4]) * T + D[1][1];
	P[1][2] = P[2][1] =
	    D[4][5] * Tsq + (D[1][5] + D[2][4]) * T + D[1][2];
	P[1][3] = P[3][1] =
	    (F[3][6] * D[4][6] + F[3][7] * D[4][7] + F[3][8] * D[4][8] +
	     F[3][9] * D[4][9]) * Tsq + (D[3][4] + F[3][6] * D[1][6] +
					 F[3][7] * D[1][7] +
					 F[3][8] * D[1][8] +
					 F[3][9] * D[1][9]) * T + D[1][3];
	P[1][4] = P[4][1] =
	    (F[4][6] * D[4][6] + F[4][7] * D[4][7] + F[4][8] * D[4][8] +
	     F[4][9] * D[4][9]) * Tsq + (D[4][4] + F[4][6] * D[1][6] +
					 F[4][7] * D[1][7] +
					 F[4][8] * D[1][8] +
					 F[4][9] * D[1][9]) * T + D[1][4];
	P[1][5] = P[5][1] =
	    (F[5][6] * D[4][6] + F[5][7] * D[4][7] + F[5][8] * D[4][8] +
	     F[5][9] * D[4][9]) * Tsq + (D[4][5] + F[5][6] * D[1][6] +
					 F[5][7] * D[1][7] +
					 F[5][8] * D[1][8] +
					 F[5][9] * D[1][9]) * T + D[1][5];
	P[1][6] = P[6][1] =
	    (F[6][7] * D[4][7] + F[6][8] * D[4][8] + F[6][9] * D[4][9] +
	     F[6][10] * D[4][10] + F[6][11] * D[4][11] +
	     F[6][12] * D[4][12]) * Tsq + (D[4][6] + F[6][7] * D[1][7] +
					   F[6][8] * D[1][8] +
					   F[6][9] * D[1][9] +
					   F[6][10] * D[1][10] +
					   F[6][11] * D[1][11] +
					   F[6][12] * D[1][12]) * T +
	    D[1][6];
	P[1][7] = P[7][1] =
	    (F[7][6] * D[4][6] + F[7][8] * D[4][8] + F[7][9] * D[4][9] +
	     F[7][10] * D[4][10] + F[7][11] * D[4][11] +
	     F[7][12] * D[4][12]) * Tsq + (D[4][7] + F[7][6] * D[1][6] +
					   F[7][8] * D[1][8] +
					   F[7][9] * D[1][9] +
					   F[7][10] * D[1][10] +
					   F[7][11] * D[1][11] +
					   F[7][12] * D[1][12]) * T +
	    D[1][7];
	P[1][8] = P[8][1] =
	    (F[8][6] * D[4][6] + F[8][7] * D[4][7] + F[8][9] * D[4][9] +
	     F[8][10] * D[4][10] + F[8][11] * D[4][11] +
	     F[8][12] * D[4][12]) * Tsq + (D[4][8] + F[8][6] * D[1][6] +
					   F[8][7] * D[1][7] +
					   F[8][9] * D[1][9] +
					   F[8][10] * D[1][10] +
					   F[8][11] * D[1][11] +
					   F[8][12] * D[1][12]) * T +
	    D[1][8];
	P[1][9] = P[9][1] =
	    (F[9][6] * D[4][6] + F[9][7] * D[4][7] + F[9][8] * D[4][8] +
	     F[9][10] * D[4][10] + F[9][11] * D[4][11] +
	     F[9][12] * D[4][12]) * Tsq + (D[4][9] + F[9][6] * D[1][6] +
					   F[9][7] * D[1][7] +
					   F[9][8] * D[1][8] +
					   F[9][10] * D[1][10] +
					   F[9][11] * D[1][11] +
					   F[9][12] * D[1][12]) * T +
	    D[1][9];
	P[1][10] = P[10][1] = D[4][10] * T + D[1][10];
	P[1][11] = P[11][1] = D[4][11] * T + D[1][11];
	P[1][12] = P[12][1] = D[4][12] * T + D[1][12];
	P[2][2] = D[5][5] * Tsq + (2 * D[2][5]) * T + D[2][2];
	P[2][3] = P[3][2] =
	    (F[3][6] * D[5][6] + F[3][7] * D[5][7] + F[3][8] * D[5][8] +
	     F[3][9] * D[5][9]) * Tsq + (D[3][5] + F[3][6] * D[2][6] +
					 F[3][7] * D[2][7] +
					 F[3][8] * D[2][8] +
					 F[3][9] * D[2][9]) * T + D[2][3];
	P[2][4] = P[4][2] =
	    (F[4][6] * D[5][6] + F[4][7] * D[5][7] + F[4][8] * D[5][8] +
	     F[4][9] * D[5][9]) * Tsq + (D[4][5] + F[4][6] * D[2][6] +
					 F[4][7] * D[2][7] +
					 F[4][8] * D[2][8] +
					 F[4][9] * D[2][9]) * T + D[2][4];
	P[2][5] = P[5][2] =
	    (F[5][6] * D[5][6] + F[5][7] * D[5][7] + F[5][8] * D[5][8] +
	     F[5][9] * D[5][9]) * Tsq + (D[5][5] + F[5][6] * D[2][6] +
					 F[5][7] * D[2][7] +
					 F[5][8] * D[2][8] +
					 F[5][9] * D[2][9]) * T + D[2][5];
	P[2][6] = P[6][2] =
	    (F[6][7] * D[5][7] + F[6][8] * D[5][8] + F[6][9] * D[5][9] +
	     F[6][10] * D[5][10] + F[6][11] * D[5][11] +
	     F[6][12] * D[5][12]) * Tsq + (D[5][6] + F[6][7] * D[2][7] +
					   F[6][8] * D[2][8] +
					   F[6][9] * D[2][9] +
					   F[6][10] * D[2][10] +
					   F[6][11] * D[2][11] +
					   F[6][12] * D[2][12]) * T +
	    D[2][6];
	P[2][7] = P[7][2] =
	    (F[7][6] * D[5][6] + F[7][8] * D[5][8] + F[7][9] * D[5][9] +
	     F[7][10] * D[5][10] + F[7][11] * D[5][11] +
	     F[7][12] * D[5][12]) * Tsq + (D[5][7] + F[7][6] * D[2][6] +
					   F[7][8] * D[2][8] +
					   F[7][9] * D[2][9] +
					   F[7][10] * D[2][10] +
					   F[7][11] * D[2][11] +
					   F[7][12] * D[2][12]) * T +
	    D[2][7];
	P[2][8] = P[8][2] =
	    (F[8][6] * D[5][6] + F[8][7] * D[5][7] + F[8][9] * D[5][9] +
	     F[8][10] * D[5][10] + F[8][11] * D[5][11] +
	     F[8][12] * D[5][12]) * Tsq + (D[5][8] + F[8][6] * D[2][6] +
					   F[8][7] * D[2][7] +
					   F[8][9] * D[2][9] +
					   F[8][10] * D[2][10] +
					   F[8][11] * D[2][11] +
					   F[8][12] * D[2][12]) * T +
	    D[2][8];
	P[2][9] = P[9][2] =
	    (F[9][6] * D[5][6] + F[9][7] * D[5][7] + F[9][8] * D[5][8] +
	     F[9][10] * D[5][10] + F[9][11] * D[5][11] +
	     F[9][12] * D[5][12]) * Tsq + (D[5][9] + F[9][6] * D[2][6] +
					   F[9][7] * D[2][7] +
					   F[9][8] * D[2][8] +
					   F[9][10] * D[2][10] +
					   F[9][11] * D[2][11] +
					   F[9][12] * D[2][12]) * T +
	    D[2][9];
	P[2][10] = P[10][2] = D[5][10] * T + D[2][10];
	P[2][11] = P[11][2] = D[5][11] * T + D[2][11];
	P[2][12] = P[12][2] = D[5][12] * T + D[2][12];
	P[3][3] =
	    (Q[3] * G[3][3] * G[3][3] + Q[4] * G[3][4] * G[3][4] +
	     Q[5] * G[3][5] * G[3][5] + F[3][9] * (F[3][9] * D[9][9] +
						   F[3][6] * D[6][9] +
						   F[3][7] * D[7][9] +
						   F[3][8] * D[8][9]) +
	     F[3][6] * (F[3][6] * D[6][6] + F[3][7] * D[6][7] +
			F[3][8] * D[6][8] + F[3][9] * D[6][9]) +
	     F[3][7] * (F[3][6] * D[6][7] + F[3][7] * D[7][7] +
			F[3][8] * D[7][8] + F[3][9] * D[7][9]) +
	     F[3][8] * (F[3][6] * D[6][8] + F[3][7] * D[7][8] +
			F[3][8] * D[8][8] + F[3][9] * D[8][9])) * Tsq +
	    (2 * F[3][6] * D[3][6] + 2 * F[3][7] * D[3][7] +
	     2 * F[3][8] * D[3][8] + 2 * F[3][9] * D[3][9]) * T + D[3][3];
	P[3][4] = P[4][3] =
	    (F[4][9] *
	     (F[3][9] * D[9][9] + F[3][6] * D[6][9] + F[3][7] * D[7][9] +
	      F[3][8] * D[8][9]) + F[4][6] * (F[3][6] * D[6][6] +
					      F[3][7] * D[6][7] +
					      F[3][8] * D[6][8] +
					      F[3][9] * D[6][9]) +
	     F[4][7] * (F[3][6] * D[6][7] + F[3][7] * D[7][7] +
			F[3][8] * D[7][8] + F[3][9] * D[7][9]) +
	     F[4][8] * (F[3][6] * D[6][8] + F[3][7] * D[7][8] +
			F[3][8] * D[8][8] + F[3][9] * D[8][9]) +
	     G[3][3] * G[4][3] * Q[3] + G[3][4] * G[4][4] * Q[4] +
	     G[3][5] * G[4][5] * Q[5]) * Tsq + (F[3][6] * D[4][6] +
						F[4][6] * D[3][6] +
						F[3][7] * D[4][7] +
						F[4][7] * D[3][7] +
						F[3][8] * D[4][8] +
						F[4][8] * D[3][8] +
						F[3][9] * D[4][9] +
						F[4][9] * D[3][9]) * T +
	    D[3][4];
	P[3][5] = P[5][3] =
	    (F[5][9] *
	     (F[3][9] * D[9][9] + F[3][6] * D[6][9] + F[3][7] * D[7][9] +
	      F[3][8] * D[8][9]) + F[5][6] * (F[3][6] * D[6][6] +
					      F[3][7] * D[6][7] +
					      F[3][8] * D[6][8] +
					      F[3][9] * D[6][9]) +
	     F[5][7] * (F[3][6] * D[6][7] + F[3][7] * D[7][7] +
			F[3][8] * D[7][8] + F[3][9] * D[7][9]) +
	     F[5][8] * (F[3][6] * D[6][8] + F[3][7] * D[7][8] +
			F[3][8] * D[8][8] + F[3][9] * D[8][9]) +
	     G[3][3] * G[5][3] * Q[3] + G[3][4] * G[5][4] * Q[4] +
	     G[3][5] * G[5][5] * Q[5]) * Tsq + (F[3][6] * D[5][6] +
						F[5][6] * D[3][6] +
						F[3][7] * D[5][7] +
						F[5][7] * D[3][7] +
						F[3][8] * D[5][8] +
						F[5][8] * D[3][8] +
						F[3][9] * D[5][9] +
						F[5][9] * D[3][9]) * T +
	    D[3][5];
	P[3][6] = P[6][3] =
	    (F[6][9] *
	     (F[3][9] * D[9][9] + F[3][6] * D[6][9] + F[3][7] * D[7][9] +
	      F[3][8] * D[8][9]) + F[6][10] * (F[3][9] * D[9][10] +
					       F[3][6] * D[6][10] +
					       F[3][7] * D[7][10] +
					       F[3][8] * D[8][10]) +
	     F[6][11] * (F[3][9] * D[9][11] + F[3][6] * D[6][11] +
			 F[3][7] * D[7][11] + F[3][8] * D[8][11]) +
	     F[6][12] * (F[3][9] * D[9][12] + F[3][6] * D[6][12] +
			 F[3][7] * D[7][12] + F[3][8] * D[8][12]) +
	     F[6][7] * (F[3][6] * D[6][7] + F[3][7] * D[7][7] +
			F[3][8] * D[7][8] + F[3][9] * D[7][9]) +
	     F[6][8] * (F[3][6] * D[6][8] + F[3][7] * D[7][8] +
			F[3][8] * D[8][8] + F[3][9] * D[8][9])) * Tsq +
	    (F[3][6] * D[6][6] + F[3][7] * D[6][7] + F[6][7] * D[3][7] +
	     F[3][8] * D[6][8] + F[6][8] * D[3][8] + F[3][9] * D[6][9] +
	     F[6][9] * D[3][9] + F[6][10] * D[3][10] +
	     F[6][11] * D[3][11] + F[6][12] * D[3][12]) * T + D[3][6];
	P[3][7] = P[7][3] =
	    (F[7][9] *
	     (F[3][9] * D[9][9] + F[3][6] * D[6][9] + F[3][7] * D[7][9] +
	      F[3][8] * D[8][9]) + F[7][10] * (F[3][9] * D[9][10] +
					       F[3][6] * D[6][10] +
					       F[3][7] * D[7][10] +
					       F[3][8] * D[8][10]) +
	     F[7][11] * (F[3][9] * D[9][11] + F[3][6] * D[6][11] +
			 F[3][7] * D[7][11] + F[3][8] * D[8][11]) +
	     F[7][12] * (F[3][9] * D[9][12] + F[3][6] * D[6][12] +
			 F[3][7] * D[7][12] + F[3][8] * D[8][12]) +
	     F[7][6] * (F[3][6] * D[6][6] + F[3][7] * D[6][7] +
			F[3][8] * D[6][8] + F[3][9] * D[6][9]) +
	     F[7][8] * (F[3][6] * D[6][8] + F[3][7] * D[7][8] +
			F[3][8] * D[8][8] + F[3][9] * D[8][9])) * Tsq +
	    (F[3][6] * D[6][7] + F[7][6] * D[3][6] + F[3][7] * D[7][7] +
	     F[3][8] * D[7][8] + F[7][8] * D[3][8] + F[3][9] * D[7][9] +
	     F[7][9] * D[3][9] + F[7][10] * D[3][10] +
	     F[7][11] * D[3][11] + F[7][12] * D[3][12]) * T + D[3][7];
	P[3][8] = P[8][3] =
	    (F[8][9] *
	     (F[3][9] * D[9][9] + F[3][6] * D[6][9] + F[3][7] * D[7][9] +
	      F[3][8] * D[8][9]) + F[8][10] * (F[3][9] * D[9][10] +
					       F[3][6] * D[6][10] +
					       F[3][7] * D[7][10] +
					       F[3][8] * D[8][10]) +
	     F[8][11] * (F[3][9] * D[9][11] + F[3][6] * D[6][11] +
			 F[3][7] * D[7][11] + F[3][8] * D[8][11]) +
	     F[8][12] * (F[3][9] * D[9][12] + F[3][6] * D[6][12] +
			 F[3][7] * D[7][12] + F[3][8] * D[8][12]) +
	     F[8][6] * (F[3][6] * D[6][6] + F[3][7] * D[6][7] +
			F[3][8] * D[6][8] + F[3][9] * D[6][9]) +
	     F[8][7] * (F[3][6] * D[6][7] + F[3][7] * D[7][7] +
			F[3][8] * D[7][8] + F[3][9] * D[7][9])) * Tsq +
	    (F[3][6] * D[6][8] + F[3][7] * D[7][8] + F[8][6] * D[3][6] +
	     F[8][7] * D[3][7] + F[3][8] * D[8][8] + F[3][9] * D[8][9] +
	     F[8][9] * D[3][9] + F[8][10] * D[3][10] +
	     F[8][11] * D[3][11] + F[8][12] * D[3][12]) * T + D[3][8];
	P[3][9] = P[9][3] =
	    (F[9][10] *
	     (F[3][9] * D[9][10] + F[3][6] * D[6][10] +
	      F[3][7] * D[7][10] + F[3][8] * D[8][10]) +
	     F[9][11] * (F[3][9] * D[9][11] + F[3][6] * D[6][11] +
			 F[3][7] * D[7][11] + F[3][8] * D[8][11]) +
	     F[9][12] * (F[3][9] * D[9][12] + F[3][6] * D[6][12] +
			 F[3][7] * D[7][12] + F[3][8] * D[8][12]) +
	     F[9][6] * (F[3][6] * D[6][6] + F[3][7] * D[6][7] +
			F[3][8] * D[6][8] + F[3][9] * D[6][9]) +
	     F[9][7] * (F[3][6] * D[6][7] + F[3][7] * D[7][7] +
			F[3][8] * D[7][8] + F[3][9] * D[7][9]) +
	     F[9][8] * (F[3][6] * D[6][8] + F[3][7] * D[7][8] +
			F[3][8] * D[8][8] + F[3][9] * D[8][9])) * Tsq +
	    (F[9][6] * D[3][6] + F[9][7] * D[3][7] + F[9][8] * D[3][8] +
	     F[3][9] * D[9][9] + F[9][10] * D[3][10] +
	     F[9][11] * D[3][11] + F[9][12] * D[3][12] +
	     F[3][6] * D[6][9] + F[3][7] * D[7][9] +
	     F[3][8] * D[8][9]) * T + D[3][9];
	P[3][10] = P[10][3] =
	    (F[3][9] * D[9][10] + F[3][6] * D[6][10] + F[3][7] * D[7][10] +
	     F[3][8] * D[8][10]) * T + D[3][10];
	P[3][11] = P[11][3] =
	    (F[3][9] * D[9][11] + F[3][6] * D[6][11] + F[3][7] * D[7][11] +
	     F[3][8] * D[8][11]) * T + D[3][11];
	P[3][12] = P[12][3] =
	    (F[3][9] * D[9][12] + F[3][6] * D[6][12] + F[3][7] * D[7][12] +
	     F[3][8] * D[8][12]) * T + D[3][12];
	P[4][4] =
	    (Q[3] * G[4][3] * G[4][3] + Q[4] * G[4][4] * G[4][4] +
	     Q[5] * G[4][5] * G[4][5] + F[4][9] * (F[4][9] * D[9][9] +
						   F[4][6] * D[6][9] +
						   F[4][7] * D[7][9] +
						   F[4][8] * D[8][9]) +
	     F[4][6] * (F[4][6] * D[6][6] + F[4][7] * D[6][7] +
			F[4][8] * D[6][8] + F[4][9] * D[6][9]) +
	     F[4][7] * (F[4][6] * D[6][7] + F[4][7] * D[7][7] +
			F[4][8] * D[7][8] + F[4][9] * D[7][9]) +
	     F[4][8] * (F[4][6] * D[6][8] + F[4][7] * D[7][8] +
			F[4][8] * D[8][8] + F[4][9] * D[8][9])) * Tsq +
	    (2 * F[4][6] * D[4][6] + 2 * F[4][7] * D[4][7] +
	     2 * F[4][8] * D[4][8] + 2 * F[4][9] * D[4][9]) * T + D[4][4];
	P[4][5] = P[5][4] =
	    (F[5][9] *
	     (F[4][9] * D[9][9] + F[4][6] * D[6][9] + F[4][7] * D[7][9] +
	      F[4][8] * D[8][9]) + F[5][6] * (F[4][6] * D[6][6] +
					      F[4][7] * D[6][7] +
					      F[4][8] * D[6][8] +
					      F[4][9] * D[6][9]) +
	     F[5][7] * (F[4][6] * D[6][7] + F[4][7] * D[7][7] +
			F[4][8] * D[7][8] + F[4][9] * D[7][9]) +
	     F[5][8] * (F[4][6] * D[6][8] + F[4][7] * D[7][8] +
			F[4][8] * D[8][8] + F[4][9] * D[8][9]) +
	     G[4][3] * G[5][3] * Q[3] + G[4][4] * G[5][4] * Q[4] +
	     G[4][5] * G[5][5] * Q[5]) * Tsq + (F[4][6] * D[5][6] +
						F[5][6] * D[4][6] +
						F[4][7] * D[5][7] +
						F[5][7] * D[4][7] +
						F[4][8] * D[5][8] +
						F[5][8] * D[4][8] +
						F[4][9] * D[5][9] +
						F[5][9] * D[4][9]) * T +
	    D[4][5];
	P[4][6] = P[6][4] =
	    (F[6][9] *
	     (F[4][9] * D[9][9] + F[4][6] * D[6][9] + F[4][7] * D[7][9] +
	      F[4][8] * D[8][9]) + F[6][10] * (F[4][9] * D[9][10] +
					       F[4][6] * D[6][10] +
					       F[4][7] * D[7][10] +
					       F[4][8] * D[8][10]) +
	     F[6][11] * (F[4][9] * D[9][11] + F[4][6] * D[6][11] +
			 F[4][7] * D[7][11] + F[4][8] * D[8][11]) +
	     F[6][12] * (F[4][9] * D[9][12] + F[4][6] * D[6][12] +
			 F[4][7] * D[7][12] + F[4][8] * D[8][12]) +
	     F[6][7] * (F[4][6] * D[6][7] + F[4][7] * D[7][7] +
			F[4][8] * D[7][8] + F[4][9] * D[7][9]) +
	     F[6][8] * (F[4][6] * D[6][8] + F[4][7] * D[7][8] +
			F[4][8] * D[8][8] + F[4][9] * D[8][9])) * Tsq +
	    (F[4][6] * D[6][6] + F[4][7] * D[6][7] + F[6][7] * D[4][7] +
	     F[4][8] * D[6][8] + F[6][8] * D[4][8] + F[4][9] * D[6][9] +
	     F[6][9] * D[4][9] + F[6][10] * D[4][10] +
	     F[6][11] * D[4][11] + F[6][12] * D[4][12]) * T + D[4][6];
	P[4][7] = P[7][4] =
	    (F[7][9] *
	     (F[4][9] * D[9][9] + F[4][6] * D[6][9] + F[4][7] * D[7][9] +
	      F[4][8] * D[8][9]) + F[7][10] * (F[4][9] * D[9][10] +
					       F[4][6] * D[6][10] +
					       F[4][7] * D[7][10] +
					       F[4][8] * D[8][10]) +
	     F[7][11] * (F[4][9] * D[9][11] + F[4][6] * D[6][11] +
			 F[4][7] * D[7][11] + F[4][8] * D[8][11]) +
	     F[7][12] * (F[4][9] * D[9][12] + F[4][6] * D[6][12] +
			 F[4][7] * D[7][12] + F[4][8] * D[8][12]) +
	     F[7][6] * (F[4][6] * D[6][6] + F[4][7] * D[6][7] +
			F[4][8] * D[6][8] + F[4][9] * D[6][9]) +
	     F[7][8] * (F[4][6] * D[6][8] + F[4][7] * D[7][8] +
			F[4][8] * D[8][8] + F[4][9] * D[8][9])) * Tsq +
	    (F[4][6] * D[6][7] + F[7][6] * D[4][6] + F[4][7] * D[7][7] +
	     F[4][8] * D[7][8] + F[7][8] * D[4][8] + F[4][9] * D[7][9] +
	     F[7][9] * D[4][9] + F[7][10] * D[4][10] +
	     F[7][11] * D[4][11] + F[7][12] * D[4][12]) * T + D[4][7];
	P[4][8] = P[8][4] =
	    (F[8][9] *
	     (F[4][9] * D[9][9] + F[4][6] * D[6][9] + F[4][7] * D[7][9] +
	      F[4][8] * D[8][9]) + F[8][10] * (F[4][9] * D[9][10] +
					       F[4][6] * D[6][10] +
					       F[4][7] * D[7][10] +
					       F[4][8] * D[8][10]) +
	     F[8][11] * (F[4][9] * D[9][11] + F[4][6] * D[6][11] +
			 F[4][7] * D[7][11] + F[4][8] * D[8][11]) +
	     F[8][12] * (F[4][9] * D[9][12] + F[4][6] * D[6][12] +
			 F[4][7] * D[7][12] + F[4][8] * D[8][12]) +
	     F[8][6] * (F[4][6] * D[6][6] + F[4][7] * D[6][7] +
			F[4][8] * D[6][8] + F[4][9] * D[6][9]) +
	     F[8][7] * (F[4][6] * D[6][7] + F[4][7] * D[7][7] +
			F[4][8] * D[7][8] + F[4][9] * D[7][9])) * Tsq +
	    (F[4][6] * D[6][8] + F[4][7] * D[7][8] + F[8][6] * D[4][6] +
	     F[8][7] * D[4][7] + F[4][8] * D[8][8] + F[4][9] * D[8][9] +
	     F[8][9] * D[4][9] + F[8][10] * D[4][10] +
	     F[8][11] * D[4][11] + F[8][12] * D[4][12]) * T + D[4][8];
	P[4][9] = P[9][4] =
	    (F[9][10] *
	     (F[4][9] * D[9][10] + F[4][6] * D[6][10] +
	      F[4][7] * D[7][10] + F[4][8] * D[8][10]) +
	     F[9][11] * (F[4][9] * D[9][11] + F[4][6] * D[6][11] +
			 F[4][7] * D[7][11] + F[4][8] * D[8][11]) +
	     F[9][12] * (F[4][9] * D[9][12] + F[4][6] * D[6][12] +
			 F[4][7] * D[7][12] + F[4][8] * D[8][12]) +
	     F[9][6] * (F[4][6] * D[6][6] + F[4][7] * D[6][7] +
			F[4][8] * D[6][8] + F[4][9] * D[6][9]) +
	     F[9][7] * (F[4][6] * D[6][7] + F[4][7] * D[7][7] +
			F[4][8] * D[7][8] + F[4][9] * D[7][9]) +
	     F[9][8] * (F[4][6] * D[6][8] + F[4][7] * D[7][8] +
			F[4][8] * D[8][8] + F[4][9] * D[8][9])) * Tsq +
	    (F[9][6] * D[4][6] + F[9][7] * D[4][7] + F[9][8] * D[4][8] +
	     F[4][9] * D[9][9] + F[9][10] * D[4][10] +
	     F[9][11] * D[4][11] + F[9][12] * D[4][12] +
	     F[4][6] * D[6][9] + F[4][7] * D[7][9] +
	     F[4][8] * D[8][9]) * T + D[4][9];
	P[4][10] = P[10][4] =
	    (F[4][9] * D[9][10] + F[4][6] * D[6][10] + F[4][7] * D[7][10] +
	     F[4][8] * D[8][10]) * T + D[4][10];
	P[4][11] = P[11][4] =
	    (F[4][9] * D[9][11] + F[4][6] * D[6][11] + F[4][7] * D[7][11] +
	     F[4][8] * D[8][11]) * T + D[4][11];
	P[4][12] = P[12][4] =
	    (F[4][9] * D[9][12] + F[4][6] * D[6][12] + F[4][7] * D[7][12] +
	     F[4][8] * D[8][12]) * T + D[4][12];
	P[5][5] =
	    (Q[3] * G[5][3] * G[5][3] + Q[4] * G[5][4] * G[5][4] +
	     Q[5] * G[5][5] * G[5][5] + F[5][9] * (F[5][9] * D[9][9] +
						   F[5][6] * D[6][9] +
						   F[5][7] * D[7][9] +
						   F[5][8] * D[8][9]) +
	     F[5][6] * (F[5][6] * D[6][6] + F[5][7] * D[6][7] +
			F[5][8] * D[6][8] + F[5][9] * D[6][9]) +
	     F[5][7] * (F[5][6] * D[6][7] + F[5][7] * D[7][7] +
			F[5][8] * D[7][8] + F[5][9] * D[7][9]) +
	     F[5][8] * (F[5][6] * D[6][8] + F[5][7] * D[7][8] +
			F[5][8] * D[8][8] + F[5][9] * D[8][9])) * Tsq +
	    (2 * F[5][6] * D[5][6] + 2 * F[5][7] * D[5][7] +
	     2 * F[5][8] * D[5][8] + 2 * F[5][9] * D[5][9]) * T + D[5][5];
	P[5][6] = P[6][5] =
	    (F[6][9] *
	     (F[5][9] * D[9][9] + F[5][6] * D[6][9] + F[5][7] * D[7][9] +
	      F[5][8] * D[8][9]) + F[6][10] * (F[5][9] * D[9][10] +
					       F[5][6] * D[6][10] +
					       F[5][7] * D[7][10] +
					       F[5][8] * D[8][10]) +
	     F[6][11] * (F[5][9] * D[9][11] + F[5][6] * D[6][11] +
			 F[5][7] * D[7][11] + F[5][8] * D[8][11]) +
	     F[6][12] * (F[5][9] * D[9][12] + F[5][6] * D[6][12] +
			 F[5][7] * D[7][12] + F[5][8] * D[8][12]) +
	     F[6][7] * (F[5][6] * D[6][7] + F[5][7] * D[7][7] +
			F[5][8] * D[7][8] + F[5][9] * D[7][9]) +
	     F[6][8] * (F[5][6] * D[6][8] + F[5][7] * D[7][8] +
			F[5][8] * D[8][8] + F[5][9] * D[8][9])) * Tsq +
	    (F[5][6] * D[6][6] + F[5][7] * D[6][7] + F[6][7] * D[5][7] +
	     F[5][8] * D[6][8] + F[6][8] * D[5][8] + F[5][9] * D[6][9] +
	     F[6][9] * D[5][9] + F[6][10] * D[5][10] +
	     F[6][11] * D[5][11] + F[6][12] * D[5][12]) * T + D[5][6];
	P[5][7] = P[7][5] =
	    (F[7][9] *
	     (F[5][9] * D[9][9] + F[5][6] * D[6][9] + F[5][7] * D[7][9] +
	      F[5][8] * D[8][9]) + F[7][10] * (F[5][9] * D[9][10] +
					       F[5][6] * D[6][10] +
					       F[5][7] * D[7][10] +
					       F[5][8] * D[8][10]) +
	     F[7][11] * (F[5][9] * D[9][11] + F[5][6] * D[6][11] +
			 F[5][7] * D[7][11] + F[5][8] * D[8][11]) +
	     F[7][12] * (F[5][9] * D[9][12] + F[5][6] * D[6][12] +
			 F[5][7] * D[7][12] + F[5][8] * D[8][12]) +
	     F[7][6] * (F[5][6] * D[6][6] + F[5][7] * D[6][7] +
			F[5][8] * D[6][8] + F[5][9] * D[6][9]) +
	     F[7][8] * (F[5][6] * D[6][8] + F[5][7] * D[7][8] +
			F[5][8] * D[8][8] + F[5][9] * D[8][9])) * Tsq +
	    (F[5][6] * D[6][7] + F[7][6] * D[5][6] + F[5][7] * D[7][7] +
	     F[5][8] * D[7][8] + F[7][8] * D[5][8] + F[5][9] * D[7][9] +
	     F[7][9] * D[5][9] + F[7][10] * D[5][10] +
	     F[7][11] * D[5][11] + F[7][12] * D[5][12]) * T + D[5][7];
	P[5][8] = P[8][5] =
	    (F[8][9] *
	     (F[5][9] * D[9][9] + F[5][6] * D[6][9] + F[5][7] * D[7][9] +
	      F[5][8] * D[8][9]) + F[8][10] * (F[5][9] * D[9][10] +
					       F[5][6] * D[6][10] +
					       F[5][7] * D[7][10] +
					       F[5][8] * D[8][10]) +
	     F[8][11] * (F[5][9] * D[9][11] + F[5][6] * D[6][11] +
			 F[5][7] * D[7][11] + F[5][8] * D[8][11]) +
	     F[8][12] * (F[5][9] * D[9][12] + F[5][6] * D[6][12] +
			 F[5][7] * D[7][12] + F[5][8] * D[8][12]) +
	     F[8][6] * (F[5][6] * D[6][6] + F[5][7] * D[6][7] +
			F[5][8] * D[6][8] + F[5][9] * D[6][9]) +
	     F[8][7] * (F[5][6] * D[6][7] + F[5][7] * D[7][7] +
			F[5][8] * D[7][8] + F[5][9] * D[7][9])) * Tsq +
	    (F[5][6] * D[6][8] + F[5][7] * D[7][8] + F[8][6] * D[5][6] +
	     F[8][7] * D[5][7] + F[5][8] * D[8][8] + F[5][9] * D[8][9] +
	     F[8][9] * D[5][9] + F[8][10] * D[5][10] +
	     F[8][11] * D[5][11] + F[8][12] * D[5][12]) * T + D[5][8];
	P[5][9] = P[9][5] =
	    (F[9][10] *
	     (F[5][9] * D[9][10] + F[5][6] * D[6][10] +
	      F[5][7] * D[7][10] + F[5][8] * D[8][10]) +
	     F[9][11] * (F[5][9] * D[9][11] + F[5][6] * D[6][11] +
			 F[5][7] * D[7][11] + F[5][8] * D[8][11]) +
	     F[9][12] * (F[5][9] * D[9][12] + F[5][6] * D[6][12] +
			 F[5][7] * D[7][12] + F[5][8] * D[8][12]) +
	     F[9][6] * (F[5][6] * D[6][6] + F[5][7] * D[6][7] +
			F[5][8] * D[6][8] + F[5][9] * D[6][9]) +
	     F[9][7] * (F[5][6] * D[6][7] + F[5][7] * D[7][7] +
			F[5][8] * D[7][8] + F[5][9] * D[7][9]) +
	     F[9][8] * (F[5][6] * D[6][8] + F[5][7] * D[7][8] +
			F[5][8] * D[8][8] + F[5][9] * D[8][9])) * Tsq +
	    (F[9][6] * D[5][6] + F[9][7] * D[5][7] + F[9][8] * D[5][8] +
	     F[5][9] * D[9][9] + F[9][10] * D[5][10] +
	     F[9][11] * D[5][11] + F[9][12] * D[5][12] +
	     F[5][6] * D[6][9] + F[5][7] * D[7][9] +
	     F[5][8] * D[8][9]) * T + D[5][9];
	P[5][10] = P[10][5] =
	    (F[5][9] * D[9][10] + F[5][6] * D[6][10] + F[5][7] * D[7][10] +
	     F[5][8] * D[8][10]) * T + D[5][10];
	P[5][11] = P[11][5] =
	    (F[5][9] * D[9][11] + F[5][6] * D[6][11] + F[5][7] * D[7][11] +
	     F[5][8] * D[8][11]) * T + D[5][11];
	P[5][12] = P[12][5] =
	    (F[5][9] * D[9][12] + F[5][6] * D[6][12] + F[5][7] * D[7][12] +
	     F[5][8] * D[8][12]) * T + D[5][12];
	P[6][6] =
	    (Q[0] * G[6][0] * G[6][0] + Q[1] * G[6][1] * G[6][1] +
	     Q[2] * G[6][2] * G[6][2] + F[6][9] * (F[6][9] * D[9][9] +
						   F[6][10] * D[9][10] +
						   F[6][11] * D[9][11] +
						   F[6][12] * D[9][12] +
						   F[6][7] * D[7][9] +
						   F[6][8] * D[8][9]) +
	     F[6][10] * (F[6][9] * D[9][10] + F[6][10] * D[10][10] +
			 F[6][11] * D[10][11] + F[6][12] * D[10][12] +
			 F[6][7] * D[7][10] + F[6][8] * D[8][10]) +
	     F[6][11] * (F[6][9] * D[9][11] + F[6][10] * D[10][11] +
			 F[6][11] * D[11][11] + F[6][12] * D[11][12] +
			 F[6][7] * D[7][11] + F[6][8] * D[8][11]) +
	     F[6][12] * (F[6][9] * D[9][12] + F[6][10] * D[10][12] +
			 F[6][11] * D[11][12] + F[6][12] * D[12][12] +
			 F[6][7] * D[7][12] + F[6][8] * D[8][12]) +
	     F[6][7] * (F[6][7] * D[7][7] + F[6][8] * D[7][8] +
			F[6][9] * D[7][9] + F[6][10] * D[7][10] +
			F[6][11] * D[7][11] + F[6][12] * D[7][12]) +
	     F[6][8] * (F[6][7] * D[7][8] + F[6][8] * D[8][8] +
			F[6][9] * D[8][9] + F[6][10] * D[8][10] +
			F[6][11] * D[8][11] + F[6][12] * D[8][12])) * Tsq +
	    (2 * F[6][7] * D[6][7] + 2 * F[6][8] * D[6][8] +
	     2 * F[6][9] * D[6][9] + 2 * F[6][10] * D[6][10] +
	     2 * F[6][11] * D[6][11] + 2 * F[6][12] * D[6][12]) * T +
	    D[6][6];
	P[6][7] = P[7][6] =
	    (F[7][9] *
	     (F[6][9] * D[9][9] + F[6][10] * D[9][10] +
	      F[6][11] * D[9][11] + F[6][12] * D[9][12] +
	      F[6][7] * D[7][9] + F[6][8] * D[8][9]) +
	     F[7][10] * (F[6][9] * D[9][10] + F[6][10] * D[10][10] +
			 F[6][11] * D[10][11] + F[6][12] * D[10][12] +
			 F[6][7] * D[7][10] + F[6][8] * D[8][10]) +
	     F[7][11] * (F[6][9] * D[9][11] + F[6][10] * D[10][11] +
			 F[6][11] * D[11][11] + F[6][12] * D[11][12] +
			 F[6][7] * D[7][11] + F[6][8] * D[8][11]) +
	     F[7][12] * (F[6][9] * D[9][12] + F[6][10] * D[10][12] +
			 F[6][11] * D[11][12] + F[6][12] * D[12][12] +
			 F[6][7] * D[7][12] + F[6][8] * D[8][12]) +
	     F[7][6] * (F[6][7] * D[6][7] + F[6][8] * D[6][8] +
			F[6][9] * D[6][9] + F[6][10] * D[6][10] +
			F[6][11] * D[6][11] + F[6][12] * D[6][12]) +
	     F[7][8] * (F[6][7] * D[7][8] + F[6][8] * D[8][8] +
			F[6][9] * D[8][9] + F[6][10] * D[8][10] +
			F[6][11] * D[8][11] + F[6][12] * D[8][12]) +
	     G[6][0] * G[7][0] * Q[0] + G[6][1] * G[7][1] * Q[1] +
	     G[6][2] * G[7][2] * Q[2]) * Tsq + (F[7][6] * D[6][6] +
						F[6][7] * D[7][7] +
						F[6][8] * D[7][8] +
						F[7][8] * D[6][8] +
						F[6][9] * D[7][9] +
						F[7][9] * D[6][9] +
						F[6][10] * D[7][10] +
						F[7][10] * D[6][10] +
						F[6][11] * D[7][11] +
						F[7][11] * D[6][11] +
						F[6][12] * D[7][12] +
						F[7][12] * D[6][12]) * T +
	    D[6][7];
	P[6][8] = P[8][6] =
	    (F[8][9] *
	     (F[6][9] * D[9][9] + F[6][10] * D[9][10] +
	      F[6][11] * D[9][11] + F[6][12] * D[9][12] +
	      F[6][7] * D[7][9] + F[6][8] * D[8][9]) +
	     F[8][10] * (F[6][9] * D[9][10] + F[6][10] * D[10][10] +
			 F[6][11] * D[10][11] + F[6][12] * D[10][12] +
			 F[6][7] * D[7][10] + F[6][8] * D[8][10]) +
	     F[8][11] * (F[6][9] * D[9][11] + F[6][10] * D[10][11] +
			 F[6][11] * D[11][11] + F[6][12] * D[11][12] +
			 F[6][7] * D[7][11] + F[6][8] * D[8][11]) +
	     F[8][12] * (F[6][9] * D[9][12] + F[6][10] * D[10][12] +
			 F[6][11] * D[11][12] + F[6][12] * D[12][12] +
			 F[6][7] * D[7][12] + F[6][8] * D[8][12]) +
	     F[8][6] * (F[6][7] * D[6][7] + F[6][8] * D[6][8] +
			F[6][9] * D[6][9] + F[6][10] * D[6][10] +
			F[6][11] * D[6][11] + F[6][12] * D[6][12]) +
	     F[8][7] * (F[6][7] * D[7][7] + F[6][8] * D[7][8] +
			F[6][9] * D[7][9] + F[6][10] * D[7][10] +
			F[6][11] * D[7][11] + F[6][12] * D[7][12]) +
	     G[6][0] * G[8][0] * Q[0] + G[6][1] * G[8][1] * Q[1] +
	     G[6][2] * G[8][2] * Q[2]) * Tsq + (F[6][7] * D[7][8] +
						F[8][6] * D[6][6] +
						F[8][7] * D[6][7] +
						F[6][8] * D[8][8] +
						F[6][9] * D[8][9] +
						F[8][9] * D[6][9] +
						F[6][10] * D[8][10] +
						F[8][10] * D[6][10] +
						F[6][11] * D[8][11] +
						F[8][11] * D[6][11] +
						F[6][12] * D[8][12] +
						F[8][12] * D[6][12]) * T +
	    D[6][8];
	P[6][9] = P[9][6] =
	    (F[9][10] *
	     (F[6][9] * D[9][10] + F[6][10] * D[10][10] +
	      F[6][11] * D[10][11] + F[6][12] * D[10][12] +
	      F[6][7] * D[7][10] + F[6][8] * D[8][10]) +
	     F[9][11] * (F[6][9] * D[9][11] + F[6][10] * D[10][11] +
			 F[6][11] * D[11][11] + F[6][12] * D[11][12] +
			 F[6][7] * D[7][11] + F[6][8] * D[8][11]) +
	     F[9][12] * (F[6][9] * D[9][12] + F[6][10] * D[10][12] +
			 F[6][11] * D[11][12] + F[6][12] * D[12][12] +
			 F[6][7] * D[7][12] + F[6][8] * D[8][12]) +
	     F[9][6] * (F[6][7] * D[6][7] + F[6][8] * D[6][8] +
			F[6][9] * D[6][9] + F[6][10] * D[6][10] +
			F[6][11] * D[6][11] + F[6][12] * D[6][12]) +
	     F[9][7] * (F[6][7] * D[7][7] + F[6][8] * D[7][8] +
			F[6][9] * D[7][9] + F[6][10] * D[7][10] +
			F[6][11] * D[7][11] + F[6][12] * D[7][12]) +
	     F[9][8] * (F[6][7] * D[7][8] + F[6][8] * D[8][8] +
			F[6][9] * D[8][9] + F[6][10] * D[8][10] +
			F[6][11] * D[8][11] + F[6][12] * D[8][12]) +
	     G[9][0] * G[6][0] * Q[0] + G[9][1] * G[6][1] * Q[1] +
	     G[9][2] * G[6][2] * Q[2]) * Tsq + (F[9][6] * D[6][6] +
						F[9][7] * D[6][7] +
						F[9][8] * D[6][8] +
						F[6][9] * D[9][9] +
						F[9][10] * D[6][10] +
						F[6][10] * D[9][10] +
						F[9][11] * D[6][11] +
						F[6][11] * D[9][11] +
						F[9][12] * D[6][12] +
						F[6][12] * D[9][12] +
						F[6][7] * D[7][9] +
						F[6][8] * D[8][9]) * T +
	    D[6][9];
	P[6][10] = P[10][6] =
	    (F[6][9] * D[9][10] + F[6][10] * D[10][10] +
	     F[6][11] * D[10][11] + F[6][12] * D[10][12] +
	     F[6][7] * D[7][10] + F[6][8] * D[8][10]) * T + D[6][10];
	P[6][11] = P[11][6] =
	    (F[6][9] * D[9][11] + F[6][10] * D[10][11] +
	     F[6][11] * D[11][11] + F[6][12] * D[11][12] +
	     F[6][7] * D[7][11] + F[6][8] * D[8][11]) * T + D[6][11];
	P[6][12] = P[12][6] =
	    (F[6][9] * D[9][12] + F[6][10] * D[10][12] +
	     F[6][11] * D[11][12] + F[6][12] * D[12][12] +
	     F[6][7] * D[7][12] + F[6][8] * D[8][12]) * T + D[6][12];
	P[7][7] =
	    (Q[0] * G[7][0] * G[7][0] + Q[1] * G[7][1] * G[7][1] +
	     Q[2] * G[7][2] * G[7][2] + F[7][9] * (F[7][9] * D[9][9] +
						   F[7][10] * D[9][10] +
						   F[7][11] * D[9][11] +
						   F[7][12] * D[9][12] +
						   F[7][6] * D[6][9] +
						   F[7][8] * D[8][9]) +
	     F[7][10] * (F[7][9] * D[9][10] + F[7][10] * D[10][10] +
			 F[7][11] * D[10][11] + F[7][12] * D[10][12] +
			 F[7][6] * D[6][10] + F[7][8] * D[8][10]) +
	     F[7][11] * (F[7][9] * D[9][11] + F[7][10] * D[10][11] +
			 F[7][11] * D[11][11] + F[7][12] * D[11][12] +
			 F[7][6] * D[6][11] + F[7][8] * D[8][11]) +
	     F[7][12] * (F[7][9] * D[9][12] + F[7][10] * D[10][12] +
			 F[7][11] * D[11][12] + F[7][12] * D[12][12] +
			 F[7][6] * D[6][12] + F[7][8] * D[8][12]) +
	     F[7][6] * (F[7][6] * D[6][6] + F[7][8] * D[6][8] +
			F[7][9] * D[6][9] + F[7][10] * D[6][10] +
			F[7][11] * D[6][11] + F[7][12] * D[6][12]) +
	     F[7][8] * (F[7][6] * D[6][8] + F[7][8] * D[8][8] +
			F[7][9] * D[8][9] + F[7][10] * D[8][10] +
			F[7][11] * D[8][11] + F[7][12] * D[8][12])) * Tsq +
	    (2 * F[7][6] * D[6][7] + 2 * F[7][8] * D[7][8] +
	     2 * F[7][9] * D[7][9] + 2 * F[7][10] * D[7][10] +
	     2 * F[7][11] * D[7][11] + 2 * F[7][12] * D[7][12]) * T +
	    D[7][7];
	P[7][8] = P[8][7] =
	    (F[8][9] *
	     (F[7][9] * D[9][9] + F[7][10] * D[9][10] +
	      F[7][11] * D[9][11] + F[7][12] * D[9][12] +
	      F[7][6] * D[6][9] + F[7][8] * D[8][9]) +
	     F[8][10] * (F[7][9] * D[9][10] + F[7][10] * D[10][10] +
			 F[7][11] * D[10][11] + F[7][12] * D[10][12] +
			 F[7][6] * D[6][10] + F[7][8] * D[8][10]) +
	     F[8][11] * (F[7][9] * D[9][11] + F[7][10] * D[10][11] +
			 F[7][11] * D[11][11] + F[7][12] * D[11][12] +
			 F[7][6] * D[6][11] + F[7][8] * D[8][11]) +
	     F[8][12] * (F[7][9] * D[9][12] + F[7][10] * D[10][12] +
			 F[7][11] * D[11][12] + F[7][12] * D[12][12] +
			 F[7][6] * D[6][12] + F[7][8] * D[8][12]) +
	     F[8][6] * (F[7][6] * D[6][6] + F[7][8] * D[6][8] +
			F[7][9] * D[6][9] + F[7][10] * D[6][10] +
			F[7][11] * D[6][11] + F[7][12] * D[6][12]) +
	     F[8][7] * (F[7][6] * D[6][7] + F[7][8] * D[7][8] +
			F[7][9] * D[7][9] + F[7][10] * D[7][10] +
			F[7][11] * D[7][11] + F[7][12] * D[7][12]) +
	     G[7][0] * G[8][0] * Q[0] + G[7][1] * G[8][1] * Q[1] +
	     G[7][2] * G[8][2] * Q[2]) * Tsq + (F[7][6] * D[6][8] +
						F[8][6] * D[6][7] +
						F[8][7] * D[7][7] +
						F[7][8] * D[8][8] +
						F[7][9] * D[8][9] +
						F[8][9] * D[7][9] +
						F[7][10] * D[8][10] +
						F[8][10] * D[7][10] +
						F[7][11] * D[8][11] +
						F[8][11] * D[7][11] +
						F[7][12] * D[8][12] +
						F[8][12] * D[7][12]) * T +
	    D[7][8];
	P[7][9] = P[9][7] =
	    (F[9][10] *
	     (F[7][9] * D[9][10] + F[7][10] * D[10][10] +
	      F[7][11] * D[10][11] + F[7][12] * D[10][12] +
	      F[7][6] * D[6][10] + F[7][8] * D[8][10]) +
	     F[9][11] * (F[7][9] * D[9][11] + F[7][10] * D[10][11] +
			 F[7][11] * D[11][11] + F[7][12] * D[11][12] +
			 F[7][6] * D[6][11] + F[7][8] * D[8][11]) +
	     F[9][12] * (F[7][9] * D[9][12] + F[7][10] * D[10][12] +
			 F[7][11] * D[11][12] + F[7][12] * D[12][12] +
			 F[7][6] * D[6][12] + F[7][8] * D[8][12]) +
	     F[9][6] * (F[7][6] * D[6][6] + F[7][8] * D[6][8] +
			F[7][9] * D[6][9] + F[7][10] * D[6][10] +
			F[7][11] * D[6][11] + F[7][12] * D[6][12]) +
	     F[9][7] * (F[7][6] * D[6][7] + F[7][8] * D[7][8] +
			F[7][9] * D[7][9] + F[7][10] * D[7][10] +
			F[7][11] * D[7][11] + F[7][12] * D[7][12]) +
	     F[9][8] * (F[7][6] * D[6][8] + F[7][8] * D[8][8] +
			F[7][9] * D[8][9] + F[7][10] * D[8][10] +
			F[7][11] * D[8][11] + F[7][12] * D[8][12]) +
	     G[9][0] * G[7][0] * Q[0] + G[9][1] * G[7][1] * Q[1] +
	     G[9][2] * G[7][2] * Q[2]) * Tsq + (F[9][6] * D[6][7] +
						F[9][7] * D[7][7] +
						F[9][8] * D[7][8] +
						F[7][9] * D[9][9] +
						F[9][10] * D[7][10] +
						F[7][10] * D[9][10] +
						F[9][11] * D[7][11] +
						F[7][11] * D[9][11] +
						F[9][12] * D[7][12] +
						F[7][12] * D[9][12] +
						F[7][6] * D[6][9] +
						F[7][8] * D[8][9]) * T +
	    D[7][9];
	P[7][10] = P[10][7] =
	    (F[7][9] * D[9][10] + F[7][10] * D[10][10] +
	     F[7][11] * D[10][11] + F[7][12] * D[10][12] +
	     F[7][6] * D[6][10] + F[7][8] * D[8][10]) * T + D[7][10];
	P[7][11] = P[11][7] =
	    (F[7][9] * D[9][11] + F[7][10] * D[10][11] +
	     F[7][11] * D[11][11] + F[7][12] * D[11][12] +
	     F[7][6] * D[6][11] + F[7][8] * D[8][11]) * T + D[7][11];
	P[7][12] = P[12][7] =
	    (F[7][9] * D[9][12] + F[7][10] * D[10][12] +
	     F[7][11] * D[11][12] + F[7][12] * D[12][12] +
	     F[7][6] * D[6][12] + F[7][8] * D[8][12]) * T + D[7][12];
	P[8][8] =
	    (Q[0] * G[8][0] * G[8][0] + Q[1] * G[8][1] * G[8][1] +
	     Q[2] * G[8][2] * G[8][2] + F[8][9] * (F[8][9] * D[9][9] +
						   F[8][10] * D[9][10] +
						   F[8][11] * D[9][11] +
						   F[8][12] * D[9][12] +
						   F[8][6] * D[6][9] +
						   F[8][7] * D[7][9]) +
	     F[8][10] * (F[8][9] * D[9][10] + F[8][10] * D[10][10] +
			 F[8][11] * D[10][11] + F[8][12] * D[10][12] +
			 F[8][6] * D[6][10] + F[8][7] * D[7][10]) +
	     F[8][11] * (F[8][9] * D[9][11] + F[8][10] * D[10][11] +
			 F[8][11] * D[11][11] + F[8][12] * D[11][12] +
			 F[8][6] * D[6][11] + F[8][7] * D[7][11]) +
	     F[8][12] * (F[8][9] * D[9][12] + F[8][10] * D[10][12] +
			 F[8][11] * D[11][12] + F[8][12] * D[12][12] +
			 F[8][6] * D[6][12] + F[8][7] * D[7][12]) +
	     F[8][6] * (F[8][6] * D[6][6] + F[8][7] * D[6][7] +
			F[8][9] * D[6][9] + F[8][10] * D[6][10] +
			F[8][11] * D[6][11] + F[8][12] * D[6][12]) +
	     F[8][7] * (F[8][6] * D[6][7] + F[8][7] * D[7][7] +
			F[8][9] * D[7][9] + F[8][10] * D[7][10] +
			F[8][11] * D[7][11] + F[8][12] * D[7][12])) * Tsq +
	    (2 * F[8][6] * D[6][8] + 2 * F[8][7] * D[7][8] +
	     2 * F[8][9] * D[8][9] + 2 * F[8][10] * D[8][10] +
	     2 * F[8][11] * D[8][11] + 2 * F[8][12] * D[8][12]) * T +
	    D[8][8];
	P[8][9] = P[9][8] =
	    (F[9][10] *
	     (F[8][9] * D[9][10] + F[8][10] * D[10][10] +
	      F[8][11] * D[10][11] + F[8][12] * D[10][12] +
	      F[8][6] * D[6][10] + F[8][7] * D[7][10]) +
	     F[9][11] * (F[8][9] * D[9][11] + F[8][10] * D[10][11] +
			 F[8][11] * D[11][11] + F[8][12] * D[11][12] +
			 F[8][6] * D[6][11] + F[8][7] * D[7][11]) +
	     F[9][12] * (F[8][9] * D[9][12] + F[8][10] * D[10][12] +
			 F[8][11] * D[11][12] + F[8][12] * D[12][12] +
			 F[8][6] * D[6][12] + F[8][7] * D[7][12]) +
	     F[9][6] * (F[8][6] * D[6][6] + F[8][7] * D[6][7] +
			F[8][9] * D[6][9] + F[8][10] * D[6][10] +
			F[8][11] * D[6][11] + F[8][12] * D[6][12]) +
	     F[9][7] * (F[8][6] * D[6][7] + F[8][7] * D[7][7] +
			F[8][9] * D[7][9] + F[8][10] * D[7][10] +
			F[8][11] * D[7][11] + F[8][12] * D[7][12]) +
	     F[9][8] * (F[8][6] * D[6][8] + F[8][7] * D[7][8] +
			F[8][9] * D[8][9] + F[8][10] * D[8][10] +
			F[8][11] * D[8][11] + F[8][12] * D[8][12]) +
	     G[9][0] * G[8][0] * Q[0] + G[9][1] * G[8][1] * Q[1] +
	     G[9][2] * G[8][2] * Q[2]) * Tsq + (F[9][6] * D[6][8] +
						F[9][7] * D[7][8] +
						F[9][8] * D[8][8] +
						F[8][9] * D[9][9] +
						F[9][10] * D[8][10] +
						F[8][10] * D[9][10] +
						F[9][11] * D[8][11] +
						F[8][11] * D[9][11] +
						F[9][12] * D[8][12] +
						F[8][12] * D[9][12] +
						F[8][6] * D[6][9] +
						F[8][7] * D[7][9]) * T +
	    D[8][9];
	P[8][10] = P[10][8] =
	    (F[8][9] * D[9][10] + F[8][10] * D[10][10] +
	     F[8][11] * D[10][11] + F[8][12] * D[10][12] +
	     F[8][6] * D[6][10] + F[8][7] * D[7][10]) * T + D[8][10];
	P[8][11] = P[11][8] =
	    (F[8][9] * D[9][11] + F[8][10] * D[10][11] +
	     F[8][11] * D[11][11] + F[8][12] * D[11][12] +
	     F[8][6] * D[6][11] + F[8][7] * D[7][11]) * T + D[8][11];
	P[8][12] = P[12][8] =
	    (F[8][9] * D[9][12] + F[8][10] * D[10][12] +
	     F[8][11] * D[11][12] + F[8][12] * D[12][12] +
	     F[8][6] * D[6][12] + F[8][7] * D[7][12]) * T + D[8][12];
	P[9][9] =
	    (Q[0] * G[9][0] * G[9][0] + Q[1] * G[9][1] * G[9][1] +
	     Q[2] * G[9][2] * G[9][2] + F[9][10] * (F[9][10] * D[10][10] +
						    F[9][11] * D[10][11] +
						    F[9][12] * D[10][12] +
						    F[9][6] * D[6][10] +
						    F[9][7] * D[7][10] +
						    F[9][8] * D[8][10]) +
	     F[9][11] * (F[9][10] * D[10][11] + F[9][11] * D[11][11] +
			 F[9][12] * D[11][12] + F[9][6] * D[6][11] +
			 F[9][7] * D[7][11] + F[9][8] * D[8][11]) +
	     F[9][12] * (F[9][10] * D[10][12] + F[9][11] * D[11][12] +
			 F[9][12] * D[12][12] + F[9][6] * D[6][12] +
			 F[9][7] * D[7][12] + F[9][8] * D[8][12]) +
	     F[9][6] * (F[9][6] * D[6][6] + F[9][7] * D[6][7] +
			F[9][8] * D[6][8] + F[9][10] * D[6][10] +
			F[9][11] * D[6][11] + F[9][12] * D[6][12]) +
	     F[9][7] * (F[9][6] * D[6][7] + F[9][7] * D[7][7] +
			F[9][8] * D[7][8] + F[9][10] * D[7][10] +
			F[9][11] * D[7][11] + F[9][12] * D[7][12]) +
	     F[9][8] * (F[9][6] * D[6][8] + F[9][7] * D[7][8] +
			F[9][8] * D[8][8] + F[9][10] * D[8][10] +
			F[9][11] * D[8][11] + F[9][12] * D[8][12])) * Tsq +
	    (2 * F[9][10] * D[9][10] + 2 * F[9][11] * D[9][11] +
	     2 * F[9][12] * D[9][12] + 2 * F[9][6] * D[6][9] +
	     2 * F[9][7] * D[7][9] + 2 * F[9][8] * D[8][9]) * T + D[9][9];
	P[9][10] = P[10][9] =
	    (F[9][10] * D[10][10] + F[9][11] * D[10][11] +
	     F[9][12] * D[10][12] + F[9][6] * D[6][10] +
	     F[9][7] * D[7][10] + F[9][8] * D[8][10]) * T + D[9][10];
	P[9][11] = P[11][9] =
	    (F[9][10] * D[10][11] + F[9][11] * D[11][11] +
	     F[9][12] * D[11][12] + F[9][6] * D[6][11] +
	     F[9][7] * D[7][11] + F[9][8] * D[8][11]) * T + D[9][11];
	P[9][12] = P[12][9] =
	    (F[9][10] * D[10][12] + F[9][11] * D[11][12] +
	     F[9][12] * D[12][12] + F[9][6] * D[6][12] +
	     F[9][7] * D[7][12] + F[9][8] * D[8][12]) * T + D[9][12];
	P[10][10] = Q[6] * Tsq + D[10][10];
	P[10][11] = P[11][10] = D[10][11];
	P[10][12] = P[12][10] = D[10][12];
	P[11][11] = Q[7] * Tsq + D[11][11];
	P[11][12] = P[12][11] = D[11][12];
	P[12][12] = Q[8] * Tsq + D[12][12];
}
#endif

//  *************  SerialUpdate *******************
//  Does the update step of the Kalman filter for the covariance and estimate
//  Outputs are Xnew & Pnew, and are written over P and X
//  Z is actual measurement, Y is predicted measurement
//  Xnew = X + K*(Z-Y), Pnew=(I-K*H)*P,
//    where K=P*H'*inv[H*P*H'+R]
//  NOTE the algorithm assumes R (measurement covariance matrix) is diagonal
//    i.e. the measurment noises are uncorrelated.
//  It therefore uses a serial update that requires no matrix inversion by
//    processing the measurements one at a time.
//  Algorithm - see Grewal and Andrews, "Kalman Filtering,2nd Ed" p.121 & p.253
//            - or see Simon, "Optimal State Estimation," 1st Ed, p.150
//  The SensorsUsed variable is a bitwise mask indicating which sensors
//     should be used in the update.
//  ************************************************

void SerialUpdate(float H[NUMV][NUMX], float R[NUMV], float Z[NUMV],
		  float Y[NUMV], float P[NUMX][NUMX], float X[NUMX],
		  uint16_t SensorsUsed)
{
	float HP[NUMX], HPHR, Error;
	uint8_t i, j, k, m;

	for (m = 0; m < NUMV; m++) {

		if (SensorsUsed & (0x01 << m)) {	// use this sensor for update

			for (j = 0; j < NUMX; j++) {	// Find Hp = H*P
				HP[j] = 0;
				for (k = 0; k < NUMX; k++)
					HP[j] += H[m][k] * P[k][j];
			}
			HPHR = R[m];	// Find  HPHR = H*P*H' + R
			for (k = 0; k < NUMX; k++)
				HPHR += HP[k] * H[m][k];

			for (k = 0; k < NUMX; k++)
				K[k][m] = HP[k] / HPHR;	// find K = HP/HPHR

			for (i = 0; i < NUMX; i++) {	// Find P(m)= P(m-1) + K*HP
				for (j = i; j < NUMX; j++)
					P[i][j] = P[j][i] =
					    P[i][j] - K[i][m] * HP[j];
			}

			Error = Z[m] - Y[m];
			for (i = 0; i < NUMX; i++)	// Find X(m)= X(m-1) + K*Error
				X[i] = X[i] + K[i][m] * Error;

		}
	}
}

//  *************  RungeKutta **********************
//  Does a 4th order Runge Kutta numerical integration step
//  Output, Xnew, is written over X
//  NOTE the algorithm assumes time invariant state equations and
//    constant inputs over integration step
//  ************************************************

void RungeKutta(float X[NUMX], float U[NUMU], float dT)
{

	float dT2 =
	    dT / 2, K1[NUMX], K2[NUMX], K3[NUMX], K4[NUMX], Xlast[NUMX];
	uint8_t i;

	for (i = 0; i < NUMX; i++)
		Xlast[i] = X[i];	// make a working copy

	StateEq(X, U, K1);	// k1 = f(x,u)
	for (i = 0; i < NUMX; i++)
		X[i] = Xlast[i] + dT2 * K1[i];
	StateEq(X, U, K2);	// k2 = f(x+0.5*dT*k1,u)
	for (i = 0; i < NUMX; i++)
		X[i] = Xlast[i] + dT2 * K2[i];
	StateEq(X, U, K3);	// k3 = f(x+0.5*dT*k2,u)
	for (i = 0; i < NUMX; i++)
		X[i] = Xlast[i] + dT * K3[i];
	StateEq(X, U, K4);	// k4 = f(x+dT*k3,u)

	// Xnew  = X + dT*(k1+2*k2+2*k3+k4)/6
	for (i = 0; i < NUMX; i++)
		X[i] =
		    Xlast[i] + dT * (K1[i] + 2 * K2[i] + 2 * K3[i] +
				     K4[i]) / 6;
}

//  *************  Model Specific Stuff  ***************************
//  ***  StateEq, MeasurementEq, LinerizeFG, and LinearizeH ********
//
//  State Variables = [Pos Vel Quaternion GyroBias NO-AccelBias]
//  Deterministic Inputs = [AngularVel Accel]
//  Disturbance Noise = [GyroNoise AccelNoise GyroRandomWalkNoise NO-AccelRandomWalkNoise]
//
//  Measurement Variables = [Pos Vel BodyFrameMagField Altimeter]
//  Inputs to Measurement = [EarthFrameMagField]
//
//  Notes: Pos and Vel in earth frame
//  AngularVel and Accel in body frame
//  MagFields are unit vectors
//  Xdot is output of StateEq()
//  F and G are outputs of LinearizeFG(), all elements not set should be zero
//  y is output of OutputEq()
//  H is output of LinearizeH(), all elements not set should be zero
//  ************************************************

void StateEq(float X[NUMX], float U[NUMU], float Xdot[NUMX])
{
	float ax, ay, az, wx, wy, wz, q0, q1, q2, q3;

	// ax=U[3]-X[13]; ay=U[4]-X[14]; az=U[5]-X[15];  // subtract the biases on accels
	ax = U[3];
	ay = U[4];
	az = U[5];		// NO BIAS STATES ON ACCELS
	wx = U[0] - X[10];
	wy = U[1] - X[11];
	wz = U[2] - X[12];	// subtract the biases on gyros
	q0 = X[6];
	q1 = X[7];
	q2 = X[8];
	q3 = X[9];

	// Pdot = V
	Xdot[0] = X[3];
	Xdot[1] = X[4];
	Xdot[2] = X[5];

	// Vdot = Reb*a
	Xdot[3] =
	    (q0 * q0 + q1 * q1 - q2 * q2 - q3 * q3) * ax + 2 * (q1 * q2 -
								q0 * q3) *
	    ay + 2 * (q1 * q3 + q0 * q2) * az;
	Xdot[4] =
	    2 * (q1 * q2 + q0 * q3) * ax + (q0 * q0 - q1 * q1 + q2 * q2 -
					    q3 * q3) * ay + 2 * (q2 * q3 -
								 q0 * q1) *
	    az;
	Xdot[5] =
	    2 * (q1 * q3 - q0 * q2) * ax + 2 * (q2 * q3 + q0 * q1) * ay +
	    (q0 * q0 - q1 * q1 - q2 * q2 + q3 * q3) * az + 9.81;

	// qdot = Q*w
	Xdot[6] = (-q1 * wx - q2 * wy - q3 * wz) / 2;
	Xdot[7] = (q0 * wx - q3 * wy + q2 * wz) / 2;
	Xdot[8] = (q3 * wx + q0 * wy - q1 * wz) / 2;
	Xdot[9] = (-q2 * wx + q1 * wy + q0 * wz) / 2;

	// best guess is that bias stays constant
	Xdot[10] = Xdot[11] = Xdot[12] = 0;
}

void LinearizeFG(float X[NUMX], float U[NUMU], float F[NUMX][NUMX],
		 float G[NUMX][NUMW])
{
	float ax, ay, az, wx, wy, wz, q0, q1, q2, q3;

	// ax=U[3]-X[13]; ay=U[4]-X[14]; az=U[5]-X[15];  // subtract the biases on accels
	ax = U[3];
	ay = U[4];
	az = U[5];		// NO BIAS STATES ON ACCELS
	wx = U[0] - X[10];
	wy = U[1] - X[11];
	wz = U[2] - X[12];	// subtract the biases on gyros
	q0 = X[6];
	q1 = X[7];
	q2 = X[8];
	q3 = X[9];

	// Pdot = V
	F[0][3] = F[1][4] = F[2][5] = 1;

	// dVdot/dq
	F[3][6] = 2 * (q0 * ax - q3 * ay + q2 * az);
	F[3][7] = 2 * (q1 * ax + q2 * ay + q3 * az);
	F[3][8] = 2 * (-q2 * ax + q1 * ay + q0 * az);
	F[3][9] = 2 * (-q3 * ax - q0 * ay + q1 * az);
	F[4][6] = 2 * (q3 * ax + q0 * ay - q1 * az);
	F[4][7] = 2 * (q2 * ax - q1 * ay - q0 * az);
	F[4][8] = 2 * (q1 * ax + q2 * ay + q3 * az);
	F[4][9] = 2 * (q0 * ax - q3 * ay + q2 * az);
	F[5][6] = 2 * (-q2 * ax + q1 * ay + q0 * az);
	F[5][7] = 2 * (q3 * ax + q0 * ay - q1 * az);
	F[5][8] = 2 * (-q0 * ax + q3 * ay - q2 * az);
	F[5][9] = 2 * (q1 * ax + q2 * ay + q3 * az);

	// dVdot/dabias & dVdot/dna  - NO BIAS STATES ON ACCELS - S0 REPEAT FOR G BELOW
	// F[3][13]=G[3][3]=-q0*q0-q1*q1+q2*q2+q3*q3; F[3][14]=G[3][4]=2*(-q1*q2+q0*q3);         F[3][15]=G[3][5]=-2*(q1*q3+q0*q2);
	// F[4][13]=G[4][3]=-2*(q1*q2+q0*q3);         F[4][14]=G[4][4]=-q0*q0+q1*q1-q2*q2+q3*q3; F[4][15]=G[4][5]=2*(-q2*q3+q0*q1);
	// F[5][13]=G[5][3]=2*(-q1*q3+q0*q2);         F[5][14]=G[5][4]=-2*(q2*q3+q0*q1);         F[5][15]=G[5][5]=-q0*q0+q1*q1+q2*q2-q3*q3;

	// dqdot/dq
	F[6][6] = 0;
	F[6][7] = -wx / 2;
	F[6][8] = -wy / 2;
	F[6][9] = -wz / 2;
	F[7][6] = wx / 2;
	F[7][7] = 0;
	F[7][8] = wz / 2;
	F[7][9] = -wy / 2;
	F[8][6] = wy / 2;
	F[8][7] = -wz / 2;
	F[8][8] = 0;
	F[8][9] = wx / 2;
	F[9][6] = wz / 2;
	F[9][7] = wy / 2;
	F[9][8] = -wx / 2;
	F[9][9] = 0;

	// dqdot/dwbias
	F[6][10] = q1 / 2;
	F[6][11] = q2 / 2;
	F[6][12] = q3 / 2;
	F[7][10] = -q0 / 2;
	F[7][11] = q3 / 2;
	F[7][12] = -q2 / 2;
	F[8][10] = -q3 / 2;
	F[8][11] = -q0 / 2;
	F[8][12] = q1 / 2;
	F[9][10] = q2 / 2;
	F[9][11] = -q1 / 2;
	F[9][12] = -q0 / 2;

	// dVdot/dna  - NO BIAS STATES ON ACCELS - S0 REPEAT FOR G HERE
	G[3][3] = -q0 * q0 - q1 * q1 + q2 * q2 + q3 * q3;
	G[3][4] = 2 * (-q1 * q2 + q0 * q3);
	G[3][5] = -2 * (q1 * q3 + q0 * q2);
	G[4][3] = -2 * (q1 * q2 + q0 * q3);
	G[4][4] = -q0 * q0 + q1 * q1 - q2 * q2 + q3 * q3;
	G[4][5] = 2 * (-q2 * q3 + q0 * q1);
	G[5][3] = 2 * (-q1 * q3 + q0 * q2);
	G[5][4] = -2 * (q2 * q3 + q0 * q1);
	G[5][5] = -q0 * q0 + q1 * q1 + q2 * q2 - q3 * q3;

	// dqdot/dnw
	G[6][0] = q1 / 2;
	G[6][1] = q2 / 2;
	G[6][2] = q3 / 2;
	G[7][0] = -q0 / 2;
	G[7][1] = q3 / 2;
	G[7][2] = -q2 / 2;
	G[8][0] = -q3 / 2;
	G[8][1] = -q0 / 2;
	G[8][2] = q1 / 2;
	G[9][0] = q2 / 2;
	G[9][1] = -q1 / 2;
	G[9][2] = -q0 / 2;

	// dwbias = random walk noise
	G[10][6] = G[11][7] = G[12][8] = 1;
	// dabias = random walk noise
	// G[13][9]=G[14][10]=G[15][11]=1;  // NO BIAS STATES ON ACCELS
}

void MeasurementEq(float X[NUMX], float Be[3], float Y[NUMV])
{
	float q0, q1, q2, q3;

	q0 = X[6];
	q1 = X[7];
	q2 = X[8];
	q3 = X[9];

	// first six outputs are P and V
	Y[0] = X[0];
	Y[1] = X[1];
	Y[2] = X[2];
	Y[3] = X[3];
	Y[4] = X[4];
	Y[5] = X[5];

	// Bb=Rbe*Be
	Y[6] =
	    (q0 * q0 + q1 * q1 - q2 * q2 - q3 * q3) * Be[0] +
	    2 * (q1 * q2 + q0 * q3) * Be[1] + 2 * (q1 * q3 -
						   q0 * q2) * Be[2];
	Y[7] =
	    2 * (q1 * q2 - q0 * q3) * Be[0] + (q0 * q0 - q1 * q1 +
					       q2 * q2 - q3 * q3) * Be[1] +
	    2 * (q2 * q3 + q0 * q1) * Be[2];
	Y[8] =
	    2 * (q1 * q3 + q0 * q2) * Be[0] + 2 * (q2 * q3 -
						   q0 * q1) * Be[1] +
	    (q0 * q0 - q1 * q1 - q2 * q2 + q3 * q3) * Be[2];

	// Alt = -Pz
	Y[9] = -X[2];
}

void LinearizeH(float X[NUMX], float Be[3], float H[NUMV][NUMX])
{
	float q0, q1, q2, q3;

	q0 = X[6];
	q1 = X[7];
	q2 = X[8];
	q3 = X[9];

	// dP/dP=I;
	H[0][0] = H[1][1] = H[2][2] = 1;
	// dV/dV=I;
	H[3][3] = H[4][4] = H[5][5] = 1;

	// dBb/dq
	H[6][6] = 2 * (q0 * Be[0] + q3 * Be[1] - q2 * Be[2]);
	H[6][7] = 2 * (q1 * Be[0] + q2 * Be[1] + q3 * Be[2]);
	H[6][8] = 2 * (-q2 * Be[0] + q1 * Be[1] - q0 * Be[2]);
	H[6][9] = 2 * (-q3 * Be[0] + q0 * Be[1] + q1 * Be[2]);
	H[7][6] = 2 * (-q3 * Be[0] + q0 * Be[1] + q1 * Be[2]);
	H[7][7] = 2 * (q2 * Be[0] - q1 * Be[1] + q0 * Be[2]);
	H[7][8] = 2 * (q1 * Be[0] + q2 * Be[1] + q3 * Be[2]);
	H[7][9] = 2 * (-q0 * Be[0] - q3 * Be[1] + q2 * Be[2]);
	H[8][6] = 2 * (q2 * Be[0] - q1 * Be[1] + q0 * Be[2]);
	H[8][7] = 2 * (q3 * Be[0] - q0 * Be[1] - q1 * Be[2]);
	H[8][8] = 2 * (q0 * Be[0] + q3 * Be[1] - q2 * Be[2]);
	H[8][9] = 2 * (q1 * Be[0] + q2 * Be[1] + q3 * Be[2]);

	// dAlt/dPz = -1
	H[9][2] = -1;
}

/**
 * @}
 * @}
 */

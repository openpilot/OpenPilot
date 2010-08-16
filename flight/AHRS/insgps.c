/**
 ******************************************************************************
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
#define NUMX 13  // number of states, X is the state vector
#define NUMW 9   // number of plant noise inputs, w is disturbance noise vector
#define NUMV 10  // number of measurements, v is the measurement noise vector
#define NUMU 6   // number of deterministic inputs, U is the input vector

// Private functions
void CovariancePrediction(float F[NUMX][NUMX], float G[NUMX][NUMW], float Q[NUMW], float dT, float P[NUMX][NUMX]);
void SerialUpdate(float H[NUMV][NUMX], float R[NUMV], float Z[NUMV], float Y[NUMV],
				  float P[NUMX][NUMX], float X[NUMX], uint16_t SensorsUsed);
void RungeKutta(float X[NUMX],float U[NUMU], float dT);
void StateEq(float X[NUMX],float U[NUMU],float Xdot[NUMX]);
void LinearizeFG(float X[NUMX],float U[NUMU], float F[NUMX][NUMX], float G[NUMX][NUMW]);
void MeasurementEq(float X[NUMX], float Be[3], float Y[NUMV]);
void LinearizeH(float X[NUMX], float Be[3], float H[NUMV][NUMX]);

// Private variables
float F[NUMX][NUMX], G[NUMX][NUMW], H[NUMV][NUMX];  // linearized system matrices
													// global to init to zero and maintain zero elements
float Be[3];                                        // local magnetic unit vector in NED frame
float P[NUMX][NUMX], X[NUMX];                       // covariance matrix and state vector
float Q[NUMW], R[NUMV];                             // input noise and measurement noise variances


//  *************  Exposed Functions ****************
//  *************************************************
void INSGPSInit()   //pretty much just a place holder for now
{
	Be[0]=1; Be[1]=0; Be[2]=0;  // local magnetic unit vector

	P[0][0]=P[1][1]=P[2][2]=25;             // initial position variance (m^2)
	P[3][3]=P[4][4]=P[5][5]=5;              // initial velocity variance (m/s)^2
	P[6][6]=P[7][7]=P[8][8]=P[9][9]=1e-5;   // initial quaternion variance
	P[10][10]=P[11][11]=P[12][12]=1e-5;     // initial gyro bias variance (rad/s)^2

	X[0]=X[1]=X[2]=X[3]=X[4]=X[5]=0;    // initial pos and vel (m)
	X[6]=1; X[7]=X[8]=X[9]=0;           // initial quaternion (level and North) (m/s)
	X[10]=X[11]=X[12]=0;                // initial gyro bias (rad/s)

	Q[0]=Q[1]=Q[2]=50e-8;           // gyro noise variance (rad/s)^2
	Q[3]=Q[4]=Q[5]=0.01;            // accelerometer noise variance (m/s^2)^2
	Q[6]=Q[7]=Q[8]=2e-5;            // gyro bias random walk variance (rad/s^2)^2

	R[0]=R[1]=0.004;       // High freq GPS horizontal position noise variance (m^2)
	R[2]=0.036;            // High freq GPS vertical position noise variance (m^2)
	R[3]=R[4]=0.004;       // High freq GPS horizontal velocity noise variance (m/s)^2
	R[5]=0;                // High freq GPS vertical velocity noise variance (m/s)^2
	R[6]=R[7]=R[8]=0.005;  // magnetometer unit vector noise variance
	R[9]=1;                // High freq altimeter noise variance (m^2)
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


void INSPrediction(float gyro_data[3], float accel_data[3], float dT)
{
	float U[6];
	float qmag;

	// rate gyro inputs in units of rad/s
	U[0]=gyro_data[0];
	U[1]=gyro_data[1];
	U[2]=gyro_data[2];

	// accelerometer inputs in units of m/s
	U[3]=accel_data[0];
	U[4]=accel_data[1];
	U[5]=accel_data[2];

    // EKF prediction step
	LinearizeFG(X,U,F,G);
    RungeKutta(X,U,dT);
    qmag=sqrt(X[6]*X[6] + X[7]*X[7] + X[8]*X[8] + X[9]*X[9]);
    X[6] /= qmag; X[7] /= qmag; X[8] /= qmag; X[9] /= qmag;
    CovariancePrediction(F,G,Q,dT,P);

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

void MagCorrection(float mag_data[3])
{
    float Z[10], Y[10];
    float Bmag, qmag;

	// magnetometer data in any units (use unit vector) and in body frame
	Bmag = sqrt(mag_data[0]*mag_data[0] + mag_data[1]*mag_data[1] + mag_data[2]*mag_data[2]);
	Z[6] = mag_data[0]/Bmag;
	Z[7] = mag_data[1]/Bmag;
	Z[8] = mag_data[2]/Bmag;

    // EKF correction step
	LinearizeH(X,Be,H);
	MeasurementEq(X,Be,Y);
	SerialUpdate(H,R,Z,Y,P,X,MagSensors);
	qmag=sqrt(X[6]*X[6] + X[7]*X[7] + X[8]*X[8] + X[9]*X[9]);
	X[6] /= qmag; X[7] /= qmag; X[8] /= qmag; X[9] /= qmag;

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

void FullCorrection(float mag_data[3], float Pos[3], float Vel[3], float BaroAlt)
{
    float Z[10], Y[10];
    float Bmag, qmag;

	// GPS Position in meters and in local NED frame
	Z[0]=Pos[0];
	Z[1]=Pos[1];
	Z[2]=Pos[2];

	// GPS Velocity in meters and in local NED frame
	Z[3]=Vel[0];
	Z[4]=Vel[1];
	Z[5]=Vel[2];

	// magnetometer data in any units (use unit vector) and in body frame
	Bmag = sqrt(mag_data[0]*mag_data[0] + mag_data[1]*mag_data[1] + mag_data[2]*mag_data[2]);
	Z[6] = mag_data[0]/Bmag;
	Z[7] = mag_data[1]/Bmag;
	Z[8] = mag_data[2]/Bmag;

	// barometric altimeter in meters and in local NED frame
	Z[9] = BaroAlt;

	// EKF correction step
	LinearizeH(X,Be,H);
	MeasurementEq(X,Be,Y);
	SerialUpdate(H,R,Z,Y,P,X,FullSensors);
	qmag=sqrt(X[6]*X[6] + X[7]*X[7] + X[8]*X[8] + X[9]*X[9]);
	X[6] /= qmag; X[7] /= qmag; X[8] /= qmag; X[9] /= qmag;

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

void GndSpeedAndMagCorrection(float Speed, float Heading, float mag_data[3])
{
    float Z[10], Y[10];
    float Bmag, qmag;

	// Ground Speed in m/s and Heading in rad
    Z[3] = Speed*cos((double)Heading);
    Z[4] = Speed*sin((double)Heading);

    // magnetometer data in any units (use unit vector) and in body frame
	Bmag = sqrt(mag_data[0]*mag_data[0] + mag_data[1]*mag_data[1] + mag_data[2]*mag_data[2]);
	Z[6] = mag_data[0]/Bmag;
	Z[7] = mag_data[1]/Bmag;
	Z[8] = mag_data[2]/Bmag;

    // EKF correction step
	LinearizeH(X,Be,H);
	MeasurementEq(X,Be,Y);
	SerialUpdate(H,R,Z,Y,P,X,GndSpeedAndMagSensors);
	qmag=sqrt(X[6]*X[6] + X[7]*X[7] + X[8]*X[8] + X[9]*X[9]);
	X[6] /= qmag; X[7] /= qmag; X[8] /= qmag; X[9] /= qmag;

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
//  Could be much more efficient using the sparse, block structure of F and G
//  ************************************************

void CovariancePrediction(float F[NUMX][NUMX], float G[NUMX][NUMW], float Q[NUMW], float dT, float P[NUMX][NUMX]){
 float Dummy[NUMX][NUMX], dTsq;
 uint8_t i,j,k;

 //  Pnew = (I+F*T)*P*(I+F*T)' + T^2*G*Q*G' = T^2[(P/T + F*P)*(I/T + F') + G*Q*G')]

   dTsq = dT*dT;

   for (i=0;i<NUMX;i++)                   // Calculate Dummy = (P/T +F*P)
    for (j=0;j<NUMX;j++){
      Dummy[i][j] = P[i][j]/dT;
      for (k=0;k<NUMX;k++) Dummy[i][j] += F[i][k]*P[k][j];
    }
   for (i=0;i<NUMX;i++)                  // Calculate Pnew = Dummy/T + Dummy*F' + G*Qw*G'
    for (j=i;j<NUMX;j++){                // Use symmetry, ie only find upper triangular
      P[i][j] = Dummy[i][j]/dT;
      for (k=0;k<NUMX;k++) P[i][j] += Dummy[i][k]*F[j][k];   // P = Dummy/T + Dummy*F'
      for (k=0;k<NUMW;k++) P[i][j] += Q[k]*G[i][k]*G[j][k];  // P = Dummy/T + Dummy*F' + G*Q*G'
      P[j][i] = P[i][j] = P[i][j]*dTsq;                      // Pnew = T^2*P and fill in lower triangular;
    }
}

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

void SerialUpdate(float H[NUMV][NUMX], float R[NUMV], float Z[NUMV], float Y[NUMV],
				  float P[NUMX][NUMX], float X[NUMX], uint16_t SensorsUsed){
 float HP[NUMX], K[NUMX], HPHR, Error;
 uint8_t i,j,k,m;

  for (m=0;m<NUMV;m++){

	  if ( SensorsUsed & (0x01<<m)){        // use this sensor for update

			for (j=0;j<NUMX;j++){                   // Find Hp = H*P
			  HP[j]=0;
			  for (k=0;k<NUMX;k++) HP[j] += H[m][k]*P[k][j];
			}
			HPHR = R[m];                            // Find  HPHR = H*P*H' + R
			for (k=0;k<NUMX;k++) HPHR += HP[k]*H[m][k];

			for (k=0;k<NUMX;k++) K[k] = HP[k]/HPHR; // find K = HP/HPHR

			for (i=0;i<NUMX;i++){                   // Find P(m)= P(m-1) + K*HP
			  for (j=i;j<NUMX;j++) P[i][j]=P[j][i] = P[i][j] - K[i]*HP[j];
			}

			Error = Z[m]-Y[m];
			for (i=0;i<NUMX;i++)               // Find X(m)= X(m-1) + K*Error
			  X[i] = X[i] + K[i]*Error;

	  }
  }
}

//  *************  RungeKutta **********************
//  Does a 4th order Runge Kutta numerical integration step
//  Output, Xnew, is written over X
//  NOTE the algorithm assumes time invariant state equations and
//    constant inputs over integration step
//  ************************************************

void RungeKutta(float X[NUMX],float U[NUMU], float dT){

 float dT2=dT/2, K1[NUMX], K2[NUMX], K3[NUMX], K4[NUMX], Xlast[NUMX];
 uint8_t i;

  for (i=0;i<NUMX;i++) Xlast[i] = X[i];        // make a working copy

  StateEq(X,U,K1);                                     // k1 = f(x,u)
  for (i=0;i<NUMX;i++) X[i] = Xlast[i] + dT2*K1[i];
  StateEq(X,U,K2);                                     // k2 = f(x+0.5*dT*k1,u)
  for (i=0;i<NUMX;i++) X[i] = Xlast[i] + dT2*K2[i];
  StateEq(X,U,K3);                                     // k3 = f(x+0.5*dT*k2,u)
  for (i=0;i<NUMX;i++) X[i] = Xlast[i] + dT*K3[i];
  StateEq(X,U,K4);                                     // k4 = f(x+dT*k3,u)

  // Xnew  = X + dT*(k1+2*k2+2*k3+k4)/6
  for (i=0;i<NUMX;i++) X[i] = Xlast[i] + dT*(K1[i]+2*K2[i]+2*K3[i]+K4[i])/6;
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

void StateEq(float X[NUMX],float U[NUMU],float Xdot[NUMX]){
  float ax, ay, az, wx, wy, wz, q0, q1, q2, q3;

  // ax=U[3]-X[13]; ay=U[4]-X[14]; az=U[5]-X[15];  // subtract the biases on accels
  ax=U[3]; ay=U[4]; az=U[5];                    // NO BIAS STATES ON ACCELS
  wx=U[0]-X[10]; wy=U[1]-X[11]; wz=U[2]-X[12];  // subtract the biases on gyros
  q0=X[6]; q1=X[7]; q2=X[8]; q3=X[9];

  // Pdot = V
  Xdot[0]=X[3];  Xdot[1]=X[4]; Xdot[2]=X[5];

  // Vdot = Reb*a
  Xdot[3]=(q0*q0+q1*q1-q2*q2-q3*q3)*ax + 2*(q1*q2-q0*q3)*ay + 2*(q1*q3+q0*q2)*az;
  Xdot[4]=2*(q1*q2+q0*q3)*ax + (q0*q0-q1*q1+q2*q2-q3*q3)*ay + 2*(q2*q3-q0*q1)*az;
  Xdot[5]=2*(q1*q3-q0*q2)*ax + 2*(q2*q3+q0*q1)*ay + (q0*q0-q1*q1-q2*q2+q3*q3)*az + 9.81;

  // qdot = Q*w
  Xdot[6] = (-q1*wx-q2*wy-q3*wz)/2;
  Xdot[7] = (q0*wx-q3*wy+q2*wz)/2;
  Xdot[8] = (q3*wx+q0*wy-q1*wz)/2;
  Xdot[9] = (-q2*wx+q1*wy+q0*wz)/2;

  // best guess is that bias stays constant
  Xdot[10]=Xdot[11]=Xdot[12]=0;
}

void LinearizeFG(float X[NUMX],float U[NUMU], float F[NUMX][NUMX], float G[NUMX][NUMW]){
  float ax, ay, az, wx, wy, wz, q0, q1, q2, q3;

  // ax=U[3]-X[13]; ay=U[4]-X[14]; az=U[5]-X[15];  // subtract the biases on accels
  ax=U[3]; ay=U[4]; az=U[5];                    // NO BIAS STATES ON ACCELS
  wx=U[0]-X[10]; wy=U[1]-X[11]; wz=U[2]-X[12];  // subtract the biases on gyros
  q0=X[6]; q1=X[7]; q2=X[8]; q3=X[9];

  // Pdot = V
  F[0][3]=F[1][4]=F[2][5]=1;

  // dVdot/dq
  F[3][6]=2*(q0*ax-q3*ay+q2*az);   F[3][7]=2*(q1*ax+q2*ay+q3*az);   F[3][8]=2*(-q2*ax+q1*ay+q0*az);  F[3][9]=2*(-q3*ax-q0*ay+q1*az);
  F[4][6]=2*(q3*ax+q0*ay-q1*az);   F[4][7]=2*(q2*ax-q1*ay-q0*az);   F[4][8]=2*(q1*ax+q2*ay+q3*az);   F[4][9]=2*(q0*ax-q3*ay+q2*az);
  F[5][6]=2*(-q2*ax+q1*ay+q0*az);  F[5][7]=2*(q3*ax+q0*ay-q1*az);   F[5][8]=2*(-q0*ax+q3*ay-q2*az);  F[5][9]=2*(q1*ax+q2*ay+q3*az);

  // dVdot/dabias & dVdot/dna  - NO BIAS STATES ON ACCELS - S0 REPEAT FOR G BELOW
  // F[3][13]=G[3][3]=-q0*q0-q1*q1+q2*q2+q3*q3; F[3][14]=G[3][4]=2*(-q1*q2+q0*q3);         F[3][15]=G[3][5]=-2*(q1*q3+q0*q2);
  // F[4][13]=G[4][3]=-2*(q1*q2+q0*q3);         F[4][14]=G[4][4]=-q0*q0+q1*q1-q2*q2+q3*q3; F[4][15]=G[4][5]=2*(-q2*q3+q0*q1);
  // F[5][13]=G[5][3]=2*(-q1*q3+q0*q2);         F[5][14]=G[5][4]=-2*(q2*q3+q0*q1);         F[5][15]=G[5][5]=-q0*q0+q1*q1+q2*q2-q3*q3;

  // dqdot/dq
  F[6][6]=0;     F[6][7]=-wx/2; F[6][8]=-wy/2; F[6][9]=-wz/2;
  F[7][6]=wx/2;  F[7][7]=0;     F[7][8]=wz/2;  F[7][9]=-wy/2;
  F[8][6]=wy/2;  F[8][7]=-wz/2; F[8][8]=0;     F[8][9]=wx/2;
  F[9][6]=wz/2;  F[9][7]=wy/2;  F[9][8]=-wx/2; F[9][9]=0;

  // dqdot/dwbias
  F[6][10]=q1/2;  F[6][11]=q2/2;  F[6][12]=q3/2;
  F[7][10]=-q0/2; F[7][11]=q3/2;  F[7][12]=-q2/2;
  F[8][10]=-q3/2; F[8][11]=-q0/2; F[8][12]=q1/2;
  F[9][10]=q2/2;  F[9][11]=-q1/2; F[9][12]=-q0/2;

  // dVdot/dna  - NO BIAS STATES ON ACCELS - S0 REPEAT FOR G HERE
  G[3][3]=-q0*q0-q1*q1+q2*q2+q3*q3; G[3][4]=2*(-q1*q2+q0*q3);         G[3][5]=-2*(q1*q3+q0*q2);
  G[4][3]=-2*(q1*q2+q0*q3);         G[4][4]=-q0*q0+q1*q1-q2*q2+q3*q3; G[4][5]=2*(-q2*q3+q0*q1);
  G[5][3]=2*(-q1*q3+q0*q2);         G[5][4]=-2*(q2*q3+q0*q1);         G[5][5]=-q0*q0+q1*q1+q2*q2-q3*q3;

  // dqdot/dnw
  G[6][0]=q1/2;  G[6][1]=q2/2;  G[6][2]=q3/2;
  G[7][0]=-q0/2; G[7][1]=q3/2;  G[7][2]=-q2/2;
  G[8][0]=-q3/2; G[8][1]=-q0/2; G[8][2]=q1/2;
  G[9][0]=q2/2;  G[9][1]=-q1/2; G[9][2]=-q0/2;

  // dwbias = random walk noise
  G[10][6]=G[11][7]=G[12][8]=1;
  // dabias = random walk noise
  // G[13][9]=G[14][10]=G[15][11]=1;  // NO BIAS STATES ON ACCELS
}

void MeasurementEq(float X[NUMX], float Be[3], float Y[NUMV]){
  float q0, q1, q2, q3;

  q0=X[6]; q1=X[7]; q2=X[8]; q3=X[9];

  // first six outputs are P and V
  Y[0]=X[0]; Y[1]=X[1]; Y[2]=X[2]; Y[3]=X[3]; Y[4]=X[4]; Y[5]=X[5];

  // Bb=Rbe*Be
  Y[6]=(q0*q0+q1*q1-q2*q2-q3*q3)*Be[0] + 2*(q1*q2+q0*q3)*Be[1] + 2*(q1*q3-q0*q2)*Be[2];
  Y[7]=2*(q1*q2-q0*q3)*Be[0] + (q0*q0-q1*q1+q2*q2-q3*q3)*Be[1] + 2*(q2*q3+q0*q1)*Be[2];
  Y[8]=2*(q1*q3+q0*q2)*Be[0] + 2*(q2*q3-q0*q1)*Be[1] + (q0*q0-q1*q1-q2*q2+q3*q3)*Be[2];

  // Alt = -Pz
  Y[9] = -X[2];
}

void LinearizeH(float X[NUMX], float Be[3], float H[NUMV][NUMX]){
  float q0, q1, q2, q3;

  q0=X[6]; q1=X[7]; q2=X[8]; q3=X[9];

  // dP/dP=I;
  H[0][0]=H[1][1]=H[2][2]=1;
  // dV/dV=I;
  H[3][3]=H[4][4]=H[5][5]=1;

  // dBb/dq
  H[6][6]=2*(q0*Be[0]+q3*Be[1]-q2*Be[2]);  H[6][7]=2*(q1*Be[0]+q2*Be[1]+q3*Be[2]); H[6][8]=2*(-q2*Be[0]+q1*Be[1]-q0*Be[2]); H[6][9]=2*(-q3*Be[0]+q0*Be[1]+q1*Be[2]);
  H[7][6]=2*(-q3*Be[0]+q0*Be[1]+q1*Be[2]); H[7][7]=2*(q2*Be[0]-q1*Be[1]+q0*Be[2]); H[7][8]=2*(q1*Be[0]+q2*Be[1]+q3*Be[2]);  H[7][9]=2*(-q0*Be[0]-q3*Be[1]+q2*Be[2]);
  H[8][6]=2*(q2*Be[0]-q1*Be[1]+q0*Be[2]);  H[8][7]=2*(q3*Be[0]-q0*Be[1]-q1*Be[2]); H[8][8]=2*(q0*Be[0]+q3*Be[1]-q2*Be[2]);  H[8][9]=2*(q1*Be[0]+q2*Be[1]+q3*Be[2]);

  // dAlt/dPz = -1
  H[9][2]=-1;
}


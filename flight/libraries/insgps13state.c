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
#include <pios_math.h>

// constants/macros/typdefs
#define NUMX 13 // number of states, X is the state vector
#define NUMW 9 // number of plant noise inputs, w is disturbance noise vector
#define NUMV 10 // number of measurements, v is the measurement noise vector
#define NUMU 6 // number of deterministic inputs, U is the input vector

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

// speed optimizations, describe matrix sparsity
// derived from state equations in
// LinearizeFG() and LinearizeH():
//
// usage F:        usage G:   usage H:
// 0123456789abc  012345678  0123456789abc
// 0...X.........  .........  X............
// 1....X........  .........  .X...........
// 2.....X.......  .........  ..X..........
// 3......XXXX...  ...XXX...  ...X.........
// 4......XXXX...  ...XXX...  ....X........
// 5......XXXX...  ...XXX...  .....X.......
// 6.......XXXXXX  XXX......  ......XXXX...
// 7......X.XXXXX  XXX......  ......XXXX...
// 8......XX.XXXX  XXX......  ......XXXX...
// 9......XXX.XXX  XXX......  ..X..........
// a.............  ......X..
// b.............  .......X.
// c.............  ........X

static const int8_t FrowMin[NUMX] = { 3, 4, 5, 6, 6, 6, 7, 6, 6, 6, 13, 13, 13 };
static const int8_t FrowMax[NUMX] = { 3, 4, 5, 9, 9, 9, 12, 12, 12, 12, -1, -1, -1 };

static const int8_t GrowMin[NUMX] = { 9, 9, 9, 3, 3, 3, 0, 0, 0, 0, 6, 7, 8 };
static const int8_t GrowMax[NUMX] = { -1, -1, -1, 5, 5, 5, 2, 2, 2, 2, 6, 7, 8 };

static const int8_t HrowMin[NUMV] = { 0, 1, 2, 3, 4, 5, 6, 6, 6, 2 };
static const int8_t HrowMax[NUMV] = { 0, 1, 2, 3, 4, 5, 9, 9, 9, 2 };

static struct EKFData {
    // linearized system matrices
    float F[NUMX][NUMX];
    float G[NUMX][NUMW];
    float H[NUMV][NUMX];
    // local magnetic unit vector in NED frame
    float Be[3];
    // covariance matrix and state vector
    float P[NUMX][NUMX];
    float X[NUMX];
    // input noise and measurement noise variances
    float Q[NUMW];
    float R[NUMV];
} ekf;

// Global variables
struct NavStruct Nav;

// *************  Exposed Functions ****************
// *************************************************

uint16_t ins_get_num_states()
{
    return NUMX;
}

void INSGPSInit() // pretty much just a place holder for now
{
    ekf.Be[0] = 1.0f;
    ekf.Be[1] = 0.0f;
    ekf.Be[2] = 0.0f; // local magnetic unit vector

    for (int i = 0; i < NUMX; i++) {
        for (int j = 0; j < NUMX; j++) {
            ekf.P[i][j] = 0.0f; // zero all terms
            ekf.F[i][j] = 0.0f;
        }

        for (int j = 0; j < NUMW; j++) {
            ekf.G[i][j] = 0.0f;
        }

        for (int j = 0; j < NUMV; j++) {
            ekf.H[j][i] = 0.0f;
        }

        ekf.X[i] = 0.0f;
    }
    for (int i = 0; i < NUMW; i++) {
        ekf.Q[i] = 0.0f;
    }
    for (int i = 0; i < NUMV; i++) {
        ekf.R[i] = 0.0f;
    }


    ekf.P[0][0]   = ekf.P[1][1] = ekf.P[2][2] = 25.0f;            // initial position variance (m^2)
    ekf.P[3][3]   = ekf.P[4][4] = ekf.P[5][5] = 5.0f;             // initial velocity variance (m/s)^2
    ekf.P[6][6]   = ekf.P[7][7] = ekf.P[8][8] = ekf.P[9][9] = 1e-5f;  // initial quaternion variance
    ekf.P[10][10] = ekf.P[11][11] = ekf.P[12][12] = 1e-9f; // initial gyro bias variance (rad/s)^2

    ekf.X[0]  = ekf.X[1] = ekf.X[2] = ekf.X[3] = ekf.X[4] = ekf.X[5] = 0.0f; // initial pos and vel (m)
    ekf.X[6]  = 1.0f;
    ekf.X[7]  = ekf.X[8] = ekf.X[9] = 0.0f;      // initial quaternion (level and North) (m/s)
    ekf.X[10] = ekf.X[11] = ekf.X[12] = 0.0f; // initial gyro bias (rad/s)

    ekf.Q[0]  = ekf.Q[1] = ekf.Q[2] = 50e-4f;        // gyro noise variance (rad/s)^2
    ekf.Q[3]  = ekf.Q[4] = ekf.Q[5] = 0.00001f;      // accelerometer noise variance (m/s^2)^2
    ekf.Q[6]  = ekf.Q[7] = ekf.Q[8] = 2e-8f;     // gyro bias random walk variance (rad/s^2)^2

    ekf.R[0]  = ekf.R[1] = 0.004f;   // High freq GPS horizontal position noise variance (m^2)
    ekf.R[2]  = 0.036f;          // High freq GPS vertical position noise variance (m^2)
    ekf.R[3]  = ekf.R[4] = 0.004f;   // High freq GPS horizontal velocity noise variance (m/s)^2
    ekf.R[5]  = 100.0f;          // High freq GPS vertical velocity noise variance (m/s)^2
    ekf.R[6]  = ekf.R[7] = ekf.R[8] = 0.005f;    // magnetometer unit vector noise variance
    ekf.R[9]  = .25f;                    // High freq altimeter noise variance (m^2)
}

void INSResetP(float PDiag[NUMX])
{
    uint8_t i, j;

    // if PDiag[i] nonzero then clear row and column and set diagonal element
    for (i = 0; i < NUMX; i++) {
        if (PDiag != 0) {
            for (j = 0; j < NUMX; j++) {
                ekf.P[i][j] = ekf.P[j][i] = 0.0f;
            }
            ekf.P[i][i] = PDiag[i];
        }
    }
}

void INSGetP(float PDiag[NUMX])
{
    uint8_t i;

    // retrieve diagonal elements (aka state variance)
    for (i = 0; i < NUMX; i++) {
        if (PDiag != 0) {
            PDiag[i] = ekf.P[i][i];
        }
    }
}

void INSSetState(float pos[3], float vel[3], float q[4], float gyro_bias[3], __attribute__((unused)) float accel_bias[3])
{
    /* Note: accel_bias not used in 13 state INS */
    ekf.X[0]  = pos[0];
    ekf.X[1]  = pos[1];
    ekf.X[2]  = pos[2];
    ekf.X[3]  = vel[0];
    ekf.X[4]  = vel[1];
    ekf.X[5]  = vel[2];
    ekf.X[6]  = q[0];
    ekf.X[7]  = q[1];
    ekf.X[8]  = q[2];
    ekf.X[9]  = q[3];
    ekf.X[10] = gyro_bias[0];
    ekf.X[11] = gyro_bias[1];
    ekf.X[12] = gyro_bias[2];
}

void INSPosVelReset(float pos[3], float vel[3])
{
    for (int i = 0; i < 6; i++) {
        for (int j = i; j < NUMX; j++) {
            ekf.P[i][j] = 0; // zero the first 6 rows and columns
            ekf.P[j][i] = 0;
        }
    }

    ekf.P[0][0] = ekf.P[1][1] = ekf.P[2][2] = 25; // initial position variance (m^2)
    ekf.P[3][3] = ekf.P[4][4] = ekf.P[5][5] = 5; // initial velocity variance (m/s)^2

    ekf.X[0]    = pos[0];
    ekf.X[1]    = pos[1];
    ekf.X[2]    = pos[2];
    ekf.X[3]    = vel[0];
    ekf.X[4]    = vel[1];
    ekf.X[5]    = vel[2];
}

void INSSetPosVelVar(float PosVar[3], float VelVar[3])
{
    ekf.R[0] = PosVar[0];
    ekf.R[1] = PosVar[1];
    ekf.R[2] = PosVar[2];
    ekf.R[3] = VelVar[0];
    ekf.R[4] = VelVar[1];
    ekf.R[5] = VelVar[2];
}

void INSSetGyroBias(float gyro_bias[3])
{
    ekf.X[10] = gyro_bias[0];
    ekf.X[11] = gyro_bias[1];
    ekf.X[12] = gyro_bias[2];
}

void INSSetAccelVar(float accel_var[3])
{
    ekf.Q[3] = accel_var[0];
    ekf.Q[4] = accel_var[1];
    ekf.Q[5] = accel_var[2];
}

void INSSetGyroVar(float gyro_var[3])
{
    ekf.Q[0] = gyro_var[0];
    ekf.Q[1] = gyro_var[1];
    ekf.Q[2] = gyro_var[2];
}

void INSSetGyroBiasVar(float gyro_bias_var[3])
{
    ekf.Q[6] = gyro_bias_var[0];
    ekf.Q[7] = gyro_bias_var[1];
    ekf.Q[8] = gyro_bias_var[2];
}

void INSSetMagVar(float scaled_mag_var[3])
{
    ekf.R[6] = scaled_mag_var[0];
    ekf.R[7] = scaled_mag_var[1];
    ekf.R[8] = scaled_mag_var[2];
}

void INSSetBaroVar(float baro_var)
{
    ekf.R[9] = baro_var;
}

void INSSetMagNorth(float B[3])
{
    float mag = sqrtf(B[0] * B[0] + B[1] * B[1] + B[2] * B[2]);

    ekf.Be[0] = B[0] / mag;
    ekf.Be[1] = B[1] / mag;
    ekf.Be[2] = B[2] / mag;
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
    LinearizeFG(ekf.X, U, ekf.F, ekf.G);
    RungeKutta(ekf.X, U, dT);
    qmag      = sqrtf(ekf.X[6] * ekf.X[6] + ekf.X[7] * ekf.X[7] + ekf.X[8] * ekf.X[8] + ekf.X[9] * ekf.X[9]);
    ekf.X[6] /= qmag;
    ekf.X[7] /= qmag;
    ekf.X[8] /= qmag;
    ekf.X[9] /= qmag;
    // CovariancePrediction(ekf.F,ekf.G,ekf.Q,dT,ekf.P);

    // Update Nav solution structure
    Nav.Pos[0] = ekf.X[0];
    Nav.Pos[1] = ekf.X[1];
    Nav.Pos[2] = ekf.X[2];
    Nav.Vel[0] = ekf.X[3];
    Nav.Vel[1] = ekf.X[4];
    Nav.Vel[2] = ekf.X[5];
    Nav.q[0]   = ekf.X[6];
    Nav.q[1]   = ekf.X[7];
    Nav.q[2]   = ekf.X[8];
    Nav.q[3]   = ekf.X[9];
    Nav.gyro_bias[0] = ekf.X[10];
    Nav.gyro_bias[1] = ekf.X[11];
    Nav.gyro_bias[2] = ekf.X[12];
}

void INSCovariancePrediction(float dT)
{
    CovariancePrediction(ekf.F, ekf.G, ekf.Q, dT, ekf.P);
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
    LinearizeH(ekf.X, ekf.Be, ekf.H);
    MeasurementEq(ekf.X, ekf.Be, Y);
    SerialUpdate(ekf.H, ekf.R, Z, Y, ekf.P, ekf.X, SensorsUsed);
    qmag       = sqrtf(ekf.X[6] * ekf.X[6] + ekf.X[7] * ekf.X[7] + ekf.X[8] * ekf.X[8] + ekf.X[9] * ekf.X[9]);
    ekf.X[6]  /= qmag;
    ekf.X[7]  /= qmag;
    ekf.X[8]  /= qmag;
    ekf.X[9]  /= qmag;

    // Update Nav solution structure
    Nav.Pos[0] = ekf.X[0];
    Nav.Pos[1] = ekf.X[1];
    Nav.Pos[2] = ekf.X[2];
    Nav.Vel[0] = ekf.X[3];
    Nav.Vel[1] = ekf.X[4];
    Nav.Vel[2] = ekf.X[5];
    Nav.q[0]   = ekf.X[6];
    Nav.q[1]   = ekf.X[7];
    Nav.q[2]   = ekf.X[8];
    Nav.q[3]   = ekf.X[9];
    Nav.gyro_bias[0] = ekf.X[10];
    Nav.gyro_bias[1] = ekf.X[11];
    Nav.gyro_bias[2] = ekf.X[12];
}

// *************  CovariancePrediction *************
// Does the prediction step of the Kalman filter for the covariance matrix
// Output, Pnew, overwrites P, the input covariance
// Pnew = (I+F*T)*P*(I+F*T)' + T^2*G*Q*G'
// Q is the discrete time covariance of process noise
// Q is vector of the diagonal for a square matrix with
// dimensions equal to the number of disturbance noise variables
// The General Method is very inefficient,not taking advantage of the sparse F and G
// The first Method is very specific to this implementation
// ************************************************

__attribute__((optimize("O3")))
void CovariancePrediction(float F[NUMX][NUMX], float G[NUMX][NUMW],
                          float Q[NUMW], float dT, float P[NUMX][NUMX])
{
    // Pnew = (I+F*T)*P*(I+F*T)' + (T^2)*G*Q*G' = (T^2)[(P/T + F*P)*(I/T + F') + G*Q*G')]

    float dT1  = 1.0f / dT; // multiplication is faster than division on fpu.
    float dTsq = dT * dT;

    float Dummy[NUMX][NUMX];
    int8_t i;

    for (i = 0; i < NUMX; i++) { // Calculate Dummy = (P/T +F*P)
        float *Firow   = F[i];
        float *Pirow   = P[i];
        float *Dirow   = Dummy[i];
        int8_t Fistart = FrowMin[i];
        int8_t Fiend   = FrowMax[i];
        int8_t j;
        for (j = 0; j < NUMX; j++) {
            Dirow[j] = Pirow[j] * dT1; // Dummy = P / T ...
            int8_t k;
            for (k = Fistart; k <= Fiend; k++) {
                Dirow[j] += Firow[k] * P[k][j]; // [] + F * P
            }
        }
    }
    for (i = 0; i < NUMX; i++) { // Calculate Pnew = (T^2) [Dummy/T + Dummy*F' + G*Qw*G']
        float *Dirow   = Dummy[i];
        float *Girow   = G[i];
        float *Pirow   = P[i];
        int8_t Gistart = GrowMin[i];
        int8_t Giend   = GrowMax[i];
        int8_t j;
        for (j = i; j < NUMX; j++) { // Use symmetry, ie only find upper triangular
            float Ptmp = Dirow[j] * dT1; // Pnew = Dummy / T ...

            {
                float *Fjrow   = F[j];
                int8_t Fjstart = FrowMin[j];
                int8_t Fjend   = FrowMax[j];
                int8_t k;
                for (k = Fjstart; k <= Fjend; k++) {
                    Ptmp += Dirow[k] * Fjrow[k]; // [] + Dummy*F' ...
                }
            }

            {
                float *Gjrow   = G[j];
                int8_t Gjstart = MAX(Gistart, GrowMin[j]);
                int8_t Gjend   = MIN(Giend, GrowMax[j]);
                int8_t k;
                for (k = Gjstart; k <= Gjend; k++) {
                    Ptmp += Q[k] * Girow[k] * Gjrow[k]; // [] + G*Q*G' ...
                }
            }

            P[j][i] = Pirow[j] = Ptmp * dTsq; // [] * (T^2)
        }
    }
}

// *************  SerialUpdate *******************
// Does the update step of the Kalman filter for the covariance and estimate
// Outputs are Xnew & Pnew, and are written over P and X
// Z is actual measurement, Y is predicted measurement
// Xnew = X + K*(Z-Y), Pnew=(I-K*H)*P,
// where K=P*H'*inv[H*P*H'+R]
// NOTE the algorithm assumes R (measurement covariance matrix) is diagonal
// i.e. the measurment noises are uncorrelated.
// It therefore uses a serial update that requires no matrix inversion by
// processing the measurements one at a time.
// Algorithm - see Grewal and Andrews, "Kalman Filtering,2nd Ed" p.121 & p.253
// - or see Simon, "Optimal State Estimation," 1st Ed, p.150
// The SensorsUsed variable is a bitwise mask indicating which sensors
// should be used in the update.
// ************************************************

void SerialUpdate(float H[NUMV][NUMX], float R[NUMV], float Z[NUMV],
                  float Y[NUMV], float P[NUMX][NUMX], float X[NUMX],
                  uint16_t SensorsUsed)
{
    float HP[NUMX], HPHR, Error;
    uint8_t i, j, k, m;
    float Km[NUMX];

    for (m = 0; m < NUMV; m++) {
        if (SensorsUsed & (0x01 << m)) { // use this sensor for update
            for (j = 0; j < NUMX; j++) { // Find Hp = H*P
                HP[j] = 0;
                for (k = HrowMin[m]; k <= HrowMax[m]; k++) {
                    HP[j] += H[m][k] * P[k][j];
                }
            }
            HPHR = R[m]; // Find  HPHR = H*P*H' + R
            for (k = HrowMin[m]; k <= HrowMax[m]; k++) {
                HPHR += HP[k] * H[m][k];
            }

            for (k = 0; k < NUMX; k++) {
                Km[k] = HP[k] / HPHR; // find K = HP/HPHR
            }
            for (i = 0; i < NUMX; i++) { // Find P(m)= P(m-1) + K*HP
                for (j = i; j < NUMX; j++) {
                    P[i][j] = P[j][i] =
                                  P[i][j] - Km[i] * HP[j];
                }
            }

            Error = Z[m] - Y[m];
            for (i = 0; i < NUMX; i++) { // Find X(m)= X(m-1) + K*Error
                X[i] = X[i] + Km[i] * Error;
            }
        }
    }
}

// *************  RungeKutta **********************
// Does a 4th order Runge Kutta numerical integration step
// Output, Xnew, is written over X
// NOTE the algorithm assumes time invariant state equations and
// constant inputs over integration step
// ************************************************

void RungeKutta(float X[NUMX], float U[NUMU], float dT)
{
    float dT2 =
        dT / 2.0f, K1[NUMX], K2[NUMX], K3[NUMX], K4[NUMX], Xlast[NUMX];
    uint8_t i;

    for (i = 0; i < NUMX; i++) {
        Xlast[i] = X[i]; // make a working copy
    }
    StateEq(X, U, K1); // k1 = f(x,u)
    for (i = 0; i < NUMX; i++) {
        X[i] = Xlast[i] + dT2 * K1[i];
    }
    StateEq(X, U, K2); // k2 = f(x+0.5*dT*k1,u)
    for (i = 0; i < NUMX; i++) {
        X[i] = Xlast[i] + dT2 * K2[i];
    }
    StateEq(X, U, K3); // k3 = f(x+0.5*dT*k2,u)
    for (i = 0; i < NUMX; i++) {
        X[i] = Xlast[i] + dT * K3[i];
    }
    StateEq(X, U, K4); // k4 = f(x+dT*k3,u)

    // Xnew  = X + dT*(k1+2*k2+2*k3+k4)/6
    for (i = 0; i < NUMX; i++) {
        X[i] =
            Xlast[i] + dT * (K1[i] + 2.0f * K2[i] + 2.0f * K3[i] +
                             K4[i]) / 6.0f;
    }
}

// *************  Model Specific Stuff  ***************************
// ***  StateEq, MeasurementEq, LinerizeFG, and LinearizeH ********
//
// State Variables = [Pos Vel Quaternion GyroBias NO-AccelBias]
// Deterministic Inputs = [AngularVel Accel]
// Disturbance Noise = [GyroNoise AccelNoise GyroRandomWalkNoise NO-AccelRandomWalkNoise]
//
// Measurement Variables = [Pos Vel BodyFrameMagField Altimeter]
// Inputs to Measurement = [EarthFrameMagField]
//
// Notes: Pos and Vel in earth frame
// AngularVel and Accel in body frame
// MagFields are unit vectors
// Xdot is output of StateEq()
// F and G are outputs of LinearizeFG(), all elements not set should be zero
// y is output of OutputEq()
// H is output of LinearizeH(), all elements not set should be zero
// ************************************************

void StateEq(float X[NUMX], float U[NUMU], float Xdot[NUMX])
{
    float ax, ay, az, wx, wy, wz, q0, q1, q2, q3;

    // ax=U[3]-X[13]; ay=U[4]-X[14]; az=U[5]-X[15];  // subtract the biases on accels
    ax = U[3];
    ay = U[4];
    az = U[5]; // NO BIAS STATES ON ACCELS
    wx = U[0] - X[10];
    wy = U[1] - X[11];
    wz = U[2] - X[12]; // subtract the biases on gyros
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
        (q0 * q0 + q1 * q1 - q2 * q2 - q3 * q3) * ax + 2.0f * (q1 * q2 -
                                                               q0 * q3) *
        ay + 2.0f * (q1 * q3 + q0 * q2) * az;
    Xdot[4] =
        2.0f * (q1 * q2 + q0 * q3) * ax + (q0 * q0 - q1 * q1 + q2 * q2 -
                                           q3 * q3) * ay + 2 * (q2 * q3 -
                                                                q0 * q1) *
        az;
    Xdot[5] =
        2.0f * (q1 * q3 - q0 * q2) * ax + 2 * (q2 * q3 + q0 * q1) * ay +
        (q0 * q0 - q1 * q1 - q2 * q2 + q3 * q3) * az + 9.81f;

    // qdot = Q*w
    Xdot[6]  = (-q1 * wx - q2 * wy - q3 * wz) / 2.0f;
    Xdot[7]  = (q0 * wx - q3 * wy + q2 * wz) / 2.0f;
    Xdot[8]  = (q3 * wx + q0 * wy - q1 * wz) / 2.0f;
    Xdot[9]  = (-q2 * wx + q1 * wy + q0 * wz) / 2.0f;

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
    az = U[5]; // NO BIAS STATES ON ACCELS
    wx = U[0] - X[10];
    wy = U[1] - X[11];
    wz = U[2] - X[12]; // subtract the biases on gyros
    q0 = X[6];
    q1 = X[7];
    q2 = X[8];
    q3 = X[9];

    // Pdot = V
    F[0][3] = F[1][4] = F[2][5] = 1.0f;

    // dVdot/dq
    F[3][6] = 2.0f * (q0 * ax - q3 * ay + q2 * az);
    F[3][7] = 2.0f * (q1 * ax + q2 * ay + q3 * az);
    F[3][8] = 2.0f * (-q2 * ax + q1 * ay + q0 * az);
    F[3][9] = 2.0f * (-q3 * ax - q0 * ay + q1 * az);
    F[4][6] = 2.0f * (q3 * ax + q0 * ay - q1 * az);
    F[4][7] = 2.0f * (q2 * ax - q1 * ay - q0 * az);
    F[4][8] = 2.0f * (q1 * ax + q2 * ay + q3 * az);
    F[4][9] = 2.0f * (q0 * ax - q3 * ay + q2 * az);
    F[5][6] = 2.0f * (-q2 * ax + q1 * ay + q0 * az);
    F[5][7] = 2.0f * (q3 * ax + q0 * ay - q1 * az);
    F[5][8] = 2.0f * (-q0 * ax + q3 * ay - q2 * az);
    F[5][9] = 2.0f * (q1 * ax + q2 * ay + q3 * az);

    // dVdot/dabias & dVdot/dna  - NO BIAS STATES ON ACCELS - S0 REPEAT FOR G BELOW
    // F[3][13]=G[3][3]=-q0*q0-q1*q1+q2*q2+q3*q3; F[3][14]=G[3][4]=2*(-q1*q2+q0*q3);         F[3][15]=G[3][5]=-2*(q1*q3+q0*q2);
    // F[4][13]=G[4][3]=-2*(q1*q2+q0*q3);         F[4][14]=G[4][4]=-q0*q0+q1*q1-q2*q2+q3*q3; F[4][15]=G[4][5]=2*(-q2*q3+q0*q1);
    // F[5][13]=G[5][3]=2*(-q1*q3+q0*q2);         F[5][14]=G[5][4]=-2*(q2*q3+q0*q1);         F[5][15]=G[5][5]=-q0*q0+q1*q1+q2*q2-q3*q3;

    // dqdot/dq
    F[6][6]  = 0;
    F[6][7]  = -wx / 2.0f;
    F[6][8]  = -wy / 2.0f;
    F[6][9]  = -wz / 2.0f;
    F[7][6]  = wx / 2.0f;
    F[7][7]  = 0;
    F[7][8]  = wz / 2.0f;
    F[7][9]  = -wy / 2.0f;
    F[8][6]  = wy / 2.0f;
    F[8][7]  = -wz / 2.0f;
    F[8][8]  = 0;
    F[8][9]  = wx / 2.0f;
    F[9][6]  = wz / 2.0f;
    F[9][7]  = wy / 2.0f;
    F[9][8]  = -wx / 2.0f;
    F[9][9]  = 0;

    // dqdot/dwbias
    F[6][10] = q1 / 2.0f;
    F[6][11] = q2 / 2.0f;
    F[6][12] = q3 / 2.0f;
    F[7][10] = -q0 / 2.0f;
    F[7][11] = q3 / 2.0f;
    F[7][12] = -q2 / 2.0f;
    F[8][10] = -q3 / 2.0f;
    F[8][11] = -q0 / 2.0f;
    F[8][12] = q1 / 2.0f;
    F[9][10] = q2 / 2.0f;
    F[9][11] = -q1 / 2.0f;
    F[9][12] = -q0 / 2.0f;

    // dVdot/dna  - NO BIAS STATES ON ACCELS - S0 REPEAT FOR G HERE
    G[3][3]  = -q0 * q0 - q1 * q1 + q2 * q2 + q3 * q3;
    G[3][4]  = 2.0f * (-q1 * q2 + q0 * q3);
    G[3][5]  = -2.0f * (q1 * q3 + q0 * q2);
    G[4][3]  = -2.0f * (q1 * q2 + q0 * q3);
    G[4][4]  = -q0 * q0 + q1 * q1 - q2 * q2 + q3 * q3;
    G[4][5]  = 2.0f * (-q2 * q3 + q0 * q1);
    G[5][3]  = 2.0f * (-q1 * q3 + q0 * q2);
    G[5][4]  = -2.0f * (q2 * q3 + q0 * q1);
    G[5][5]  = -q0 * q0 + q1 * q1 + q2 * q2 - q3 * q3;

    // dqdot/dnw
    G[6][0]  = q1 / 2.0f;
    G[6][1]  = q2 / 2.0f;
    G[6][2]  = q3 / 2.0f;
    G[7][0]  = -q0 / 2.0f;
    G[7][1]  = q3 / 2.0f;
    G[7][2]  = -q2 / 2.0f;
    G[8][0]  = -q3 / 2.0f;
    G[8][1]  = -q0 / 2.0f;
    G[8][2]  = q1 / 2.0f;
    G[9][0]  = q2 / 2.0f;
    G[9][1]  = -q1 / 2.0f;
    G[9][2]  = -q0 / 2.0f;

    // dwbias = random walk noise
    G[10][6] = G[11][7] = G[12][8] = 1.0f;
    // dabias = random walk noise
    // G[13][9]=G[14][10]=G[15][11]=1;  // NO BIAS STATES ON ACCELS
}

void MeasurementEq(float X[NUMX], float Be[3], float Y[NUMV])
{
    float q0, q1, q2, q3;

    q0   = X[6];
    q1   = X[7];
    q2   = X[8];
    q3   = X[9];

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
        2.0f * (q1 * q2 + q0 * q3) * Be[1] + 2.0f * (q1 * q3 -
                                                     q0 * q2) * Be[2];
    Y[7] =
        2.0f * (q1 * q2 - q0 * q3) * Be[0] + (q0 * q0 - q1 * q1 +
                                              q2 * q2 - q3 * q3) * Be[1] +
        2.0f * (q2 * q3 + q0 * q1) * Be[2];
    Y[8] =
        2.0f * (q1 * q3 + q0 * q2) * Be[0] + 2.0f * (q2 * q3 -
                                                     q0 * q1) * Be[1] +
        (q0 * q0 - q1 * q1 - q2 * q2 + q3 * q3) * Be[2];

    // Alt = -Pz
    Y[9] = -1.0f * X[2];
}

void LinearizeH(float X[NUMX], float Be[3], float H[NUMV][NUMX])
{
    float q0, q1, q2, q3;

    q0 = X[6];
    q1 = X[7];
    q2 = X[8];
    q3 = X[9];

    // dP/dP=I;
    H[0][0] = H[1][1] = H[2][2] = 1.0f;
    // dV/dV=I;
    H[3][3] = H[4][4] = H[5][5] = 1.0f;

    // dBb/dq
    H[6][6] = 2.0f * (q0 * Be[0] + q3 * Be[1] - q2 * Be[2]);
    H[6][7] = 2.0f * (q1 * Be[0] + q2 * Be[1] + q3 * Be[2]);
    H[6][8] = 2.0f * (-q2 * Be[0] + q1 * Be[1] - q0 * Be[2]);
    H[6][9] = 2.0f * (-q3 * Be[0] + q0 * Be[1] + q1 * Be[2]);
    H[7][6] = 2.0f * (-q3 * Be[0] + q0 * Be[1] + q1 * Be[2]);
    H[7][7] = 2.0f * (q2 * Be[0] - q1 * Be[1] + q0 * Be[2]);
    H[7][8] = 2.0f * (q1 * Be[0] + q2 * Be[1] + q3 * Be[2]);
    H[7][9] = 2.0f * (-q0 * Be[0] - q3 * Be[1] + q2 * Be[2]);
    H[8][6] = 2.0f * (q2 * Be[0] - q1 * Be[1] + q0 * Be[2]);
    H[8][7] = 2.0f * (q3 * Be[0] - q0 * Be[1] - q1 * Be[2]);
    H[8][8] = 2.0f * (q0 * Be[0] + q3 * Be[1] - q2 * Be[2]);
    H[8][9] = 2.0f * (q1 * Be[0] + q2 * Be[1] + q3 * Be[2]);

    // dAlt/dPz = -1
    H[9][2] = -1.0f;
}

/**
 * @}
 * @}
 */

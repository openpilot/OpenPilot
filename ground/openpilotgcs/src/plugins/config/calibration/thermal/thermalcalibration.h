/**
 ******************************************************************************
 *
 * @file       thermalcalibration.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2014.
 *
 * @brief      Barometer, Gyroscope and Accelerometer thermal calibration algorithms
 * @see        The GNU Public License (GPL) Version 3
 * @defgroup
 * @{
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
#ifndef THERMALCALIBRATION_H
#define THERMALCALIBRATION_H
#include "../calibrationutils.h"

namespace OpenPilot {
class ThermalCalibration {
    static const int GYRO_X_POLY_DEGREE  = 1;
    static const int GYRO_Y_POLY_DEGREE  = 1;
    static const int GYRO_Z_POLY_DEGREE  = 2;

    static const int ACCEL_X_POLY_DEGREE = 1;
    static const int ACCEL_Y_POLY_DEGREE = 1;
    static const int ACCEL_Z_POLY_DEGREE = 1;

    static const int BARO_PRESSURE_POLY_DEGREE = 3;
    // TODO: determine max allowable relative error
    static const double BARO_PRESSURE_MAX_REL_ERROR = 1E-6f;
    static const double ACCEL_X_MAX_REL_ERROR  = 1E-6f;
    static const double ACCEL_Y_MAX_REL_ERROR  = 1E-6f;
    static const double ACCEL_Z_MAX_REL_ERROR  = 1E-6f;
    static const double GYRO_X_MAX_REL_ERROR   = 1E-6f;
    static const double GYRO_Y_MAX_REL_ERROR   = 1E-6f;
    static const double GYRO_Z_MAX_REL_ERROR   = 1E-6f;
public:

    /**
     * @brief ComputeStats
     * @param samplesX X values for input samples
     * @param samplesY Y values for input samples
     * @param correctionPoly coefficients for the correction polynomial
     * @param degrees Degree of the correction polynomial
     * @param initialSigma Standard deviation calculated over input samples
     * @param rebiasedSigma Standard deviation calculated over calibrated samples
     */
    static void ComputeStats(Eigen::VectorXf *samplesX, Eigen::VectorXf *samplesY, Eigen::VectorXf *correctionPoly, float *initialSigma, float *rebiasedSigma);

    /**
     * @brief produce the calibration polinomial coefficients from pressure and temperature samples
     * @param pressure Pressure samples
     * @param temperature Temperature samples
     * @param result Polinomial coefficients to be sent to board (x0, x1, x2, x3)
     * @param inputSigma a float populated with input sample variance
     * @param CalibratedSigma float populated with calibrated data variance
     * @return
     */
    static bool BarometerCalibration(Eigen::VectorXf pressure, Eigen::VectorXf temperature, float *result, float *inputSigma, float *calibratedSigma);

    /**
     * @brief AccelerometerCalibration produce the calibration polinomial coefficients from accelerometer axis and temperature samples
     * @param pressure
     * @param temperature
     * @param result a float[3] array containing value to populate calibration settings (x,y,z)
     * @param inputSigma a float[3] array populated with input sample variance
     * @param CalibratedSigma float[3] array populated with calibrated data variance
     * @return
     */
    static bool AccelerometerCalibration(Eigen::VectorXf samplesX, Eigen::VectorXf samplesY, Eigen::VectorXf samplesZ, Eigen::VectorXf temperature, float *result, float *inputSigma, float *calibratedSigma);

    /**
     * @brief GyroscopeCalibration produce the calibration polinomial coefficients from gyroscopes axis and temperature samples
     * @param samplesX
     * @param samplesY
     * @param samplesZ
     * @param temperature
     * @param result a float[4] array containing value to populate calibration settings (x,y,z1, z2)
     * @param inputSigma a float[3] array populated with input sample variance
     * @param CalibratedSigma float[3] array populated with calibrated data variance
     * @return
     */
    static bool GyroscopeCalibration(Eigen::VectorXf samplesX, Eigen::VectorXf samplesY, Eigen::VectorXf samplesZ, Eigen::VectorXf temperature, float *result, float *inputSigma, float *calibratedSigma);


private:
    static void copyToArray(float *result, Eigen::VectorXf solution, int elements);
    ThermalCalibration();
    static int searchReferenceValue(float value, Eigen::VectorXf values);
};
}
#endif // THERMALCALIBRATION_H

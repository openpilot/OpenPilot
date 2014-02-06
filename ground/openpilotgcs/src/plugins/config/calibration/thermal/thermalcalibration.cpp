/**
 ******************************************************************************
 *
 * @file       thermalcalibration.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2014.
 *
 * @brief      Thermal calibration algorithms
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
#include "QDebug"
#include "thermalcalibration.h"
using namespace OpenPilot;

void ThermalCalibration::ComputeStats(Eigen::VectorXf *samplesX, Eigen::VectorXf *samplesY, Eigen::VectorXf *correctionPoly, float *initialSigma, float *rebiasedSigma)
{
    *initialSigma = CalibrationUtils::ComputeSigma(samplesY);
    Eigen::VectorXf bias(samplesX->rows());
    OpenPilot::CalibrationUtils::ComputePoly(samplesX, correctionPoly, &bias);
    Eigen::VectorXf rebiasedY(*samplesY);
    rebiasedY.array() -= bias.array();
    *rebiasedSigma     = CalibrationUtils::ComputeSigma(&rebiasedY);
}

bool ThermalCalibration::BarometerCalibration(Eigen::VectorXf pressure, Eigen::VectorXf temperature, float *result, float *inputSigma, float *calibratedSigma)
{
    // assume the nearest reading to 20°C as the "zero bias" point
    int index20deg = searchReferenceValue(20.0f, temperature);

    qDebug() << "Ref zero is " << index20deg << " T: " << temperature[index20deg] << " P:" << pressure[index20deg];
    float refZero  = pressure[index20deg];
    pressure.array() -= refZero;
    qDebug() << "Rebiased zero is " << pressure[index20deg];

    Eigen::VectorXf solution(BARO_PRESSURE_POLY_DEGREE + 1);
    if (!CalibrationUtils::PolynomialCalibration(&temperature, &pressure, BARO_PRESSURE_POLY_DEGREE, solution, BARO_PRESSURE_MAX_REL_ERROR)) {
        return false;
    }
    copyToArray(result, solution, BARO_PRESSURE_POLY_DEGREE + 1);
    ComputeStats(&temperature, &pressure, &solution, inputSigma, calibratedSigma);
    return (*calibratedSigma) < (*inputSigma);
}

bool ThermalCalibration::AccelerometerCalibration(Eigen::VectorXf samplesX, Eigen::VectorXf samplesY, Eigen::VectorXf samplesZ, Eigen::VectorXf temperature, float *result, float *inputSigma, float *calibratedSigma)
{
    Eigen::VectorXf solution(ACCEL_X_POLY_DEGREE + 1);

    if (!CalibrationUtils::PolynomialCalibration(&temperature, &samplesX, ACCEL_X_POLY_DEGREE, solution, ACCEL_X_MAX_REL_ERROR)) {
        return false;
    }
    result[0]   = solution[1];

    solution[0] = 0;
    ComputeStats(&temperature, &samplesX, &solution, &inputSigma[0], &calibratedSigma[0]);

    solution.resize(ACCEL_Y_POLY_DEGREE + 1);
    if (!CalibrationUtils::PolynomialCalibration(&temperature, &samplesY, ACCEL_Y_POLY_DEGREE, solution, ACCEL_Y_MAX_REL_ERROR)) {
        return false;
    }
    result[1]   = solution[1];

    solution[0] = 0;
    ComputeStats(&temperature, &samplesY, &solution, &inputSigma[1], &calibratedSigma[1]);

    solution.resize(ACCEL_Z_POLY_DEGREE + 1);
    if (!CalibrationUtils::PolynomialCalibration(&temperature, &samplesZ, ACCEL_Z_POLY_DEGREE, solution, ACCEL_Z_MAX_REL_ERROR)) {
        return false;
    }
    result[2]   = solution[1];

    solution[0] = 0;
    ComputeStats(&temperature, &samplesZ, &solution, &inputSigma[2], &calibratedSigma[2]);
    return (inputSigma[0] > calibratedSigma[0]) && (inputSigma[1] > calibratedSigma[1]) && (inputSigma[2] > calibratedSigma[2]);
}


bool ThermalCalibration::GyroscopeCalibration(Eigen::VectorXf samplesX, Eigen::VectorXf samplesY, Eigen::VectorXf samplesZ, Eigen::VectorXf temperature, float *result, float *inputSigma, float *calibratedSigma)
{
    Eigen::VectorXf solution(GYRO_X_POLY_DEGREE + 1);

    if (!CalibrationUtils::PolynomialCalibration(&temperature, &samplesX, GYRO_X_POLY_DEGREE, solution, GYRO_X_MAX_REL_ERROR)) {
        return false;
    }

    result[0]   = solution[1];
    result[1]   = solution[2];
    solution[0] = 0;
    ComputeStats(&temperature, &samplesX, &solution, &inputSigma[0], &calibratedSigma[0]);


    solution.resize(GYRO_Y_POLY_DEGREE + 1);
    if (!CalibrationUtils::PolynomialCalibration(&temperature, &samplesY, GYRO_Y_POLY_DEGREE, solution, GYRO_Y_MAX_REL_ERROR)) {
        return false;
    }
    result[2]   = solution[1];
    result[3]   = solution[2];
    solution[0] = 0;
    ComputeStats(&temperature, &samplesY, &solution, &inputSigma[1], &calibratedSigma[1]);

    solution.resize(GYRO_Z_POLY_DEGREE + 1);
    if (!CalibrationUtils::PolynomialCalibration(&temperature, &samplesZ, GYRO_Z_POLY_DEGREE, solution, GYRO_Z_MAX_REL_ERROR)) {
        return false;
    }
    result[4]   = solution[1];
    result[5]   = solution[2];
    solution[0] = 0;
    ComputeStats(&temperature, &samplesZ, &solution, &inputSigma[2], &calibratedSigma[2]);
    return (inputSigma[0] > calibratedSigma[0]) && (inputSigma[1] > calibratedSigma[1]) && (inputSigma[2] > calibratedSigma[2]);
}

void ThermalCalibration::copyToArray(float *result, Eigen::VectorXf solution, int elements)
{
    for (int i = 0; i < elements; i++) {
        result[i] = solution[i];
    }
}

int ThermalCalibration::searchReferenceValue(float value, Eigen::VectorXf values)
{
    for (int i = 0; i < values.size(); i++) {
        if (!(values[i] < value)) {
            return i;
        }
    }
    return values.size() - 1;
}

ThermalCalibration::ThermalCalibration()
{}

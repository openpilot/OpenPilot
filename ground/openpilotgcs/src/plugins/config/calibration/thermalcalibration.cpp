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
#include "thermalcalibration.h"
#include "QDebug"
using namespace OpenPilot;
bool ThermalCalibration::BarometerCalibration(Eigen::VectorXf pressure, Eigen::VectorXf temperature, float *result)
{
    // assume the nearest reading to 20°C as the "zero bias" point
    int index20deg = searchReferenceValue(20.0f, temperature);
    qDebug() << "Ref zero is " << index20deg << " T: " << temperature[index20deg] << " P:" << pressure[index20deg];
    float refZero = pressure[index20deg];
    pressure.array() -= refZero;
    qDebug() << "Rebased zero is " << pressure[index20deg];


    Eigen::VectorXf solution(BARO_PRESSURE_POLY_DEGREE + 1);
    if(!CalibrationUtils::PolynomialCalibration(temperature, pressure, BARO_PRESSURE_POLY_DEGREE, solution)){
        return false;
    }

    copyToArray(result, solution, BARO_PRESSURE_POLY_DEGREE + 1);
    return true;
}

bool ThermalCalibration::AccelerometerCalibration(Eigen::VectorXf samplesX, Eigen::VectorXf samplesY, Eigen::VectorXf samplesZ, Eigen::VectorXf temperature, float *result)
{
    Eigen::VectorXf solution(ACCEL_X_POLY_DEGREE + 1);
    if(!CalibrationUtils::PolynomialCalibration(temperature, samplesX, ACCEL_X_POLY_DEGREE, solution)){
        return false;
    }
    result[0] = solution[1];

    solution.resize(ACCEL_Y_POLY_DEGREE + 1);
    if(!CalibrationUtils::PolynomialCalibration(temperature, samplesY, ACCEL_Y_POLY_DEGREE, solution)){
        return false;
    }
    result[1] = solution[1];

    solution.resize(ACCEL_Z_POLY_DEGREE + 1);
    if(!CalibrationUtils::PolynomialCalibration(temperature, samplesZ, ACCEL_Z_POLY_DEGREE, solution)){
        return false;
    }
    result[2] = solution[1];

    return true;
}

bool ThermalCalibration::GyroscopeCalibration(Eigen::VectorXf samplesX, Eigen::VectorXf samplesY, Eigen::VectorXf samplesZ, Eigen::VectorXf temperature, float *result)
{
    Eigen::VectorXf solution(GYRO_X_POLY_DEGREE + 1);
    if(!CalibrationUtils::PolynomialCalibration(temperature, samplesX, GYRO_X_POLY_DEGREE, solution)){
        return false;
    }
    result[0] = solution[1];
    std::cout << solution << std::endl << std::endl;

    solution.resize(GYRO_Y_POLY_DEGREE + 1);
    if(!CalibrationUtils::PolynomialCalibration(temperature, samplesY, GYRO_Y_POLY_DEGREE, solution)){
        return false;
    }
    result[1] = solution[1];
    std::cout << solution << std::endl << std::endl;

    solution.resize(GYRO_Z_POLY_DEGREE + 1);
    if(!CalibrationUtils::PolynomialCalibration(temperature, samplesZ, GYRO_Z_POLY_DEGREE, solution)){
        return false;
    }
    result[2] = solution[1];
    result[3] = solution[2];
    std::cout << solution << std::endl;

    return true;
}

void ThermalCalibration::copyToArray(float *result, Eigen::VectorXf solution, int elements)
{
    for(int i = 0; i < elements; i++){
        result[i] = solution[i];
    }
}

int ThermalCalibration::searchReferenceValue(float value, Eigen::VectorXf values){
    for(int i = 0; i < values.size(); i++){
        if(!(values[i] < value)){
            return i;
        }
    }
    return values.size() - 1;
}

ThermalCalibration::ThermalCalibration()
{
}

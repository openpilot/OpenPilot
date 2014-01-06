/**
 ******************************************************************************
 *
 * @file       calibrationutils.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2013.
 *
 * @brief      Utilities for calibration using ellipsoid fit algorithm
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

#ifndef CALIBRATIONUTILS_H
#define CALIBRATIONUTILS_H
#include <Eigen/Core>
#include <Eigen/Eigenvalues>
#include <Eigen/Dense>
#include <iostream>

namespace OpenPilot{

class CalibrationUtils
{
public:
    CalibrationUtils();
    bool Calibrate();
    Eigen::VectorXf samples_x;
    Eigen::VectorXf samples_y;
    Eigen::VectorXf samples_z;
    Eigen::Matrix3f CalibrationMatrix;
    Eigen::Vector3f Scale;
    Eigen::Vector3f Bias;
    float NominalRange;
private:
    void EllipsoidFit(Eigen::VectorXf *samplesX, Eigen::VectorXf *samplesY, Eigen::VectorXf *samplesZ,
                      Eigen::Vector3f *center,
                      Eigen::VectorXf *radii,
                      Eigen::MatrixXf *evecs);
};

}
#endif // CALIBRATIONUTILS_H

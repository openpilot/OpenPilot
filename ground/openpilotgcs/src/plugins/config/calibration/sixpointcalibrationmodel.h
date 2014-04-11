/**
 ******************************************************************************
 *
 * @file       sixpointcalibrationmodel.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2014.
 *
 * @brief      Six point calibration for Magnetometer and Accelerometer
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
#ifndef SIXPOINTCALIBRATIONMODEL_H
#define SIXPOINTCALIBRATIONMODEL_H
#include <QMutex>
#include <QObject>
#include <QList>
#include "calibration/calibrationutils.h"

#include <revocalibration.h>
#include <accelgyrosettings.h>
#include <homelocation.h>
#include <accelstate.h>
#include <magstate.h>
namespace OpenPilot {
class SixPointCalibrationModel : public QObject {
    Q_OBJECT

    typedef struct {
        RevoCalibration::DataFields   revoCalibration;
        AccelGyroSettings::DataFields accelGyroSettings;
    } SavedSettings;
public:
    explicit SixPointCalibrationModel(QObject *parent = 0);

signals:
    void displayVisualHelp(QString elementID);
    void displayInstructions(QString instructions, bool replace);
    void disableAllCalibrations();
    void enableAllCalibrations();
    void storeAndClearBoardRotation();
    void recallBoardRotation();
    void savePositionEnabledChanged(bool state);

public slots:
    // Slots for calibrating the mags
    void magStart();
    void accelStart();
    void savePositionData();
private slots:
    void getSample(UAVObject *obj);
private:
    void start(bool calibrateAccel, bool calibrateMag);
    UAVObjectManager *getObjectManager();


    UAVObject::Metadata initialAccelStateMdata;
    UAVObject::Metadata initialMagStateMdata;
    float initialMagCorrectionRate;
    SavedSettings savedSettings;

    int position;

    bool calibratingMag;
    bool calibratingAccel;

    double accel_data_x[6], accel_data_y[6], accel_data_z[6];
    double mag_data_x[6], mag_data_y[6], mag_data_z[6];

    QMutex sensorsUpdateLock;

    // ! Computes the scale and bias of the mag based on collected data
    void compute(bool mag, bool accel);

    bool collectingData;
    QList<double> accel_accum_x;
    QList<double> accel_accum_y;
    QList<double> accel_accum_z;
    QList<double> mag_accum_x;
    QList<double> mag_accum_y;
    QList<double> mag_accum_z;
    void showHelp(QString image);
};
}
#endif // SIXPOINTCALIBRATIONMODEL_H

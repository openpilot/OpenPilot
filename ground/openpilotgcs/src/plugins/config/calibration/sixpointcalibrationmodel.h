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

#include "wizardmodel.h"
#include "calibration/calibrationutils.h"
#include <revocalibration.h>

#include <auxmagsettings.h>
#include <accelgyrosettings.h>
#include <homelocation.h>
#include <accelstate.h>
#include <magsensor.h>
#include <auxmagsensor.h>

#include <QMutex>
#include <QObject>
#include <QList>
#include <QString>

namespace OpenPilot {
class SixPointCalibrationModel : public QObject {
    Q_OBJECT

public:
    explicit SixPointCalibrationModel(QObject *parent = 0);

    bool dirty()
    {
        return m_dirty;
    }

signals:
    void started();
    void stopped();
    void storeAndClearBoardRotation();
    void recallBoardRotation();
    void savePositionEnabledChanged(bool state);
    void progressChanged(int value);
    void displayVisualHelp(QString elementID);
    void displayInstructions(QString text, WizardModel::MessageType type = WizardModel::Info);

public slots:
    void magStart();
    void accelStart();
    void savePositionData();
    void save();

private slots:
    void getSample(UAVObject *obj);
    void continouslyGetMagSamples(UAVObject *obj);

private:
    class CalibrationStep {
public:
        CalibrationStep(QString newVisualHelp, QString newInstructions)
        {
            visualHelp   = newVisualHelp;
            instructions = newInstructions;
        }
        QString visualHelp;
        QString instructions;
    };

    typedef struct {
        UAVObject::Metadata accelStateMetadata;
        UAVObject::Metadata magSensorMetadata;
        UAVObject::Metadata auxMagSensorMetadata;
        AuxMagSettings::DataFields auxMagSettings;
        RevoCalibration::DataFields   revoCalibrationData;
        AccelGyroSettings::DataFields accelGyroSettingsData;
    } Memento;

    typedef struct {
        RevoCalibration::DataFields   revoCalibrationData;
        AuxMagSettings::DataFields auxMagSettingsData;
        AccelGyroSettings::DataFields accelGyroSettingsData;
    } Result;

    bool calibratingMag;
    bool calibratingAuxMag;
    bool calibratingAccel;

    QList<CalibrationStep> calibrationStepsMag;
    QList<CalibrationStep> calibrationStepsAccelOnly;
    QList<CalibrationStep> *currentSteps;
    int position;

    Memento memento;
    Result result;

    bool collectingData;
    bool m_dirty;

    QMutex sensorsUpdateLock;

    double accel_data_x[6], accel_data_y[6], accel_data_z[6];

    QList<double> accel_accum_x;
    QList<double> accel_accum_y;
    QList<double> accel_accum_z;

    QList<double> mag_accum_x;
    QList<double> mag_accum_y;
    QList<double> mag_accum_z;
    QList<float> mag_fit_x;
    QList<float> mag_fit_y;
    QList<float> mag_fit_z;

    QList<double> aux_mag_accum_x;
    QList<double> aux_mag_accum_y;
    QList<double> aux_mag_accum_z;
    QList<float> aux_mag_fit_x;
    QList<float> aux_mag_fit_y;
    QList<float> aux_mag_fit_z;

    // convenience pointers
    RevoCalibration *revoCalibration;
    AccelGyroSettings *accelGyroSettings;
    AuxMagSettings *auxMagSettings;
    AccelState *accelState;
    MagSensor *magSensor;
    AuxMagSensor *auxMagSensor;
    HomeLocation *homeLocation;

    void start(bool calibrateAccel, bool calibrateMag);
    // Computes the scale and bias of the mag based on collected data
    void compute();
    void showHelp(QString image);
    UAVObjectManager *getObjectManager();
    void calcCalibration(QList<float> x, QList<float> y, QList<float> z, double Be_length, float calibrationMatrix[], float bias[]);
};
}

#endif // SIXPOINTCALIBRATIONMODEL_H

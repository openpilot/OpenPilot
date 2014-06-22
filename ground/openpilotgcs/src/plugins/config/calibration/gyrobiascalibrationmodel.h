/**
 ******************************************************************************
 *
 * @file       gyrobiascalibrationmodel.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2014.
 * @addtogroup board level calibration
 * @{
 * @addtogroup ConfigPlugin Config Plugin
 * @{
 * @brief Telemetry configuration panel
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


#ifndef GYROBIASCALIBRATIONMODEL_H
#define GYROBIASCALIBRATIONMODEL_H

#include "wizardmodel.h"
#include "extensionsystem/pluginmanager.h"
#include "uavobjectmanager.h"
#include "uavobject.h"

#include <gyrostate.h>
#include <gyrosensor.h>
#include <attitudesettings.h>
#include <revocalibration.h>
#include <accelgyrosettings.h>

#include <QObject>

namespace OpenPilot {
class GyroBiasCalibrationModel : public QObject {
    Q_OBJECT

public:
    explicit GyroBiasCalibrationModel(QObject *parent = 0);

    bool dirty()
    {
        return m_dirty;
    }

signals:
    void started();
    void stopped();
    void storeAndClearBoardRotation();
    void recallBoardRotation();
    void progressChanged(int value);
    void displayVisualHelp(QString elementID);
    void displayInstructions(QString text, WizardModel::MessageType type = WizardModel::Info);

public slots:
    void start();
    void save();

private slots:
    void getSample(UAVObject *obj);

private:
    typedef struct {
        UAVObject::Metadata gyroStateMetadata;
        UAVObject::Metadata gyroSensorMetadata;
        RevoCalibration::DataFields revoCalibrationData;
        AttitudeSettings::DataFields attitudeSettingsData;
    } Memento;

    QMutex sensorsUpdateLock;

    bool collectingData;
    bool m_dirty;

    Memento memento;

    QList<double> gyro_accum_x;
    QList<double> gyro_accum_y;
    QList<double> gyro_accum_z;
    QList<double> gyro_state_accum_x;
    QList<double> gyro_state_accum_y;
    QList<double> gyro_state_accum_z;

    // convenience pointers
    GyroState *gyroState;
    GyroSensor *gyroSensor;
    RevoCalibration *revoCalibration;
    AttitudeSettings *attitudeSettings;
    AccelGyroSettings *accelGyroSettings;

    UAVObjectManager *getObjectManager();
};
}

#endif // GYROBIASCALIBRATIONMODEL_H

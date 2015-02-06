/**
 ******************************************************************************
 *
 * @file       levelcalibrationmodel.h
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

#ifndef LEVELCALIBRATIONMODEL_H
#define LEVELCALIBRATIONMODEL_H

#include "wizardmodel.h"
#include "calibration/calibrationutils.h"
#include <attitudestate.h>
#include <attitudesettings.h>

#include <QObject>
#include <QMutex>
#include <QList>

namespace OpenPilot {
class LevelCalibrationModel : public QObject {
    Q_OBJECT

public:
    explicit LevelCalibrationModel(QObject *parent = 0);

    bool dirty()
    {
        return m_dirty;
    }

signals:
    void started();
    void stopped();
    void savePositionEnabledChanged(bool state);
    void progressChanged(int value);
    void displayVisualHelp(QString elementID);
    void displayInstructions(QString text, WizardModel::MessageType type = WizardModel::Info);

public slots:
    void start();
    void savePosition();
    void save();

private slots:
    void getSample(UAVObject *obj);

private:
    typedef struct {
        UAVObject::Metadata attitudeStateMdata;
    } Memento;

    QMutex sensorsUpdateLock;
    int position;

    bool collectingData;
    bool m_dirty;

    Memento memento;

    QList<double> rot_accum_roll;
    QList<double> rot_accum_pitch;
    double rot_data_roll;
    double rot_data_pitch;

    // convenience pointers
    AttitudeState *attitudeState;
    AttitudeSettings *attitudeSettings;

    UAVObjectManager *getObjectManager();
};
}

#endif // LEVELCALIBRATIONMODEL_H

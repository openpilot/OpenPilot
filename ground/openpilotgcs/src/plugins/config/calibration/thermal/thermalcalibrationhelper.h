/**
 ******************************************************************************
 *
 * @file       thermalcalibrationhelper.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2014.
 *
 * @brief      Thermal calibration helper functions
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
#ifndef THERMALCALIBRATIONHELPER_H
#define THERMALCALIBRATIONHELPER_H

#include <QObject>
#include <QtCore>

#include "uavobjectmanager.h"
#include <uavobject.h>
#include <uavobjectmanager.h>

#include "extensionsystem/pluginmanager.h"

// UAVOs
#include <accelsensor.h>
#include <gyrosensor.h>
#include <barosensor.h>
#include "accelgyrosettings.h"


// Calibration data
#include <accelgyrosettings.h>
#include <revocalibration.h>
#include <revosettings.h>


typedef struct {
    // this is not needed for revo, but should for CC/CC3D
    // AccelGyroSettings::DataFields accelGyroSettings;
    RevoSettings::DataFields revoSettings;
    UAVObject::Metadata gyroSensorMeta;
    UAVObject::Metadata accelSensorMeta;
    UAVObject::Metadata baroensorMeta;
    bool statusSaved = false;
} thermalCalibrationBoardSettings;

class ThermalCalibrationHelper : public QObject {
    Q_OBJECT
public:
    explicit ThermalCalibrationHelper(QObject *parent = 0);

    /* board settings save/restore */
    bool saveBoardInitialSettings();
    bool restoreInitialSettings();
    bool isBoardInitialSettingsSaved()
    {
        return m_boardInitialSettings.statusSaved;
    }
private:
    void setMetadataForCalibration(UAVDataObject *uavo);
    void clearBoardInitialSettingsSaved()
    {
        m_boardInitialSettings.statusSaved = false;
    }
signals:
    void statusRestoreCompleted(bool succesful);
    void statusSaveCompleted(bool succesful);
public slots:
    void statusSave();
    void statusRestore();

    /* board configuration setup for calibration */
public:
    bool setupBoardForCalibration();
signals:
    void setupBoardCompleted(bool succesful);

public slots:
    void setupBoard();

private:
    thermalCalibrationBoardSettings m_boardInitialSettings;
    UAVObjectManager *getObjectManager();
};

#endif // THERMALCALIBRATIONHELPER_H

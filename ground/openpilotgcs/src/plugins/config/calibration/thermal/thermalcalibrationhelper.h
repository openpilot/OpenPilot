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
#include <QTime>
#include "uavobjectmanager.h"
#include <uavobject.h>
#include <uavobjectmanager.h>

#include "extensionsystem/pluginmanager.h"

// UAVOs
#include <accelsensor.h>
#include <gyrosensor.h>
#include <barosensor.h>
#include <magsensor.h>
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
    float temperature()
    {
        return m_temperature;
    }

    float gradient()
    {
        return m_gradient;
    }

    int processPercentage()
    {
        return m_processPercentage;
    }

signals:
    void statusRestoreCompleted(bool succesful);
    void statusSaveCompleted(bool succesful);
    void setupBoardCompleted(bool succesful);
    void temperatureChanged(float value);
    void gradientChanged(float value);
    void processPercentageChanged(int percentage);
    void collectionCompleted();
    void calculationCompleted();
    void abort();

public slots:
    /**
     * @brief statusSave save the initial board status/configuration to be restored later
     */
    void statusSave();

    /**
     * @brief statusRestore restore previous saved status.
     */
    void statusRestore();

    /**
     * @brief setupBoard prepare board settings for acquisition state
     */
    void setupBoard();

    /**
     * @brief initAcquisition Initialize the helper class for data acquisition/collection
     */
    void initAcquisition();

    void calculate();

    void collectSample(UAVObject *sample);
    void endAcquisition();
    void setProcessPercentage(int value)
    {
        if (m_processPercentage != value) {
            m_processPercentage = value;
            emit processPercentageChanged(value);
        }
    }

private:
    void updateTemp(float temp);
    void connectUAVOs();
    void disconnectUAVOs();

    QMutex sensorsUpdateLock;

    QList<AccelSensor::DataFields> m_accelSamples;
    QList<GyroSensor::DataFields> m_gyroSamples;
    QList<BaroSensor::DataFields> m_baroSamples;
    QList<MagSensor::DataFields> m_magSamples;

    QTime m_startTime;
    // temperature checkpoints, used to calculate temp gradient
    const int TimeBetweenCheckpoints = 10;
    QTime m_lastCheckpointTime;
    float m_lastCheckpointTemp;
    float m_gradient;
    float m_temperature;
    float m_initialGradient;
    const int ProcessPercentageSaveSettings    = 5;
    const int ProcessPercentageSetupBoard      = 10;
    const int ProcessPercentageBaseAcquisition = 15;
    const int ProcessPercentageBaseCalculation = 85;
    const float TargetGradient = 0.5f;
    int m_targetduration;
    int m_processPercentage;

    /* board settings save/restore */
    bool setupBoardForCalibration();
    bool saveBoardInitialSettings();
    bool restoreInitialSettings();
    bool isBoardInitialSettingsSaved()
    {
        return m_boardInitialSettings.statusSaved;
    }
    void clearBoardInitialSettingsSaved()
    {
        m_boardInitialSettings.statusSaved = false;
    }
    thermalCalibrationBoardSettings m_boardInitialSettings;

    void setMetadataForCalibration(UAVDataObject *uavo);
    UAVObjectManager *getObjectManager();
};

#endif // THERMALCALIBRATIONHELPER_H

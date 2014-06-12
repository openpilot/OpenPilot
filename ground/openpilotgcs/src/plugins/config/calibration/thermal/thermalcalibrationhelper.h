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
#include <QTemporaryDir>
#include <QTextStream>

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

#include "../wizardmodel.h"

namespace OpenPilot {
typedef struct {
    // this is not needed for revo, but should for CC/CC3D
    // AccelGyroSettings::DataFields accelGyroSettings;
    RevoSettings::DataFields revoSettings;
    AccelGyroSettings::DataFields accelGyroSettings;
    UAVObject::Metadata gyroSensorMeta;
    UAVObject::Metadata accelSensorMeta;
    UAVObject::Metadata baroensorMeta;
    bool statusSaved;
} Memento;

typedef struct {
    bool  baroCalibrated;
    float baro[4];
    bool  accelCalibrated;
    float accel[3];
    bool  gyroCalibrated;
    float gyro[6];

    float baroInSigma;
    float baroOutSigma;

    float accelInSigma[3];
    float accelOutSigma[3];

    float gyroInSigma[3];
    float gyroOutSigma[3];
    float baroTempMin;
    float baroTempMax;
    float accelGyroTempMin;
    float accelGyroTempMax;
} thermalCalibrationResults;

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

    float range()
    {
        return fabs(m_maxTemperature - m_minTemperature);
    }

    int processPercentage()
    {
        return m_progress;
    }

    void endAcquisition();

    bool calibrationSuccessful()
    {
        return (range() > TargetTempDelta) && baroCalibrationSuccessful();
    }

    bool baroCalibrationSuccessful()
    {
        return m_results.baroCalibrated;
    }

    bool gyroCalibrationSuccessful()
    {
        return m_results.gyroCalibrated;
    }

    bool accelCalibrationSuccessful()
    {
        return m_results.accelCalibrated;
    }

    void copyResultToSettings();

signals:
    void statusRestoreCompleted(bool succesful);
    void statusSaveCompleted(bool succesful);
    void setupBoardCompleted(bool succesful);
    void collectionCompleted();
    void calculationCompleted();

    void temperatureChanged(float temperature);
    void temperatureGradientChanged(float temperatureGradient);
    void temperatureRangeChanged(float temperatureRange);

    void progressChanged(int value);
    void progressMaxChanged(int value);

    void instructionsAdded(QString text, WizardModel::MessageType type = WizardModel::Info);


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

    void stopAcquisition()
    {
        QMutexLocker lock(&sensorsUpdateLock);
        emit collectionCompleted();
    }

    void calculate();

    void collectSample(UAVObject *sample);
    void setProgress(int value)
    {
        if (m_progress != value) {
            m_progress = value;
            emit progressChanged(value);
        }
    }
    void setProgressMax(int value)
    {
        m_progressMax = value;
        emit progressMaxChanged(value);
    }

    void addInstructions(QString text, WizardModel::MessageType type = WizardModel::Info)
    {
        emit instructionsAdded(text, type);
    }

    void cleanup();

private:
    float getTemperature();
    void updateTemperature(float temp);

    void connectUAVOs();
    void disconnectUAVOs();

    QFile m_debugFile;
    QTextStream m_debugStream;
    QScopedPointer<QTemporaryDir> m_tempdir;
    void createDebugLog();
    void closeDebugLog();

    QMutex sensorsUpdateLock;

    QList<AccelSensor::DataFields> m_accelSamples;
    QList<GyroSensor::DataFields> m_gyroSamples;
    QList<BaroSensor::DataFields> m_baroSamples;
    QList<MagSensor::DataFields> m_magSamples;

    // temperature checkpoints, used to calculate temp gradient
    const static int TimeBetweenCheckpoints = 10;

    bool m_acquiring;
    bool m_forceStopAcquisition;

    QTime m_startTime;
    QTime m_lastCheckpointTime;
    float m_lastCheckpointTemp;

    float m_temperature;
    float m_minTemperature;
    float m_maxTemperature;
    float m_gradient;
    float m_initialGradient;

    int m_targetduration;

    int m_progress;
    int m_progressMax;

    const static float TargetGradient  = 0.20f;
    const static float TargetTempDelta = 10.0f;

    // convenience pointers
    AccelSensor *accelSensor;
    GyroSensor *gyroSensor;
    BaroSensor *baroSensor;
    MagSensor *magSensor;
    AccelGyroSettings *accelGyroSettings;
    RevoSettings *revoSettings;

    /* board settings save/restore */
    bool setupBoardForCalibration();
    bool saveBoardInitialSettings();
    bool restoreInitialSettings();
    bool isBoardInitialSettingsSaved()
    {
        return m_memento.statusSaved;
    }
    void clearBoardInitialSettingsSaved()
    {
        m_memento.statusSaved = false;
    }
    Memento m_memento;
    thermalCalibrationResults m_results;

    void setMetadataForCalibration(UAVDataObject *uavo);
    UAVObjectManager *getObjectManager();
};
}
#endif // THERMALCALIBRATIONHELPER_H

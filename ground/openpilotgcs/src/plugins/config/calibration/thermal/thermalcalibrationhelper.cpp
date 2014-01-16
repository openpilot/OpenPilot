/**
 ******************************************************************************
 *
 * @file       thermalcalibrationhelper.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2013.
 *
 * @brief      Utilities for thermal calibration
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
#include "thermalcalibrationhelper.h"

ThermalCalibrationHelper::ThermalCalibrationHelper(QObject *parent) :
    QObject(parent)
{
    m_boardInitialSettings = thermalCalibrationBoardSettings();
}

/**
 * @brief Change board settings to prepare it for calibration
 * @return
 */
bool ThermalCalibrationHelper::setupBoardForCalibration()
{
    qDebug() << "setupBoardForCalibration";

    UAVObjectManager *objManager = getObjectManager();
    Q_ASSERT(objManager);

    // accelSensor Meta
    AccelSensor *accelSensor = AccelSensor::GetInstance(objManager);
    Q_ASSERT(accelSensor);
    setMetadataForCalibration(accelSensor);

    // gyroSensor Meta
    GyroSensor *gyroSensor = GyroSensor::GetInstance(objManager);
    Q_ASSERT(gyroSensor);
    setMetadataForCalibration(gyroSensor);

    // baroSensor Meta
    BaroSensor *baroSensor = BaroSensor::GetInstance(objManager);
    Q_ASSERT(baroSensor);
    setMetadataForCalibration(baroSensor);

    // clean any correction before calibrating
    RevoSettings *revoSettings = RevoSettings::GetInstance(objManager);
    Q_ASSERT(revoSettings);
    RevoSettings::DataFields revoSettingsData = revoSettings->getData();
    for (int i = 0; i < RevoSettings::BAROTEMPCORRECTIONPOLYNOMIAL_NUMELEM; i++) {
        revoSettingsData.BaroTempCorrectionPolynomial[i] = 0.0f;
    }
    revoSettings->setData(revoSettingsData);

    return true;
}

/**
 * @brief Save board status to be later restored using restoreBoardStatus
 * @return
 */
bool ThermalCalibrationHelper::saveBoardInitialSettings()
{
    // Store current board status:
    qDebug() << "Save initial settings";

    UAVObjectManager *objManager = getObjectManager();
    Q_ASSERT(objManager);
    // accelSensor Meta
    AccelSensor *accelSensor     = AccelSensor::GetInstance(objManager);
    Q_ASSERT(accelSensor);
    m_boardInitialSettings.accelSensorMeta = accelSensor->getMetadata();
    // gyroSensor Meta
    GyroSensor *gyroSensor = GyroSensor::GetInstance(objManager);
    Q_ASSERT(gyroSensor);
    m_boardInitialSettings.gyroSensorMeta = gyroSensor->getMetadata();

    // baroSensor Meta
    BaroSensor *baroSensor = BaroSensor::GetInstance(objManager);
    Q_ASSERT(baroSensor);
    m_boardInitialSettings.baroensorMeta = baroSensor->getMetadata();

    // revoSettings data
    RevoSettings *revoSettings = RevoSettings::GetInstance(objManager);
    Q_ASSERT(revoSettings);
    m_boardInitialSettings.revoSettings = revoSettings->getData();

    // accelGyroSettings data
    /*
     * Note: for revolution it is not neede but in case of CC we would prevent having
     * a new set of xxxSensor UAVOs beside actual xxxState so it may be needed to reset the following
       AccelGyroSettings *accelGyroSettings = AccelGyroSettings::GetInstance(objManager);
       Q_ASSERT(accelGyroSettings);
       m_boardInitialSettings.accelGyroSettings = accelGyroSettings->getData();
     */
    m_boardInitialSettings.statusSaved = true;
    return true;
}

/**
 * @brief restore board settings from status saved calling saveBoardStatus
 * @return true if success
 */
bool ThermalCalibrationHelper::restoreInitialSettings()
{
    if (!m_boardInitialSettings.statusSaved) {
        return false;
    }
    // restore initial board status
    UAVObjectManager *objManager = getObjectManager();
    Q_ASSERT(objManager);

    // accelSensor Meta
    AccelSensor *accelSensor = AccelSensor::GetInstance(objManager);
    Q_ASSERT(accelSensor);
    accelSensor->setMetadata(m_boardInitialSettings.accelSensorMeta);

    // gyroSensor Meta
    GyroSensor *gyroSensor = GyroSensor::GetInstance(objManager);
    Q_ASSERT(gyroSensor);
    gyroSensor->setMetadata(m_boardInitialSettings.gyroSensorMeta);

    // baroSensor Meta
    BaroSensor *baroSensor = BaroSensor::GetInstance(objManager);
    Q_ASSERT(baroSensor);
    baroSensor->setMetadata(m_boardInitialSettings.baroensorMeta);

    // revoSettings data
    RevoSettings *revoSettings = RevoSettings::GetInstance(objManager);
    Q_ASSERT(revoSettings);
    revoSettings->setData(m_boardInitialSettings.revoSettings);

    m_boardInitialSettings.statusSaved = false;
    return true;
}


/* Methods called from transitions */

void ThermalCalibrationHelper::setupBoard()
{
    setProcessPercentage(ProcessPercentageSetupBoard);
    if (setupBoardForCalibration()) {
        emit setupBoardCompleted(true);
    } else {
        emit setupBoardCompleted(false);
    }
}

void ThermalCalibrationHelper::statusRestore()
{
    if (isBoardInitialSettingsSaved() && restoreInitialSettings()) {
        clearBoardInitialSettingsSaved();
        emit statusRestoreCompleted(true);
    } else {
        emit statusRestoreCompleted(false);
    }
}

void ThermalCalibrationHelper::statusSave()
{
    setProcessPercentage(ProcessPercentageSaveSettings);
    // prevent saving multiple times
    if (!isBoardInitialSettingsSaved() && saveBoardInitialSettings()) {
        emit statusSaveCompleted(true);
    } else {
        emit statusSaveCompleted(false);
    }
}

void ThermalCalibrationHelper::initAcquisition()
{
    setProcessPercentage(ProcessPercentageBaseAcquisition);
    QMutexLocker lock(&sensorsUpdateLock);
    m_targetduration  = 0;
    m_gradient = 0.0f;
    m_initialGradient = m_gradient;

    // Clear all samples
    m_accelSamples.clear();
    m_gyroSamples.clear();
    m_baroSamples.clear();
    m_magSamples.clear();

    // retrieve current temperature/time as initial checkpoint.
    m_lastCheckpointTime = QTime::currentTime();
    m_startTime = m_lastCheckpointTime;
    BaroSensor *baroSensor = BaroSensor::GetInstance(getObjectManager());
    Q_ASSERT(baroSensor);
    m_lastCheckpointTemp = baroSensor->getTemperature();
    m_gradient = 0;
    connectUAVOs();
}
void ThermalCalibrationHelper::collectSample(UAVObject *sample)
{
    QMutexLocker lock(&sensorsUpdateLock);

    switch (sample->getObjID()) {
    case AccelSensor::OBJID:
    {
        AccelSensor *reading = AccelSensor::GetInstance(getObjectManager());
        Q_ASSERT(reading);
        m_accelSamples.append(reading->getData());
        break;
    }
    case GyroSensor::OBJID:
    {
        GyroSensor *reading = GyroSensor::GetInstance(getObjectManager());
        Q_ASSERT(reading);
        m_gyroSamples.append(reading->getData());
        break;
    }
    case BaroSensor::OBJID:
    {
        BaroSensor *reading = BaroSensor::GetInstance(getObjectManager());
        Q_ASSERT(reading);
        m_baroSamples.append(reading->getData());
        // this is needed as temperature is low pass filtered
        m_temperature = reading->getTemperature();
        updateTemp(m_temperature);
        break;
    }
    case MagSensor::OBJID:
    {
        MagSensor *reading = MagSensor::GetInstance(getObjectManager());
        Q_ASSERT(reading);
        m_magSamples.append(reading->getData());
        break;
    }
    default:
    {
        qDebug() << " unexpected object " << sample->getObjID();
    }
    }
}


void ThermalCalibrationHelper::calculate()
{
    setProcessPercentage(ProcessPercentageBaseCalculation);
}


/* helper methods */
void ThermalCalibrationHelper::updateTemp(float temp)
{
    int elapsed = m_startTime.secsTo(QTime::currentTime());
    int secondsSinceLastCheck = m_lastCheckpointTime.secsTo(QTime::currentTime());

    m_temperature = m_temperature * 0.9f + temp * 0.1f;
    emit temperatureChanged(m_temperature);

    if (secondsSinceLastCheck > TimeBetweenCheckpoints) {
        // gradient is expressed in °C/min
        float gradient = 60.0 * (m_temperature - m_lastCheckpointTemp) / (float)secondsSinceLastCheck;
        m_gradient = gradient;
        emit gradientChanged(gradient);

        qDebug() << "Temp Gradient " << gradient << " Elapsed" << elapsed;

        m_lastCheckpointTime = QTime::currentTime();
        m_lastCheckpointTemp = m_temperature;
    }
    // at least a checkpoint has been reached
    if (elapsed > TimeBetweenCheckpoints) {
        // .1 is a "very" small value in this case thats > 0
        if (m_initialGradient < .1 && m_gradient > .1) {
            m_initialGradient = m_gradient;
        }
        if (m_gradient < TargetGradient) {
            endAcquisition();
        }

        if (m_targetduration != 0) {
            int tmp = ((ProcessPercentageBaseCalculation - ProcessPercentageBaseAcquisition)
                       * elapsed) / m_targetduration;
            tmp = tmp > ProcessPercentageBaseCalculation - 5 ? ProcessPercentageBaseCalculation - 5 : tmp;
            setProcessPercentage(tmp);
        } else if (m_gradient > .1 && m_initialGradient / 2.0f > m_gradient) {
            qDebug() << "M_gradient " << m_gradient << " Elapsed" << elapsed << " m_initialGradient" << m_initialGradient;
            // make a rough estimation of the time needed
            m_targetduration = elapsed * 5;
        }
    }
}

void ThermalCalibrationHelper::endAcquisition()
{
    // this is called from the collectSample that already has the lock
    // QMutexLocker lock(&sensorsUpdateLock);
    disconnectUAVOs();
    emit collectionCompleted();
}

void ThermalCalibrationHelper::connectUAVOs()
{
    AccelSensor *accel = AccelSensor::GetInstance(getObjectManager());

    connect(accel, SIGNAL(objectUpdated(UAVObject *)), this, SLOT(collectSample(UAVObject *)));

    GyroSensor *gyro = GyroSensor::GetInstance(getObjectManager());
    connect(gyro, SIGNAL(objectUpdated(UAVObject *)), this, SLOT(collectSample(UAVObject *)));

    BaroSensor *baro = BaroSensor::GetInstance(getObjectManager());
    connect(baro, SIGNAL(objectUpdated(UAVObject *)), this, SLOT(collectSample(UAVObject *)));

    MagSensor *mag   = MagSensor::GetInstance(getObjectManager());
    connect(mag, SIGNAL(objectUpdated(UAVObject *)), this, SLOT(collectSample(UAVObject *)));
}

void ThermalCalibrationHelper::disconnectUAVOs()
{
    AccelSensor *accel = AccelSensor::GetInstance(getObjectManager());

    disconnect(accel, SIGNAL(objectUpdated(UAVObject *)), this, SLOT(collectSample(UAVObject *)));

    GyroSensor *gyro = GyroSensor::GetInstance(getObjectManager());
    disconnect(gyro, SIGNAL(objectUpdated(UAVObject *)), this, SLOT(collectSample(UAVObject *)));

    BaroSensor *baro = BaroSensor::GetInstance(getObjectManager());
    disconnect(baro, SIGNAL(objectUpdated(UAVObject *)), this, SLOT(collectSample(UAVObject *)));

    MagSensor *mag   = MagSensor::GetInstance(getObjectManager());
    disconnect(mag, SIGNAL(objectUpdated(UAVObject *)), this, SLOT(collectSample(UAVObject *)));
}
void ThermalCalibrationHelper::setMetadataForCalibration(UAVDataObject *uavo)
{
    Q_ASSERT(uavo);
    UAVObject::Metadata meta = uavo->getMetadata();
    UAVObject::SetFlightTelemetryUpdateMode(meta, UAVObject::UPDATEMODE_PERIODIC);
    meta.flightTelemetryUpdatePeriod = 100;
    uavo->setMetadata(meta);
}

/**
 * Util function to get a pointer to the object manager
 * @return pointer to the UAVObjectManager
 */
UAVObjectManager *ThermalCalibrationHelper::getObjectManager()
{
    ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
    UAVObjectManager *objMngr = pm->getObject<UAVObjectManager>();

    Q_ASSERT(objMngr);
    return objMngr;
}

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
#include "thermalcalibration.h"
namespace OpenPilot {
ThermalCalibrationHelper::ThermalCalibrationHelper(QObject *parent) :
    QObject(parent)
{
    m_boardInitialSettings    = thermalCalibrationBoardSettings();
    m_boardInitialSettings.statusSaved = false;
    m_results = thermalCalibrationResults();
    m_results.accelCalibrated = false;
    m_results.baroCalibrated  = false;
    m_results.gyroCalibrated  = false;
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

    // Clean up any gyro/accel correction before calibrating
    AccelGyroSettings *accelGyroSettings = AccelGyroSettings::GetInstance(objManager);
    Q_ASSERT(accelGyroSettings);
    AccelGyroSettings::DataFields data   = accelGyroSettings->getData();
    for (int i = 0; i < AccelGyroSettings::ACCEL_TEMP_COEFF_NUMELEM; i++) {
        data.accel_temp_coeff[i] = 0.0f;
    }

    for (int i = 0; i < AccelGyroSettings::GYRO_TEMP_COEFF_NUMELEM; i++) {
        data.gyro_temp_coeff[i] = 0.0f;
    }
    accelGyroSettings->setData(data);

    // clean any correction before calibrating
    RevoSettings *revoSettings = RevoSettings::GetInstance(objManager);
    Q_ASSERT(revoSettings);
    RevoSettings::DataFields revoSettingsData = revoSettings->getData();
    for (uint i = 0; i < RevoSettings::BAROTEMPCORRECTIONPOLYNOMIAL_NUMELEM; i++) {
        revoSettingsData.BaroTempCorrectionPolynomial[i] = 0.0f;
    }
    revoSettingsData.BaroTempCorrectionExtent[0] = 0.0f;
    revoSettingsData.BaroTempCorrectionExtent[1] = 0.0f;

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

    // accelGyroSettings data
    AccelGyroSettings *accelGyroSettings = AccelGyroSettings::GetInstance(objManager);
    Q_ASSERT(accelGyroSettings);
    m_boardInitialSettings.accelGyroSettings = accelGyroSettings->getData();

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

    // AccelGyroSettings data
    AccelGyroSettings *accelGyroSettings = AccelGyroSettings::GetInstance(objManager);
    Q_ASSERT(accelGyroSettings);
    accelGyroSettings->setData(m_boardInitialSettings.accelGyroSettings);

    // revoSettings data
    RevoSettings *revoSettings = RevoSettings::GetInstance(objManager);
    Q_ASSERT(revoSettings);
    revoSettings->setData(m_boardInitialSettings.revoSettings);

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
    m_forceStopAcquisition = false;
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
    int count = m_baroSamples.count();
    Eigen::VectorXf datax(count);
    Eigen::VectorXf datay(1);
    Eigen::VectorXf dataz(1);
    Eigen::VectorXf datat(count);

    for (int x = 0; x < count; x++) {
        datax[x] = m_baroSamples[x].Pressure;
        datat[x] = m_baroSamples[x].Temperature;
    }

    m_results.baroCalibrated = ThermalCalibration::BarometerCalibration(datax, datat, m_results.baro, &m_results.baroInSigma, &m_results.baroOutSigma);

    m_results.baroTempMin    = datat.array().minCoeff();
    m_results.baroTempMax    = datat.array().maxCoeff();
    setProcessPercentage(processPercentage() + 2);
    count = m_gyroSamples.count();
    datax.resize(count);
    datay.resize(count);
    dataz.resize(count);
    datat.resize(count);

    for (int x = 0; x < count; x++) {
        datax[x] = m_gyroSamples[x].x;
        datay[x] = m_gyroSamples[x].y;
        dataz[x] = m_gyroSamples[x].z;
        datat[x] = m_gyroSamples[x].temperature;
    }

    m_results.gyroCalibrated = ThermalCalibration::GyroscopeCalibration(datax, datay, dataz, datat, m_results.gyro, m_results.gyroInSigma, m_results.gyroOutSigma);

    // TODO: sanity checks needs to be enforced before accel calibration can be enabled and usable.
    /*
       setProcessPercentage(processPercentage() + 2);
       count = m_accelSamples.count();
       datax.resize(count);
       datay.resize(count);
       dataz.resize(count);
       datat.resize(count);

       for(int x = 0; x < count; x++){
        datax[x] = m_accelSamples[x].x;
        datay[x] = m_accelSamples[x].y;
        dataz[x] = m_accelSamples[x].z;
        datat[x] = m_accelSamples[x].temperature;
       }

       m_results.accelCalibrated = ThermalCalibration::AccelerometerCalibration(datax, datay, dataz, datat, m_results.accel);
     */
    m_results.accelCalibrated = false;

    qDebug() << QStringLiteral("Calibration results");
    qDebug() << QStringLiteral("Baro cal {%1, %2, %3, %4}; initial variance: %5; Calibrated variance %6")
        .arg(m_results.baro[0]).arg(m_results.baro[1]).arg(m_results.baro[2]).arg(m_results.baro[3])
        .arg(m_results.baroInSigma).arg(m_results.baroOutSigma);
    qDebug() << QStringLiteral("Gyro cal x{%1} y{%2} z{%3, %4}; initial variance: {%5, %6, %7}; Calibrated variance {%8, %9, %10}")
        .arg(m_results.gyro[0]).arg(m_results.gyro[1]).arg(m_results.gyro[2]).arg(m_results.baro[3])
        .arg(m_results.gyroInSigma[0]).arg(m_results.gyroInSigma[1]).arg(m_results.gyroInSigma[2])
        .arg(m_results.gyroOutSigma[0]).arg(m_results.gyroOutSigma[1]).arg(m_results.gyroOutSigma[2]);
    qDebug() << QStringLiteral("Accel cal x{%1} y{%2} z{%3}; initial variance: {%4, %5, %6}; Calibrated variance {%7, %8, %9}")
        .arg(m_results.accel[0]).arg(m_results.accel[1]).arg(m_results.accel[2])
        .arg(m_results.accelInSigma[0]).arg(m_results.accelInSigma[1]).arg(m_results.accelInSigma[2])
        .arg(m_results.accelOutSigma[0]).arg(m_results.accelOutSigma[1]).arg(m_results.accelOutSigma[2]);
    copyResultToSettings();
    emit calculationCompleted();
}


/* helper methods */
void ThermalCalibrationHelper::updateTemp(float temp)
{
    int elapsed = m_startTime.secsTo(QTime::currentTime());
    int secondsSinceLastCheck = m_lastCheckpointTime.secsTo(QTime::currentTime());

    m_temperature = m_temperature * 0.95f + temp * 0.05f;
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
        if (m_gradient < TargetGradient || m_forceStopAcquisition) {
            emit collectionCompleted();
        }

        if (m_targetduration != 0) {
            int tmp = ((ProcessPercentageBaseCalculation - ProcessPercentageBaseAcquisition)
                       * elapsed) / m_targetduration;
            tmp = tmp > ProcessPercentageBaseCalculation - 5 ? ProcessPercentageBaseCalculation - 5 : tmp;
            setProcessPercentage(tmp);
        } else if (m_gradient > .1 && m_initialGradient / 2.0f > m_gradient) {
            qDebug() << "M_gradient " << m_gradient << " Elapsed" << elapsed << " m_initialGradient" << m_initialGradient;
            // make a rough estimation of the time needed
            m_targetduration = elapsed * 7;
        }
    }
}

void ThermalCalibrationHelper::endAcquisition()
{
    disconnectUAVOs();
    setProcessPercentage(ProcessPercentageBaseCalculation);
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

void ThermalCalibrationHelper::copyResultToSettings()
{
    UAVObjectManager *objManager = getObjectManager();

    Q_ASSERT(objManager);

    if (m_results.baroCalibrated) {
        RevoSettings *revosettings = RevoSettings::GetInstance(objManager);
        Q_ASSERT(revosettings);
        RevoSettings::DataFields revosettingsdata = revosettings->getData();
        revosettingsdata.BaroTempCorrectionPolynomial[0] = m_results.baro[0];
        revosettingsdata.BaroTempCorrectionPolynomial[1] = m_results.baro[1];
        revosettingsdata.BaroTempCorrectionPolynomial[2] = m_results.baro[2];
        revosettingsdata.BaroTempCorrectionPolynomial[3] = m_results.baro[3];
        revosettingsdata.BaroTempCorrectionExtent[0]     = m_results.baroTempMin;
        revosettingsdata.BaroTempCorrectionExtent[1]     = m_results.baroTempMax;
        revosettings->setData(revosettingsdata);
        revosettings->updated();
    }

    if (m_results.gyroCalibrated || m_results.accelCalibrated) {
        AccelGyroSettings *accelGyroSettings = AccelGyroSettings::GetInstance(objManager);
        Q_ASSERT(accelGyroSettings);
        AccelGyroSettings::DataFields data   = accelGyroSettings->getData();

        if (m_results.gyroCalibrated) {
            data.gyro_temp_coeff[0] = m_results.gyro[0];
            data.gyro_temp_coeff[1] = m_results.gyro[1];
            data.gyro_temp_coeff[2] = m_results.gyro[2];
            data.gyro_temp_coeff[3] = m_results.gyro[3];
        }

        if (m_results.accelCalibrated) {
            data.accel_temp_coeff[0] = m_results.gyro[0];
            data.accel_temp_coeff[1] = m_results.gyro[1];
            data.accel_temp_coeff[2] = m_results.gyro[2];
        }
        accelGyroSettings->setData(data);
        accelGyroSettings->updated();
    }
    setProcessPercentage(100.0f);
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
}

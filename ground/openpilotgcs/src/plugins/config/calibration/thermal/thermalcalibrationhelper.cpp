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
#include <uavobjectutil/uavobjectutilmanager.h>
#include <uavtalk/telemetrymanager.h>
#include "version_info/version_info.h"

#include <math.h>

// uncomment to simulate board warming up (no need to put it in the fridge...)
// #define SIMULATE

namespace OpenPilot {
ThermalCalibrationHelper::ThermalCalibrationHelper(QObject *parent) :
    QObject(parent)
{
    m_tempdir.reset(new QTemporaryDir());

    m_memento = Memento();
    m_memento.statusSaved = false;

    m_results = Results();
    m_results.accelCalibrated = false;
    m_results.gyroCalibrated  = false;
    m_results.baroCalibrated  = false;

    m_progress        = -1;
    m_progressMax     = -1;

    accelSensor       = AccelSensor::GetInstance(getObjectManager());
    gyroSensor        = GyroSensor::GetInstance(getObjectManager());
    baroSensor        = BaroSensor::GetInstance(getObjectManager());
    magSensor         = MagSensor::GetInstance(getObjectManager());
    accelGyroSettings = AccelGyroSettings::GetInstance(getObjectManager());
    revoSettings      = RevoSettings::GetInstance(getObjectManager());

    ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
    TelemetryManager *telMngr = pm->getObject<TelemetryManager>();
    connect(telMngr, SIGNAL(disconnected()), this, SLOT(cleanup()));
}

/**
 * @brief Change board settings to prepare it for calibration
 * @return
 */
bool ThermalCalibrationHelper::setupBoardForCalibration()
{
    setMetadataForCalibration(accelSensor);
    setMetadataForCalibration(gyroSensor);
    setMetadataForCalibration(baroSensor);

    // Clean up any gyro/accel correction before calibrating
    AccelGyroSettings::DataFields data = accelGyroSettings->getData();
    for (uint i = 0; i < AccelGyroSettings::ACCEL_TEMP_COEFF_NUMELEM; i++) {
        data.accel_temp_coeff[i] = 0.0f;
    }

    for (uint i = 0; i < AccelGyroSettings::GYRO_TEMP_COEFF_NUMELEM; i++) {
        data.gyro_temp_coeff[i] = 0.0f;
    }

    data.temp_calibrated_extent[0] = 0.0f;
    data.temp_calibrated_extent[1] = 0.0f;

    accelGyroSettings->setData(data);

    // clean any correction before calibrating
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
    m_memento.accelSensorMeta   = accelSensor->getMetadata();
    m_memento.gyroSensorMeta    = gyroSensor->getMetadata();
    m_memento.baroensorMeta     = baroSensor->getMetadata();
    m_memento.accelGyroSettings = accelGyroSettings->getData();
    m_memento.revoSettings = revoSettings->getData();

    /*
     * TODO: for revolution it is not needed but in case of CC we would prevent having
     * a new set of xxxSensor UAVOs beside actual xxxState so it may be needed to reset the following
       m_memento.accelGyroSettings = accelGyroSettings->getData();
     */
    m_memento.statusSaved  = true;
    return true;
}

/**
 * @brief restore board settings from status saved calling saveBoardStatus
 * @return true if success
 */
bool ThermalCalibrationHelper::restoreInitialSettings()
{
    if (!m_memento.statusSaved) {
        return false;
    }

    accelSensor->setMetadata(m_memento.accelSensorMeta);
    gyroSensor->setMetadata(m_memento.gyroSensorMeta);
    baroSensor->setMetadata(m_memento.baroensorMeta);
    accelGyroSettings->setData(m_memento.accelGyroSettings);
    revoSettings->setData(m_memento.revoSettings);

    return true;
}

/* Methods called from transitions */

void ThermalCalibrationHelper::setupBoard()
{
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
    // prevent saving multiple times
    if (!isBoardInitialSettingsSaved() && saveBoardInitialSettings()) {
        emit statusSaveCompleted(true);
    } else {
        emit statusSaveCompleted(false);
    }
}

void ThermalCalibrationHelper::initAcquisition()
{
    QMutexLocker lock(&sensorsUpdateLock);

    // Clear all samples
    m_accelSamples.clear();
    m_gyroSamples.clear();
    m_baroSamples.clear();
    m_magSamples.clear();

    m_results.accelCalibrated = false;
    m_results.gyroCalibrated  = false;
    m_results.baroCalibrated  = false;

    // retrieve current temperature/time as initial checkpoint.
    m_startTime            = m_lastCheckpointTime = QTime::currentTime();
    m_temperature          = getTemperature();
    m_lastCheckpointTemp   = m_temperature;
    m_minTemperature       = m_temperature;
    m_maxTemperature       = m_temperature;
    m_gradient = 0;
    m_initialGradient      = 0;

    m_targetduration       = 0;

    m_rangeReached         = false;

    m_forceStopAcquisition = false;
    m_acquiring            = true;

    createDebugLog();
    connectUAVOs();
}

void ThermalCalibrationHelper::collectSample(UAVObject *sample)
{
    QMutexLocker lock(&sensorsUpdateLock);

    if (!m_acquiring) {
        return;
    }

    switch (sample->getObjID()) {
    case AccelSensor::OBJID:
        m_accelSamples.append(accelSensor->getData());
        m_debugStream << "ACCEL:: " << m_accelSamples.last().temperature
                      << "\t" << QDateTime::currentDateTime().toString("hh.mm.ss.zzz")
                      << "\t" << m_accelSamples.last().x
                      << "\t" << m_accelSamples.last().y
                      << "\t" << m_accelSamples.last().z << endl;
        break;

    case GyroSensor::OBJID:
        m_gyroSamples.append(gyroSensor->getData());
        m_debugStream << "GYRO:: " << m_gyroSamples.last().temperature
                      << "\t" << QDateTime::currentDateTime().toString("hh.mm.ss.zzz")
                      << "\t" << m_gyroSamples.last().x
                      << "\t" << m_gyroSamples.last().y
                      << "\t" << m_gyroSamples.last().z << endl;
        break;

    case BaroSensor::OBJID:
    {
        float temp = getTemperature();
        BaroSensor::DataFields data = baroSensor->getData();
#ifdef SIMULATE
        data.Temperature = temp;
        data.Pressure   += 10.0f * temp;
#endif
        m_baroSamples.append(data);
        m_debugStream << "BARO:: " << m_baroSamples.last().Temperature
                      << "\t" << QDateTime::currentDateTime().toString("hh.mm.ss.zzz")
                      << "\t" << m_baroSamples.last().Pressure
                      << "\t" << m_baroSamples.last().Altitude << endl;
        // must be done last as this call might end acquisition and close the debug log file
        updateTemperature(temp);
        break;
    }

    case MagSensor::OBJID:
        m_magSamples.append(magSensor->getData());
        m_debugStream << "MAG:: " << "\t" << QDateTime::currentDateTime().toString("hh.mm.ss.zzz")
                      << "\t" << m_magSamples.last().x
                      << "\t" << m_magSamples.last().y
                      << "\t" << m_magSamples.last().z << endl;
        break;

    default:
        qDebug() << "Unexpected object" << sample->getObjID();
    }
}

float ThermalCalibrationHelper::getTemperature()
{
#ifdef SIMULATE
    float t = m_startTime.msecsTo(QTime::currentTime()) / 1000.0f;
    // Simulate a temperature rise using Newton's law of cooling
    // See http://en.wikipedia.org/wiki/Newton%27s_law_of_cooling#Newton.27s_law_of_cooling
    // Initial temp : 20
    // Final temp : 40
    // Time constant (t0) : 10.0
    // For a plot of the function, see http://fooplot.com/#W3sidHlwZSI6MCwiZXEiOiI0MC0yMCplXigteC8xMCkiLCJjb2xvciI6IiMwMDAwMDAifSx7InR5cGUiOjEwMDAsIndpbmRvdyI6WyItOTIuMjAzMjYzMTkzMzY4ODMiLCI5Ni45NzE2MzQ3NzU0MDAwOCIsIi00NC4zNzkzODMzMjU1NzY3NTQiLCI3Mi4wMzU5Mzg1MDEzNTc5OSJdfV0-
    double t0 = 10.0;
    return 40.0 - 20.0 * exp(-t / t0);

#else
    return baroSensor->getTemperature();

#endif
}

void ThermalCalibrationHelper::endAcquisition()
{
    disconnectUAVOs();
}

void ThermalCalibrationHelper::cleanup()
{
    m_acquiring = false;
    disconnectUAVOs();
    closeDebugLog();
}

void ThermalCalibrationHelper::calculate()
{
    // baro
    int count = m_baroSamples.count();
    Eigen::VectorXf datax(count);
    Eigen::VectorXf datay(1);
    Eigen::VectorXf dataz(1);
    Eigen::VectorXf datat(count);

    for (int x = 0; x < count; x++) {
        datax[x] = m_baroSamples[x].Pressure;
        datat[x] = m_baroSamples[x].Temperature;
    }

    m_results.baroCalibrated = ThermalCalibration::BarometerCalibration(datax, datat, m_results.baro,
                                                                        &m_results.baroInSigma, &m_results.baroOutSigma);
    if (m_results.baroCalibrated) {
        addInstructions(tr("Barometer is calibrated."));
    } else {
        qDebug() << "Failed to calibrate baro!";
        addInstructions(tr("Failed to calibrate barometer!"), WizardModel::Warn);
    }

    m_results.baroTempMin = datat.array().minCoeff();
    m_results.baroTempMax = datat.array().maxCoeff();

    // gyro
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

    m_results.gyroCalibrated = ThermalCalibration::GyroscopeCalibration(datax, datay, dataz, datat, m_results.gyro,
                                                                        m_results.gyroInSigma, m_results.gyroOutSigma);
    if (m_results.gyroCalibrated) {
        addInstructions(tr("Gyro is calibrated."));
    } else {
        qDebug() << "Failed to calibrate gyro!";
        addInstructions(tr("Failed to calibrate gyro!"), WizardModel::Warn);
    }

    // accel
    m_results.accelGyroTempMin = datat.array().minCoeff();
    m_results.accelGyroTempMax = datat.array().maxCoeff();
    // TODO: sanity checks needs to be enforced before accel calibration can be enabled and usable.
    /*
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
    m_results.accelCalibrated  = false;
    QString str = QStringLiteral("INFO::Calibration results") + "\n";
    str += QStringLiteral("INFO::Baro cal {%1, %2, %3, %4}; initial variance: %5; Calibrated variance %6")
           .arg(m_results.baro[0]).arg(m_results.baro[1]).arg(m_results.baro[2]).arg(m_results.baro[3])
           .arg(m_results.baroInSigma).arg(m_results.baroOutSigma) + "\n";
    str += QStringLiteral("INFO::Gyro cal x{%1, %2} y{%3, %4} z{%5, %6}; initial variance: {%7, %8, %9}; Calibrated variance {%10, %11, %12}")
           .arg(m_results.gyro[0]).arg(m_results.gyro[1])
           .arg(m_results.gyro[2]).arg(m_results.gyro[3])
           .arg(m_results.gyro[4]).arg(m_results.gyro[5])
           .arg(m_results.gyroInSigma[0]).arg(m_results.gyroInSigma[1]).arg(m_results.gyroInSigma[2])
           .arg(m_results.gyroOutSigma[0]).arg(m_results.gyroOutSigma[1]).arg(m_results.gyroOutSigma[2]) + "\n";
    str += QStringLiteral("INFO::Accel cal x{%1} y{%2} z{%3}; initial variance: {%4, %5, %6}; Calibrated variance {%7, %8, %9}")
           .arg(m_results.accel[0]).arg(m_results.accel[1]).arg(m_results.accel[2])
           .arg(m_results.accelInSigma[0]).arg(m_results.accelInSigma[1]).arg(m_results.accelInSigma[2])
           .arg(m_results.accelOutSigma[0]).arg(m_results.accelOutSigma[1]).arg(m_results.accelOutSigma[2]) + "\n";
    qDebug() << str;
    m_debugStream << str;
    emit calculationCompleted();
    closeDebugLog();
}

/* helper methods */
void ThermalCalibrationHelper::updateTemperature(float temp)
{
    int elapsed = m_startTime.secsTo(QTime::currentTime());
    int secondsSinceLastCheck = m_lastCheckpointTime.secsTo(QTime::currentTime());

    // temperature is low pass filtered
    m_temperature = m_temperature * 0.95f + temp * 0.05f;
    emit temperatureChanged(m_temperature);

    // temperature range
    if (m_temperature < m_minTemperature) {
        m_minTemperature = m_temperature;
    }
    if (m_temperature > m_maxTemperature) {
        m_maxTemperature = m_temperature;
    }
    if (!m_rangeReached && (range() >= TargetTempDelta)) {
        m_rangeReached = true;
        addInstructions(tr("Target temperature span has been acquired. Acquisition may be ended or, preferably, continued."));
    }
    emit temperatureRangeChanged(range());

    if (secondsSinceLastCheck > TimeBetweenCheckpoints) {
        // gradient is expressed in °C/min
        m_gradient = 60.0 * (m_temperature - m_lastCheckpointTemp) / (float)secondsSinceLastCheck;
        emit temperatureGradientChanged(gradient());

        qDebug() << "Temp Gradient " << gradient() << " Elapsed" << elapsed;
        m_debugStream << "INFO::Trace Temp Gradient " << gradient() << " Elapsed" << elapsed << endl;

        m_lastCheckpointTime = QTime::currentTime();
        m_lastCheckpointTemp = m_temperature;
    }
    // at least a checkpoint has been reached
    if (elapsed > TimeBetweenCheckpoints) {
        // .1 is a "very" small value in this case thats > 0
        if (m_initialGradient < .1 && m_gradient > .1) {
            m_initialGradient = m_gradient;
        }

        if (m_targetduration != 0) {
            int tmp = (100 * elapsed) / m_targetduration;
            setProgress(tmp);
        } else if ((m_gradient > .1) && ((m_initialGradient / 2.0f) > m_gradient)) {
            // make a rough estimation of the time needed
            m_targetduration = elapsed * 8;

            setProgressMax(100);

            QTime time = QTime(0, 0).addSecs(m_targetduration);
            QString timeString = time.toString(tr("m''s''''"));
            addInstructions(tr("Estimated acquisition duration is %1.").arg(timeString));

            QString str = QStringLiteral("INFO::Trace gradient : %1, elapsed : %2 initial gradient : %3, target : %4")
                          .arg(m_gradient).arg(elapsed).arg(m_initialGradient).arg(m_targetduration);
            qDebug() << str;
            m_debugStream << str << endl;
        }

        if ((m_gradient < TargetGradient) || m_forceStopAcquisition) {
            m_acquiring = false;
            emit collectionCompleted();
        }
    }
}

void ThermalCalibrationHelper::connectUAVOs()
{
    connect(accelSensor, SIGNAL(objectUpdated(UAVObject *)), this, SLOT(collectSample(UAVObject *)));
    connect(gyroSensor, SIGNAL(objectUpdated(UAVObject *)), this, SLOT(collectSample(UAVObject *)));
    connect(baroSensor, SIGNAL(objectUpdated(UAVObject *)), this, SLOT(collectSample(UAVObject *)));
    connect(magSensor, SIGNAL(objectUpdated(UAVObject *)), this, SLOT(collectSample(UAVObject *)));
}

void ThermalCalibrationHelper::disconnectUAVOs()
{
    disconnect(accelSensor, SIGNAL(objectUpdated(UAVObject *)), this, SLOT(collectSample(UAVObject *)));
    disconnect(gyroSensor, SIGNAL(objectUpdated(UAVObject *)), this, SLOT(collectSample(UAVObject *)));
    disconnect(baroSensor, SIGNAL(objectUpdated(UAVObject *)), this, SLOT(collectSample(UAVObject *)));
    disconnect(magSensor, SIGNAL(objectUpdated(UAVObject *)), this, SLOT(collectSample(UAVObject *)));
}

void ThermalCalibrationHelper::createDebugLog()
{
    if (m_debugFile.isOpen()) {
        closeDebugLog();
    }
    if (m_tempdir->isValid()) {
        QString filename = QStringLiteral("thcaldebug_%1.txt").arg(QDateTime::currentDateTime().toString("dd.MM.yyyy-hh.mm.ss.zzz"));
        QDir dir = QDir(m_tempdir->path());
        m_debugFile.setFileName(dir.filePath(filename));
        if (!m_debugFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
            m_debugStream.setDevice(0);
            return;
        }
        qDebug() << "Saving debug data for this session to " << dir.filePath(filename);

        m_debugStream.setDevice(&m_debugFile);

        ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
        UAVObjectUtilManager *utilMngr     = pm->getObject<UAVObjectUtilManager>();
        deviceDescriptorStruct board = utilMngr->getBoardDescriptionStruct();

        m_debugStream << "INFO::Hardware" << " type:" << QString().setNum(board.boardType, 16)
                      << " revision:" << QString().setNum(board.boardRevision, 16)
                      << " serial:" << QString(utilMngr->getBoardCPUSerial().toHex()) << endl;

        QString uavo = board.uavoHash.toHex();
        m_debugStream << "INFO::firmware tag:" << board.gitTag
                      << " date:" << board.gitDate
                      << " hash:" << board.gitHash
                      << " uavo:" << uavo.left(8) << endl;

        m_debugStream << "INFO::gcs tag:" << VersionInfo::tagOrBranch() + VersionInfo::dirty()
                      << " date:" << VersionInfo::dateTime()
                      << " hash:" << VersionInfo::hash().left(8)
                      << " uavo:" << VersionInfo::uavoHash().left(8) << endl;
    }
}

void ThermalCalibrationHelper::closeDebugLog()
{
    if (m_debugFile.isOpen()) {
        m_debugStream.flush();
        m_debugStream.setDevice(0);
        m_debugFile.close();
    }
}

void ThermalCalibrationHelper::copyResultToSettings()
{
    if (calibrationSuccessful()) {
        RevoSettings::DataFields revoSettingsData = revoSettings->getData();
        revoSettingsData.BaroTempCorrectionPolynomial[0] = m_results.baro[0];
        revoSettingsData.BaroTempCorrectionPolynomial[1] = m_results.baro[1];
        revoSettingsData.BaroTempCorrectionPolynomial[2] = m_results.baro[2];
        revoSettingsData.BaroTempCorrectionPolynomial[3] = m_results.baro[3];
        revoSettingsData.BaroTempCorrectionExtent[0]     = m_results.baroTempMin;
        revoSettingsData.BaroTempCorrectionExtent[1]     = m_results.baroTempMax;
        revoSettings->setData(revoSettingsData);

        AccelGyroSettings::DataFields data = accelGyroSettings->getData();
        if (m_results.gyroCalibrated) {
            data.gyro_temp_coeff[0] = m_results.gyro[0];
            data.gyro_temp_coeff[1] = m_results.gyro[1];
            data.gyro_temp_coeff[2] = m_results.gyro[2];
            data.gyro_temp_coeff[3] = m_results.gyro[3];
            data.gyro_temp_coeff[4] = m_results.gyro[4];
            data.gyro_temp_coeff[5] = m_results.gyro[5];
        }

        if (m_results.accelCalibrated) {
            data.accel_temp_coeff[0] = m_results.gyro[0];
            data.accel_temp_coeff[1] = m_results.gyro[1];
            data.accel_temp_coeff[2] = m_results.gyro[2];
        }
        data.temp_calibrated_extent[0] = m_results.accelGyroTempMin;
        data.temp_calibrated_extent[1] = m_results.accelGyroTempMax;

        accelGyroSettings->setData(data);
    }
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

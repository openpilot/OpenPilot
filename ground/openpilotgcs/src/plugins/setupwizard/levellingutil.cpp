/**
 ******************************************************************************
 *
 * @file       levellingutil.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2012.
 * @addtogroup
 * @{
 * @addtogroup LevellingUtil
 * @{
 * @brief
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

#include "levellingutil.h"
#include "extensionsystem/pluginmanager.h"
#include "uavobjectmanager.h"
#include "attitudesettings.h"
#include "accels.h"
#include "gyros.h"


LevellingUtil::LevellingUtil(long measurementCount, long measurementRate) : QObject(),
    m_isMeasuring(false), m_accelMeasurementCount(measurementCount), m_gyroMeasurementCount(measurementCount),
    m_accelMeasurementRate(measurementRate), m_gyroMeasurementRate(measurementRate)
{
}

LevellingUtil::LevellingUtil(long accelMeasurementCount, long accelMeasurementRate,
                             long gyroMeasurementCount, long gyroMeasurementRate) : QObject(),
    m_isMeasuring(false), m_accelMeasurementCount(accelMeasurementCount), m_gyroMeasurementCount(gyroMeasurementCount),
    m_accelMeasurementRate(accelMeasurementRate), m_gyroMeasurementRate(gyroMeasurementRate)
{
}

void LevellingUtil::start()
{
    if(!m_isMeasuring) {
        startMeasurement();

        // Set up timeout timer
        connect(&m_timeoutTimer, SIGNAL(timeout()), this, SLOT(timeout()));
        //Set timeout to max time of measurement + 10 secs
        m_timeoutTimer.start(std::max(m_accelMeasurementCount * m_accelMeasurementRate, m_gyroMeasurementCount * m_gyroMeasurementRate) + 10000);
    }
}

void LevellingUtil::abort()
{
    if(m_isMeasuring) {
        stopMeasurement();
    }
}

void LevellingUtil::gyroMeasurementsUpdated(UAVObject *obj)
{
    QMutexLocker locker(&m_measurementMutex);

    if(m_receivedGyroUpdates < m_gyroMeasurementCount) {
        ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
        UAVObjectManager * uavObjectManager = pm->getObject<UAVObjectManager>();
        Q_ASSERT(uavObjectManager);

        Gyros * gyros = Gyros::GetInstance(uavObjectManager);
        Gyros::DataFields gyrosData = gyros->getData();

        m_gyroX += gyrosData.x;
        m_gyroY += gyrosData.y;
        m_gyroZ += gyrosData.z;

        m_receivedGyroUpdates++;
        emit progress(m_receivedGyroUpdates + m_receivedAccelUpdates, m_gyroMeasurementCount + m_accelMeasurementCount);
    }
    else if (m_receivedAccelUpdates >= m_accelMeasurementCount &&
             m_receivedGyroUpdates >= m_gyroMeasurementCount && m_isMeasuring) {
        stopMeasurement();
        emit done(calculateLevellingData());
    }
}

void LevellingUtil::accelMeasurementsUpdated(UAVObject *obj)
{
    QMutexLocker locker(&m_measurementMutex);

    if(m_receivedAccelUpdates < m_accelMeasurementCount) {
        ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
        UAVObjectManager * uavObjectManager = pm->getObject<UAVObjectManager>();
        Q_ASSERT(uavObjectManager);

        Accels * accels = Accels::GetInstance(uavObjectManager);
        Accels::DataFields accelsData = accels->getData();

        m_accelerometerX += accelsData.x;
        m_accelerometerY += accelsData.y;
        m_accelerometerZ += accelsData.z;

        m_receivedAccelUpdates++;
        emit progress(m_receivedGyroUpdates + m_receivedAccelUpdates, m_gyroMeasurementCount + m_accelMeasurementCount);
    }
    else if (m_receivedAccelUpdates >= m_accelMeasurementCount &&
             m_receivedGyroUpdates >= m_gyroMeasurementCount && m_isMeasuring) {
        stopMeasurement();
        emit done(calculateLevellingData());
    }
}

void LevellingUtil::timeout()
{
    QMutexLocker locker(&m_measurementMutex);

    stopMeasurement();
    emit timeout(tr("Calibration timed out before receiving required updates."));
}

void LevellingUtil::startMeasurement()
{
    QMutexLocker locker(&m_measurementMutex);

    m_isMeasuring = true;

    // Reset variables
    m_receivedAccelUpdates = 0;
    m_accelerometerX = 0;
    m_accelerometerY = 0;
    m_accelerometerZ = 0;

    m_receivedGyroUpdates = 0;
    m_gyroX = 0;
    m_gyroY = 0;
    m_gyroZ = 0;

    ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
    UAVObjectManager * uavObjectManager = pm->getObject<UAVObjectManager>();
    Q_ASSERT(uavObjectManager);

    // Disable gyro bias correction to see raw data
    AttitudeSettings::DataFields attitudeSettingsData = AttitudeSettings::GetInstance(uavObjectManager)->getData();
    attitudeSettingsData.BiasCorrectGyro = AttitudeSettings::BIASCORRECTGYRO_FALSE;
    AttitudeSettings::GetInstance(uavObjectManager)->setData(attitudeSettingsData);

    // Set up to receive updates for accels
    UAVDataObject *uavObject = Accels::GetInstance(uavObjectManager);
    connect(uavObject, SIGNAL(objectUpdated(UAVObject*)), this, SLOT(accelMeasurementsUpdated(UAVObject*)));

    // Set update period for accels
    m_previousAccelMetaData = uavObject->getMetadata();
    UAVObject::Metadata newMetaData = m_previousAccelMetaData;
    UAVObject::SetFlightTelemetryUpdateMode(newMetaData, UAVObject::UPDATEMODE_PERIODIC);
    newMetaData.flightTelemetryUpdatePeriod = m_accelMeasurementRate;
    uavObject->setMetadata(newMetaData);

    // Set up to receive updates from gyros
    uavObject = Gyros::GetInstance(uavObjectManager);
    connect(uavObject, SIGNAL(objectUpdated(UAVObject*)), this, SLOT(gyroMeasurementsUpdated(UAVObject*)));

    // Set update period for gyros
    m_previousGyroMetaData = uavObject->getMetadata();
    newMetaData = m_previousGyroMetaData;
    UAVObject::SetFlightTelemetryUpdateMode(newMetaData, UAVObject::UPDATEMODE_PERIODIC);
    newMetaData.flightTelemetryUpdatePeriod = m_gyroMeasurementRate;
    uavObject->setMetadata(newMetaData);
}

void LevellingUtil::stopMeasurement()
{
    m_isMeasuring = false;

    //Stop timeout timer
    m_timeoutTimer.stop();
    disconnect(&m_timeoutTimer, SIGNAL(timeout()), this, SLOT(timeout()));

    ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
    UAVObjectManager * uavObjectManager = pm->getObject<UAVObjectManager>();
    Q_ASSERT(uavObjectManager);

    // Stop listening for updates from accels
    UAVDataObject *uavObject = Accels::GetInstance(uavObjectManager);
    disconnect(uavObject, SIGNAL(objectUpdated(UAVObject*)), this, SLOT(accelMeasurementsUpdated(UAVObject*)));
    uavObject->setMetadata(m_previousAccelMetaData);

    // Stop listening for updates from gyros
    uavObject = Gyros::GetInstance(uavObjectManager);
    disconnect(uavObject, SIGNAL(objectUpdated(UAVObject*)), this, SLOT(gyroMeasurementsUpdated(UAVObject*)));
    uavObject->setMetadata(m_previousGyroMetaData);

    // Enable gyro bias correction again
    AttitudeSettings::DataFields attitudeSettingsData = AttitudeSettings::GetInstance(uavObjectManager)->getData();
    attitudeSettingsData.BiasCorrectGyro = AttitudeSettings::BIASCORRECTGYRO_TRUE;
    AttitudeSettings::GetInstance(uavObjectManager)->setData(attitudeSettingsData);
}

accelGyroBias LevellingUtil::calculateLevellingData()
{
    accelGyroBias bias;
    bias.m_accelerometerXBias = m_accelerometerX / (double)m_receivedAccelUpdates / ACCELERATION_SCALE;
    bias.m_accelerometerYBias = m_accelerometerY / (double)m_receivedAccelUpdates / ACCELERATION_SCALE;
    bias.m_accelerometerZBias = (m_accelerometerZ / (double)m_receivedAccelUpdates + G) / ACCELERATION_SCALE;

    bias.m_gyroXBias = m_gyroX / (double)m_receivedGyroUpdates * 100.0;
    bias.m_gyroYBias = m_gyroY / (double)m_receivedGyroUpdates * 100.0;
    bias.m_gyroZBias = m_gyroZ / (double)m_receivedGyroUpdates * 100.0;
    return bias;
}


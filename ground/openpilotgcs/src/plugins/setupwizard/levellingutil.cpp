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


LevellingUtil::LevellingUtil(long measurementCount, long measurementPeriod) : QObject(),
    m_isMeasuring(false), m_measurementCount(measurementCount), m_measurementPeriod(measurementPeriod)
{
}

void LevellingUtil::start()
{
    if(!m_isMeasuring) {
        startMeasurement();

        // Set up timeout timer
        connect(&m_timeoutTimer, SIGNAL(timeout()), this, SLOT(timeout()));
        m_timeoutTimer.start(m_measurementCount * m_measurementPeriod * 2);
    }
}

void LevellingUtil::abort()
{
    if(m_isMeasuring) {
        stopMeasurement();
    }
}

void LevellingUtil::measurementsUpdated(UAVObject *obj)
{
    QMutexLocker locker(&m_measurementMutex);

    m_receivedUpdates++;
    emit progress(m_receivedUpdates, m_measurementCount);

    if(m_receivedUpdates < m_measurementCount) {
        ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
        UAVObjectManager * uavObjectManager = pm->getObject<UAVObjectManager>();
        Q_ASSERT(uavObjectManager);

        Accels * accels = Accels::GetInstance(uavObjectManager);
        Accels::DataFields accelsData = accels->getData();

        m_accelerometerX.append(accelsData.x);
        m_accelerometerY.append(accelsData.y);
        m_accelerometerZ.append(accelsData.z);

        Gyros * gyros = Gyros::GetInstance(uavObjectManager);
        Gyros::DataFields gyrosData = gyros->getData();

        m_gyroX.append(gyrosData.x);
        m_gyroY.append(gyrosData.y);
        m_gyroZ.append(gyrosData.z);
    }
    else if (m_receivedUpdates >= m_measurementCount) {
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
    m_receivedUpdates = 0;
    m_accelerometerX.clear();
    m_accelerometerY.clear();
    m_accelerometerZ.clear();
    m_gyroX.clear();
    m_gyroY.clear();
    m_gyroZ.clear();

    ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
    UAVObjectManager * uavObjectManager = pm->getObject<UAVObjectManager>();
    Q_ASSERT(uavObjectManager);

    // Disable gyro bias correction to see raw data
    AttitudeSettings::DataFields attitudeSettingsData = AttitudeSettings::GetInstance(uavObjectManager)->getData();
    attitudeSettingsData.BiasCorrectGyro = AttitudeSettings::BIASCORRECTGYRO_FALSE;
    AttitudeSettings::GetInstance(uavObjectManager)->setData(attitudeSettingsData);

    // Set up to receive updates
    UAVDataObject *uavObject = Accels::GetInstance(uavObjectManager);
    connect(uavObject, SIGNAL(objectUpdated(UAVObject*)), this, SLOT(measurementsUpdated(UAVObject*)));

    // Set update period
    m_previousMetaData = uavObject->getMetadata();
    UAVObject::Metadata newMetaData = m_previousMetaData;
    UAVObject::SetFlightTelemetryUpdateMode(newMetaData, UAVObject::UPDATEMODE_PERIODIC);
    newMetaData.flightTelemetryUpdatePeriod = m_measurementPeriod;
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

    // Stop listening for updates
    UAVDataObject *uavObject = Accels::GetInstance(uavObjectManager);
    disconnect(uavObject, SIGNAL(objectUpdated(UAVObject*)), this, SLOT(measurementsUpdated(UAVObject*)));
    uavObject->setMetadata(m_previousMetaData);

    // Enable gyro bias correction again
    AttitudeSettings::DataFields attitudeSettingsData = AttitudeSettings::GetInstance(uavObjectManager)->getData();
    attitudeSettingsData.BiasCorrectGyro = AttitudeSettings::BIASCORRECTGYRO_TRUE;
    AttitudeSettings::GetInstance(uavObjectManager)->setData(attitudeSettingsData);
}

accelGyroBias LevellingUtil::calculateLevellingData()
{
    accelGyroBias bias;
    bias.m_accelerometerXBias = listMean(m_accelerometerX) / ACCELERATION_SCALE;
    bias.m_accelerometerYBias = listMean(m_accelerometerY) / ACCELERATION_SCALE;
    bias.m_accelerometerZBias = (listMean(m_accelerometerZ) + G) / ACCELERATION_SCALE;

    bias.m_gyroXBias = listMean(m_gyroX) * 100.0f;
    bias.m_gyroYBias = listMean(m_gyroY) * 100.0f;
    bias.m_gyroZBias = listMean(m_gyroZ) * 100.0f;
    return bias;
}

double LevellingUtil::listMean(QList<double> list)
{
    double accum = 0;
    for(int i = 0; i < list.size(); i++) {
        accum += list.at(i);
    }
    return accum / list.size();
}


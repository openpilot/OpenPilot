/**
 ******************************************************************************
 *
 * @file       outputcalibrationutil.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2012.
 * @addtogroup
 * @{
 * @addtogroup OutputCalibrationUtil
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

#include "outputcalibrationutil.h"
#include "actuatorcommand.h"
#include "extensionsystem/pluginmanager.h"
#include "vehicleconfigurationhelper.h"
#include "manualcontrolsettings.h"

bool OutputCalibrationUtil::c_prepared = false;
ActuatorCommand::Metadata OutputCalibrationUtil::c_savedActuatorCommandMetaData;

OutputCalibrationUtil::OutputCalibrationUtil(QObject *parent) :
    QObject(parent), m_outputChannel(-1), m_safeValue(1000)
{
}

OutputCalibrationUtil::~OutputCalibrationUtil()
{
    stopChannelOutput();
}

void OutputCalibrationUtil::startOutputCalibration()
{
    if (!c_prepared) {
        ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
        UAVObjectManager *uavObjectManager = pm->getObject<UAVObjectManager>();
        Q_ASSERT(uavObjectManager);

        ActuatorCommand *actuatorCommand = ActuatorCommand::GetInstance(uavObjectManager);
        Q_ASSERT(actuatorCommand);
        UAVObject::Metadata metaData     = actuatorCommand->getMetadata();
        c_savedActuatorCommandMetaData = metaData;

        // Enable actuator control from GCS...
        // Store current metadata for later restore
        UAVObject::SetFlightAccess(metaData, UAVObject::ACCESS_READONLY);
        UAVObject::SetFlightTelemetryUpdateMode(metaData, UAVObject::UPDATEMODE_ONCHANGE);
        UAVObject::SetGcsTelemetryAcked(metaData, false);
        UAVObject::SetGcsTelemetryUpdateMode(metaData, UAVObject::UPDATEMODE_ONCHANGE);
        metaData.gcsTelemetryUpdatePeriod = 100;

        // Apply changes
        actuatorCommand->setMetadata(metaData);
        actuatorCommand->updated();
        c_prepared = true;
        qDebug() << "OutputCalibrationUtil started.";
    }
}

void OutputCalibrationUtil::stopOutputCalibration()
{
    if (c_prepared) {
        ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
        UAVObjectManager *uavObjectManager = pm->getObject<UAVObjectManager>();
        Q_ASSERT(uavObjectManager);

        // Restore metadata to what it was before
        ActuatorCommand *actuatorCommand = ActuatorCommand::GetInstance(uavObjectManager);
        Q_ASSERT(actuatorCommand);
        actuatorCommand->setMetadata(c_savedActuatorCommandMetaData);
        actuatorCommand->updated();
        c_prepared = false;
        qDebug() << "OutputCalibrationUtil stopped.";
    }
}

void OutputCalibrationUtil::startChannelOutput(quint16 channel, quint16 safeValue)
{
    if (c_prepared) {
        if (m_outputChannel < 0 && channel < ActuatorCommand::CHANNEL_NUMELEM) {
            // Start output...
            m_outputChannel = channel;
            m_safeValue     = safeValue;
            qDebug() << "Output for channel " << m_outputChannel + 1 << " started.";
        }
    } else {
        qDebug() << "OutputCalibrationUtil not started.";
    }
}

void OutputCalibrationUtil::stopChannelOutput()
{
    if (c_prepared) {
        if (m_outputChannel >= 0) {
            qDebug() << "Stopping output for channel " << m_outputChannel + 1 << "...";
            // Stop output...
            setChannelOutputValue(m_safeValue);
            qDebug() << "Settings output for channel " << m_outputChannel + 1 << " to " << m_safeValue;
            qDebug() << "Output for channel " << m_outputChannel + 1 << " stopped.";

            m_outputChannel = -1;
        }
    } else {
        qDebug() << "OutputCalibrationUtil not started.";
    }
}

void OutputCalibrationUtil::setChannelOutputValue(quint16 value)
{
    if (c_prepared) {
        if (m_outputChannel >= 0) {
            // Set output value
            qDebug() << "Setting output value for channel " << m_outputChannel << " to " << value << ".";
            ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
            UAVObjectManager *uavObjectManager = pm->getObject<UAVObjectManager>();
            Q_ASSERT(uavObjectManager);

            ActuatorCommand *actuatorCommand = ActuatorCommand::GetInstance(uavObjectManager);
            Q_ASSERT(actuatorCommand);

            ActuatorCommand::DataFields data = actuatorCommand->getData();
            data.Channel[m_outputChannel] = value;
            actuatorCommand->setData(data);
        } else {
            qDebug() << "OutputCalibrationUtil not started.";
        }
    }
}

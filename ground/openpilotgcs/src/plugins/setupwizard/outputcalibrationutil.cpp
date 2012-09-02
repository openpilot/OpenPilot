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

OutputCalibrationUtil::OutputCalibrationUtil(QObject *parent) :
    QObject(parent), m_outputChannel(0)
{
    ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
    m_uavObjectManager = pm->getObject<UAVObjectManager>();
    Q_ASSERT(m_uavObjectManager);
}

void OutputCalibrationUtil::startChannelOutput(quint16 channel)
{
    if(m_outputChannel == 0 && channel > 0 && channel <= ActuatorCommand::CHANNEL_NUMELEM)
    {
        ActuatorCommand *actuatorCommand = ActuatorCommand::GetInstance(m_uavObjectManager);
        Q_ASSERT(actuatorCommand);
        UAVObject::Metadata metaData = actuatorCommand->getMetadata();

        m_savedActuatorMetadata = metaData;
        UAVObject::SetFlightAccess(metaData, UAVObject::ACCESS_READONLY);
        UAVObject::SetFlightTelemetryUpdateMode(metaData, UAVObject::UPDATEMODE_ONCHANGE);
        UAVObject::SetGcsTelemetryAcked(metaData, false);
        UAVObject::SetGcsTelemetryUpdateMode(metaData, UAVObject::UPDATEMODE_ONCHANGE);
        metaData.gcsTelemetryUpdatePeriod = 100;

        actuatorCommand->setMetadata(metaData);
        actuatorCommand->updated();

        //Start output...
        m_outputChannel = channel;
    }
}

void OutputCalibrationUtil::stopChannelOutput()
{
    if(m_outputChannel > 0)
    {
        //Stop output...
        ActuatorCommand *actuatorCommand = ActuatorCommand::GetInstance(m_uavObjectManager);
        Q_ASSERT(actuatorCommand);
        UAVObject::Metadata metaData = actuatorCommand->getMetadata();

        // Restore metadata to what it was before
        metaData = m_savedActuatorMetadata;
        actuatorCommand->setMetadata(metaData);
        actuatorCommand->updated();

        m_outputChannel = 0;
    }
}

void OutputCalibrationUtil::setChannelOutputValue(quint16 value)
{
    if(m_outputChannel > 0)
    {
        //Set output value
        ActuatorCommand *actuatorCommand = ActuatorCommand::GetInstance(m_uavObjectManager);
        Q_ASSERT(actuatorCommand);
        ActuatorCommand::DataFields data = actuatorCommand->getData();
        data.Channel[m_outputChannel - 1] = value;
        actuatorCommand->setData(data);
    }
}

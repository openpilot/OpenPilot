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

const quint16 OutputCalibrationUtil::UPDATE_CHANNEL_MAPPING[10] = {1, 1, 1, 2, 3, 4, 3, 3, 4, 4};

OutputCalibrationUtil::OutputCalibrationUtil(QObject *parent) :
    QObject(parent), m_outputChannel(0)
{
    ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
    m_uavObjectManager = pm->getObject<UAVObjectManager>();
    Q_ASSERT(m_uavObjectManager);
}

void OutputCalibrationUtil::startChannelOutput(quint16 escUpdateRate, quint16 channel)
{
    if(m_outputChannel == 0 && channel > 0 && channel <= ActuatorCommand::CHANNEL_NUMELEM)
    {
        //Set actuator settings for channel
        ActuatorSettings *actuatorSettings = ActuatorSettings::GetInstance(m_uavObjectManager);
        Q_ASSERT(actuatorSettings);
        ActuatorSettings::DataFields data = actuatorSettings->getData();
        m_savedActuatorSettingData = data;

        quint16 actuatorChannel = channel - 1;
        data.ChannelType[actuatorChannel] = ActuatorSettings::CHANNELTYPE_PWM;
        data.ChannelAddr[actuatorChannel] = actuatorChannel;
        data.ChannelMin[actuatorChannel] = 1000;
        data.ChannelNeutral[actuatorChannel] = 1000;
        data.ChannelMax[actuatorChannel] = 2000;

        data.ChannelUpdateFreq[UPDATE_CHANNEL_MAPPING[actuatorChannel]] = escUpdateRate;

        actuatorSettings->setData(data);

        ActuatorCommand *actuatorCommand = ActuatorCommand::GetInstance(m_uavObjectManager);
        Q_ASSERT(actuatorCommand);
        UAVObject::Metadata metaData = actuatorCommand->getMetadata();
        m_savedActuatorCommandMetadata = metaData;

        //Enable actuator control from GCS...
        //Store current metadata for later restore
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
        // Restore metadata to what it was before
        ActuatorCommand *actuatorCommand = ActuatorCommand::GetInstance(m_uavObjectManager);
        Q_ASSERT(actuatorCommand);
        actuatorCommand->setMetadata(m_savedActuatorCommandMetadata);
        actuatorCommand->updated();

        ActuatorSettings *actuatorSettings = ActuatorSettings::GetInstance(m_uavObjectManager);
        Q_ASSERT(actuatorSettings);
        actuatorSettings->setData(m_savedActuatorSettingData);
        actuatorSettings->updated();

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

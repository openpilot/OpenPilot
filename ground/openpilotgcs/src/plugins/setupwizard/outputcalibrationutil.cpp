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

const quint16 OutputCalibrationUtil::UPDATE_CHANNEL_MAPPING[10] = {0, 0, 0, 1, 2, 3, 2, 2, 3, 3};

OutputCalibrationUtil::OutputCalibrationUtil(QObject *parent) :
    QObject(parent), m_outputChannel(-1), m_safeValue(1000)
{
    ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
    m_uavObjectManager = pm->getObject<UAVObjectManager>();
    Q_ASSERT(m_uavObjectManager);
}

OutputCalibrationUtil::~OutputCalibrationUtil()
{
    stopChannelOutput();
}

void OutputCalibrationUtil::setupOutputRates(const QList<quint16> &outputRates)
{
    //Set actuator settings for channels
    ActuatorSettings *actuatorSettings = ActuatorSettings::GetInstance(m_uavObjectManager);
    Q_ASSERT(actuatorSettings);
    ActuatorSettings::DataFields data = actuatorSettings->getData();

    for(int i = 0; i < outputRates.size(); i++) {
        data.ChannelType[i] = ActuatorSettings::CHANNELTYPE_PWM;
        data.ChannelAddr[i] = i;
        data.ChannelMin[i] = 1000;
        data.ChannelNeutral[i] = 1000;
        data.ChannelMax[i] = 2000;
        data.ChannelUpdateFreq[UPDATE_CHANNEL_MAPPING[i]] = outputRates[i];
    }

    actuatorSettings->setData(data);
    actuatorSettings->updated();
}

void OutputCalibrationUtil::startChannelOutput(quint16 channel, quint16 safeValue)
{
    if(m_outputChannel < 0 && channel >= 0 && channel < ActuatorCommand::CHANNEL_NUMELEM)
    {
        //Start output...
        m_outputChannel = channel;
        m_safeValue = safeValue;

        qDebug() << "Starting output for channel " << m_outputChannel << "...";

        ActuatorCommand *actuatorCommand = ActuatorCommand::GetInstance(m_uavObjectManager);
        Q_ASSERT(actuatorCommand);
        UAVObject::Metadata metaData = actuatorCommand->getMetadata();
        m_savedActuatorCommandMetadata = metaData;

        //Store current data for later restore
        m_savedActuatorCommandData = actuatorCommand->getData();

        //Enable actuator control from GCS...
        //Store current metadata for later restore
        UAVObject::SetFlightAccess(metaData, UAVObject::ACCESS_READONLY);
        UAVObject::SetFlightTelemetryUpdateMode(metaData, UAVObject::UPDATEMODE_ONCHANGE);
        UAVObject::SetGcsTelemetryAcked(metaData, false);
        UAVObject::SetGcsTelemetryUpdateMode(metaData, UAVObject::UPDATEMODE_ONCHANGE);
        metaData.gcsTelemetryUpdatePeriod = 100;

        //Apply changes
        actuatorCommand->setMetadata(metaData);
        actuatorCommand->updated();

        qDebug() << "Output for channel " << m_outputChannel << " started.";
    }
}

void OutputCalibrationUtil::stopChannelOutput()
{
    if(m_outputChannel >= 0)
    {
        qDebug() << "Stopping output for channel " << m_outputChannel << "...";
        //Stop output...
        setChannelOutputValue(m_safeValue);

        // Restore metadata to what it was before
        ActuatorCommand *actuatorCommand = ActuatorCommand::GetInstance(m_uavObjectManager);
        Q_ASSERT(actuatorCommand);
        actuatorCommand->setData(m_savedActuatorCommandData);
        actuatorCommand->setMetadata(m_savedActuatorCommandMetadata);
        actuatorCommand->updated();

        qDebug() << "Output for channel " << m_outputChannel << " stopped.";

        m_outputChannel = -1;
    }
}

void OutputCalibrationUtil::setChannelOutputValue(quint16 value)
{
    if(m_outputChannel >= 0)
    {
        //Set output value
        qDebug() << "Setting output value for channel " << m_outputChannel << " to " << value << ".";
        ActuatorCommand *actuatorCommand = ActuatorCommand::GetInstance(m_uavObjectManager);
        Q_ASSERT(actuatorCommand);
        ActuatorCommand::DataFields data = actuatorCommand->getData();
        data.Channel[m_outputChannel] = value;
        actuatorCommand->setData(data);
    }
}

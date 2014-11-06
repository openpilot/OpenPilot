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
#include "uavobject.h"
#include "uavobjectmanager.h"
#include "extensionsystem/pluginmanager.h"
#include "vehicleconfigurationhelper.h"
#include "manualcontrolsettings.h"

bool OutputCalibrationUtil::c_prepared = false;
ActuatorCommand::Metadata OutputCalibrationUtil::c_savedActuatorCommandMetaData;

OutputCalibrationUtil::OutputCalibrationUtil(QObject *parent) :
    QObject(parent), m_safeValue(1000)
{}

OutputCalibrationUtil::~OutputCalibrationUtil()
{
    stopChannelOutput();
}

ActuatorCommand *OutputCalibrationUtil::getActuatorCommandObject()
{
    ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();

    Q_ASSERT(pm);

    UAVObjectManager *uavObjectManager = pm->getObject<UAVObjectManager>();
    Q_ASSERT(uavObjectManager);

    ActuatorCommand *actuatorCommand   = ActuatorCommand::GetInstance(uavObjectManager);
    Q_ASSERT(actuatorCommand);

    return actuatorCommand;
}

void OutputCalibrationUtil::startOutputCalibration()
{
    if (!c_prepared) {
        ActuatorCommand *actuatorCommand = getActuatorCommandObject();
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
        ActuatorCommand *actuatorCommand = getActuatorCommandObject();
        actuatorCommand->setMetadata(c_savedActuatorCommandMetaData);
        actuatorCommand->updated();
        c_prepared = false;
        qDebug() << "OutputCalibrationUtil stopped.";
    }
}

void OutputCalibrationUtil::startChannelOutput(quint16 channel, quint16 safeValue) {
    QList<quint16> channels;
    channels.append(channel);
    startChannelOutput(channels, safeValue);
}

void OutputCalibrationUtil::startChannelOutput(QList<quint16> channels, quint16 safeValue)
{
    if (c_prepared) {
        m_outputChannels = channels;
        m_safeValue = safeValue;
    } else {
        qDebug() << "OutputCalibrationUtil not started.";
    }
}

void OutputCalibrationUtil::stopChannelOutput()
{
    if (c_prepared) {
        setChannelOutputValue(m_safeValue);
        m_outputChannels.clear();
        qDebug() << "OutputCalibrationUtil output stopped.";
    } else {
        qDebug() << "OutputCalibrationUtil not started.";
    }
}

void OutputCalibrationUtil::setChannelOutputValue(quint16 value)
{
    if (c_prepared) {
        ActuatorCommand *actuatorCommand = getActuatorCommandObject();
        ActuatorCommand::DataFields data = actuatorCommand->getData();
        foreach (quint32 channel, m_outputChannels) {
            // Set output value
            if (channel <= ActuatorCommand::CHANNEL_NUMELEM){
                qDebug() << "OutputCalibrationUtil setting output value for channel " << channel << " to " << value << ".";
                data.Channel[channel] = value;
            } else {
                qDebug() << "OutputCalibrationUtil could not set output value for channel " << channel
                         << " to " << value << "." << "Channel out of bounds" << channel << ".";
            }
        }
        actuatorCommand->setData(data);
    } else {
        qDebug() << "OutputCalibrationUtil not started.";
    }
}

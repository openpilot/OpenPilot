/**
 ******************************************************************************
 *
 * @file       gcscontrolgadgetconfiguration.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup GCSControlGadgetPlugin GCSControl Gadget Plugin
 * @{
 * @brief A gadget to control the UAV, either from the keyboard or a joystick
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

#include "gcscontrolgadgetconfiguration.h"

/**
 * Loads a saved configuration or defaults if non exist.
 *
 */
GCSControlGadgetConfiguration::GCSControlGadgetConfiguration(QString classId, QSettings* qSettings, QObject *parent) :
    IUAVGadgetConfiguration(classId, parent),
    rollChannel(-1),
    pitchChannel(-1),
    yawChannel(-1),
    throttleChannel(-1)
{
    int i;
    for (i=0;i<8;i++)
    {
        buttonSettings[i].ActionID=0;
        buttonSettings[i].FunctionID=0;
        buttonSettings[i].Amount=0;
        channelReverse[i] = 0;
    }
    //if a saved configuration exists load it
    if(qSettings != 0) {
        controlsMode = qSettings->value("controlsMode").toInt();
        rollChannel = qSettings->value("rollChannel").toInt();
        pitchChannel = qSettings->value("pitchChannel").toInt();
        yawChannel = qSettings->value("yawChannel").toInt();
        throttleChannel = qSettings->value("throttleChannel").toInt();

        udp_port = qSettings->value("controlPortUDP").toUInt();
        udp_host = QHostAddress(qSettings->value("controlHostUDP").toString());

        int i;
        for (i=0;i<8;i++)
        {
            buttonSettings[i].ActionID = qSettings->value(QString().sprintf("button%dAction",i)).toInt();
            buttonSettings[i].FunctionID = qSettings->value(QString().sprintf("button%dFunction",i)).toInt();
            buttonSettings[i].Amount = qSettings->value(QString().sprintf("button%dAmount",i)).toDouble();
            channelReverse[i] = qSettings->value(QString().sprintf("channel%dReverse",i)).toBool();
        }
    }

}

void GCSControlGadgetConfiguration::setUDPControlSettings(int port, QString host)
{
    udp_port = port;
    udp_host = QHostAddress(host);
}

int GCSControlGadgetConfiguration::getUDPControlPort()
{
    return udp_port;
}
QHostAddress GCSControlGadgetConfiguration::getUDPControlHost()
{
    return udp_host;
}

void GCSControlGadgetConfiguration::setRPYTchannels(int roll, int pitch, int yaw, int throttle) {
    rollChannel = roll;
    pitchChannel = pitch;
    yawChannel = yaw;
    throttleChannel = throttle;
}

QList<int> GCSControlGadgetConfiguration::getChannelsMapping()
{
    QList<int> ql;
    ql << rollChannel << pitchChannel << yawChannel << throttleChannel;
    return ql;
}
QList<bool> GCSControlGadgetConfiguration::getChannelsReverse()
{
    QList<bool> ql;
    int i;
    for (i=0;i<8;i++)ql << channelReverse[i];

    return ql;
}

/**
 * Clones a configuration.
 *
 */
IUAVGadgetConfiguration *GCSControlGadgetConfiguration::clone()
{
    GCSControlGadgetConfiguration *m = new GCSControlGadgetConfiguration(this->classId());

    m->controlsMode = controlsMode;
    m->rollChannel = rollChannel;
    m->pitchChannel = pitchChannel;
    m->yawChannel = yawChannel;
    m->throttleChannel = throttleChannel;

    m->udp_host = udp_host;
    m->udp_port = udp_port;

    int i;
    for (i=0;i<8;i++)
    {
        m->buttonSettings[i].ActionID = buttonSettings[i].ActionID;
        m->buttonSettings[i].FunctionID = buttonSettings[i].FunctionID;
        m->buttonSettings[i].Amount = buttonSettings[i].Amount;
        m->channelReverse[i] = channelReverse[i];
    }


    return m;
}

/**
 * Saves a configuration.
 *
 */
void GCSControlGadgetConfiguration::saveConfig(QSettings* settings) const {
    settings->setValue("controlsMode", controlsMode);
    settings->setValue("rollChannel", rollChannel);
    settings->setValue("pitchChannel", pitchChannel);
    settings->setValue("yawChannel", yawChannel);
    settings->setValue("throttleChannel", throttleChannel);

    settings->setValue("controlPortUDP",QString::number(udp_port));
    settings->setValue("controlHostUDP",udp_host.toString());

    int i;
    for (i=0;i<8;i++)
    {
        settings->setValue(QString().sprintf("button%dAction",i), buttonSettings[i].ActionID);
        settings->setValue(QString().sprintf("button%dFunction",i), buttonSettings[i].FunctionID);
        settings->setValue(QString().sprintf("button%dAmount",i), buttonSettings[i].Amount);
        settings->setValue(QString().sprintf("channel%dReverse",i), channelReverse[i]);
    }

}

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
    //if a saved configuration exists load it
    if(qSettings != 0) {
        controlsMode = qSettings->value("controlsMode").toInt();
        rollChannel = qSettings->value("rollChannel").toInt();
        pitchChannel = qSettings->value("pitchChannel").toInt();
        yawChannel = qSettings->value("yawChannel").toInt();
        throttleChannel = qSettings->value("throttleChannel").toInt();
    }

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
}

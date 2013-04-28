/**
 ******************************************************************************
 *
 * @file       monitorgadgetconfiguration.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup TelemetryPlugin Telemetry Plugin
 * @{
 * @brief A gadget that displays a 3D representation of the UAV
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

#include "monitorgadgetconfiguration.h"

MonitorGadgetConfiguration::MonitorGadgetConfiguration(QString classId, QSettings *qSettings, QObject *parent) :
    IUAVGadgetConfiguration(classId, parent)
{
    // if a saved configuration exists load it
    if (qSettings != 0) {
    }
}

IUAVGadgetConfiguration *MonitorGadgetConfiguration::clone()
{
    MonitorGadgetConfiguration *mv = new MonitorGadgetConfiguration(this->classId());

    return mv;
}

/**
 * Saves a configuration.
 *
 */
void MonitorGadgetConfiguration::saveConfig(QSettings *qSettings) const
{
//    qSettings->setValue("acFilename", Utils::PathUtils().RemoveDataPath(m_acFilename));
//    qSettings->setValue("bgFilename", Utils::PathUtils().RemoveDataPath(m_bgFilename));
//    qSettings->setValue("enableVbo", m_enableVbo);
}

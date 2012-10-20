/**
 ******************************************************************************
 *
 * @file       serialpluginconfiguration.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @see        The GNU Public License (GPL) Version 3
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup SerialPlugin Serial Connection Plugin
 * @{
 * @brief Impliments serial connection to the flight hardware for Telemetry
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

#include "serialpluginconfiguration.h"
#include "utils/pathutils.h"
#include <coreplugin/icore.h>

/**
 * Loads a saved configuration or defaults if non exist.
 *
 */
SerialPluginConfiguration::SerialPluginConfiguration(QString classId, QSettings* qSettings, QObject *parent) :
    IUAVGadgetConfiguration(classId, parent),
    m_speed("57600")
{
    Q_UNUSED(qSettings);

    settings = Core::ICore::instance()->settings();
}

/**
 * Clones a configuration.
 *
 */
IUAVGadgetConfiguration *SerialPluginConfiguration::clone()
{
    SerialPluginConfiguration *m = new SerialPluginConfiguration(this->classId());
        m->m_speed=m_speed;
    return m;
}

/**
 * Saves a configuration.
 *
 */
void SerialPluginConfiguration::saveConfig(QSettings* settings) const {
   settings->setValue("speed", m_speed);
}
void SerialPluginConfiguration::restoresettings()
{
    settings->beginGroup(QLatin1String("SerialConnection"));
    QString str=(settings->value(QLatin1String("speed"), tr("")).toString());
    if(str.isEmpty())
        m_speed="57600";
    else
        m_speed=str;
    settings->endGroup();

}
void SerialPluginConfiguration::savesettings() const
{
    settings->beginGroup(QLatin1String("SerialConnection"));
    settings->setValue(QLatin1String("speed"), m_speed);
    settings->endGroup();
}
SerialPluginConfiguration::~SerialPluginConfiguration()
{
}

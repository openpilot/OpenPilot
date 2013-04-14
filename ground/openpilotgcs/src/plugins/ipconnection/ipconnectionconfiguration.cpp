/**
 ******************************************************************************
 *
 * @file       IPconnectionconfiguration.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup IPConnPlugin IP Telemetry Plugin
 * @{
 * @brief IP Connection Plugin impliment telemetry over TCP/IP and UDP/IP
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

#include "ipconnectionconfiguration.h"
#include <coreplugin/icore.h>

IPconnectionConfiguration::IPconnectionConfiguration(QString classId, QSettings* qSettings, QObject *parent) :
    IUAVGadgetConfiguration(classId, parent),
    m_HostName("127.0.0.1"),
    m_Port(1000),
    m_UseTCP(1)
{
    Q_UNUSED(qSettings);

    settings = Core::ICore::instance()->settings();
}

IPconnectionConfiguration::~IPconnectionConfiguration()
{
}

IUAVGadgetConfiguration *IPconnectionConfiguration::clone()
{
    IPconnectionConfiguration *m = new IPconnectionConfiguration(this->classId());
    m->m_Port = m_Port;
    m->m_HostName = m_HostName;
    m->m_UseTCP = m_UseTCP;
    return m;
}

/**
 * Saves a configuration.
 *
 */
void IPconnectionConfiguration::saveConfig(QSettings* qSettings) const {
   qSettings->setValue("port", m_Port);
   qSettings->setValue("hostName", m_HostName);
   qSettings->setValue("useTCP", m_UseTCP);
}

void IPconnectionConfiguration::savesettings() const
{
    settings->beginGroup(QLatin1String("IPconnection"));

        settings->beginWriteArray("Current");
        settings->setArrayIndex(0);
        settings->setValue(QLatin1String("HostName"), m_HostName);
        settings->setValue(QLatin1String("Port"), m_Port);
        settings->setValue(QLatin1String("UseTCP"), m_UseTCP);
        settings->endArray();
        settings->endGroup();
}


void IPconnectionConfiguration::restoresettings()
{
    settings->beginGroup(QLatin1String("IPconnection"));

        settings->beginReadArray("Current");
        settings->setArrayIndex(0);
        m_HostName = (settings->value(QLatin1String("HostName"), tr("")).toString());
        m_Port = (settings->value(QLatin1String("Port"), tr("")).toInt());
        m_UseTCP = (settings->value(QLatin1String("UseTCP"), tr("")).toInt());
        settings->endArray();
        settings->endGroup();


}


/**
 ******************************************************************************
 *
 * @file       IPconnectionconfiguration.h
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

#ifndef IPconnectionCONFIGURATION_H
#define IPconnectionCONFIGURATION_H

#include <coreplugin/iuavgadgetconfiguration.h>
#include <QtCore/QString>
#include <QtCore/QSettings>

using namespace Core;

class IPconnectionConfiguration : public IUAVGadgetConfiguration
{
Q_OBJECT
Q_PROPERTY(QString HostName READ HostName WRITE setHostName)
Q_PROPERTY(int Port READ Port WRITE setPort)
Q_PROPERTY(int UseTCP READ UseTCP WRITE setUseTCP)

public:
    explicit IPconnectionConfiguration(QString classId, QSettings* qSettings = 0, QObject *parent = 0);

    virtual ~IPconnectionConfiguration();
    void saveConfig(QSettings* settings) const;
    //void savesettings(QSettings* settings) const;
    //void restoresettings(QSettings* settings);
    void savesettings() const;
    void restoresettings();
     IUAVGadgetConfiguration *clone();

    QString HostName() const { return m_HostName; }
    int Port() const { return m_Port; }
    int UseTCP() const { return m_UseTCP; }


public slots:
    void setHostName(QString HostName) { m_HostName = HostName; }
    void setPort(int Port) { m_Port = Port; }
    void setUseTCP(int UseTCP) { m_UseTCP = UseTCP; }

private:
    QString m_HostName;
    int m_Port;
    int m_UseTCP;
    QSettings* settings;


};

#endif // IPconnectionCONFIGURATION_H

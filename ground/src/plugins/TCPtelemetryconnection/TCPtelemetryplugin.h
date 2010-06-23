/**
 ******************************************************************************
 *
 * @file       TCPtelemetryplugin.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief
 * @see        The GNU Public License (GPL) Version 3
 * @defgroup   TCPtelemetry_plugin
 * @{
 *
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

#ifndef TCPtelemetryPLUGIN_H
#define TCPtelemetryPLUGIN_H

#include "TCPtelemetry_global.h"
#include "TCPtelemetryoptionspage.h"
#include "TCPtelemetryconfiguration.h"
#include "TCPtelemetryfactory.h"
#include "coreplugin/iconnection.h"
#include <extensionsystem/iplugin.h>
class QTcpSocket;

class IConnection;
/**
*   Define a connection via the IConnection interface
*   Plugin will add a instance of this class to the pool,
*   so the connection manager can use it.
*/
class TCPtelemetry_EXPORT TCPtelemetryConnection
    : public Core::IConnection
{
    Q_OBJECT
public:
    TCPtelemetryConnection();
    virtual ~TCPtelemetryConnection();

    virtual QStringList availableDevices();
    virtual QIODevice *openDevice(const QString &deviceName);
    virtual void closeDevice(const QString &deviceName);

    virtual QString connectionName();
    virtual QString shortName();

protected slots:
    void onEnumerationChanged();
private:
       QTcpSocket *tcpSocket;

};


class TCPtelemetry_EXPORT TCPtelemetryPlugin
    : public ExtensionSystem::IPlugin
{
    Q_OBJECT

public:
    TCPtelemetryPlugin();
    ~TCPtelemetryPlugin();

    virtual bool initialize(const QStringList &arguments, QString *error_message);
    virtual void extensionsInitialized();

private:
    //TCPtelemetryConfiguration *m_config;
    //TCPtelemetryOptionsPage *m_optionspage;
    TCPtelemetryFactory *m_factory;
    TCPtelemetryConnection *m_connection;

};


#endif // TCPtelemetryPLUGIN_H

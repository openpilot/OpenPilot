/**
 ******************************************************************************
 *
 * @file       IPconnectionplugin.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      Register connection object for the core connection manager
 * @see        The GNU Public License (GPL) Version 3
 * @defgroup   IPconnection_plugin
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

//The core of this plugin has been directly copied from the serial plugin and converted to work over a TCP link instead of a direct serial link

#include "ipconnectionplugin.h"


#include <extensionsystem/pluginmanager.h>
#include <coreplugin/icore.h>

#include <QtCore/QtPlugin>
#include <QtGui/QMainWindow>
#include <QtGui/QMessageBox>
#include <QtNetwork/QAbstractSocket>
#include <QtNetwork/QTcpSocket>
#include <QtNetwork/QUdpSocket>

#include <QDebug>

IPconnectionConnection::IPconnectionConnection()
{
    //create all our objects
    m_config = new IPconnectionConfiguration("IP Network Telemetry", NULL, this);
    m_config->restoresettings();

    m_optionspage = new IPconnectionOptionsPage(m_config,this);

    //just signal whenever we have a device event...
    QMainWindow *mw = Core::ICore::instance()->mainWindow();
    QObject::connect(mw, SIGNAL(deviceChange()),
                     this, SLOT(onEnumerationChanged()));
    QObject::connect(m_optionspage, SIGNAL(availableDevChanged()),
                     this, SLOT(onEnumerationChanged()));
}

IPconnectionConnection::~IPconnectionConnection()
{
    //tcpSocket->close ();
    //delete(tcpSocket);
}

void IPconnectionConnection::onEnumerationChanged()
{//no change from serial plugin
    emit availableDevChanged(this);
}



QStringList IPconnectionConnection::availableDevices()
{
    QStringList list;

    //we only have one "device" as defined by the configuration m_config
    list.append((const QString )m_config->HostName());

    return list;
}

QIODevice *IPconnectionConnection::openDevice(const QString &deviceName)
{
           const int Timeout = 5 * 1000;
            int state;
            QString HostName;
            int Port;
            QMessageBox msgBox;

            if (m_config->UseTCP()) {
                tcpSocket = new QTcpSocket(this);
            } else {
                tcpSocket = new QUdpSocket(this);
            }

            //get the configuration info
            HostName = m_config->HostName();
            Port = m_config->Port();

            //do sanity check on hostname and port...
            if((HostName.length()==0)||(Port<1)){
                msgBox.setText((const QString )"Please configure Host and Port options before opening the connection");
                msgBox.exec();

            }
            else {
                //try to connect...
                tcpSocket->connectToHost((const QString )HostName, Port);

                //in blocking mode so we wait for the connection to succeed
                if (tcpSocket->waitForConnected(Timeout)) {
                    return tcpSocket;
                }
                //tell user something went wrong
                msgBox.setText((const QString )tcpSocket->errorString ());
                msgBox.exec();
            }
/* BUGBUG TODO - returning null here leads to segfault because some caller still calls disconnect without checking our return value properly
 * someone needs to debug this, I got lost in the calling chain.*/
    //return NULL;
    return tcpSocket;
}

void IPconnectionConnection::closeDevice(const QString &deviceName)
{
    //still having problems with the app crashing when we reference the tcpsocket outside the openDevice function...
    //tcpSocket->close ();
    //delete(tcpSocket);
    // Note: CorvusCorax thinks its because the socket got deleted by the caller before this close() is even called
    // so we end up with a dangling reference! Is this a bug?

}


QString IPconnectionConnection::connectionName()
{//updated from serial plugin
    return QString("Network telemetry port");
}

QString IPconnectionConnection::shortName()
{//updated from serial plugin
    if (m_config->UseTCP()) {
        return QString("TCP");
    } else {
        return QString("UDP");
    }
}


IPconnectionPlugin::IPconnectionPlugin()
{//no change from serial plugin
}

IPconnectionPlugin::~IPconnectionPlugin()
{//manually remove the options page object
    removeObject(m_connection->Optionspage());
}

void IPconnectionPlugin::extensionsInitialized()
{
    addAutoReleasedObject(m_connection);
}

bool IPconnectionPlugin::initialize(const QStringList &arguments, QString *errorString)
{
    Q_UNUSED(arguments);
    Q_UNUSED(errorString);
    m_connection = new IPconnectionConnection();
    //must manage this registration of child object ourselves
    //if we use an autorelease here it causes the GCS to crash
    //as it is deleting objects as the app closes...
    addObject(m_connection->Optionspage());

    return true;
}

Q_EXPORT_PLUGIN(IPconnectionPlugin)

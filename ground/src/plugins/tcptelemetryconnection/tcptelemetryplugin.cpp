/**
 ******************************************************************************
 *
 * @file       TCPtelemetryplugin.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      Register connection object for the core connection manager
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

//The core of this plugin has been directly copied from the serial plugin and converted to work over a TCP link instead of a direct serial link

#include "tcptelemetryplugin.h"


#include <extensionsystem/pluginmanager.h>
#include <coreplugin/icore.h>

#include <QtCore/QtPlugin>
#include <QtGui/QMainWindow>
#include <QtGui/QMessageBox>
#include <QtNetwork/QTcpSocket>

#include <QDebug>

TCPtelemetryConnection::TCPtelemetryConnection()
{
    //create all our objects
    m_config = new TCPtelemetryConfiguration("TCP Connection", NULL, this);
    m_config->restoresettings();

    m_optionspage = new TCPtelemetryOptionsPage(m_config,this);

    //just signal whenever we have a device event...
    QMainWindow *mw = Core::ICore::instance()->mainWindow();
    QObject::connect(mw, SIGNAL(deviceChange()),
                     this, SLOT(onEnumerationChanged()));
    QObject::connect(m_optionspage, SIGNAL(availableDevChanged()),
                     this, SLOT(onEnumerationChanged()));
}

TCPtelemetryConnection::~TCPtelemetryConnection()
{
    //tcpSocket->close ();
    //delete(tcpSocket);
}

void TCPtelemetryConnection::onEnumerationChanged()
{//no change from serial plugin
    emit availableDevChanged(this);
}



QStringList TCPtelemetryConnection::availableDevices()
{
    QStringList list;

    //we only have one "device" as defined by the configuration m_config
    list.append((const QString )m_config->HostName());

    return list;
}

QIODevice *TCPtelemetryConnection::openDevice(const QString &deviceName)
{
           const int Timeout = 5 * 1000;
            int state;
            QString HostName;
            int Port;
            QMessageBox msgBox;

            tcpSocket = new QTcpSocket(this);

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
    return NULL;
}

void TCPtelemetryConnection::closeDevice(const QString &deviceName)
{
    //still having problems with the app crashing when we reference the tcpsocket outside the openDevice function...
    //tcpSocket->close ();
    //delete(tcpSocket);

}


QString TCPtelemetryConnection::connectionName()
{//updated from serial plugin
    return QString("TCP telemetry port");
}

QString TCPtelemetryConnection::shortName()
{//updated from serial plugin
    return QString("TCP");
}


TCPtelemetryPlugin::TCPtelemetryPlugin()
{//no change from serial plugin
}

TCPtelemetryPlugin::~TCPtelemetryPlugin()
{//manually remove the options page object
    removeObject(m_connection->Optionspage());
}

void TCPtelemetryPlugin::extensionsInitialized()
{
    addAutoReleasedObject(m_connection);
}

bool TCPtelemetryPlugin::initialize(const QStringList &arguments, QString *errorString)
{
    Q_UNUSED(arguments);
    Q_UNUSED(errorString);
    m_connection = new TCPtelemetryConnection();
    //must manage this registration of child object ourselves
    //if we use an autorelease here it causes the GCS to crash
    //as it is deleting objects as the app closes...
    addObject(m_connection->Optionspage());

    return true;
}

Q_EXPORT_PLUGIN(TCPtelemetryPlugin)

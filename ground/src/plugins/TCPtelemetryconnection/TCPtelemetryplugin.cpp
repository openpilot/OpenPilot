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

#include "TCPtelemetryplugin.h"


#include <extensionsystem/pluginmanager.h>
#include <coreplugin/icore.h>

#include <QtCore/QtPlugin>
#include <QtGui/QMainWindow>
#include <QtGui/QMessageBox>
#include <QtNetwork/QTcpSocket>

#include <QDebug>

TCPtelemetryConnection::TCPtelemetryConnection()
{
    tcpSocket = new QTcpSocket(this);

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
    /* if (tcpSocket->state()>0){
         tcpSocket->disconnectFromHost ();
     }*/
    tcpSocket->close ();


   // delete(m_optionspage);
   // delete(m_config);

}

void TCPtelemetryConnection::onEnumerationChanged()
{//no change from serial plugin


    emit availableDevChanged(this);
}



QStringList TCPtelemetryConnection::availableDevices()
{
    QStringList list;

    list.append((const QString )m_config->HostName());

    return list;
}

QIODevice *TCPtelemetryConnection::openDevice(const QString &deviceName)
{
           const int Timeout = 5 * 1000;


            //if (tcpSocket->state()>0){
            //    tcpSocket->close();
            //}
            tcpSocket->connectToHost((const QString )m_config->HostName(), m_config->Port());

            if (tcpSocket->waitForConnected(Timeout)) {
                return tcpSocket;
            }

            QMessageBox msgBox;
             msgBox.setText((const QString )tcpSocket->errorString ());
             msgBox.exec();
    return NULL;
}

void TCPtelemetryConnection::closeDevice(const QString &deviceName)
{
   /* if (tcpSocket->state()>0){
        tcpSocket->disconnectFromHost ();
    }*/
//tcpSocket->close();
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
{//no change from serial plugin


}

void TCPtelemetryPlugin::extensionsInitialized()
{//updated from serial plugin

    addAutoReleasedObject(m_connection);
}

bool TCPtelemetryPlugin::initialize(const QStringList &arguments, QString *errorString)
{//no change from serial plugin

    Q_UNUSED(arguments);
    Q_UNUSED(errorString);
    m_connection = new TCPtelemetryConnection();
    addAutoReleasedObject(m_connection->Optionspage());

    return true;
}

Q_EXPORT_PLUGIN(TCPtelemetryPlugin)

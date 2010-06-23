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
#include <QtNetwork/QTcpSocket>

#include <QDebug>

TCPtelemetryConnection::TCPtelemetryConnection()
{//no change from serial plugin

    //I'm cheating a little bit here:
    //Knowing if the device enumeration really changed is a bit complicated
    //so I just signal it whenever we have a device event...
    QMainWindow *mw = Core::ICore::instance()->mainWindow();
    QObject::connect(mw, SIGNAL(deviceChange()),
                     this, SLOT(onEnumerationChanged()));
}

TCPtelemetryConnection::~TCPtelemetryConnection()
{//no change from serial plugin


}

void TCPtelemetryConnection::onEnumerationChanged()
{//no change from serial plugin


    emit availableDevChanged(this);
}

/*bool sortPorts(const QextPortInfo &s1,const QextPortInfo &s2)
{
    return s1.portName<s2.portName;
}*/

QStringList TCPtelemetryConnection::availableDevices()
{
    QStringList list;
    /*QList<QextPortInfo> ports = QextTCPtelemetryEnumerator::getPorts();

    //sort the list by port number (nice idea from PT_Dreamer :))
    qSort(ports.begin(), ports.end(),sortPorts);
    foreach( QextPortInfo port, ports ) {
       list.append(port.friendName);
    }*/
    //for the first attempt just hard code the IP and PORT
    list.append((const QString )"Test OpenPilot");

    return list;
}

QIODevice *TCPtelemetryConnection::openDevice(const QString &deviceName)
{

    /*QList<QextPortInfo> ports = QextTCPtelemetryEnumerator::getPorts();
    foreach( QextPortInfo port, ports ) {
           if(port.friendName == deviceName)
            {
            //we need to handle port settings here...
            PortSettings set;
            set.BaudRate = BAUD57600;
            set.DataBits = DATA_8;
            set.Parity = PAR_NONE;
            set.StopBits = STOP_1;
            set.FlowControl = FLOW_OFF;
            set.Timeout_Millisec = 500;
#ifdef Q_OS_WIN
            return new QextTCPtelemetryPort(port.portName, set);
#else
            return new QextTCPtelemetryPort(port.physName, set);
#endif
        }
    }*/
           const int Timeout = 5 * 1000;


            tcpSocket = new QTcpSocket(this);

            tcpSocket->connectToHost((const QString )"192.168.10.77", 9100);

            if (tcpSocket->waitForConnected(Timeout)) {
                return tcpSocket;
            }


    return NULL;
}

void TCPtelemetryConnection::closeDevice(const QString &deviceName)
{//no change from serial plugin


    //nothing to do here
}


QString TCPtelemetryConnection::connectionName()
{//updated from serial plugin


    return QString("TCPtelemetry port");
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
    //m_optionspage = new TCPtelemetryOptionsPage(NULL,this);
    //addAutoReleasedObject(m_optionspage);
    m_connection = new TCPtelemetryConnection();
    //m_factory = new TCPtelemetryFactory(this);
    //addAutoReleasedObject(m_factory);

    return true;
}

Q_EXPORT_PLUGIN(TCPtelemetryPlugin)

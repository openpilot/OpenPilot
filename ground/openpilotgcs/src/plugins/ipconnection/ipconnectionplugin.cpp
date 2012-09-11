/**
 ******************************************************************************
 *
 * @file       IPconnectionplugin.cpp
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

//The core of this plugin has been directly copied from the serial plugin and converted to work over a TCP link instead of a direct serial link

#include "ipconnectionplugin.h"


#include <extensionsystem/pluginmanager.h>
#include <coreplugin/icore.h>
#include "ipconnection_internal.h"

#include <QtCore/QtPlugin>
#include <QtGui/QMainWindow>
#include <QtGui/QMessageBox>
#include <QtNetwork/QAbstractSocket>
#include <QtNetwork/QTcpSocket>
#include <QtNetwork/QUdpSocket>
#include <QWaitCondition>
#include <QMutex>
#include <coreplugin/threadmanager.h>

#include <QDebug>

//Communication between IPconnectionConnection::OpenDevice() and IPConnection::onOpenDevice()
QString errorMsg;
QWaitCondition openDeviceWait;
QWaitCondition closeDeviceWait;
//QReadWriteLock dummyLock;
QMutex ipConMutex;
QAbstractSocket *ret;

IPConnection::IPConnection(IPconnectionConnection *connection) : QObject()
{
    moveToThread(Core::ICore::instance()->threadManager()->getRealTimeThread());

    QObject::connect(connection, SIGNAL(CreateSocket(QString,int,bool)),
                     this, SLOT(onOpenDevice(QString,int,bool)));
    QObject::connect(connection, SIGNAL(CloseSocket(QAbstractSocket*)),
                     this, SLOT(onCloseDevice(QAbstractSocket*)));
}

/*IPConnection::~IPConnection()
{

}*/

void IPConnection::onOpenDevice(QString HostName, int Port, bool UseTCP)
{
    QAbstractSocket *ipSocket;
    const int Timeout = 5 * 1000;

    ipConMutex.lock();
    if (UseTCP) {
        ipSocket = new QTcpSocket(this);
    } else {
        ipSocket = new QUdpSocket(this);
    }

    //do sanity check on hostname and port...
    if((HostName.length()==0)||(Port<1)){
        errorMsg = "Please configure Host and Port options before opening the connection";

    }
    else {
        //try to connect...
        ipSocket->connectToHost(HostName, Port);

        //in blocking mode so we wait for the connection to succeed
        if (ipSocket->waitForConnected(Timeout)) {
            ret = ipSocket;
            openDeviceWait.wakeAll();
            ipConMutex.unlock();
            return;
        }
        //tell user something went wrong
        errorMsg = ipSocket->errorString ();
    }
    /* BUGBUG TODO - returning null here leads to segfault because some caller still calls disconnect without checking our return value properly
    * someone needs to debug this, I got lost in the calling chain.*/
    ret = NULL;
    openDeviceWait.wakeAll();
    ipConMutex.unlock();
}

void IPConnection::onCloseDevice(QAbstractSocket *ipSocket)
{
    ipConMutex.lock();
    ipSocket->close ();
    delete(ipSocket);
    closeDeviceWait.wakeAll();
    ipConMutex.unlock();
}


IPConnection * connection = 0;

IPconnectionConnection::IPconnectionConnection()
{
    ipSocket = NULL;
    //create all our objects
    m_config = new IPconnectionConfiguration("IP Network Telemetry", NULL, this);
    m_config->restoresettings();

    m_optionspage = new IPconnectionOptionsPage(m_config,this);

    if(!connection)
        connection = new IPConnection(this);

    //just signal whenever we have a device event...
    QMainWindow *mw = Core::ICore::instance()->mainWindow();
    QObject::connect(mw, SIGNAL(deviceChange()),
                     this, SLOT(onEnumerationChanged()));
    QObject::connect(m_optionspage, SIGNAL(availableDevChanged()),
                     this, SLOT(onEnumerationChanged()));
}

IPconnectionConnection::~IPconnectionConnection()
{//clean up out resources...
    if (ipSocket){
        ipSocket->close ();
        delete(ipSocket);
    }
    if(connection)
    {
        delete connection;
        connection = NULL;
    }
}

void IPconnectionConnection::onEnumerationChanged()
{//no change from serial plugin
    emit availableDevChanged(this);
}



QList <Core::IConnection::device> IPconnectionConnection::availableDevices()
{
    QList <Core::IConnection::device> list;
    device d;
    if (m_config->HostName().length()>1)
        d.displayName=(const QString )m_config->HostName();
    else
        d.displayName="Unconfigured";
    d.name=(const QString )m_config->HostName();
    //we only have one "device" as defined by the configuration m_config
    list.append(d);

    return list;
}

QIODevice *IPconnectionConnection::openDevice(const QString &)
{
    QString HostName;
    int Port;
    bool UseTCP;
    QMessageBox msgBox;

    //get the configuration info
    HostName = m_config->HostName();
    Port = m_config->Port();
    UseTCP = m_config->UseTCP();

    if (ipSocket){
        //Andrew: close any existing socket... this should never occur
        ipConMutex.lock();
        emit CloseSocket(ipSocket);
        closeDeviceWait.wait(&ipConMutex);
        ipConMutex.unlock();
        ipSocket = NULL;
    }

    ipConMutex.lock();
    emit CreateSocket(HostName, Port, UseTCP);
    openDeviceWait.wait(&ipConMutex);
    ipConMutex.unlock();
    ipSocket = ret;
    if(ipSocket == NULL)
    {
        msgBox.setText((const QString )errorMsg);
        msgBox.exec();
    }
    return ipSocket;
}

void IPconnectionConnection::closeDevice(const QString &)
{
    if (ipSocket){
        ipConMutex.lock();
        emit CloseSocket(ipSocket);
        closeDeviceWait.wait(&ipConMutex);
        ipConMutex.unlock();
        ipSocket = NULL;
    }
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

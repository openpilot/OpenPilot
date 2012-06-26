/**
 ******************************************************************************
 *
 * @file       serialplugin.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
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

#include "serialplugin.h"

#include <extensionsystem/pluginmanager.h>
#include <coreplugin/icore.h>

#include <QtCore/QtPlugin>
#include <QtGui/QMainWindow>

#include <QDebug>



SerialEnumerationThread::SerialEnumerationThread(SerialConnection *serial)
    : m_serial(serial),
    m_running(true)
{
}

SerialEnumerationThread::~SerialEnumerationThread()
{
    m_running = false;
    //wait for the thread to terminate
    if(wait(2100) == false)
        qDebug() << "Cannot terminate SerialEnumerationThread";
}

void SerialEnumerationThread::run()
{
    QList <Core::IConnection::device> devices = m_serial->availableDevices();

    while(m_running)
    {
        if(!m_serial->deviceOpened())
        {
            QList <Core::IConnection::device> newDev = m_serial->availableDevices();
            if(devices != newDev)
            {
                devices = newDev;
                emit enumerationChanged();
            }
        }

        msleep(2000); //update available devices every two seconds (doesn't need more)
    }
}


SerialConnection::SerialConnection()
    : enablePolling(true), m_enumerateThread(this)
{
    serialHandle = NULL;
    m_config = new SerialPluginConfiguration("Serial Telemetry", NULL, this);
    m_config->restoresettings();

    m_optionspage = new SerialPluginOptionsPage(m_config,this);


    // Experimental: enable polling on all OS'es since there
    // were reports that autodetect does not work on XP amongst
    // others.

    //#ifdef Q_OS_WIN
//    //I'm cheating a little bit here:
//    //Knowing if the device enumeration really changed is a bit complicated
//    //so I just signal it whenever we have a device event...
//    QMainWindow *mw = Core::ICore::instance()->mainWindow();
//    QObject::connect(mw, SIGNAL(deviceChange()),
//                     this, SLOT(onEnumerationChanged()));
//#else
    // Other OSes do not send such signals:
    QObject::connect(&m_enumerateThread, SIGNAL(enumerationChanged()),
                     this, SLOT(onEnumerationChanged()));
    m_enumerateThread.start();
//#endif
}

SerialConnection::~SerialConnection()
{
}

void SerialConnection::onEnumerationChanged()
{
    if (enablePolling)
        emit availableDevChanged(this);
}

bool sortPorts(const QextPortInfo &s1,const QextPortInfo &s2)
{
    return s1.portName<s2.portName;
}

QList <Core::IConnection::device> SerialConnection::availableDevices()
{
    QList <Core::IConnection::device> list;

    if (enablePolling) {
        QList<QextPortInfo> ports = QextSerialEnumerator::getPorts();

        //sort the list by port number (nice idea from PT_Dreamer :))
        qSort(ports.begin(), ports.end(),sortPorts);
        foreach( QextPortInfo port, ports ) {
           device d;
           d.displayName=port.friendName;
           d.name=port.physName;
           list.append(d);
        }
    }

    return list;
}

QIODevice *SerialConnection::openDevice(const QString &deviceName)
{
    if (serialHandle){
        closeDevice(deviceName);
    }
    QList<QextPortInfo> ports = QextSerialEnumerator::getPorts();
    foreach( QextPortInfo port, ports ) {
           if(port.physName == deviceName)
            {
            //we need to handle port settings here...
            PortSettings set;
            set.BaudRate = stringToBaud(m_config->speed());
            qDebug()<<"Serial telemetry running at "<<m_config->speed();
            set.DataBits = DATA_8;
            set.Parity = PAR_NONE;
            set.StopBits = STOP_1;
            set.FlowControl = FLOW_OFF;
            set.Timeout_Millisec = 500;
#ifdef Q_OS_WIN
            serialHandle = new QextSerialPort(port.portName, set);
#else
            serialHandle = new QextSerialPort(port.physName, set);
#endif
            m_deviceOpened = true;
            return serialHandle;
        }
    }
    return NULL;
}

void SerialConnection::closeDevice(const QString &deviceName)
{
    Q_UNUSED(deviceName);
    //we have to delete the serial connection we created
    if (serialHandle){
        serialHandle->deleteLater();
        serialHandle = NULL;
        m_deviceOpened = false;
    }
}


QString SerialConnection::connectionName()
{
    return QString("Serial port");
}

QString SerialConnection::shortName()
{
    return QString("Serial");
}

/**
 Tells the Serial plugin to stop polling for serial devices
 */
void SerialConnection::suspendPolling()
{
    enablePolling = false;
}

/**
 Tells the Serial plugin to resume polling for serial devices
 */
void SerialConnection::resumePolling()
{
    enablePolling = true;
}

BaudRateType SerialConnection::stringToBaud(QString str)
{   
    if(str=="1200")
        return BAUD1200;
    if(str=="1800")
        return BAUD1800;
    else if(str=="2400")
        return BAUD2400;
    else if(str== "4800")
        return BAUD4800;
    else if(str== "9600")
        return BAUD9600;
    else if(str== "14400")
        return BAUD14400;
    else if(str== "19200")
        return BAUD19200;
    else if(str== "38400")
        return BAUD38400;
    else if(str== "56000")
        return BAUD56000;
    else if(str== "57600")
        return BAUD57600;
    else if(str== "76800")
        return BAUD76800;
    else if(str== "115200")
        return BAUD115200;
    else if(str== "128000")
        return BAUD128000;
    else if(str== "230400")
        return BAUD230400;
    else if(str== "256000")
        return BAUD256000;
    else if(str== "460800")
        return BAUD460800;
    else if(str== "921600")
        return BAUD921600;
    else
        return BAUD57600;
}

SerialPlugin::SerialPlugin()
{
}

SerialPlugin::~SerialPlugin()
{
    removeObject(m_connection->Optionspage());
}

void SerialPlugin::extensionsInitialized()
{
    addAutoReleasedObject(m_connection);
}

bool SerialPlugin::initialize(const QStringList &arguments, QString *errorString)
{
    Q_UNUSED(arguments);
    Q_UNUSED(errorString);
    m_connection = new SerialConnection();
    //must manage this registration of child object ourselves
    //if we use an autorelease here it causes the GCS to crash
    //as it is deleting objects as the app closes...
    addObject(m_connection->Optionspage());
    return true;
}

Q_EXPORT_PLUGIN(SerialPlugin)

/**
 ******************************************************************************
 *
 * @file       rawhidplugin.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      Register connection object for the core connection manager
 * @see        The GNU Public License (GPL) Version 3
 * @defgroup   rawhid_plugin
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

#include "rawhidplugin.h"
#include "rawhid.h"
#include <extensionsystem/pluginmanager.h>

#include <QtCore/QtPlugin>
#include <QtCore/QMutexLocker>

#include "pjrc_rawhid.h"

#include "rawhid_const.h"


RawHIDEnumerationThread::RawHIDEnumerationThread(RawHIDConnection *rawhid)
    : m_rawhid(rawhid),
    m_running(true)
{
}

RawHIDEnumerationThread::~RawHIDEnumerationThread()
{
    m_running = false;
    //wait for the thread to terminate
    if(wait(1000) == false)
        qDebug() << "Cannot terminate rawhid thread";
}

void RawHIDEnumerationThread::run()
{
    QStringList devices = m_rawhid->availableDevices();

    while(m_running)
    {
        if(!m_rawhid->deviceOpened())
        {
            QStringList newDev = m_rawhid->availableDevices();
            if(devices != newDev)
            {
                devices = newDev;
                emit enumerationChanged();
            }
        }

        msleep(500); //update available devices twice per second
    }
}


RawHIDConnection::RawHIDConnection()
    : m_enumerateThread(this)
{
    QObject::connect(&m_enumerateThread, SIGNAL(enumerationChanged()),
                     this, SLOT(onEnumerationChanged()));
    m_enumerateThread.start();
}

RawHIDConnection::~RawHIDConnection()
{}

void RawHIDConnection::onEnumerationChanged()
{
    emit availableDevChanged(this);
}

QStringList RawHIDConnection::availableDevices()
{
    QMutexLocker locker(&m_enumMutex);

    QStringList devices;
    pjrc_rawhid dev;

    //open all device we can
    int opened = dev.open(MAX_DEVICES, VID, PID, USAGE_PAGE, USAGE);

    //for each devices found, get serial number and close it back
    for(int i=0; i<opened; i++)
    {
        char serial[256];
        dev.getserial(i, serial);
        devices.append(QString::fromAscii(serial, DEV_SERIAL_LEN));
        dev.close(i);
    }

    return devices;
}

QIODevice *RawHIDConnection::openDevice(const QString &deviceName)
{
    m_deviceOpened = true;
    return new RawHID(deviceName);
}

void RawHIDConnection::closeDevice(const QString &deviceName)
{
    m_deviceOpened = false;
}


QString RawHIDConnection::connectionName()
{
    return QString("Raw HID USB");
}

QString RawHIDConnection::shortName()
{
    return QString("USB");
}





RawHIDPlugin::RawHIDPlugin()
{
}

RawHIDPlugin::~RawHIDPlugin()
{

}

void RawHIDPlugin::extensionsInitialized()
{
    addAutoReleasedObject(new RawHIDConnection);
}

bool RawHIDPlugin::initialize(const QStringList & arguments, QString * errorString)
{
    Q_UNUSED(arguments);
    Q_UNUSED(errorString);

    return true;
}


Q_EXPORT_PLUGIN(RawHIDPlugin)

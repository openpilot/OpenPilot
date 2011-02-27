/**
 ******************************************************************************
 *
 * @file       rawhidplugin.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup RawHIDPlugin Raw HID Plugin
 * @{
 * @brief Impliments a HID USB connection to the flight hardware as a QIODevice
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

// **********************************************************************

RawHIDEnumerationThread::RawHIDEnumerationThread(RawHIDConnection *rawhid) :
	QThread(rawhid),	// Pip
	m_rawhid(rawhid),
    m_running(true)
{
	if (m_rawhid)
		connect(m_rawhid, SLOT(destroyed(QObject *)), this, SLOT(onRawHidConnectionDestroyed(QObject *)));	// Pip
}

RawHIDEnumerationThread::~RawHIDEnumerationThread()
{
	mutex.lock();
		m_rawhid = NULL;	// safe guard
	mutex.unlock();

    m_running = false;

	// wait for the thread to terminate
	if (!wait(100))
        qDebug() << "Cannot terminate RawHIDEnumerationThread";
}

void RawHIDEnumerationThread::onRawHidConnectionDestroyed(QObject *obj)	// Pip
{
	QMutexLocker locker(&mutex);

	if (!m_rawhid || m_rawhid != obj)
		return;

	m_rawhid = NULL;
}

void RawHIDEnumerationThread::run()
{
    QStringList devices = m_rawhid->availableDevices();

	int counter = 0;

	while (m_running)
    {
		mutex.lock();	// Pip

		// update available devices every second (doesn't need more)
		if (m_rawhid)
		{
                        if (!m_rawhid->deviceOpened())	// this was stopping us getting enumerations changes fed back
			{
				if (++counter >= 100)
				{
					counter = 0;

					QStringList newDev = m_rawhid->availableDevices();
					if (devices != newDev)
					{
						devices = newDev;
						emit enumerationChanged();
					}
				}
			}
//			else
//				counter = 0;
		}
		else
			counter = 0;

		mutex.unlock();	// Pip

		msleep(10);
    }
}

// **********************************************************************

RawHIDConnection::RawHIDConnection()
    : m_enumerateThread(this)
{
    //added by andrew
    RawHidHandle = NULL;
    enablePolling = true;

	QObject::connect(&m_enumerateThread, SIGNAL(enumerationChanged()), this, SLOT(onEnumerationChanged()));

    m_enumerateThread.start();
}

RawHIDConnection::~RawHIDConnection()
{
	if (RawHidHandle)
	{
	}
}

void RawHIDConnection::onEnumerationChanged()
{
	if (RawHidHandle)	// Pip
	{	// check to see if the connection has closed
		if (!RawHidHandle->isOpen())
		{	// connection has closed .. hmmmm, this connection is still showing as open after the USB device is unplugged from PC
			delete RawHidHandle;
			RawHidHandle = NULL;

			emit deviceClosed(this);
		}
	}

    if (enablePolling)
        emit availableDevChanged(this);
}

QStringList RawHIDConnection::availableDevices()
{
    QMutexLocker locker(&m_enumMutex);

    QStringList devices;

    if (enablePolling) {
        pjrc_rawhid dev;
        //open all device we can
		int opened = dev.open(USB_MAX_DEVICES, USB_VID, USB_PID, USB_USAGE_PAGE, USB_USAGE);

        //for each devices found, get serial number and close it back
        for(int i=0; i<opened; i++)
        {
            devices.append(dev.getserial(i));
            dev.close(i);
        }
    }

    return devices;
}

void RawHIDConnection::onRawHidDestroyed(QObject *obj)	// Pip
{
	if (!RawHidHandle || RawHidHandle != obj)
		return;

	RawHidHandle = NULL;
}

QIODevice *RawHIDConnection::openDevice(const QString &deviceName)
{
    //added by andrew
	if (RawHidHandle)
        closeDevice(deviceName);
    //end added by andrew

    //return new RawHID(deviceName);
    RawHidHandle = new RawHID(deviceName);

	connect(RawHidHandle, SLOT(destroyed(QObject *)), this, SLOT(onRawHidDestroyed(QObject *)));	// Pip

    return RawHidHandle;
}

void RawHIDConnection::closeDevice(const QString &deviceName)
{
    Q_UNUSED(deviceName);
    //added by andrew...
	if (RawHidHandle)
	{
        RawHidHandle->close();

		delete RawHidHandle;
		RawHidHandle = NULL;

		emit deviceClosed(this);	// Pip
    }
    //end added by andrew
}

QString RawHIDConnection::connectionName()
{
    return QString("Raw HID USB");
}

QString RawHIDConnection::shortName()
{
    return QString("USB");
}

/**
 Tells the Raw HID plugin to stop polling for USB devices
 */
void RawHIDConnection::suspendPolling()
{
    enablePolling = false;
}

/**
 Tells the Raw HID plugin to resume polling for USB devices
 */
void RawHIDConnection::resumePolling()
{
    enablePolling = true;
}

// **********************************************************************

RawHIDPlugin::RawHIDPlugin()
{
	hidConnection = NULL;	// Pip
}

RawHIDPlugin::~RawHIDPlugin()
{

}

void RawHIDPlugin::extensionsInitialized()
{
	hidConnection = new RawHIDConnection();
	addAutoReleasedObject(hidConnection);

    //temp for test
    //addAutoReleasedObject(new RawHIDTestThread);
}

bool RawHIDPlugin::initialize(const QStringList & arguments, QString * errorString)
{
    Q_UNUSED(arguments);
    Q_UNUSED(errorString);

    return true;
}

Q_EXPORT_PLUGIN(RawHIDPlugin)

// **********************************************************************

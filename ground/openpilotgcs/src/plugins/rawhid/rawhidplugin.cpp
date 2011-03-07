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

RawHIDConnection::RawHIDConnection()
    : m_usbMonitor(this)
{
    //added by andrew
    RawHidHandle = NULL;
    enablePolling = true;

    connect(&m_usbMonitor, SIGNAL(deviceDiscovered(USBPortInfo)), this, SLOT(onDeviceConnected()));
    connect(&m_usbMonitor, SIGNAL(deviceRemoved(USBPortInfo)), this, SLOT(onDeviceDisconnected()));

}

RawHIDConnection::~RawHIDConnection()
{
	if (RawHidHandle)
            if (RawHidHandle->isOpen())
                RawHidHandle->close();

        m_usbMonitor.quit();
        m_usbMonitor.wait(500);
}

/**
  The USB monitor tells us a new device appeared
  */
void RawHIDConnection::onDeviceConnected()
{
    emit availableDevChanged(this);
}

/**
  The USB monitor tells us a device disappeard
  */
void RawHIDConnection::onDeviceDisconnected()
{
    emit deviceClosed(this);
    if (enablePolling)
        emit availableDevChanged(this);
}

/**
  Returns the list of all currently available devices
  */
QStringList RawHIDConnection::availableDevices()
{
    QStringList devices;

    QList<USBPortInfo> portsList = m_usbMonitor.availableDevices(0x20a0, -1, -1);
    // We currently list devices by their serial number
    foreach(USBPortInfo prt, portsList) {
        devices.append(prt.serialNumber);
    }

    return devices;
}


/// TODO: still needed ???
void RawHIDConnection::onRawHidDestroyed(QObject *obj)	// Pip
{
	if (!RawHidHandle || RawHidHandle != obj)
		return;

	RawHidHandle = NULL;
}

/// TODO: still needed ???
void RawHIDConnection::onRawHidClosed()
{
	if (RawHidHandle)
	{
		emit deviceClosed(this);
	}
}

QIODevice *RawHIDConnection::openDevice(const QString &deviceName)
{
    //added by andrew
	if (RawHidHandle)
        closeDevice(deviceName);
    //end added by andrew

    //return new RawHID(deviceName);
    RawHidHandle = new RawHID(deviceName);

	if (RawHidHandle)
	{
		connect(RawHidHandle, SIGNAL(closed()), this, SLOT(onRawHidClosed()), Qt::QueuedConnection);
		connect(RawHidHandle, SIGNAL(destroyed(QObject *)), this, SLOT(onRawHidDestroyed(QObject *)), Qt::QueuedConnection);
	}

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

		emit deviceClosed(this);
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

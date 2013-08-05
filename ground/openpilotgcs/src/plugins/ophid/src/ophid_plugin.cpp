/**
 ******************************************************************************
 *
 * @file       opHID_plugin.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2013.
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup  opHIDPlugin HID Plugin
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

#include "ophid_global.h"
#include "ophid_plugin.h"
#include <extensionsystem/pluginmanager.h>

#include <QtCore/QtPlugin>
#include <QtCore/QMutexLocker>

#include "ophid_hidapi.h"
#include "ophid_const.h"


/**
 * \brief Constructor
 *
 * \note
 *
 */
RawHIDConnection::RawHIDConnection()
{
    enablePolling = true;

    m_usbMonitor  = USBMonitor::instance();

#ifndef __APPLE__
    connect(m_usbMonitor, SIGNAL(deviceDiscovered(USBPortInfo)), this, SLOT(onDeviceConnected()));
    connect(m_usbMonitor, SIGNAL(deviceRemoved(USBPortInfo)), this, SLOT(onDeviceDisconnected()));
#else
    connect(m_usbMonitor, SIGNAL(deviceDiscovered()), this, SLOT(onDeviceConnected()));
    connect(m_usbMonitor, SIGNAL(deviceRemoved()), this, SLOT(onDeviceDisconnected()));
#endif
}


/**
 * \brief Destructor
 *
 * \note
 *
 */
RawHIDConnection::~RawHIDConnection()
{}


/**
 * \brief New device plugged
 *
 * \note The USB monitor tells us a new device appeared
 *
 */
void RawHIDConnection::onDeviceConnected()
{
    emit availableDevChanged(this);
}


/**
 * \brief Device unplugged
 *
 * \note The USB monitor tells us a new device disappeared
 *
 */
void RawHIDConnection::onDeviceDisconnected()
{
    qDebug() << "onDeviceDisconnected()";
    if (enablePolling) {
        emit availableDevChanged(this);
    }
}


/**
 * \brief Available devices
 *
 * \return List of all currently available devices
 */
QList < Core::IConnection::device> RawHIDConnection::availableDevices()
{
    QList < Core::IConnection::device> devices;

    QList<USBPortInfo> portsList = m_usbMonitor->availableDevices(USBMonitor::idVendor_OpenPilot, -1, -1, USBMonitor::Running);

    // We currently list devices by their serial number
    device dev;
    foreach(USBPortInfo prt, portsList) {
        dev.name = prt.serialNumber;
        dev.displayName = prt.product;
        devices.append(dev);
    }
    return devices;
}


/**
 * \brief Open device
 *
 * \param[in] deviceName String name of the device to open
 * \return initialized handle
 */
QIODevice *RawHIDConnection::openDevice(const QString &deviceName)
{
    OPHID_TRACE("IN");

    ophidHandle  = new opHID();

    deviceHandle = new opHID_hidapi();

    nb_of_devices_opened = deviceHandle->open(USB_MAX_DEVICES,
                                              USB_VID,
                                              USB_PID_ANY,
                                              USB_USAGE_PAGE,
                                              USB_USAGE,
                                              deviceName);

    if (nb_of_devices_opened == 1) {
        ophidHandle->deviceBind(deviceHandle);
    }

    OPHID_TRACE("OUT");

    return ophidHandle;
}

/**
 * \brief device opened
 *
 * \return status of the device (opened (1) or not (0)).
 */
/*
   bool RawHIDConnection::deviceOpened()
   {
    return RawHidHandle->isOpen();
   }
 */

/**
 * \brief Close device
 *
 * \param[in] deviceName String name of the device to close
 */
void RawHIDConnection::closeDevice(const QString &deviceName)
{
    OPHID_TRACE("IN");

    Q_UNUSED(deviceName);

    ophidHandle->deviceUnbind();

    deviceHandle->close(0);

    delete deviceHandle;

    delete ophidHandle;

    OPHID_TRACE("OUT");
}


/**
 * \brief Get connection name
 *
 * \return name of the connection
 */
QString RawHIDConnection::connectionName()
{
    return QString("Raw HID USB");
}


/**
 * \brief Get shorter connection name
 *
 * \return shorter name of the connection
 */
QString RawHIDConnection::shortName()
{
    return QString("USB");
}


/**
 * \brief Suspend polling
 *
 * \note Tells the Raw HID plugin to stop polling for USB devices
 */
void RawHIDConnection::suspendPolling()
{
    enablePolling = false;
}


/**
 * \brief Resume polling
 *
 * \note Tells the Raw HID plugin to resume polling for USB devices
 */
void RawHIDConnection::resumePolling()
{
    enablePolling = true;
}


/**
 * \brief Plugin Constructor
 *
 * \note
 *
 */
RawHIDPlugin::RawHIDPlugin()
{
    hidConnection = NULL; // Pip
}


/**
 * \brief Plugin Destructor
 *
 * \note
 *
 */
RawHIDPlugin::~RawHIDPlugin()
{
    m_usbMonitor->quit();
    m_usbMonitor->wait(500);
}


/**
 * \brief Instantiate a connection
 *
 * \note
 *
 */
void RawHIDPlugin::extensionsInitialized()
{
    hidConnection = new RawHIDConnection();
    addAutoReleasedObject(hidConnection);

    // temp for test
    // addAutoReleasedObject(new RawHIDTestThread);
}


/**
 * \brief instantiate the udev monotor engine
 *
 * \note
 *
 */
bool RawHIDPlugin::initialize(const QStringList & arguments, QString *errorString)
{
    Q_UNUSED(arguments);
    Q_UNUSED(errorString);

    // We have to create the USB Monitor here:
    m_usbMonitor = new USBMonitor(this);

    return true;
}

Q_EXPORT_PLUGIN(RawHIDPlugin)

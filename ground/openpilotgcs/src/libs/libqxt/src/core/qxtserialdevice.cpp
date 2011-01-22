/****************************************************************************
 **
 ** Copyright (C) Qxt Foundation. Some rights reserved.
 **
 ** This file is part of the QxtCore module of the Qxt library.
 **
 ** This library is free software; you can redistribute it and/or modify it
 ** under the terms of the Common Public License, version 1.0, as published
 ** by IBM, and/or under the terms of the GNU Lesser General Public License,
 ** version 2.1, as published by the Free Software Foundation.
 **
 ** This file is provided "AS IS", without WARRANTIES OR CONDITIONS OF ANY
 ** KIND, EITHER EXPRESS OR IMPLIED INCLUDING, WITHOUT LIMITATION, ANY
 ** WARRANTIES OR CONDITIONS OF TITLE, NON-INFRINGEMENT, MERCHANTABILITY OR
 ** FITNESS FOR A PARTICULAR PURPOSE.
 **
 ** You should have received a copy of the CPL and the LGPL along with this
 ** file. See the LICENSE file and the cpl1.0.txt/lgpl-2.1.txt files
 ** included with the source distribution for more information.
 ** If you did not receive a copy of the licenses, contact the Qxt Foundation.
 **
 ** <http://libqxt.org>  <foundation@libqxt.org>
 **
 ****************************************************************************/
#include "qxtserialdevice.h"
#include "qxtserialdevice_p.h"

QxtSerialDevicePrivate::QxtSerialDevicePrivate() {
    fd = -1;
    notifier = 0;
}

/**
 * Creates a new QxtSerialDevice with the specified parent and sets the device name to \a device.
 */
QxtSerialDevice::QxtSerialDevice(const QString& device, QObject* parent) : QIODevice(parent) {
    QXT_INIT_PRIVATE(QxtSerialDevice);
    setDeviceName(device);
    setBaud(Baud9600);
}

/**
 * Creates a new QxtSerialDevice with the specified parent.
 */
QxtSerialDevice::QxtSerialDevice(QObject* parent) : QIODevice(parent) {
    QXT_INIT_PRIVATE(QxtSerialDevice);
    setBaud(Baud9600);
}

/**
 * \reimp
 */
bool QxtSerialDevice::atEnd() const {
    return (bytesAvailable() == 0);
}

/**
 * \reimp
 */
bool QxtSerialDevice::canReadLine() const {
    if(QIODevice::canReadLine()) return true;
    if(openMode() & QIODevice::Unbuffered) return false;
    if(qxt_d().constFillBuffer()) return false;
    return qxt_d().buffer.contains('\n');
}

/**
 * \reimp
 */
bool QxtSerialDevice::isSequential() const {
    return true;
}

/**
 * Sets the device name to \a device. This is a device node like "/dev/ttyS0" on UNIX or a device name like "COM1" on Windows.
 * \sa deviceName
 */
void QxtSerialDevice::setDeviceName(const QString& device) {
    qxt_d().device = device;
}

/**
 * Returns the current device name.
 * \sa device
 */
QString QxtSerialDevice::deviceName() const {
    return qxt_d().device;
}

/**
 * Returns the file descriptor for the open device. If the device is not open, this function returns -1.
 */
int QxtSerialDevice::handle() const {
    if(!isOpen()) return -1;
    return qxt_d().fd;
}

bool QxtSerialDevice::open(const QString& device, OpenMode mode) {
    setDeviceName(device);
    return(open(mode));
}

bool QxtSerialDevice::setPortSettings(PortSettings setup) {
    qxt_d().portSettings = setup;
    return qxt_d().setPortSettings(setup);
}

QxtSerialDevice::PortSettings QxtSerialDevice::portSettings() const {
    return qxt_d().portSettings;
}

QxtSerialDevice::PortSetting QxtSerialDevice::dataBits() const {
    return PortSetting(int(qxt_d().portSettings & BitMask));
}

QxtSerialDevice::PortSetting QxtSerialDevice::flowControl() const {
    return PortSetting(int(qxt_d().portSettings & FlowMask));
}

QxtSerialDevice::PortSetting QxtSerialDevice::parity() const {
    return PortSetting(int(qxt_d().portSettings & ParityMask));
}

QxtSerialDevice::PortSetting QxtSerialDevice::stopBits() const {
    return PortSetting(int(qxt_d().portSettings & StopMask));
}


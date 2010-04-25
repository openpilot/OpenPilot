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
#ifndef QXTSERIALDEVICE_P_H
#define QXTSERIALDEVICE_P_H

#include "qxtserialdevice.h"
#include <QByteArray>

#ifdef Q_OS_UNIX
#include <termios.h>
#endif

QT_FORWARD_DECLARE_CLASS(QSocketNotifier)

class QxtSerialDevicePrivate : public QObject, public QxtPrivate<QxtSerialDevice> {
Q_OBJECT
public:
    QxtSerialDevicePrivate();
    
    int fd;
    bool errorState;
    QSocketNotifier* notifier;
    mutable QByteArray buffer;
    QString device;
    int baud, flow, format;
    QxtSerialDevice::PortSettings portSettings; // duplicated for convenience
#ifdef Q_OS_UNIX
    termios reset, settings;
#endif

    qint64 deviceBuffer() const;
    bool setPortSettings(QxtSerialDevice::PortSettings settings);
    bool updateSettings();
    
public slots:
    int fillBuffer();
    int constFillBuffer() const;
};

#endif // QXTSERIALDEVICE_P_H

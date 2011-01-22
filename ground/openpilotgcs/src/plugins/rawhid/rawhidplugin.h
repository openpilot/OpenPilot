/**
 ******************************************************************************
 *
 * @file       rawhid.h
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

#ifndef RAWHIDPLUGIN_H
#define RAWHIDPLUGIN_H

#include "rawhid_global.h"
#include "rawhid.h"

#include "coreplugin/iconnection.h"
#include <extensionsystem/iplugin.h>

#include <QtCore/QMutex>
#include <QtCore/QThread>

class IConnection;
class RawHIDConnection;


/**
*   Helper thread to check on device connection/disconnection
*   Underlying HID library is not really easy to use,
*   so we have to poll for device modification in a separate thread
*/
class RAWHID_EXPORT RawHIDEnumerationThread : public QThread
{
    Q_OBJECT
public:
    RawHIDEnumerationThread(RawHIDConnection *rawhid);
    virtual ~RawHIDEnumerationThread();

    virtual void run();

signals:
    void enumerationChanged();

protected:
    RawHIDConnection *m_rawhid;
    bool m_running;
};


/**
*   Define a connection via the IConnection interface
*   Plugin will add a instance of this class to the pool,
*   so the connection manager can use it.
*/
class RAWHID_EXPORT RawHIDConnection
    : public Core::IConnection
{
    Q_OBJECT
public:
    RawHIDConnection();
    virtual ~RawHIDConnection();

    virtual QStringList availableDevices();
    virtual QIODevice *openDevice(const QString &deviceName);
    virtual void closeDevice(const QString &deviceName);

    virtual QString connectionName();
    virtual QString shortName();
    virtual void suspendPolling();
    virtual void resumePolling();

    bool deviceOpened() {return m_deviceOpened;}

protected slots:
    void onEnumerationChanged();

private:
    RawHID *RawHidHandle;
    bool enablePolling;

protected:
    QMutex m_enumMutex;
    RawHIDEnumerationThread m_enumerateThread;
    bool m_deviceOpened;
};

class RAWHID_EXPORT RawHIDPlugin
    : public ExtensionSystem::IPlugin
{
    Q_OBJECT

public:
    RawHIDPlugin();
    ~RawHIDPlugin();

    virtual bool initialize(const QStringList &arguments, QString *error_message);
    virtual void extensionsInitialized();
private:
    RawHIDConnection *hidConnection;
};


//usb hid test thread
#include "coreplugin/icore.h"
#include "coreplugin/connectionmanager.h"
class RawHIDTestThread: public QObject
{
    Q_OBJECT
public:
    RawHIDTestThread();
protected slots:
    void onDeviceConnect(QIODevice *);
    void onDeviceDisconnect();
    void onReadyRead();
    void onBytesWritten(qint64 sz);
protected:
    QIODevice *dev;
};

#endif // RAWHIDPLUGIN_H

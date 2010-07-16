/**
 ******************************************************************************
 *
 * @file       serialplugin.h
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

#ifndef SERIALPLUGIN_H
#define SERIALPLUGIN_H

#include "serial_global.h"
#include <qextserialport.h>
#include <qextserialenumerator.h>
#include "coreplugin/iconnection.h"
#include <extensionsystem/iplugin.h>

class IConnection;
class QextSerialEnumerator;

/**
*   Define a connection via the IConnection interface
*   Plugin will add a instance of this class to the pool,
*   so the connection manager can use it.
*/
class SERIAL_EXPORT SerialConnection
    : public Core::IConnection
{
    Q_OBJECT
public:
    SerialConnection();
    virtual ~SerialConnection();

    virtual QStringList availableDevices();
    virtual QIODevice *openDevice(const QString &deviceName);
    virtual void closeDevice(const QString &deviceName);

    virtual QString connectionName();
    virtual QString shortName();

private:
    QextSerialPort*  serialHandle;

protected slots:
    void onEnumerationChanged();
};


class SERIAL_EXPORT SerialPlugin
    : public ExtensionSystem::IPlugin
{
    Q_OBJECT

public:
    SerialPlugin();
    ~SerialPlugin();

    virtual bool initialize(const QStringList &arguments, QString *error_message);
    virtual void extensionsInitialized();
};


#endif // SERIALPLUGIN_H

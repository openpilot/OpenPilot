/**
 ******************************************************************************
 *
 * @file       serialplugin.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      Register connection object for the core connection manager
 * @see        The GNU Public License (GPL) Version 3
 * @defgroup   serial_plugin
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

#include "serialplugin.h"

#include <extensionsystem/pluginmanager.h>
#include <coreplugin/icore.h>

#include <QtCore/QtPlugin>
#include <QtGui/QMainWindow>
#include <qextserialport.h>
#include <qextserialenumerator.h>

#include <QDebug>

SerialConnection::SerialConnection()
{
    //I'm cheating a little bit here:
    //Knowing if the device enumeration really changed is a bit complicated
    //so I just signal it whenever we have a device event...
    QMainWindow *mw = Core::ICore::instance()->mainWindow();
    QObject::connect(mw, SIGNAL(deviceChange()),
                     this, SLOT(onEnumerationChanged()));
}

SerialConnection::~SerialConnection()
{
}

void SerialConnection::onEnumerationChanged()
{
    emit availableDevChanged(this);
}

QStringList SerialConnection::availableDevices()
{
    QStringList list;
    QList<QextPortInfo> ports = QextSerialEnumerator::getPorts();
    foreach( QextPortInfo port, ports ) {
        list.append(port.friendName);
    }

    return list;
}

QIODevice *SerialConnection::openDevice(const QString &deviceName)
{
    QList<QextPortInfo> ports = QextSerialEnumerator::getPorts();
    foreach( QextPortInfo port, ports ) {
        if(port.friendName == deviceName)
        {
            //we need to handle port settings here...
            return new QextSerialPort(port.portName);
        }
    }
    return NULL;
}

void SerialConnection::closeDevice(const QString &deviceName)
{
    //nothing to do here
}


QString SerialConnection::connectionName()
{
    return QString("Serial port");
}

QString SerialConnection::shortName()
{
    return QString("Serial");
}


SerialPlugin::SerialPlugin()
{
}

SerialPlugin::~SerialPlugin()
{

}

void SerialPlugin::extensionsInitialized()
{
    addAutoReleasedObject(new SerialConnection);
}

bool SerialPlugin::initialize(const QStringList &arguments, QString *errorString)
{
    Q_UNUSED(arguments);
    Q_UNUSED(errorString);

    return true;
}

Q_EXPORT_PLUGIN(SerialPlugin)

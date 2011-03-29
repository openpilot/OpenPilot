/**
 ******************************************************************************
 *
 * @file       iconnection.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 *             Parts by Nokia Corporation (qt-info@nokia.com) Copyright (C) 2009.
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup CorePlugin Core Plugin
 * @{
 * @brief The Core GCS plugin
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

#ifndef ICONNECTION_H
#define ICONNECTION_H

#include <QObject>
#include <QtCore/QStringList>
#include <QtCore/QIODevice>

#include "core_global.h"

namespace Core {

/**
*   An IConnection object define a "type of connection",
*   for instance USB, Serial, Network, ...
*/
class CORE_EXPORT IConnection : public QObject
{
    Q_OBJECT

public:
    /**
    *   Return the list of devices found on the system
    */
    struct device
    {
        QString name;
        QString displayName;
        bool operator==(device i){return this->name==i.name;}
    };

    virtual QList <device> availableDevices() = 0;

    /**
    *   Open a device, and return a QIODevice interface from it
    *   It should be a dynamically created object as it will be
    *   deleted by the connection manager.
    */
    virtual QIODevice *openDevice(const QString &deviceName) = 0;

    virtual void closeDevice(const QString &deviceName) { Q_UNUSED(deviceName) };

    /**
    *   Connection type name "USB HID"
    */
    virtual QString connectionName() = 0;

    /**
    *   Short name to display in a combo box
    */
    virtual QString shortName() {return connectionName();}

    /**
     * Manage whether the plugin is allowed to poll for devices
     * or not
     */
    virtual void suspendPolling() {};
    virtual void resumePolling() {};

signals:
    /**
    *   Available devices list has changed, signal it to connection manager (and whoever wants to know)
    */
    void availableDevChanged(IConnection *);
};

} //namespace Core

#endif // ICONNECTION_H

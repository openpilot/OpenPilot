/**
 ******************************************************************************
 *
 * @file       uavtalkplugin.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup UAVTalkPlugin UAVTalk Plugin
 * @{
 * @brief The UAVTalk protocol plugin
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
#ifndef UAVTALKPLUGIN_H
#define UAVTALKPLUGIN_H

#include <extensionsystem/iplugin.h>
#include <extensionsystem/pluginmanager.h>
#include <QtPlugin>
#include "telemetrymonitor.h"
#include "telemetry.h"
#include "uavtalk.h"
#include "telemetrymanager.h"
#include "uavobjectmanager.h"

class UAVTALK_EXPORT UAVTalkPlugin: public ExtensionSystem::IPlugin
{
    Q_OBJECT

public:
    UAVTalkPlugin();
    ~UAVTalkPlugin();

    void extensionsInitialized();
    bool initialize(const QStringList & arguments, QString * errorString);
    void shutdown();

protected slots:
    void onDeviceConnect(QIODevice *dev);
    void onDeviceDisconnect();

private:
    UAVObjectManager* objMngr;
    TelemetryManager* telMngr;
};

#endif // UAVTALKPLUGIN_H

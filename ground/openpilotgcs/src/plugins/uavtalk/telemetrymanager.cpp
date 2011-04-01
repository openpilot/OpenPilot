/**
 ******************************************************************************
 *
 * @file       telemetrymanager.cpp
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

#include "telemetrymanager.h"
#include <extensionsystem/pluginmanager.h>
#include <coreplugin/icore.h>
#include <coreplugin/threadmanager.h>

TelemetryManager::TelemetryManager() :
    autopilotConnected(false)
{
    moveToThread(Core::ICore::instance()->threadManager()->getRealTimeThread());
    // Get UAVObjectManager instance
    ExtensionSystem::PluginManager* pm = ExtensionSystem::PluginManager::instance();
    objMngr = pm->getObject<UAVObjectManager>();

    // connect to start stop signals
    connect(this, SIGNAL(myStart()), this, SLOT(onStart()),Qt::QueuedConnection);
    connect(this, SIGNAL(myStop()), this, SLOT(onStop()),Qt::QueuedConnection);
}

TelemetryManager::~TelemetryManager()
{
}

bool TelemetryManager::isConnected()
{
    return autopilotConnected;
}

void TelemetryManager::start(QIODevice *dev)
{
    device=dev;
    emit myStart();
}

void TelemetryManager::onStart()
{
    utalk = new UAVTalk(device, objMngr);
    telemetry = new Telemetry(utalk, objMngr);
    telemetryMon = new TelemetryMonitor(objMngr, telemetry);
    connect(telemetryMon, SIGNAL(connected()), this, SLOT(onConnect()));
    connect(telemetryMon, SIGNAL(disconnected()), this, SLOT(onDisconnect()));
}


void TelemetryManager::stop()
{
    emit myStop();
}

void TelemetryManager::onStop()
{
    telemetryMon->disconnect(this);
    delete telemetryMon;
    delete telemetry;
    delete utalk;
    onDisconnect();
}

void TelemetryManager::onConnect()
{
    autopilotConnected = true;
    emit connected();
}

void TelemetryManager::onDisconnect()
{
    autopilotConnected = false;
    emit disconnected();
}

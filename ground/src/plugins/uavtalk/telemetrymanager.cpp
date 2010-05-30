/**
 ******************************************************************************
 *
 * @file       telemetrymanager.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 *             Parts by Nokia Corporation (qt-info@nokia.com) Copyright (C) 2009.
 * @brief
 * @see        The GNU Public License (GPL) Version 3
 * @defgroup
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

#include "telemetrymanager.h"
#include <extensionsystem/pluginmanager.h>

TelemetryManager::TelemetryManager()
{
    // Get UAVObjectManager instance
    ExtensionSystem::PluginManager* pm = ExtensionSystem::PluginManager::instance();
    objMngr = pm->getObject<UAVObjectManager>();
}

TelemetryManager::~TelemetryManager()
{

}

void TelemetryManager::start(QIODevice *dev)
{
    utalk = new UAVTalk(dev, objMngr);
    telemetry = new Telemetry(utalk, objMngr);
    telemetryMon = new TelemetryMonitor(objMngr, telemetry);
    connect(telemetryMon, SIGNAL(connected()), this, SLOT(onConnect()));
    connect(telemetryMon, SIGNAL(disconnected()), this, SLOT(onDisconnect()));
}

void TelemetryManager::stop()
{
    telemetryMon->disconnect(this);
    delete telemetryMon;
    delete telemetry;
    delete utalk;
}

void TelemetryManager::onConnect()
{
    emit connected();
}

void TelemetryManager::onDisconnect()
{
    emit disconnected();
}

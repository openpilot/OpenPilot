/**
 ******************************************************************************
 *
 * @file       flightlogmanager.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2012.
 * @addtogroup [Group]
 * @{
 * @addtogroup FlightLogManager
 * @{
 * @brief [Brief]
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

#include "flightlogmanager.h"
#include "extensionsystem/pluginmanager.h"

#include "debuglogcontrol.h"

FlightLogManager::FlightLogManager(QObject *parent) :
    QObject(parent) {

    ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
    m_objectManager = pm->getObject<UAVObjectManager>();
    Q_ASSERT(m_objectManager);

    m_flightLogStatus = DebugLogStatus::GetInstance(m_objectManager);
    Q_ASSERT(m_flightLogStatus);

}

FlightLogManager::~FlightLogManager() {

}

void FlightLogManager::clearAllLogs() {

}

void FlightLogManager::retrieveLogs(int flight) {

}

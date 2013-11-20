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

    DebugLogEntry *entry = new DebugLogEntry();
    entry->setFlight(1);
    m_logEntries.append(entry);
    entry = new DebugLogEntry();
    entry->setFlight(2);
    m_logEntries.append(entry);
    entry = new DebugLogEntry();
    entry->setFlight(3);
    m_logEntries.append(entry);
    entry = new DebugLogEntry();
    entry->setFlight(4);
    m_logEntries.append(entry);
    entry = new DebugLogEntry();
    entry->setFlight(5);
    m_logEntries.append(entry);
    entry = new DebugLogEntry();
    entry->setFlight(6);
    m_logEntries.append(entry);
    entry = new DebugLogEntry();
    entry->setFlight(7);
    m_logEntries.append(entry);
    entry = new DebugLogEntry();
    entry->setFlight(8);
    m_logEntries.append(entry);
    entry = new DebugLogEntry();
    entry->setFlight(9);
    m_logEntries.append(entry);

}

FlightLogManager::~FlightLogManager() {

}

void addEntries(QQmlListProperty<DebugLogEntry> *list, DebugLogEntry *entry) {
    Q_UNUSED(list);
    Q_UNUSED(entry);
}

int countEntries(QQmlListProperty<DebugLogEntry> *list) {
    return static_cast< QList<DebugLogEntry *> *>(list->data)->size();
}

DebugLogEntry* entryAt(QQmlListProperty<DebugLogEntry> *list, int index) {
    return static_cast< QList<DebugLogEntry *> *>(list->data)->at(index);
}

void clearEntries(QQmlListProperty<DebugLogEntry> *list) {
    return static_cast< QList<DebugLogEntry *> *>(list->data)->clear();
}

QQmlListProperty<DebugLogEntry> FlightLogManager::logEntries() {
    return QQmlListProperty<DebugLogEntry>(this, &m_logEntries, &addEntries, &countEntries, &entryAt, &clearEntries);
}

void FlightLogManager::clearAllLogs() {

    //Clear on flight side
    m_logEntries.clear();
}

void FlightLogManager::retrieveLogs(int flight) {
    //Get logs from flight side
}

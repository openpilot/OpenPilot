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
#include "uavobjecthelper.h"

FlightLogManager::FlightLogManager(QObject *parent) :
    QObject(parent) {

    ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
    m_objectManager = pm->getObject<UAVObjectManager>();
    Q_ASSERT(m_objectManager);

    m_flightLogControl = DebugLogControl::GetInstance(m_objectManager);
    Q_ASSERT(m_flightLogControl);

    m_flightLogStatus = DebugLogStatus::GetInstance(m_objectManager);
    Q_ASSERT(m_flightLogStatus);

    m_flightLogEntry = DebugLogEntry::GetInstance(m_objectManager);
    Q_ASSERT(m_flightLogEntry);
}

FlightLogManager::~FlightLogManager() {

}

void addEntries(QQmlListProperty<ExtendedDebugLogEntry> *list, ExtendedDebugLogEntry *entry) {
    Q_UNUSED(list);
    Q_UNUSED(entry);
}

int countEntries(QQmlListProperty<ExtendedDebugLogEntry> *list) {
    return static_cast< QList<ExtendedDebugLogEntry *> *>(list->data)->size();
}

ExtendedDebugLogEntry* entryAt(QQmlListProperty<ExtendedDebugLogEntry> *list, int index) {
    return static_cast< QList<ExtendedDebugLogEntry *> *>(list->data)->at(index);
}

void clearEntries(QQmlListProperty<ExtendedDebugLogEntry> *list) {
    return static_cast< QList<ExtendedDebugLogEntry *> *>(list->data)->clear();
}

QQmlListProperty<ExtendedDebugLogEntry> FlightLogManager::logEntries() {
    return QQmlListProperty<ExtendedDebugLogEntry>(this, &m_logEntries, &addEntries, &countEntries, &entryAt, &clearEntries);
}

void FlightLogManager::clearAllLogs() {

    //Clear on flight side
    m_logEntries.clear();
}

void FlightLogManager::retrieveLogs(int flightToRetrieve) {

    UAVObjectUpdaterHelper updateHelper;
    UAVObjectRequestHelper requestHelper;

    //Get logs from flight side
    m_logEntries.clear();

    // Set up what to retrieve
    bool timedOut = false;
    int startFlight = (flightToRetrieve == -1) ? 0 : flightToRetrieve;
    int endFlight = (flightToRetrieve == -1 ) ? m_flightLogStatus->getFlight() : flightToRetrieve;

    // Prepare to send request for event retrieval
    m_flightLogControl->setOperation(DebugLogControl::OPERATION_RETRIEVE);
    for(int flight = startFlight; flight <= endFlight; flight++) {
        m_flightLogControl->setFlight(flight);
        bool gotLast = false;
        int entry = 0;
        while(!gotLast) {

            // Send request for loading flight entry on flight side and wait for ack/nack
            m_flightLogControl->setEntry(entry);

            UAVObjectUpdaterHelper::Result result = updateHelper.doObjectAndWait(m_flightLogControl, UAVTALK_TIMEOUT);
            if(result == UAVObjectUpdaterHelper::SUCCESS) {
                result = requestHelper.doObjectAndWait(m_flightLogEntry, UAVTALK_TIMEOUT);
                if(result == UAVObjectUpdaterHelper::TIMEOUT) {
                    timedOut = true;
                    break;
                } else {
                    if(!m_flightLogEntry->getType() == DebugLogEntry::TYPE_EMPTY &&
                            m_flightLogEntry->getFlight() == flight && m_flightLogEntry->getEntry() == entry) {

                        //Ok, we retrieved the entry, and it was the correct one. clone it and add it to the list

                        ExtendedDebugLogEntry* logEntry = new ExtendedDebugLogEntry();
                        logEntry->setData(m_flightLogEntry->getData());
                        m_logEntries.append(logEntry);

                        // Increment to get next entry from flight side
                        entry++;
                    } else {
                        // We are done, not more entries on this flight
                        break;
                    }
                }
            } else {
                break;
            }
        }
        if(timedOut) {
            // We timed out, do something smart here to alert the user
            break;
        }
    }
    emit logEntriesChanged();
}

void FlightLogManager::exportLogs()
{

}

ExtendedDebugLogEntry::ExtendedDebugLogEntry() : DebugLogEntry()
{
}

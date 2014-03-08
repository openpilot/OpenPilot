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

#include <QApplication>
#include <QFileDialog>

#include "debuglogcontrol.h"
#include "uavobjecthelper.h"
#include "uavtalk/uavtalk.h"
#include "utils/logfile.h"
#include <uavobjectutil/uavobjectutilmanager.h>

FlightLogManager::FlightLogManager(QObject *parent) :
    QObject(parent), m_disableControls(false),
    m_disableExport(true), m_cancelDownload(false),
    m_adjustExportedTimestamps(true)
{
    ExtensionSystem::PluginManager *pluginManager = ExtensionSystem::PluginManager::instance();
    Q_ASSERT(pluginManager);

    m_objectManager    = pluginManager->getObject<UAVObjectManager>();
    Q_ASSERT(m_objectManager);

    m_telemtryManager = pluginManager->getObject<TelemetryManager>();
    Q_ASSERT(m_telemtryManager);

    connect(m_telemtryManager, SIGNAL(connected()), this, SLOT(connectionStatusChanged()));
    connect(m_telemtryManager, SIGNAL(disconnected()), this, SLOT(connectionStatusChanged()));
    connectionStatusChanged();

    m_flightLogControl = DebugLogControl::GetInstance(m_objectManager);
    Q_ASSERT(m_flightLogControl);

    m_flightLogStatus  = DebugLogStatus::GetInstance(m_objectManager);
    Q_ASSERT(m_flightLogStatus);
    connect(m_flightLogStatus, SIGNAL(FlightChanged(quint16)), this, SLOT(updateFlightEntries(quint16)));

    m_flightLogEntry = DebugLogEntry::GetInstance(m_objectManager);
    Q_ASSERT(m_flightLogEntry);

    updateFlightEntries(m_flightLogStatus->getFlight());

    setupUAVOWrappers();

    setupLogSettings();
}

FlightLogManager::~FlightLogManager()
{
    while (!m_logEntries.isEmpty()) {
        delete m_logEntries.takeFirst();
    }
}

void addLogEntries(QQmlListProperty<ExtendedDebugLogEntry> *list, ExtendedDebugLogEntry *entry)
{
    Q_UNUSED(list);
    Q_UNUSED(entry);
}

int countLogEntries(QQmlListProperty<ExtendedDebugLogEntry> *list)
{
    return static_cast< QList<ExtendedDebugLogEntry *> *>(list->data)->size();
}

ExtendedDebugLogEntry *logEntryAt(QQmlListProperty<ExtendedDebugLogEntry> *list, int index)
{
    return static_cast< QList<ExtendedDebugLogEntry *> *>(list->data)->at(index);
}

void clearLogEntries(QQmlListProperty<ExtendedDebugLogEntry> *list)
{
    return static_cast< QList<ExtendedDebugLogEntry *> *>(list->data)->clear();
}

QQmlListProperty<ExtendedDebugLogEntry> FlightLogManager::logEntries()
{
    return QQmlListProperty<ExtendedDebugLogEntry>(this, &m_logEntries, &addLogEntries, &countLogEntries, &logEntryAt, &clearLogEntries);
}

void addUAVOEntries(QQmlListProperty<UAVOLogSettingsWrapper> *list, UAVOLogSettingsWrapper *entry)
{
    Q_UNUSED(list);
    Q_UNUSED(entry);
}

int countUAVOEntries(QQmlListProperty<UAVOLogSettingsWrapper> *list)
{
    return static_cast< QList<UAVOLogSettingsWrapper *> *>(list->data)->size();
}

UAVOLogSettingsWrapper *uavoEntryAt(QQmlListProperty<UAVOLogSettingsWrapper> *list, int index)
{
    return static_cast< QList<UAVOLogSettingsWrapper *> *>(list->data)->at(index);
}

void clearUAVOEntries(QQmlListProperty<UAVOLogSettingsWrapper> *list)
{
    return static_cast< QList<UAVOLogSettingsWrapper *> *>(list->data)->clear();
}

QQmlListProperty<UAVOLogSettingsWrapper> FlightLogManager::uavoEntries()
{
    return QQmlListProperty<UAVOLogSettingsWrapper>(this, &m_uavoEntries, &addUAVOEntries, &countUAVOEntries, &uavoEntryAt, &clearUAVOEntries);
}

QStringList FlightLogManager::flightEntries()
{
    return m_flightEntries;
}

void FlightLogManager::clearAllLogs()
{
    setDisableControls(true);
    QApplication::setOverrideCursor(Qt::WaitCursor);

    // Clear on flight side
    UAVObjectUpdaterHelper updateHelper;

    m_flightLogControl->setFlight(0);
    m_flightLogControl->setEntry(0);
    m_flightLogControl->setOperation(DebugLogControl::OPERATION_FORMATFLASH);
    if (updateHelper.doObjectAndWait(m_flightLogControl, UAVTALK_TIMEOUT) == UAVObjectUpdaterHelper::SUCCESS) {
        // Then empty locally
        clearLogList();
    }

    QApplication::restoreOverrideCursor();
    setDisableControls(false);
}

void FlightLogManager::clearLogList()
{
    QList<ExtendedDebugLogEntry *> tmpList(m_logEntries);
    m_logEntries.clear();

    emit logEntriesChanged();
    setDisableExport(true);

    while (!tmpList.isEmpty()) {
        delete tmpList.takeFirst();
    }
}

void FlightLogManager::retrieveLogs(int flightToRetrieve)
{
    setDisableControls(true);
    QApplication::setOverrideCursor(Qt::WaitCursor);
    m_cancelDownload = false;
    UAVObjectUpdaterHelper updateHelper;
    UAVObjectRequestHelper requestHelper;

    clearLogList();

    // Set up what to retrieve
    int startFlight = (flightToRetrieve == -1) ? 0 : flightToRetrieve;
    int endFlight   = (flightToRetrieve == -1) ? m_flightLogStatus->getFlight() : flightToRetrieve;

    // Prepare to send request for event retrieval
    m_flightLogControl->setOperation(DebugLogControl::OPERATION_RETRIEVE);
    for (int flight = startFlight; flight <= endFlight; flight++) {
        m_flightLogControl->setFlight(flight);
        bool gotLast = false;
        int entry    = 0;
        while (!gotLast) {
            // Send request for loading flight entry on flight side and wait for ack/nack
            m_flightLogControl->setEntry(entry);

            if (updateHelper.doObjectAndWait(m_flightLogControl, UAVTALK_TIMEOUT) == UAVObjectUpdaterHelper::SUCCESS &&
                requestHelper.doObjectAndWait(m_flightLogEntry, UAVTALK_TIMEOUT) == UAVObjectUpdaterHelper::SUCCESS) {
                if (m_flightLogEntry->getType() != DebugLogEntry::TYPE_EMPTY) {
                    // Ok, we retrieved the entry, and it was the correct one. clone it and add it to the list
                    ExtendedDebugLogEntry *logEntry = new ExtendedDebugLogEntry();
                    logEntry->setData(m_flightLogEntry->getData(), m_objectManager);
                    m_logEntries << logEntry;

                    // Increment to get next entry from flight side
                    entry++;
                } else {
                    // We are done, not more entries on this flight
                    gotLast = true;
                }
            } else {
                // We failed for some reason
                break;
            }
            if (m_cancelDownload) {
                break;
            }
        }
        if (m_cancelDownload) {
            break;
        }
    }

    if (m_cancelDownload) {
        clearLogList();
        m_cancelDownload = false;
    }

    emit logEntriesChanged();
    setDisableExport(m_logEntries.count() == 0);

    QApplication::restoreOverrideCursor();
    setDisableControls(false);
}

void FlightLogManager::exportToOPL(QString fileName)
{
    // Fix the file name
    fileName.replace(QString(".opl"), QString("%1.opl"));

    // Loop and create a new file for each flight.
    int currentEntry  = 0;
    int currentFlight = 0;
    quint32 adjustedBaseTime = 0;
    // Continue until all entries are exported
    while (currentEntry < m_logEntries.count()) {
        if (m_adjustExportedTimestamps) {
            adjustedBaseTime = m_logEntries[currentEntry]->getFlightTime();
        }

        // Get current flight
        currentFlight = m_logEntries[currentEntry]->getFlight();

        LogFile logFile;
        logFile.useProvidedTimeStamp(true);

        // Set the file name to contain flight number
        logFile.setFileName(fileName.arg(tr("_flight-%1").arg(currentFlight + 1)));
        logFile.open(QIODevice::WriteOnly);
        UAVTalk uavTalk(&logFile, m_objectManager);

        // Export entries until no more available or flight changes
        while (currentEntry < m_logEntries.count() && m_logEntries[currentEntry]->getFlight() == currentFlight) {
            ExtendedDebugLogEntry *entry = m_logEntries[currentEntry];

            // Only log uavobjects
            if (entry->getType() == ExtendedDebugLogEntry::TYPE_UAVOBJECT) {
                // Set timestamp that should be logged for this entry
                logFile.setNextTimeStamp(entry->getFlightTime() - adjustedBaseTime);

                // Use UAVTalk to log complete message to file
                uavTalk.sendObject(entry->uavObject(), false, false);
                qDebug() << entry->getFlightTime() - adjustedBaseTime << "=" << entry->toStringBrief();
            }
            currentEntry++;
        }

        logFile.close();
    }
}

void FlightLogManager::exportToCSV(QString fileName)
{
    QFile csvFile(fileName);
    if (csvFile.open(QFile::WriteOnly | QFile::Truncate)) {
        QTextStream csvStream(&csvFile);
        quint32 baseTime = 0;
        quint32 currentFlight = 0;
        csvStream << "Flight" << '\t' << "Flight Time" << '\t' << "Entry" << '\t' << "Data" << '\n';
        foreach (ExtendedDebugLogEntry *entry , m_logEntries) {
            if(m_adjustExportedTimestamps && entry->getFlight() != currentFlight) {
                currentFlight = entry->getFlight();
                baseTime = entry->getFlightTime();
            }
            entry->toCSV(&csvStream, baseTime);
        }
        csvStream.flush();
        csvFile.flush();
        csvFile.close();
    }
}

void FlightLogManager::exportToXML(QString fileName)
{
    QFile xmlFile(fileName);
    if (xmlFile.open(QFile::WriteOnly | QFile::Truncate)) {

        QXmlStreamWriter xmlWriter(&xmlFile);
        xmlWriter.setAutoFormatting(true);
        xmlWriter.setAutoFormattingIndent(4);

        xmlWriter.writeStartDocument("1.0", true);
        xmlWriter.writeStartElement("logs");
        xmlWriter.writeComment("This file was created by the flight log export in OpenPilot GCS.");

        quint32 baseTime = 0;
        quint32 currentFlight = 0;
        foreach (ExtendedDebugLogEntry *entry , m_logEntries) {
            if(m_adjustExportedTimestamps && entry->getFlight() != currentFlight) {
                currentFlight = entry->getFlight();
                baseTime = entry->getFlightTime();
            }
            entry->toXML(&xmlWriter, baseTime);
        }
        xmlWriter.writeEndElement();
        xmlWriter.writeEndDocument();
        xmlFile.flush();
        xmlFile.close();
    }
}

void FlightLogManager::exportLogs()
{
    if (m_logEntries.isEmpty()) {
        return;
    }

    setDisableControls(true);
    QApplication::setOverrideCursor(Qt::WaitCursor);

    QString oplFilter = tr("OpenPilot Log file %1").arg("(*.opl)");
    QString csvFilter = tr("Text file %1").arg("(*.csv)");
    QString xmlFilter = tr("XML file %1").arg("(*.xml)");

    QString selectedFilter = csvFilter;

    QString fileName = QFileDialog::getSaveFileName(NULL, tr("Save Log Entries"),
                                                    QString("OP-%1").arg(QDateTime::currentDateTime().toString("yyyy-MM-dd_hh-mm-ss")),
                                                    QString("%1;;%2;;%3").arg(oplFilter, csvFilter, xmlFilter), &selectedFilter);
    if (!fileName.isEmpty()) {
        if (selectedFilter == oplFilter) {
            if (!fileName.endsWith(".opl")) {
                fileName.append(".opl");
            }
            exportToOPL(fileName);
        } else if (selectedFilter == csvFilter) {
            if (!fileName.endsWith(".csv")) {
                fileName.append(".csv");
            }
            exportToCSV(fileName);
        } else if (selectedFilter == xmlFilter) {
            if (!fileName.endsWith(".xml")) {
                fileName.append(".xml");
            }
            exportToXML(fileName);
        }
    }

    QApplication::restoreOverrideCursor();
    setDisableControls(false);
}

void FlightLogManager::cancelExportLogs()
{
    m_cancelDownload = true;
}

void FlightLogManager::updateFlightEntries(quint16 currentFlight)
{
    Q_UNUSED(currentFlight);

    int flights = m_flightLogStatus->getFlight();
    if (m_flightEntries.count() == 0 || (m_flightEntries.count() - 1 != flights)) {
        m_flightEntries.clear();

        m_flightEntries << tr("All");
        for (int i = 0; i <= flights; i++) {
            m_flightEntries << QString::number(i + 1);
        }

        emit flightEntriesChanged();
    }
}

void FlightLogManager::setupUAVOWrappers()
{
    foreach(QList<UAVObject*> objectList , m_objectManager->getObjects()) {
        UAVObject* object = objectList.at(0);
        if (!object->isMetaDataObject() && !object->isSettingsObject()) {
            m_uavoEntries.append(new UAVOLogSettingsWrapper(object));
            qDebug() << objectList.at(0)->getName();
        }
    }
    emit uavoEntriesChanged();
}

void FlightLogManager::setupLogSettings()
{
    m_logSettings << tr("Disabled") << tr("When updated") << tr("Every 10ms") << tr("Every 50ms") << tr("Every 100ms")
                  << tr("Every 500ms") << tr("Every second") << tr("Every 5s") << tr("Every 10s") << tr("Every 30s") << tr("Every minute");
}

void FlightLogManager::connectionStatusChanged()
{
    if (m_telemtryManager->isConnected()) {
        ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
        UAVObjectUtilManager *utilMngr     = pm->getObject<UAVObjectUtilManager>();
        setBoardConnected(utilMngr->getBoardModel() == 0x0903);
    } else {
        setBoardConnected(false);
    }
}

ExtendedDebugLogEntry::ExtendedDebugLogEntry() : DebugLogEntry(),
    m_object(0)
{}

ExtendedDebugLogEntry::~ExtendedDebugLogEntry()
{
    if (m_object) {
        delete m_object;
        m_object = 0;
    }
}

QString ExtendedDebugLogEntry::getLogString()
{
    if (getType() == DebugLogEntry::TYPE_TEXT) {
        return QString((const char *)getData().Data);
    } else if (getType() == DebugLogEntry::TYPE_UAVOBJECT) {
        return m_object->toString().replace("\n", " ").replace("\t", " ");
    } else {
        return "";
    }
}

void ExtendedDebugLogEntry::toXML(QXmlStreamWriter *xmlWriter, quint32 baseTime)
{
    xmlWriter->writeStartElement("entry");
    xmlWriter->writeAttribute("flight", QString::number(getFlight() + 1));
    xmlWriter->writeAttribute("flighttime", QString::number(getFlightTime() - baseTime));
    xmlWriter->writeAttribute("entry", QString::number(getEntry()));
    if (getType() == DebugLogEntry::TYPE_TEXT) {
        xmlWriter->writeAttribute("type", "text");
        xmlWriter->writeTextElement("message", QString((const char *)getData().Data));
    } else if (getType() == DebugLogEntry::TYPE_UAVOBJECT) {
        xmlWriter->writeAttribute("type", "uavobject");
        m_object->toXML(xmlWriter);
    }
    xmlWriter->writeEndElement(); //entry
}

void ExtendedDebugLogEntry::toCSV(QTextStream *csvStream, quint32 baseTime)
{
    QString data;
    if (getType() == DebugLogEntry::TYPE_TEXT) {
        data = QString((const char *)getData().Data);
    } else if (getType() == DebugLogEntry::TYPE_UAVOBJECT) {
        data = m_object->toString().replace("\n", "").replace("\t", "");
    }
    *csvStream << QString::number(getFlight() + 1) << '\t' << QString::number(getFlightTime() - baseTime) << '\t' << QString::number(getEntry()) << '\t' << data << '\n';
}

void ExtendedDebugLogEntry::setData(const DebugLogEntry::DataFields &data, UAVObjectManager *objectManager)
{
    DebugLogEntry::setData(data);

    if (getType() == DebugLogEntry::TYPE_UAVOBJECT) {
        UAVDataObject *object = (UAVDataObject *)objectManager->getObject(getObjectID(), getInstanceID());
        Q_ASSERT(object);
        m_object = object->clone(getInstanceID());
        m_object->unpack(getData().Data);
    }
}


UAVOLogSettingsWrapper::UAVOLogSettingsWrapper() : QObject()
{}

UAVOLogSettingsWrapper::UAVOLogSettingsWrapper(UAVObject *object) : QObject(),
    m_object(object), m_setting(DISABLED)
{}

UAVOLogSettingsWrapper::~UAVOLogSettingsWrapper()
{}

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
#include <QXmlStreamReader>
#include <QMessageBox>
#include <QDebug>

#include "debuglogcontrol.h"
#include "uavobjecthelper.h"
#include "uavtalk/uavtalk.h"
#include "utils/logfile.h"
#include "uavdataobject.h"
#include <uavobjectutil/uavobjectutilmanager.h>

FlightLogManager::FlightLogManager(QObject *parent) :
    QObject(parent), m_disableControls(false),
    m_disableExport(true), m_cancelDownload(false),
    m_adjustExportedTimestamps(true)
{
    ExtensionSystem::PluginManager *pluginManager = ExtensionSystem::PluginManager::instance();

    Q_ASSERT(pluginManager);

    m_objectManager     = pluginManager->getObject<UAVObjectManager>();
    Q_ASSERT(m_objectManager);

    m_telemtryManager   = pluginManager->getObject<TelemetryManager>();
    Q_ASSERT(m_telemtryManager);

    m_objectUtilManager = pluginManager->getObject<UAVObjectUtilManager>();
    Q_ASSERT(m_objectUtilManager);

    m_flightLogControl  = DebugLogControl::GetInstance(m_objectManager);
    Q_ASSERT(m_flightLogControl);

    m_flightLogStatus   = DebugLogStatus::GetInstance(m_objectManager);
    Q_ASSERT(m_flightLogStatus);
    connect(m_flightLogStatus, SIGNAL(FlightChanged(quint16)), this, SLOT(updateFlightEntries(quint16)));

    m_flightLogEntry    = DebugLogEntry::GetInstance(m_objectManager);
    Q_ASSERT(m_flightLogEntry);

    m_flightLogSettings = DebugLogSettings::GetInstance(m_objectManager);
    Q_ASSERT(m_flightLogSettings);

    m_objectPersistence = ObjectPersistence::GetInstance(m_objectManager);
    Q_ASSERT(m_objectPersistence);

    updateFlightEntries(m_flightLogStatus->getFlight());

    setupLogSettings();
    setupLogStatuses();
    setupUAVOWrappers();

    connect(m_telemtryManager, SIGNAL(connected()), this, SLOT(connectionStatusChanged()));
    connect(m_telemtryManager, SIGNAL(disconnected()), this, SLOT(connectionStatusChanged()));
    connectionStatusChanged();
}

FlightLogManager::~FlightLogManager()
{
    while (!m_logEntries.isEmpty()) {
        delete m_logEntries.takeFirst();
    }
    while (!m_uavoEntries.isEmpty()) {
        delete m_uavoEntries.takeFirst();
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
    m_flightLogControl->setOperation(DebugLogControl::OPERATION_DELETEALL);
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
        int slot     = 0;
        while (!gotLast) {
            // Send request for loading flight entry on flight side and wait for ack/nack
            m_flightLogControl->setEntry(slot);

            if (updateHelper.doObjectAndWait(m_flightLogControl, UAVTALK_TIMEOUT) == UAVObjectUpdaterHelper::SUCCESS &&
                requestHelper.doObjectAndWait(m_flightLogEntry, UAVTALK_TIMEOUT) == UAVObjectUpdaterHelper::SUCCESS) {
                if (m_flightLogEntry->getType() != DebugLogEntry::TYPE_EMPTY) {
                    // Ok, we retrieved the entry, and it was the correct one. clone it and add it to the list
                    ExtendedDebugLogEntry *logEntry = new ExtendedDebugLogEntry();

                    logEntry->setData(m_flightLogEntry->getData(), m_objectManager);
                    m_logEntries << logEntry;
                    if (logEntry->getData().Type == DebugLogEntry::TYPE_MULTIPLEUAVOBJECTS) {
                        const quint32 total_len  = sizeof(DebugLogEntry::DataFields);
                        const quint32 data_len   = sizeof(((DebugLogEntry::DataFields *)0)->Data);
                        const quint32 header_len = total_len - data_len;

                        DebugLogEntry::DataFields fields;
                        quint32 start = logEntry->getData().Size;

                        // cycle until there is space for another object
                        while (start + header_len + 1 < data_len) {
                            memset(&fields, 0xFF, total_len);
                            memcpy(&fields, &logEntry->getData().Data[start], header_len);
                            // check wether a packed object is found
                            // note that empty data blocks are set as 0xFF in flight side to minimize flash wearing
                            // thus as soon as this read outside of used area, the test will fail as lenght would be 0xFFFF
                            quint32 toread = header_len + fields.Size;
                            if (!(toread + start > data_len)) {
                                memcpy(&fields, &logEntry->getData().Data[start], toread);
                                ExtendedDebugLogEntry *subEntry = new ExtendedDebugLogEntry();
                                subEntry->setData(fields, m_objectManager);
                                m_logEntries << subEntry;
                            }
                            start += toread;
                        }
                    }

                    // Increment to get next entry from flight side
                    slot++;
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
            if (entry->getType() == ExtendedDebugLogEntry::TYPE_UAVOBJECT || entry->getType() == ExtendedDebugLogEntry::TYPE_MULTIPLEUAVOBJECTS) {
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
        foreach(ExtendedDebugLogEntry * entry, m_logEntries) {
            if (m_adjustExportedTimestamps && entry->getFlight() != currentFlight) {
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
        foreach(ExtendedDebugLogEntry * entry, m_logEntries) {
            if (m_adjustExportedTimestamps && entry->getFlight() != currentFlight) {
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

    QString fileName = QFileDialog::getSaveFileName(NULL, tr("Save Log Entries"), QDir::homePath(),
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

void FlightLogManager::loadSettings()
{
    QString xmlFilter = tr("XML file %1").arg("(*.xml)");
    QString fileName  = QFileDialog::getOpenFileName(NULL, tr("Load Log Settings"), QDir::homePath(), QString("%1").arg(xmlFilter));

    if (!fileName.isEmpty()) {
        if (!fileName.endsWith(".xml")) {
            fileName.append(".xml");
        }
        QFile xmlFile(fileName);
        QString errorString;
        if (xmlFile.open(QFile::ReadOnly)) {
            QXmlStreamReader xmlReader(&xmlFile);
            while (xmlReader.readNextStartElement() && xmlReader.name() == "settings") {
                bool ok;

                int version = xmlReader.attributes().value("version").toInt(&ok);
                if (!ok || version != LOG_SETTINGS_FILE_VERSION) {
                    errorString = tr("The file has the wrong version or could not parse version information.");
                    break;
                }

                setLoggingEnabled(xmlReader.attributes().value("enabled").toInt(&ok));
                if (!ok) {
                    errorString = tr("Could not parse enabled attribute.");
                    break;
                }

                while (xmlReader.readNextStartElement()) {
                    if (xmlReader.name() == "setting") {
                        QString name = xmlReader.attributes().value("name").toString();
                        int level    = xmlReader.attributes().value("level").toInt(&ok);
                        if (ok) {
                            int period = xmlReader.attributes().value("period").toInt(&ok);
                            if (ok && updateLogWrapper(name, level, period)) {} else {
                                errorString = tr("Could not parse period attribute, or object with name '%1' could not be found.").arg(name);
                                break;
                            }
                        } else {
                            errorString = tr("Could not parse level attribute on setting '%1'").arg(name);
                            break;
                        }
                    }
                    xmlReader.skipCurrentElement();
                }
                xmlReader.skipCurrentElement();
            }
            if (!xmlReader.atEnd() && (xmlReader.hasError() || !errorString.isEmpty())) {
                QMessageBox::warning(NULL, tr("Settings file corrupt."),
                                     tr("The file loaded is not in the correct format.\nPlease check the file.\n%1")
                                     .arg(xmlReader.hasError() ? xmlReader.errorString() : errorString),
                                     QMessageBox::Ok);
            }
        }
        xmlFile.close();
    }
}

void FlightLogManager::saveSettings()
{
    QString xmlFilter = tr("XML file %1").arg("(*.xml)");
    QString fileName  = QFileDialog::getSaveFileName(NULL, tr("Save Log Settings"),
                                                     QDir::homePath(), QString("%1").arg(xmlFilter));

    if (!fileName.isEmpty()) {
        if (!fileName.endsWith(".xml")) {
            fileName.append(".xml");
        }
        QFile xmlFile(fileName);

        if (xmlFile.open(QFile::WriteOnly | QFile::Truncate)) {
            QXmlStreamWriter xmlWriter(&xmlFile);
            xmlWriter.setAutoFormatting(true);
            xmlWriter.setAutoFormattingIndent(4);

            xmlWriter.writeStartDocument("1.0", true);
            xmlWriter.writeComment("This file was created by the flight log settings function in OpenPilot GCS.");
            xmlWriter.writeStartElement("settings");
            xmlWriter.writeAttribute("version", QString::number(LOG_SETTINGS_FILE_VERSION));
            xmlWriter.writeAttribute("enabled", QString::number(m_loggingEnabled));
            foreach(UAVOLogSettingsWrapper * wrapper, m_uavoEntries) {
                xmlWriter.writeStartElement("setting");
                xmlWriter.writeAttribute("name", wrapper->name());
                xmlWriter.writeAttribute("level", QString::number(wrapper->setting()));
                xmlWriter.writeAttribute("period", QString::number(wrapper->period()));
                xmlWriter.writeEndElement();
            }
            xmlWriter.writeEndElement();
            xmlWriter.writeEndDocument();
            xmlFile.flush();
            xmlFile.close();
        }
    }
}

void FlightLogManager::resetSettings(bool clear)
{
    setLoggingEnabled(clear ? 0 : m_flightLogSettings->getLoggingEnabled());
    foreach(UAVOLogSettingsWrapper * wrapper, m_uavoEntries) {
        wrapper->reset(clear);
    }
}

void FlightLogManager::saveSettingsToBoard()
{
    m_flightLogSettings->setLoggingEnabled(m_loggingEnabled);
    m_flightLogSettings->updated();
    saveUAVObjectToFlash(m_flightLogSettings);

    foreach(UAVOLogSettingsWrapper * wrapper, m_uavoEntries) {
        if (wrapper->dirty()) {
            UAVObject::Metadata meta = wrapper->object()->getMetadata();
            wrapper->object()->SetLoggingUpdateMode(meta, wrapper->settingAsUpdateMode());
            meta.loggingUpdatePeriod = wrapper->period();

            // As metadata are set up to update via telemetry on change
            // this call will send the update to the board.
            wrapper->object()->setMetadata(meta);

            if (saveUAVObjectToFlash(wrapper->object()->getMetaObject())) {
                wrapper->setDirty(false);
            }
        }
    }
}

bool FlightLogManager::saveUAVObjectToFlash(UAVObject *object)
{
    m_objectUtilManager->saveObjectToSD(object);
    return true;
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
    foreach(QList<UAVObject *> objectList, m_objectManager->getObjects()) {
        UAVObject *object = objectList.at(0);

        if (!object->isMetaDataObject() && !object->isSettingsObject()) {
            UAVOLogSettingsWrapper *wrapper = new UAVOLogSettingsWrapper(qobject_cast<UAVDataObject *>(object));
            m_uavoEntries.append(wrapper);
            m_uavoEntriesHash[wrapper->name()] = wrapper;
        }
    }
    emit uavoEntriesChanged();
}

void FlightLogManager::setupLogSettings()
{
    // Corresponds to:
    // typedef enum {
    // UPDATEMODE_MANUAL    = 0,  /** Manually update object, by calling the updated() function */
    // UPDATEMODE_PERIODIC  = 1, /** Automatically update object at periodic intervals */
    // UPDATEMODE_ONCHANGE  = 2, /** Only update object when its data changes */
    // UPDATEMODE_THROTTLED = 3 /** Object is updated on change, but not more often than the interval time */
    // } UpdateMode;

    m_logSettings << tr("Disabled") << tr("Periodically") << tr("When updated") << tr("Throttled");
}

void FlightLogManager::setupLogStatuses()
{
    m_logStatuses << tr("Never") << tr("Only when Armed") << tr("Always");
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
    if (boardConnected()) {
        resetSettings(false);
    }
}

bool FlightLogManager::updateLogWrapper(QString name, int level, int period)
{
    UAVOLogSettingsWrapper *wrapper = m_uavoEntriesHash[name];

    if (wrapper) {
        wrapper->setSetting(level);
        wrapper->setPeriod(period);
        qDebug() << "Wrapper" << name << ", level" << level << ", period" << period;
        return true;
    }
    return false;
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
    } else if (getType() == DebugLogEntry::TYPE_UAVOBJECT || getType() == DebugLogEntry::TYPE_MULTIPLEUAVOBJECTS) {
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
    } else if (getType() == DebugLogEntry::TYPE_UAVOBJECT || getType() == DebugLogEntry::TYPE_MULTIPLEUAVOBJECTS) {
        xmlWriter->writeAttribute("type", "uavobject");
        m_object->toXML(xmlWriter);
    }
    xmlWriter->writeEndElement(); // entry
}

void ExtendedDebugLogEntry::toCSV(QTextStream *csvStream, quint32 baseTime)
{
    QString data;

    if (getType() == DebugLogEntry::TYPE_TEXT) {
        data = QString((const char *)getData().Data);
    } else if (getType() == DebugLogEntry::TYPE_UAVOBJECT || getType() == DebugLogEntry::TYPE_MULTIPLEUAVOBJECTS) {
        data = m_object->toString().replace("\n", "").replace("\t", "");
    }
    *csvStream << QString::number(getFlight() + 1) << '\t' << QString::number(getFlightTime() - baseTime) << '\t' << QString::number(getEntry()) << '\t' << data << '\n';
}

void ExtendedDebugLogEntry::setData(const DebugLogEntry::DataFields &data, UAVObjectManager *objectManager)
{
    DebugLogEntry::setData(data);

    if (getType() == DebugLogEntry::TYPE_UAVOBJECT || getType() == DebugLogEntry::TYPE_MULTIPLEUAVOBJECTS) {
        UAVDataObject *object = (UAVDataObject *)objectManager->getObject(getObjectID(), getInstanceID());
        Q_ASSERT(object);
        m_object = object->clone(getInstanceID());
        m_object->unpack(getData().Data);
    }
}


UAVOLogSettingsWrapper::UAVOLogSettingsWrapper() : QObject()
{}

UAVOLogSettingsWrapper::UAVOLogSettingsWrapper(UAVDataObject *object) : QObject(),
    m_object(object), m_setting(DISABLED), m_period(0), m_dirty(0)
{
    reset(false);
}

UAVOLogSettingsWrapper::~UAVOLogSettingsWrapper()
{}

void UAVOLogSettingsWrapper::reset(bool clear)
{
    setSetting(m_object->GetLoggingUpdateMode(m_object->getMetadata()));
    setPeriod(m_object->getMetadata().loggingUpdatePeriod);
    if (clear) {
        int oldSetting = setting();
        int oldPeriod  = period();
        setSetting(0);
        setPeriod(0);
        setDirty(oldSetting != setting() || oldPeriod != period());
    } else {
        setDirty(false);
    }
}

UAVObject::UpdateMode UAVOLogSettingsWrapper::settingAsUpdateMode()
{
    switch (m_setting) {
    case 0:
        return UAVObject::UPDATEMODE_MANUAL;

    case 1:
        return UAVObject::UPDATEMODE_PERIODIC;

    case 2:
        return UAVObject::UPDATEMODE_ONCHANGE;

    case 3:
        return UAVObject::UPDATEMODE_THROTTLED;

    default:
        return UAVObject::UPDATEMODE_MANUAL;
    }
}

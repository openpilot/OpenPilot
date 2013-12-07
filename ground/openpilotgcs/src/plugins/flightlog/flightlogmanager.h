/**
 ******************************************************************************
 *
 * @file       flightlogmanager.h
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

#ifndef FLIGHTLOGMANAGER_H
#define FLIGHTLOGMANAGER_H

#include <QObject>
#include <QList>
#include <QQmlListProperty>
#include <QSemaphore>

#include "uavobjectmanager.h"
#include "debuglogentry.h"
#include "debuglogstatus.h"
#include "debuglogcontrol.h"

class ExtendedDebugLogEntry : public DebugLogEntry {
    Q_OBJECT Q_PROPERTY(QString LogString READ getLogString WRITE setLogString NOTIFY LogStringUpdated)

public:
    explicit ExtendedDebugLogEntry();
    ~ExtendedDebugLogEntry();

    QString getLogString();
    UAVDataObject *uavObject()
    {
        return m_object;
    }

    void setData(const DataFields& data, UAVObjectManager *objectManager);

public slots:
    void setLogString(QString arg)
    {
        Q_UNUSED(arg);
    }

signals:
    void LogStringUpdated(QString arg);

private:
    UAVDataObject *m_object;
};

class FlightLogManager : public QObject {
    Q_OBJECT Q_PROPERTY(DebugLogStatus *flightLogStatus READ flightLogStatus)
    Q_PROPERTY(QQmlListProperty<ExtendedDebugLogEntry> logEntries READ logEntries NOTIFY logEntriesChanged)
    Q_PROPERTY(QStringList flightEntries READ flightEntries NOTIFY flightEntriesChanged)
    Q_PROPERTY(bool disableControls READ disableControls WRITE setDisableControls NOTIFY disableControlsChanged)
    Q_PROPERTY(bool disableExport READ disableExport WRITE setDisableExport NOTIFY disableExportChanged)
    Q_PROPERTY(bool adjustExportedTimestamps READ adjustExportedTimestamps WRITE setAdjustExportedTimestamps NOTIFY adjustExportedTimestampsChanged)

public:
    explicit FlightLogManager(QObject *parent = 0);
    ~FlightLogManager();

    QQmlListProperty<ExtendedDebugLogEntry> logEntries();
    QStringList flightEntries();

    DebugLogStatus *flightLogStatus() const
    {
        return m_flightLogStatus;
    }

    bool disableControls() const
    {
        return m_disableControls;
    }

    bool disableExport() const
    {
        return m_disableExport;
    }

    void clearLogList();

    bool adjustExportedTimestamps() const
    {
        return m_adjustExportedTimestamps;
    }

signals:
    void logEntriesChanged();
    void flightEntriesChanged();
    void disableControlsChanged(bool arg);
    void disableExportChanged(bool arg);

    void adjustExportedTimestampsChanged(bool arg);

public slots:
    void clearAllLogs();
    void retrieveLogs(int flightToRetrieve = -1);
    void exportLogs();
    void cancelExportLogs();

    void setDisableControls(bool arg)
    {
        if (m_disableControls != arg) {
            m_disableControls = arg;
            emit disableControlsChanged(arg);
        }
    }

    void setDisableExport(bool arg)
    {
        if (m_disableExport != arg) {
            m_disableExport = arg;
            emit disableExportChanged(arg);
        }
    }

    void setAdjustExportedTimestamps(bool arg)
    {
        if (m_adjustExportedTimestamps != arg) {
            m_adjustExportedTimestamps = arg;
            emit adjustExportedTimestampsChanged(arg);
        }
    }

private slots:
    void updateFlightEntries(quint16 currentFlight);

private:
    UAVObjectManager *m_objectManager;
    DebugLogControl *m_flightLogControl;
    DebugLogStatus *m_flightLogStatus;
    DebugLogEntry *m_flightLogEntry;
    QList<ExtendedDebugLogEntry *> m_logEntries;
    QStringList m_flightEntries;

    static const int UAVTALK_TIMEOUT = 4000;
    bool m_disableControls;
    bool m_disableExport;
    bool m_cancelDownload;
    bool m_adjustExportedTimestamps;
};

#endif // FLIGHTLOGMANAGER_H

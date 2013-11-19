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
#include <QtDeclarative/QDeclarativeListProperty>

class FlightLogEntry : public QObject {
    Q_OBJECT Q_PROPERTY(int flightTime READ getFlightTime NOTIFY FlightTimeChanged)
    Q_PROPERTY(int flight READ getFlight NOTIFY FlightChanged)
    Q_PROPERTY(int entry READ getEntry NOTIFY EntryChanged)
    Q_PROPERTY(int type READ getType NOTIFY TypeChanged)

    int m_flightTime;
    int m_flight;
    int m_entry;
    int m_type;

public:
    explicit FlightLogEntry(QObject *parent = 0);
    ~FlightLogEntry();

    int getFlightTime() const
    {
        return m_flightTime;
    }

    int getFlight() const
    {
        return m_flight;
    }

    int getEntry() const
    {
        return m_entry;
    }

    int getType() const
    {
        return m_type;
    }

signals:
    void FlightTimeChanged(int arg);
    void FlightChanged(int arg);
    void EntryChanged(int arg);
    void TypeChanged(int arg);
};

class FlightLogManager : public QObject {
    Q_OBJECT Q_PROPERTY(int flightsRecorded READ flightsRecorded NOTIFY flightsRecordedChanged)
    Q_PROPERTY(int logsRecordedLastFlight READ logsRecordedLastFlight NOTIFY logsRecordedLastFlightChanged)
    Q_PROPERTY(int totalLogsRecorded READ totalLogsRecorded NOTIFY totalLogsRecordedChanged)
    Q_PROPERTY(int freeLogEntries READ freeLogEntries NOTIFY freeLogEntriesChanged)
    Q_PROPERTY(QDeclarativeListProperty<FlightLogEntry *> records READ records)

    int m_flightsRecorded;
    int m_logsRecordedLastFlight;
    int m_totalLogsRecorded;
    int m_freeLogEntries;
    QDeclarativeListProperty<FlightLogEntry *> m_records;

public:
    explicit FlightLogManager(QObject *parent = 0);
    ~FlightLogManager();

    int flightsRecorded() const
    {
        return m_flightsRecorded;
    }

    int logsRecordedLastFlight() const
    {
        return m_logsRecordedLastFlight;
    }

    int totalLogsRecorded() const
    {
        return m_totalLogsRecorded;
    }

    int freeLogEntries() const
    {
        return m_freeLogEntries;
    }

    QDeclarativeListProperty<FlightLogEntry *> records() const
    {
        return m_records;
    }

signals:
    void flightsRecordedChanged(int arg);
    void logsRecordedLastFlightChanged(int arg);
    void totalLogsRecordedChanged(int arg);
    void freeLogEntriesChanged(int arg);

public slots:
    void clearAllLogs();
    void retrieveLogs(int flight = -1);
};

#endif // FLIGHTLOGMANAGER_H

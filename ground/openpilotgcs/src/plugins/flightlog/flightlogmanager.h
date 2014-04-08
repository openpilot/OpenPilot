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
#include <QHash>
#include <QQmlListProperty>
#include <QSemaphore>
#include <QXmlStreamWriter>
#include <QTextStream>

#include "uavobjectmanager.h"
#include "uavobjectutilmanager.h"
#include "debuglogentry.h"
#include "debuglogstatus.h"
#include "debuglogsettings.h"
#include "debuglogcontrol.h"
#include "objectpersistence.h"
#include "uavtalk/telemetrymanager.h"

class UAVOLogSettingsWrapper : public QObject {
    Q_OBJECT Q_PROPERTY(UAVDataObject *object READ object NOTIFY objectChanged)
    Q_PROPERTY(QString name READ name NOTIFY nameChanged)
    Q_PROPERTY(int setting READ setting WRITE setSetting NOTIFY settingChanged)
    Q_PROPERTY(int period READ period WRITE setPeriod NOTIFY periodChanged)
    Q_PROPERTY(bool dirty READ dirty WRITE setDirty NOTIFY dirtyChanged)

public:
    enum UAVLogSetting { DISABLED = 0, ON_CHANGE, THROTTLED, PERIODICALLY };

    explicit UAVOLogSettingsWrapper();
    explicit UAVOLogSettingsWrapper(UAVDataObject *object);
    ~UAVOLogSettingsWrapper();

    QString name() const
    {
        return m_object->getName();
    }

    int setting() const
    {
        return m_setting;
    }

    UAVObject::UpdateMode settingAsUpdateMode();

    int period() const
    {
        return m_period;
    }

    UAVDataObject *object() const
    {
        return m_object;
    }

    bool dirty() const
    {
        return m_dirty;
    }

public slots:
    void setSetting(int setting)
    {
        if (m_setting != setting) {
            m_setting = setting;
            setDirty(true);
            if (m_setting != 1 && m_setting != 3) {
                setPeriod(0);
            }
            emit settingChanged(setting);
        }
    }

    void setPeriod(int arg)
    {
        if (m_period != arg) {
            m_period = arg;
            setDirty(true);
            emit periodChanged(arg);
        }
    }

    void setDirty(bool arg)
    {
        if (m_dirty != arg) {
            m_dirty = arg;
            emit dirtyChanged(arg);
        }
    }

    void reset(bool clear);

signals:
    void settingChanged(int setting);
    void nameChanged(QString name);
    void periodChanged(int period);
    void objectChanged(UAVDataObject *arg);

    void dirtyChanged(bool arg);

private:
    UAVDataObject *m_object;
    int m_setting;
    int m_period;
    bool m_dirty;
};

class ExtendedDebugLogEntry : public DebugLogEntry {
    Q_OBJECT Q_PROPERTY(QString LogString READ getLogString WRITE setLogString NOTIFY LogStringUpdated)

public:
    explicit ExtendedDebugLogEntry();
    ~ExtendedDebugLogEntry();

    QString getLogString();
    void toXML(QXmlStreamWriter *xmlWriter, quint32 baseTime);
    void toCSV(QTextStream *csvStream, quint32 baseTime);
    UAVDataObject *uavObject()
    {
        return m_object;
    }

    void setData(const DataFields & data, UAVObjectManager *objectManager);

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
    Q_PROPERTY(DebugLogControl * flightLogControl READ flightLogControl)
    Q_PROPERTY(DebugLogSettings * flightLogSettings READ flightLogSettings)
    Q_PROPERTY(QQmlListProperty<ExtendedDebugLogEntry> logEntries READ logEntries NOTIFY logEntriesChanged)
    Q_PROPERTY(QStringList flightEntries READ flightEntries NOTIFY flightEntriesChanged)
    Q_PROPERTY(bool disableControls READ disableControls WRITE setDisableControls NOTIFY disableControlsChanged)
    Q_PROPERTY(bool disableExport READ disableExport WRITE setDisableExport NOTIFY disableExportChanged)
    Q_PROPERTY(bool adjustExportedTimestamps READ adjustExportedTimestamps WRITE setAdjustExportedTimestamps NOTIFY adjustExportedTimestampsChanged)
    Q_PROPERTY(bool boardConnected READ boardConnected WRITE setBoardConnected NOTIFY boardConnectedChanged)

    Q_PROPERTY(QQmlListProperty<UAVOLogSettingsWrapper> uavoEntries READ uavoEntries NOTIFY uavoEntriesChanged)
    Q_PROPERTY(QStringList logSettings READ logSettings NOTIFY logSettingsChanged)
    Q_PROPERTY(QStringList logStatuses READ logStatuses NOTIFY logStatusesChanged)
    Q_PROPERTY(int loggingEnabled READ loggingEnabled WRITE setLoggingEnabled NOTIFY loggingEnabledChanged)


public:
    explicit FlightLogManager(QObject *parent = 0);
    ~FlightLogManager();

    QQmlListProperty<ExtendedDebugLogEntry> logEntries();
    QQmlListProperty<UAVOLogSettingsWrapper> uavoEntries();

    QStringList flightEntries();

    QStringList logSettings()
    {
        return m_logSettings;
    }

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

    bool boardConnected() const
    {
        return m_boardConnected;
    }

    QStringList logStatuses() const
    {
        return m_logStatuses;
    }

    DebugLogControl *flightLogControl() const
    {
        return m_flightLogControl;
    }

    DebugLogSettings *flightLogSettings() const
    {
        return m_flightLogSettings;
    }

    int loggingEnabled() const
    {
        return m_loggingEnabled;
    }

signals:
    void logEntriesChanged();
    void flightEntriesChanged();
    void logSettingsChanged();
    void uavoEntriesChanged();
    void disableControlsChanged(bool arg);
    void disableExportChanged(bool arg);
    void adjustExportedTimestampsChanged(bool arg);
    void boardConnectedChanged(bool arg);

    void logStatusesChanged(QStringList arg);
    void loggingEnabledChanged(int arg);

public slots:
    void clearAllLogs();
    void retrieveLogs(int flightToRetrieve = -1);
    void exportLogs();
    void cancelExportLogs();
    void loadSettings();
    void saveSettings();
    void resetSettings(bool clear);
    void saveSettingsToBoard();
    bool saveUAVObjectToFlash(UAVObject *object);

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

    void setBoardConnected(bool arg)
    {
        if (m_boardConnected != arg) {
            m_boardConnected = arg;
            emit boardConnectedChanged(arg);
        }
    }

    void setLoggingEnabled(int arg)
    {
        if (m_loggingEnabled != arg) {
            m_loggingEnabled = arg;
            emit loggingEnabledChanged(arg);
        }
    }

private slots:
    void updateFlightEntries(quint16 currentFlight);
    void setupUAVOWrappers();
    void setupLogSettings();
    void setupLogStatuses();
    void connectionStatusChanged();
    bool updateLogWrapper(QString name, int level, int period);

private:
    UAVObjectManager *m_objectManager;
    UAVObjectUtilManager *m_objectUtilManager;
    TelemetryManager *m_telemtryManager;
    DebugLogControl *m_flightLogControl;
    DebugLogStatus *m_flightLogStatus;
    DebugLogEntry *m_flightLogEntry;
    DebugLogSettings *m_flightLogSettings;
    ObjectPersistence *m_objectPersistence;

    QList<ExtendedDebugLogEntry *> m_logEntries;
    QStringList m_flightEntries;
    QStringList m_logSettings;
    QStringList m_logStatuses;

    QList<UAVOLogSettingsWrapper *> m_uavoEntries;
    QHash<QString, UAVOLogSettingsWrapper *> m_uavoEntriesHash;

    void exportToOPL(QString fileName);
    void exportToCSV(QString fileName);
    void exportToXML(QString fileName);

    static const int UAVTALK_TIMEOUT = 4000;
    static const int LOG_SETTINGS_FILE_VERSION = 1;
    bool m_disableControls;
    bool m_disableExport;
    bool m_cancelDownload;
    bool m_adjustExportedTimestamps;
    bool m_boardConnected;
    int m_loggingEnabled;
};

#endif // FLIGHTLOGMANAGER_H

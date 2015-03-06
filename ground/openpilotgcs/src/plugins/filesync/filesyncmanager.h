/**
 ******************************************************************************
 *
 * @file       FileSyncManager.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2012.
 * @addtogroup [Group]
 * @{
 * @addtogroup FileSyncManager
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

#ifndef FILESYNCMANAGER_H
#define FILESYNCMANAGER_H

#include <QObject>
#include <QList>
#include <QHash>
#include <QQmlListProperty>
#include <QSemaphore>
#include <QXmlStreamWriter>
#include <QTextStream>

#include "uavobjectmanager.h"
#include "uavobjectutilmanager.h"
#include "sync.h"
#include "objectpersistence.h"
#include "uavtalk/telemetrymanager.h"


class FileSyncEntry : public QObject {
    Q_OBJECT Q_PROPERTY(QString FileName READ getFileName WRITE setFileName NOTIFY FileNameUpdated)
    Q_PROPERTY(quint32 FileSize READ getFileSize WRITE setFileSize NOTIFY FileSizeUpdated)
    Q_PROPERTY(quint32 FileType READ getFileType WRITE setFileType NOTIFY FileTypeUpdated)

public:
    explicit FileSyncEntry() : m_size(0)
    {
    }

    ~FileSyncEntry()
    {
        m_size = 0;
    }

    QString getFileName()
    {
        return m_name;
    }

    quint32 getFileSize()
    {
        return m_size;
    }

    quint32 getFileType()
    {
        return m_type;
    }

    void setName(const char* name)
    {
        m_name = name;
    }

    void setSize(quint32 size)
    {
        m_size = size;
    }

    void setType(quint32 type)
    {
        m_type = type;
    }

public slots:
    void setFileName(QString arg)
    {
        Q_UNUSED(arg);
    }
    void setFileSize(quint32 arg)
    {
        Q_UNUSED(arg);
    }
    void setFileType(quint32 arg)
    {
        Q_UNUSED(arg);
    }

signals:
    void FileNameUpdated(QString arg);
    void FileSizeUpdated(quint32 arg);
    void FileTypeUpdated(quint32 arg);

private:
    quint32 m_size;
    QString m_name;
    quint32 m_type;
};


class FileSyncManager : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool disableControls READ disableControls WRITE setDisableControls NOTIFY disableControlsChanged)
    Q_PROPERTY(QQmlListProperty<FileSyncEntry> fileEntries READ fileEntries NOTIFY fileEntriesChanged)
    Q_PROPERTY(bool boardConnected READ boardConnected WRITE setBoardConnected NOTIFY boardConnectedChanged)
    Q_PROPERTY(int fileEntriesCount READ fileEntriesCount NOTIFY fileEntriesChanged)
    Q_PROPERTY(QStringList device READ device NOTIFY deviceChanged)

public:
    explicit FileSyncManager(QObject *parent = 0);
    ~FileSyncManager();

    QQmlListProperty<FileSyncEntry> fileEntries();

    void clearFileList();

    bool boardConnected() const
    {
        return m_boardConnected;
    }

    bool disableControls() const
    {
        return m_disableControls;
    }

    QStringList device() const
    {
        return m_device;
    }

signals:
    void fileEntriesChanged();
    void boardConnectedChanged(bool arg);
    void disableControlsChanged(bool arg);
    void deviceChanged();

public slots:
    void format();
    void cancelFileSync();
    void listFiles();
    void deleteFile(int filenameidx);
    void downloadFile(int filenameidx);
    void uploadFile();
    int deviceFreeKBytes()
    {
        // get device stats.
        return 2048;
    }

    void setDeviceIdx(int arg)
    {
        qDebug() << "[SYNC] SetDeviceIdx:" << arg;
        if (m_deviceIdx != arg) {
            m_deviceIdx = arg;
            //emit deviceIdxChanged(arg);
        }
    }

    int fileEntriesCount()
    {
        return m_fileEntries.count();
    }

    void setBoardConnected(bool arg)
    {
        qDebug() << "[SYNC] SetBoardConnected: " << arg;
        if (m_boardConnected != arg) {
            m_boardConnected = arg;
            emit boardConnectedChanged(arg);
        }
    }

    void setDisableControls(bool arg)
    {
        qDebug() << "[SYNC] SetDisableControls:" << arg;
        if (m_disableControls != arg) {
            m_disableControls = arg;
            emit disableControlsChanged(arg);
        }
    }

private slots:
    void connectionStatusChanged();
    void deviceSettings();

private:
    UAVObjectManager *m_objectManager;
    UAVObjectUtilManager *m_objectUtilManager;
    TelemetryManager *m_telemtryManager;
    ObjectPersistence *m_objectPersistence;
    Sync *m_sync;

    QList<FileSyncEntry *> m_fileEntries;
    QStringList m_device;

    static const int UAVTALK_TIMEOUT = 4000;
    bool m_cancelSync;
    bool m_boardConnected;
    bool m_disableControls;
    int m_deviceIdx;
};

#endif // FILESYNCMANAGER_H

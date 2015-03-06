/**
 ******************************************************************************
 *
 * @file       flightlogmanager.cpp
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

#include "filesyncmanager.h"
#include "extensionsystem/pluginmanager.h"

#include <QApplication>
#include <QFileDialog>
#include <QXmlStreamReader>
#include <QMessageBox>
#include <QDebug>
#include <QTextStream>

#include "sync.h"
#include "uavobjecthelper.h"
#include "uavtalk/uavtalk.h"
#include "utils/logfile.h"
#include "uavdataobject.h"
#include <uavobjectutil/uavobjectutilmanager.h>

#define TXT_FILE_EXT ".txt"
#define LUA_FILE_EXT ".lua"


FileSyncManager::FileSyncManager(QObject *parent) :
    QObject(parent), m_cancelSync(false), m_disableControls(false)
{
    ExtensionSystem::PluginManager *pluginManager = ExtensionSystem::PluginManager::instance();

    Q_ASSERT(pluginManager);

    m_objectManager = pluginManager->getObject<UAVObjectManager>();
    Q_ASSERT(m_objectManager);

    m_telemtryManager = pluginManager->getObject<TelemetryManager>();
    Q_ASSERT(m_telemtryManager);

    m_objectUtilManager = pluginManager->getObject<UAVObjectUtilManager>();
    Q_ASSERT(m_objectUtilManager);

    m_sync  = Sync::GetInstance(m_objectManager);
    Q_ASSERT(m_sync);

    m_objectPersistence = ObjectPersistence::GetInstance(m_objectManager);
    Q_ASSERT(m_objectPersistence);

    deviceSettings();

    connect(m_telemtryManager, SIGNAL(connected()), this, SLOT(connectionStatusChanged()));
    connect(m_telemtryManager, SIGNAL(disconnected()), this, SLOT(connectionStatusChanged()));
    connectionStatusChanged();
}


FileSyncManager::~FileSyncManager()
{
    while (!m_fileEntries.isEmpty()) {
        delete m_fileEntries.takeFirst();
    }
}


void addFileEntries(QQmlListProperty<FileSyncEntry> *list, FileSyncEntry *entry)
{
    Q_UNUSED(list);
    Q_UNUSED(entry);
}


int countFileEntries(QQmlListProperty<FileSyncEntry> *list)
{
    return static_cast< QList<FileSyncEntry *> *>(list->data)->size();
}


FileSyncEntry *fileEntryAt(QQmlListProperty<FileSyncEntry> *list, int index)
{
    return static_cast< QList<FileSyncEntry *> *>(list->data)->at(index);
}


void clearFileEntries(QQmlListProperty<FileSyncEntry> *list)
{
    return static_cast< QList<FileSyncEntry *> *>(list->data)->clear();
}


QQmlListProperty<FileSyncEntry> FileSyncManager::fileEntries()
{
    return QQmlListProperty<FileSyncEntry>(this, &m_fileEntries, &addFileEntries, &countFileEntries, &fileEntryAt, &clearFileEntries);
}


void FileSyncManager::deviceSettings()
{
    // Corresponds to:
    // typedef enum {
    // DEVICE_EXTERNALFLASH    = 0, /** external onboard SPI flash */
    // DEVICE_INTERNALFLASH    = 1, /** internal flash */
    // DEVICE_MICROSDCARD      = 2, /** uSD card (for board with SD card reader only) */
    // DEVICE_USBSTICK         = 3  /** USB stick (for board with additional USB_OTG port) */
    // DEVICE_GPS              = 4  /** GPS (for board with OP-GPSv9 connected) */
    // } Device;

    m_device << tr("ExternalFlash") << tr("InternalFlash") << tr("MicroSDCard") << tr("USBStick") << tr("GPS");
}


void FileSyncManager::format()
{
    setDisableControls(true);
    QApplication::setOverrideCursor(Qt::WaitCursor);

    UAVObjectUpdaterHelper updateHelper;

    qDebug() << "[SYNC] Format: START";

    m_sync->setDevice(Sync::DEVICE_EXTERNALFLASH);
    m_sync->setCommand(Sync::COMMAND_FORMAT);

    updateHelper.doObjectAndWait(m_sync, UAVTALK_TIMEOUT);

    emit fileEntriesChanged();

    listFiles();

    qDebug() << "[SYNC] Format: END";

    QApplication::restoreOverrideCursor();
    setDisableControls(false);
}


void FileSyncManager::listFiles()
{
    setDisableControls(true);
    QApplication::setOverrideCursor(Qt::WaitCursor);
    m_cancelSync = false;

    UAVObjectUpdaterHelper updateHelper;
    UAVObjectRequestHelper requestHelper;

    qDebug() << "[SYNC] ListFiles: START";

    m_sync->setTotalSize(Sync::DATA_NUMELEM);
    m_sync->setDataSize(Sync::DATA_NUMELEM);
    m_sync->setCommand(Sync::COMMAND_LIST);
    m_sync->setDevice(/*m_deviceIdx*/0);
    m_sync->setOffset(0);

    clearFileList();

    while(m_sync->getOffset() <= m_sync->getTotalSize()) {

        /* The last object might be smaller, adjust the size */
        if ((m_sync->getTotalSize() - m_sync->getOffset()) < m_sync->getDataSize())
            m_sync->setDataSize(m_sync->getTotalSize() - m_sync->getOffset());

        if (m_sync->getDataSize() &&
            (updateHelper.doObjectAndWait(m_sync, UAVTALK_TIMEOUT) == UAVObjectUpdaterHelper::SUCCESS) &&
            (requestHelper.doObjectAndWait(m_sync, UAVTALK_TIMEOUT) == UAVObjectRequestHelper::SUCCESS)) {
            // Retrieve the files and their sizes and add to the file list.
            const quint32 file_size = Sync::NAME_NUMELEM;
            const quint32 file_info_size = file_size + sizeof(m_sync->getData().TotalSize);
            quint32 file_count = m_sync->getDataSize() / file_info_size;
            qDebug() << "[SYNC] File_info_size:" << file_info_size;
            if (!file_count) {
                break;
            }
            for (quint32 i = 0; i < file_count; i++) {
                FileSyncEntry *fileEntry = new FileSyncEntry();
                // The order of the two fields matters QStrings need the size as parameter.
                fileEntry->setSize(*(quint32*)&m_sync->getData().Data[i*file_info_size + file_size]);
                fileEntry->setName((char*)&m_sync->getData().Data[i*file_info_size]);
                fileEntry->setType(5);
                m_fileEntries << fileEntry;
                if (m_cancelSync) {
                    break;
                }
                qDebug() << "[SYNC] File #" << (i + 1);
                qDebug() << "[SYNC] Name:" << (char*)&m_sync->getData().Data[i*file_info_size];
                //qDebug() << "[SYNC] Name:" << fileEntry->getFileName();
                qDebug() << "[SYNC] Size:" << *(quint32*)&m_sync->getData().Data[i*file_info_size + file_size];

            }
            m_sync->setOffset(m_sync->getOffset() + m_sync->getDataSize());
        }
        else
        {
            break;
        }

        if (m_cancelSync) {
            break;
        }
    }

    if (m_cancelSync) {
        clearFileList();
        m_cancelSync = false;
    }

    emit fileEntriesChanged();

    qDebug() << "[SYNC] ListFiles: END";

    QApplication::restoreOverrideCursor();
    setDisableControls(false);
}


void FileSyncManager::clearFileList()
{
    QList<FileSyncEntry *> tmpList(m_fileEntries);
    m_fileEntries.clear();

    emit fileEntriesChanged();
    //setDisableExport(true);
    qDebug() << "[SYNC] ClearFileList";
    while (!tmpList.isEmpty()) {
        delete tmpList.takeFirst();
    }
}


void FileSyncManager::deleteFile(int filenameidx)
{
    setDisableControls(true);
    QApplication::setOverrideCursor(Qt::WaitCursor);
    m_cancelSync = false;

    UAVObjectUpdaterHelper updateHelper;
    UAVObjectRequestHelper requestHelper;

    qDebug() << "[SYNC] DeleteFile: START";
    qDebug() << "[SYNC] File #" << filenameidx;

    // Set request.
    Sync::DataFields fields;
    memset(&fields, 0, sizeof(Sync::DataFields));
    fields.Command = Sync::COMMAND_DELETE;
    fields.Device = Sync::DEVICE_EXTERNALFLASH;

    // Get the file name to delete.
    FileSyncEntry *entry = m_fileEntries.at(filenameidx);
    memcpy(fields.Name, entry->getFileName().toLocal8Bit().data(), Sync::NAME_NUMELEM);

    m_sync->setData(fields);

    if (updateHelper.doObjectAndWait(m_sync, UAVTALK_TIMEOUT) == UAVObjectUpdaterHelper::SUCCESS) {
        qDebug() << "[SYNC] File deleted:" << (char*)m_sync->getData().Name;
        qDebug() << "[SYNC] Status:" << m_sync->getData().Status;
    }

    if (m_cancelSync) {
        clearFileList();
        m_cancelSync = false;
    }

    emit fileEntriesChanged();

    listFiles();

    qDebug() << "[SYNC] DeleteFile: END";

    QApplication::restoreOverrideCursor();
    setDisableControls(false);
}


void FileSyncManager::downloadFile(int filenameidx)
{
    char *buffer = NULL;
    setDisableControls(true);
    QApplication::setOverrideCursor(Qt::WaitCursor);
    m_cancelSync = false;

    bool err_req, err_res;
    UAVObjectUpdaterHelper updateHelper;
    UAVObjectRequestHelper requestHelper;

    qDebug() << "[SYNC] DownloadFile: START";
    qDebug() << "[SYNC] File #" << filenameidx;

    // Set request
    Sync::DataFields fields;
    memset(&fields, 0, sizeof(Sync::DataFields));
    FileSyncEntry *entry = m_fileEntries.at(filenameidx);
    memcpy(fields.Name, entry->getFileName().toLocal8Bit().data(), Sync::NAME_NUMELEM);
    fields.TotalSize = entry->getFileSize();
    fields.DataSize = Sync::DATA_NUMELEM;
    fields.Command = Sync::COMMAND_DOWNLOAD;
    fields.Device = Sync::DEVICE_EXTERNALFLASH;
    fields.Offset = 0;
    m_sync->setData(fields);

    while(m_sync->getOffset() <= m_sync->getTotalSize()) {

        /* The last object might be smaller, adjust the size */
        if ((m_sync->getTotalSize() - m_sync->getOffset()) < m_sync->getDataSize())
            m_sync->setDataSize(m_sync->getTotalSize() - m_sync->getOffset());

        if (m_sync->getDataSize() &&
            (err_req = (updateHelper.doObjectAndWait(m_sync, UAVTALK_TIMEOUT)) == UAVObjectUpdaterHelper::SUCCESS) &&
            (err_res = (requestHelper.doObjectAndWait(m_sync, UAVTALK_TIMEOUT)) == UAVObjectRequestHelper::SUCCESS)) {

            // Retrieve info from the packet.
            const quint32 file_size = m_sync->getTotalSize();
            const quint32 file_packet_size = m_sync->getDataSize();
            const quint32 file_packet_offset = m_sync->getOffset();

            qDebug() << "[SYNC] File size:" << file_size;
            qDebug() << "[SYNC] File packet size:" << file_packet_size;
            qDebug() << "[SYNC] File packet offset:" << file_packet_offset;
            qDebug() << "[SYNC] File name:" << (char*)m_sync->getData().Name;
            qDebug() << "[SYNC] Status:" << m_sync->getData().Status;

            // There is no data in that object, that's unexpected.
            if (!file_packet_size) {
                // TODO: out of sync: we should have already got the full file
                qDebug() << "[SYNC] ERROR: Out of sync...";
                break;
            }

            // Get the file system stats of that device and compare with the max capacity of that device.
            if (0/*file_size > 2*1024*1024*/) {
                // TODO: if the file size if bigger than the capacity of the device, something is up
                // Ex: the external SPI flash is 2MB on the revo. If the file is 3MB, that's not good.
                // TODO: Add capacity in the file status object and break it down per device.
                qDebug() << "[SYNC] ERROR: File too big...";
                break;
            }

            // If this is the first packet, there is extra work to do...
            if (!file_packet_offset) {
                buffer = new char[file_size];
                if (!buffer) {
                    qDebug() << "[SYNC] ERROR: Could not allocate buffer...";
                    break;
                }
            }

            // Copy payload
            memcpy(buffer+file_packet_offset, m_sync->getData().Data, file_packet_size);

            // Prepare next request
            m_sync->setOffset(file_packet_offset + file_packet_size);

            // If this is the last packet, we need to store that file and do some house cleaning
            if (m_sync->getOffset() == file_size) {

                QFile::OpenMode flag = QFile::WriteOnly | QFile::Truncate;

                // Default to home directory with original file name
                QString Filter = tr("file %1").arg("Text files (*.txt);; Lua scripts (*.lua);; Log files (*.log);; uavo files (*.uavo)");
                QString fileName  = QFileDialog::getSaveFileName(NULL, tr("Save File"),
                                                                 QDir::homePath()+"/"+(char*)m_sync->getData().Name, QString("%1").arg(Filter));

                if (!fileName.isEmpty()) {

                    /* The QIODevice::Text flag passed to open() tells Qt to convert Windows-style
                     * line terminators ("\r\n") into C++-style terminators ("\n").
                     * By default, QFile assumes binary, i.e. it doesn't perform any conversion
                     * on the bytes stored in the file.*/
                    if (fileName.endsWith(TXT_FILE_EXT) ||
                        fileName.endsWith(LUA_FILE_EXT))
                        flag |= QIODevice::Text;

                    // save file.
                    QFile saveFile(fileName);
                    if (saveFile.open(flag)) {
                        saveFile.write(buffer);
                        saveFile.flush();
                        saveFile.close();
                        qDebug() << "[SYNC] File saved:" << fileName;
                    }
                }
                else
                {
                    qDebug() << "[SYNC] Error, no file selected!";
                }
            }
        }
        else
        {
            if (err_req == UAVObjectUpdaterHelper::TIMEOUT)
                qDebug() << "[SYNC] Error Req Timeout!!!";
            if (err_res == UAVObjectRequestHelper::TIMEOUT)
                qDebug() << "[SYNC] Error Res Timeout!!!";
            break;
        }

        if (m_cancelSync) {
            break;
        }
    }

    if (buffer)
        delete(buffer);

    if (m_cancelSync) {
        clearFileList();
        m_cancelSync = false;
    }

    emit fileEntriesChanged();

    qDebug() << "[SYNC] DownloadFile: END";

    QApplication::restoreOverrideCursor();
    setDisableControls(false);
}


void FileSyncManager::uploadFile()
{
    char *buffer = NULL;
    setDisableControls(true);
    QApplication::setOverrideCursor(Qt::WaitCursor);
    m_cancelSync = false;

    bool err_req, err_res;
    UAVObjectUpdaterHelper updateHelper;
    UAVObjectRequestHelper requestHelper;

    qDebug() << "[SYNC] UploadFile: START";

    QFile::OpenMode flag = QFile::ReadOnly;

    // Default to home directory with original file name
    QString Filter = tr("file %1").arg("Text files (*.txt);; Lua scripts (*.lua);; Log files (*.log);; uavo files (*.uavo)");
    QString fileName = QFileDialog::getOpenFileName(NULL, tr("Select File to upload"),
                                                    QDir::homePath(),
                                                    QString("%1").arg(Filter));

    if (!fileName.isEmpty()) {

        // Buffer the file to send
        /* The QIODevice::Text flag passed to open() tells Qt to convert Windows-style
         * line terminators ("\r\n") into C++-style terminators ("\n").
         * By default, QFile assumes binary, i.e. it doesn't perform any conversion
         * on the bytes stored in the file.*/
        if (fileName.endsWith(TXT_FILE_EXT) ||
            fileName.endsWith(LUA_FILE_EXT))
            flag |= QIODevice::Text;

        QFile openFile(fileName);
        QFileInfo fileInfo(fileName);

        // Only keep the filename
        fileName = fileInfo.fileName();
        const quint32 file_size = openFile.size();

        qDebug() << "[SYNC] File name:" << fileName;
        qDebug() << "[SYNC] File size:" << file_size;

        buffer = new char[file_size];

        if (!buffer) {
            qDebug() << "[SYNC] ERROR: Could not allocate buffer...";
            goto exit;
        }

        if (!openFile.open(flag)) {
            qDebug() << "[SYNC] ERROR: Could not open the file (fileName)...";
            goto exit;
        }

        // Fill buffer
        openFile.read(buffer, file_size);
        openFile.close();

        // Prepare object to send
        Sync::DataFields fields;
        memset(&fields, 0, sizeof(Sync::DataFields));
        memcpy(fields.Name, fileName.toLocal8Bit().data(), Sync::NAME_NUMELEM);
        fields.TotalSize = (qint64)file_size;
        fields.DataSize = Sync::DATA_NUMELEM;
        fields.Command = Sync::COMMAND_UPLOAD;
        fields.Device = Sync::DEVICE_EXTERNALFLASH;
        fields.Offset = 0;
        memcpy(fields.Data, buffer, fields.DataSize);
        m_sync->setData(fields);

        // Split payload and update object until entire file is sent
        while(m_sync->getOffset() <= m_sync->getTotalSize()) {

            /* The last object might be smaller, adjust the size */
            if ((m_sync->getTotalSize() - m_sync->getOffset()) < m_sync->getDataSize())
                m_sync->setDataSize(m_sync->getTotalSize() - m_sync->getOffset());

            if (m_sync->getDataSize() &&
                (err_req = (updateHelper.doObjectAndWait(m_sync, UAVTALK_TIMEOUT)) == UAVObjectUpdaterHelper::SUCCESS) &&
                (err_res = (requestHelper.doObjectAndWait(m_sync, UAVTALK_TIMEOUT)) == UAVObjectRequestHelper::SUCCESS)) {

                // Very unlikely the payload in the object would change dynamically but let's assum it could.
                const quint32 file_packet_size = m_sync->getDataSize();
                const quint32 file_packet_offset = m_sync->getOffset();

                qDebug() << "[SYNC] File packet size:" << file_packet_size;
                qDebug() << "[SYNC] File packet offset:" << file_packet_offset;
                qDebug() << "[SYNC] Status:" << m_sync->getData().Status;

                // Copy payload
                memcpy(m_sync->getData().Data, buffer+file_packet_offset, file_packet_size);

                // Prepare next request
                m_sync->setOffset(file_packet_offset + file_packet_size);
            }
            else
            {
                if (err_req == UAVObjectUpdaterHelper::TIMEOUT)
                    qDebug() << "[SYNC] Error Req Timeout!!!";
                if (err_res == UAVObjectRequestHelper::TIMEOUT)
                    qDebug() << "[SYNC] Error Res Timeout!!!";
                break;
            }

            if (m_cancelSync) {
                break;
            }
        }
    }

exit:
    if (buffer)
        delete(buffer);

    if (m_cancelSync) {
        clearFileList();
        m_cancelSync = false;
    }

    emit fileEntriesChanged();

    listFiles();

    qDebug() << "[SYNC] UploadFile: END";

    QApplication::restoreOverrideCursor();
    setDisableControls(false);
}


void FileSyncManager::cancelFileSync()
{
    qDebug() << "[SYNC] CancelFileSync";
    m_cancelSync = true;
}


void FileSyncManager::connectionStatusChanged()
{
    if (m_telemtryManager->isConnected()) {
        ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
        UAVObjectUtilManager *utilMngr = pm->getObject<UAVObjectUtilManager>();
        setBoardConnected(utilMngr->getBoardModel() == 0x0903);
    } else {
        setBoardConnected(false);
    }
    if (boardConnected()) {
        listFiles();
    }
}


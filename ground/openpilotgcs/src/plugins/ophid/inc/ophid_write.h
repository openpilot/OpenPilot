/**
 ******************************************************************************
 *
 * @file       ophid_write.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2013.
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup opHIDPlugin HID Plugin
 * @{
 * @brief Impliments a HID USB connection to the flight hardware as a QIODevice
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

#ifndef OPHID_WRITE_H
#define OPHID_WRITE_H

#include "ophid.h"
#include "ophid_const.h"
#include "coreplugin/connectionmanager.h"
#include <extensionsystem/pluginmanager.h>
#include <QtGlobal>
#include <QList>
#include <QMutexLocker>
#include <QSemaphore>
#include <QWaitCondition>


class opHID;


class opHIDWriteWorker : public QObject {
    Q_OBJECT

public:

    opHIDWriteWorker(opHID *hid);
    virtual ~opHIDWriteWorker();

    // Add some data to be written without waiting
    int pushDataToWrite(const char *data, int size);

    // Return the number of bytes buffered
    qint64 getBytesToWrite();

public slots:

    void process();

    void stop();

signals:

    void finished();

    qint64 bytesSent(qint64 bytes);

protected:

    QByteArray m_writeBuffer;

    QMutex m_writeBufMtx;

    QMutex m_leaveSigMtx;

    QSemaphore m_msg_sem;

    opHID *m_hid;

    // Flag to leave thread.
    bool m_terminate;
};

#endif // ifndef OPHID_WRITE_H

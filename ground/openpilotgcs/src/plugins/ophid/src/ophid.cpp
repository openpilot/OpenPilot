/**
 ******************************************************************************
 *
 * @file       rawhid.cpp
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

#include "ophid.h"
#include "ophid_const.h"
#include "coreplugin/connectionmanager.h"
#include <extensionsystem/pluginmanager.h>
#include <QtGlobal>
#include <QList>
#include <QMutexLocker>
#include <QWaitCondition>


class IConnection;

opHID::opHID() : QIODevice()
{
    OPHID_TRACE("IN");

    m_mutex = new QMutex(QMutex::Recursive);


    // Move workers to thread
    writeWorker = new opHIDWriteWorker(this);

    // Bind worker to thread
    writeWorker->moveToThread(&writeThread);

    //
    // connect(writeWorker, SIGNAL(&opHIDWriteWorker::bytesSent(qint64)), this, SLOT(&QIODevice::bytesWritten(qint64)));

    // Connect the Thread's started() signal to the process slot in the writeWorker
    connect(&writeThread, SIGNAL(started()), writeWorker, SLOT(process()));

    // Signal the thread to quit when finished signal is triggered from worker.
    connect(writeWorker, SIGNAL(finished()), &writeThread, SLOT(quit()));

    // Mark worker using the same finish signal for deletion
    connect(writeWorker, SIGNAL(finished()), writeWorker, SLOT(deleteLater()));

    // Delete thread only after it has fully shut down.
    connect(&writeThread, SIGNAL(finished()), &writeThread, SLOT(deleteLater()));


    // READ thread (read from USB device)
    readWorker = new opHIDReadWorker(this);

    // Bind worker to thread
    readWorker->moveToThread(&readThread);

    // Connect the Thread's started() signal to the process slot in the writeWorker
    connect(&readThread, SIGNAL(started()), readWorker, SLOT(process()));

    // Signal the thread to quit when finished signal is triggered from worker.
    connect(readWorker, SIGNAL(finished()), &readThread, SLOT(quit()));

    // Mark worker using the same finish signal for deletion
    connect(readWorker, SIGNAL(finished()), readWorker, SLOT(deleteLater()));

    // Delete thread only after it has fully shut down.
    connect(&readThread, SIGNAL(finished()), &readThread, SLOT(deleteLater()));

    OPHID_TRACE("OUT");
}


opHID::~opHID()
{
    OPHID_TRACE("IN");

    OPHID_TRACE("OUT");
}


/**
 * @brief deviceBind. This method register the USB device to thread
 */
void opHID::deviceBind(opHID_hidapi *deviceHandle)
{
    OPHID_TRACE("IN");

    m_deviceHandle = deviceHandle;

    writeThread.start();

    readThread.start();

    OPHID_TRACE("OUT");
}


/**
 * @brief The USB device is not present anymore, Stop the thread.
 */
bool opHID::deviceUnbind(void)
{
    OPHID_TRACE("IN");

    // Prevent user to access the QIODevice
    close();

    // Stop writing to device Thread
    if (writeWorker) {
        writeWorker->stop();
        if (!writeThread.wait(5000)) {
            OPHID_ERROR("Deadlock detected trying to stop write Thread!");
            writeThread.terminate();
            writeThread.wait();
        }
    }

    // Stop reading from device Thread
    if (readWorker) {
        emit readWorker->stop();
        if (!readThread.wait(5000)) {
            OPHID_ERROR("Deadlock detected trying to stop read Thread!");
            readThread.terminate();
            readThread.wait();
        }
    }

    m_deviceHandle = NULL;

    OPHID_TRACE("OUT");

    return 0;
}


/**
 * @brief Open officially the QIODevice for read/write operation
 */
bool opHID::open(OpenMode mode)
{
    OPHID_TRACE("IN");

    QMutexLocker locker(m_mutex);

    QIODevice::open(mode);

    OPHID_TRACE("OUT");

    return true;
}


/**
 * @brief Used from the read thread to trigger the event on the QIODevice
 *        to tell there is something ready to be read (this will unblock blocking read)
 */
void opHID::readyRead(void)
{
    QIODevice::readyRead();
}


/**
 * @brief Close officialy the QIODevice for read/write operation
 */
void opHID::close()
{
    OPHID_TRACE("IN");

    QIODevice::close();

    OPHID_TRACE("OUT");
}


/**
 * @brief Helper function
 */
opHID_hidapi *opHID::deviceInstanceGet(void)
{
    return m_deviceHandle;
}


/**
 * @brief Nature of the IODevice
 */
bool opHID::isSequential() const
{
    return true;
}


/**
 * @brief Number of bytes available for reading
 */
qint64 opHID::bytesAvailable() const
{
    QMutexLocker locker(m_mutex);

    if (!readWorker || !m_deviceHandle) {
        return -1;
    }

    return readWorker->getBytesAvailable() + QIODevice::bytesAvailable();
}


/**
 * @brief Number of bytes ready to be written
 */
qint64 opHID::bytesToWrite() const
{
    QMutexLocker locker(m_mutex);

    if (!writeWorker || !m_deviceHandle) {
        return -1;
    }

    return writeWorker->getBytesToWrite() + QIODevice::bytesToWrite();
}


/**
 * @brief QIODevice read callback
 */
qint64 opHID::readData(char *data, qint64 maxSize)
{
    QMutexLocker locker(m_mutex);

    if (!readWorker || !data || !m_deviceHandle) {
        return -1;
    }

    return readWorker->getReadData(data, (int)maxSize);
}


/**
 * @brief QIODevice write callback
 */
qint64 opHID::writeData(const char *data, qint64 maxSize)
{
    QMutexLocker locker(m_mutex);

    if (!writeWorker || !data || !m_deviceHandle) {
        return -1;
    }

    return writeWorker->pushDataToWrite(data, maxSize);
}

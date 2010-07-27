/**
 ******************************************************************************
 *
 * @file       rawhid.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup RawHIDPlugin Raw HID Plugin
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

#include "rawhid.h"

#include "rawhid_const.h"

#include <QtGlobal>
#include <QMutexLocker>
#include <QWaitCondition>

//timeout value used when we want to return directly without waiting
static const int READ_TIMEOUT = 200;
static const int READ_SIZE = 64;

static const int WRITE_TIMEOUT = 200;
static const int WRITE_SIZE = 64;




/**
*   Thread to desynchronize reading from the device
*/
class RawHIDReadThread : public QThread
{
public:
    RawHIDReadThread(RawHID *hid);
    virtual ~RawHIDReadThread();

    /** Return the data read so far without waiting */
    int getReadData(char *data, int size);

    /** return the bytes buffered */
    qint64 getBytesAvailable();

protected:
    void run();

    /** QByteArray might not be the most efficient way to implement
    a circular buffer but it's good enough and very simple */
    QByteArray m_readBuffer;

    /** A mutex to protect read buffer */
    QMutex m_readBufMtx;

    RawHID *m_hid;

    pjrc_rawhid *hiddev;
    int hidno;

    bool m_running;
};


/**
*  This class is nearly the same than RawHIDReadThread but for writing
*/
class RawHIDWriteThread : public QThread
{
public:
    RawHIDWriteThread(RawHID *hid);
    virtual ~RawHIDWriteThread();

    /** Add some data to be written without waiting */
    int pushDataToWrite(const char *data, int size);

    /** Return the number of bytes buffered */
    qint64 getBytesToWrite();

protected:
    void run();

    /** QByteArray might not be the most efficient way to implement
    a circular buffer but it's good enough and very simple */
    QByteArray m_writeBuffer;

    /** A mutex to protect read buffer */
    QMutex m_writeBufMtx;

    /** Synchronize task with data arival */
    QWaitCondition m_newDataToWrite;

    RawHID *m_hid;

    pjrc_rawhid *hiddev;
    int hidno;

    bool m_running;
};



RawHIDReadThread::RawHIDReadThread(RawHID *hid)
    : m_hid(hid),
    hiddev(&hid->dev),
    hidno(hid->m_deviceNo),
    m_running(true)
{
}

RawHIDReadThread::~RawHIDReadThread()
{
    m_running = false;
    //wait for the thread to terminate
    if(wait(1000) == false)
        qDebug() << "Cannot terminate RawHIDReadThread";
}

void RawHIDReadThread::run()
{
    while(m_running)
    {
        //here we use a temporary buffer so we don't need to lock
        //the mutex while we are reading from the device
        char buffer[READ_SIZE] = {0};
        int ret = hiddev->receive(hidno, buffer, READ_SIZE, READ_TIMEOUT);

        if(ret > 0) //read some data
        {
            m_readBufMtx.lock();
            /* First byte is report ID, can be ignored */
            /* Second byte is the number of valid bytes */
            m_readBuffer.append(&buffer[2], buffer[1]);
            // m_readBuffer.append(buffer, ret);
            m_readBufMtx.unlock();

            emit m_hid->readyRead();
        }
        else if(ret == 0) //nothing read
        {
        }
        else // < 0 => error
        {
        }
    }
}

int RawHIDReadThread::getReadData(char *data, int size)
{
    QMutexLocker lock(&m_readBufMtx);

    size = qMin(size, m_readBuffer.size());

    memcpy(data, m_readBuffer.constData(), size);
    m_readBuffer.remove(0, size);

    return size;
}

qint64 RawHIDReadThread::getBytesAvailable()
{
    QMutexLocker lock(&m_readBufMtx);
    return m_readBuffer.size();
}

RawHIDWriteThread::RawHIDWriteThread(RawHID *hid)
    : m_hid(hid),
    hiddev(&hid->dev),
    hidno(hid->m_deviceNo),
    m_running(true)
{
}

RawHIDWriteThread::~RawHIDWriteThread()
{
    m_running = false;
    //wait for the thread to terminate
    if(wait(1000) == false)
        qDebug() << "Cannot terminate RawHIDReadThread";
}

void RawHIDWriteThread::run()
{
    qDebug() << "Write thread started";
    while(m_running)
    {
        char buffer[WRITE_SIZE] = {0};
        m_writeBufMtx.lock();
        int size = qMin(WRITE_SIZE, m_writeBuffer.size());
        while(size <= 0)
        {
            //wait on new data to write condition, the timeout
            //enable the thread to shutdown properly
            m_newDataToWrite.wait(&m_writeBufMtx, 200);
            if(!m_running)
                return;
            else
                size = qMin(WRITE_SIZE, m_writeBuffer.size());
        }

        //make a temporary copy so we don't need to lock the mutex
        //during actual device access
        memcpy(buffer, m_writeBuffer.constData(), size);
        m_writeBufMtx.unlock();

        int ret = hiddev->send(hidno, buffer, size, WRITE_TIMEOUT);

        if(ret > 0)
        {
            //only remove the size actually written to the device
            m_writeBufMtx.lock();
            m_writeBuffer.remove(0, ret);
            m_writeBufMtx.unlock();

            emit m_hid->bytesWritten(ret);
        }
        else if(ret < 0) // < 0 => error
        {
        }
        else
        {
            qDebug() << "No data written to device ??";
        }
    }
}

int RawHIDWriteThread::pushDataToWrite(const char *data, int size)
{
    //QMutexLocker lock(&m_writeBufMtx);

    m_writeBuffer.append(data, size);
    m_newDataToWrite.wakeOne(); //signal that new data arrived
    return size;
}

qint64 RawHIDWriteThread::getBytesToWrite()
{
    // QMutexLocker lock(&m_writeBufMtx);
    return m_writeBuffer.size();
}

RawHID::RawHID(const QString &deviceName)
    :QIODevice(),
    serialNumber(deviceName),
    m_deviceNo(-1),
    m_readThread(NULL),
    m_writeThread(NULL)
{
    //find the device the user want to open and close the other
    int opened = dev.open(MAX_DEVICES, VID, PID, USAGE_PAGE, USAGE);

    //for each devices found, get serial number and close
    for(int i=0; i<opened; i++)
    {
        char serial[256];
        dev.getserial(i, serial);
        if(deviceName == QString::fromAscii(serial, DEV_SERIAL_LEN))
            m_deviceNo = i;
        else
            dev.close(i);
    }

    //didn't find the device we are trying to open (shouldnt happen)
    if(m_deviceNo < 0)
    {
        qDebug() << "Error: cannot open device " << deviceName;
        return;
    }

    m_readThread = new RawHIDReadThread(this);
    m_writeThread = new RawHIDWriteThread(this);

    m_readThread->start();
    m_writeThread->start();
}

RawHID::~RawHID()
{
    if(m_readThread)
        delete m_readThread;

    if(m_writeThread)
        delete m_writeThread;
}

bool RawHID::open(OpenMode mode)
{
    if(m_deviceNo < 0)
        return false;

    QIODevice::open(mode);

    if(!m_readThread)
        m_readThread = new RawHIDReadThread(this);

    if(!m_writeThread)
        m_writeThread = new RawHIDWriteThread(this);

    return true;
}

void RawHID::close()
{
    if(m_readThread)
    {
        delete m_readThread;
        m_readThread = NULL;
    }

    if(m_writeThread)
    {
        delete m_writeThread;
        m_writeThread = NULL;
    }

    dev.close(m_deviceNo);

    QIODevice::close();
}

bool RawHID::isSequential() const
{
    return true;
}

qint64 RawHID::bytesAvailable() const
{
    return m_readThread->getBytesAvailable() + QIODevice::bytesAvailable();
}

qint64 RawHID::bytesToWrite() const
{
    return m_writeThread->getBytesToWrite() + QIODevice::bytesToWrite();
}

qint64 RawHID::readData(char *data, qint64 maxSize)
{
    return m_readThread->getReadData(data, maxSize);
}

qint64 RawHID::writeData(const char *data, qint64 maxSize)
{
    return m_writeThread->pushDataToWrite(data, maxSize);
}


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
#include "coreplugin/connectionmanager.h"
#include <extensionsystem/pluginmanager.h>
#include <QtGlobal>
#include <QList>
#include <QMutexLocker>
#include <QWaitCondition>

class IConnection;

//timeout value used when we want to return directly without waiting
static const int READ_TIMEOUT = 200;
static const int READ_SIZE = 64;

static const int WRITE_TIMEOUT = 1000;
static const int WRITE_SIZE = 64;



// *********************************************************************************

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

public slots:
    void terminate() {
        m_running = false;
    }

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


// *********************************************************************************

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

public slots:
    void terminate() {
        m_running = false;
    }

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

// *********************************************************************************

RawHIDReadThread::RawHIDReadThread(RawHID *hid)
    : m_hid(hid),
    hiddev(&hid->dev),
    hidno(hid->m_deviceNo),
    m_running(true)
{
    hid->m_startedMutex->lock();
}

RawHIDReadThread::~RawHIDReadThread()
{
    m_running = false;
    //wait for the thread to terminate
    if(wait(10000) == false)
        qDebug() << "Cannot terminate RawHIDReadThread";
}

void RawHIDReadThread::run()
{
    m_running = m_hid->openDevice();
    while(m_running)
    {
        //here we use a temporary buffer so we don't need to lock
        //the mutex while we are reading from the device

        // Want to read in regular chunks that match the packet size the device
        // is using.  In this case it is 64 bytes (the interrupt packet limit)
        // although it would be nice if the device had a different report to
        // configure this
        char buffer[READ_SIZE] = {0};

        int ret = hiddev->receive(hidno, buffer, READ_SIZE, READ_TIMEOUT);

        if(ret > 0) //read some data
        {
            QMutexLocker lock(&m_readBufMtx);
            // Note: Preprocess the USB packets in this OS independent code
            // First byte is report ID, second byte is the number of valid bytes
            m_readBuffer.append(&buffer[2], buffer[1]);

            emit m_hid->readyRead();
        }
        else if(ret == 0) //nothing read
        {
        }
        else // < 0 => error
        {
            //TODO! make proper error handling, this only quick hack for unplug freeze
            m_running=false;
        }
    }
    m_hid->closeDevice();
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

// *********************************************************************************

RawHIDWriteThread::~RawHIDWriteThread()
{
    m_running = false;
    //wait for the thread to terminate
    if(wait(10000) == false)
        qDebug() << "Cannot terminate RawHIDReadThread";
}

void RawHIDWriteThread::run()
{
    while(m_running)
    {
        char buffer[WRITE_SIZE] = {0};

        m_writeBufMtx.lock();
        int size = qMin(WRITE_SIZE-2, m_writeBuffer.size());
        while(size <= 0)
        {
            //wait on new data to write condition, the timeout
            //enable the thread to shutdown properly
            m_newDataToWrite.wait(&m_writeBufMtx, 200);
            if(!m_running)
                return;

            size = m_writeBuffer.size();
        }

        //NOTE: data size is limited to 2 bytes less than the
        //usb packet size (64 bytes for interrupt) to make room
        //for the reportID and valid data length
        size = qMin(WRITE_SIZE-2, m_writeBuffer.size());
        memcpy(&buffer[2], m_writeBuffer.constData(), size);
        buffer[1] = size; //valid data length
        buffer[0] = 2;    //reportID
        m_writeBufMtx.unlock();

        // must hold lock through the send to know how much was sent
        int ret = hiddev->send(hidno, buffer, WRITE_SIZE, WRITE_TIMEOUT);

        if(ret > 0)
        {
            //only remove the size actually written to the device            
            QMutexLocker lock(&m_writeBufMtx);
            m_writeBuffer.remove(0, size);

            emit m_hid->bytesWritten(ret - 2);
        }
        else if(ret < 0) // < 0 => error
        {
            //TODO! make proper error handling, this only quick hack for unplug freeze
            m_running=false;
            qDebug() << "Error writing to device";
        }
        else
        {
            qDebug() << "No data written to device ??";
        }
    }
}

int RawHIDWriteThread::pushDataToWrite(const char *data, int size)
{
    QMutexLocker lock(&m_writeBufMtx);

    m_writeBuffer.append(data, size);
    m_newDataToWrite.wakeOne(); //signal that new data arrived

    return size;
}

qint64 RawHIDWriteThread::getBytesToWrite()
{
    // QMutexLocker lock(&m_writeBufMtx);
    return m_writeBuffer.size();
}

// *********************************************************************************

RawHID::RawHID(const QString &deviceName)
    :QIODevice(),
    serialNumber(deviceName),
    m_deviceNo(-1),
    m_readThread(NULL),
	m_writeThread(NULL),
	m_mutex(NULL)
{

    m_mutex = new QMutex(QMutex::Recursive);
    m_startedMutex = new QMutex();

    // detect if the USB device is unplugged
    QObject::connect(&dev, SIGNAL(deviceUnplugged(int)), this, SLOT(onDeviceUnplugged(int)));

    m_writeThread = new RawHIDWriteThread(this);

    // Starting the read thread will lock the m_startexMutex until the
    // device is opened (which happens in that thread).
    m_readThread = new RawHIDReadThread(this);
    m_readThread->start();

    m_startedMutex->lock();
}

/**
 * @brief RawHID::openDevice This method opens the USB connection
 * It is uses as a callback from the read thread so that the USB
 * system code is registered in that thread instead of the calling
 * thread (usually UI)
 */
bool RawHID::openDevice() {
    int opened = dev.open(USB_MAX_DEVICES, USBMonitor::idVendor_OpenPilot, -1, USB_USAGE_PAGE, USB_USAGE);
    for (int i =0; i< opened; i++) {
        if (serialNumber == dev.getserial(i))
            m_deviceNo = i;
        else
            dev.close(i);
    }

    // Now things are opened or not (from read thread) allow the constructor to complete
    m_startedMutex->unlock();

    //didn't find the device we are trying to open (shouldnt happen)
    device_open = opened >= 0;
    if (opened < 0)
    {
        return false;
    }

    m_writeThread->start();

    return true;
}

/**
 * @brief RawHID::closeDevice This method closes the USB connection
 * It is uses as a callback from the read thread so that the USB
 * system code is unregistered from that thread\
 */
bool RawHID::closeDevice() {
    dev.close(m_deviceNo);
}

RawHID::~RawHID()
{
    // If the read thread exists then the device is open
    if (m_readThread)
        close();
}

void RawHID::onDeviceUnplugged(int num)
{
	if (num != m_deviceNo)
		return;

	// the USB device has been unplugged
	close();
}

bool RawHID::open(OpenMode mode)
{
	QMutexLocker locker(m_mutex);

	if (m_deviceNo < 0)
        return false;

    QIODevice::open(mode);

    Q_ASSERT(m_readThread);
    Q_ASSERT(m_writeThread);
    if (m_readThread) m_readThread->start();
    if (m_writeThread) m_writeThread->start();

    return true;
}

void RawHID::close()
{
    qDebug() << "RawHID::close()";
    emit aboutToClose();
    if (m_writeThread)
    {
        qDebug() << "About to terminate write thread";
        m_writeThread->terminate();
        delete m_writeThread;
        m_writeThread = NULL;
        qDebug() << "Write thread terminated";
    }


    if (m_readThread)
    {
        qDebug() << "About to terminate read thread";
        m_readThread->terminate();
        delete m_readThread; // calls wait
        m_readThread = NULL;
        qDebug() << "Read thread terminated";
    }

    emit closed();

    QIODevice::close();
}

bool RawHID::isSequential() const
{
    return true;
}

qint64 RawHID::bytesAvailable() const
{
	QMutexLocker locker(m_mutex);

	if (!m_readThread)
		return -1;

	return m_readThread->getBytesAvailable() + QIODevice::bytesAvailable();
}

qint64 RawHID::bytesToWrite() const
{
	QMutexLocker locker(m_mutex);

	if (!m_writeThread)
		return -1;

    return m_writeThread->getBytesToWrite() + QIODevice::bytesToWrite();
}

qint64 RawHID::readData(char *data, qint64 maxSize)
{
	QMutexLocker locker(m_mutex);

	if (!m_readThread || !data)
		return -1;

	return m_readThread->getReadData(data, maxSize);
}

qint64 RawHID::writeData(const char *data, qint64 maxSize)
{
	QMutexLocker locker(m_mutex);

	if (!m_writeThread || !data)
		return -1;

	return m_writeThread->pushDataToWrite(data, maxSize);
}

// *********************************************************************************

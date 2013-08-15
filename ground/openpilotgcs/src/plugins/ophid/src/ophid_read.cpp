#/**
 ******************************************************************************
 *
 * @file       ophid_read.cpp
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

#include "ophid_read.h"

class IConnection;


#define OPHID_USB_INT_DEVICE_READ_TIMEOUT 200
#define OPHID_USB_INT_BUFFER_SIZE         1024
#define OPHID_USB_INT_HEADER_SIZE         2

/**
 * @brief Constructor
 */
opHIDReadWorker::opHIDReadWorker(opHID *hid)
    : m_hid(hid),
    m_terminate(false)
{
    OPHID_TRACE("IN");

    OPHID_TRACE("OUT");
}


/**
 * @brief Destructor
 */
opHIDReadWorker::~opHIDReadWorker()
{
    OPHID_TRACE("IN");

    OPHID_TRACE("OUT");
}


/**
 * @brief Do what we can to make sure the thread will leave it's loop.
 */
void opHIDReadWorker::stop()
{
    OPHID_TRACE("IN");

    m_terminate = true;

    m_readBufMtx.unlock();

    m_leaveSigMtx.lock();

    OPHID_DEBUG("Stopping the thread.");

    emit finished();

    OPHID_TRACE("OUT");
}


/**
 * @brief Thread's loop to read to USB device.
 */
void opHIDReadWorker::process()
{
    int ret;
    int size;
    char buffer[OPHID_USB_INT_BUFFER_SIZE] = { 0 };

    OPHID_TRACE("IN");

    // Do not delete the device under our feet please.
    m_leaveSigMtx.lock();

    while (1) {
        // Quiting the thread properly.
        if (m_terminate) {
            OPHID_DEBUG("Ready to leave.");
            m_leaveSigMtx.unlock();
            break;
        }

        // Request MAX bytes since mecanism to know is not implemented yet.
        ret = m_hid->deviceInstanceGet()->receive(0,
                                                  buffer,
                                                  OPHID_USB_INT_BUFFER_SIZE,
                                                  OPHID_USB_INT_DEVICE_READ_TIMEOUT);

        // Append received frame in fifo if applicable.
        if (ret > 0) {
            size = buffer[1] + OPHID_USB_INT_HEADER_SIZE;
            m_readBufMtx.lock();
            m_readBuffer.append(buffer, size);
            m_readBufMtx.unlock();
            emit m_hid->readyRead();
        } else if (ret == 0) {
            OPHID_DEBUG("HID receive nothing.");
        } else {
            OPHID_ERROR("HID receive failed (%d).", ret);
        }
    }

    emit finished();

    OPHID_TRACE("OUT");
}


/**
 * @brief Get data received from USB device.
 */
int opHIDReadWorker::getReadData(char *data, int size)
{
    QMutexLocker lock(&m_readBufMtx);
    static int log_size_max = 0;

    int received_data_size = m_readBuffer.constData()[1];
    int current_size = qMin(size, received_data_size);

    if (received_data_size > log_size_max) {
        log_size_max = received_data_size;
        OPHID_DEBUG("The biggest packet received from the device is now %d Bytes.", log_size_max);
    }

    if (received_data_size > size) {
        OPHID_DEBUG("Wrong data size: requested %d but received %d from device (min: %d)", size, received_data_size, current_size);
    }

    memcpy(data, &m_readBuffer.constData()[2], current_size);
    m_readBuffer.remove(0, received_data_size + OPHID_USB_INT_HEADER_SIZE);

    return current_size;
}


/**
 * @brief Number of bytes pending already read from USB device.
 */
qint64 opHIDReadWorker::getBytesAvailable()
{
    QMutexLocker lock(&m_readBufMtx);

    return m_readBuffer.size();
}

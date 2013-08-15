/**
 ******************************************************************************
 *
 * @file       ophid_write.cpp
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

#include "ophid_write.h"

class IConnection;

#define OPHID_USB_INT_DEVICE_WRITE_TIMEOUT 1000
#define OPHID_USB_INT_BUFFER_SIZE          64
#define OPHID_USB_INT_HEADER_SIZE          2
#define OPHID_USB_INT_DATA_SIZE_MAX        (OPHID_USB_INT_BUFFER_SIZE - OPHID_USB_INT_HEADER_SIZE)


/**
 * @brief Constructor
 */
opHIDWriteWorker::opHIDWriteWorker(opHID *hid)
    : m_hid(hid),
    m_terminate(false)
{
    OPHID_TRACE("IN");

    OPHID_TRACE("OUT");
}


/**
 * @brief Destructor
 */
opHIDWriteWorker::~opHIDWriteWorker()
{
    OPHID_TRACE("IN");

    OPHID_TRACE("OUT");
}


/**
 * @brief Do what we can to make sure the thread will leave it's loop.
 */
void opHIDWriteWorker::stop()
{
    OPHID_TRACE("IN");

    m_terminate = true;

    m_writeBufMtx.unlock();

    m_msg_sem.release();

    m_leaveSigMtx.lock();

    OPHID_DEBUG("Stopping the thread.");

    emit finished();

    OPHID_TRACE("OUT");
}


/**
 * @brief Thread's loop to write to USB device.
 */
void opHIDWriteWorker::process()
{
    int size;
    int ret;

    OPHID_TRACE("IN");

    // Do not delete the device under our feet please.
    m_leaveSigMtx.lock();

    while (1) {
        // Wait until there is something to send.
        m_msg_sem.acquire();

        // Quiting the thread properly.
        if (m_terminate) {
            OPHID_DEBUG("Ready to leave");
            m_leaveSigMtx.unlock();
            break;
        }

        // Get the size of the buffer to send.
        size = *(m_writeBuffer.constData() + 1) + OPHID_USB_INT_HEADER_SIZE;

        // Send buffer to the device.
        ret  = m_hid->deviceInstanceGet()->send(0,
                                                (void *)m_writeBuffer.constData(),
                                                size,
                                                OPHID_USB_INT_DEVICE_WRITE_TIMEOUT);

        if (ret == size) {
            m_writeBufMtx.lock();
            m_writeBuffer.remove(0, size);
            m_writeBufMtx.unlock();
            // OPHID_ERROR("Devices sent %d from %d", ret, size);
            // emit m_hid->bytesWritten(ret - 2);
            emit bytesSent(ret - 2);
        } else if (ret < 0) {
            OPHID_ERROR("Could not write to device (%d) [Unplugged|Disconnected|Error].", ret);
            m_writeBufMtx.lock();
            m_writeBuffer.clear();
            m_writeBufMtx.unlock();
        } else {
            OPHID_ERROR("Sent %d but only %d went to the device.", ret, size);
        }
    }

    emit finished();

    OPHID_TRACE("OUT");
}


/**
 * @brief Add data to be sent to USB device.
 */
int opHIDWriteWorker::pushDataToWrite(const char *data, int size)
{
    QMutexLocker lock(&m_writeBufMtx);
    const char usb_report_id = 2;
    char usb_report_data_size = 0;
    int remaining_data_to_push = 0;
    static int log_size_max  = 0;
    int offset = 0;
 
    // Log max packet size 
    if (size > log_size_max) {
       log_size_max = size;
       OPHID_DEBUG("The biggest packet sent to the device is now %d Bytes.", log_size_max);
    }

    // Add buffer to be sent.
    remaining_data_to_push = size;
    while (remaining_data_to_push)
    {
        usb_report_data_size = qMin(OPHID_USB_INT_DATA_SIZE_MAX, remaining_data_to_push);
        m_writeBuffer.append(&usb_report_id, 1);
        m_writeBuffer.append(&usb_report_data_size, 1);
        m_writeBuffer.append(data + offset, usb_report_data_size);
        offset += usb_report_data_size;
        remaining_data_to_push = remaining_data_to_push - usb_report_data_size;
        m_msg_sem.release();
    }
    return size;
}


/**
 * @brief Number of bytes pending to be sent to USB device.
 */
qint64 opHIDWriteWorker::getBytesToWrite()
{
    QMutexLocker lock(&m_writeBufMtx);

    return m_writeBuffer.size();
}

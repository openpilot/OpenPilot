/**
 ******************************************************************************
 *
 * @file       opHID_usbsignal.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2013.
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup opHIDPlugin OpenPilot HID Plugin
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

#include "opHID_usbsignal.h"
#include <QDebug>


/**
 * \brief trigger device discovered signal
 *
 * \note
 *
 * \param[in] port.
 */
void USBSignalFilter::m_deviceDiscovered(USBPortInfo port)
{
    if ((port.vendorID == m_vid || m_vid == -1) &&
        (port.productID == m_pid || m_pid == -1) &&
        ((port.bcdDevice >> 8) == m_boardModel || m_boardModel == -1) &&
        ((port.bcdDevice & 0x00ff) == m_runState || m_runState == -1)) {
        qDebug() << "USBSignalFilter emit device discovered";
        emit deviceDiscovered();
    }
}


/**
 * \brief Constructor
 *
 * \note
 *
 * \param[in] vid USB vendor id of the device to open (-1 for any).
 * \param[in] pid USB product id of the device to open (-1 for any).
 * \param[in] boardModel.
 * \param[in] runState.
 */
USBSignalFilter::USBSignalFilter(int vid, int pid, int boardModel, int runState) :
    m_vid(vid),
    m_pid(pid),
    m_boardModel(boardModel),
    m_runState(runState)
{
    connect(USBMonitor::instance(),
            SIGNAL(deviceDiscovered(USBPortInfo)),
            this,
            SLOT(m_deviceDiscovered(USBPortInfo)),
            Qt::QueuedConnection);
}

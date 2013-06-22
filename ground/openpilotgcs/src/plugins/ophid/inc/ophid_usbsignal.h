/**
 ******************************************************************************
 *
 * @file       ophid_usbsignal.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2013.
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup opHIDPlugin Raw HID Plugin
 * @{
 * @brief Monitors the USB bus for devince insertion/removal
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

#ifndef OPHID_USBSIGNAL_H
#define OPHID_USBSIGNAL_H

#include <QObject>
#include "ophid_usbmon.h"

class OPHID_EXPORT USBSignalFilter : public QObject {
    Q_OBJECT

private:
    int m_vid;
    int m_pid;
    int m_boardModel;
    int m_runState;

signals:
    void deviceDiscovered();

private slots:
    void m_deviceDiscovered(USBPortInfo port);

public:
    USBSignalFilter(int vid, int pid, int boardModel, int runState);
};

#endif // OPHID_USBSIGNAL_H

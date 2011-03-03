/**
 ******************************************************************************
 *
 * @file       usbmonitor_win.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup RawHIDPlugin Raw HID Plugin
 * @{
 * @brief Implements the USB monitor on Windows using system API
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



#include "usbmonitor.h"
#include <QDebug>

#define printf qDebug

void USBMonitor::deviceEventReceived() {

    qDebug() << "Device event";
    // Dispatch and emit the signals here...

}


USBMonitor::USBMonitor(QObject *parent): QThread(parent) {

}

USBMonitor::~USBMonitor()
{
    quit();
}

/**
Returns a list of all currently available devices
*/
QList<USBPortInfo> USBMonitor::availableDevices()
{

}

/**
  Be a bit more picky and ask only for a specific type of device:
  */
QList<USBPortInfo> USBMonitor::availableDevices(int vid, int pid, int bcdDevice)
{
    QList<USBPortInfo> allPorts = availableDevices();
    QList<USBPortInfo> thePortsWeWant;

    foreach (USBPortInfo port, allPorts) {
        thePortsWeWant.append(port);
    }

    return thePortsWeWant;
}

/**
 ******************************************************************************
 *
 * @file       usbmonitor_linux.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup RawHIDPlugin Raw HID Plugin
 * @{
 * @brief Implements the USB monitor on Linux using libudev
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

/*
  Need help wihth udev ?
   There is a great tuturial at: http://www.signal11.us/oss/udev/
  */

#include "usbmonitor.h"
#include <QDebug>

#define printf qDebug

void USBMonitor::deviceEventReceived() {

    qDebug() << "Device event";
    struct udev_device *dev;

    dev = udev_monitor_receive_device(this->monitor);
    if (dev) {
        printf("Got Device");
        printf("   Node: %s", udev_device_get_devnode(dev));
        printf("   Subsystem: %s", udev_device_get_subsystem(dev));
        printf("   Devtype: %s", udev_device_get_devtype(dev));
        printf("   Action: %s", udev_device_get_action(dev));
        udev_device_unref(dev);
    }
    else {
        printf("No Device from receive_device(). An error occured.");
    }

}


USBMonitor::USBMonitor(QObject *parent): QThread(parent) {

    this->context = udev_new();

    this->monitor = udev_monitor_new_from_netlink(this->context, "udev");
//    udev_monitor_filter_add_match_subsystem_devtype(
//        this->monitor, "hidraw", NULL);
    udev_monitor_enable_receiving(this->monitor);
    this->monitorNotifier = new QSocketNotifier(
        udev_monitor_get_fd(this->monitor), QSocketNotifier::Read, this);
    connect(this->monitorNotifier, SIGNAL(activated(int)),
            this, SLOT(deviceEventReceived()));
    qDebug() << "Starting the Udev client";

    start(); // Start the thread event loop so that the socketnotifier works
}

USBMonitor::~USBMonitor()
{
    quit();
}

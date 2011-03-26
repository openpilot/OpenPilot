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


void printPortInfo(struct udev_device *dev)
{
    printf("   Node: %s", udev_device_get_devnode(dev));
    printf("   Subsystem: %s", udev_device_get_subsystem(dev));
    printf("   Devtype: %s", udev_device_get_devtype(dev));
    printf("   Action: %s", udev_device_get_action(dev));
    /* From here, we can call get_sysattr_value() for each file
    in the device's /sys entry. The strings passed into these
    functions (idProduct, idVendor, serial, etc.) correspond
    directly to the files in the directory which represents
    the USB device. Note that USB strings are Unicode, UCS2
    encoded, but the strings returned from
    udev_device_get_sysattr_value() are UTF-8 encoded. */
    printf("  VID/PID/bcdDevice : %s %s %s",
           udev_device_get_sysattr_value(dev,"idVendor"),
           udev_device_get_sysattr_value(dev, "idProduct"),
           udev_device_get_sysattr_value(dev,"bcdDevice"));
    printf("  %s   -  %s",
            udev_device_get_sysattr_value(dev,"manufacturer"),
            udev_device_get_sysattr_value(dev,"product"));
    printf("  serial: %s",
            udev_device_get_sysattr_value(dev, "serial"));

}


void USBMonitor::deviceEventReceived() {

    qDebug() << "Device event";
    struct udev_device *dev;

    dev = udev_monitor_receive_device(this->monitor);
    if (dev) {
        printf("------- Got Device Event");
        QString action = QString(udev_device_get_action(dev));
        QString devtype = QString(udev_device_get_devtype(dev));
        if (action == "add" && devtype == "usb_device") {
            printPortInfo(dev);
            emit deviceDiscovered(makePortInfo(dev));
        } else if (action == "remove" && devtype == "usb_device"){
            printPortInfo(dev);
            emit deviceRemoved(makePortInfo(dev));
        }

        udev_device_unref(dev);
    }
    else {
        printf("No Device from receive_device(). An error occured.");
    }

}


USBMonitor* USBMonitor::instance()
{
    return m_instance;
}

USBMonitor* USBMonitor::m_instance = 0;

/**
  Initialize the udev monitor here
  */
USBMonitor::USBMonitor(QObject *parent): QThread(parent) {

    m_instance = this;

    this->context = udev_new();

    this->monitor = udev_monitor_new_from_netlink(this->context, "udev");
    udev_monitor_filter_add_match_subsystem_devtype(
        this->monitor, "usb", NULL);
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

/**
Returns a list of all currently available devices
*/
QList<USBPortInfo> USBMonitor::availableDevices()
{
    QList<USBPortInfo> devicesList;
    struct udev_list_entry *devices, *dev_list_entry;
    struct udev_enumerate *enumerate;
    struct udev_device *dev;

    enumerate = udev_enumerate_new(this->context);
    udev_enumerate_add_match_subsystem(enumerate,"usb");
//    udev_enumerate_add_match_sysattr(enumerate, "idVendor", "20a0");
    udev_enumerate_scan_devices(enumerate);
    devices = udev_enumerate_get_list_entry(enumerate);
    // Will use the 'native' udev functions to loop:
    udev_list_entry_foreach(dev_list_entry,devices) {
        const char *path;

        /* Get the filename of the /sys entry for the device
        and create a udev_device object (dev) representing it */
        path = udev_list_entry_get_name(dev_list_entry);
        dev = udev_device_new_from_syspath(this->context, path);
        if (QString(udev_device_get_devtype(dev)) == "usb_device")
            devicesList.append(makePortInfo(dev));
        udev_device_unref(dev);
    }
    /* free the enumerator object */
    udev_enumerate_unref(enumerate);

    return devicesList;

}

/**
  Be a bit more picky and ask only for a specific type of device:
     On OpenPilot, the bcdDeviceLSB indicates the run state: bootloader or running.
     bcdDeviceMSB indicates the board model.
  */
QList<USBPortInfo> USBMonitor::availableDevices(int vid, int pid, int bcdDeviceMSB, int bcdDeviceLSB)
{
    QList<USBPortInfo> allPorts = availableDevices();
    QList<USBPortInfo> thePortsWeWant;

    foreach (USBPortInfo port, allPorts) {
        if((port.vendorID==vid || vid==-1) && (port.productID==pid || pid==-1) && ((port.bcdDevice>>8)==bcdDeviceMSB || bcdDeviceMSB==-1) &&
                ( (port.bcdDevice&0x00ff) ==bcdDeviceLSB || bcdDeviceLSB==-1))
            thePortsWeWant.append(port);
    }
    return thePortsWeWant;
}




USBPortInfo USBMonitor::makePortInfo(struct udev_device *dev)
{
    USBPortInfo prtInfo;

    //////////
    //  Debug info
    //////////
#ifdef DEBUG
    printPortInfo(dev);
#endif


    bool ok;
    prtInfo.vendorID = QString(udev_device_get_sysattr_value(dev, "idVendor")).toInt(&ok, 16);
    prtInfo.productID = QString(udev_device_get_sysattr_value(dev, "idProduct")).toInt(&ok, 16);
    prtInfo.serialNumber = QString(udev_device_get_sysattr_value(dev, "serial"));
    prtInfo.manufacturer = QString(udev_device_get_sysattr_value(dev,"manufacturer"));
    prtInfo.product = QString(udev_device_get_sysattr_value(dev,"product"));
//    prtInfo.UsagePage = QString(udev_device_get_sysattr_value(dev,""));
//    prtInfo.Usage = QString(udev_device_get_sysattr_value(dev,""));
    prtInfo.bcdDevice = QString(udev_device_get_sysattr_value(dev,"bcdDevice")).toInt(&ok, 16);





    return prtInfo;

}


/**
 * This snippet monitors devices using libudev.
 */

#include "usbmonitor.h"
#include <QDebug>
#include <QTimer>



void USBMonitor::deviceEventReceived() {

    qDebug() << "Device event";
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
/*
    connect(&pollingTimer, SIGNAL(timeout()), this, SLOT(udevDataAvailable()));
    pollingTimer.start(100);
    */
    start();
}

USBMonitor::~USBMonitor()
{

}

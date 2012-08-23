/**
 ******************************************************************************
 *
 * @file       usbmonitor_mac.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup RawHIDPlugin Raw HID Plugin
 * @{
 * @brief Implements the USB monitor on Mac using XXXXX
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
#include <IOKit/IOKitLib.h>
#include <IOKit/hid/IOHIDLib.h>
#include <CoreFoundation/CFString.h>
#include <CoreFoundation/CFArray.h>
#include <QMutexLocker>
#include <QDebug>

#define printf qDebug

//! Local helper functions
static bool HID_GetIntProperty(IOHIDDeviceRef dev, CFStringRef property, int * value);
static bool HID_GetStrProperty(IOHIDDeviceRef dev, CFStringRef property, QString & value);

/**
  Initialize the USB monitor here
  */
USBMonitor::USBMonitor(QObject *parent): QThread(parent) {
    hid_manager=NULL;
    IOReturn ret;

    m_instance = this;

    listMutex = new QMutex();
    knowndevices.clear();

    hid_manager = IOHIDManagerCreate(kCFAllocatorDefault, kIOHIDOptionsTypeNone);
    if (hid_manager == NULL || CFGetTypeID(hid_manager) != IOHIDManagerGetTypeID()) {
        if (hid_manager) CFRelease(hid_manager);
        Q_ASSERT(0);
    }

    // No matching filter
    IOHIDManagerSetDeviceMatching(hid_manager, NULL);

    // set up a callbacks for device attach & detach
    IOHIDManagerScheduleWithRunLoop(hid_manager, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);
    IOHIDManagerRegisterDeviceMatchingCallback(hid_manager, attach_callback, NULL);
    IOHIDManagerRegisterDeviceRemovalCallback(hid_manager, detach_callback, NULL);
    ret = IOHIDManagerOpen(hid_manager, kIOHIDOptionsTypeNone);
    if (ret != kIOReturnSuccess) {
        IOHIDManagerUnscheduleFromRunLoop(hid_manager,
                                          CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);
        CFRelease(hid_manager);
        return;
    }

    start();
}

USBMonitor::~USBMonitor()
{
    //if(hid_manager != NULL)
    //    IOHIDManagerUnscheduleFromRunLoop(hid_manager, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);
    quit();
}

void USBMonitor::deviceEventReceived() {

    qDebug() << "Device event";
}

USBMonitor* USBMonitor::instance()
{
    return m_instance;
}

USBMonitor* USBMonitor::m_instance = 0;


void USBMonitor::removeDevice(IOHIDDeviceRef dev) {
    for( int i = 0; i < knowndevices.length(); i++) {
        USBPortInfo port = knowndevices.at(i);
        if(port.dev_handle == dev) {
            QMutexLocker locker(listMutex);
            knowndevices.removeAt(i);
            emit deviceRemoved(port);
            return;
        }
    }


}

/**
  * @brief Static callback for the USB driver to indicate device removed
  */
void USBMonitor::detach_callback(void *context, IOReturn r, void *hid_mgr, IOHIDDeviceRef dev)
{
    Q_UNUSED(context);
    Q_UNUSED(r);
    Q_UNUSED(hid_mgr);

    qDebug() << "USBMonitor: Device detached event";
    instance()->removeDevice(dev);
}

void USBMonitor::addDevice(USBPortInfo info) {
    QMutexLocker locker(listMutex);
    knowndevices.append(info);
    emit deviceDiscovered(info);
}

void USBMonitor::attach_callback(void *context, IOReturn r, void *hid_mgr, IOHIDDeviceRef dev)
{
    Q_UNUSED(context);
    Q_UNUSED(r);
    Q_UNUSED(hid_mgr);

    bool got_properties = true;

    USBPortInfo deviceInfo;

    deviceInfo.dev_handle = dev;

    qDebug() << "USBMonitor: Device attached event";

    // Populate the device info structure
    got_properties &= HID_GetIntProperty(dev, CFSTR( kIOHIDVendorIDKey ), &deviceInfo.vendorID);
    got_properties &= HID_GetIntProperty(dev, CFSTR( kIOHIDProductIDKey ), &deviceInfo.productID);
    got_properties &= HID_GetIntProperty(dev, CFSTR( kIOHIDVersionNumberKey ), &deviceInfo.bcdDevice);
    got_properties &= HID_GetStrProperty(dev, CFSTR( kIOHIDSerialNumberKey ), deviceInfo.serialNumber);
    got_properties &= HID_GetStrProperty(dev, CFSTR( kIOHIDProductKey ), deviceInfo.product);
    got_properties &= HID_GetStrProperty(dev, CFSTR( kIOHIDManufacturerKey ), deviceInfo.manufacturer);
    // TOOD: Eventually want to take array of usages if devices start needing that
    got_properties &= HID_GetIntProperty(dev, CFSTR( kIOHIDPrimaryUsageKey ), &deviceInfo.Usage);
    got_properties &= HID_GetIntProperty(dev, CFSTR( kIOHIDPrimaryUsagePageKey ), &deviceInfo.UsagePage);

    // Currently only enumerating objects that have the complete list of properties
    if(got_properties) {
        qDebug() << "USBMonitor: Adding device";
        instance()->addDevice(deviceInfo);
    }
}

/**
Returns a list of all currently available devices
*/
QList<USBPortInfo> USBMonitor::availableDevices()
{
    //QMutexLocker locker(listMutex);
    return knowndevices;
}

/**
  * @brief Be a bit more picky and ask only for a specific type of device:
  * @param[in] vid VID to screen or -1 to ignore
  * @param[in] pid PID to screen or -1 to ignore
  * @param[in] bcdDeviceMSB MSB of bcdDevice to screen or -1 to ignore
  * @param[in] bcdDeviceLSB LSB of bcdDevice to screen or -1 to ignore
  * @return List of USBPortInfo that meet this criterion
  * @note
  *   On OpenPilot, the bcdDeviceLSB indicates the run state: bootloader or running.
  *   bcdDeviceMSB indicates the board model.
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

/**
  * @brief Helper function get get a HID integer property
  * @param[in] dev Device reference
  * @param[in] property The property to get (constants defined in IOKIT)
  * @param[out] value Pointer to integer to set
  * @return True if successful, false otherwise
  */
static bool HID_GetIntProperty(IOHIDDeviceRef dev, CFStringRef property, int * value) {
    CFTypeRef prop = IOHIDDeviceGetProperty(dev, property);
    if (prop) {
        if (CFNumberGetTypeID() == CFGetTypeID(prop)) { // if a number
            CFNumberGetValue((CFNumberRef) prop, kCFNumberSInt32Type, value);
            return true;
        }
    }
    return false;
}

/**
  * @brief Helper function get get a HID string property
  * @param[in] dev Device reference
  * @param[in] property The property to get (constants defined in IOKIT)
  * @param[out] value The QString to set
  * @return True if successful, false otherwise
  */
static bool HID_GetStrProperty(IOHIDDeviceRef dev, CFStringRef property, QString & value) {
    CFTypeRef prop = IOHIDDeviceGetProperty(dev, property);
    if (prop) {
        if (CFStringGetTypeID() == CFGetTypeID(prop)) { // if a string
            char buffer[2550];
            bool success = CFStringGetCString ( (CFStringRef) prop, buffer, sizeof(buffer), kCFStringEncodingMacRoman );
            value = QString(buffer);
            return success;
        }
    }
    return false;
}

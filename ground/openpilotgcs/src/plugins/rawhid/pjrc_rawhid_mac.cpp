/* @file pjrc_rawhid_mac.cpp
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup RawHIDPlugin Raw HID Plugin
 * @{
 * @brief Impliments a HID USB connection to the flight hardware as a QIODevice
 *****************************************************************************/

/* Simple Raw HID functions for Linux - for use with Teensy RawHID example
 * http://www.pjrc.com/teensy/rawhid.html
 * Copyright (c) 2009 PJRC.COM, LLC
 *
 *  rawhid_open - open 1 or more devices
 *  rawhid_recv - receive a packet
 *  rawhid_send - send a packet
 *  rawhid_close - close a device
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above description, website URL and copyright notice and this permission
 * notice shall be included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 * Version 1.0: Initial Release
 */


#include "pjrc_rawhid.h"

#include <unistd.h>
#include <QString>
#include <QThread>
#include <QTimer>
#include <QCoreApplication>

struct timeout_info {
    CFRunLoopRef loopRef;
    bool timed_out;
};

pjrc_rawhid::pjrc_rawhid() :
    device_open(false), hid_manager(NULL), buffer_count(0), unplugged(false)
{
    m_writeMutex = new QMutex();
    m_readMutex = new QMutex();
}

pjrc_rawhid::~pjrc_rawhid()
{
    if (device_open) {
        close(0);
    }

    if (m_writeMutex) {
        delete m_writeMutex;
        m_writeMutex = NULL;
    }

    if (m_readMutex) {
        delete m_readMutex;
        m_readMutex = NULL;
    }
}

/**
  * @brief open - open 1 or more devices
  * @param[in] max maximum number of devices to open
  * @param[in] vid Vendor ID, or -1 if any
  * @param[in] pid Product ID, or -1 if any
  * @param[in] usage_page top level usage page, or -1 if any
  * @param[in] usage top level usage number, or -1 if any
  * @returns actual number of devices opened
  */
int pjrc_rawhid::open(int max, int vid, int pid, int usage_page, int usage)
{
    CFMutableDictionaryRef dict;
    CFNumberRef num;
    IOReturn ret;

    Q_ASSERT(hid_manager == NULL);
    Q_ASSERT(device_open == false);

    attach_count = 0;

    // Start the HID Manager
    hid_manager = IOHIDManagerCreate(kCFAllocatorDefault, kIOHIDOptionsTypeNone);
    if (hid_manager == NULL || CFGetTypeID(hid_manager) != IOHIDManagerGetTypeID()) {
        if (hid_manager) CFRelease(hid_manager);
        return 0;
    }

    if (vid > 0 || pid > 0 || usage_page > 0 || usage > 0) {
        // Tell the HID Manager what type of devices we want
        dict = CFDictionaryCreateMutable(kCFAllocatorDefault, 0,
                                         &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
        if (!dict) return 0;
        if (vid > 0) {
            num = CFNumberCreate(kCFAllocatorDefault, kCFNumberIntType, &vid);
            CFDictionarySetValue(dict, CFSTR(kIOHIDVendorIDKey), num);
            CFRelease(num);
        }
        if (pid > 0) {
            num = CFNumberCreate(kCFAllocatorDefault, kCFNumberIntType, &pid);
            CFDictionarySetValue(dict, CFSTR(kIOHIDProductIDKey), num);
            CFRelease(num);
        }
        if (usage_page > 0) {
            num = CFNumberCreate(kCFAllocatorDefault, kCFNumberIntType, &usage_page);
            CFDictionarySetValue(dict, CFSTR(kIOHIDPrimaryUsagePageKey), num);
            CFRelease(num);
        }
        if (usage > 0) {
            num = CFNumberCreate(kCFAllocatorDefault, kCFNumberIntType, &usage);
            CFDictionarySetValue(dict, CFSTR(kIOHIDPrimaryUsageKey), num);
            CFRelease(num);
        }
        IOHIDManagerSetDeviceMatching(hid_manager, dict);
        CFRelease(dict);
    } else {
        IOHIDManagerSetDeviceMatching(hid_manager, NULL);
    }

    // Set the run loop reference before configuring the attach callback
    the_correct_runloop = CFRunLoopGetCurrent();

    // set up a callbacks for device attach & detach
    IOHIDManagerScheduleWithRunLoop(hid_manager, CFRunLoopGetCurrent(),
                                    kCFRunLoopDefaultMode);
    IOHIDManagerRegisterDeviceMatchingCallback(hid_manager, pjrc_rawhid::attach_callback, this);
    IOHIDManagerRegisterDeviceRemovalCallback(hid_manager, pjrc_rawhid::dettach_callback, this);
    ret = IOHIDManagerOpen(hid_manager, kIOHIDOptionsTypeNone);
    if (ret != kIOReturnSuccess) {
        IOHIDManagerUnscheduleFromRunLoop(hid_manager,
                                          CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);
        CFRelease(hid_manager);
        return 0;
    }

    // let it do the callback for all devices
    while (CFRunLoopRunInMode(kCFRunLoopDefaultMode, 0, true) == kCFRunLoopRunHandledSource) ;

    // count up how many were added by the callback
    return attach_count;
}

/**
 * @brief receive - receive a packet
 * @param[in] num device to receive from (unused now)
 * @param[in] buf buffer to receive packet
 * @param[in] len buffer's size
 * @param[in] timeout = time to wait, in milliseconds
 * @returns number of bytes received, or -1 on error
 */
int pjrc_rawhid::receive(int, void *buf, int len, int timeout)
{
    QMutexLocker locker(m_readMutex);
    Q_UNUSED(locker);

    if (!device_open)
        return -1;

    // Pass information to the callback to stop this run loop and signal if a timeout occurred
    struct timeout_info info;
    info.loopRef = CFRunLoopGetCurrent();;
    info.timed_out = false;
    CFRunLoopTimerContext context;
    memset(&context, 0, sizeof(context));
    context.info = &info;

    // Set up the timer for the timeout
    CFRunLoopTimerRef timer;
    timer = CFRunLoopTimerCreate(NULL, CFAbsoluteTimeGetCurrent() + (double)timeout / 1000.0, 0, 0, 0, timeout_callback, &context);
    CFRunLoopAddTimer(CFRunLoopGetCurrent(), timer, kCFRunLoopDefaultMode);

    received_runloop = CFRunLoopGetCurrent();

    // Run the CFRunLoop until either a timeout or data is available
    while(1) {
        if (buffer_count != 0) {
            if (len > buffer_count) len = buffer_count;
            memcpy(buf, buffer, len);
            buffer_count = 0;
            break;
        } else if (info.timed_out) {
            len = 0;
            break;
        }
        CFRunLoopRun(); // Wait for data
    }

    CFRunLoopTimerInvalidate(timer);
    CFRelease(timer);

    received_runloop = NULL;

    return len;
}

/**
 * @brief Helper class that will workaround the fact
 * that the HID send is broken on OSX
 */
class Sender : public QThread
{
public:
    Sender(IOHIDDeviceRef d, uint8_t * b, int l) :
        dev(d), buf(b), len(l), result(-1) { }

    void run() {
        ret = IOHIDDeviceSetReport(dev, kIOHIDReportTypeOutput, 2, buf, len);
        result = (ret == kIOReturnSuccess) ? len : -1;
    }

    int result;
    IOReturn ret;
private:
    IOHIDDeviceRef dev;
    uint8_t * buf;
    int len;
};

/**
 * @brief send - send a packet
 * @param[in] num device to transmit to (zero based)
 * @param[in] buf buffer containing packet to send
 * @param[in] len number of bytes to transmit
 * @param[in] timeout = time to wait, in milliseconds
 * @returns number of bytes sent, or -1 on error
 */
int pjrc_rawhid::send(int, void *buf, int len, int timeout)
{
    // This lock ensures that when closing we don't do it until the
    // write has terminated (and then the device_open flag is set to false)
    QMutexLocker locker(m_writeMutex);
    Q_UNUSED(locker);

    if(!device_open || unplugged) {
        return -1;
    }

    uint8_t *report_buf = (uint8_t *) malloc(len);
    memcpy(&report_buf[0], buf,len);

    QEventLoop el;
    Sender sender(dev, report_buf, len);
    connect(&sender, SIGNAL(finished()), &el, SLOT(quit()));
    sender.start();
    QTimer::singleShot(timeout, &el, SLOT(quit()));
    el.exec();

    return sender.result;
}

//! Get the serial number for a HID device
QString pjrc_rawhid::getserial(int num) {
    QMutexLocker locker(m_readMutex);
    Q_UNUSED(locker);

    if (!device_open || unplugged)
        return "";

    CFTypeRef serialnum = IOHIDDeviceGetProperty(dev, CFSTR(kIOHIDSerialNumberKey));
    if(serialnum && CFGetTypeID(serialnum) == CFStringGetTypeID())
    {
        //Note: I'm not sure it will always succeed if encoded as MacRoman but that
        //is a superset of UTF8 so I think this is fine
        CFStringRef str = (CFStringRef)serialnum;
        const char * buf = CFStringGetCStringPtr(str, kCFStringEncodingMacRoman);
        return QString(buf);
    }

    return QString("Error");
}

//! Close the HID device
void pjrc_rawhid::close(int)
{
    // Make sure any pending locks are done
    QMutexLocker lock(m_writeMutex);

    if (device_open) {
        device_open = false;
        CFRunLoopStop(the_correct_runloop);

        if (!unplugged) {
            IOHIDDeviceUnscheduleFromRunLoop(dev, the_correct_runloop, kCFRunLoopDefaultMode);
            IOHIDDeviceRegisterInputReportCallback(dev, buffer, sizeof(buffer), NULL, NULL);
            IOHIDDeviceClose(dev, kIOHIDOptionsTypeNone);
        }

        IOHIDManagerRegisterDeviceRemovalCallback(hid_manager, NULL, NULL);
        IOHIDManagerClose(hid_manager, 0);

        dev = NULL;
        hid_manager = NULL;
    }
}

/**
 * @brief input Called to add input data to the buffer
 * @param[in] id Report id
 * @param[in] data The data buffer
 * @param[in] len The report length
 */
void pjrc_rawhid::input(uint8_t *data, CFIndex len)
{
    if (!device_open)
        return;

    if (len > BUFFER_SIZE) len = BUFFER_SIZE;
    // Note: packet preprocessing done in OS independent code
    memcpy(buffer, &data[0], len);
    buffer_count = len;

    if (received_runloop)
        CFRunLoopStop(received_runloop);
}

//! Callback for the HID driver on an input report
void pjrc_rawhid::input_callback(void *c, IOReturn ret, void *sender, IOHIDReportType type, uint32_t id, uint8_t *data, CFIndex len)
{
    if (ret != kIOReturnSuccess || len < 1) return;

    pjrc_rawhid *context = (pjrc_rawhid *) c;
    context->input(data, len);
}

//! Timeout used for the
void pjrc_rawhid::timeout_callback(CFRunLoopTimerRef, void *i)
{
    struct timeout_info *info = (struct timeout_info *) i;
    info->timed_out = true;
    CFRunLoopStop(info->loopRef);
}

//! Called on a dettach event
void pjrc_rawhid::dettach(IOHIDDeviceRef d)
{
    unplugged = true;
    if (d == dev)
        emit deviceUnplugged(0);
}

//! Called from the USB system and forwarded to the instance (context)
void pjrc_rawhid::dettach_callback(void *context, IOReturn, void *, IOHIDDeviceRef dev)
{
    pjrc_rawhid *p = (pjrc_rawhid*) context;
    p->dettach(dev);
}

/**
 * @brief Called by the USB system
 * @param dev The device that was attached
 */
void pjrc_rawhid::attach(IOHIDDeviceRef d)
{
    // Store the device handle
    dev = d;

    if (IOHIDDeviceOpen(dev, kIOHIDOptionsTypeNone) != kIOReturnSuccess) return;
    // Disconnect the attach callback since we don't want to automatically reconnect
    IOHIDManagerRegisterDeviceMatchingCallback(hid_manager, NULL, NULL);
    IOHIDDeviceScheduleWithRunLoop(dev, the_correct_runloop, kCFRunLoopDefaultMode);
    IOHIDDeviceRegisterInputReportCallback(dev, buffer, sizeof(buffer), pjrc_rawhid::input_callback, this);

    attach_count++;
    device_open = true;
    unplugged = false;
}

//! Called from the USB system and forwarded to the instance (context)
void pjrc_rawhid::attach_callback(void *context, IOReturn r, void *hid_mgr, IOHIDDeviceRef dev)
{
    pjrc_rawhid *p = (pjrc_rawhid*) context;
    p->attach(dev);
}


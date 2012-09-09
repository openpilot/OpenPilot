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
#include <IOKit/IOKitLib.h>
#include <IOKit/hid/IOHIDLib.h>
#include <CoreFoundation/CFString.h>
#include <QString>
#include <QThread>
#include <QTimer>
#include <QCoreApplication>

class delay : public QThread
{
public:
    static void msleep(unsigned long msecs)
    {
        QThread::msleep(msecs);
    }
};

#define BUFFER_SIZE 64

//#define printf qDebug
#define printf

typedef struct hid_struct hid_t;
typedef struct buffer_struct buffer_t;
static hid_t *first_hid = NULL;
static hid_t *last_hid = NULL;
// Make sure we use the correct runloop
CFRunLoopRef the_correct_runloop = NULL;
struct hid_struct {
    IOHIDDeviceRef ref;
    int open;
    uint8_t buffer[BUFFER_SIZE];
    buffer_t *first_buffer;
    buffer_t *last_buffer;
    struct hid_struct *prev;
    struct hid_struct *next;
};
struct buffer_struct {
    struct buffer_struct *next;
    uint32_t len;
    uint8_t buf[BUFFER_SIZE];
};

static void add_hid(hid_t *);
static hid_t * get_hid(int);
static void free_all_hid(void);
static void hid_close(hid_t *);
static void attach_callback(void *, IOReturn, void *, IOHIDDeviceRef);
static void detach_callback(void *, IOReturn, void *hid_mgr, IOHIDDeviceRef dev);
static void input_callback(void *, IOReturn, void *, IOHIDReportType, uint32_t, uint8_t *, CFIndex);
static void output_callback(hid_t *context, IOReturn ret, void *sender, IOHIDReportType type, uint32_t id, uint8_t *data, CFIndex len);
static void timeout_callback(CFRunLoopTimerRef, void *);


pjrc_rawhid::pjrc_rawhid()
{
    first_hid = NULL;
    last_hid = NULL;
}

pjrc_rawhid::~pjrc_rawhid()
{
}

//  open - open 1 or more devices
//
//    Inputs:
//	max = maximum number of devices to open
//	vid = Vendor ID, or -1 if any
//	pid = Product ID, or -1 if any
//	usage_page = top level usage page, or -1 if any
//	usage = top level usage number, or -1 if any
//    Output:
//	actual number of devices opened
//
int pjrc_rawhid::open(int max, int vid, int pid, int usage_page, int usage)
{
    static IOHIDManagerRef hid_manager=NULL;
    CFMutableDictionaryRef dict;
    CFNumberRef num;
    IOReturn ret;
    hid_t *p;
    int count=0;

    if (first_hid) free_all_hid();
    //printf("pjrc_rawhid_open, max=%d\n", max);
    if (max < 1) return 0;
    // Start the HID Manager
    // http://developer.apple.com/technotes/tn2007/tn2187.html
    if (!hid_manager) {
        hid_manager = IOHIDManagerCreate(kCFAllocatorDefault, kIOHIDOptionsTypeNone);
        if (hid_manager == NULL || CFGetTypeID(hid_manager) != IOHIDManagerGetTypeID()) {
            if (hid_manager) CFRelease(hid_manager);
            return 0;
        }
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
    IOHIDManagerRegisterDeviceMatchingCallback(hid_manager, attach_callback, NULL);
    IOHIDManagerRegisterDeviceRemovalCallback(hid_manager, detach_callback, NULL);
    ret = IOHIDManagerOpen(hid_manager, kIOHIDOptionsTypeNone);
    if (ret != kIOReturnSuccess) {
        printf("Could not start IOHIDManager");
        IOHIDManagerUnscheduleFromRunLoop(hid_manager,
                                          CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);
        CFRelease(hid_manager);
        return 0;
    }
    printf("run loop\n");
    // let it do the callback for all devices
    while (CFRunLoopRunInMode(kCFRunLoopDefaultMode, 0, true) == kCFRunLoopRunHandledSource) ;
    // count up how many were added by the callback
    for (p = first_hid; p; p = p->next) count++;
    return count;
}

//  recveive - receive a packet
//    Inputs:
//	num = device to receive from (zero based)
//	buf = buffer to receive packet
//	len = buffer's size
//	timeout = time to wait, in milliseconds
//    Output:
//	number of bytes received, or -1 on error
//
int pjrc_rawhid::receive(int num, void *buf, int len, int timeout)
{
   hid_t *hid;
   buffer_t *b;
   CFRunLoopTimerRef timer=NULL;
   CFRunLoopTimerContext context;
   int ret=0, timeout_occurred=0;

   if (len < 1) return 0;
   hid = get_hid(num);
   if (!hid || !hid->open) return -1;
   if ((b = hid->first_buffer) != NULL) {
       if (len > b->len) len = b->len;
       memcpy(buf, b->buf, len);
       hid->first_buffer = b->next;
       free(b);
       return len;
   }
   memset(&context, 0, sizeof(context));
   context.info = &timeout_occurred;
   timer = CFRunLoopTimerCreate(NULL, CFAbsoluteTimeGetCurrent() +
                                (double)timeout / 1000.0, 0, 0, 0, timeout_callback, &context);
   CFRunLoopAddTimer(CFRunLoopGetCurrent(), timer, kCFRunLoopDefaultMode);
   the_correct_runloop = CFRunLoopGetCurrent();
   //qDebug("--");
   while (1) {
       //qDebug(".");
       CFRunLoopRun(); // Found the problem: somehow the input_callback does not
                       // stop this CFRunLoopRun because it is hooked to a different run loop !!!
                       // Hence the use of the "correct_runloop" variable above.
       //qDebug("  ..");

       if ((b = hid->first_buffer) != NULL) {
           if (len > b->len) len = b->len;
           memcpy(buf, b->buf, len);
           hid->first_buffer = b->next;
           free(b);
           ret = len;
           //qDebug("*************");
           break;
       }
       if (!hid->open) {
           printf("pjrc_rawhid_recv, device not open\n");
           ret = -1;
           break;
       }
       if (timeout_occurred)
            break;
   }
   CFRunLoopTimerInvalidate(timer);
   CFRelease(timer);
   return ret;
}

//  send - send a packet
//    Inputs:
//	num = device to transmit to (zero based)
//	buf = buffer containing packet to send
//	len = number of bytes to transmit
//	timeout = time to wait, in milliseconds
//    Output:
//	number of bytes sent, or -1 on error
//
int pjrc_rawhid::send(int num, void *buf, int len, int timeout)
{
    hid_t *hid;
    int result=-100;

    hid = get_hid(num);
    if (!hid || !hid->open) return -1;
#if 1
#warning "Send timeout not implemented on MACOSX"
    uint8_t *report_buf = (uint8_t *) malloc(len);
    memcpy(&report_buf[0], buf,len);
    // Note: packet processing done in OS indepdent code
    IOReturn ret = IOHIDDeviceSetReport(hid->ref, kIOHIDReportTypeOutput, 2, (uint8_t *)report_buf, len);
    result = (ret == kIOReturnSuccess) ? len : -1;
    if (err_get_system(ret) == err_get_system(sys_iokit))
    {

        // The error was in the I/O Kit system
        UInt32 codeValue = err_get_code(ret);
        qDebug("Returned: %x", codeValue);
        // Can now perform test on error code, display it to user, or whatever.
        usleep(1000000);
    }

#endif
#if 0
    // No matter what I tried this never actually sends an output
    // report and output_callback never gets called.  Why??
    // Did I miss something?  This is exactly the same params as
    // the sync call that works.  Is it an Apple bug?
    // (submitted to Apple on 22-sep-2009, problem ID 7245050)
    //
    IOHIDDeviceScheduleWithRunLoop(hid->ref, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);
    // should already be scheduled with run loop by attach_callback,
    // sadly this doesn't make any difference either way
    //
    IOHIDDeviceSetReportWithCallback(hid->ref, kIOHIDReportTypeOutput,
                                     0, buf, len, (double)timeout / 1000.0, output_callback, &result);
    while (1) {
        printf("enter run loop (send)\n");
        CFRunLoopRun();
        printf("leave run loop (send)\n");
        if (result > -100) break;
        if (!hid->open) {
            result = -1;
            break;
        }
    }
#endif
    return result;
}

QString pjrc_rawhid::getserial(int num) {
    hid_t *hid;
    char buf[128];

    hid = get_hid(num);

    if (!hid || !hid->open) return QString("Error");

    CFTypeRef serialnum = IOHIDDeviceGetProperty(hid->ref, CFSTR(kIOHIDSerialNumberKey));
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

//  close - close a device
//
//    Inputs:
//	num = device to close (zero based)
//    Output
//	(nothing)
//
void pjrc_rawhid::close(int num)
{
    hid_t *hid;

    hid = get_hid(num);
    if (!hid || !hid->open) return;
    hid_close(hid);
    hid->open = 0;
}

//
//
// Private Functions
//
//
static void input_callback(void *context, IOReturn ret, void *sender, IOHIDReportType type, uint32_t id, uint8_t *data, CFIndex len)
{
    buffer_t *n;
    hid_t *hid;

    //qDebug("input_callback, ret: %i - report id: %i buf: %x %x, len: %d\n", ret, id, data[0], data[1], len);
    if (ret != kIOReturnSuccess || len < 1) return;
    hid = (hid_t*)context;
    if (!hid || hid->ref != sender) return;
    printf("Processing packet");
    n = (buffer_t *)malloc(sizeof(buffer_t));
    if (!n) return;
    if (len > BUFFER_SIZE) len = BUFFER_SIZE;
    // Note: packet preprocessing done in OS independent code
    memcpy(n->buf, &data[0], len);
    n->len = len;
    n->next = NULL;
    if (!hid->first_buffer || !hid->last_buffer) {
            hid->first_buffer = hid->last_buffer = n;
    } else {
            hid->last_buffer->next = n;
            hid->last_buffer = n;
    }

    Q_ASSERT(the_correct_runloop != NULL);
    CFRunLoopStop(the_correct_runloop);
}

static void timeout_callback(CFRunLoopTimerRef timer, void *info)
{
   //qDebug("timeout_callback\n");
   *(int *)info = 1;
   //qDebug() << "Stop CFRunLoop from timeout_callback" << CFRunLoopGetCurrent();
   CFRunLoopStop(CFRunLoopGetCurrent());
}

static void add_hid(hid_t *h)
{
    if (!first_hid || !last_hid) {
            first_hid = last_hid = h;
            h->next = h->prev = NULL;
            return;
    }
    last_hid->next = h;
    h->prev = last_hid;
    h->next = NULL;
    last_hid = h;
}


static hid_t * get_hid(int num)
{
    hid_t *p;
    for (p = first_hid; p && num > 0; p = p->next, num--) ;
    return p;
}


static void free_all_hid(void)
{
    hid_t *p, *q;

    for (p = first_hid; p; p = p->next) {
            hid_close(p);
    }
    p = first_hid;
    while (p) {
            q = p;
            p = p->next;
            free(q);
    }
    first_hid = last_hid = NULL;
}


static void hid_close(hid_t *hid)
{
    if (!hid || !hid->open || !hid->ref) return;
    IOHIDManagerRef hid_manager = IOHIDManagerCreate(kCFAllocatorDefault, kIOHIDOptionsTypeNone);
    IOHIDManagerRegisterDeviceMatchingCallback(hid_manager, NULL, NULL);
    IOHIDManagerRegisterDeviceRemovalCallback(hid_manager, NULL, NULL);
    IOHIDDeviceUnscheduleFromRunLoop(hid->ref, CFRunLoopGetCurrent( ), kCFRunLoopDefaultMode);
    IOHIDDeviceClose(hid->ref, kIOHIDOptionsTypeNone);
    IOHIDManagerClose(hid_manager, 0);
    hid->ref = NULL;
}

static void detach_callback(void *context, IOReturn r, void *hid_mgr, IOHIDDeviceRef dev)
{
    hid_t *p;

    printf("detach callback\n");
    for (p = first_hid; p; p = p->next) {
            if (p->ref == dev) {
                    p->open = 0;
                    CFRunLoopStop(CFRunLoopGetCurrent());
                    return;
            }
    }
}

static void attach_callback(void *context, IOReturn r, void *hid_mgr, IOHIDDeviceRef dev)
{
    struct hid_struct *h;

    printf("attach callback\n");
    if (IOHIDDeviceOpen(dev, kIOHIDOptionsTypeNone) != kIOReturnSuccess) return;
    h = (hid_t *)malloc(sizeof(hid_t));
    if (!h) return;
    memset(h, 0, sizeof(hid_t));
    IOHIDDeviceScheduleWithRunLoop(dev, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);
    IOHIDDeviceRegisterInputReportCallback(dev, h->buffer, sizeof(h->buffer), input_callback, h);
    h->ref = dev;
    h->open = 1;
    add_hid(h);
}

static void output_callback(hid_t *context, IOReturn ret, void *sender, IOHIDReportType type, uint32_t id, uint8_t *data, CFIndex len)
{
        printf("output_callback, r=%d\n", ret);
        if (ret == kIOReturnSuccess) {
                *(int *)context = len;
        } else {
                // timeout if not success?
                *(int *)context = 0;
        }
        CFRunLoopStop(CFRunLoopGetCurrent());
}


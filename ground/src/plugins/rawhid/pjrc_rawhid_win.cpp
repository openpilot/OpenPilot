/* @file pjrc_rawhid_windows.cpp
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup RawHIDPlugin Raw HID Plugin
 * @{
 * @brief Impliments a HID USB connection to the flight hardware as a QIODevice
 *****************************************************************************/

/* Simple Raw HID functions for Windows - for use with Teensy RawHID example
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
/* See: http://msdn.microsoft.com/en-us/library/ms794141.aspx */

#include "pjrc_rawhid.h"

#include <windows.h>
#include <setupapi.h>
#include <ddk/hidsdi.h>
#include <ddk/hidclass.h>
#include <QString>

#define printf qDebug

typedef struct hid_struct hid_t;
struct hid_struct {
        HANDLE handle;
        int open;
        struct hid_struct *prev;
        struct hid_struct *next;
};
hid_t *first_hid;
hid_t *last_hid;
HANDLE rx_event;
HANDLE tx_event;
CRITICAL_SECTION rx_mutex;
CRITICAL_SECTION tx_mutex;

static void add_hid(hid_t *h);
static hid_t* get_hid(int num);
static void free_all_hid(void);
static void hid_close(hid_t *hid);
static void print_win32_err(void);

pjrc_rawhid::pjrc_rawhid()
{
    first_hid = NULL;
    last_hid = NULL;
    rx_event = NULL;
    tx_event = NULL;
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
    GUID guid;
    HDEVINFO info;
    DWORD index=0, reqd_size;
    SP_DEVICE_INTERFACE_DATA iface;
    SP_DEVICE_INTERFACE_DETAIL_DATA *details;
    HIDD_ATTRIBUTES attrib;
    PHIDP_PREPARSED_DATA hid_data;
    HIDP_CAPS capabilities;
    HANDLE h;
    BOOL ret;
    hid_t *hid;
    int count=0;

    if (first_hid) free_all_hid();
    if (max < 1) return 0;
    if (!rx_event) {
            rx_event = CreateEvent(NULL, TRUE, TRUE, NULL);
            tx_event = CreateEvent(NULL, TRUE, TRUE, NULL);
            InitializeCriticalSection(&rx_mutex);
            InitializeCriticalSection(&tx_mutex);
    }
    HidD_GetHidGuid(&guid);
    info = SetupDiGetClassDevs(&guid, NULL, NULL, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);
    if (info == INVALID_HANDLE_VALUE) return 0;
    for (index=0; 1 ;index++) {
            iface.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);
            ret = SetupDiEnumDeviceInterfaces(info, NULL, &guid, index, &iface);
            if (!ret) return count;
            SetupDiGetInterfaceDeviceDetail(info, &iface, NULL, 0, &reqd_size, NULL);
            details = (SP_DEVICE_INTERFACE_DETAIL_DATA *)malloc(reqd_size);
            if (details == NULL) continue;

            memset(details, 0, reqd_size);
            details->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);
            ret = SetupDiGetDeviceInterfaceDetail(info, &iface, details,
                    reqd_size, NULL, NULL);
            if (!ret) {
                    free(details);
                    continue;
            }
            h = CreateFile(details->DevicePath, GENERIC_READ|GENERIC_WRITE,
                    FILE_SHARE_READ|FILE_SHARE_WRITE, NULL,
                    OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);
            free(details);
            if (h == INVALID_HANDLE_VALUE) continue;
            attrib.Size = sizeof(HIDD_ATTRIBUTES);
            ret = HidD_GetAttributes(h, &attrib);
            //printf("vid: %4x\n", attrib.VendorID);
            if (!ret || (vid > 0 && attrib.VendorID != vid) ||
              (pid > 0 && attrib.ProductID != pid) ||
              !HidD_GetPreparsedData(h, &hid_data)) {
                    CloseHandle(h);
                    continue;
            }
            if (!HidP_GetCaps(hid_data, &capabilities) ||
              (usage_page > 0 && capabilities.UsagePage != usage_page) ||
              (usage > 0 && capabilities.Usage != usage)) {
                    HidD_FreePreparsedData(hid_data);
                    CloseHandle(h);
                    continue;
            }
            HidD_FreePreparsedData(hid_data);
            hid = (struct hid_struct *)malloc(sizeof(struct hid_struct));
            if (!hid) {
                    CloseHandle(h);
                    continue;
            }
            hid->handle = h;
            hid->open = 1;
            add_hid(hid);
            count++;
            if (count >= max) return count;
    }
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
        unsigned char tmpbuf[516]={0};
        OVERLAPPED ov;
        DWORD n, r;

        if (sizeof(tmpbuf) < len) return -1;
        hid = get_hid(num);
        if (!hid || !hid->open) return -1;
        EnterCriticalSection(&rx_mutex);
        ResetEvent(&rx_event);
        memset(&ov, 0, sizeof(ov));
        ov.hEvent = rx_event;
        if (!ReadFile(hid->handle, tmpbuf, len, NULL, &ov)) {
                if (GetLastError() != ERROR_IO_PENDING) goto return_error;
                r = WaitForSingleObject(rx_event, timeout);
                if (r == WAIT_TIMEOUT) goto return_timeout;
                if (r != WAIT_OBJECT_0) goto return_error;
        }
        if (!GetOverlappedResult(hid->handle, &ov, &n, FALSE)) goto return_error;
        LeaveCriticalSection(&rx_mutex);
        if (n <= 0) return -1;
        n--;
        if (n > len) n = len;
        memcpy(buf, tmpbuf, n);
        return n;
return_timeout:
        CancelIo(hid->handle);
        LeaveCriticalSection(&rx_mutex);
        return 0;
return_error:
        print_win32_err();
        LeaveCriticalSection(&rx_mutex);
        return -1;
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
        unsigned char tmpbuf[64]={0};
        OVERLAPPED ov;
        DWORD n, r;

        if (sizeof(tmpbuf) < len) return -1;
        hid = get_hid(num);
        if (!hid || !hid->open) return -1;
        EnterCriticalSection(&tx_mutex);
        ResetEvent(&tx_event);
        memset(&ov, 0, sizeof(ov));
        ov.hEvent = tx_event;
        memcpy(tmpbuf, buf, len);
        if (!WriteFile(hid->handle, tmpbuf, 64, NULL, &ov)) {
                if (GetLastError() != ERROR_IO_PENDING) goto return_error;
                r = WaitForSingleObject(tx_event, timeout);
                if (r == WAIT_TIMEOUT) goto return_timeout;
                if (r != WAIT_OBJECT_0) goto return_error;
        }
        if (!GetOverlappedResult(hid->handle, &ov, &n, FALSE)) goto return_error;
        LeaveCriticalSection(&tx_mutex);
        if (n <= 0) return -1;
        return n - 1;
return_timeout:
        CancelIo(hid->handle);
        LeaveCriticalSection(&tx_mutex);
        return 0;
return_error:
        print_win32_err();
        LeaveCriticalSection(&tx_mutex);
        return -1;
}

QString pjrc_rawhid::getserial(int num)
{
    hid_t *hid;
    char temp[126];

    hid = get_hid(num);
    if (!hid || !hid->open)
        return "";

    /* Should we do some "critical section" stuff here?? */
    if(!HidD_GetSerialNumberString(hid->handle, temp, sizeof(temp))) {
        print_win32_err();
        return QString("Error");
    }

    return QString().fromUtf16((ushort*)temp,-1);
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
}

//
//
// Private Functions
//
//
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
        CloseHandle(hid->handle);
        hid->handle = NULL;
}

static void print_win32_err(void)
{
        char buf[256];
        char temp[256];
        DWORD err;

        err = GetLastError();
        //FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, err, 0, (WCHAR*)buf, sizeof(buf), NULL);
        FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, err, MAKELANGID(LANG_ENGLISH, SUBLANG_DEFAULT), (WCHAR*)buf, sizeof(buf), NULL);
        WideCharToMultiByte( CP_ACP, 0, (WCHAR*)buf, sizeof(buf), temp, sizeof(temp), NULL, NULL );
        printf("err %ld: %s\n", err, temp);
}

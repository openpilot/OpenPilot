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
#include <QMetaType>
#include <QString>
#include <initguid.h>
#define printf qDebug

pjrc_rawhid::pjrc_rawhid()
{
    if( !QMetaType::isRegistered( QMetaType::type("USVPortInfo") ) )
        qRegisterMetaType<USBPortInfo>("USBPortInfo");
#if (defined QT_GUI_LIB)
    notificationWidget = 0;
#endif // Q_OS_WIN
    first_hid = NULL;
    last_hid = NULL;
    rx_event = NULL;
    tx_event = NULL;
}

pjrc_rawhid::~pjrc_rawhid()
{
#if (defined QT_GUI_LIB)
    if( notificationWidget )
        delete notificationWidget;
#endif
}

//  open - open 1 or more devices
//
//    Inputs:
//      max = maximum number of devices to open
//      vid = Vendor ID, or -1 if any
//      pid = Product ID, or -1 if any
//      usage_page = top level usage page, or -1 if any
//      usage = top level usage number, or -1 if any
//    Output:
//      actual number of devices opened
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

        if (!rx_event)
        {
                rx_event = CreateEvent(NULL, TRUE, TRUE, NULL);
                tx_event = CreateEvent(NULL, TRUE, TRUE, NULL);
                InitializeCriticalSection(&rx_mutex);
                InitializeCriticalSection(&tx_mutex);
    }

        HidD_GetHidGuid(&guid);

        info = SetupDiGetClassDevs(&guid, NULL, NULL, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);
        if (info == INVALID_HANDLE_VALUE) return 0;

        for (index=0; 1 ;index++)
        {
                iface.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);
                ret = SetupDiEnumDeviceInterfaces(info, NULL, &guid, index, &iface);
                if (!ret) return count;

                SetupDiGetInterfaceDeviceDetail(info, &iface, NULL, 0, &reqd_size, NULL);
                details = (SP_DEVICE_INTERFACE_DETAIL_DATA *)malloc(reqd_size);
                if (details == NULL) continue;

                memset(details, 0, reqd_size);
                details->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);
                ret = SetupDiGetDeviceInterfaceDetail(info, &iface, details, reqd_size, NULL, NULL);
                if (!ret)
                {
                        free(details);
                        continue;
                }

                h = CreateFile(details->DevicePath, GENERIC_READ|GENERIC_WRITE, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);
                if (h == INVALID_HANDLE_VALUE)
                {
                        DWORD err = GetLastError();

                        // I get ERROR_ACCESS_DENIED with most/all my input devices (mice/trackballs/tablet).
                        // Let's not log it :)
                        if (err == ERROR_ACCESS_DENIED)
                        {
                                free(details);
                                continue;
                        }

                        // qDebug wipes the GetLastError() it seems, so do that after print_win32_err().
                        print_win32_err(err);
                        qDebug() << "Problem opening handle, path: " << QString().fromWCharArray(details->DevicePath);

                        free(details);
                        continue;
                }

                free(details);

                attrib.Size = sizeof(HIDD_ATTRIBUTES);
                ret = HidD_GetAttributes(h, &attrib);
                //printf("vid: %4x\n", attrib.VendorID);
                if (!ret || (vid > 0 && attrib.VendorID != vid) ||
              (pid > 0 && attrib.ProductID != pid) ||
                          !HidD_GetPreparsedData(h, &hid_data))
                {
                        CloseHandle(h);
                        continue;
                }

                if (!HidP_GetCaps(hid_data, &capabilities) ||
              (usage_page > 0 && capabilities.UsagePage != usage_page) ||
                          (usage > 0 && capabilities.Usage != usage))
                {
                        HidD_FreePreparsedData(hid_data);
                        CloseHandle(h);
                        continue;
                }

                HidD_FreePreparsedData(hid_data);

                hid = (struct hid_struct *)malloc(sizeof(struct hid_struct));
                if (!hid)
                {
                        CloseHandle(h);
                        continue;
                }

//              COMMTIMEOUTS CommTimeouts;
//              CommTimeouts.ReadIntervalTimeout = 100;                 // 100ms
//              CommTimeouts.ReadTotalTimeoutConstant = 5;              // ms
//              CommTimeouts.ReadTotalTimeoutMultiplier = 1;    //
//              CommTimeouts.WriteTotalTimeoutConstant = 5;             // ms
//              CommTimeouts.WriteTotalTimeoutMultiplier = 1;   //
//              if (!SetCommTimeouts(h, &CommTimeouts))
//              {
////                    DWORD err = GetLastError();
//
//              }

                qDebug("Open: Handle address: %li for num: %i", (long int) h, count);

                hid->handle = h;
                add_hid(hid);

                count++;
                if (count >= max) return count;
        }

        return count;
}

//  recveive - receive a packet
//    Inputs:
//      num = device to receive from (zero based)
//      buf = buffer to receive packet
//      len = buffer's size
//      timeout = time to wait, in milliseconds
//    Output:
//      number of bytes received, or -1 on error
//
int pjrc_rawhid::receive(int num, void *buf, int len, int timeout)
{
        OVERLAPPED ov;
        DWORD n;

        hid_t *hid = get_hid(num);
        if (!hid)
                return -1;
        if (!hid->handle)
                return -1;

        EnterCriticalSection(&rx_mutex);

        ResetEvent(&rx_event);

        memset(&ov, 0, sizeof(ov));
        ov.hEvent = rx_event;

        if (!ReadFile(hid->handle, buf, len, NULL, &ov))
        {
                DWORD err = GetLastError();

                if (err == ERROR_DEVICE_NOT_CONNECTED)
                {       // the device has been unplugged
                        print_win32_err(err);
                        hid_close(hid);
                        LeaveCriticalSection(&rx_mutex);
                        emit deviceUnplugged(num);
                        return -1;
                }

                if (err != ERROR_IO_PENDING)
                {
                        print_win32_err(err);
                        LeaveCriticalSection(&rx_mutex);
                        return -1;
                }

                DWORD r = WaitForSingleObject(rx_event, timeout);
                if (r == WAIT_TIMEOUT)
                {
                        CancelIo(hid->handle);
                        LeaveCriticalSection(&rx_mutex);
                        return 0;
                }
                if (r != WAIT_OBJECT_0)
                {
                        DWORD err = GetLastError();
                        print_win32_err(err);
                        LeaveCriticalSection(&rx_mutex);
                        return -1;
                }
        }

        if (!GetOverlappedResult(hid->handle, &ov, &n, FALSE))
        {
                DWORD err = GetLastError();
                print_win32_err(err);

                if (err == ERROR_DEVICE_NOT_CONNECTED)
                {       // the device has been unplugged
                        hid_close(hid);
                        LeaveCriticalSection(&rx_mutex);
                        emit deviceUnplugged(num);
                        return -1;
                }

                LeaveCriticalSection(&rx_mutex);
                return -1;
        }

        LeaveCriticalSection(&rx_mutex);

        if (n <= 0) return -1;

//      qDebug("Received %i bytes, first %x, second %x", len, *((char *) buf),*((char *)buf + 1));

        if ((int)n > len) n = len;
        return n;
}

//  send - send a packet
//    Inputs:
//      num = device to transmit to (zero based)
//      buf = buffer containing packet to send
//      len = number of bytes to transmit
//      timeout = time to wait, in milliseconds
//    Output:
//      number of bytes sent, or -1 on error
//
int pjrc_rawhid::send(int num, void *buf, int len, int timeout)
{
        OVERLAPPED ov;
        DWORD n, r;

        hid_t *hid = get_hid(num);
        if (!hid)
                return -1;
        if (!hid->handle)
                return -1;

//      qDebug("Send: Handle address: %li for num: %i", (long int) hid->handle, num);

        EnterCriticalSection(&tx_mutex);

        ResetEvent(&tx_event);

        memset(&ov, 0, sizeof(ov));
        ov.hEvent = tx_event;

//      qDebug("Trying to write %u bytes.  First %x second %x",len, *((char *) buf), *((char *)buf + 1));

        if (!WriteFile(hid->handle, buf, len, NULL, &ov))
        {
                DWORD err = GetLastError();

                if (err == ERROR_DEVICE_NOT_CONNECTED)
                {       // the device has been unplugged
                        hid_close(hid);
                        LeaveCriticalSection(&tx_mutex);
                        emit deviceUnplugged(num);
                        return -1;
                }

                if (err == ERROR_SUCCESS || err == ERROR_IO_PENDING)
                {
//                      qDebug("Waiting for write to finish");
                        r = WaitForSingleObject(tx_event, timeout);
                        if (r == WAIT_TIMEOUT)
                        {
                                CancelIo(hid->handle);
                                LeaveCriticalSection(&tx_mutex);
                                return 0;
                        }
                        if (r != WAIT_OBJECT_0)
                        {
                                DWORD err = GetLastError();
                                print_win32_err(err);
                                LeaveCriticalSection(&tx_mutex);
                                return -1;
                        }
                }
                else
                {
//                      qDebug("Error writing to file");
                        print_win32_err(err);
                        LeaveCriticalSection(&tx_mutex);
                        return -1;
                }
        }

        if (!GetOverlappedResult(hid->handle, &ov, &n, FALSE))
        {
                DWORD err = GetLastError();

                qDebug("Problem getting overlapped result");
                print_win32_err(err);

                if (err == ERROR_DEVICE_NOT_CONNECTED)
                {       // the device has been unplugged
                        hid_close(hid);
                        LeaveCriticalSection(&tx_mutex);
                        emit deviceUnplugged(num);
                        return -1;
                }
        }

        LeaveCriticalSection(&tx_mutex);

        if (n <= 0) return -1;
        return n;
}

QString pjrc_rawhid::getserial(int num)
{
        hid_t *hid = get_hid(num);
        if (!hid)
                return "";
        if (!hid->handle)
                return "";

        // Should we do some "critical section" stuff here??
        char temp[126];
        if (!HidD_GetSerialNumberString(hid->handle, temp, sizeof(temp)))
        {
                DWORD err = GetLastError();
                print_win32_err(err);

                if (err == ERROR_DEVICE_NOT_CONNECTED)
                {       // the device has been unplugged
                        hid_close(hid);
                        emit deviceUnplugged(num);
                        return "";
                }

                return QString("Error");
        }

        return QString().fromUtf16((ushort*)temp,-1);
}

//  close - close a device
//
//    Inputs:
//      num = device to close (zero based)
//    Output
//      (nothing)
//
void pjrc_rawhid::close(int num)
{
        hid_close(get_hid(num));
}

void pjrc_rawhid::add_hid(hid_t *h)
{
        if (!h) return;

        if (!first_hid || !last_hid)
        {
                first_hid = last_hid = h;
                h->next = h->prev = NULL;
                return;
        }

        last_hid->next = h;
        h->prev = last_hid;
        h->next = NULL;
        last_hid = h;
}

hid_t * pjrc_rawhid::get_hid(int num)
{
        hid_t *p;
        for (p = first_hid; p && num > 0; p = p->next, num--) ;
        return p;
}

void pjrc_rawhid::free_all_hid(void)
{
        for (hid_t *p = first_hid; p; p = p->next)
                hid_close(p);

        hid_t *p = first_hid;
        while (p)
        {
                hid_t *q = p;
                p = p->next;
                free(q);
        }

        first_hid = last_hid = NULL;
}

void pjrc_rawhid::hid_close(hid_t *hid)
{
        if (!hid) return;
        if (!hid->handle) return;

        CloseHandle(hid->handle);
        hid->handle = NULL;
}

void pjrc_rawhid::print_win32_err(DWORD err)
{
        char buf[256];
        char temp[256];

        //FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, err, 0, (WCHAR*)buf, sizeof(buf), NULL);
        FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, err, MAKELANGID(LANG_ENGLISH, SUBLANG_DEFAULT), (WCHAR*)buf, sizeof(buf), NULL);
        WideCharToMultiByte( CP_ACP, 0, (WCHAR*)buf, sizeof(buf), temp, sizeof(temp), NULL, NULL );
        printf("err %ld: %s\n", err, temp);
}

// see http://msdn.microsoft.com/en-us/library/ms791134.aspx for list of GUID classes
#ifndef GUID_DEVCLASS_PORTS
DEFINE_GUID(GUID_DEVCLASS_PORTS, 0x4d1e55b2, 0xf16f, 0x11cf, 0x88, 0xcb, 0x00, 0x11, 0x11, 0x00, 0x00, 0x30);
#endif

/* Gordon Schumacher's macros for TCHAR -> QString conversions and vice versa */
#ifdef UNICODE
#define QStringToTCHAR(x)     (wchar_t*) x.utf16()
#define PQStringToTCHAR(x)    (wchar_t*) x->utf16()
#define TCHARToQString(x)     QString::fromUtf16((ushort*)(x))
#define TCHARToQStringN(x,y)  QString::fromUtf16((ushort*)(x),(y))
#else
#define QStringToTCHAR(x)     x.local8Bit().constData()
#define PQStringToTCHAR(x)    x->local8Bit().constData()
#define TCHARToQString(x)     QString::fromLocal8Bit((x))
#define TCHARToQStringN(x,y)  QString::fromLocal8Bit((x),(y))
#endif /*UNICODE*/

void pjrc_rawhid::setUpNotifications( )
{
#ifdef QT_GUI_LIB
    if(notificationWidget)
        return;
    notificationWidget = new USBRegistrationWidget(this);

    DEV_BROADCAST_DEVICEINTERFACE dbh;
    ZeroMemory(&dbh, sizeof(dbh));
    dbh.dbcc_size = sizeof(dbh);
    dbh.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;
    CopyMemory(&dbh.dbcc_classguid, &GUID_DEVCLASS_PORTS, sizeof(GUID));
    if( RegisterDeviceNotification( notificationWidget->winId( ), &dbh, DEVICE_NOTIFY_WINDOW_HANDLE ) == NULL)
        qWarning() << "RegisterDeviceNotification failed:" << GetLastError();
    // setting up notifications doesn't tell us about devices already connected
    // so get those manually
#else
    qWarning("GUI not enabled - can't register for device notifications.");
#endif // QT_GUI_LIB
}
LRESULT pjrc_rawhid::onDeviceChangeWin( WPARAM wParam, LPARAM lParam )
{
    if ( DBT_DEVICEARRIVAL == wParam || DBT_DEVICEREMOVECOMPLETE == wParam )
    {
        PDEV_BROADCAST_HDR pHdr = (PDEV_BROADCAST_HDR)lParam;
        if( pHdr->dbch_devicetype == DBT_DEVTYP_DEVICEINTERFACE )
        {
            PDEV_BROADCAST_DEVICEINTERFACE pDevInf = (PDEV_BROADCAST_DEVICEINTERFACE)pHdr;
            // delimiters are different across APIs...change to backslash.  ugh.
            QString deviceID = TCHARToQString(pDevInf->dbcc_name).toUpper().replace("#", "\\");
            matchAndDispatchChangedDevice(deviceID, GUID_DEVCLASS_PORTS, wParam);
        }
    }
    return 0;
}
#ifdef QT_GUI_LIB
bool USBRegistrationWidget::winEvent( MSG* message, long* result )
{
    if ( message->message == WM_DEVICECHANGE ) {
        qese->onDeviceChangeWin( message->wParam, message->lParam );
        *result = 1;
        return true;
    }
    return false;
}
#endif
bool pjrc_rawhid::matchAndDispatchChangedDevice(const QString & deviceID, const GUID & guid, WPARAM wParam)
{
    bool rv = false;
    DWORD dwFlag = (DBT_DEVICEARRIVAL == wParam) ? DIGCF_PRESENT : DIGCF_ALLCLASSES;
    HDEVINFO devInfo;
    if( (devInfo = SetupDiGetClassDevs(&guid,NULL,NULL,dwFlag)) != INVALID_HANDLE_VALUE )
    {
        SP_DEVINFO_DATA spDevInfoData;
        spDevInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
        for(int i=0; SetupDiEnumDeviceInfo(devInfo, i, &spDevInfoData); i++)
        {
            DWORD nSize=0 ;
            TCHAR buf[MAX_PATH];
            if ( SetupDiGetDeviceInstanceId(devInfo, &spDevInfoData, buf, MAX_PATH, &nSize) &&
                 deviceID.contains(TCHARToQString(buf))) // we found a match
            {
                rv = true;
                USBPortInfo info;
                //info.productID = info.vendorID = 0;
                getDeviceDetailsWin( &info, devInfo, &spDevInfoData, wParam );
                if( wParam == DBT_DEVICEARRIVAL )
                    emit deviceDiscovered(info);
                else if( wParam == DBT_DEVICEREMOVECOMPLETE )
                    emit deviceRemoved(info);
                break;
            }
        }
        SetupDiDestroyDeviceInfoList(devInfo);
    }
    return rv;
}
bool pjrc_rawhid::getDeviceDetailsWin( USBPortInfo* portInfo, HDEVINFO devInfo, PSP_DEVINFO_DATA devData, WPARAM wParam )
{
    portInfo->friendName = getDeviceProperty(devInfo, devData, SPDRP_FRIENDLYNAME);
    if( wParam == DBT_DEVICEARRIVAL)
        portInfo->physName = getDeviceProperty(devInfo, devData, SPDRP_PHYSICAL_DEVICE_OBJECT_NAME);
    portInfo->enumName = getDeviceProperty(devInfo, devData, SPDRP_ENUMERATOR_NAME);
    QString hardwareIDs = getDeviceProperty(devInfo, devData, SPDRP_HARDWAREID);
    QRegExp idRx("VID_(\\w+)&PID_(\\w+)");
    if( hardwareIDs.toUpper().contains(idRx) )
    {
        bool dummy;
        portInfo->vendorID = idRx.cap(1).toInt(&dummy, 16);
        portInfo->productID = idRx.cap(2).toInt(&dummy, 16);
    }
    return true;
}
QString pjrc_rawhid::getDeviceProperty(HDEVINFO devInfo, PSP_DEVINFO_DATA devData, DWORD property)
{
    DWORD buffSize = 0;
    SetupDiGetDeviceRegistryProperty(devInfo, devData, property, NULL, NULL, 0, & buffSize);
    BYTE* buff = new BYTE[buffSize];
    SetupDiGetDeviceRegistryProperty(devInfo, devData, property, NULL, buff, buffSize, NULL);
    QString result = TCHARToQString(buff);
    delete [] buff;
    return result;
}

/**
 ******************************************************************************
 *
 * @file       usbmonitor_win.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2013.
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup opHIDPlugin HID Plugin
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

#include <QMetaType>
#include <QString>
#include <initguid.h>
#include <QEventLoop>
#include <QTimer>
#include "ophid_usbmon.h"
#include <QDebug>
#include "ophid_const.h"

#define printf qDebug

static QMutex DeviceChangeMutex;

void USBMonitor::deviceEventReceived()
{
    qDebug() << "Device event";
    // Dispatch and emit the signals here...
}

USBMonitor *USBMonitor::instance()
{
    return m_instance;
}

USBMonitor *USBMonitor::m_instance = 0;


USBMonitor::USBMonitor(QObject *parent) : QThread(parent)
{
    HidD_GetHidGuid(&guid_hid);
    if (!QMetaType::isRegistered(QMetaType::type("USBPortInfo"))) {
        qRegisterMetaType<USBPortInfo>("USBPortInfo");
    }
#if (defined QT_GUI_LIB)
    notificationWidget = 0;
#endif // Q_OS_WIN
    setUpNotifications();
    m_instance = this;
}

USBMonitor::~USBMonitor()
{
#if (defined QT_GUI_LIB)
    if (notificationWidget) {
        delete notificationWidget;
    }
#endif
    quit();
}

/**
   Be a bit more picky and ask only for a specific type of device:
     On OpenPilot, the bcdDeviceLSB indicates the run state: bootloader or running.
     bcdDeviceMSB indicates the board model.
 */
QList<USBPortInfo> USBMonitor::availableDevices(int vid, int pid, int bcdDeviceMSB, int bcdDeviceLSB)
{
    int i;
    QList<USBPortInfo> thePortsWeWant;

    OPHID_TRACE("IN");
    
    // Trigger enumeration
    //OPHID_DEBUG("Enumerate");
    //availableDevices();

    // Print the list
    qDebug() << "List off (" << knowndevices.length() << ") devices that are tracked:";
    foreach(USBPortInfo info, knowndevices /*thePortsWeWant*/) {
        qDebug() << "product:" << info.product 
	         << " bcdDevice:" << info.bcdDevice 
		 << " devicePath:" << info.devicePath;

        //
        if ((info.vendorID == vid || vid == -1) &&
            (info.productID == pid || pid == -1) && 
	    ((info.bcdDevice >> 8) == bcdDeviceMSB || bcdDeviceMSB == -1) &&
            ((info.bcdDevice & 0x00ff) == bcdDeviceLSB || bcdDeviceLSB == -1)) {
            thePortsWeWant.append(info);
	    OPHID_DEBUG("Found device.");
        }
    }

    OPHID_TRACE("OUT");
    
    return thePortsWeWant;
}


// see http://msdn.microsoft.com/en-us/library/ms791134.aspx for list of GUID classes
/*#ifndef GUID_DEVCLASS_PORTS
   DEFINE_GUID(GUID_DEVCLASS_PORTS, //0x4d1e55b2, 0xf16f, 0x11cf, 0x88, 0xcb, 0x00, 0x11, 0x11, 0x00, 0x00, 0x30);
                                 //0x745a17a0, 0x74d3, 0x11d0, 0xb6, 0xfe, 0x00, 0xa0, 0xc9, 0x0f, 0x57, 0xda);
                                   0xA5DCBF10, 0x6530, 0x11D2, 0x90, 0x1F, 0x00, 0xC0, 0x4F, 0xB9, 0x51, 0xED);
   #endif
 */
/* Gordon Schumacher's macros for TCHAR -> QString conversions and vice versa */
#ifdef UNICODE
#define QStringToTCHAR(x)     (wchar_t *)x.utf16()
#define PQStringToTCHAR(x)    (wchar_t *)x->utf16()
#define TCHARToQString(x)     QString::fromUtf16((ushort *)(x))
#define TCHARToQStringN(x, y) QString::fromUtf16((ushort *)(x), (y))
#else
#define QStringToTCHAR(x)     x.local8Bit().constData()
#define PQStringToTCHAR(x)    x->local8Bit().constData()
#define TCHARToQString(x)     QString::fromLocal8Bit((x))
#define TCHARToQStringN(x, y) QString::fromLocal8Bit((x), (y))
#endif /*UNICODE*/
#define HID2IGNOREUC "COL02"
#define HID2IGNORElc "col02"

void USBMonitor::setUpNotifications()
{
#ifdef QT_GUI_LIB
    if (notificationWidget) {
        return;
    }
    notificationWidget = new USBRegistrationWidget(this);

    DEV_BROADCAST_DEVICEINTERFACE dbh;
    ZeroMemory(&dbh, sizeof(dbh));
    dbh.dbcc_size = sizeof(dbh);
    dbh.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;
    CopyMemory(&dbh.dbcc_classguid, &guid_hid, sizeof(GUID));
    if (RegisterDeviceNotification(notificationWidget->winId(), &dbh, DEVICE_NOTIFY_WINDOW_HANDLE) == NULL) {
        qWarning() << "RegisterDeviceNotification failed:" << GetLastError();
    }
    // setting up notifications doesn't tell us about devices already connected
    // so get those manually
    foreach(USBPortInfo port, availableDevices())
    emit deviceDiscovered(port);
#else
    qWarning("GUI not enabled - can't register for device notifications.");
#endif // QT_GUI_LIB
}
LRESULT USBMonitor::onDeviceChangeWin(WPARAM wParam, LPARAM lParam)
{
    OPHID_TRACE("IN");
    if (DBT_DEVICEARRIVAL == wParam || DBT_DEVICEREMOVECOMPLETE == wParam) {
        PDEV_BROADCAST_HDR pHdr = (PDEV_BROADCAST_HDR)lParam;
        if (pHdr->dbch_devicetype == DBT_DEVTYP_DEVICEINTERFACE) {
            PDEV_BROADCAST_DEVICEINTERFACE pDevInf = (PDEV_BROADCAST_DEVICEINTERFACE)pHdr;
            // delimiters are different across APIs...change to backslash.  ugh.
            QString deviceID = TCHARToQString(pDevInf->dbcc_name).toUpper().replace("#", "\\");
            matchAndDispatchChangedDevice(deviceID, guid_hid, wParam);
        }
    }
    OPHID_TRACE("OUT");
    return 0;
}
#ifdef QT_GUI_LIB
bool USBRegistrationWidget::winEvent(MSG *message, long *result)
{
    bool ret = false;

    if (message->message == WM_DEVICECHANGE) {
        OPHID_TRACE("IN");
        qese->onDeviceChangeWin(message->wParam, message->lParam);
        *result = 1;
        ret = true;
        OPHID_TRACE("OUT");
    }

    return ret;
}
#endif
bool USBMonitor::matchAndDispatchChangedDevice(const QString & deviceID, const GUID & guid, WPARAM wParam)
{
    qDebug() << "USB_MONITOR matchAndDispatchChangedDevice deviceID=" << deviceID;
    bool rv = false;
    bool rc;
    DWORD dwFlag = (DBT_DEVICEARRIVAL == wParam) ? DIGCF_PRESENT : 0/*DIGCF_ALLCLASSES*/;
    HDEVINFO devInfo;
//    QString qHIDSymbolicName="HID"; 
//    WCHAR wHIDSymbolicName[10];
//    qHIDSymbolicName.toWCharArray(wHIDSymbolicName);
    
    if ((devInfo = SetupDiGetClassDevs(&guid, NULL, NULL, dwFlag | DIGCF_DEVICEINTERFACE)) != INVALID_HANDLE_VALUE) {
        SP_DEVINFO_DATA spDevInfoData;
        spDevInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
        for (DWORD i = 0; SetupDiEnumDeviceInfo(devInfo, i, &spDevInfoData); i++) {
            DWORD nSize = 0;
            TCHAR buf[MAX_PATH];
	    rc = SetupDiGetDeviceInstanceId(devInfo, &spDevInfoData, buf, MAX_PATH, &nSize);
	    qDebug() << "PATH:" << TCHARToQString(buf);
            if (rc && deviceID.contains(TCHARToQString(buf)) /*&& !QString().contains(TCHARToQString(HID2IGNOREUC))*/) { // we found a match
	        qDebug() << "ACCEPTED" << TCHARToQString(buf);
                USBPortInfo info;
                info.devicePath = deviceID;
                if (wParam == DBT_DEVICEARRIVAL) {
                    qDebug() << "USB_MONITOR INSERTION";
                    if (infoFromHandle(guid, info, devInfo, i) != 1) {
                        qDebug() << "USB_MONITOR infoFromHandle failed on matchAndDispatchChangedDevice";
                        break;
                    }
		    if (knowndevices.length()) {
                        foreach(USBPortInfo m_info, knowndevices) {
		    
		            qDebug() << m_info.devicePath;
		            qDebug() << info.devicePath;

                            if (m_info.serialNumber == info.serialNumber && 
			        m_info.productID == info.productID && 
				m_info.bcdDevice == info.bcdDevice && 
				m_info.devicePath == info.devicePath) {
                                qDebug() << "USB_MONITOR device already present don't emit signal";
                                break;
                            }
                        }
		    }
                    if (info.bcdDevice == 0 || info.product.isEmpty()) {
                        qDebug() << "USB_MONITOR empty information on device not emiting signal";
                        break;
                    }
                    knowndevices.append(info);
                    qDebug() << "USB_MONITOR emit device discovered on device:" 
		             << info.product 
			     << info.bcdDevice;
                    emit deviceDiscovered(info);
                    break;
                } else if (wParam == DBT_DEVICEREMOVECOMPLETE) {
                    bool found = false;
    //                DeviceChangeMutex.lock();
                    for (int x = 0; x < knowndevices.count(); ++x) {
                       // if (knowndevices[x].devicePath == deviceID) {
                            USBPortInfo temp = knowndevices.at(x);
                            knowndevices.removeAt(x);
                            qDebug() << "USB_MONITOR emit device removed on device:" << temp.product << temp.bcdDevice;
                          
			    //emit deviceRemoved(temp);
			 
                            found = true;
                    //        break;
                    //    }
                    }

                    //if (!found) {
                    //    qDebug() << "USB_MONITOR emit device removed on unknown device";
                        emit deviceRemoved(info);
                    //}
		    //else
		        break;
//		    DeviceChangeMutex.unlock();
                }
                break;
            }
        }
        SetupDiDestroyDeviceInfoList(devInfo);
    }
    OPHID_TRACE("OUT");
    return rv;
}

QString USBMonitor::getDeviceProperty(HDEVINFO devInfo, PSP_DEVINFO_DATA devData, DWORD property)
{
    DWORD buffSize = 0;

    SetupDiGetDeviceRegistryProperty(devInfo, devData, property, NULL, NULL, 0, &buffSize);
    BYTE *buff     = new BYTE[buffSize];
    SetupDiGetDeviceRegistryProperty(devInfo, devData, property, NULL, buff, buffSize, NULL);
    QString result = TCHARToQString(buff);
    delete[] buff;
    return result;
}


QList<USBPortInfo> USBMonitor::availableDevices() {

     enumerateDevicesWin(guid_hid);
     return knowndevices;
}


void USBMonitor::enumerateDevicesWin(const GUID & guid)
{
    HDEVINFO devInfo;
    USBPortInfo info;
    DWORD j = 0;
    DWORD i = 0;
    OPHID_TRACE("IN");
    DeviceChangeMutex.lock();

//    QString qHIDSymbolicName = "HID";
//    WCHAR wHIDSymbolicName[10];
//    qHIDSymbolicName.toWCharArray(wHIDSymbolicName);
    // qDebug()<<"enumerateDevicesWin1";
    if ((devInfo = SetupDiGetClassDevs(&guid, NULL, NULL, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE)) != INVALID_HANDLE_VALUE) {
        // qDebug()<<"enumerateDevicesWin2";
        SP_DEVINFO_DATA devInfoData;
        devInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
	// only handle the first one enumerated that matches openpilot vendor
	// the second one is expected to be the gamepad controller which we don't want to deal with
        for (i = 0; SetupDiEnumDeviceInfo(devInfo, i, &devInfoData); i++) {
            int r = infoFromHandle(guid, info, devInfo, i);
            if (r == 1) {
                //infoList->append(info);
		knowndevices.append(info);
		j++;
	//	break;
            } else if (r == OPHID_ERROR_ENUMERATION) {
                break;
            }
        }
        SetupDiDestroyDeviceInfoList(devInfo);
    }
    OPHID_DEBUG("Added %d device(s).", j);
    DeviceChangeMutex.unlock();
    OPHID_TRACE("OUT");
}

int USBMonitor::infoFromHandle(const GUID & guid, USBPortInfo & info, HDEVINFO & devInfo, DWORD & index)
{
    OPHID_TRACE("IN");
    // qDebug()<<"index0="<<index;
    int ret;
    HANDLE h;
    SP_DEVICE_INTERFACE_DATA iface;
    SP_DEVICE_INTERFACE_DETAIL_DATA *details;
    DWORD reqd_size;
    HIDD_ATTRIBUTES attrib;
    PHIDP_PREPARSED_DATA hid_data;
    HIDP_CAPS capabilities;
    QString qDevicePath;

    iface.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);

    ret = SetupDiEnumDeviceInterfaces(devInfo, NULL, &guid, index, &iface);
    if (!ret) {
        ret = OPHID_ERROR_ENUMERATION;
	goto leave;
    }
    // qDebug()<<"index1="<<index;
    SetupDiGetInterfaceDeviceDetail(devInfo, &iface, NULL, 0, &reqd_size, NULL);
    details = (SP_DEVICE_INTERFACE_DETAIL_DATA *)malloc(reqd_size);
    if (details == NULL) {
        ret = 2;
	goto leave;
    }
    // qDebug()<<"index2="<<index;
    memset(details, 0, reqd_size);
    details->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);
    ret = SetupDiGetDeviceInterfaceDetail(devInfo, &iface, details, reqd_size, NULL, NULL);
    if (!ret) {
        free(details);
        ret = 2;
	goto leave;
    }
    // qDebug()<<"index3="<<index;
   qDevicePath = QString().fromWCharArray(details->DevicePath).toUpper();

   // Exclude any non-openpilot devices
   if (!qDevicePath.contains("VID_20A0")) {
      ret = 2;
      goto leave;
   }

   if (qDevicePath.contains("COL02")) { 
      ret = 2;
      goto leave;
   }
   //.replace("#","\\").remove(0,4);
   //WCHAR wDevicePath[256];
   //qDevicePath.toWCharArray(wDevicePath);
   qDebug() << "Found device with valid PATH: " << qDevicePath;
    h = CreateFile(details->DevicePath /*DevicePath*/, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);

    if (h == INVALID_HANDLE_VALUE) {
        DWORD err = GetLastError();

        // I get ERROR_ACCESS_DENIED with most/all my input devices (mice/trackballs/tablet).
        // Let's not log it :)
        if (err == ERROR_ACCESS_DENIED) {
            free(details);
	    ret = 2;
	    goto leave;
        }

        qDebug() << "Problem opening handle, path: " << QString().fromWCharArray(details->DevicePath);

        free(details);
        ret = 2;
	goto leave;
    }
    // qDebug()<<"index4="<<index;
    free(details);
    // qDebug()<<"DETAILS???"<<QString().fromWCharArray(details->DevicePath).toUpper().replace("#", "\\");
    attrib.Size    = sizeof(HIDD_ATTRIBUTES);
    ret = HidD_GetAttributes(h, &attrib);
    info.vendorID  = attrib.VendorID;
    info.productID = attrib.ProductID;
    info.bcdDevice = attrib.VersionNumber;
    info.devicePath = qDevicePath;

    if (attrib.VendorID != 0x20A0) {
       CloseHandle(h);
       ret = 2;
       goto leave;
    }

    if (!ret || !HidD_GetPreparsedData(h, &hid_data)) {
        CloseHandle(h);
        ret = 2;
	goto leave;
    }
    // qDebug()<<"index5="<<index;
    if (!HidP_GetCaps(hid_data, &capabilities)) {
        HidD_FreePreparsedData(hid_data);
        CloseHandle(h);
        ret = 2;
	goto leave;
    }
    // qDebug()<<"index6="<<index;
    info.UsagePage = capabilities.UsagePage;
    info.Usage     = capabilities.Usage;
    HidD_FreePreparsedData(hid_data);
    char temp[126];
    HidD_GetSerialNumberString(h, temp, sizeof(temp));
    info.serialNumber = QString().fromUtf16((ushort *)temp, -1);
    HidD_GetManufacturerString(h, temp, sizeof(temp));
    info.manufacturer = QString().fromUtf16((ushort *)temp, -1);
    HidD_GetProductString(h, temp, sizeof(temp));
    info.product = QString().fromUtf16((ushort *)temp, -1);
    // qDebug()<<"index="<<index<<"ProductID="<<info.product;
    CloseHandle(h);
    h = NULL;
    ret = 1;

leave:
    OPHID_TRACE("OUT");
    return ret;
}

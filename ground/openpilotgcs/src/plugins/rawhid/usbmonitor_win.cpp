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

#include <QMetaType>
#include <QString>
#include <initguid.h>

#include "usbmonitor.h"
#include <QDebug>

#define printf qDebug

void USBMonitor::deviceEventReceived() {

    qDebug() << "Device event";
    // Dispatch and emit the signals here...

}


USBMonitor::USBMonitor(QObject *parent): QThread(parent) {
    if( !QMetaType::isRegistered( QMetaType::type("USBPortInfo") ) )
        qRegisterMetaType<USBPortInfo>("USBPortInfo");
#if (defined QT_GUI_LIB)
    notificationWidget = 0;
#endif // Q_OS_WIN
    setUpNotifications();
}

USBMonitor::~USBMonitor()
{
#if (defined QT_GUI_LIB)
    if( notificationWidget )
        delete notificationWidget;
#endif
    quit();
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

void USBMonitor::setUpNotifications( )
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
    foreach( USBPortInfo port,availableDevices() )
      emit deviceDiscovered( port );
#else
    qWarning("GUI not enabled - can't register for device notifications.");
#endif // QT_GUI_LIB
}
LRESULT USBMonitor::onDeviceChangeWin( WPARAM wParam, LPARAM lParam )
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
bool USBMonitor::matchAndDispatchChangedDevice(const QString & deviceID, const GUID & guid, WPARAM wParam)
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
                info.productID = info.vendorID = 0;
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
bool USBMonitor::getDeviceDetailsWin( USBPortInfo* portInfo, HDEVINFO devInfo, PSP_DEVINFO_DATA devData, WPARAM wParam )
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
    qDebug()<<portInfo->productID;
    return true;
}
QString USBMonitor::getDeviceProperty(HDEVINFO devInfo, PSP_DEVINFO_DATA devData, DWORD property)
{
    DWORD buffSize = 0;
    SetupDiGetDeviceRegistryProperty(devInfo, devData, property, NULL, NULL, 0, & buffSize);
    BYTE* buff = new BYTE[buffSize];
    SetupDiGetDeviceRegistryProperty(devInfo, devData, property, NULL, buff, buffSize, NULL);
    QString result = TCHARToQString(buff);
    delete [] buff;
    return result;
}
/**
Returns a list of all currently available devices
*/
QList<USBPortInfo> USBMonitor::availableDevices()
{
    QList<USBPortInfo> ports;
    enumerateDevicesWin(GUID_DEVCLASS_PORTS, &ports);
    return ports;
}
void USBMonitor::enumerateDevicesWin( const GUID & guid, QList<USBPortInfo>* infoList )
{
    HDEVINFO devInfo;
    if( (devInfo = SetupDiGetClassDevs(&guid, NULL, NULL, DIGCF_PRESENT)) != INVALID_HANDLE_VALUE)
    {
        SP_DEVINFO_DATA devInfoData;
        devInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
        for(int i = 0; SetupDiEnumDeviceInfo(devInfo, i, &devInfoData); i++)
        {
            USBPortInfo info;
            info.productID = info.vendorID = 0;
            getDeviceDetailsWin( &info, devInfo, &devInfoData );
            infoList->append(info);
        }
        SetupDiDestroyDeviceInfoList(devInfo);
    }
}

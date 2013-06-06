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


USBMonitor *USBMonitor::m_instance = 0;


/**
 * \brief Device event received
 *
 */
void USBMonitor::deviceEventReceived()
{
    OPHID_DEBUG("Device event");
}


/**
 * \brief Get the instance of the USBMONITOR
 *
 * \return instance
 */
USBMonitor *USBMonitor::instance()
{
    return m_instance;
}


/**
 * \brief Constructor
 *
 */
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


/**
 * \brief Destructor
 *
 */
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
 * \brief return the device that matches the describtion
 *
 * \param[in] vid
 * \param[in] pid
 * \param[in] bcddeviceMSN Board model
 * \param[in] bcdLSB Run state (bl or running)
 * \return list
 * \retval  device
 */
QList<USBPortInfo> USBMonitor::availableDevices(int vid, int pid, int bcdDeviceMSB, int bcdDeviceLSB)
{
    QList<USBPortInfo> thePortsWeWant;

    OPHID_TRACE("IN");

    // Print the list
    qDebug() << "List off (" << knowndevices.length() << ") devices that are tracked:";
    foreach(USBPortInfo info, knowndevices /*thePortsWeWant*/) {
        qDebug() << "product:" << info.product
                 << " bcdDevice:" << info.bcdDevice
                 << " devicePath:" << info.devicePath;

        // Filter to return only the one request (if exists)
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


/**
 * \brief Create all appropriate filter and connctions
 *
 * \note Poll the devices to create our internal list of already connected devices
 *       This is to handle the case when GCS is started with a device already connected
 *
 */
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
    // discover the devices curently connected
    foreach(USBPortInfo port, availableDevices())
    emit deviceDiscovered(port);
#else
    qWarning("GUI not enabled - can't register for device notifications.");
#endif // QT_GUI_LIB
}


/**
 * \brief filter out the windows signal we don't want to handle
 *
 * \param[in] wParam event
 * \param[out] lParam interface
 * \return status.
 * \retval 0
 */
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


/**
 * \brief re-root windows event for any device changes in the system
 *
 * \param[in] message
 * \param[out] result Broadcast that we took care of the event, if it is not the case, let the Widget know it so it can handle it instead.
 * \return status.
 * \retval device handled or not
 */
#ifdef QT_GUI_LIB
bool USBRegistrationWidget::winEvent(MSG *message, long *result)
{
    bool ret = false;

    if (message->message == WM_DEVICECHANGE) {
        OPHID_TRACE("IN");
        qese->onDeviceChangeWin(message->wParam, message->lParam);
        *result = 1;
        ret     = true;
        OPHID_TRACE("OUT");
    }

    return ret;
}
#endif


/**
 * \brief filter out the device based on information and populate to be added
 *
 * \note Triggered from device plug/unplug windows signal
 *
 * \param[in] deviceID The device that triggered the event
 * \param[in] guid Device class
 * \param[in] wParam Windows event
 * \return status.
 * \retval 0
 */
bool USBMonitor::matchAndDispatchChangedDevice(const QString & deviceID, const GUID & guid, WPARAM wParam)
{
    OPHID_TRACE("IN");

    qDebug() << "[STATUS CHANGE] from device ID: " << deviceID;
    bool rc;
    SP_DEVINFO_DATA spDevInfoData;
    DWORD dwFlag = (DBT_DEVICEARRIVAL == wParam) ? DIGCF_PRESENT : 0 /*DIGCF_ALLCLASSES*/;
    HDEVINFO devInfo;

    devInfo = SetupDiGetClassDevs(&guid, NULL, NULL, dwFlag | DIGCF_DEVICEINTERFACE);

    if (devInfo != INVALID_HANDLE_VALUE) {
        spDevInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
        for (DWORD i = 0; SetupDiEnumDeviceInfo(devInfo, i, &spDevInfoData); i++) {
            DWORD nSize = 0;
            TCHAR buf[MAX_PATH];
            rc = SetupDiGetDeviceInstanceId(devInfo, &spDevInfoData, buf, MAX_PATH, &nSize);
            qDebug() << "Found:" << TCHARToQString(buf);
            if (rc && deviceID.contains(TCHARToQString(buf))) {
                qDebug() << "[MATCH] " << TCHARToQString(buf);
                USBPortInfo info;
                info.devicePath = deviceID;
                if (wParam == DBT_DEVICEARRIVAL) {
                    qDebug() << "[INSERTED]";
                    if (infoFromHandle(guid, info, devInfo, i) != OPHID_NO_ERROR) {
                        OPHID_ERROR("Not found");
                        break;
                    }
                    if (knowndevices.length()) {
                        foreach(USBPortInfo m_info, knowndevices) {
                            if (m_info.serialNumber == info.serialNumber &&
                                m_info.productID == info.productID &&
                                m_info.bcdDevice == info.bcdDevice &&
                                m_info.devicePath == info.devicePath) {
                                OPHID_ERROR("Already present");
                                break;
                            }
                        }
                    }
                    if (info.bcdDevice == 0 || info.product.isEmpty()) {
                        OPHID_ERROR("Missing parameters");
                        break;
                    }
                    knowndevices.append(info);
                    qDebug() << "[SIGNAL] Device discovered on device:"
                             << info.product
                             << info.bcdDevice;
                    emit deviceDiscovered(info);
                    break;
                } else if (wParam == DBT_DEVICEREMOVECOMPLETE) {
                    for (int x = 0; x < knowndevices.count(); ++x) {
                        USBPortInfo temp = knowndevices.at(x);
                        knowndevices.removeAt(x);
                        qDebug() << "[SIGNAL] Device removed on device:"
                                 << temp.product
                                 << temp.bcdDevice;
                    }
                    emit deviceRemoved(info);
                    break;
                }
                break;
            }
        }
        SetupDiDestroyDeviceInfoList(devInfo);
    }
    OPHID_TRACE("OUT");
    return 0;
}


/**
 * \brief Get the list of currently handled devices
 *
 * \return QList
 * \retval  List of handled devices
 */
QList<USBPortInfo> USBMonitor::availableDevices()
{
    enumerateDevicesWin(guid_hid);
    return knowndevices;
}


/**
 * \brief enumerate devices
 *
 * \note fill our device list. This is called only from pooling during init.
 *
 * \param[in] GUID Filter for HID devices
 */
void USBMonitor::enumerateDevicesWin(const GUID & guid)
{
    HDEVINFO devInfo;
    USBPortInfo info;
    DWORD j = 0;
    DWORD i = 0;
    SP_DEVINFO_DATA devInfoData;

    OPHID_TRACE("IN");

    devInfo = SetupDiGetClassDevs(&guid, NULL, NULL, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);

    if (devInfo != INVALID_HANDLE_VALUE) {
        devInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
        for (i = 0; SetupDiEnumDeviceInfo(devInfo, i, &devInfoData); i++) {
            int r = infoFromHandle(guid, info, devInfo, i);
            if (r == OPHID_NO_ERROR) {
                knowndevices.append(info);
                j++;
                // break;
            } else if (r == OPHID_ERROR_ENUMERATION) {
                break;
            }
        }
        SetupDiDestroyDeviceInfoList(devInfo);
    }
    OPHID_DEBUG("Added %d device(s).", j);
    OPHID_TRACE("OUT");
}


/**
 * \brief filter out the device based on information and populate to be added
 *
 * \note Called from pooling during startup and from device plug/unplug windows signal
 *
 * \param[in] GUID Filter for HID devices
 * \param[out] info new device to be added to our private list (once all filter are validated)
 * \param[in] devInfo device information set.
 * \param[in] index
 * \return status.
 * \retval  OPHID_NO_ERROR, OPHID_ERROR_RET, OPHID_ERROR_ENUMERATION
 */
int USBMonitor::infoFromHandle(const GUID & guid, USBPortInfo & info, HDEVINFO & devInfo, DWORD & index)
{
    OPHID_TRACE("IN");
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

    // Enumerate the device interface that are contained in the device information set.
    ret = SetupDiEnumDeviceInterfaces(devInfo, NULL, &guid, index, &iface);
    if (!ret) {
        ret = OPHID_ERROR_ENUMERATION;
        goto leave;
    }

    // Fill the interface information
    SetupDiGetInterfaceDeviceDetail(devInfo, &iface, NULL, 0, &reqd_size, NULL);
    details = (SP_DEVICE_INTERFACE_DETAIL_DATA *)malloc(reqd_size);
    if (details == NULL) {
        ret = OPHID_ERROR_RET;
        goto leave;
    }
    memset(details, 0, reqd_size);
    details->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);
    ret = SetupDiGetDeviceInterfaceDetail(devInfo, &iface, details, reqd_size, NULL, NULL);
    if (!ret) {
        free(details);
        ret = OPHID_ERROR_RET;
        goto leave;
    }
    qDevicePath = QString().fromWCharArray(details->DevicePath).toUpper();

    // Exclude any non-openpilot devices
    if (!qDevicePath.contains("VID_20A0")) {
        ret = OPHID_ERROR_RET;
        goto leave;
    }
    // Exclude second hid which (probably) is the gamepad controller
    if (qDevicePath.contains("COL02")) {
        ret = OPHID_ERROR_RET;
        goto leave;
    }

    qDebug() << "Found device with valid PATH: " << qDevicePath;
    h = CreateFile(details->DevicePath,
                   GENERIC_READ | GENERIC_WRITE,
                   FILE_SHARE_READ | FILE_SHARE_WRITE,
                   NULL,
                   OPEN_EXISTING,
                   FILE_FLAG_OVERLAPPED,
                   NULL);

    if (h == INVALID_HANDLE_VALUE) {
        DWORD err = GetLastError();

        // I get ERROR_ACCESS_DENIED with most/all my input devices (mice/trackballs/tablet).
        // Let's not log it :)
        if (err == ERROR_ACCESS_DENIED) {
            free(details);
            ret = OPHID_ERROR_RET;
            goto leave;
        }

        qDebug() << "Problem opening handle, path: "
                 << QString().fromWCharArray(details->DevicePath);

        free(details);
        ret = OPHID_ERROR_RET;
        goto leave;
    }

    free(details);
    attrib.Size     = sizeof(HIDD_ATTRIBUTES);
    ret = HidD_GetAttributes(h, &attrib);
    info.vendorID   = attrib.VendorID;
    info.productID  = attrib.ProductID;
    info.bcdDevice  = attrib.VersionNumber;
    info.devicePath = qDevicePath;

    if (attrib.VendorID != 0x20A0) {
        CloseHandle(h);
        ret = OPHID_ERROR_RET;
        goto leave;
    }

    if (!ret || !HidD_GetPreparsedData(h, &hid_data)) {
        CloseHandle(h);
        ret = OPHID_ERROR_RET;
        goto leave;
    }
    if (!HidP_GetCaps(hid_data, &capabilities)) {
        HidD_FreePreparsedData(hid_data);
        CloseHandle(h);
        ret = OPHID_ERROR_RET;
        goto leave;
    }

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
    CloseHandle(h);
    h   = NULL;
    ret = OPHID_NO_ERROR;

leave:
    OPHID_TRACE("OUT");
    return ret;
}

/**
 ******************************************************************************
 *
 * @file       usbmonitor.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup RawHIDPlugin Raw HID Plugin
 * @{
 * @brief Monitors the USB bus for devince insertion/removal
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

#ifndef USBMONITOR_H
#define USBMONITOR_H

#include "rawhid_global.h"

#include <QThread>
#include <QMutex>

// Depending on the OS, we'll need different things:
#if defined( Q_OS_MAC)
#include <IOKit/IOKitLib.h>
#include <IOKit/hid/IOHIDLib.h>
#elif defined(Q_OS_UNIX)

#include <libudev.h>
#include <QSocketNotifier>

#elif defined (Q_OS_WIN32)
#ifndef _WIN32_WINNT
    #define _WIN32_WINNT 0x0500
#endif
#ifndef _WIN32_WINDOWS
    #define _WIN32_WINDOWS 0x0500
#endif
#ifndef WINVER
    #define WINVER 0x0500
#endif
#include <windows.h>
#include <dbt.h>
#include <setupapi.h>
#include <ddk/hidsdi.h>
#include <ddk/hidclass.h>
#endif



#ifdef Q_OS_WIN
#ifdef QT_GUI_LIB
#include <QWidget>
class USBMonitor;

class USBRegistrationWidget : public QWidget
{
    Q_OBJECT
public:
    USBRegistrationWidget( USBMonitor* qese ) {
        this->qese = qese;
    }
    ~USBRegistrationWidget( ) {}

protected:
    USBMonitor* qese;
    bool winEvent( MSG* message, long* result );
};
#endif
#endif

struct USBPortInfo {
    //QString friendName; ///< Friendly name.
    //QString physName;
    //QString enumName;   ///< It seems its the only one with meaning
    QString serialNumber; // As a string as it can be anything, really...
    QString manufacturer;
    QString product;
#if defined(Q_OS_WIN32)
    QString devicePath; //only has meaning on windows
#elif  defined(Q_OS_MAC)
    IOHIDDeviceRef dev_handle;
#endif
    int UsagePage;
    int Usage;
    int vendorID;       ///< Vendor ID.
    int productID;      ///< Product ID
    int bcdDevice;
};

/**
*   A monitoring thread which will wait for device events.
*/

class RAWHID_EXPORT USBMonitor : public QThread
{
    Q_OBJECT

public:
    enum RunState {
        Bootloader = 0x01,
        Running = 0x02
    };

    enum USBConstants {
        idVendor_OpenPilot = 0x20a0,
        idProduct_OpenPilot = 0x415a,
        idProduct_CopterControl = 0x415b,
        idProduct_PipXtreme = 0x415c
    };

    static USBMonitor *instance();

    USBMonitor(QObject *parent = 0);
    ~USBMonitor();
    QList<USBPortInfo> availableDevices();
    QList<USBPortInfo> availableDevices(int vid, int pid, int boardModel, int runState);
    #if defined (Q_OS_WIN32)
    LRESULT onDeviceChangeWin( WPARAM wParam, LPARAM lParam );
    #endif
signals:
    /*!
      A new device has been connected to the system.

      setUpNotifications() must be called first to enable event-driven device notifications.
      Currently only implemented on Windows and OS X.
      \param info The device that has been discovered.
    */
    void deviceDiscovered( const USBPortInfo & info );
    /*!
      A device has been disconnected from the system.

      setUpNotifications() must be called first to enable event-driven device notifications.
      Currently only implemented on Windows and OS X.
      \param info The device that was disconnected.
    */
    void deviceRemoved( const USBPortInfo & info );

private slots:
    /**
     Callback available for whenever the system that is put in place gets
     an event
     */
    void deviceEventReceived();

private:

    //! Mutex for modifying the list of available devices
    QMutex * listMutex;

    //! List of known devices maintained by callbacks
    QList<USBPortInfo> knowndevices;

    Q_DISABLE_COPY(USBMonitor)
    static USBMonitor *m_instance;


    // Depending on the OS, we'll need different things:
#if defined( Q_OS_MAC)
    static void attach_callback(void *context, IOReturn r, void *hid_mgr, IOHIDDeviceRef dev);
    static void detach_callback(void *context, IOReturn r, void *hid_mgr, IOHIDDeviceRef dev);
    void addDevice(USBPortInfo info);
    void removeDevice(IOHIDDeviceRef dev);
    IOHIDManagerRef hid_manager;
#elif defined(Q_OS_UNIX)
    struct udev *context;
    struct udev_monitor *monitor;
    QSocketNotifier *monitorNotifier;
    USBPortInfo makePortInfo(struct udev_device *dev);
#elif defined (Q_OS_WIN32)
    GUID guid_hid;
    void setUpNotifications();
     /*!
     * Get specific property from registry.
     * \param devInfo pointer to the device information set that contains the interface
     *    and its underlying device. Returned by SetupDiGetClassDevs() function.
     * \param devData pointer to an SP_DEVINFO_DATA structure that defines the device instance.
     *    this is returned by SetupDiGetDeviceInterfaceDetail() function.
     * \param property registry property. One of defined SPDRP_* constants.
     * \return property string.
     */
    static QString getDeviceProperty(HDEVINFO devInfo, PSP_DEVINFO_DATA devData, DWORD property);
    static int infoFromHandle(const GUID & guid,USBPortInfo & info,HDEVINFO & devInfo,DWORD & index);
    static void enumerateDevicesWin( const GUID & guidDev, QList<USBPortInfo>* infoList );
    bool matchAndDispatchChangedDevice(const QString & deviceID, const GUID & guid, WPARAM wParam);
#ifdef QT_GUI_LIB
    USBRegistrationWidget* notificationWidget;
#endif
#endif

};

#endif // USBMONITOR_H

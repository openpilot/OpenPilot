/**
 ******************************************************************************
 *
 * @file       pjrc_rawhid.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup RawHIDPlugin Raw HID Plugin
 * @{
 * @brief Impliments a HID USB connection to the flight hardware as a QIODevice
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

#ifndef PJRC_RAWHID_H
#define PJRC_RAWHID_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <QDebug>
#include <QString>
#include "rawhid_global.h"

#if defined( Q_OS_MAC)

// todo:

#elif defined(Q_OS_UNIX)
//#elif defined(Q_OS_LINUX)
#include <usb.h>
#include <QDebug>
#include <QString>
#elif defined(Q_OS_WIN32)
#define _WIN32_WINNT 0x0500
#define _WIN32_WINDOWS 0x0500
#define WINVER 0x0500
#include <windows.h>
#include <dbt.h>
#include <setupapi.h>
#include <ddk/hidsdi.h>
#include <ddk/hidclass.h>
#endif

// ************

#if defined( Q_OS_MAC)

// todo:

#elif defined(Q_OS_UNIX)
//#elif defined(Q_OS_LINUX)

typedef struct hid_struct hid_t;
struct hid_struct
{
    usb_dev_handle *usb;
    int open;
    int iface;
    int ep_in;
    int ep_out;
    struct hid_struct *prev;
    struct hid_struct *next;
};

#elif defined(Q_OS_WIN32)

typedef struct hid_struct hid_t;

struct hid_struct
{
    HANDLE handle;
    int open;
    struct hid_struct *prev;
    struct hid_struct *next;
};

#endif

// ************

//this all stuff was added by ME

struct USBPortInfo {
    QString friendName; ///< Friendly name.
    QString physName;
    QString enumName;   ///< It seems its the only one with meaning
    int vendorID;       ///< Vendor ID.
    int productID;      ///< Product ID
};
#ifdef Q_OS_WIN
#ifdef QT_GUI_LIB
#include <QWidget>
class pjrc_rawhid;

class USBRegistrationWidget : public QWidget
{
    Q_OBJECT
public:
    USBRegistrationWidget( pjrc_rawhid* qese ) {
        this->qese = qese;
    }
    ~USBRegistrationWidget( ) { }

protected:
    pjrc_rawhid* qese;
    bool winEvent( MSG* message, long* result );
};
#endif // QT_GUI_LIB
#endif // Q_OS_WIN

class RAWHID_EXPORT pjrc_rawhid: public QObject
{
    Q_OBJECT
#ifdef Q_OS_WIN
public:
    LRESULT onDeviceChangeWin( WPARAM wParam, LPARAM lParam );
    static QList<USBPortInfo> getPorts();//should exhist for every plattform
private:
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

    static bool getDeviceDetailsWin( USBPortInfo* portInfo, HDEVINFO devInfo,
                                     PSP_DEVINFO_DATA devData, WPARAM wParam = DBT_DEVICEARRIVAL );
    static void enumerateDevicesWin( const GUID & guidDev, QList<USBPortInfo>* infoList );
    bool matchAndDispatchChangedDevice(const QString & deviceID, const GUID & guid, WPARAM wParam);
#ifdef QT_GUI_LIB
    USBRegistrationWidget* notificationWidget;
#endif
#endif /*Q_OS_WIN*/
public:
    pjrc_rawhid();
    ~pjrc_rawhid();
    /*!
      Enable event-driven notifications of board discovery/removal.
    */
    void setUpNotifications( );
    int open(int max, int vid, int pid, int usage_page, int usage);
    int receive(int num, void *buf, int len, int timeout);
    void close(int num);
    int send(int num, void *buf, int len, int timeout);
    QString getserial(int num);
    void mytest(int num);
signals:
     void deviceUnplugged(int);//just to make pips changes compile
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
private:
#if defined( Q_OS_MAC)

    // todo:

#elif defined(Q_OS_UNIX)
    //#elif defined(Q_OS_LINUX)

    hid_t *first_hid;
    hid_t *last_hid;

    void add_hid(hid_t *h);
    hid_t * get_hid(int num);
    void free_all_hid(void);
    void hid_close(hid_t *hid);
    int hid_parse_item(uint32_t *val, uint8_t **data, const uint8_t *end);

#elif defined(Q_OS_WIN32)

    hid_t *first_hid;
    hid_t *last_hid;
    HANDLE rx_event;
    HANDLE tx_event;
    CRITICAL_SECTION rx_mutex;
    CRITICAL_SECTION tx_mutex;

    void add_hid(hid_t *h);
    hid_t * get_hid(int num);
    void free_all_hid(void);
    void hid_close(hid_t *hid);
    void print_win32_err(DWORD err);

#endif
};

#endif

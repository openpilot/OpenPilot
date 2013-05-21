/**
 ******************************************************************************
 *
 * @file       pjrc_rawhid.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2013.
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup opHIDPlugin HID Plugin
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

#ifndef OPHID_HIDAPI_H
#define OPHID_HIDAPI_H
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <QDebug>
#include <QString>
#include <QMutex>
#if defined(Q_OS_UNIX)
#include <usb.h>
#endif
#include "hidapi.h"
#include "opHID_const.h"
#include "opHID_global.h"

struct hidapi_wrapper_device
{
    struct hid_device_info *list_ptr;
    hid_device *handle;
};


class OPHID_EXPORT opHID_hidapi: public QObject
{

     Q_OBJECT

public:

    opHID_hidapi();

    ~opHID_hidapi();

    int enumerate(int *devices_found);

    int open(int max, int vid, int pid, int usage_page, int usage);

    int receive(int, void *buf, int len, int timeout);

    void close(int num);

    int send(int num, void *buf, int len, int timeout);

    QString getserial(int num);

private:
    struct hidapi_wrapper_device hidapi_device;

    /** A mutex to protect hid write */
    QMutex hid_write_Mtx;

    /** A mutex to protect hid read */
    QMutex hid_read_Mtx;

signals:
    void deviceUnplugged(int);
};

#endif

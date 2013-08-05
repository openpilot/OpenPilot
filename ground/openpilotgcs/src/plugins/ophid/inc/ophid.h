/**
 ******************************************************************************
 *
 * @file       ophid.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2013.
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup opHIDPlugin Raw HID Plugin
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

#ifndef OPHID_H
#define OPHID_H

#include "ophid_global.h"

#include <QThread>
#include <QIODevice>
#include <QMutex>
#include <QByteArray>
#include "ophid_hidapi.h"
#include "ophid_usbmon.h"
#include "ophid_read.h"
#include "ophid_write.h"

class opHIDReadWorker;
class opHIDWriteWorker;

/**
 *   The actual IO device that will be used to communicate
 *   with the board/device.
 */
class OPHID_EXPORT opHID : public QIODevice {
    Q_OBJECT

public:
    opHID();
    virtual ~opHID();
    virtual bool open(OpenMode mode);
    virtual void close();
    virtual bool isSequential() const;

    void deviceBind(opHID_hidapi *deviceHandle);
    bool deviceUnbind(void);

    opHID_hidapi *deviceInstanceGet(void);
    void readyRead(void);


signals:
    void closed();
    void deviceAvailable(bool);

protected:
    virtual qint64 readData(char *data, qint64 maxSize);
    virtual qint64 writeData(const char *data, qint64 maxSize);
    virtual qint64 bytesAvailable() const;
    virtual qint64 bytesToWrite() const;

    opHID_hidapi *m_deviceHandle;
    opHIDReadWorker *readWorker;
    opHIDWriteWorker *writeWorker;

    QThread writeThread;
    QThread readThread;

    QMutex *m_mutex;
};

#endif // OPHID_H

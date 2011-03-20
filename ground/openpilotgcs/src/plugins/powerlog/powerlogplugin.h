/**
 ******************************************************************************
 * @file       powerlogplugin.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @see        The GNU Public License (GPL) Version 3
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup powerlogplugin
 * @{
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

#ifndef POWERLOGPLUGIN_H_
#define POWERLOGPLUGIN_H_

#include <coreplugin/icore.h>
#include <coreplugin/coreconstants.h>
#include <coreplugin/actionmanager/actionmanager.h>
#include <coreplugin/iconnection.h>
#include <extensionsystem/iplugin.h>
#include "rawhid/usbmonitor.h"
#include "rawhid/pjrc_rawhid.h"

#include <QThread>
#include <QReadWriteLock>
#include <QFile>

using namespace std;

typedef unsigned long ULONG;    // 4 Bytes
typedef short SHORT;
typedef unsigned short USHORT;  // 2 Bytes
typedef unsigned char BYTE;     // 1 Byte
typedef unsigned short WORD;    // 2 Bytes
typedef unsigned long DWORD;    // 4 Bytes



#define BUF_LEN 64
struct POWERLOG_HID_PACK
{
        BYTE Len;
        BYTE Type;
        DWORD Interval;
        BYTE LogState;
        SHORT Current;
        USHORT Volt;
        DWORD Cap;
        SHORT Cell[6];
        USHORT RPM;
        SHORT Temp[4];
        USHORT Period;
        USHORT Pulse;
};

enum
{
        TYPE_DATA_ONLINE = 0x10,
        TYPE_DATA_OFFLINE = 0x11,
        TYPE_ORDER = 0x20,
};




class PowerlogPlugin;

class PowerlogThread : public QThread
{
    Q_OBJECT

public:
    bool openFile(QString file, PowerlogPlugin * parent);

private slots:

public slots:
    void stopLogging();

protected:
    void run();
    QReadWriteLock lock;
    QFile logFile;
    QTextStream fileStream;

private:
    void ShowInf(char *pBuf);
    void GetShowValue(QString label,DWORD Value,WORD Len,WORD Dot);
};


class PowerlogPlugin : public ExtensionSystem::IPlugin
{
    Q_OBJECT


public:
    PowerlogPlugin();
    ~PowerlogPlugin();

    void extensionsInitialized();
    bool initialize(const QStringList & arguments, QString * errorString);
    void shutdown();

    void setPowerlogMenuTitle(QString str);


signals:
    void stopLoggingSignal(void);

protected:

private slots:
    void receiveLog();
    void devConnected(USBPortInfo);
    void devRemoved(USBPortInfo);

private:
    Core::Command* cmd;
    QString devSerialNumber;
    PowerlogThread* loggingThread;
    bool logging;

};
#endif /* POWERLOGPLUGIN_H_ */
/**
 * @}
 * @}
 */

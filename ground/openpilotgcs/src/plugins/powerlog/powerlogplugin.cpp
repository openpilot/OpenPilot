/**
 ******************************************************************************
 *
 * @file       powerlogplugin.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @see        The GNU Public License (GPL) Version 3
 * @brief      Junsi Powerlog utility Plugin
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup   PowerLog
 * @{
 *
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

#include "powerlogplugin.h"
#include "rawhid/usbmonitor.h"
#include <QDebug>
#include <QtPlugin>
#include <QThread>
#include <QStringList>
#include <QDir>
#include <QFileDialog>
#include <QList>
#include <QErrorMessage>
#include <QWriteLocker>
#include <QDateTime>

#include <extensionsystem/pluginmanager.h>
#include <QKeySequence>



/**
  * Sets the file to use for logging and takes the parent plugin
  * to connect to stop logging signal
  * @param[in] file File name to write to
  * @param[in] parent plugin
  */
bool PowerlogThread::openFile(QString file, PowerlogPlugin * parent)
{
    logFile.setFileName(file);
    logFile.open(QIODevice::WriteOnly);
    fileStream.setDevice(&logFile);

    connect(parent,SIGNAL(stopLoggingSignal()),this,SLOT(stopLogging()));

    return true;
};

/**
  * Get all logs from Powerlog
  */
void PowerlogThread::run()
{


    // TODO: pop up a dialog here!

    qDebug() << "Connect a Junsi PowerLog 6S and watch the logging output";
    pjrc_rawhid hidHandle;
    int numDevices = hidHandle.open(1, 0x0483,0x5750,0,0); //0xff9c,0x0001);
    if( numDevices == 0 )
        numDevices = hidHandle.open(1,0x0483,0,0,0);

    qDebug() << numDevices << " device(s) opened";
    if (numDevices == 0)
        return;

    //hidHandle.mytest(0);

    char buf[BUF_LEN];
    buf[0] = 2;
    buf[1] = 0;

    fileStream << "Interval,Current,Volt,Cap,Cell1,Cell2,Cell3,Cell4,Cell5,Cell6,RPM,Temp0,Temp1,Temp2,Temp3,Period,Pulse\n";

    while (int received = hidHandle.receive(0, buf, BUF_LEN, 3500) ) {
        ShowInf(buf);
        fileStream.flush(); // Just to be sure...
    }

    stopLogging();

}

/**
  * Pass this command to the correct thread then close the file
  */
void PowerlogThread::stopLogging()
{
    QWriteLocker locker(&lock);
    fileStream.flush();
    logFile.close();
    quit();
}

/**
  * Formats the content of the buffer we just read and write
  * to the logfile
  */
void PowerlogThread::ShowInf(char *pBuf)
{
    POWERLOG_HID_PACK Inf;
    int i;
    int Count;

    Count=0;
    Inf.Len = pBuf[Count];
    Count += sizeof(Inf.Len);

    Inf.Type = pBuf[Count];
    Count += sizeof(Inf.Type);

    Inf.Interval = *((DWORD *)&pBuf[Count]);
    fileStream << QString::number(Inf.Interval) << ",";

    Count += sizeof(Inf.Interval);

    Inf.LogState = pBuf[Count];
    Count += sizeof(Inf.LogState);

    if(((Inf.Type == TYPE_DATA_ONLINE)||(Inf.Type == TYPE_DATA_OFFLINE)) && (Inf.Len == 0x29))//0x27
    {
        Inf.Current = *((SHORT *)&pBuf[Count]);
        Count += sizeof(Inf.Current);
        GetShowValue(QString("Current"),Inf.Current,5,2);

        Inf.Volt = *((USHORT *)&pBuf[Count]);
        Count += sizeof(Inf.Volt);
        GetShowValue(QString("Voltage"),Inf.Volt,5,2);

        Inf.Cap = *((DWORD *)&pBuf[Count]);
        Count += sizeof(Inf.Cap);
        GetShowValue(QString("Cap"),Inf.Cap,6,0);

        for(i=0;i<6;i++)
        {
                Inf.Cell[i] = *((SHORT *)&pBuf[Count]);
                Count += sizeof(Inf.Cell[i]);
        }
        GetShowValue(QString("Cell 1"),Inf.Cell[0],5,3);
        GetShowValue(QString("Cell 2"),Inf.Cell[1],5,3);
        GetShowValue(QString("Cell 3"),Inf.Cell[2],5,3);
        GetShowValue(QString("Cell 4"),Inf.Cell[3],5,3);
        GetShowValue(QString("Cell 5"),Inf.Cell[4],5,3);
        GetShowValue(QString("Cell 6"),Inf.Cell[5],5,3);

        Inf.RPM = *((USHORT *)&pBuf[Count]);
        Count += sizeof(Inf.RPM);
        GetShowValue(QString("RPM"),Inf.RPM,6,0);

        for(i=0;i<4;i++)
        {
                Inf.Temp[i] = *((SHORT *)&pBuf[Count]);
                Count += sizeof(Inf.Temp[i]);

        }
        GetShowValue(QString("Int Temp0"),Inf.Temp[0],4,1);

        if (Inf.Temp[1]==0x7fff)
                fileStream << "0.0,";
        else
                GetShowValue(QString("Ext temp1"),Inf.Temp[1],4,1);

        if (Inf.Temp[2]==0x7fff)
            fileStream << "0.0,";
        else
            GetShowValue(QString("Ext temp2"),Inf.Temp[2],4,1);

        if (Inf.Temp[3]==0x7fff)
            fileStream << "0.0,";
        else
            GetShowValue(QString("Ext temp3"),Inf.Temp[3],4,1);

        Inf.Period = *((USHORT *)&pBuf[Count]);
        Count += sizeof(Inf.Period);
        GetShowValue(QString("Period:"),Inf.Period,6,0);

        Inf.Pulse = *((USHORT *)&pBuf[Count]);
        Count += sizeof(Inf.Pulse);
        GetShowValue(QString("Pulse:"),Inf.Pulse,6,0);

        fileStream << "\n";
    }
}

/**
  * Formats a numeric value
  */
void PowerlogThread::GetShowValue(QString label, DWORD Value, WORD Len, WORD Dot)
{
    QString out;

    if (Value < 0) {
        fileStream << "-";
        Value = -Value;
    }

    if(Dot==1)
            fileStream << Value/10 << "." << Value%10;   // printf("%ld.%01lu",Value/10,Value%10);
    else if(Dot==2)
            fileStream << Value/100 << "." << Value%100; // printf("%ld.%02lu",Value/100,Value%100);
    else if(Dot==3)
            fileStream << Value/1000 << "." << Value%1000; // printf("%ld.%03lu",Value/1000,Value%1000);
    else if(Dot==4)
            fileStream << Value/10000 << "." << Value%10000; // printf("%ld.%04lu",Value/10000,Value%10000);
    else
            fileStream << Value; // printf("%ld",Value);

    fileStream << out << ",";

}



/****************************************************************
    Logging plugin
 ********************************/


PowerlogPlugin::PowerlogPlugin() :
    devSerialNumber(""),
    logging(false),
    loggingThread(NULL)
{

}

PowerlogPlugin::~PowerlogPlugin()
{

}

/**
  * Add Powerlog plugin entry to File menu
  */
bool PowerlogPlugin::initialize(const QStringList& args, QString *errMsg)
{
    Q_UNUSED(args);
    Q_UNUSED(errMsg);


    // Add Menu entry
    Core::ActionManager* am = Core::ICore::instance()->actionManager();
    Core::ActionContainer* ac = am->actionContainer(Core::Constants::M_TOOLS);

    // Command to start logging
    cmd = am->registerAction(new QAction(this),
                                            "PowerlogPlugin.Transfer",
                                            QList<int>() <<
                                            Core::Constants::C_GLOBAL_ID);
    cmd->action()->setText("Receive from PowerLog6S...");

    ac->menu()->addSeparator();
    ac->appendGroup("Utilities");
    ac->addAction(cmd, "Utilities");

    connect(cmd->action(), SIGNAL(triggered(bool)), this, SLOT(receiveLog()));

    // At this stage we know that other plugins we depend upon are
    // initialized, in prticular the USB Monitor is now running:
    USBMonitor *mon = USBMonitor::instance();
    connect(mon,SIGNAL(deviceDiscovered(USBPortInfo)), this, SLOT(devConnected(USBPortInfo)));
    connect(mon,SIGNAL(deviceRemoved(USBPortInfo)), this, SLOT(devRemoved(USBPortInfo)));

    return true;
}

/**
  * The action that is triggered by the menu item which opens the
  * file and begins log reception if successful
  */
void PowerlogPlugin::receiveLog()
{
    if (logging) {
        loggingThread->stopLogging();
        logging = false;
        cmd->action()->setText("Receive from PowerLog6S...");
    } else {
        QString fileName = QFileDialog::getSaveFileName(NULL, tr("Log filename"),
                                    tr("PowerLog-%0.csv").arg(QDateTime::currentDateTime().toString("yyyy-MM-dd_hh-mm-ss")),
                                    tr("Comma Separated Values (*.csv)"));
        if (fileName.isEmpty())
            return;

        loggingThread = new PowerlogThread();
        if (loggingThread->openFile(fileName,this)) {
            loggingThread->start();
            cmd->action()->setText("Stop PowerLog6S reception");
            logging = true;
        }
    }

}

/**
  Device connected, check whether it is a powerlog & act accordingly
  */
void PowerlogPlugin::devConnected(USBPortInfo port)
{
    if (devSerialNumber.length() > 0)
        return;
    if ((port.vendorID == 0x0483) && (port.productID==0x5750)) {
        devSerialNumber = port.serialNumber;
        cmd->action()->setEnabled(true);
    }
}


/**
  Device Removed, check whether it is a powerlog & act accordingly.
  As when the device is removed, we don't get the info on the device,
  we have to list all available remaining devices and check if the serial
  number of our device is missing...
  */
void PowerlogPlugin::devRemoved(USBPortInfo port)
{
    bool foundDevice;
    QList<USBPortInfo> ports = USBMonitor::instance()->availableDevices();
    foreach(USBPortInfo port, ports) {
        if ((port.vendorID == 0x0483) && (port.productID==0x5750) &&
                (devSerialNumber == port.serialNumber)) {
            foundDevice = true;
            break;
        }
    }
    if (!foundDevice) {
        devSerialNumber = QString("");
        cmd->action()->setEnabled(false);
        // Also stop logging in case we were logging:
        if (loggingThread)
            loggingThread->stopLogging();
    }
}


void PowerlogPlugin::extensionsInitialized()
{
    cmd->action()->setEnabled(false);
    QList<USBPortInfo> ports = USBMonitor::instance()->availableDevices();
    foreach(USBPortInfo port, ports) {
        if ((port.vendorID == 0x0483) && (port.productID==0x5750)) {
            devSerialNumber = port.serialNumber;
            cmd->action()->setEnabled(true);
            break;
        }
    }
}

void PowerlogPlugin::shutdown()
{
    // Do nothing
}
Q_EXPORT_PLUGIN(PowerlogPlugin)

/**
 * @}
 * @}
 */

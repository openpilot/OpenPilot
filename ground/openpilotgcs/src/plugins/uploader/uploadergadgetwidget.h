/**
 ******************************************************************************
 *
 * @file       uploadergadgetwidget.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup YModemUploader YModem Serial Uploader Plugin
 * @{
 * @brief The YModem protocol serial uploader plugin
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

#ifndef UPLOADERGADGETWIDGET_H
#define UPLOADERGADGETWIDGET_H

#include "ui_uploader.h"
#include "delay.h"
#include "devicewidget.h"
#include "runningdevicewidget.h"
#include "op_dfu.h"
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>

#include "extensionsystem/pluginmanager.h"
#include "uavobjectmanager.h"
#include "uavobject.h"

#include "coreplugin/icore.h"
#include "coreplugin/connectionmanager.h"

#include "ophid/inc/ophid_plugin.h"
#include <QWidget>
#include <QLabel>
#include <QLineEdit>
#include <QThread>
#include <QMessageBox>
#include <QTimer>
#include "devicedescriptorstruct.h"
#include <QProgressDialog>
#include <QErrorMessage>
#include <QDesktopServices>
#include "uploader_global.h"
#include "enums.h"

using namespace OP_DFU;
using namespace uploader;

class FlightStatus;

// A dialog that will time out and close after a given delay
class TimedDialog: public QProgressDialog {
    Q_OBJECT

public:
    TimedDialog(const QString &title, const QString &labelText, int timeout, QWidget *parent = 0, Qt::WindowFlags flags = 0);

    enum DialogCode { Rejected, Accepted, TimedOut };

public slots:
    int exec();

private slots:
    void perform();

private:
    QProgressBar *bar;
};


class UPLOADER_EXPORT UploaderGadgetWidget : public QWidget {
    Q_OBJECT

public:
    UploaderGadgetWidget(QWidget *parent = 0);
    ~UploaderGadgetWidget();
    void log(QString str);
    bool autoUpdateCapable();

public slots:
    void onAutopilotConnect();
    void onAutopilotDisconnect();
    void populate();
    void openHelp();
    bool autoUpdate();
    void autoUpdateProgress(int);

signals:
    void autoUpdateSignal(uploader::AutoUpdateStep, QVariant);

private:
    Ui_UploaderWidget *m_config;
    DFUObject *dfu;
    IAPStep currentStep;
    bool resetOnly;
    void clearLog();
    QString getPortDevice(const QString &friendName);
    QProgressDialog *m_progress;
    QTimer *m_timer;
    QLineEdit *openFileNameLE;
    QEventLoop m_eventloop;
    QErrorMessage *msg;
    void connectSignalSlot(QWidget *widget);
    int autoUpdateConnectTimeout;
    FlightStatus *getFlightStatus();
    void bootButtonsSetEnable(bool enabled);
    static const int AUTOUPDATE_CLOSE_TIMEOUT;
    QTimer autoUpdateCloseTimer;
private slots:
    void onPhisicalHWConnect();
    void versionMatchCheck();
    void error(QString errorString, int errorNumber);
    void info(QString infoString, int infoNumber);
    void goToBootloader(UAVObject * = NULL, bool = false);
    void systemHalt();
    void systemReset();
    void systemBoot();
    void systemSafeBoot();
    void systemEraseBoot();
    void commonSystemBoot(bool safeboot = false, bool erase = false);
    void systemRescue();
    void getSerialPorts();
    void performAuto();
    void uploadStarted();
    void uploadEnded(bool succeed);
    void downloadStarted();
    void downloadEnded(bool succeed);
    void startAutoUpdate();
    void finishAutoUpdate();
    void closeAutoUpdate();
    void autoUpdateStatus(uploader::AutoUpdateStep status, QVariant value);

};

#endif // UPLOADERGADGETWIDGET_H

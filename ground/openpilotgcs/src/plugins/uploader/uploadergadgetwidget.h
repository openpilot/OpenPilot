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
#include "uploader_global.h"

#include "enums.h"
#include "op_dfu.h"

#include <QProgressDialog>

using namespace OP_DFU;
using namespace uploader;

class FlightStatus;
class UAVObject;

class TimedDialog : public QProgressDialog {
    Q_OBJECT

public:
    TimedDialog(const QString &title, const QString &labelText, int timeout, QWidget *parent = 0, Qt::WindowFlags flags = 0);

private slots:
    void perform();

private:
    QProgressBar *bar;
};

// A helper class to wait for board connection and disconnection events
// until a the desired number of connected boards is found
// or until a timeout is reached
class ConnectionWaiter : public QObject {
    Q_OBJECT

public:
    ConnectionWaiter(int targetDeviceCount, int timeout, QWidget *parent = 0);

    enum ResultCode { Ok, Canceled, TimedOut };

public slots:
    int exec();
    void cancel();
    void quit();

    static int openDialog(const QString &title, const QString &labelText, int targetDeviceCount, int timeout, QWidget *parent = 0, Qt::WindowFlags flags = 0);

signals:
    void timeChanged(int elapsed);

private slots:
    void perform();
    void deviceEvent();

private:
    QEventLoop eventLoop;
    QTimer timer;
    // timeout in ms
    int timeout;
    // elapsed time in seconds
    int elapsed;
    int targetDeviceCount;
    int result;
};

class UPLOADER_EXPORT UploaderGadgetWidget : public QWidget {
    Q_OBJECT

public:
    UploaderGadgetWidget(QWidget *parent = 0);
    ~UploaderGadgetWidget();

    static const int BOARD_EVENT_TIMEOUT;
    static const int AUTOUPDATE_CLOSE_TIMEOUT;

    void log(QString str);
    bool autoUpdateCapable();

public slots:
    void onAutopilotConnect();
    void onAutopilotDisconnect();
    void populate();
    void openHelp();
    void autoUpdateDisconnectProgress(int);
    void autoUpdateConnectProgress(int);
    void autoUpdateFlashProgress(int);

signals:
    void autoUpdateSignal(uploader::AutoUpdateStep, QVariant);
    void boardHalted();
    void boardBooted();

private:
    Ui_UploaderWidget *m_config;
    DFUObject *dfu;
    IAPStep currentStep;
    bool resetOnly;

    void clearLog();
    QString getPortDevice(const QString &friendName);
    void connectSignalSlot(QWidget *widget);
    FlightStatus *getFlightStatus();
    void bootButtonsSetEnable(bool enabled);
    int confirmEraseSettingsMessageBox();
    int cannotHaltMessageBox();
    int cannotResetMessageBox();
    void startAutoUpdate(bool erase);

private slots:
    void onPhysicalHWConnect();
    void goToBootloader(UAVObject * = NULL, bool = false);
    void systemHalt();
    void systemReset();
    void systemBoot();
    void systemSafeBoot();
    void systemEraseBoot();
    void commonSystemBoot(bool safeboot = false, bool erase = false);
    void systemRescue();
    void getSerialPorts();
    void uploadStarted();
    void uploadEnded(bool succeed);
    void downloadStarted();
    void downloadEnded(bool succeed);
    void startAutoUpdate();
    void startAutoUpdateErase();
    bool autoUpdate(bool erase);
    void finishAutoUpdate();
    void closeAutoUpdate();
    void autoUpdateStatus(uploader::AutoUpdateStep status, QVariant value);
};

#endif // UPLOADERGADGETWIDGET_H

/**
 ******************************************************************************
 *
 * @file       uploadergadgetwidget.cpp
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
#include "uploadergadgetwidget.h"

#include "flightstatus.h"
#include "oplinkstatus.h"
#include "delay.h"
#include "devicewidget.h"
#include "runningdevicewidget.h"

#include <extensionsystem/pluginmanager.h>
#include <coreplugin/icore.h>
#include <coreplugin/coreconstants.h>
#include <coreplugin/connectionmanager.h>
#include <uavtalk/telemetrymanager.h>

#include <QDesktopServices>
#include <QMessageBox>
#include <QProgressBar>
#include <QDebug>
#include "rebootdialog.h"

#define DFU_DEBUG true

const int UploaderGadgetWidget::BOARD_EVENT_TIMEOUT = 20000;
const int UploaderGadgetWidget::AUTOUPDATE_CLOSE_TIMEOUT = 7000;
const int UploaderGadgetWidget::REBOOT_TIMEOUT     = 20000;
const int UploaderGadgetWidget::ERASE_TIMEOUT      = 20000;
const int UploaderGadgetWidget::BOOTLOADER_TIMEOUT = 20000;

TimedDialog::TimedDialog(const QString &title, const QString &labelText, int timeout, QWidget *parent, Qt::WindowFlags flags) :
    QProgressDialog(labelText, tr("Cancel"), 0, timeout, parent, flags), bar(new QProgressBar(this))
{
    setWindowTitle(title);
    setAutoReset(false);
    // open immediately...
    setMinimumDuration(0);
    // setup progress bar
    bar->setRange(0, timeout);
    bar->setFormat(tr("Timing out in %1 seconds").arg(timeout));
    setBar(bar);
}

void TimedDialog::perform()
{
    setValue(value() + 1);
    int remaining = bar->maximum() - bar->value();
    if (remaining > 0) {
        bar->setFormat(tr("Timing out in %1 seconds").arg(remaining));
    } else {
        bar->setFormat(tr("Timed out after %1 seconds").arg(bar->maximum()));
    }
}

ConnectionWaiter::ConnectionWaiter(int targetDeviceCount, int timeout, QWidget *parent) : QObject(parent), eventLoop(this), timer(this), timeout(timeout), elapsed(0), targetDeviceCount(targetDeviceCount), result(ConnectionWaiter::Ok)
{}

int ConnectionWaiter::exec()
{
    connect(USBMonitor::instance(), SIGNAL(deviceDiscovered(USBPortInfo)), this, SLOT(deviceEvent()));
    connect(USBMonitor::instance(), SIGNAL(deviceRemoved(USBPortInfo)), this, SLOT(deviceEvent()));

    connect(&timer, SIGNAL(timeout()), this, SLOT(perform()));
    timer.start(1000);

    emit timeChanged(0);
    eventLoop.exec();

    return result;
}

void ConnectionWaiter::cancel()
{
    quit();
    result = ConnectionWaiter::Canceled;
}

void ConnectionWaiter::quit()
{
    disconnect(USBMonitor::instance(), SIGNAL(deviceDiscovered(USBPortInfo)), this, SLOT(deviceEvent()));
    disconnect(USBMonitor::instance(), SIGNAL(deviceRemoved(USBPortInfo)), this, SLOT(deviceEvent()));
    timer.stop();
    eventLoop.exit();
}

void ConnectionWaiter::perform()
{
    ++elapsed;
    emit timeChanged(elapsed);
    int remaining = timeout - elapsed * 1000;
    if (remaining <= 0) {
        result = ConnectionWaiter::TimedOut;
        quit();
    }
}

void ConnectionWaiter::deviceEvent()
{
    if (USBMonitor::instance()->availableDevices(0x20a0, -1, -1, -1).length() == targetDeviceCount) {
        quit();
    }
}

int ConnectionWaiter::openDialog(const QString &title, const QString &labelText, int targetDeviceCount, int timeout, QWidget *parent, Qt::WindowFlags flags)
{
    TimedDialog dlg(title, labelText, timeout / 1000, parent, flags);
    ConnectionWaiter waiter(targetDeviceCount, timeout, parent);

    connect(&dlg, SIGNAL(canceled()), &waiter, SLOT(cancel()));
    connect(&waiter, SIGNAL(timeChanged(int)), &dlg, SLOT(perform()));
    return waiter.exec();
}

UploaderGadgetWidget::UploaderGadgetWidget(QWidget *parent) : QWidget(parent)
{
    m_config    = new Ui_UploaderWidget();
    m_config->setupUi(this);
    m_currentIAPStep = IAP_STATE_READY;
    m_resetOnly = false;
    m_dfu = NULL;
    m_autoUpdateClosing = false;

    // Listen to autopilot connection events
    ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
    TelemetryManager *telMngr = pm->getObject<TelemetryManager>();
    connect(telMngr, SIGNAL(connected()), this, SLOT(onAutopilotConnect()));
    connect(telMngr, SIGNAL(disconnected()), this, SLOT(onAutopilotDisconnect()));

    Core::ConnectionManager *cm = Core::ICore::instance()->connectionManager();
    connect(cm, SIGNAL(deviceConnected(QIODevice *)), this, SLOT(onPhysicalHWConnect()));

    connect(m_config->haltButton, SIGNAL(clicked()), this, SLOT(systemHalt()));
    connect(m_config->resetButton, SIGNAL(clicked()), this, SLOT(systemReset()));
    connect(m_config->bootButton, SIGNAL(clicked()), this, SLOT(systemBoot()));
    connect(m_config->safeBootButton, SIGNAL(clicked()), this, SLOT(systemSafeBoot()));
    connect(m_config->eraseBootButton, SIGNAL(clicked()), this, SLOT(systemEraseBoot()));
    connect(m_config->rescueButton, SIGNAL(clicked()), this, SLOT(systemRescue()));

    getSerialPorts();

    connect(m_config->autoUpdateButton, SIGNAL(clicked()), this, SLOT(startAutoUpdate()));
    connect(m_config->autoUpdateEraseButton, SIGNAL(clicked()), this, SLOT(startAutoUpdateErase()));
    connect(m_config->autoUpdateOkButton, SIGNAL(clicked()), this, SLOT(closeAutoUpdate()));
    m_config->autoUpdateButton->setEnabled(autoUpdateCapable());
    m_config->autoUpdateEraseButton->setEnabled(autoUpdateCapable());
    m_config->autoUpdateGroupBox->setVisible(false);

    m_config->refreshPorts->setIcon(QIcon(":uploader/images/view-refresh.svg"));

    bootButtonsSetEnable(false);

    connect(m_config->refreshPorts, SIGNAL(clicked()), this, SLOT(getSerialPorts()));

    connect(m_config->pbHelp, SIGNAL(clicked()), this, SLOT(openHelp()));
    // And check whether by any chance we are not already connected
    if (telMngr->isConnected()) {
        onAutopilotConnect();
    }
}

bool sortPorts(const QSerialPortInfo &s1, const QSerialPortInfo &s2)
{
    return s1.portName() < s2.portName();
}

/**
   Gets the list of serial ports
 */
void UploaderGadgetWidget::getSerialPorts()
{
    QStringList list;

    // Populate the telemetry combo box:
    m_config->telemetryLink->clear();

    list.append(QString("USB"));
    QList<QSerialPortInfo> ports = QSerialPortInfo::availablePorts();

    // sort the list by port number (nice idea from PT_Dreamer :))
    qSort(ports.begin(), ports.end(), sortPorts);
    foreach(QSerialPortInfo port, ports) {
        list.append(port.portName());
    }

    m_config->telemetryLink->addItems(list);
}


QString UploaderGadgetWidget::getPortDevice(const QString &friendName)
{
    QList<QSerialPortInfo> ports = QSerialPortInfo::availablePorts();
    foreach(QSerialPortInfo port, ports) {
        if (port.portName() == friendName) {
            return port.portName();
        }
    }
    return "";
}

void UploaderGadgetWidget::connectSignalSlot(QWidget *widget)
{
    connect(qobject_cast<DeviceWidget *>(widget), SIGNAL(uploadStarted()), this, SLOT(uploadStarted()));
    connect(qobject_cast<DeviceWidget *>(widget), SIGNAL(uploadEnded(bool)), this, SLOT(uploadEnded(bool)));
    connect(qobject_cast<DeviceWidget *>(widget), SIGNAL(downloadStarted()), this, SLOT(downloadStarted()));
    connect(qobject_cast<DeviceWidget *>(widget), SIGNAL(downloadEnded(bool)), this, SLOT(downloadEnded(bool)));
}

FlightStatus *UploaderGadgetWidget::getFlightStatus()
{
    ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();

    Q_ASSERT(pm);
    UAVObjectManager *objManager = pm->getObject<UAVObjectManager>();
    Q_ASSERT(objManager);
    FlightStatus *status = dynamic_cast<FlightStatus *>(objManager->getObject("FlightStatus"));
    Q_ASSERT(status);
    return status;
}

void UploaderGadgetWidget::bootButtonsSetEnable(bool enabled)
{
    m_config->bootButton->setEnabled(enabled);
    m_config->safeBootButton->setEnabled(enabled);

    // this feature is supported only on BL revision >= 4
    bool isBL4 = ((m_dfu != NULL) && (m_dfu->devices[0].BL_Version >= 4));
    m_config->eraseBootButton->setEnabled(isBL4 && enabled);
}

void UploaderGadgetWidget::onPhysicalHWConnect()
{
    bootButtonsSetEnable(false);
    m_config->rescueButton->setEnabled(false);
    m_config->telemetryLink->setEnabled(false);
}

/**
   Enables widget buttons if autopilot connected
 */
void UploaderGadgetWidget::onAutopilotConnect()
{
    QTimer::singleShot(1000, this, SLOT(populate()));
}

void UploaderGadgetWidget::populate()
{
    m_config->haltButton->setEnabled(true);
    m_config->resetButton->setEnabled(true);
    bootButtonsSetEnable(false);
    m_config->rescueButton->setEnabled(false);
    m_config->telemetryLink->setEnabled(false);

    // Add a very simple widget with Board model & serial number
    // Delete all previous tabs:
    while (m_config->systemElements->count()) {
        QWidget *qw = m_config->systemElements->widget(0);
        m_config->systemElements->removeTab(0);
        delete qw;
    }
    RunningDeviceWidget *dw = new RunningDeviceWidget(this);
    dw->populate();
    m_config->systemElements->addTab(dw, tr("Connected Device"));
}

/**
   Enables widget buttons if autopilot disconnected
 */
void UploaderGadgetWidget::onAutopilotDisconnect()
{
    m_config->haltButton->setEnabled(false);
    m_config->resetButton->setEnabled(false);
    bootButtonsSetEnable(true);
    if (m_currentIAPStep == IAP_STATE_BOOTLOADER) {
        m_config->rescueButton->setEnabled(false);
        m_config->telemetryLink->setEnabled(false);
    } else {
        m_config->rescueButton->setEnabled(true);
        m_config->telemetryLink->setEnabled(true);
    }
}

static void sleep(int ms)
{
    QEventLoop eventLoop;
    QTimer::singleShot(ms, &eventLoop, SLOT(quit()));

    eventLoop.exec();
}

/**
   Tell the mainboard to go to bootloader:
   - Send the relevant IAP commands
   - setup callback for MoBo acknowledge
 */
void UploaderGadgetWidget::goToBootloader(UAVObject *callerObj, bool success)
{
    Q_UNUSED(callerObj);

    ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
    UAVObjectManager *objManager = pm->getObject<UAVObjectManager>();
    UAVObject *fwIAP = dynamic_cast<UAVDataObject *>(objManager->getObject("FirmwareIAPObj"));

    switch (m_currentIAPStep) {
    case IAP_STATE_READY:
        m_config->haltButton->setEnabled(false);
        getSerialPorts(); // Useful in case a new serial port appeared since the initial list,
                          // otherwise we won't find it when we stop the board.
        // The board is running, send the 1st IAP Reset order:
        fwIAP->getField("Command")->setValue("1122");
        fwIAP->getField("BoardRevision")->setDouble(0);
        fwIAP->getField("BoardType")->setDouble(0);
        connect(fwIAP, SIGNAL(transactionCompleted(UAVObject *, bool)), this, SLOT(goToBootloader(UAVObject *, bool)));
        m_currentIAPStep = IAP_STATE_STEP_1;
        clearLog();
        log("IAP Step 1");
        fwIAP->updated();
        emit progressUpdate(JUMP_TO_BL, QVariant(1));
        break;
    case IAP_STATE_STEP_1:
        if (!success) {
            log("Oops, failure step 1");
            log("Reset did NOT happen");
            m_currentIAPStep = IAP_STATE_READY;
            disconnect(fwIAP, SIGNAL(transactionCompleted(UAVObject *, bool)), this, SLOT(goToBootloader(UAVObject *, bool)));
            m_config->haltButton->setEnabled(true);
            emit progressUpdate(FAILURE, QVariant());
            emit bootloaderFailed();
            break;
        }
        sleep(600);
        fwIAP->getField("Command")->setValue("2233");
        m_currentIAPStep = IAP_STATE_STEP_2;
        log("IAP Step 2");
        fwIAP->updated();
        emit progressUpdate(JUMP_TO_BL, QVariant(2));
        break;
    case IAP_STATE_STEP_2:
        if (!success) {
            log("Oops, failure step 2");
            log("Reset did NOT happen");
            m_currentIAPStep = IAP_STATE_READY;
            disconnect(fwIAP, SIGNAL(transactionCompleted(UAVObject *, bool)), this, SLOT(goToBootloader(UAVObject *, bool)));
            m_config->haltButton->setEnabled(true);
            emit progressUpdate(FAILURE, QVariant());
            emit bootloaderFailed();
            break;
        }
        sleep(600);
        fwIAP->getField("Command")->setValue("3344");
        m_currentIAPStep = IAP_STEP_RESET;
        log("IAP Step 3");
        emit progressUpdate(JUMP_TO_BL, QVariant(3));
        fwIAP->updated();
        break;
    case IAP_STEP_RESET:
    {
        m_currentIAPStep = IAP_STATE_READY;
        if (!success) {
            log("Oops, failure step 3");
            log("Reset did NOT happen");
            disconnect(fwIAP, SIGNAL(transactionCompleted(UAVObject *, bool)), this, SLOT(goToBootloader(UAVObject *, bool)));
            m_config->haltButton->setEnabled(true);
            emit progressUpdate(FAILURE, QVariant());
            emit bootloaderFailed();
            break;
        }

        // The board is now reset: we have to disconnect telemetry
        Core::ConnectionManager *cm = Core::ICore::instance()->connectionManager();
        QString dli = cm->getCurrentDevice().getConName();
        QString dlj = cm->getCurrentDevice().getConName();
        cm->disconnectDevice();
        sleep(200);
        // Tell connections to stop their polling threads: otherwise it will mess up DFU
        cm->suspendPolling();
        sleep(200);
        log("Board Halt");
        emit progressUpdate(JUMP_TO_BL, QVariant(4));
        m_config->boardStatus->setText(tr("Bootloader"));
        if (dlj.startsWith("USB")) {
            m_config->telemetryLink->setCurrentIndex(m_config->telemetryLink->findText("USB"));
        } else {
            m_config->telemetryLink->setCurrentIndex(m_config->telemetryLink->findText(dli));
        }

        disconnect(fwIAP, SIGNAL(transactionCompleted(UAVObject *, bool)), this, SLOT(goToBootloader(UAVObject *, bool)));

        m_currentIAPStep = IAP_STATE_BOOTLOADER;

        // Tell the mainboard to get into bootloader state:
        log("Detecting devices, please wait a few seconds...");
        if (!m_dfu) {
            if (dlj.startsWith("USB")) {
                m_dfu = new DFUObject(DFU_DEBUG, false, QString());
            } else {
                m_dfu = new DFUObject(DFU_DEBUG, true, getPortDevice(dli));
            }
        }
        if (!m_dfu->ready()) {
            log("Could not enter DFU mode.");
            delete m_dfu;
            m_dfu = NULL;
            cm->resumePolling();
            m_currentIAPStep = IAP_STATE_READY;
            m_config->boardStatus->setText(tr("Bootloader?"));
            m_config->haltButton->setEnabled(true);
            emit progressUpdate(FAILURE, QVariant());
            emit bootloaderFailed();
            return;
        }
        m_dfu->AbortOperation();
        if (!m_dfu->enterDFU(0)) {
            log("Could not enter DFU mode.");
            delete m_dfu;
            m_dfu = NULL;
            cm->resumePolling();
            m_currentIAPStep = IAP_STATE_READY;
            m_config->boardStatus->setText(tr("Bootloader?"));
            emit progressUpdate(FAILURE, QVariant());
            emit bootloaderFailed();
            return;
        }

        sleep(500);
        m_dfu->findDevices();
        log(QString("Found %1 device(s).").arg(QString::number(m_dfu->numberOfDevices)));
        if (m_dfu->numberOfDevices < 0 || m_dfu->numberOfDevices > 3) {
            log("Inconsistent number of devices! Aborting");
            delete m_dfu;
            m_dfu = NULL;
            cm->resumePolling();
            emit progressUpdate(FAILURE, QVariant());
            emit bootloaderFailed();
            return;
        }
        // Delete all previous tabs:
        while (m_config->systemElements->count()) {
            QWidget *qw = m_config->systemElements->widget(0);
            m_config->systemElements->removeTab(0);
            delete qw;
        }
        for (int i = 0; i < m_dfu->numberOfDevices; i++) {
            DeviceWidget *dw = new DeviceWidget(this);
            connectSignalSlot(dw);
            dw->setDeviceID(i);
            dw->setDfu(m_dfu);
            dw->populate();
            m_config->systemElements->addTab(dw, tr("Device") + QString::number(i));
        }

        QApplication::processEvents();

        // Need to re-enable in case we were not connected
        bootButtonsSetEnable(true);
        emit progressUpdate(JUMP_TO_BL, QVariant(5));
        emit bootloaderSuccess();

        if (m_resetOnly) {
            m_resetOnly = false;
            delay::msleep(3500);
            systemBoot();
            break;
        }
    }
    break;
    case IAP_STATE_BOOTLOADER:
        // We should never end up here anyway.
        emit progressUpdate(FAILURE, QVariant());
        emit bootloaderFailed();
        break;
    }
}

void UploaderGadgetWidget::systemHalt()
{
    // The board can not be halted when in armed state.
    // If board is armed, or arming. Show message with notice.
    FlightStatus *status = getFlightStatus();

    if (status->getArmed() == FlightStatus::ARMED_DISARMED) {
        goToBootloader();
    } else {
        cannotHaltMessageBox();
    }
}

/**
   Tell the mainboard to reset:
   - Send the relevant IAP commands
   - setup callback for MoBo acknowledge
 */
void UploaderGadgetWidget::systemReset()
{
    FlightStatus *status = getFlightStatus();

    // The board can not be reset when in armed state.
    // If board is armed, or arming. Show message with notice.
    if (status->getArmed() == FlightStatus::ARMED_DISARMED) {
        m_resetOnly = true;
        if (m_dfu) {
            delete m_dfu;
            m_dfu = NULL;
        }
        clearLog();
        log("Board Reset initiated.");
        goToBootloader();
    } else {
        cannotResetMessageBox();
    }
}

void UploaderGadgetWidget::systemBoot()
{
    commonSystemBoot(false, false);
}

void UploaderGadgetWidget::systemSafeBoot()
{
    commonSystemBoot(true, false);
}

void UploaderGadgetWidget::systemEraseBoot()
{
    switch (confirmEraseSettingsMessageBox()) {
    case QMessageBox::Ok:
        commonSystemBoot(true, true);
        break;
    case QMessageBox::Help:
        QDesktopServices::openUrl(QUrl(tr("http://wiki.openpilot.org/display/Doc/Erase+board+settings"), QUrl::StrictMode));
        break;
    }
}

void UploaderGadgetWidget::rebootWithDialog()
{
    RebootDialog dialog(this);

    dialog.exec();
}

void UploaderGadgetWidget::systemReboot()
{
    ResultEventLoop eventLoop;

    connect(this, SIGNAL(bootloaderSuccess()), &eventLoop, SLOT(success()));
    connect(this, SIGNAL(bootloaderFailed()), &eventLoop, SLOT(fail()));

    goToBootloader();

    if (eventLoop.run(REBOOT_TIMEOUT) != 0) {
        emit progressUpdate(FAILURE, QVariant());
        return;
    }

    disconnect(this, SIGNAL(bootloaderSuccess()), &eventLoop, SLOT(success()));
    disconnect(this, SIGNAL(bootloaderFailed()), &eventLoop, SLOT(fail()));

    commonSystemBoot(false, false);

    ExtensionSystem::PluginManager *pluginManager = ExtensionSystem::PluginManager::instance();
    Q_ASSERT(pluginManager);
    TelemetryManager *telemetryManager = pluginManager->getObject<TelemetryManager>();
    Q_ASSERT(telemetryManager);

    if (!telemetryManager->isConnected()) {
        progressUpdate(BOOTING, QVariant());
        ResultEventLoop eventLoop;

        connect(telemetryManager, SIGNAL(connected()), &eventLoop, SLOT(success()));

        if (eventLoop.run(REBOOT_TIMEOUT) != 0) {
            emit progressUpdate(FAILURE, QVariant());
            return;
        }

        disconnect(telemetryManager, SIGNAL(connected()), &eventLoop, SLOT(success()));
    }

    emit progressUpdate(SUCCESS, QVariant());
}

/**
 * Tells the system to boot (from Bootloader state)
 * @param[in] safeboot Indicates whether the firmware should use the stock HWSettings
 */
void UploaderGadgetWidget::commonSystemBoot(bool safeboot, bool erase)
{
    clearLog();
    bootButtonsSetEnable(false);
    // Suspend telemety & polling in case it is not done yet
    Core::ConnectionManager *cm = Core::ICore::instance()->connectionManager();
    cm->disconnectDevice();
    cm->suspendPolling();

    QString devName = m_config->telemetryLink->currentText();
    log("Attempting to boot the system through " + devName + ".");
    repaint();

    if (!m_dfu) {
        if (devName == "USB") {
            m_dfu = new DFUObject(DFU_DEBUG, false, QString());
        } else {
            m_dfu = new DFUObject(DFU_DEBUG, true, getPortDevice(devName));
        }
    }
    m_dfu->AbortOperation();
    if (!m_dfu->enterDFU(0)) {
        log("Could not enter DFU mode.");
        delete m_dfu;
        m_dfu = NULL;
        bootButtonsSetEnable(true);
        m_config->rescueButton->setEnabled(true); // Boot not possible, maybe Rescue OK?
        emit bootFailed();
        return;
    }
    log("Booting system...");
    m_dfu->JumpToApp(safeboot, erase);
    // Restart the polling thread
    cm->resumePolling();
    m_config->rescueButton->setEnabled(true);
    m_config->telemetryLink->setEnabled(true);
    m_config->boardStatus->setText(tr("Running"));
    if (m_currentIAPStep == IAP_STATE_BOOTLOADER) {
        // Freeze the tabs, they are not useful anymore and their buttons
        // will cause segfaults or weird stuff if we use them.
        for (int i = 0; i < m_config->systemElements->count(); i++) {
            // OP-682 arriving here too "early" (before the devices are refreshed) was leading to a crash
            // OP-682 the crash was due to an unchecked cast in the line below that would cast a RunningDeviceGadget to a DeviceGadget
            DeviceWidget *qw = dynamic_cast<DeviceWidget *>(m_config->systemElements->widget(i));
            if (qw) {
                // OP-682 fixed a second crash by disabling *all* buttons in the device widget
                // disabling the buttons is only half of the solution as even if the buttons are enabled
                // the app should not crash
                qw->freeze();
            }
        }
    }
    m_currentIAPStep = IAP_STATE_READY;
    log("You can now reconnect telemetry...");
    delete m_dfu; // Frees up the USB/Serial port too
    emit bootSuccess();
    m_dfu = NULL;
}

bool UploaderGadgetWidget::autoUpdateCapable()
{
    return QDir(":/firmware").exists();
}

bool UploaderGadgetWidget::autoUpdate(bool erase)
{
    if (m_oplinkwatchdog.isConnected() &&
        m_oplinkwatchdog.opLinkType() == OPLinkWatchdog::OPLINK_MINI) {
        emit progressUpdate(FAILURE, QVariant(tr("To upgrade the OPLinkMini board please disconnect it from the USB port, "
                                                 "press the Upgrade again button and follow instructions on screen.")));
        emit autoUpdateFailed();
        return false;
    }

    ExtensionSystem::PluginManager *pluginManager = ExtensionSystem::PluginManager::instance();
    Q_ASSERT(pluginManager);
    TelemetryManager *telemetryManager = pluginManager->getObject<TelemetryManager>();
    Q_ASSERT(telemetryManager);

    if (USBMonitor::instance()->availableDevices(0x20a0, -1, -1, -1).length() > 0 &&
        telemetryManager->connectionState() != TelemetryManager::TELEMETRY_CONNECTED) {
        // Wait for the board to completely connect
        ResultEventLoop eventLoop;
        connect(telemetryManager, SIGNAL(connected()), &eventLoop, SLOT(success()));

        if (telemetryManager->connectionState() != TelemetryManager::TELEMETRY_CONNECTED
            && eventLoop.run(REBOOT_TIMEOUT) != 0) {
            emit progressUpdate(FAILURE, QVariant(tr("Timed out while waiting for a board to be fully connected!")));
            emit autoUpdateFailed();
            return false;
        }

        disconnect(telemetryManager, SIGNAL(connected()), &eventLoop, SLOT(success()));
    }
    if (USBMonitor::instance()->availableDevices(0x20a0, -1, -1, -1).length() == 0) {
        ConnectionWaiter waiter(1, BOARD_EVENT_TIMEOUT);
        connect(&waiter, SIGNAL(timeChanged(int)), this, SLOT(autoUpdateConnectProgress(int)));
        if (waiter.exec() == ConnectionWaiter::TimedOut) {
            emit progressUpdate(FAILURE, QVariant(tr("Timed out while waiting for a board to be connected!")));
            emit autoUpdateFailed();
            return false;
        }
    } else {
        ResultEventLoop eventLoop;
        connect(this, SIGNAL(bootloaderSuccess()), &eventLoop, SLOT(success()));
        connect(this, SIGNAL(bootloaderFailed()), &eventLoop, SLOT(fail()));

        goToBootloader();

        if (eventLoop.run(BOOTLOADER_TIMEOUT) != 0) {
            emit progressUpdate(FAILURE, QVariant(tr("Failed to enter bootloader mode.")));
            emit autoUpdateFailed();
            return false;
        }

        disconnect(this, SIGNAL(bootloaderSuccess()), &eventLoop, SLOT(success()));
        disconnect(this, SIGNAL(bootloaderFailed()), &eventLoop, SLOT(fail()));
    }

    if (m_dfu) {
        delete m_dfu;
        m_dfu = NULL;
    }

    Core::ConnectionManager *connectionManager = Core::ICore::instance()->connectionManager();
    m_dfu = new DFUObject(DFU_DEBUG, false, QString());
    m_dfu->AbortOperation();
    emit progressUpdate(JUMP_TO_BL, QVariant());

    if (!m_dfu->enterDFU(0) || !m_dfu->findDevices() ||
        (m_dfu->numberOfDevices != 1) || m_dfu->numberOfDevices > 5) {
        delete m_dfu;
        m_dfu = NULL;
        connectionManager->resumePolling();
        emit progressUpdate(FAILURE, QVariant(tr("Failed to enter bootloader mode.")));
        emit autoUpdateFailed();
        return false;
    }

    QString filename;
    emit progressUpdate(LOADING_FW, QVariant());
    switch (m_dfu->devices[0].ID) {
    case 0x301:
        filename = "fw_oplinkmini";
        break;
    case 0x902:
        filename = "fw_revoproto";
        break;
    case 0x903:
        filename = "fw_revolution";
        break;
    case 0x904:
        filename = "fw_discoveryf4bare";
        break;
    default:
        emit progressUpdate(FAILURE, QVariant(tr("Unknown board id '0x%1'").arg(QString::number(m_dfu->devices[0].ID, 16))));
        emit autoUpdateFailed();
        return false;
    }
    filename = ":/firmware/" + filename + ".opfw";
    QByteArray firmware;
    if (!QFile::exists(filename)) {
        emit progressUpdate(FAILURE, QVariant(tr("Firmware image not found.")));
        emit autoUpdateFailed();
        return false;
    }
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly)) {
        emit progressUpdate(FAILURE, QVariant(tr("Could not open firmware image for reading.")));
        emit autoUpdateFailed();
        return false;
    }
    firmware = file.readAll();
    QEventLoop eventLoop2;
    connect(m_dfu, SIGNAL(progressUpdated(int)), this, SLOT(autoUpdateFlashProgress(int)));
    connect(m_dfu, SIGNAL(uploadFinished(OP_DFU::Status)), &eventLoop2, SLOT(quit()));
    emit progressUpdate(UPLOADING_FW, QVariant());
    if (!m_dfu->enterDFU(0)) {
        emit progressUpdate(FAILURE, QVariant(tr("Could not enter direct firmware upload mode.")));
        emit autoUpdateFailed();
        return false;
    }
    m_dfu->AbortOperation();
    if (!m_dfu->UploadFirmware(filename, false, 0)) {
        emit progressUpdate(FAILURE, QVariant(tr("Firmware upload failed.")));
        emit autoUpdateFailed();
        return false;
    }
    eventLoop2.exec();
    QByteArray desc = firmware.right(100);
    emit progressUpdate(UPLOADING_DESC, QVariant());
    if (m_dfu->UploadDescription(desc) != OP_DFU::Last_operation_Success) {
        emit progressUpdate(FAILURE, QVariant(tr("Failed to upload firmware description.")));
        emit autoUpdateFailed();
        return false;
    }

    commonSystemBoot(false, erase);

    // Wait for board to connect to GCS again after boot and erase
    // Theres a special case with OPLink
    if (!telemetryManager->isConnected() && !m_oplinkwatchdog.isConnected()) {
        progressUpdate(erase ? BOOTING_AND_ERASING : BOOTING, QVariant());
        ResultEventLoop eventLoop;
        connect(telemetryManager, SIGNAL(connected()), &eventLoop, SLOT(success()));
        connect(&m_oplinkwatchdog, SIGNAL(opLinkMiniConnected()), &eventLoop, SLOT(success()));

        if (eventLoop.run(REBOOT_TIMEOUT + (erase ? ERASE_TIMEOUT : 0)) != 0) {
            emit progressUpdate(FAILURE, QVariant(tr("Timed out while booting.")));
            emit autoUpdateFailed();
            return false;
        }

        disconnect(&m_oplinkwatchdog, SIGNAL(opLinkMiniConnected()), &eventLoop, SLOT(success()));
        disconnect(telemetryManager, SIGNAL(connected()), &eventLoop, SLOT(success()));
    }

    emit progressUpdate(SUCCESS, QVariant());
    emit autoUpdateSuccess();
    return true;
}

void UploaderGadgetWidget::autoUpdateDisconnectProgress(int value)
{
    emit progressUpdate(WAITING_DISCONNECT, value);
}

void UploaderGadgetWidget::autoUpdateConnectProgress(int value)
{
    emit progressUpdate(WAITING_CONNECT, value);
}

void UploaderGadgetWidget::autoUpdateFlashProgress(int value)
{
    emit progressUpdate(UPLOADING_FW, value);
}

/**
   Attempt a guided procedure to put both boards in BL mode when
   the system is not bootable
 */
void UploaderGadgetWidget::systemRescue()
{
    Core::ConnectionManager *cm = Core::ICore::instance()->connectionManager();

    cm->disconnectDevice();
    // stop the polling thread: otherwise it will mess up DFU
    cm->suspendPolling();

    // Delete all previous tabs:
    while (m_config->systemElements->count()) {
        QWidget *qw = m_config->systemElements->widget(0);
        m_config->systemElements->removeTab(0);
        delete qw;
    }

    // Existing DFU objects will have a handle over USB and will
    // disturb everything for the rescue process:
    if (m_dfu) {
        delete m_dfu;
        m_dfu = NULL;
    }

    // Avoid users pressing Rescue twice.
    m_config->rescueButton->setEnabled(false);

    // Now we're good to go
    clearLog();
    log("**********************************************************");
    log("** Follow those instructions to attempt a system rescue **");
    log("**********************************************************");
    log("You will be prompted to first connect USB, then system power");

    // Check if boards are connected and, if yes, prompt user to disconnect them all
    if (USBMonitor::instance()->availableDevices(0x20a0, -1, -1, -1).length() > 0) {
        QString labelText = QString("<p align=\"left\">%1</p>").arg(tr("Please disconnect your OpenPilot board."));
        int result = ConnectionWaiter::openDialog(tr("System Rescue"), labelText, 0, BOARD_EVENT_TIMEOUT, this);
        switch (result) {
        case ConnectionWaiter::Canceled:
            m_config->rescueButton->setEnabled(true);
            return;

        case ConnectionWaiter::TimedOut:
            QMessageBox::warning(this, tr("System Rescue"), tr("Timed out while waiting for all boards to be disconnected!"));
            m_config->rescueButton->setEnabled(true);
            return;
        }
    }

    // Now prompt user to connect board
    QString labelText = QString("<p align=\"left\">%1<br>%2</p>").arg(tr("Please connect your OpenPilot board.")).arg(tr("Board must be connected to the USB port!"));
    int result = ConnectionWaiter::openDialog(tr("System Rescue"), labelText, 1, BOARD_EVENT_TIMEOUT, this);
    switch (result) {
    case ConnectionWaiter::Canceled:
        m_config->rescueButton->setEnabled(true);
        return;

    case ConnectionWaiter::TimedOut:
        QMessageBox::warning(this, tr("System Rescue"), tr("Timed out while waiting for a board to be connected!"));
        m_config->rescueButton->setEnabled(true);
        return;
    }

    log("Detecting first board...");
    m_dfu = new DFUObject(DFU_DEBUG, false, QString());
    m_dfu->AbortOperation();
    if (!m_dfu->enterDFU(0)) {
        log("Could not enter DFU mode.");
        delete m_dfu;
        m_dfu = NULL;
        cm->resumePolling();
        m_config->rescueButton->setEnabled(true);
        return;
    }
    if (!m_dfu->findDevices() || (m_dfu->numberOfDevices != 1)) {
        log("Could not detect a board, aborting!");
        delete m_dfu;
        m_dfu = NULL;
        cm->resumePolling();
        m_config->rescueButton->setEnabled(true);
        return;
    }
    log(QString("Found %1 device(s).").arg(m_dfu->numberOfDevices));

    if (m_dfu->numberOfDevices > 5) {
        log("Inconsistent number of devices, aborting!");
        delete m_dfu;
        m_dfu = NULL;
        cm->resumePolling();
        m_config->rescueButton->setEnabled(true);
        return;
    }
    for (int i = 0; i < m_dfu->numberOfDevices; i++) {
        DeviceWidget *dw = new DeviceWidget(this);
        connectSignalSlot(dw);
        dw->setDeviceID(i);
        dw->setDfu(m_dfu);
        dw->populate();
        m_config->systemElements->addTab(dw, tr("Device") + QString::number(i));
    }
    m_config->haltButton->setEnabled(false);
    m_config->resetButton->setEnabled(false);
    bootButtonsSetEnable(true);
    m_config->rescueButton->setEnabled(false);

    // So that we can boot from the GUI afterwards.
    m_currentIAPStep = IAP_STATE_BOOTLOADER;
}

void UploaderGadgetWidget::uploadStarted()
{
    m_config->haltButton->setEnabled(false);
    bootButtonsSetEnable(false);
    m_config->resetButton->setEnabled(false);
    m_config->rescueButton->setEnabled(false);
}

void UploaderGadgetWidget::uploadEnded(bool succeed)
{
    Q_UNUSED(succeed);
    // device is halted so no halt
    m_config->haltButton->setEnabled(false);
    bootButtonsSetEnable(true);
    // device is halted so no reset
    m_config->resetButton->setEnabled(false);
    m_config->rescueButton->setEnabled(true);
}

void UploaderGadgetWidget::downloadStarted()
{
    m_config->haltButton->setEnabled(false);
    bootButtonsSetEnable(false);
    m_config->resetButton->setEnabled(false);
    m_config->rescueButton->setEnabled(false);
}

void UploaderGadgetWidget::downloadEnded(bool succeed)
{
    Q_UNUSED(succeed);
    // device is halted so no halt
    m_config->haltButton->setEnabled(false);
    bootButtonsSetEnable(true);
    // device is halted so no reset
    m_config->resetButton->setEnabled(false);
    m_config->rescueButton->setEnabled(true);
}

void UploaderGadgetWidget::startAutoUpdate()
{
    startAutoUpdate(false);
}

void UploaderGadgetWidget::startAutoUpdateErase()
{
    startAutoUpdate(true);
}

void UploaderGadgetWidget::startAutoUpdate(bool erase)
{
    m_config->autoUpdateProgressBar->setValue(0);
    autoUpdateStatus(uploader::JUMP_TO_BL, QVariant());
    m_config->buttonFrame->setEnabled(false);
    m_config->splitter->setEnabled(false);
    m_config->autoUpdateGroupBox->setVisible(true);
    m_config->autoUpdateOkButton->setEnabled(false);

    connect(this, SIGNAL(progressUpdate(uploader::ProgressStep, QVariant)), this, SLOT(autoUpdateStatus(uploader::ProgressStep, QVariant)));
    autoUpdate(erase);
}

void UploaderGadgetWidget::finishAutoUpdate()
{
    disconnect(this, SIGNAL(progressUpdate(uploader::ProgressStep, QVariant)), this, SLOT(autoUpdateStatus(uploader::ProgressStep, QVariant)));
    m_config->autoUpdateOkButton->setEnabled(true);
    m_autoUpdateClosing = true;

    // wait a bit and "close" auto update
    QTimer::singleShot(AUTOUPDATE_CLOSE_TIMEOUT, this, SLOT(closeAutoUpdate()));
}

void UploaderGadgetWidget::closeAutoUpdate()
{
    if (m_autoUpdateClosing) {
        m_config->autoUpdateGroupBox->setVisible(false);
        m_config->buttonFrame->setEnabled(true);
        m_config->splitter->setEnabled(true);
    }
    m_autoUpdateClosing = false;
}

void UploaderGadgetWidget::autoUpdateStatus(uploader::ProgressStep status, QVariant value)
{
    QString msg;
    int remaining;

    switch (status) {
    case uploader::WAITING_DISCONNECT:
        m_config->autoUpdateLabel->setText(tr("Waiting for all OpenPilot boards to be disconnected from USB."));
        m_config->autoUpdateProgressBar->setMaximum(BOARD_EVENT_TIMEOUT / 1000);
        m_config->autoUpdateProgressBar->setValue(value.toInt());
        remaining = m_config->autoUpdateProgressBar->maximum() - m_config->autoUpdateProgressBar->value();
        m_config->autoUpdateProgressBar->setFormat(tr("Timing out in %1 seconds").arg(remaining));
        break;
    case uploader::WAITING_CONNECT:
        m_config->autoUpdateLabel->setText(tr("Please connect the OpenPilot board to the USB port."));
        m_config->autoUpdateProgressBar->setMaximum(BOARD_EVENT_TIMEOUT / 1000);
        m_config->autoUpdateProgressBar->setValue(value.toInt());
        remaining = m_config->autoUpdateProgressBar->maximum() - m_config->autoUpdateProgressBar->value();
        m_config->autoUpdateProgressBar->setFormat(tr("Timing out in %1 seconds").arg(remaining));
        break;
    case uploader::JUMP_TO_BL:
        m_config->autoUpdateLabel->setText(tr("Bringing the board into boot loader mode. Please wait."));
        m_config->autoUpdateProgressBar->setFormat(tr("Step %1").arg(value.toInt()));
        m_config->autoUpdateProgressBar->setMaximum(5);
        m_config->autoUpdateProgressBar->setValue(value.toInt());
        break;
    case uploader::LOADING_FW:
        m_config->autoUpdateLabel->setText(tr("Preparing to upload firmware to the board."));
        break;
    case uploader::UPLOADING_FW:
        m_config->autoUpdateLabel->setText(tr("Uploading firmware to the board."));
        m_config->autoUpdateProgressBar->setFormat("%p%");
        m_config->autoUpdateProgressBar->setMaximum(100);
        m_config->autoUpdateProgressBar->setValue(value.toInt());
        break;
    case uploader::UPLOADING_DESC:
        m_config->autoUpdateLabel->setText(tr("Uploading description of the new firmware to the board."));
        break;
    case uploader::BOOTING:
        m_config->autoUpdateLabel->setText(tr("Rebooting the board. Please wait."));
        break;
    case uploader::BOOTING_AND_ERASING:
        m_config->autoUpdateLabel->setText(tr("Rebooting and erasing the board. Please wait."));
        break;
    case uploader::SUCCESS:
        m_config->autoUpdateProgressBar->setValue(m_config->autoUpdateProgressBar->maximum());
        msg = tr("Board was updated successfully. Press OK to finish.");
        m_config->autoUpdateLabel->setText(QString("<font color='green'>%1</font>").arg(msg));
        finishAutoUpdate();
        break;
    case uploader::FAILURE:
        msg = value.toString();
        if (msg.isEmpty()) {
            msg = tr("Something went wrong.");
        }
        msg += tr(" Press OK to finish, you will have to manually upgrade the board.");

        m_config->autoUpdateLabel->setText(QString("<font color='red'>%1</font>").arg(msg));
        finishAutoUpdate();
        break;
    }
}

/**
   Update log entry
 */
void UploaderGadgetWidget::log(QString str)
{
    qDebug() << "UploaderGadgetWidget -" << str;
    m_config->textBrowser->append(str);
}

void UploaderGadgetWidget::clearLog()
{
    m_config->textBrowser->clear();
}

/**
 * Remove all the device widgets...
 */
UploaderGadgetWidget::~UploaderGadgetWidget()
{
    while (m_config->systemElements->count()) {
        QWidget *qw = m_config->systemElements->widget(0);
        m_config->systemElements->removeTab(0);
        delete qw;
        qw = 0;
    }
}

void UploaderGadgetWidget::openHelp()
{
    QDesktopServices::openUrl(QUrl(tr("http://wiki.openpilot.org/x/AoBZ"), QUrl::StrictMode));
}

int UploaderGadgetWidget::confirmEraseSettingsMessageBox()
{
    QMessageBox msgBox(this);

    msgBox.setWindowTitle(tr("Confirm Settings Erase?"));
    msgBox.setIcon(QMessageBox::Question);
    msgBox.setText(tr("Do you want to erase all settings from the board?"));
    msgBox.setInformativeText(tr("Settings cannot be recovered after this operation.\nThe board will be restarted and all settings erased."));
    msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel | QMessageBox::Help);
    return msgBox.exec();
}

int UploaderGadgetWidget::cannotHaltMessageBox()
{
    QMessageBox msgBox(this);

    msgBox.setWindowTitle(tr("Cannot Halt Board!"));
    msgBox.setIcon(QMessageBox::Warning);
    msgBox.setText(tr("The controller board is armed and can not be halted."));
    msgBox.setInformativeText(tr("Please make sure the board is not armed and then press Halt again to proceed or use Rescue to force a firmware upgrade."));
    msgBox.setStandardButtons(QMessageBox::Ok);
    return msgBox.exec();
}

int UploaderGadgetWidget::cannotResetMessageBox()
{
    QMessageBox msgBox(this);

    msgBox.setWindowTitle(tr("Cannot Reset Board!"));
    msgBox.setIcon(QMessageBox::Warning);
    msgBox.setText(tr("The controller board is armed and can not be reset."));
    msgBox.setInformativeText(tr("Please make sure the board is not armed and then press reset again to proceed or power cycle to force a board reset."));
    msgBox.setStandardButtons(QMessageBox::Ok);
    return msgBox.exec();
}


int ResultEventLoop::run(int millisTimout)
{
    m_timer.singleShot(millisTimout, this, SLOT(fail()));
    return exec();
}

void ResultEventLoop::success()
{
    m_timer.stop();
    exit(0);
}

void ResultEventLoop::fail()
{
    m_timer.stop();
    exit(-1);
}

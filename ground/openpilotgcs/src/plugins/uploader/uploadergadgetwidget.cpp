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
#include "../../../../../build/ground/openpilotgcs/gcsversioninfo.h"
#include <coreplugin/coreconstants.h>
#include <QDebug>
#include "flightstatus.h"

#define DFU_DEBUG true

UploaderGadgetWidget::UploaderGadgetWidget(QWidget *parent) : QWidget(parent)
{
    m_config = new Ui_UploaderWidget();
    m_config->setupUi(this);
    currentStep = IAP_STATE_READY;
    resetOnly=false;
    dfu = NULL;
    m_timer = 0;
    m_progress = 0;
    msg=new QErrorMessage(this);
    // Listen to autopilot connection events
    ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
    TelemetryManager* telMngr = pm->getObject<TelemetryManager>();
    connect(telMngr, SIGNAL(connected()), this, SLOT(onAutopilotConnect()));
    connect(telMngr, SIGNAL(connected()), this, SLOT(versionMatchCheck()));

    connect(telMngr, SIGNAL(disconnected()), this, SLOT(onAutopilotDisconnect()));

    connect(m_config->haltButton, SIGNAL(clicked()), this, SLOT(systemHalt()));
    connect(m_config->resetButton, SIGNAL(clicked()), this, SLOT(systemReset()));
    connect(m_config->bootButton, SIGNAL(clicked()), this, SLOT(systemBoot()));
    connect(m_config->safeBootButton, SIGNAL(clicked()), this, SLOT(systemSafeBoot()));
    connect(m_config->rescueButton, SIGNAL(clicked()), this, SLOT(systemRescue()));
    Core::ConnectionManager *cm = Core::ICore::instance()->connectionManager();
    connect(cm,SIGNAL(deviceConnected(QIODevice*)),this,SLOT(onPhisicalHWConnect()));
    getSerialPorts();

    QIcon rbi;
    rbi.addFile(QString(":uploader/images/view-refresh.svg"));
    m_config->refreshPorts->setIcon(rbi);

    connect(m_config->refreshPorts, SIGNAL(clicked()), this, SLOT(getSerialPorts()));

    connect(m_config->pbHelp,SIGNAL(clicked()),this,SLOT(openHelp()));
    // And check whether by any chance we are not already connected
    if (telMngr->isConnected())
    {
        onAutopilotConnect();
        versionMatchCheck();
    }
}


bool sortPorts(const QextPortInfo &s1,const QextPortInfo &s2)
{
    return s1.portName<s2.portName;
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
    QList<QextPortInfo> ports = QextSerialEnumerator::getPorts();

    //sort the list by port number (nice idea from PT_Dreamer :))
    qSort(ports.begin(), ports.end(),sortPorts);
    foreach( QextPortInfo port, ports ) {
       list.append(port.friendName);
    }

    m_config->telemetryLink->addItems(list);
}


QString UploaderGadgetWidget::getPortDevice(const QString &friendName)
{
    QList<QextPortInfo> ports = QextSerialEnumerator::getPorts();
    foreach( QextPortInfo port, ports ) {
           if(port.friendName == friendName)
#ifdef Q_OS_WIN
            return port.portName;
#else
            return port.physName;
#endif
        }
    return "";
}

void UploaderGadgetWidget::connectSignalSlot(QWidget *widget)
{
    connect(qobject_cast<deviceWidget *>(widget),SIGNAL(uploadStarted()),this,SLOT(uploadStarted()));
    connect(qobject_cast<deviceWidget *>(widget),SIGNAL(uploadEnded(bool)),this,SLOT(uploadEnded(bool)));
}

FlightStatus *UploaderGadgetWidget::getFlightStatus()
{
    ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
    Q_ASSERT(pm);
    UAVObjectManager *objManager = pm->getObject<UAVObjectManager>();
    Q_ASSERT(objManager);
    FlightStatus *status = dynamic_cast<FlightStatus*>(objManager->getObject(QString("FlightStatus")));
    Q_ASSERT(status);
    return status;
}

void UploaderGadgetWidget::onPhisicalHWConnect()
{
    m_config->bootButton->setEnabled(false);
    m_config->safeBootButton->setEnabled(false);
    m_config->rescueButton->setEnabled(false);
    m_config->telemetryLink->setEnabled(false);
}

/**
  Enables widget buttons if autopilot connected
  */
void UploaderGadgetWidget::onAutopilotConnect(){
    QTimer::singleShot(1000,this,SLOT(populate()));
}

void UploaderGadgetWidget::populate()
{
    m_config->haltButton->setEnabled(true);
    m_config->resetButton->setEnabled(true);
    m_config->safeBootButton->setEnabled(false);
    m_config->bootButton->setEnabled(false);
    m_config->rescueButton->setEnabled(false);
    m_config->telemetryLink->setEnabled(false);

    // Add a very simple widget with Board model & serial number
    // Delete all previous tabs:
    while (m_config->systemElements->count()) {
         QWidget *qw = m_config->systemElements->widget(0);
         m_config->systemElements->removeTab(0);
         delete qw;
    }
    runningDeviceWidget* dw = new runningDeviceWidget(this);
    dw->populate();
    m_config->systemElements->addTab(dw, QString("Connected Device"));
}

/**
  Enables widget buttons if autopilot disconnected
  */
void UploaderGadgetWidget::onAutopilotDisconnect(){
    m_config->haltButton->setEnabled(false);
    m_config->resetButton->setEnabled(false);
    m_config->bootButton->setEnabled(true);
    m_config->safeBootButton->setEnabled(true);
    if (currentStep == IAP_STATE_BOOTLOADER) {
        m_config->rescueButton->setEnabled(false);
        m_config->telemetryLink->setEnabled(false);
    } else {
        m_config->rescueButton->setEnabled(true);
        m_config->telemetryLink->setEnabled(true);
    }
}


/**
  Tell the mainboard to go to bootloader:
   - Send the relevant IAP commands
   - setup callback for MoBo acknowledge
   */
void UploaderGadgetWidget::goToBootloader(UAVObject* callerObj, bool success)
{
    Q_UNUSED(callerObj);

    ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
    UAVObjectManager *objManager = pm->getObject<UAVObjectManager>();
    UAVObject *fwIAP = dynamic_cast<UAVDataObject*>(objManager->getObject(QString("FirmwareIAPObj")));

    switch (currentStep) {
    case IAP_STATE_READY:
        m_config->haltButton->setEnabled(false);
        getSerialPorts(); // Useful in case a new serial port appeared since the initial list,
                          // otherwise we won't find it when we stop the board.
        // The board is running, send the 1st IAP Reset order:
        fwIAP->getField("Command")->setValue("1122");
        fwIAP->getField("BoardRevision")->setDouble(0);
        fwIAP->getField("BoardType")->setDouble(0);
        connect(fwIAP,SIGNAL(transactionCompleted(UAVObject*,bool)),this,SLOT(goToBootloader(UAVObject*, bool)));
        currentStep = IAP_STATE_STEP_1;
        clearLog();
        log(QString("IAP Step 1"));
        fwIAP->updated();
        break;
    case IAP_STATE_STEP_1:
        if (!success)  {
            log(QString("Oops, failure step 1"));
            log("Reset did NOT happen");
            currentStep = IAP_STATE_READY;
            disconnect(fwIAP, SIGNAL(transactionCompleted(UAVObject*,bool)),this,SLOT(goToBootloader(UAVObject*, bool)));
            m_config->haltButton->setEnabled(true);
            break;
        }
        QTimer::singleShot(600, &m_eventloop, SLOT(quit()));
        m_eventloop.exec();
        fwIAP->getField("Command")->setValue("2233");
        currentStep = IAP_STATE_STEP_2;
        log(QString("IAP Step 2"));
        fwIAP->updated();
        break;
    case IAP_STATE_STEP_2:
        if (!success) {
            log(QString("Oops, failure step 2"));
            log("Reset did NOT happen");
            currentStep = IAP_STATE_READY;
            disconnect(fwIAP, SIGNAL(transactionCompleted(UAVObject*,bool)),this,SLOT(goToBootloader(UAVObject*, bool)));
            m_config->haltButton->setEnabled(true);
            break;
        }
        QTimer::singleShot(600, &m_eventloop, SLOT(quit()));
        m_eventloop.exec();
        fwIAP->getField("Command")->setValue("3344");
        currentStep = IAP_STEP_RESET;
        log(QString("IAP Step 3"));
        fwIAP->updated();
        break;
    case IAP_STEP_RESET:
    {
        currentStep = IAP_STATE_READY;
        if (!success) {
            log("Oops, failure step 3");
            log("Reset did NOT happen");
            disconnect(fwIAP, SIGNAL(transactionCompleted(UAVObject*,bool)),this,SLOT(goToBootloader(UAVObject*, bool)));
            m_config->haltButton->setEnabled(true);
            break;
        }

        // The board is now reset: we have to disconnect telemetry
        Core::ConnectionManager *cm = Core::ICore::instance()->connectionManager();
        QString dli = cm->getCurrentDevice().getConName();
        QString dlj = cm->getCurrentDevice().getConName();
        cm->disconnectDevice();
        QTimer::singleShot(200, &m_eventloop, SLOT(quit()));
        m_eventloop.exec();
        // Tell connections to stop their polling threads: otherwise it will mess up DFU
        cm->suspendPolling();
        QTimer::singleShot(200, &m_eventloop, SLOT(quit()));
        m_eventloop.exec();
        log("Board Halt");
        m_config->boardStatus->setText("Bootloader");
        if (dlj.startsWith("USB"))
            m_config->telemetryLink->setCurrentIndex(m_config->telemetryLink->findText("USB"));
        else
            m_config->telemetryLink->setCurrentIndex(m_config->telemetryLink->findText(dli));

        disconnect(fwIAP, SIGNAL(transactionCompleted(UAVObject*,bool)),this,SLOT(goToBootloader(UAVObject*, bool)));

        currentStep = IAP_STATE_BOOTLOADER;

        // Tell the mainboard to get into bootloader state:
        log("Detecting devices, please wait a few seconds...");
        if (!dfu) {
            if (dlj.startsWith("USB"))
                dfu = new DFUObject(DFU_DEBUG, false, QString());
            else
                dfu = new DFUObject(DFU_DEBUG,true, getPortDevice(dli));
        }
        if (!dfu->ready())
        {
            log("Could not enter DFU mode.");
            delete dfu;
            dfu = NULL;
            cm->resumePolling();
            currentStep = IAP_STATE_READY;
            m_config->boardStatus->setText("Bootloader?");
            m_config->haltButton->setEnabled(true);
            return;
        }
        dfu->AbortOperation();
        if(!dfu->enterDFU(0))
        {
            log("Could not enter DFU mode.");
            delete dfu;
            dfu = NULL;
            cm->resumePolling();
            currentStep = IAP_STATE_READY;
            m_config->boardStatus->setText("Bootloader?");
            return;
        }
        //dfu.StatusRequest();
        dfu->findDevices();
        log(QString("Found ") + QString::number(dfu->numberOfDevices) + QString(" device(s)."));
        if (dfu->numberOfDevices < 0 || dfu->numberOfDevices > 3) {
            log("Inconsistent number of devices! Aborting");
            delete dfu;
            dfu = NULL;
            cm->resumePolling();
            return;
        }
        // Delete all previous tabs:
        while (m_config->systemElements->count()) {
             QWidget *qw = m_config->systemElements->widget(0);
             m_config->systemElements->removeTab(0);
             delete qw;
        }
        for(int i=0;i<dfu->numberOfDevices;i++) {
            deviceWidget* dw = new deviceWidget(this);
            connectSignalSlot(dw);
            dw->setDeviceID(i);
            dw->setDfu(dfu);
            dw->populate();
            m_config->systemElements->addTab(dw, QString("Device") + QString::number(i));
        }
        /*
        m_config->haltButton->setEnabled(false);
        m_config->resetButton->setEnabled(false);
        */
        // Need to re-enable in case we were not connected
        m_config->bootButton->setEnabled(true);
        m_config->safeBootButton->setEnabled(true);
        /*
        m_config->telemetryLink->setEnabled(false);
        m_config->rescueButton->setEnabled(false);
        */
        if (resetOnly) {
            resetOnly=false;
            delay::msleep(3500);
            systemBoot();
            break;
        }

    }
        break;
    case IAP_STATE_BOOTLOADER:
        // We should never end up here anyway.
        break;
    }

}

void UploaderGadgetWidget::systemHalt()
{
    FlightStatus* status = getFlightStatus();

    // The board can not be halted when in armed state.
    // If board is armed, or arming. Show message with notice.
    if (status->getArmed() == FlightStatus::ARMED_DISARMED) {
        goToBootloader();
    }
    else {
        QMessageBox mbox(this);
        mbox.setText(QString(tr("The controller board is armed and can not be halted.\n\n"
                                "Please make sure the board is not armed and then press halt again to proceed\n"
                                "or use the rescue option to force a firmware upgrade.")));
        mbox.setStandardButtons(QMessageBox::Ok);
        mbox.setIcon(QMessageBox::Warning);
        mbox.exec();
    }
}

/**
  Tell the mainboard to reset:
   - Send the relevant IAP commands
   - setup callback for MoBo acknowledge
   */
void UploaderGadgetWidget::systemReset()
{
    FlightStatus* status = getFlightStatus();

    // The board can not be reset when in armed state.
    // If board is armed, or arming. Show message with notice.
    if (status->getArmed() == FlightStatus::ARMED_DISARMED) {
        resetOnly = true;
        if (dfu) {
            delete dfu;
            dfu = NULL;
        }
        m_config->textBrowser->clear();
        log("Board Reset initiated.");
        goToBootloader();
    }
    else {
        QMessageBox mbox(this);
        mbox.setText(QString(tr("The controller board is armed and can not be reset.\n\n"
                                "Please make sure the board is not armed and then press reset again to proceed\n"
                                "or power cycle to force a board reset.")));
        mbox.setStandardButtons(QMessageBox::Ok);
        mbox.setIcon(QMessageBox::Warning);
        mbox.exec();
    }
}

void UploaderGadgetWidget::systemBoot()
{
  commonSystemBoot(false);
}

void UploaderGadgetWidget::systemSafeBoot()
{
  commonSystemBoot(true);
}

/**
  * Tells the system to boot (from Bootloader state)
  * @param[in] safeboot Indicates whether the firmware should use the stock HWSettings
  */
void UploaderGadgetWidget::commonSystemBoot(bool safeboot)
{
    clearLog();
    m_config->bootButton->setEnabled(false);
    m_config->safeBootButton->setEnabled(false);

    // Suspend telemety & polling in case it is not done yet
    Core::ConnectionManager *cm = Core::ICore::instance()->connectionManager();
    cm->disconnectDevice();
    cm->suspendPolling();

    QString devName = m_config->telemetryLink->currentText();
    log("Attempting to boot the system through " + devName + ".");
    repaint();

    if (!dfu) {
        if (devName == "USB")
            dfu = new DFUObject(DFU_DEBUG, false, QString());
        else
            dfu = new DFUObject(DFU_DEBUG,true,getPortDevice(devName));
    }
    dfu->AbortOperation();
    if(!dfu->enterDFU(0))
    {
        log("Could not enter DFU mode.");
        delete dfu;
        dfu = NULL;
        m_config->bootButton->setEnabled(true);
        m_config->safeBootButton->setEnabled(true);
        m_config->rescueButton->setEnabled(true); // Boot not possible, maybe Rescue OK?
        return;
    }
    log("Booting system...");
    dfu->JumpToApp(safeboot);
    // Restart the polling thread
    cm->resumePolling();
    m_config->rescueButton->setEnabled(true);
    m_config->telemetryLink->setEnabled(true);
    m_config->boardStatus->setText("Running");
    if (currentStep == IAP_STATE_BOOTLOADER ) {
        // Freeze the tabs, they are not useful anymore and their buttons
        // will cause segfaults or weird stuff if we use them.
        for (int i=0; i< m_config->systemElements->count(); i++) {
             deviceWidget *qw = (deviceWidget*)m_config->systemElements->widget(i);
             qw->freeze();
        }
    }
    currentStep = IAP_STATE_READY;
    log("You can now reconnect telemetry...");
    delete dfu; // Frees up the USB/Serial port too
    dfu = NULL;
}
bool UploaderGadgetWidget::autoUpdateCapable()
{
    return QDir(":/build").exists();
}

bool UploaderGadgetWidget::autoUpdate()
{
    Core::ConnectionManager *cm = Core::ICore::instance()->connectionManager();
    cm->disconnectDevice();
    cm->suspendPolling();
    if (dfu) {
        delete dfu;
        dfu = NULL;
    }
    QEventLoop loop;
    QTimer timer;
    timer.setSingleShot(true);
    connect(&timer,SIGNAL(timeout()),&loop,SLOT(quit()));
    while(USBMonitor::instance()->availableDevices(0x20a0,-1,-1,-1).length()>0)
    {
             emit autoUpdateSignal(WAITING_DISCONNECT,QVariant());
             if(QMessageBox::warning(this,tr("OpenPilot Uploader"),tr("Please disconnect all openpilot boards"),QMessageBox::Ok,QMessageBox::Cancel)==QMessageBox::Cancel)
             {
                     emit autoUpdateSignal(FAILURE,QVariant());
                     return false;
             }
             timer.start(500);
             loop.exec();
    }
    emit autoUpdateSignal(WAITING_CONNECT,0);
    autoUpdateConnectTimeout=0;
    m_timer = new QTimer(this);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(performAuto()));
    m_timer->start(1000);
    connect(USBMonitor::instance(), SIGNAL(deviceDiscovered(USBPortInfo)),&m_eventloop, SLOT(quit()));
    m_eventloop.exec();
    if(!m_timer->isActive())
    {
        m_timer->stop();
        emit autoUpdateSignal(FAILURE,QVariant());
        return false;
    }
    m_timer->stop();
    dfu = new DFUObject(DFU_DEBUG, false, QString());
    dfu->AbortOperation();
    emit autoUpdateSignal(JUMP_TO_BL,QVariant());
    if(!dfu->enterDFU(0))
    {
        delete dfu;
        dfu = NULL;
        cm->resumePolling();
        emit autoUpdateSignal(FAILURE,QVariant());
        return false;
    }
    if(!dfu->findDevices() || (dfu->numberOfDevices != 1))
    {
        delete dfu;
        dfu = NULL;
        cm->resumePolling();
        emit autoUpdateSignal(FAILURE,QVariant());
        return false;
    }
    if (dfu->numberOfDevices > 5) {
        delete dfu;
        dfu = NULL;
        cm->resumePolling();
        emit autoUpdateSignal(FAILURE,QVariant());
        return false;
    }
    QString filename;
    emit autoUpdateSignal(LOADING_FW,QVariant());
    switch (dfu->devices[0].ID)
    {
    case 0x401:
        filename="fw_coptercontrol";
        break;
    case 0x402:
        filename="fw_coptercontrol";
        break;
    default:
        emit autoUpdateSignal(FAILURE,QVariant());
        return false;
        break;
    }
    filename=":/build/"+filename+"/"+filename+".opfw";
    QByteArray firmware;
    if(!QFile::exists(filename))
    {
        emit autoUpdateSignal(FAILURE,QVariant());
        return false;
    }
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly)) {
        emit autoUpdateSignal(FAILURE,QVariant());
        return false;
    }
    firmware = file.readAll();
    connect(dfu, SIGNAL(progressUpdated(int)), this, SLOT(autoUpdateProgress(int)));
    connect(dfu, SIGNAL(uploadFinished(OP_DFU::Status)), &m_eventloop, SLOT(quit()));
    emit autoUpdateSignal(UPLOADING_FW,QVariant());
    if(!dfu->enterDFU(0))
    {
        emit autoUpdateSignal(FAILURE,QVariant());
        return false;
    }
    dfu->AbortOperation();
    if(!dfu->UploadFirmware(filename,false,0))
    {
        emit autoUpdateSignal(FAILURE,QVariant());
        return false;
    }
    m_eventloop.exec();
    QByteArray desc = firmware.right(100);
    emit autoUpdateSignal(UPLOADING_DESC,QVariant());
    if(dfu->UploadDescription(desc)!= OP_DFU::Last_operation_Success)
    {
        emit autoUpdateSignal(FAILURE,QVariant());
        return false;
    }
    systemBoot();
    emit autoUpdateSignal(SUCCESS,QVariant());
    return true;
}

void UploaderGadgetWidget::autoUpdateProgress(int value)
{
    emit autoUpdateSignal(UPLOADING_FW,value);
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
    if (dfu) {
        delete dfu;
        dfu = NULL;
    }
    // Avoid users pressing Rescue twice.
    m_config->rescueButton->setEnabled(false);

    // Now we're good to go:
    clearLog();
    log("**********************************************************");
    log("** Follow those instructions to attempt a system rescue **");
    log("**********************************************************");
    log("You will be prompted to first connect USB, then system power");
    if(USBMonitor::instance()->availableDevices(0x20a0,-1,-1,-1).length()>0)
    {
        if(QMessageBox::warning(this,tr("OpenPilot Uploader"),tr("Please disconnect all openpilot boards"),QMessageBox::Ok,QMessageBox::Cancel)==QMessageBox::Cancel)
        {
            m_config->rescueButton->setEnabled(true);
            return;
        }
    }
    // Now we're good to go:
    clearLog();
    log("**********************************************************");
    log("** Follow those instructions to attempt a system rescue **");
    log("**********************************************************");
    log("You will be prompted to first connect USB, then system power");
    m_progress = new QProgressDialog(tr("Please connect the board (USB only!)"), tr("Cancel"), 0, 20);
    QProgressBar * bar=new QProgressBar(m_progress);
    bar->setFormat("Timeout");
    m_progress->setBar(bar);
    m_progress->setMinimumDuration(0);
    m_progress->setRange(0,20);
    connect(m_progress, SIGNAL(canceled()), this, SLOT(cancel()));
    m_timer = new QTimer(this);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(perform()));
    m_timer->start(1000);
    connect(USBMonitor::instance(), SIGNAL(deviceDiscovered(USBPortInfo)),&m_eventloop, SLOT(quit()));
    m_eventloop.exec();
    if(!m_timer->isActive())
    {
        m_progress->close();
        m_timer->stop();
        QMessageBox::warning(this,tr("Openpilot Uploader"),tr("No board connection was detected!"));
        m_config->rescueButton->setEnabled(true);
        return;
    }
    m_progress->close();
    m_timer->stop();
    log("... Detecting First Board...");
    repaint();
    dfu = new DFUObject(DFU_DEBUG, false, QString());
    dfu->AbortOperation();
    if(!dfu->enterDFU(0))
    {
        log("Could not enter DFU mode.");
        delete dfu;
        dfu = NULL;
        cm->resumePolling();
        m_config->rescueButton->setEnabled(true);
        return;
    }
    if(!dfu->findDevices() || (dfu->numberOfDevices != 1))
    {
        log("Could not detect a board, aborting!");
        delete dfu;
        dfu = NULL;
        cm->resumePolling();
        m_config->rescueButton->setEnabled(true);
        return;
    }
    log(QString("Found ") + QString::number(dfu->numberOfDevices) + QString(" device(s)."));
    if (dfu->numberOfDevices > 5) {
        log("Inconsistent number of devices, aborting!");
        delete dfu;
        dfu = NULL;
        cm->resumePolling();
        m_config->rescueButton->setEnabled(true);
        return;
    }
    for(int i=0;i<dfu->numberOfDevices;i++) {
        deviceWidget* dw = new deviceWidget(this);
        connectSignalSlot(dw);
        dw->setDeviceID(i);
        dw->setDfu(dfu);
        dw->populate();
        m_config->systemElements->addTab(dw, QString("Device") + QString::number(i));
    }
    m_config->haltButton->setEnabled(false);
    m_config->resetButton->setEnabled(false);
    m_config->bootButton->setEnabled(true);
    m_config->safeBootButton->setEnabled(true);
    m_config->rescueButton->setEnabled(false);
    currentStep = IAP_STATE_BOOTLOADER; // So that we can boot from the GUI afterwards.
}

void UploaderGadgetWidget::perform()
{
    if(m_progress->value()==19)
    {
        m_timer->stop();
        m_eventloop.exit();
    }
    m_progress->setValue(m_progress->value()+1);
}
void UploaderGadgetWidget::performAuto()
{
    ++autoUpdateConnectTimeout;
    emit autoUpdateSignal(WAITING_CONNECT,autoUpdateConnectTimeout*5);
    if(autoUpdateConnectTimeout==20)
    {
        m_timer->stop();
        m_eventloop.exit();
    }

}
void UploaderGadgetWidget::cancel()
{
    m_timer->stop();
    m_eventloop.exit();
}

void UploaderGadgetWidget::uploadStarted()
{
    m_config->bootButton->setEnabled(false);
    m_config->safeBootButton->setEnabled(false);
}

void UploaderGadgetWidget::uploadEnded(bool succeed)
{
    Q_UNUSED(succeed);
    m_config->bootButton->setEnabled(true);
    m_config->safeBootButton->setEnabled(true);
}

/**
  Update log entry
  */
void UploaderGadgetWidget::log(QString str)
{
   m_config->textBrowser->append(str);
   m_config->textBrowser->repaint();

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
    if (m_progress) {
        delete m_progress;
        m_progress = 0;
    }
    if (m_timer) {
        delete m_timer;
        m_timer = 0;
    }
}


/**
Shows a message box with an error string.

@param errorString	The error string to display.

@param errorNumber      Not used

*/
void UploaderGadgetWidget::error(QString errorString, int errorNumber)
{
    Q_UNUSED(errorNumber);
    QMessageBox msgBox;
    msgBox.setIcon(QMessageBox::Critical);
    msgBox.setText(errorString);
    msgBox.exec();
    m_config->boardStatus->setText(errorString);
}
/**
Shows a message box with an information string.

@param infoString	The information string to display.

@param infoNumber       Not used

*/
void UploaderGadgetWidget::info(QString infoString, int infoNumber)
{
    Q_UNUSED(infoNumber);
    m_config->boardStatus->setText(infoString);
}

void UploaderGadgetWidget::versionMatchCheck()
{
    ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
    UAVObjectUtilManager *utilMngr = pm->getObject<UAVObjectUtilManager>();
    deviceDescriptorStruct boardDescription = utilMngr->getBoardDescriptionStruct();
    QByteArray uavoHashArray;
    QString uavoHash = QString::fromLatin1(Core::Constants::UAVOSHA1_STR);
    uavoHash.chop(2);
    uavoHash.remove(0,2);
    uavoHash=uavoHash.trimmed();
    bool ok;
    foreach(QString str,uavoHash.split(","))
    {
        uavoHashArray.append(str.toInt(&ok,16));
    }

    QByteArray fwVersion=boardDescription.uavoHash;
    if (fwVersion != uavoHashArray) {

        QString gcsDescription = QString::fromLatin1(Core::Constants::GCS_REVISION_STR);
        QString gcsGitHash = gcsDescription.mid(gcsDescription.indexOf(":")+1, 8);
        gcsGitHash.remove( QRegExp("^[0]*") );
        QString gcsGitDate = gcsDescription.mid(gcsDescription.indexOf(" ")+1, 14);

        QString gcsUavoHashStr;
        QString fwUavoHashStr;
        foreach(char i, fwVersion)
        {
            fwUavoHashStr.append(QString::number(i,16).right(2));
        }
        foreach(char i, uavoHashArray)
        {
            gcsUavoHashStr.append(QString::number(i,16).right(2));
        }
        QString gcsVersion = gcsGitDate + " (" + gcsGitHash + "-"+ gcsUavoHashStr.left(8) + ")";
        QString fwVersion = boardDescription.gitDate + " (" + boardDescription.gitHash + "-" + fwUavoHashStr.left(8) + ")";

        QString warning = QString(tr(
            "GCS and firmware versions of the UAV objects set do not match which can cause configuration problems. "
            "GCS version: %1 Firmware version: %2.")).arg(gcsVersion).arg(fwVersion);
        msg->showMessage(warning);
    }
}

void UploaderGadgetWidget::openHelp()
{
    QDesktopServices::openUrl( QUrl("http://wiki.openpilot.org/x/AoBZ", QUrl::StrictMode) );
}

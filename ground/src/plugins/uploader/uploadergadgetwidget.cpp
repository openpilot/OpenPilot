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

UploaderGadgetWidget::UploaderGadgetWidget(QWidget *parent) : QWidget(parent)
{
    m_config = new Ui_UploaderWidget();
    m_config->setupUi(this);
    currentStep = IAP_STATE_READY;
    rescueStep = RESCUE_STEP0;
    resetOnly=false;
    dfu = NULL;

    //m_config->systemElements->addTab((QWidget*)new deviceWidget(),QString("Device"));

    connect(m_config->haltButton, SIGNAL(clicked()), this, SLOT(goToBootloader()));
    connect(m_config->resetButton, SIGNAL(clicked()), this, SLOT(systemReset()));
    connect(m_config->bootButton, SIGNAL(clicked()), this, SLOT(systemBoot()));
    connect(m_config->rescueButton, SIGNAL(clicked()), this, SLOT(systemRescue()));

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
        // The board is running, send the 1st IAP Reset order:
        fwIAP->getField("Command")->setValue("1122");
        connect(fwIAP,SIGNAL(transactionCompleted(UAVObject*,bool)),this,SLOT(goToBootloader(UAVObject*, bool)));
        currentStep = IAP_STATE_STEP_1;
        fwIAP->updated();
        clearLog();
        log(QString("IAP Step 1"));
        break;
    case IAP_STATE_STEP_1:
        if (!success)  {
            log(QString("Oops, failure step 1"));
            log("Reset did NOT happen");
            currentStep = IAP_STATE_READY;
            disconnect(fwIAP, SIGNAL(transactionCompleted(UAVObject*,bool)),this,SLOT(goToBootloader(UAVObject*, bool)));
            break;
        }
        delay::msleep(600);
        fwIAP->getField("Command")->setValue("2233");
        currentStep = IAP_STATE_STEP_2;
        fwIAP->updated();
        log(QString("IAP Step 2"));
        break;
    case IAP_STATE_STEP_2:
        if (!success) {
            log(QString("Oops, failure step 2"));
            log("Reset did NOT happen");
            currentStep = IAP_STATE_READY;
            disconnect(fwIAP, SIGNAL(transactionCompleted(UAVObject*,bool)),this,SLOT(goToBootloader(UAVObject*, bool)));
            break;
        }
        delay::msleep(600);
        fwIAP->getField("Command")->setValue("3344");
        currentStep = IAP_STEP_RESET;
        fwIAP->updated();
        log(QString("IAP Step 3"));
        break;
    case IAP_STEP_RESET:
    {
        currentStep = IAP_STATE_READY;
        if (success) {
            log("Oops, unexpected success step 3");
            log("Reset did NOT happen");
            disconnect(fwIAP, SIGNAL(transactionCompleted(UAVObject*,bool)),this,SLOT(goToBootloader(UAVObject*, bool)));
            break;
        }
        // The board is now reset: we have to disconnect telemetry
        Core::ConnectionManager *cm = Core::ICore::instance()->connectionManager();
        cm->disconnectDevice();
        log("Board Reset");

        ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
        disconnect(fwIAP, SIGNAL(transactionCompleted(UAVObject*,bool)),this,SLOT(goToBootloader(UAVObject*, bool)));
        if (resetOnly) {
            resetOnly=false;
            break;
        }
        // stop the polling thread: otherwise it will mess up DFU
        RawHIDConnection *cnx =  pm->getObject<RawHIDConnection>();
        cnx->suspendPolling();

        // Tell the mainboard to get into bootloader state:
        log("Going into Bootloader mode...");
        dfu = new DFUObject(false);
        dfu->AbortOperation();
        if(!dfu->enterDFU(0))
        {
            log("Could not enter DFU mode.");
            return;
        }
        m_config->boardStatus->setText("Bootloader");
        currentStep = IAP_STATE_BOOTLOADER;
        //dfu.StatusRequest();
        dfu->findDevices();
        log(QString("Found ") + QString::number(dfu->numberOfDevices) + QString(" device(s)."));
        // Delete all previous tabs:
        while (m_config->systemElements->count()) {
             QWidget *qw = m_config->systemElements->widget(0);
             m_config->systemElements->removeTab(0);
             delete qw;
        }
        for(int i=0;i<dfu->numberOfDevices;i++) {
            deviceWidget* dw = new deviceWidget(this);
            dw->setDeviceID(i);
            dw->setDfu(dfu);
            dw->populate();
            m_config->systemElements->addTab(dw, QString("Device") + QString::number(i));
        }
        m_config->haltButton->setEnabled(false);
        m_config->resetButton->setEnabled(false);
        m_config->bootButton->setEnabled(true);
    }
        break;
    case IAP_STATE_BOOTLOADER:
        // We should never end up here anyway.
        break;
    }

}

/**
  Tell the mainboard to reset:
   - Send the relevant IAP commands
   - setup callback for MoBo acknowledge
   */
void UploaderGadgetWidget::systemReset()
{
    resetOnly = true;
    m_config->textBrowser->clear();
    log("Board Reset initiated.");
    goToBootloader();
}

/**
  Tells the system to boot (from Bootloader state)
  */
void UploaderGadgetWidget::systemBoot()
{
    if (currentStep == IAP_STATE_BOOTLOADER) {
        clearLog();
        if (!dfu)
            dfu = new DFUObject(true);
        dfu->AbortOperation();
        if(!dfu->enterDFU(0))
        {
            log("Could not enter DFU mode.");
            return;
        }
        log("Booting system...");
        dfu->JumpToApp();
        currentStep = IAP_STATE_READY;
        // stop the polling thread: otherwise it will mess up DFU
        ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
        RawHIDConnection *cnx =  pm->getObject<RawHIDConnection>();
        cnx->resumePolling();
        m_config->bootButton->setEnabled(false);
        m_config->haltButton->setEnabled(true);
        m_config->resetButton->setEnabled(true);
        m_config->boardStatus->setText("Running");
        // Freeze the tabs, they are not useful anymore and their buttons
        // will cause segfaults or weird stuff if we use them.
        for (int i=0; i< m_config->systemElements->count(); i++) {
             deviceWidget *qw = (deviceWidget*)m_config->systemElements->widget(i);
             qw->freeze();
        }

        delete dfu;

    } else {
        log("Not in bootloader mode!");
    }
}

/**
  Attempt a guided procedure to put both boards in BL mode when
  the system is not bootable
  */
void UploaderGadgetWidget::systemRescue()
{
    switch (rescueStep) {
    case RESCUE_STEP0: {
        Core::ConnectionManager *cm = Core::ICore::instance()->connectionManager();
        cm->disconnectDevice();

        ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
        // stop the polling thread: otherwise it will mess up DFU
        RawHIDConnection *cnx =  pm->getObject<RawHIDConnection>();
        cnx->suspendPolling();
        // Delete all previous tabs:
        while (m_config->systemElements->count()) {
             QWidget *qw = m_config->systemElements->widget(0);
             m_config->systemElements->removeTab(0);
             delete qw;
        }
        if (dfu) {
            delete dfu;
            dfu = NULL;
        }
        clearLog();
        log("**********************************************************");
        log("** Follow those instructions to attempt a system rescue **");
        log("**********************************************************");
        log("You will be prompted to first connect USB, then system power");
        log ("Connect USB in 2 seconds...");
        rescueStep = RESCUE_STEP1;
        QTimer::singleShot(1000, this, SLOT(systemRescue()));
    }
        break;
    case RESCUE_STEP1:
        rescueStep = RESCUE_STEP2;
        log ("            ...1...");
        QTimer::singleShot(1000, this, SLOT(systemRescue()));
        break;
    case RESCUE_STEP2:
        rescueStep = RESCUE_STEP3;
        log("            ...Now!");
        QTimer::singleShot(1000, this, SLOT(systemRescue()));
        break;
    case RESCUE_STEP3:
        log("... Detecting Mainboard...");
        repaint();
        if (!dfu)
            dfu = new DFUObject(true);
        dfu->AbortOperation();
        if(!dfu->enterDFU(0))
        {
            log("Could not enter DFU mode.");
            return;
        }
        if(!dfu->findDevices())
        {
            log("Could not detect mainboard.");
            return;
        }
        rescueStep = RESCUE_POWER1;
        log("Connect Power in 2 second...");
        QTimer::singleShot(1000, this, SLOT(systemRescue()));
        break;
    case RESCUE_POWER1:
        rescueStep = RESCUE_POWER2;
        log("            ...1...");
        QTimer::singleShot(1000, this, SLOT(systemRescue()));
        break;
    case RESCUE_POWER2:
        log("... NOW!\n***\nWaiting...");
        rescueStep = RESCUE_DETECT;
        QTimer::singleShot(4000, this, SLOT(systemRescue()));
        break;
    case RESCUE_DETECT:
        rescueStep = RESCUE_STEP0;
        log("Detecting AHRS...");
        repaint();
        if(!dfu->findDevices())
        {
            log("Could not detect devices.");
            return;
        }
        log(QString("Found ") + QString::number(dfu->numberOfDevices) + QString(" device(s)."));
        if (dfu->numberOfDevices > 5) {
            log("Inconsistent number of devices, aborting!");
            return;
        }
        for(int i=0;i<dfu->numberOfDevices;i++) {
            deviceWidget* dw = new deviceWidget(this);
            dw->setDeviceID(i);
            dw->setDfu(dfu);
            dw->populate();
            m_config->systemElements->addTab(dw, QString("Device") + QString::number(i));
        }
        m_config->haltButton->setEnabled(false);
        m_config->resetButton->setEnabled(false);
        m_config->bootButton->setEnabled(true);
        currentStep = IAP_STATE_BOOTLOADER; // So that we can boot from the GUI afterwards.
    }


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

UploaderGadgetWidget::~UploaderGadgetWidget()
{

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

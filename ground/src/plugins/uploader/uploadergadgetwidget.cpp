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
    resetOnly=false;

    connect(m_config->haltButton, SIGNAL(clicked()), this, SLOT(goToBootloader()));
    connect(m_config->resetButton, SIGNAL(clicked()), this, SLOT(systemReset()));


}

/**
  Tell the mainboard to go to bootloader:
   - Send the relevant IAP commands
   - setup callback for MoBo acknowledge
   */
void UploaderGadgetWidget::goToBootloader(UAVObject* callerObj, bool success)
{
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
        log(QString("IAP Step 1"));
        break;
    case IAP_STATE_STEP_1:
        if (!success)  {
            log(QString("Oops, failure step 1"));
            currentStep == IAP_STATE_READY;
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
            currentStep == IAP_STATE_READY;
            break;
        }
        delay::msleep(600);
        fwIAP->getField("Command")->setValue("3344");
        currentStep = IAP_STEP_RESET;
        fwIAP->updated();
        log(QString("IAP Step 3"));
        break;
    case IAP_STEP_RESET: {
        currentStep = IAP_STATE_READY;
        if (success) {
            log("Oops, unexpected success step 3");
            log("Reset did NOT happen");
            break;
        }
        // The board is now reset: we have to disconnect telemetry
        Core::ConnectionManager *cm = Core::ICore::instance()->connectionManager();
        cm->disconnectDevice();
        log(QString("Board Reset"));

        ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
        disconnect(fwIAP, SIGNAL(transactionCompleted(UAVObject*,bool)),this,SLOT(goToBootloader(UAVObject*, bool)));
        if (resetOnly) {
            resetOnly=false;
            break;
        }
        // stop the polling thread: otherwise it will mess up DFU
        RawHIDConnection *cnx =  pm->getObject<RawHIDConnection>();
        cnx->suspendPolling();

    }

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
  Update status
  */
void UploaderGadgetWidget::log(QString str)
{
   m_config->textBrowser->append(str);

}

//user pressed send, send file using a new thread with qymodem library
void UploaderGadgetWidget::send()
{
    Ymodem->SendFileT(openFileNameLE->text());
}
//destructor !!?! do I need to delete something else?
UploaderGadgetWidget::~UploaderGadgetWidget()
{
    delete Port;
    delete Ymodem;
}

//from load configuration, creates a new qymodemsend class with the the port
/**
Cteates a new qymodemsend class.

@param port	The serial port to use.


*/
void UploaderGadgetWidget::setPort(QextSerialPort* port)
{

    Port=port;
    Ymodem=new QymodemSend(*Port);
    //only now can we connect this signals
    //signals errors
    connect(Ymodem,SIGNAL(Error(QString,int))
            ,this,SLOT(error(QString,int)));
    //signals new information
    connect(Ymodem,SIGNAL(Information(QString,int)),
            this,SLOT(info(QString,int)));
    //signals new percentage value
    connect(Ymodem,SIGNAL(Percent(int)),
            this,SLOT(updatePercSlot(int)));
}
/**
Updates progress bar value.

@param i	New percentage value.

*/
void UploaderGadgetWidget::updatePercSlot(int i)
{
    progressBar->setValue(i);
}
/**

Opens an open file dialog.

*/
void UploaderGadgetWidget::setOpenFileName()
{
    QFileDialog::Options options;
    QString selectedFilter;
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    tr("QFileDialog::getOpenFileName()"),
                                                    openFileNameLE->text(),
                                                    tr("All Files (*);;Text Files (*.bin)"),
                                                    &selectedFilter,
                                                    options);
    if (!fileName.isEmpty()) openFileNameLE->setText(fileName);

}
/**
Shows a message box with an error string.

@param errorString	The error string to display.

@param errorNumber      Not used

*/
void UploaderGadgetWidget::error(QString errorString, int errorNumber)
{
    QMessageBox msgBox;
    msgBox.setIcon(QMessageBox::Critical);
    msgBox.setText(errorString);
    msgBox.exec();
    status->setText(errorString);
}
/**
Shows a message box with an information string.

@param infoString	The information string to display.

@param infoNumber       Not used

*/
void UploaderGadgetWidget::info(QString infoString, int infoNumber)
{
    status->setText(infoString);
}

/**
 ******************************************************************************
 *
 * @file       devicewidget.cpp
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
#include "devicewidget.h"

deviceWidget::deviceWidget(QWidget *parent) :
    QWidget(parent)
{
    myDevice = new Ui_deviceWidget();
    myDevice->setupUi(this);
    devicePic = NULL; // Initialize pointer to null

    // Initialization of the Device icon display
    myDevice->devicePicture->setScene(new QGraphicsScene(this));

    connect(myDevice->verifyButton, SIGNAL(clicked()), this, SLOT(verifyFirmware()));
    connect(myDevice->retrieveButton, SIGNAL(clicked()), this, SLOT(downloadFirmware()));
    connect(myDevice->updateButton, SIGNAL(clicked()), this, SLOT(uploadFirmware()));
}


void deviceWidget::showEvent(QShowEvent *event)
{
    Q_UNUSED(event)
    // Thit fitInView method should only be called now, once the
    // widget is shown, otherwise it cannot compute its values and
    // the result is usually a ahrsbargraph that is way too small.
    if (devicePic)
       myDevice->devicePicture->fitInView(devicePic,Qt::KeepAspectRatio);
}

void deviceWidget::resizeEvent(QResizeEvent* event)
{
    Q_UNUSED(event);
    if (devicePic)
        myDevice->devicePicture->fitInView(devicePic, Qt::KeepAspectRatio);
}


void deviceWidget::setDeviceID(int devID){
    deviceID = devID;
}

void deviceWidget::setDfu(DFUObject *dfu)
{
    m_dfu = dfu;
}

/**
  Fills the various fields for the device
  */
void deviceWidget::populate()
{
    int id = m_dfu->devices[deviceID].ID;
    myDevice->deviceID->setText(QString("Device ID: ") + QString::number(id));
    // DeviceID tells us what sort of HW we have detected:
    // display a nice icon:
    myDevice->devicePicture->scene()->clear();
    if (devicePic)
        delete devicePic;
    devicePic = new QGraphicsSvgItem();
    devicePic->setSharedRenderer(new QSvgRenderer());

    switch (id) {
    case 1:
        devicePic->renderer()->load(QString(":/uploader/images/deviceID-1.svg"));
        break;
    case 69:
        devicePic->renderer()->load(QString(":/uploader/images/deviceID-69.svg"));
        break;
    default:
        break;
    }
    devicePic->setElementId("device");
    myDevice->devicePicture->scene()->addItem(devicePic);
    myDevice->devicePicture->setSceneRect(devicePic->boundingRect());
    myDevice->devicePicture->fitInView(devicePic,Qt::KeepAspectRatio);

    bool r = m_dfu->devices[deviceID].Readable;
    bool w = m_dfu->devices[deviceID].Writable;
    myDevice->deviceACL->setText(QString("Access: ") + QString(r ? "R" : "-") + QString(w ? "W" : "-"));
    myDevice->maxCodeSize->setText(QString("Max code size: ") +QString::number(m_dfu->devices[deviceID].SizeOfCode));
    myDevice->fwCRC->setText(QString("FW CRC: ") + QString::number(m_dfu->devices[deviceID].FW_CRC));
    myDevice->BLVersion->setText(QString("BL Version: ") + QString::number(m_dfu->devices[deviceID].BL_Version));

    int size=((OP_DFU::device)m_dfu->devices[deviceID]).SizeOfDesc;
    m_dfu->enterDFU(deviceID);
    QString str = m_dfu->DownloadDescription(size);
    myDevice->description->setMaxLength(size);
    myDevice->description->setText(str.left(str.indexOf(QChar(255))));

    myDevice->statusLabel->setText(QString("Ready..."));

}

/**
  Freezes the contents of the widget so that a user cannot
  try to modify the contents
  */
void deviceWidget::freeze()
{
   myDevice->description->setEnabled(false);
   myDevice->updateButton->setEnabled(false);
   myDevice->verifyButton->setEnabled(false);
   myDevice->retrieveButton->setEnabled(false);
}

/**
  Updates status message
  */
void deviceWidget::status(QString str)
{
    myDevice->statusLabel->setText(str);
}

/**
  Verifies the firmware CRC
  */
void deviceWidget::verifyFirmware()
{

}

/**
  Sends a firmware to the device
  */
void deviceWidget::uploadFirmware()
{
    if (!m_dfu->devices[deviceID].Writable) {
        status("Device not writable!");
        return;
    }

    bool verify = false;
    if (m_dfu->devices[deviceID].Readable)
        verify = true;

    QString filename = setOpenFileName();

    if (filename.isEmpty()) {
        status("Empty filename");
        return;
    }

    status("Starting firmware upload");
    // We don't know which device was used previously, so we
    // are cautious and reenter DFU for this deviceID:
    if(!m_dfu->enterDFU(deviceID))
    {
        status("Error:Could not enter DFU mode");
        return;
    }
    OP_DFU::Status ret=m_dfu->StatusRequest();
    qDebug() << m_dfu->StatusToString(ret);
    m_dfu->AbortOperation(); // Necessary, otherwise I get random failures.

    connect(m_dfu, SIGNAL(progressUpdated(int)), this, SLOT(setProgress(int)));
    connect(m_dfu, SIGNAL(operationProgress(QString)), this, SLOT(status(QString)));
    connect(m_dfu, SIGNAL(uploadFinished(OP_DFU::Status)), this, SLOT(uploadFinished(OP_DFU::Status)));
    bool retstatus = m_dfu->UploadFirmware(filename.toAscii(),verify, deviceID);
    if(!retstatus ) {
        status("Could not start upload");
        return;
    }
    status("Uploading, please wait...");
}

/**
  Retrieves the firmware from the device
  */
void deviceWidget::downloadFirmware()
{
    if (!m_dfu->devices[deviceID].Readable) {
        myDevice->statusLabel->setText(QString("Device not readable!"));
        return;
    }

    myDevice->retrieveButton->setEnabled(false);
    filename = setOpenFileName();

    if (filename.isEmpty()) {
        status("Empty filename");
        return;
    }

    status("Downloading firmware from device");
    connect(m_dfu, SIGNAL(progressUpdated(int)), this, SLOT(setProgress(int)));
    connect(m_dfu, SIGNAL(downloadFinished()), this, SLOT(downloadFinished()));
    downloadedFirmware.clear(); // Empty the byte array
    bool ret = m_dfu->DownloadFirmware(&downloadedFirmware,deviceID);
    if(!ret) {
        status(QString("Could not start download!"));
        return;
    }
    status("Download started, please wait");
}

/**
  Callback for the firmware download result
  */
void deviceWidget::downloadFinished()
{
    disconnect(m_dfu, SIGNAL(downloadFinished()), this, SLOT(downloadFinished()));
    disconnect(m_dfu, SIGNAL(progressUpdated(int)), this, SLOT(setProgress(int)));
    status("Download successful");
    // Now save the result (use the utility function from OP_DFU)
    m_dfu->SaveByteArrayToFile(filename, downloadedFirmware);
    myDevice->retrieveButton->setEnabled(true);
}

/**
  Callback for the firmware upload result
  */
void deviceWidget::uploadFinished(OP_DFU::Status retstatus)
{
    disconnect(m_dfu, SIGNAL(uploadFinished(OP_DFU::Status)), this, SLOT(uploadFinished(OP_DFU::Status)));
    disconnect(m_dfu, SIGNAL(progressUpdated(int)), this, SLOT(setProgress(int)));
    disconnect(m_dfu, SIGNAL(operationProgress(QString)), this, SLOT(status(QString)));
    if(retstatus != OP_DFU::Last_operation_Success) {
        status(QString("Upload failed with code: ") + m_dfu->StatusToString(retstatus).toLatin1().data());
        return;
    } else
    if(!myDevice->description->text().isEmpty()) {
        status(QString("Updating description"));
        repaint(); // Make sure the text above shows right away
        retstatus = m_dfu->UploadDescription(myDevice->description->text());
        if( retstatus != OP_DFU::Last_operation_Success) {
            status(QString("Upload failed with code: ") + m_dfu->StatusToString(retstatus).toLatin1().data());
            return;
        }
    }
    status("Upload successful");

}

/**
  Slot to update the progress bar
  */
void deviceWidget::setProgress(int percent)
{
    myDevice->progressBar->setValue(percent);
}

/**

Opens an open file dialog.

*/
QString deviceWidget::setOpenFileName()
{
    QFileDialog::Options options;
    QString selectedFilter;
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    tr("Select firmware file"),
                                                    "",
                                                    tr("Firmware Files (*.bin)"),
                                                    &selectedFilter,
                                                    options);
    return fileName;
}

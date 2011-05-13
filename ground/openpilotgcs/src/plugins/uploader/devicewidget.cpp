/**
 ******************************************************************************
 *
 * @file       devicewidget.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup Uploader Serial and USB Uploader Plugin
 * @{
 * @brief The USB and Serial protocol uploader plugin
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

    QPixmap pix = QPixmap(QString(":uploader/images/view-refresh.svg"));
    myDevice->statusIcon->setPixmap(pix);
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
    myDevice->deviceID->setText(QString("Device ID: ") + QString::number(id, 16));
    // DeviceID tells us what sort of HW we have detected:
    // display a nice icon:
    myDevice->devicePicture->scene()->clear();
    if (devicePic)
        delete devicePic;
    devicePic = new QGraphicsSvgItem();
    devicePic->setSharedRenderer(new QSvgRenderer());

    switch (id) {
    case 0x0101:
        devicePic->renderer()->load(QString(":/uploader/images/deviceID-0101.svg"));
        break;
    case 0x0301:
        devicePic->renderer()->load(QString(":/uploader/images/deviceID-0301.svg"));
        break;
    case 0x0401:
        devicePic->renderer()->load(QString(":/uploader/images/deviceID-0401.svg"));
        break;
    case 0x0201:
        devicePic->renderer()->load(QString(":/uploader/images/deviceID-0201.svg"));
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
    QByteArray desc = m_dfu->DownloadDescriptionAsBA(size);
    if (! populateStructuredDescription(desc)) {
        // desc was not a structured description
        QString str = m_dfu->DownloadDescription(size);
        myDevice->description->setMaxLength(size);
        myDevice->description->setText(str.left(str.indexOf(QChar(255))));
        myDevice->buildDate->setText("Warning: development firmware");
        myDevice->commitTag->setText("");
    }


    status("Ready...", STATUSICON_INFO);

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
  Populates the widget field with the description in case
  it is structured properly
  */
bool deviceWidget::populateStructuredDescription(QByteArray desc)
{
    if (desc.startsWith("OpFw")) {
        // This looks like a binary with a description at the end
        /*
        #  4 bytes: header: "OpFw"
        #  4 bytes: GIT commit tag (short version of SHA1)
        #  4 bytes: Unix timestamp of compile time
        #  2 bytes: target platform. Should follow same rule as BOARD_TYPE and BOARD_REVISION in board define files.
        #  26 bytes: commit tag if it is there, otherwise "Unreleased". Zero-padded
        #   ---- 40 bytes limit ---
        #  20 bytes: SHA1 sum of the firmware.
        #  40 bytes: free for now.
        */
        // I don't want to use structs, ok ?
        quint32 gitCommitTag = desc.at(4)&0xFF;
        for (int i=1;i<4;i++) {
            gitCommitTag = gitCommitTag<<8;
            gitCommitTag += desc.at(4+i) & 0xFF;
        }
        myDevice->commitTag->setText("GIT tag 0x" + QString::number(gitCommitTag,16));
        quint32 buildDate = desc.at(8)&0xFF;
        for (int i=1;i<4;i++) {
            buildDate = buildDate<<8;
            buildDate += desc.at(8+i) & 0xFF;
        }

        myDevice->buildDate->setText(QString("Build time: ") + QDateTime::fromTime_t(buildDate).toString());
        QByteArray targetPlatform = desc.mid(12,2);
        // TODO: check platform compatibility
        QString dscText = QString(desc.mid(14,26));
        myDevice->description->setText(dscText);

        return true;
    }

    return false;

}

/**
  Updates status message for messages coming from DFU
  */
void deviceWidget::dfuStatus(QString str)
{
    status(str, STATUSICON_RUNNING);
}

/**
  Updates status message
  */
void deviceWidget::status(QString str, StatusIcon ic)
{
    QPixmap px;
    myDevice->statusLabel->setText(str);
    switch (ic) {
    case STATUSICON_RUNNING:
        px.load(QString(":/uploader/images/system-run.svg"));
        break;
    case STATUSICON_OK:
        px.load(QString(":/uploader/images/dialog-apply.svg"));
        break;
    case STATUSICON_FAIL:
        px.load(QString(":/uploader/images/process-stop.svg"));
        break;
    default:
        px.load(QString(":/uploader/images/gtk-info.svg"));
    }
    myDevice->statusIcon->setPixmap(px);
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
        status("Device not writable!", STATUSICON_FAIL);
        return;
    }

    bool verify = false;
    /* TODO: does not work properly on current Bootloader!
    if (m_dfu->devices[deviceID].Readable)
        verify = true;
     */

    QString filename = setOpenFileName();

    if (filename.isEmpty()) {
        status("Empty filename", STATUSICON_FAIL);
        return;
    }

    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly)) {
        status("Can't open file", STATUSICON_FAIL);
        return;
    }

    QByteArray arr = file.readAll();
    QByteArray desc = arr.right(100);
    if (populateStructuredDescription(desc)) {
        descriptionArray = desc;
        // Now do sanity checking:
        // - Check whether board type matches firmware:
        int board = m_dfu->devices[deviceID].ID;
        int firmwareBoard = ((desc.at(12)&0xff)<<8) + (desc.at(13)&0xff);
        if (firmwareBoard != board) {
            status("Error: firmware does not match board", STATUSICON_FAIL);
            return;
        }

        // Check the firmware embedded in the file:
        QByteArray firmwareHash = desc.mid(40,20);
        QByteArray fileHash = QCryptographicHash::hash(arr.left(arr.length()-100), QCryptographicHash::Sha1);
        if (firmwareHash != fileHash) {
            status("Error: firmware file corrupt", STATUSICON_FAIL);
            return;
        }



    } else {
        // The firmware is not packaged, just upload the text in the description field
        // if it is there.
        descriptionArray.clear();
    }




    status("Starting firmware upload", STATUSICON_RUNNING);
    // We don't know which device was used previously, so we
    // are cautious and reenter DFU for this deviceID:
    if(!m_dfu->enterDFU(deviceID))
    {
        status("Error:Could not enter DFU mode", STATUSICON_FAIL);
        return;
    }
    OP_DFU::Status ret=m_dfu->StatusRequest();
    qDebug() << m_dfu->StatusToString(ret);
    m_dfu->AbortOperation(); // Necessary, otherwise I get random failures.

    connect(m_dfu, SIGNAL(progressUpdated(int)), this, SLOT(setProgress(int)));
    connect(m_dfu, SIGNAL(operationProgress(QString)), this, SLOT(dfuStatus(QString)));
    connect(m_dfu, SIGNAL(uploadFinished(OP_DFU::Status)), this, SLOT(uploadFinished(OP_DFU::Status)));
    bool retstatus = m_dfu->UploadFirmware(filename.toAscii(),verify, deviceID);
    if(!retstatus ) {
        status("Could not start upload", STATUSICON_FAIL);
        return;
    }
    status("Uploading, please wait...", STATUSICON_RUNNING);
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
    filename = setSaveFileName();

    if (filename.isEmpty()) {
        status("Empty filename", STATUSICON_FAIL);
        return;
    }

    status("Downloading firmware from device", STATUSICON_RUNNING);
    connect(m_dfu, SIGNAL(progressUpdated(int)), this, SLOT(setProgress(int)));
    connect(m_dfu, SIGNAL(downloadFinished()), this, SLOT(downloadFinished()));
    downloadedFirmware.clear(); // Empty the byte array
    bool ret = m_dfu->DownloadFirmware(&downloadedFirmware,deviceID);
    if(!ret) {
        status("Could not start download!", STATUSICON_FAIL);
        return;
    }
    status("Download started, please wait", STATUSICON_RUNNING);
}

/**
  Callback for the firmware download result
  */
void deviceWidget::downloadFinished()
{
    disconnect(m_dfu, SIGNAL(downloadFinished()), this, SLOT(downloadFinished()));
    disconnect(m_dfu, SIGNAL(progressUpdated(int)), this, SLOT(setProgress(int)));
    status("Download successful", STATUSICON_OK);
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
    disconnect(m_dfu, SIGNAL(operationProgress(QString)), this, SLOT(dfuStatus(QString)));
    if(retstatus != OP_DFU::Last_operation_Success) {
        status(QString("Upload failed with code: ") + m_dfu->StatusToString(retstatus).toLatin1().data(), STATUSICON_FAIL);
        return;
    } else
    if (!descriptionArray.isEmpty()) {
        // We have a structured array to save
        status(QString("Updating description"), STATUSICON_RUNNING);
        repaint(); // Make sure the text above shows right away
        retstatus = m_dfu->UploadDescription(descriptionArray);
        if( retstatus != OP_DFU::Last_operation_Success) {
            status(QString("Upload failed with code: ") + m_dfu->StatusToString(retstatus).toLatin1().data(), STATUSICON_FAIL);
            return;
        }

    } else if (!myDevice->description->text().isEmpty()) {
       // Fallback: we save the description field:
       status(QString("Updating description"), STATUSICON_RUNNING);
       repaint(); // Make sure the text above shows right away
       retstatus = m_dfu->UploadDescription(myDevice->description->text());
       if( retstatus != OP_DFU::Last_operation_Success) {
           status(QString("Upload failed with code: ") + m_dfu->StatusToString(retstatus).toLatin1().data(), STATUSICON_FAIL);
           return;
       }
    }

    status("Upload successful", STATUSICON_OK);

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
QString deviceWidget::setSaveFileName()
{
    QFileDialog::Options options;
    QString selectedFilter;
    QString fileName = QFileDialog::getSaveFileName(this,
                                                    tr("Select firmware file"),
                                                    "",
                                                    tr("Firmware Files (*.bin)"),
                                                    &selectedFilter,
                                                    options);
    return fileName;
}

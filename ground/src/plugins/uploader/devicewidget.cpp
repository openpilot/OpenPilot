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

    connect(myDevice->verifyButton, SIGNAL(clicked()), this, SLOT(verifyFirmware()));
    connect(myDevice->retrieveButton, SIGNAL(clicked()), this, SLOT(downloadFirmware()));
    connect(myDevice->updateButton, SIGNAL(clicked()), this, SLOT(uploadFirmware()));
}

void deviceWidget::setDeviceID(int devID){
    deviceID = devID;
}

void deviceWidget::setDfu(OP_DFU *dfu)
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
        myDevice->statusLabel->setText(QString("Device not writable!"));
        return;
    }

    bool verify = true;
    QString filename = setOpenFileName();

    if (filename.isEmpty()) {
        status("Empty filename");
        return;
    }

    status("Uploading firmware to device");
    repaint(); // FW Upload is not threaded and will mostly freeze the UI...
    connect(m_dfu, SIGNAL(progressUpdated(int)), this, SLOT(setProgress(int)));
    OP_DFU::Status retstatus = m_dfu->UploadFirmware(filename.toAscii(),verify, deviceID);
    if(retstatus != OP_DFU::Last_operation_Success) {
        status(QString("Upload failed with code: ") + m_dfu->StatusToString(retstatus).toLatin1().data());
        return;
    } else
    if(!myDevice->description->text().isEmpty()) {
        retstatus = m_dfu->UploadDescription(myDevice->description->text());
        if( retstatus != OP_DFU::Last_operation_Success) {
            status(QString("Upload failed with code: ") + m_dfu->StatusToString(retstatus).toLatin1().data());
            return;
        }
    }
    status("Uploading Succeded!");
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

    /*
                qint32 size=((OP_DFU::device)dfu.devices[device]).SizeOfCode;
                bool ret=dfu.SaveByteArrayToFile(file.toAscii(),dfu.StartDownload(size,OP_DFU::FW));
                return ret;
*/
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
                                                    tr("All Files (*);;Firmware Files (*.bin)"),
                                                    &selectedFilter,
                                                    options);
    return fileName;
}

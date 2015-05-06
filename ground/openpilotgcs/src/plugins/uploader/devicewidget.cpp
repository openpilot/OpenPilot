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

DeviceWidget::DeviceWidget(QWidget *parent) :
    QWidget(parent)
{
    myDevice = new Ui_deviceWidget();
    myDevice->setupUi(this);

    // Initialization of the Device icon display
    myDevice->verticalGroupBox_loaded->setVisible(false);
    myDevice->groupCustom->setVisible(false);
    myDevice->confirmCheckBox->setVisible(false);
    myDevice->gVDevice->setScene(new QGraphicsScene(this));
    connect(myDevice->retrieveButton, SIGNAL(clicked()), this, SLOT(downloadFirmware()));
    connect(myDevice->updateButton, SIGNAL(clicked()), this, SLOT(uploadFirmware()));
    connect(myDevice->pbLoad, SIGNAL(clicked()), this, SLOT(loadFirmware()));
    connect(myDevice->confirmCheckBox, SIGNAL(stateChanged(int)), this, SLOT(confirmCB(int)));
    myDevice->statusIcon->setPixmap(QPixmap(":uploader/images/view-refresh.svg"));

    myDevice->lblCertified->setText("");
}

void DeviceWidget::showEvent(QShowEvent *event)
{
    Q_UNUSED(event)
    // Thit fitInView method should only be called now, once the
    // widget is shown, otherwise it cannot compute its values and
    // the result is usually a ahrsbargraph that is way too small
    myDevice->gVDevice->fitInView(devicePic.rect(), Qt::KeepAspectRatio);
}

void DeviceWidget::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);
    myDevice->gVDevice->fitInView(devicePic.rect(), Qt::KeepAspectRatio);
}

void DeviceWidget::setDeviceID(int devID)
{
    deviceID = devID;
}

void DeviceWidget::setDfu(DFUObject *dfu)
{
    m_dfu = dfu;
}

/**
   Fills the various fields for the device
 */
void DeviceWidget::populate()
{
    int id = m_dfu->devices[deviceID].ID;

    myDevice->lbldevID->setText(tr("Device ID: ") + QString::number(id, 16));
    // DeviceID tells us what sort of HW we have detected:
    // display a nice icon:
    myDevice->gVDevice->scene()->clear();
    myDevice->lblDevName->setText(deviceDescriptorStruct::idToBoardName(id));
    myDevice->lblHWRev->setText(tr("HW Revision: ") + QString::number(id & 0x00FF, 16));

    switch (id) {
    case 0x0301:
        devicePic.load(":/uploader/images/gcs-board-oplink.png");
        break;
    case 0x0903:
    case 0x0904:
        devicePic.load(":/uploader/images/gcs-board-revo.png");
        break;
    default:
        // Clear
        devicePic.load("");
        break;
    }
    myDevice->gVDevice->scene()->addPixmap(devicePic);
    myDevice->gVDevice->setSceneRect(devicePic.rect());
    myDevice->gVDevice->fitInView(devicePic.rect(), Qt::KeepAspectRatio);

    bool r = m_dfu->devices[deviceID].Readable;
    bool w = m_dfu->devices[deviceID].Writable;

    myDevice->lblAccess->setText(tr("Flash access: ") + QString(r ? "R" : "-") + QString(w ? "W" : "-"));
    myDevice->lblMaxCode->setText(tr("Max code size: ") + QString::number(m_dfu->devices[deviceID].SizeOfCode));
    myDevice->lblCRC->setText(QString::number(m_dfu->devices[deviceID].FW_CRC));
    myDevice->lblBLVer->setText(tr("BL version: ") + QString::number(m_dfu->devices[deviceID].BL_Version));
    int size = ((OP_DFU::device)m_dfu->devices[deviceID]).SizeOfDesc;
    m_dfu->enterDFU(deviceID);
    QByteArray desc = m_dfu->DownloadDescriptionAsBA(size);

    if (!populateBoardStructuredDescription(desc)) {
        // desc was not a structured description
        QString str = m_dfu->DownloadDescription(size);
        myDevice->lblDescription->setText((!str.isEmpty()) ? str : tr("Unknown"));
        myDevice->lblCertified->setPixmap(QPixmap(":uploader/images/warning.svg"));
        myDevice->lblCertified->setToolTip(tr("Custom Firmware Build"));
        myDevice->lblBuildDate->setText(tr("Unknown"));
        myDevice->lblGitTag->setText(tr("Unknown"));
        myDevice->lblBrdName->setText(tr("Unknown"));
    }
    myDevice->filenameLabel->setText(tr("No file loaded"));
    status("Ready...", STATUSICON_INFO);
    QString fwFileName = getDevFirmwarePath();
    QFile fwFile(fwFileName);
    if (fwFile.exists()) {
        loadFirmware(fwFileName);
    }
}

/**
   Freezes the contents of the widget so that a user cannot
   try to modify the contents
 */
void DeviceWidget::freeze()
{
    updateButtons(false);
}

void DeviceWidget::updateButtons(bool enabled)
{
    if (!enabled) {
        myDevice->description->setEnabled(false);
        myDevice->pbLoad->setEnabled(false);
        myDevice->confirmCheckBox->setEnabled(false);
        myDevice->updateButton->setEnabled(false);
        myDevice->retrieveButton->setEnabled(false);
    } else {
        myDevice->description->setEnabled(true);
        // Load button (i.e. choose file) is always enabled
        myDevice->pbLoad->setEnabled(true);
        myDevice->confirmCheckBox->setEnabled(true);
        // Update/Upload button is enabled if the "I know what I'm doing!" check box is checked
        myDevice->updateButton->setEnabled(myDevice->confirmCheckBox->checkState() == Qt::Checked);
        // Retreive/Download button is always enabled
        myDevice->retrieveButton->setEnabled(true);
    }
}

/**
   Populates the widget field with the description in case
   it is structured properly
 */
bool DeviceWidget::populateBoardStructuredDescription(QByteArray desc)
{
    if (UAVObjectUtilManager::descriptionToStructure(desc, onBoardDescription)) {
        myDevice->lblGitTag->setText(onBoardDescription.gitHash);
        myDevice->lblBuildDate->setText(onBoardDescription.gitDate.insert(4, "-").insert(7, "-"));
        if (onBoardDescription.gitTag.startsWith("RELEASE", Qt::CaseSensitive)) {
            myDevice->lblDescription->setText(onBoardDescription.gitTag);
            myDevice->lblCertified->setPixmap(QPixmap(":uploader/images/application-certificate.svg"));
            myDevice->lblCertified->setToolTip(tr("Tagged officially released firmware build"));
        } else {
            myDevice->lblDescription->setText(onBoardDescription.gitTag);
            myDevice->lblCertified->setPixmap(QPixmap(":uploader/images/warning.svg"));
            myDevice->lblCertified->setToolTip(tr("Untagged or custom firmware build"));
        }

        myDevice->lblBrdName->setText(deviceDescriptorStruct::idToBoardName(onBoardDescription.boardType << 8 | onBoardDescription.boardRevision));

        return true;
    }

    return false;
}

bool DeviceWidget::populateLoadedStructuredDescription(QByteArray desc)
{
    if (UAVObjectUtilManager::descriptionToStructure(desc, LoadedDescription)) {
        myDevice->lblGitTagL->setText(LoadedDescription.gitHash);
        myDevice->lblBuildDateL->setText(LoadedDescription.gitDate.insert(4, "-").insert(7, "-"));
        if (LoadedDescription.gitTag.startsWith("RELEASE", Qt::CaseSensitive)) {
            myDevice->lblDescritpionL->setText(LoadedDescription.gitTag);
            myDevice->description->setText(LoadedDescription.gitTag);
            myDevice->lblCertifiedL->setPixmap(QPixmap(":uploader/images/application-certificate.svg"));
            myDevice->lblCertifiedL->setToolTip(tr("Tagged officially released firmware build"));
        } else {
            myDevice->lblDescritpionL->setText(LoadedDescription.gitTag);
            myDevice->description->setText(LoadedDescription.gitTag);
            myDevice->lblCertifiedL->setPixmap(QPixmap(":uploader/images/warning.svg"));
            myDevice->lblCertifiedL->setToolTip(tr("Untagged or custom firmware build"));
        }
        myDevice->lblBrdNameL->setText(deviceDescriptorStruct::idToBoardName(LoadedDescription.boardType << 8 | LoadedDescription.boardRevision));

        return true;
    }
    return false;
}

/**
   Updates status message for messages coming from DFU
 */
void DeviceWidget::dfuStatus(QString str)
{
    status(str, STATUSICON_RUNNING);
}

void DeviceWidget::confirmCB(int value)
{
    Q_UNUSED(value);
    updateButtons(true);
}

/**
   Updates status message
 */
void DeviceWidget::status(QString str, StatusIcon ic)
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

void DeviceWidget::loadFirmware()
{
    QString file = setOpenFileName();

    loadFirmware(file);
}

void DeviceWidget::loadFirmware(QString fwfilename)
{
    myDevice->verticalGroupBox_loaded->setVisible(false);
    myDevice->groupCustom->setVisible(false);

    filename = fwfilename;

    myDevice->confirmCheckBox->setVisible(false);
    myDevice->confirmCheckBox->setChecked(false);

    if (filename.isEmpty()) {
        status("Empty filename", STATUSICON_FAIL);
        return;
    }

    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly)) {
        status("Can't open file", STATUSICON_FAIL);
        return;
    }

    loadedFW = file.readAll();

    QByteArray desc = loadedFW.right(100);
    QPixmap px;
    if (loadedFW.length() > (int)m_dfu->devices[deviceID].SizeOfCode) {
        myDevice->lblCRCL->setText(tr("Can't calculate, file too big for device"));
    } else {
        myDevice->lblCRCL->setText(QString::number(DFUObject::CRCFromQBArray(loadedFW, m_dfu->devices[deviceID].SizeOfCode)));
    }

    // myDevice->lblFirmwareSizeL->setText(QString("Firmware size: ")+QVariant(loadedFW.length()).toString()+ QString(" bytes"));
    if (populateLoadedStructuredDescription(desc)) {
        myDevice->confirmCheckBox->setChecked(true);
        myDevice->verticalGroupBox_loaded->setVisible(true);
        myDevice->groupCustom->setVisible(false);
        if (myDevice->lblCRC->text() == myDevice->lblCRCL->text()) {
            myDevice->statusLabel->setText(tr("The board has the same firmware as loaded. No need to update."));
            px.load(QString(":/uploader/images/warning.svg"));
        } else if (myDevice->lblDevName->text() != myDevice->lblBrdNameL->text()) {
            myDevice->statusLabel->setText(tr("WARNING: the loaded firmware is for different hardware. Do not update!"));
            px.load(QString(":/uploader/images/error.svg"));
        } else if (QDateTime::fromString(onBoardDescription.gitDate) > QDateTime::fromString(LoadedDescription.gitDate)) {
            myDevice->statusLabel->setText(tr("The board has newer firmware than loaded. Are you sure you want to update?"));
            px.load(QString(":/uploader/images/warning.svg"));
        } else if (!LoadedDescription.gitTag.startsWith("RELEASE", Qt::CaseSensitive)) {
            myDevice->statusLabel->setText(tr("The loaded firmware is untagged or custom build. Update only if it was received from a trusted source (official website or your own build)."));
            px.load(QString(":/uploader/images/warning.svg"));
        } else {
            myDevice->statusLabel->setText(tr("This is the tagged officially released OpenPilot firmware."));
            px.load(QString(":/uploader/images/gtk-info.svg"));
        }
    } else {
        myDevice->statusLabel->setText(tr("WARNING: the loaded firmware was not packaged with the OpenPilot format. Do not update unless you know what you are doing."));
        px.load(QString(":/uploader/images/error.svg"));
        myDevice->confirmCheckBox->setChecked(false);
        myDevice->confirmCheckBox->setVisible(true);
        myDevice->verticalGroupBox_loaded->setVisible(false);
        myDevice->groupCustom->setVisible(true);
    }
    myDevice->filenameLabel->setText(tr("Firmware loaded: ") + filename);
    myDevice->statusIcon->setPixmap(px);
}

/**
   Sends a firmware to the device
 */
void DeviceWidget::uploadFirmware()
{
    // clear progress bar now
    // this avoids displaying an error message and the progress at 100% at the same time
    setProgress(0);
    updateButtons(false);

    if (!m_dfu->devices[deviceID].Writable) {
        status("Device not writable!", STATUSICON_FAIL);
        updateButtons(true);
        return;
    }

    bool verify     = false;
    /* TODO: does not work properly on current Bootloader!
       if (m_dfu->devices[deviceID].Readable)
        verify = true;
     */

    QByteArray desc = loadedFW.right(100);
    if (desc.startsWith("OpFw")) {
        descriptionArray = desc;
        // Now do sanity checking:
        // - Check whether board type matches firmware:
        int board = m_dfu->devices[deviceID].ID;
        int firmwareBoard = ((desc.at(12) & 0xff) << 8) + (desc.at(13) & 0xff);
        if ((board == 0x901 && firmwareBoard == 0x902) || // L3GD20 revo supports Revolution firmware
            (board == 0x902 && firmwareBoard == 0x903)) { // RevoMini1 supporetd by RevoMini2 firmware
            // These firmwares are designed to be backwards compatible
        } else if (firmwareBoard != board) {
            status("Error: firmware does not match board", STATUSICON_FAIL);
            updateButtons(true);
            return;
        }
        // Check the firmware embedded in the file:
        QByteArray firmwareHash = desc.mid(40, 20);
        QByteArray fileHash     = QCryptographicHash::hash(loadedFW.left(loadedFW.length() - 100), QCryptographicHash::Sha1);
        if (firmwareHash != fileHash) {
            status("Error: firmware file corrupt", STATUSICON_FAIL);
            updateButtons(true);
            return;
        }
    } else {
        // The firmware is not packaged, just upload the text in the description field
        // if it is there.
        descriptionArray.clear();
    }

    status("Starting firmware upload", STATUSICON_RUNNING);
    emit uploadStarted();

    // We don't know which device was used previously, so we
    // are cautious and reenter DFU for this deviceID:
    if (!m_dfu->enterDFU(deviceID)) {
        emit uploadEnded(false);
        status("Error:Could not enter DFU mode", STATUSICON_FAIL);
        updateButtons(true);
        return;
    }
    OP_DFU::Status ret = m_dfu->StatusRequest();
    qDebug() << m_dfu->StatusToString(ret);
    m_dfu->AbortOperation(); // Necessary, otherwise I get random failures.

    connect(m_dfu, SIGNAL(progressUpdated(int)), this, SLOT(setProgress(int)));
    connect(m_dfu, SIGNAL(operationProgress(QString)), this, SLOT(dfuStatus(QString)));
    connect(m_dfu, SIGNAL(uploadFinished(OP_DFU::Status)), this, SLOT(uploadFinished(OP_DFU::Status)));
    bool retstatus = m_dfu->UploadFirmware(filename, verify, deviceID);
    if (!retstatus) {
        emit uploadEnded(false);
        status("Could not start upload!", STATUSICON_FAIL);
        updateButtons(true);
        return;
    }

    status("Uploading, please wait...", STATUSICON_RUNNING);
}

/**
   Retrieves the firmware from the device
 */
void DeviceWidget::downloadFirmware()
{
    // clear progress bar now
    // this avoids displaying an error message and the progress at 100% at the same time
    setProgress(0);
    updateButtons(false);

    if (!m_dfu->devices[deviceID].Readable) {
        myDevice->statusLabel->setText(QString("Device not readable!"));
        status("Device not readable!", STATUSICON_FAIL);
        updateButtons(true);
        return;
    }

    filename = setSaveFileName();
    if (filename.isEmpty()) {
        status("Empty filename", STATUSICON_FAIL);
        updateButtons(true);
        return;
    }

    status("Starting firmware download", STATUSICON_RUNNING);
    emit downloadStarted();

    connect(m_dfu, SIGNAL(progressUpdated(int)), this, SLOT(setProgress(int)));
    connect(m_dfu, SIGNAL(downloadFinished()), this, SLOT(downloadFinished()));

    downloadedFirmware.clear(); // Empty the byte array
    bool ret = m_dfu->DownloadFirmware(&downloadedFirmware, deviceID);

    if (!ret) {
        emit downloadEnded(false);
        status("Could not start download!", STATUSICON_FAIL);
        updateButtons(true);
        return;
    }

    status("Downloading, please wait...", STATUSICON_RUNNING);
}

/**
   Callback for the firmware download result
 */
void DeviceWidget::downloadFinished()
{
    disconnect(m_dfu, SIGNAL(downloadFinished()), this, SLOT(downloadFinished()));
    disconnect(m_dfu, SIGNAL(progressUpdated(int)), this, SLOT(setProgress(int)));

    // Now save the result (use the utility function from OP_DFU)
    m_dfu->SaveByteArrayToFile(filename, downloadedFirmware);

    emit downloadEnded(true);
    status("Download successful", STATUSICON_OK);
    updateButtons(true);
}

/**
   Callback for the firmware upload result
 */
void DeviceWidget::uploadFinished(OP_DFU::Status retstatus)
{
    disconnect(m_dfu, SIGNAL(uploadFinished(OP_DFU::Status)), this, SLOT(uploadFinished(OP_DFU::Status)));
    disconnect(m_dfu, SIGNAL(progressUpdated(int)), this, SLOT(setProgress(int)));
    disconnect(m_dfu, SIGNAL(operationProgress(QString)), this, SLOT(dfuStatus(QString)));

    if (retstatus != OP_DFU::Last_operation_Success) {
        emit uploadEnded(false);
        status(QString("Upload failed with code: ") + m_dfu->StatusToString(retstatus).toLatin1().data(), STATUSICON_FAIL);
        updateButtons(true);
        return;
    } else if (!descriptionArray.isEmpty()) {
        // We have a structured array to save
        status(QString("Updating description"), STATUSICON_RUNNING);
        repaint(); // Make sure the text above shows right away
        retstatus = m_dfu->UploadDescription(descriptionArray);
        if (retstatus != OP_DFU::Last_operation_Success) {
            emit uploadEnded(false);
            status(QString("Upload failed with code: ") + m_dfu->StatusToString(retstatus).toLatin1().data(), STATUSICON_FAIL);
            updateButtons(true);
            return;
        }
    } else if (!myDevice->description->text().isEmpty()) {
        // Fallback: we save the description field:
        status(QString("Updating description"), STATUSICON_RUNNING);
        repaint(); // Make sure the text above shows right away
        retstatus = m_dfu->UploadDescription(myDevice->description->text());
        if (retstatus != OP_DFU::Last_operation_Success) {
            emit uploadEnded(false);
            status(QString("Upload failed with code: ") + m_dfu->StatusToString(retstatus).toLatin1().data(), STATUSICON_FAIL);
            updateButtons(true);
            return;
        }
    }

    populate();

    emit uploadEnded(true);
    status("Upload successful", STATUSICON_OK);
    updateButtons(true);
}

/**
   Slot to update the progress bar
 */
void DeviceWidget::setProgress(int percent)
{
    myDevice->progressBar->setValue(percent);
}

QString DeviceWidget::getDevFirmwarePath()
{
    QDir fwDirectory;
    QString fwDirectoryStr;

    fwDirectoryStr = QCoreApplication::applicationDirPath();
    fwDirectory    = QDir(fwDirectoryStr);
#ifdef Q_OS_WIN
    fwDirectory.cd("../..");
    fwDirectoryStr = fwDirectory.absolutePath();
#elif defined Q_OS_LINUX
    fwDirectory.cd("../..");
    fwDirectoryStr = fwDirectory.absolutePath();
#elif defined Q_OS_MAC
    fwDirectory.cd("../../../../../..");
    fwDirectoryStr = fwDirectory.absolutePath();
#endif
    fwDirectoryStr = fwDirectoryStr + "/fw_" + myDevice->lblBrdName->text().toLower() + "/fw_" + myDevice->lblBrdName->text().toLower() + ".opfw";
    return fwDirectoryStr;
}

/**
 * Opens an open file dialog.
 */
QString DeviceWidget::setOpenFileName()
{
    QString fwDirectoryStr = getDevFirmwarePath();

    // Format filename for file chooser
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    tr("Select firmware file"),
                                                    fwDirectoryStr,
                                                    tr("Firmware Files (*.opfw *.bin)"));

    return fileName;
}

/**
 * Set the save file name
 */
QString DeviceWidget::setSaveFileName()
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

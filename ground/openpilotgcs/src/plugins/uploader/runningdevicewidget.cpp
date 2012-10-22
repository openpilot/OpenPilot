/**
 ******************************************************************************
 *
 * @file       runningdevicewidget.cpp
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
#include "runningdevicewidget.h"
#include "devicedescriptorstruct.h"
#include "uploadergadgetwidget.h"
runningDeviceWidget::runningDeviceWidget(QWidget *parent) :
    QWidget(parent)
{
    myDevice = new Ui_runningDeviceWidget();
    myDevice->setupUi(this);

    // Initialization of the Device icon display
    myDevice->devicePicture->setScene(new QGraphicsScene(this));

}


void runningDeviceWidget::showEvent(QShowEvent *event)
{
    Q_UNUSED(event)
    // Thit fitInView method should only be called now, once the
    // widget is shown, otherwise it cannot compute its values and
    // the result is usually a ahrsbargraph that is way too small.
    myDevice->devicePicture->fitInView(devicePic.rect(),Qt::KeepAspectRatio);
}

void runningDeviceWidget::resizeEvent(QResizeEvent* event)
{
    Q_UNUSED(event);
    myDevice->devicePicture->fitInView(devicePic.rect(), Qt::KeepAspectRatio);
}

/**
  Fills the various fields for the device
  */
void runningDeviceWidget::populate()
{

    ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
    UAVObjectUtilManager* utilMngr = pm->getObject<UAVObjectUtilManager>();
    int id = utilMngr->getBoardModel();

    myDevice->lblDeviceID->setText(QString("Device ID: ") + QString::number(id, 16));
    myDevice->lblBoardName->setText(deviceDescriptorStruct::idToBoardName(id));
    myDevice->lblHWRev->setText(QString(tr("HW Revision: "))+QString::number(id & 0x00FF, 16));
    qDebug()<<"CRC"<<utilMngr->getFirmwareCRC();
    myDevice->lblCRC->setText(QString(tr("Firmware CRC: "))+QVariant(utilMngr->getFirmwareCRC()).toString());
    // DeviceID tells us what sort of HW we have detected:
    // display a nice icon:
    myDevice->devicePicture->scene()->clear();

    switch (id) {
    case 0x0101:
        devicePic.load("");//TODO
        break;
    case 0x0201:
        devicePic.load("");//TODO
        break;
    case 0x0301:
        devicePic.load(":/uploader/images/pipx.png");
        break;
    case 0x0401:
        devicePic.load(":/uploader/images/gcs-board-cc.png");
        break;
    case 0x0402:
        devicePic.load(":/uploader/images/gcs-board-cc3d.png");
        break;
    default:
        break;
    }
    myDevice->devicePicture->scene()->addPixmap(devicePic);
    myDevice->devicePicture->setSceneRect(devicePic.rect());
    myDevice->devicePicture->fitInView(devicePic.rect(),Qt::KeepAspectRatio);

    QString serial = utilMngr->getBoardCPUSerial().toHex();
     myDevice->CPUSerial->setText(serial);

    QByteArray description = utilMngr->getBoardDescription();
    deviceDescriptorStruct devDesc;
    if(UAVObjectUtilManager::descriptionToStructure(description,devDesc))
    {
        if(devDesc.gitTag.startsWith("RELEASE",Qt::CaseSensitive))
        {
            myDevice->lblFWTag->setText(QString("Firmware tag: ")+devDesc.gitTag);
            QPixmap pix = QPixmap(QString(":uploader/images/application-certificate.svg"));
            myDevice->lblCertified->setPixmap(pix);
            myDevice->lblCertified->setToolTip(tr("Tagged officially released firmware build"));

        }
        else
        {
            myDevice->lblFWTag->setText(QString("Firmware tag: ")+devDesc.gitTag);
            QPixmap pix = QPixmap(QString(":uploader/images/warning.svg"));
            myDevice->lblCertified->setPixmap(pix);
            myDevice->lblCertified->setToolTip(tr("Untagged or custom firmware build"));
        }
        myDevice->lblGitCommitTag->setText("Git commit hash: "+devDesc.gitHash);
        myDevice->lblFWDate->setText(QString("Firmware date: ") + devDesc.gitDate.insert(4,"-").insert(7,"-"));
    }
    else
    {

        myDevice->lblFWTag->setText(QString("Firmware tag: ")+QString(description).left(QString(description).indexOf(QChar(255))));
        myDevice->lblGitCommitTag->setText("Git commit tag: Unknown");
        myDevice->lblFWDate->setText(QString("Firmware date: Unknown"));
        QPixmap pix = QPixmap(QString(":uploader/images/warning.svg"));
        myDevice->lblCertified->setPixmap(pix);
        myDevice->lblCertified->setToolTip(tr("Custom Firmware Build"));
    }
    //status("Ready...", STATUSICON_INFO);
}


/**
  Updates status message
  */
/*
void runningDeviceWidget::status(QString str, StatusIcon ic)
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
*/

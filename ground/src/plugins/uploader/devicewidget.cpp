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
    myDevice->deviceID->setText(QString("ID: ") + QString::number(id));
    bool r = m_dfu->devices[deviceID].Readable;
    bool w = m_dfu->devices[deviceID].Writable;
    myDevice->deviceACL->setText(r ? "R" : "-" +  w ? "W" : "-");
    int size=((OP_DFU::device)m_dfu->devices[deviceID]).SizeOfDesc;
    m_dfu->enterDFU(deviceID);
    QString str = m_dfu->DownloadDescription(size);
    myDevice->description->setMaxLength(size);
    myDevice->description->setText(str.left(str.indexOf(QChar(255))));


}

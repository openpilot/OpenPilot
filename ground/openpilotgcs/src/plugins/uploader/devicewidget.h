/**
 ******************************************************************************
 *
 * @file       devicewidget.h
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

#ifndef DEVICEWIDGET_H
#define DEVICEWIDGET_H

#include "ui_devicewidget.h"
#include "uploadergadgetwidget.h"
#include "op_dfu.h"
#include <QWidget>
#include <QFileDialog>
#include <QErrorMessage>
#include <QByteArray>
#include <QtSvg/QGraphicsSvgItem>
#include <QtSvg/QSvgRenderer>
#include <QCryptographicHash>
#include "uavobjectutilmanager.h"
#include "devicedescriptorstruct.h"
#include <QDir>
#include <QCoreApplication>
#include "uploader_global.h"

using namespace OP_DFU;
class UPLOADER_EXPORT deviceWidget : public QWidget
{
    Q_OBJECT
public:
    deviceWidget( QWidget *parent = 0);
    void setDeviceID(int devID);
    void setDfu(DFUObject* dfu);
    void populate();
    void freeze();
    typedef enum { STATUSICON_OK, STATUSICON_RUNNING, STATUSICON_FAIL, STATUSICON_INFO} StatusIcon;
    QString setOpenFileName();
    QString setSaveFileName();
private:
    deviceDescriptorStruct onBoardDescription;
    deviceDescriptorStruct LoadedDescription;
    QByteArray loadedFW;
    Ui_deviceWidget *myDevice;
    int deviceID;
    DFUObject *m_dfu;
    QByteArray downloadedFirmware;
    QString filename;
    QPixmap devicePic;
    QByteArray descriptionArray;
    void status(QString str, StatusIcon ic);
    bool populateBoardStructuredDescription(QByteArray arr);
    bool populateLoadedStructuredDescription(QByteArray arr);

signals:
    void uploadStarted();
    void uploadEnded(bool success);
public slots:
    void uploadFirmware();
    void loadFirmware();
    void downloadFirmware();
    void setProgress(int);
    void downloadFinished();
    void uploadFinished(OP_DFU::Status);
    void dfuStatus(QString);
    void confirmCB(int);

protected:
    void showEvent(QShowEvent *event);
    void resizeEvent(QResizeEvent *event);

};

#endif // DEVICEWIDGET_H

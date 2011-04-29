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
#include <QtSvg/QGraphicsSvgItem>
#include <QtSvg/QSvgRenderer>

using namespace OP_DFU;

class deviceWidget : public QWidget
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
    Ui_deviceWidget *myDevice;
    int deviceID;
    DFUObject *m_dfu;
    QByteArray downloadedFirmware;
    QString filename;
    QGraphicsSvgItem *devicePic;
    void status(QString str, StatusIcon ic);


signals:

public slots:
    void verifyFirmware();
    void uploadFirmware();
    void downloadFirmware();
    void setProgress(int);
    void downloadFinished();
    void uploadFinished(OP_DFU::Status);
    void dfuStatus(QString);

protected:
    void showEvent(QShowEvent *event);
    void resizeEvent(QResizeEvent *event);

};

#endif // DEVICEWIDGET_H

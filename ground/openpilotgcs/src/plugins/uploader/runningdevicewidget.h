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

#ifndef RUNNINGDEVICEWIDGET_H
#define RUNNINGDEVICEWIDGET_H

#include "ui_runningdevicewidget.h"

#include <QWidget>
#include <QErrorMessage>
#include <QtSvg/QGraphicsSvgItem>
#include <QtSvg/QSvgRenderer>
#include "uavtalk/telemetrymanager.h"
#include "extensionsystem/pluginmanager.h"
#include "uavobjectmanager.h"
#include "uavobject.h"
#include "uavobjectutilmanager.h"
#include "uploader_global.h"

class UPLOADER_EXPORT runningDeviceWidget : public QWidget
{
    Q_OBJECT
public:
    runningDeviceWidget( QWidget *parent = 0);
    void populate();
    void freeze();
    QString setOpenFileName();
    QString setSaveFileName();
    typedef enum { STATUSICON_OK, STATUSICON_RUNNING, STATUSICON_FAIL, STATUSICON_INFO} StatusIcon;

private:
    Ui_runningDeviceWidget *myDevice;
    int deviceID;
    QPixmap devicePic;
    //void status(QString str, StatusIcon ic);


signals:

protected:
    void showEvent(QShowEvent *event);
    void resizeEvent(QResizeEvent *event);

};

#endif // RUNNINGDEVICEWIDGET_H

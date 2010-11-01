/**
 ******************************************************************************
 *
 * @file       uploadergadgetwidget.h
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

#ifndef UPLOADERGADGETWIDGET_H
#define UPLOADERGADGETWIDGET_H

#include "ui_uploader.h"
#include "delay.h"

#include "extensionsystem/pluginmanager.h"
#include "uavobjects/uavobjectmanager.h"
#include "uavobjects/uavobject.h"

#include "rawhid/rawhidplugin.h"
#include <QtGui/QWidget>
#include <qextserialport/src/qextserialport.h>
#include <qymodem/src/qymodemsend.h>
#include <QLabel>
#include <QLineEdit>
#include <QThread>
#include <QProgressBar>
#include <QStringList>
#include <QFileDialog>
#include <QMessageBox>
//using namespace qmapcontrol;


class UploaderGadgetWidget : public QWidget
{
    Q_OBJECT

public:
    UploaderGadgetWidget(QWidget *parent = 0);
   ~UploaderGadgetWidget();
    typedef enum { IAP_STATE_READY, IAP_STATE_STEP_1, IAP_STATE_STEP_2, IAP_STEP_RESET} IAPStep;

   void setPort(QextSerialPort* port);


private:
     Ui_UploaderWidget *m_config;
     IAPStep currentStep;
     bool resetOnly;
     void log(QString str);

     QLineEdit* openFileNameLE;
     QextSerialPort *Port;
     QLabel* status;
     QymodemSend * Ymodem;
     QProgressBar *progressBar;
     QTimer *timer;

private slots:
    void setOpenFileName();
    void send();
    void error(QString errorString,int errorNumber);
    void info(QString infoString,int infoNumber);
    void updatePercSlot(int);
    void goToBootloader(UAVObject* = NULL, bool = false);
    void systemReset();
};

#endif // UPLOADERGADGETWIDGET_H

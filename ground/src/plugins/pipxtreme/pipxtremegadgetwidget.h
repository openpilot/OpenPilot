/**
 ******************************************************************************
 *
 * @file       pipxtremegadgetwidget.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @{
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

#ifndef PIPXTREMEGADGETWIDGET_H
#define PIPXTREMEGADGETWIDGET_H

#include "ui_pipxtreme.h"
#include "delay.h"
//#include "op_dfu.h"

#include <qextserialport.h>
#include <qextserialenumerator.h>

#include "uavtalk/telemetrymanager.h"
#include "extensionsystem/pluginmanager.h"
#include "uavobjects/uavobjectmanager.h"
#include "uavobjects/uavobject.h"
#include "rawhid/rawhidplugin.h"

#include <QtGui/QWidget>
#include <QLabel>
#include <QLineEdit>
#include <QThread>
#include <QMessageBox>
#include <QTimer>

//using namespace OP_DFU;

class PipXtremeGadgetWidget : public QWidget
{
    Q_OBJECT

public:
    PipXtremeGadgetWidget(QWidget *parent = 0);
   ~PipXtremeGadgetWidget();

    typedef enum { IAP_STATE_READY, IAP_STATE_STEP_1} IAPStep;
    typedef enum { RESCUE_STEP0, RESCUE_STEP1, RESCUE_STEP2, RESCUE_STEP3, RESCUE_POWER1, RESCUE_POWER2, RESCUE_DETECT } RescueStep;

public slots:
    void onModemConnect();
    void onModemDisconnect();

protected:
    void resizeEvent(QResizeEvent *event);

private:
     Ui_PipXtremeWidget *m_config;
//     DFUObject *dfu;
     IAPStep currentStep;
     RescueStep rescueStep;
     bool resetOnly;

     QString getPortDevice(const QString &friendName);

private slots:
    void error(QString errorString,int errorNumber);
    void goToAPIMode(UAVObject* = NULL, bool = false);
    void systemReset();
    void systemBoot();
    void getSerialPorts();
};

#endif

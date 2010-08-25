/**
 ******************************************************************************
 *
 * @file       configahrstwidget.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup ConfigPlugin Config Plugin
 * @{
 * @brief Telemetry configuration panel
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
#ifndef CONFIGAHRSWIDGET_H
#define CONFIGAHRSWIDGET_H

#include "ui_ahrs.h"
#include "configtaskwidget.h"
#include "extensionsystem/pluginmanager.h"
#include "uavobjects/uavobjectmanager.h"
#include "uavobjects/uavobject.h"
#include <QtGui/QWidget>
#include <QtSvg/QSvgRenderer>
#include <QtSvg/QGraphicsSvgItem>
#include <QList>


class ConfigAHRSWidget: public ConfigTaskWidget
{
    Q_OBJECT

public:
    ConfigAHRSWidget(QWidget *parent = 0);
    ~ConfigAHRSWidget();

private:
    Ui_AHRSWidget *m_ahrs;
    QGraphicsSvgItem *ahrsbargraph;
    QGraphicsSvgItem *accel_x;
    QGraphicsSvgItem *accel_y;
    QGraphicsSvgItem *accel_z;
    QGraphicsSvgItem *gyro_x;
    QGraphicsSvgItem *gyro_y;
    QGraphicsSvgItem *gyro_z;
    QGraphicsSvgItem *mag_x;
    QGraphicsSvgItem *mag_y;
    QGraphicsSvgItem *mag_z;
    double maxBarHeight;
    int phaseCounter;
    const static double maxVarValue = 0.1;

private slots:
    void launchAHRSCalibration();
    void saveAHRSCalibration();
    void calibPhase2();

protected:
   void showEvent(QShowEvent *event);


};

#endif // ConfigAHRSWidget_H

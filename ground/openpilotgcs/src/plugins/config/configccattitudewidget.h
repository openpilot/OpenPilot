/**
 ******************************************************************************
 *
 * @file       configccattitudewidget.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup ConfigPlugin Config Plugin
 * @{
 * @brief Configure the properties of the attitude module in CopterControl
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
#ifndef CCATTITUDEWIDGET_H
#define CCATTITUDEWIDGET_H

#include "ui_ccattitude.h"
#include "configtaskwidget.h"
#include "extensionsystem/pluginmanager.h"
#include "uavobjectmanager.h"
#include "uavobject.h"
#include <QtGui/QWidget>
#include <QTimer>
#include <QMutex>

class Ui_Widget;

class ConfigCCAttitudeWidget : public ConfigTaskWidget
{
    Q_OBJECT

public:
    explicit ConfigCCAttitudeWidget(QWidget *parent = 0);
    ~ConfigCCAttitudeWidget();

private slots:
    void attitudeRawUpdated(UAVObject * obj);
    void timeout();
    void startAccelCalibration();
    void saveAttitudeSettings();
    void applyAttitudeSettings();
    void getCurrentAttitudeSettings();

private:
    QMutex startStop;
    Ui_ccattitude *ui;
    QTimer timer;
    UAVObject::Metadata initialMdata;

    int updates;

    QList<double> x_accum, y_accum, z_accum;

    static const int NUM_ACCEL_UPDATES = 10;
    static const float ACCEL_SCALE = 0.004f * 9.81f;
};

#endif // CCATTITUDEWIDGET_H

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
#include "uavobjectmanager.h"
#include "uavobject.h"
#include <QtGui/QWidget>
#include <QtSvg/QSvgRenderer>
#include <QtSvg/QGraphicsSvgItem>
#include <QList>
#include <QTimer>
#include <QMutex>

class ConfigAHRSWidget: public ConfigTaskWidget
{
    Q_OBJECT

public:
    ConfigAHRSWidget(QWidget *parent = 0);
    ~ConfigAHRSWidget();

private:
    void drawVariancesGraph();
    void displayPlane(QString elementID);

    Ui_AHRSWidget *m_ahrs;
    QGraphicsSvgItem *paperplane;
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
    QMutex attitudeRawUpdateLock;
    double maxBarHeight;
    int phaseCounter;
    int progressBarIndex;
    QTimer progressBarTimer;
    const static double maxVarValue;
    const static int calibrationDelay;

    bool collectingData;

    QList<double> gyro_accum_x;
    QList<double> gyro_accum_y;
    QList<double> gyro_accum_z;
    QList<double> accel_accum_x;
    QList<double> accel_accum_y;
    QList<double> accel_accum_z;
    QList<double> mag_accum_x;
    QList<double> mag_accum_y;
    QList<double> mag_accum_z;

    double accel_data_x[6];
    double accel_data_y[6];
    double accel_data_z[6];
    double mag_data_x[6];
    double mag_data_y[6];
    double mag_data_z[6];
    int position;


    UAVObject::Metadata initialMdata;

    double listMean(QList<double> list);
    void computeGyroDrift();

private slots:
    void enableHomeLocSave(UAVObject *obj);
    void launchAHRSCalibration();
    void saveAHRSCalibration();
    void launchAccelBiasCalibration();
    void calibPhase2();
    void incrementProgress();
    void ahrsSettingsRequest();
    void ahrsSettingsSaveRAM();
    void ahrsSettingsSaveSD();
    void savePositionData();
    void computeScaleBias();
    void sixPointCalibrationMode();
    void attitudeRawUpdated(UAVObject * obj);
    void accelBiasattitudeRawUpdated(UAVObject*);
    void driftCalibrationAttitudeRawUpdated(UAVObject*);
    void launchGyroDriftCalibration();

protected:
    void showEvent(QShowEvent *event);
    void resizeEvent(QResizeEvent *event);

};

#endif // ConfigAHRSWidget_H

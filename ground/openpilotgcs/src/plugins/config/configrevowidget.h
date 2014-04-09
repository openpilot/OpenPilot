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
#ifndef CONFIGREVOWIDGET_H
#define CONFIGREVOWIDGET_H

#include "ui_revosensors.h"
#include "configtaskwidget.h"

#include "../uavobjectwidgetutils/configtaskwidget.h"
#include "extensionsystem/pluginmanager.h"
#include "uavobjectmanager.h"
#include "uavobject.h"
#include <QWidget>
#include <QtSvg/QSvgRenderer>
#include <QtSvg/QGraphicsSvgItem>
#include <QList>
#include <QTimer>
#include <QMutex>
#include "calibration/thermal/thermalcalibrationmodel.h"
class Ui_Widget;


class ConfigRevoWidget : public ConfigTaskWidget {
    Q_OBJECT
    typedef struct {
        RevoCalibration::DataFields revoCalibration;
        AccelGyroSettings::DataFields accelGyroSettings;
    } SavedSettings;
public:
    ConfigRevoWidget(QWidget *parent = 0);
    ~ConfigRevoWidget();

private:
    void displayPlane(QGraphicsView *view, QString elementID);

    // ! Computes the scale and bias of the mag based on collected data
    void computeScaleBias(bool mag, bool accel);

    SavedSettings savedSettings;

    OpenPilot::ThermalCalibrationModel *m_thermalCalibrationModel;
    Ui_RevoSensorsWidget *m_ui;
    QMutex sensorsUpdateLock;
    double maxBarHeight;
    int phaseCounter;
    const static double maxVarValue;
    const static int calibrationDelay = 10;

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

    double accel_data_x[6], accel_data_y[6], accel_data_z[6];
    double mag_data_x[6], mag_data_y[6], mag_data_z[6];

    bool calibratingMag = false;
    bool calibratingAccel = false;

    UAVObject::Metadata initialAccelStateMdata;
    UAVObject::Metadata initialGyroStateMdata;
    UAVObject::Metadata initialMagStateMdata;
    UAVObject::Metadata initialBaroSensorMdata;
    float initialMagCorrectionRate;

    int position;

    static const int NOISE_SAMPLES = 100;

    // Board rotation store/recall
    qint16 storedBoardRotation[3];
    bool isBoardRotationStored;
    void storeAndClearBoardRotation();
    void recallBoardRotation();


private slots:
    // ! Overriden method from the configTaskWidget to update UI
    virtual void refreshWidgetsValues(UAVObject *object = NULL);

    // Slots for calibrating the mags
    void doStartSixPointCalibrationMag();
    void doStartSixPointCalibrationAccel();
    void doStartSixPointCalibration(bool calibrateAccel, bool calibrateMag);
    void doGetSixPointCalibrationMeasurement(UAVObject *obj);
    void savePositionData();

    // Slots for calibrating the accel and gyro
    void doStartAccelGyroBiasCalibration();


    void doGetAccelGyroBiasData(UAVObject *);

    // Slot for clearing home location
    void clearHomeLocation();

protected:
    void showEvent(QShowEvent *event);
    void resizeEvent(QResizeEvent *event);
};

#endif // ConfigRevoWidget_H

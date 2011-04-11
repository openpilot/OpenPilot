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

#include <Eigen/StdVector>

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

#include <Eigen/Core>

class ConfigAHRSWidget: public ConfigTaskWidget
{
    Q_OBJECT

public:
    ConfigAHRSWidget(QWidget *parent = 0);
    ~ConfigAHRSWidget();
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW
    
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

    // TODO: Store these in std::vectors
    Eigen::Vector3f gyro_data[60];
    Eigen::Vector3f accel_data[60];
    Eigen::Vector3f mag_data[60];
    int position;
    int n_positions;

    UAVObject::Metadata initialMdata;

    void computeGyroDrift();


    // Saved parameters for calibration.  In the event of a calibration failure,
    // the old parameters will be restored.
    Eigen::Vector3f saved_gyro_bias;

    Eigen::Vector3f saved_accel_bias;
    Eigen::Vector3f saved_accel_scale;
    Eigen::Vector3f saved_accel_ortho;
    Eigen::Vector3f saved_accel_rotation;

    Eigen::Vector3f saved_mag_scale;
    Eigen::Vector3f saved_mag_bias;

    bool
    updateScaleFactors(UAVObjectField *scale,
    		UAVObjectField *bias ,
    		UAVObjectField *ortho,
    		const Eigen::Matrix3f& updateScale,
    		const Eigen::Vector3f& updateBias,
    		const Eigen::Vector3f& oldScale,
    		const Eigen::Vector3f& oldBias,
    		const Eigen::Vector3f& oldOrtho = Eigen::Vector3f::Zero());

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
    void multiPointCalibrationMode();
//    void sixPointCalibrationMode();      // this function no longer exists
    void attitudeRawUpdated(UAVObject * obj);
    void accelBiasattitudeRawUpdated(UAVObject*);
    void driftCalibrationAttitudeRawUpdated(UAVObject*);
    void launchGyroDriftCalibration();
    void resetCalibrationDefaults();
    void cacheCurrentCalibration();

protected:
    void showEvent(QShowEvent *event);
    void resizeEvent(QResizeEvent *event);

};

#endif // ConfigAHRSWidget_H

/**
 ******************************************************************************
 *
 * @file       configccattitudewidget.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup ConfigPlugin Config Plugin
 * @{
 * @brief Configure Attitude module on CopterControl
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
#include "configccattitudewidget.h"
#include "ui_ccattitude.h"
#include "utils/coordinateconversions.h"
#include "attitudesettings.h"
#include <QMutexLocker>
#include <QMessageBox>
#include <QDebug>
#include <QDesktopServices>
#include <QUrl>
#include "accels.h"
#include "gyros.h"
#include <extensionsystem/pluginmanager.h>
#include <coreplugin/generalsettings.h>

ConfigCCAttitudeWidget::ConfigCCAttitudeWidget(QWidget *parent) :
        ConfigTaskWidget(parent),
        ui(new Ui_ccattitude)
{
    ui->setupUi(this);
    forceConnectedState(); //dynamic widgets don't recieve the connected signal
    connect(ui->zeroBias,SIGNAL(clicked()),this,SLOT(startAccelCalibration()));

    ExtensionSystem::PluginManager *pm=ExtensionSystem::PluginManager::instance();
    Core::Internal::GeneralSettings * settings=pm->getObject<Core::Internal::GeneralSettings>();
    if(!settings->useExpertMode())
        ui->applyButton->setVisible(false);
    
    addApplySaveButtons(ui->applyButton,ui->saveButton);
    addUAVObject("AttitudeSettings");

    // Connect the help button
    connect(ui->ccAttitudeHelp, SIGNAL(clicked()), this, SLOT(openHelp()));
    addUAVObjectToWidgetRelation("AttitudeSettings","ZeroDuringArming",ui->zeroGyroBiasOnArming);

    addUAVObjectToWidgetRelation("AttitudeSettings","BoardRotation",ui->rollBias,AttitudeSettings::BOARDROTATION_ROLL);
    addUAVObjectToWidgetRelation("AttitudeSettings","BoardRotation",ui->pitchBias,AttitudeSettings::BOARDROTATION_PITCH);
    addUAVObjectToWidgetRelation("AttitudeSettings","BoardRotation",ui->yawBias,AttitudeSettings::BOARDROTATION_YAW);
    addWidget(ui->zeroBias);
    refreshWidgetsValues();
}

ConfigCCAttitudeWidget::~ConfigCCAttitudeWidget()
{
    delete ui;
}

void ConfigCCAttitudeWidget::sensorsUpdated(UAVObject * obj) {
    QMutexLocker locker(&startStop);

    ui->zeroBiasProgress->setValue((float) qMin(accelUpdates,gyroUpdates) / NUM_SENSOR_UPDATES * 100);

    Accels * accels = Accels::GetInstance(getObjectManager());
    Gyros * gyros = Gyros::GetInstance(getObjectManager());

    if(obj->getObjID() == Accels::OBJID && accelUpdates < NUM_SENSOR_UPDATES) {
        accelUpdates++;
        Accels::DataFields accelsData = accels->getData();
        x_accum.append(accelsData.x);
        y_accum.append(accelsData.y);
        z_accum.append(accelsData.z);
    } else if (obj->getObjID() == Gyros::OBJID && gyroUpdates < NUM_SENSOR_UPDATES) {
        gyroUpdates++;
        Gyros::DataFields gyrosData = gyros->getData();
        x_gyro_accum.append(gyrosData.x);
        y_gyro_accum.append(gyrosData.y);
        z_gyro_accum.append(gyrosData.z);
    } else if ( accelUpdates >= NUM_SENSOR_UPDATES && gyroUpdates >= NUM_SENSOR_UPDATES) {
        timer.stop();
        disconnect(obj,SIGNAL(objectUpdated(UAVObject*)),this,SLOT(sensorsUpdated(UAVObject*)));
        disconnect(&timer,SIGNAL(timeout()),this,SLOT(timeout()));

        float x_bias = listMean(x_accum) / ACCEL_SCALE;
        float y_bias = listMean(y_accum) / ACCEL_SCALE;
        float z_bias = (listMean(z_accum) + 9.81) / ACCEL_SCALE;

        float x_gyro_bias = listMean(x_gyro_accum) * 100.0f;
        float y_gyro_bias = listMean(y_gyro_accum) * 100.0f;
        float z_gyro_bias = listMean(z_gyro_accum) * 100.0f;
        accels->setMetadata(initialAccelsMdata);
        gyros->setMetadata(initialGyrosMdata);

        AttitudeSettings::DataFields attitudeSettingsData = AttitudeSettings::GetInstance(getObjectManager())->getData();
        // We offset the gyro bias by current bias to help precision
        attitudeSettingsData.AccelBias[0] += x_bias;
        attitudeSettingsData.AccelBias[1] += y_bias;
        attitudeSettingsData.AccelBias[2] += z_bias;
        attitudeSettingsData.GyroBias[0] = -x_gyro_bias;
        attitudeSettingsData.GyroBias[1] = -y_gyro_bias;
        attitudeSettingsData.GyroBias[2] = -z_gyro_bias;
        attitudeSettingsData.BiasCorrectGyro = AttitudeSettings::BIASCORRECTGYRO_TRUE;
        AttitudeSettings::GetInstance(getObjectManager())->setData(attitudeSettingsData);
        this->setDirty(true);
    } else {
        // Possible to get here if weird threading stuff happens.  Just ignore updates.
        qDebug("Unexpected accel update received.");
    }
}

void ConfigCCAttitudeWidget::timeout() {
    QMutexLocker locker(&startStop);
    UAVDataObject * obj = Accels::GetInstance(getObjectManager());
    disconnect(obj,SIGNAL(objectUpdated(UAVObject*)),this,SLOT(sensorsUpdated(UAVObject*)));
    disconnect(&timer,SIGNAL(timeout()),this,SLOT(timeout()));

    Accels * accels = Accels::GetInstance(getObjectManager());
    Gyros * gyros = Gyros::GetInstance(getObjectManager());
    accels->setMetadata(initialAccelsMdata);
    gyros->setMetadata(initialGyrosMdata);

    QMessageBox msgBox;
    msgBox.setText(tr("Calibration timed out before receiving required updates."));
    msgBox.setStandardButtons(QMessageBox::Ok);
    msgBox.setDefaultButton(QMessageBox::Ok);
    msgBox.exec();

}

void ConfigCCAttitudeWidget::startAccelCalibration() {
    QMutexLocker locker(&startStop);

    accelUpdates = 0;
    gyroUpdates = 0;
    x_accum.clear();
    y_accum.clear();
    z_accum.clear();
    x_gyro_accum.clear();
    y_gyro_accum.clear();
    z_gyro_accum.clear();

    // Disable gyro bias correction to see raw data
    AttitudeSettings::DataFields attitudeSettingsData = AttitudeSettings::GetInstance(getObjectManager())->getData();
    attitudeSettingsData.BiasCorrectGyro = AttitudeSettings::BIASCORRECTGYRO_FALSE;
    AttitudeSettings::GetInstance(getObjectManager())->setData(attitudeSettingsData);

    // Set up to receive updates
    UAVDataObject * accels = Accels::GetInstance(getObjectManager());
    UAVDataObject * gyros = Gyros::GetInstance(getObjectManager());
    connect(accels,SIGNAL(objectUpdated(UAVObject*)),this,SLOT(sensorsUpdated(UAVObject*)));
    connect(gyros,SIGNAL(objectUpdated(UAVObject*)),this,SLOT(sensorsUpdated(UAVObject*)));

    // Set up timeout timer
    timer.start(10000);
    connect(&timer,SIGNAL(timeout()),this,SLOT(timeout()));

    // Speed up updates
    initialAccelsMdata = accels->getMetadata();
    UAVObject::Metadata accelsMdata = initialAccelsMdata;
    UAVObject::SetFlightTelemetryUpdateMode(accelsMdata, UAVObject::UPDATEMODE_PERIODIC);
    accelsMdata.flightTelemetryUpdatePeriod = 30;
    accels->setMetadata(accelsMdata);

    initialGyrosMdata = gyros->getMetadata();
    UAVObject::Metadata gyrosMdata = initialGyrosMdata;
    UAVObject::SetFlightTelemetryUpdateMode(gyrosMdata, UAVObject::UPDATEMODE_PERIODIC);
    gyrosMdata.flightTelemetryUpdatePeriod = 30;
    gyros->setMetadata(gyrosMdata);

}

void ConfigCCAttitudeWidget::openHelp()
{

    QDesktopServices::openUrl( QUrl("http://wiki.openpilot.org/x/44Cf", QUrl::StrictMode) );
}

void ConfigCCAttitudeWidget::enableControls(bool enable)
{
    if(ui->zeroBias)
        ui->zeroBias->setEnabled(enable);
    ConfigTaskWidget::enableControls(enable);

}

void ConfigCCAttitudeWidget::updateObjectsFromWidgets()
{
    ConfigTaskWidget::updateObjectsFromWidgets();

    ui->zeroBiasProgress->setValue(0);
}

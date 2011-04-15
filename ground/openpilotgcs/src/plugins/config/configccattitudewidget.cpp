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
#include <QMutexLocker>
#include <QErrorMessage>
#include <QDebug>

ConfigCCAttitudeWidget::ConfigCCAttitudeWidget(QWidget *parent) :
        ConfigTaskWidget(parent),
        ui(new Ui_ccattitude)
{
    ui->setupUi(this);
    connect(ui->zeroBias,SIGNAL(clicked()),this,SLOT(startAccelCalibration()));
    connect(ui->saveButton,SIGNAL(clicked()),this,SLOT(saveAttitudeSettings()));
    connect(ui->rollBias,SIGNAL(valueChanged(int)),this,SLOT(attitudeBiasChanged(int)));
    connect(ui->pitchBias,SIGNAL(valueChanged(int)),this,SLOT(attitudeBiasChanged(int)));
    connect(ui->yawBias,SIGNAL(valueChanged(int)),this,SLOT(attitudeBiasChanged(int)));
}

ConfigCCAttitudeWidget::~ConfigCCAttitudeWidget()
{
    delete ui;
}

void ConfigCCAttitudeWidget::attitudeRawUpdated(UAVObject * obj) {
    QMutexLocker locker(&startStop);
    UAVDataObject * attitudeRaw = dynamic_cast<UAVDataObject*>(obj);

    ui->zeroBiasProgress->setValue((float) updates / NUM_ACCEL_UPDATES * 100);

    if(updates < NUM_ACCEL_UPDATES) {
        updates++;
        UAVObjectField * field = obj->getField(QString("accels"));
        x_accum.append(field->getDouble(0));
        y_accum.append(field->getDouble(1));
        z_accum.append(field->getDouble(2));
    } else {
        timer.stop();
        disconnect(obj,SIGNAL(objectUpdated(UAVObject*)),this,SLOT(attitudeRawUpdated(UAVObject*)));
        disconnect(&timer,SIGNAL(timeout()),this,SLOT(timeout()));

        float x_bias = listMean(x_accum) / ACCEL_SCALE;
        float y_bias = listMean(y_accum) / ACCEL_SCALE;
        float z_bias = (listMean(z_accum) + 9.81) / ACCEL_SCALE;

        obj->setMetadata(initialMdata);

        UAVDataObject * settings = dynamic_cast<UAVDataObject*>(getObjectManager()->getObject(QString("AttitudeSettings")));
        UAVObjectField * field = settings->getField("AccelBias");
        field->setDouble(field->getDouble(0) + x_bias,0);
        field->setDouble(field->getDouble(1) + y_bias,1);
        field->setDouble(field->getDouble(2) + z_bias,2);
        settings->updated();
        ui->status->setText("Calibration done.");

    }
}

void ConfigCCAttitudeWidget::timeout() {
    QMutexLocker locker(&startStop);
    UAVObjectManager * objMngr = getObjectManager();
    UAVDataObject * obj = dynamic_cast<UAVDataObject*>(getObjectManager()->getObject(QString("AttitudeRaw")));
    disconnect(obj,SIGNAL(objectUpdated(UAVObject*)),this,SLOT(attitudeRawUpdated(UAVObject*)));
    disconnect(&timer,SIGNAL(timeout()),this,SLOT(timeout()));

    QErrorMessage errmsg;
    errmsg.showMessage("Calibration timed out before receiving required updates");
    errmsg.exec();
}

void ConfigCCAttitudeWidget::attitudeBiasChanged(int val) {
    UAVDataObject * settings = dynamic_cast<UAVDataObject*>(getObjectManager()->getObject(QString("AttitudeSettings")));
    UAVObjectField * field = settings->getField("RotationQuaternion");

    float RPY[3], q[4];
    RPY[0] = ui->rollBias->value();
    RPY[1] = ui->pitchBias->value();
    RPY[2] = ui->yawBias->value();

    Utils::CoordinateConversions().RPY2Quaternion(RPY,q);

    field->setDouble(q[0]*127,0);
    field->setDouble(q[1]*127,1);
    field->setDouble(q[2]*127,2);
    field->setDouble(q[3]*127,3);

    settings->updated();
}

void ConfigCCAttitudeWidget::startAccelCalibration() {
    QMutexLocker locker(&startStop);

    updates = 0;
    x_accum.clear();
    y_accum.clear();
    z_accum.clear();

    ui->status->setText("Calibrating...");

    // Set up to receive updates
    UAVDataObject * obj = dynamic_cast<UAVDataObject*>(getObjectManager()->getObject(QString("AttitudeRaw")));
    connect(obj,SIGNAL(objectUpdated(UAVObject*)),this,SLOT(attitudeRawUpdated(UAVObject*)));

    // Set up timeout timer
    timer.start(2000);
    connect(&timer,SIGNAL(timeout()),this,SLOT(timeout()));

    // Speed up updates
    initialMdata = obj->getMetadata();
    UAVObject::Metadata mdata = initialMdata;
    mdata.flightTelemetryUpdateMode = UAVObject::UPDATEMODE_PERIODIC;
    mdata.flightTelemetryUpdatePeriod = 100;
    obj->setMetadata(mdata);

}

void ConfigCCAttitudeWidget::saveAttitudeSettings() {
    UAVDataObject * obj = dynamic_cast<UAVDataObject*>(getObjectManager()->getObject(QString("AttitudeSettings")));
    saveObjectToSD(obj);
}

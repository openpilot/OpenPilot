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
#include "accelstate.h"
#include "accelgyrosettings.h"
#include "gyrostate.h"
#include <extensionsystem/pluginmanager.h>
#include <coreplugin/generalsettings.h>

ConfigCCAttitudeWidget::ConfigCCAttitudeWidget(QWidget *parent) :
    ConfigTaskWidget(parent),
    ui(new Ui_ccattitude)
{
    ui->setupUi(this);
    connect(ui->zeroBias, SIGNAL(clicked()), this, SLOT(startAccelCalibration()));

    ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
    Core::Internal::GeneralSettings *settings = pm->getObject<Core::Internal::GeneralSettings>();
    if (!settings->useExpertMode()) {
        ui->applyButton->setVisible(false);
    }

    addApplySaveButtons(ui->applyButton, ui->saveButton);
    addUAVObject("AttitudeSettings");
    addUAVObject("AccelGyroSettings");

    // Connect the help button
    connect(ui->ccAttitudeHelp, SIGNAL(clicked()), this, SLOT(openHelp()));

    addWidgetBinding("AttitudeSettings", "ZeroDuringArming", ui->zeroGyroBiasOnArming);
    addWidgetBinding("AttitudeSettings", "AccelTau", ui->accelTauSpinbox);

    addWidgetBinding("AttitudeSettings", "BoardRotation", ui->rollBias, AttitudeSettings::BOARDROTATION_ROLL);
    addWidgetBinding("AttitudeSettings", "BoardRotation", ui->pitchBias, AttitudeSettings::BOARDROTATION_PITCH);
    addWidgetBinding("AttitudeSettings", "BoardRotation", ui->yawBias, AttitudeSettings::BOARDROTATION_YAW);
    addWidget(ui->zeroBias);
    populateWidgets();
    refreshWidgetsValues();
    forceConnectedState();
}

ConfigCCAttitudeWidget::~ConfigCCAttitudeWidget()
{
    delete ui;
}

void ConfigCCAttitudeWidget::sensorsUpdated(UAVObject *obj)
{
    if (!timer.isActive()) {
        // ignore updates that come in after the timer has expired
        return;
    }

    AccelState *accelState = AccelState::GetInstance(getObjectManager());
    GyroState *gyroState   = GyroState::GetInstance(getObjectManager());

    // Accumulate samples until we have _at least_ NUM_SENSOR_UPDATES samples
    // for both gyros and accels.
    // Note that, at present, we stash the samples and then compute the bias
    // at the end, even though the mean could be accumulated as we go.
    // In future, a better algorithm could be used.
    if (obj->getObjID() == AccelState::OBJID) {
        accelUpdates++;
        AccelState::DataFields accelStateData = accelState->getData();
        x_accum.append(accelStateData.x);
        y_accum.append(accelStateData.y);
        z_accum.append(accelStateData.z);
    } else if (obj->getObjID() == GyroState::OBJID) {
        gyroUpdates++;
        GyroState::DataFields gyroStateData = gyroState->getData();
        x_gyro_accum.append(gyroStateData.x);
        y_gyro_accum.append(gyroStateData.y);
        z_gyro_accum.append(gyroStateData.z);
    }

    // update the progress indicator
    ui->zeroBiasProgress->setValue((float)qMin(accelUpdates, gyroUpdates) / NUM_SENSOR_UPDATES * 100);

    // If we have enough samples, then stop sampling and compute the biases
    if (accelUpdates >= NUM_SENSOR_UPDATES && gyroUpdates >= NUM_SENSOR_UPDATES) {
        timer.stop();
        disconnect(obj, SIGNAL(objectUpdated(UAVObject *)), this, SLOT(sensorsUpdated(UAVObject *)));
        disconnect(&timer, SIGNAL(timeout()), this, SLOT(timeout()));

        float x_bias = listMean(x_accum);
        float y_bias = listMean(y_accum);
        float z_bias = (listMean(z_accum) + 9.81);

        float x_gyro_bias = listMean(x_gyro_accum);
        float y_gyro_bias = listMean(y_gyro_accum);
        float z_gyro_bias = listMean(z_gyro_accum);
        accelState->setMetadata(initialAccelStateMdata);
        gyroState->setMetadata(initialGyroStateMdata);

        AccelGyroSettings::DataFields accelGyroSettingsData = AccelGyroSettings::GetInstance(getObjectManager())->getData();
        AttitudeSettings::DataFields attitudeSettingsData   = AttitudeSettings::GetInstance(getObjectManager())->getData();
        // We offset the gyro bias by current bias to help precision
        accelGyroSettingsData.accel_bias[0] += x_bias;
        accelGyroSettingsData.accel_bias[1] += y_bias;
        accelGyroSettingsData.accel_bias[2] += z_bias;
        accelGyroSettingsData.gyro_bias[0]   = -x_gyro_bias;
        accelGyroSettingsData.gyro_bias[1]   = -y_gyro_bias;
        accelGyroSettingsData.gyro_bias[2]   = -z_gyro_bias;
        attitudeSettingsData.BiasCorrectGyro = AttitudeSettings::BIASCORRECTGYRO_TRUE;
        AttitudeSettings::GetInstance(getObjectManager())->setData(attitudeSettingsData);
        AccelGyroSettings::GetInstance(getObjectManager())->setData(accelGyroSettingsData);
        this->setDirty(true);

        // reenable controls
        enableControls(true);
    }
}

void ConfigCCAttitudeWidget::timeout()
{
    UAVDataObject *obj = AccelState::GetInstance(getObjectManager());

    disconnect(obj, SIGNAL(objectUpdated(UAVObject *)), this, SLOT(sensorsUpdated(UAVObject *)));
    disconnect(&timer, SIGNAL(timeout()), this, SLOT(timeout()));

    AccelState *accelState = AccelState::GetInstance(getObjectManager());
    GyroState *gyroState   = GyroState::GetInstance(getObjectManager());
    accelState->setMetadata(initialAccelStateMdata);
    gyroState->setMetadata(initialGyroStateMdata);

    QMessageBox msgBox;
    msgBox.setText(tr("Calibration timed out before receiving required updates."));
    msgBox.setStandardButtons(QMessageBox::Ok);
    msgBox.setDefaultButton(QMessageBox::Ok);
    msgBox.exec();

    // reset progress indicator
    ui->zeroBiasProgress->setValue(0);
    // reenable controls
    enableControls(true);
}

void ConfigCCAttitudeWidget::startAccelCalibration()
{
    // disable controls during sampling
    enableControls(false);

    accelUpdates = 0;
    gyroUpdates  = 0;
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
    UAVDataObject *accelState = AccelState::GetInstance(getObjectManager());
    UAVDataObject *gyroState  = GyroState::GetInstance(getObjectManager());
    connect(accelState, SIGNAL(objectUpdated(UAVObject *)), this, SLOT(sensorsUpdated(UAVObject *)));
    connect(gyroState, SIGNAL(objectUpdated(UAVObject *)), this, SLOT(sensorsUpdated(UAVObject *)));

    // Speed up updates
    initialAccelStateMdata = accelState->getMetadata();
    UAVObject::Metadata accelStateMdata = initialAccelStateMdata;
    UAVObject::SetFlightTelemetryUpdateMode(accelStateMdata, UAVObject::UPDATEMODE_PERIODIC);
    accelStateMdata.flightTelemetryUpdatePeriod = 30; // ms
    accelState->setMetadata(accelStateMdata);

    initialGyroStateMdata = gyroState->getMetadata();
    UAVObject::Metadata gyroStateMdata = initialGyroStateMdata;
    UAVObject::SetFlightTelemetryUpdateMode(gyroStateMdata, UAVObject::UPDATEMODE_PERIODIC);
    gyroStateMdata.flightTelemetryUpdatePeriod = 30; // ms
    gyroState->setMetadata(gyroStateMdata);

    // Set up timeout timer
    timer.setSingleShot(true);
    timer.start(5000 + (NUM_SENSOR_UPDATES * qMax(accelStateMdata.flightTelemetryUpdatePeriod,
                                                  gyroStateMdata.flightTelemetryUpdatePeriod)));
    connect(&timer, SIGNAL(timeout()), this, SLOT(timeout()));
}

void ConfigCCAttitudeWidget::openHelp()
{
    QDesktopServices::openUrl(QUrl("http://wiki.openpilot.org/x/44Cf", QUrl::StrictMode));
}

void ConfigCCAttitudeWidget::setAccelFiltering(bool active)
{
    Q_UNUSED(active);
    setDirty(true);
}

void ConfigCCAttitudeWidget::enableControls(bool enable)
{
    ui->zeroBias->setEnabled(enable);
    ui->zeroGyroBiasOnArming->setEnabled(enable);
    ui->accelTauSpinbox->setEnabled(enable);
    ConfigTaskWidget::enableControls(enable);
}

void ConfigCCAttitudeWidget::updateObjectsFromWidgets()
{
    ConfigTaskWidget::updateObjectsFromWidgets();

    ui->zeroBiasProgress->setValue(0);
}

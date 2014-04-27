/**
 ******************************************************************************
 *
 * @file       sixpointcalibrationmodel.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2014.
 *
 * @brief      Six point calibration for Magnetometer and Accelerometer
 * @see        The GNU Public License (GPL) Version 3
 * @defgroup
 * @{
 *
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
#include "sixpointcalibrationmodel.h"
#include <QThread>
#include "extensionsystem/pluginmanager.h"
#include <QMessageBox>
#include "math.h"
#include "calibration/calibrationuiutils.h"

#define POINT_SAMPLE_SIZE 50
#define GRAVITY           9.81f
#define sign(x) ((x < 0) ? -1 : 1)

namespace OpenPilot {
SixPointCalibrationModel::SixPointCalibrationModel(QObject *parent) :
    QObject(parent),
    collectingData(false),
    calibratingMag(false),
    calibratingAccel(false),
    position(-1)
{}

/********** Six point calibration **************/

void SixPointCalibrationModel::magStart()
{
    start(false, true);
}
void SixPointCalibrationModel::accelStart()
{
    start(true, false);
}

/**
 * Called by the "Start" button.  Sets up the meta data and enables the
 * buttons to perform six point calibration of the magnetometer (optionally
 * accel) to compute the scale and bias of this sensor based on the current
 * home location magnetic strength.
 */
void SixPointCalibrationModel::start(bool calibrateAccel, bool calibrateMag)
{
    calibratingAccel = calibrateAccel;
    calibratingMag   = calibrateMag;
    // Store and reset board rotation before calibration starts
    storeAndClearBoardRotation();

    RevoCalibration *revoCalibration     = RevoCalibration::GetInstance(getObjectManager());
    HomeLocation *homeLocation = HomeLocation::GetInstance(getObjectManager());
    AccelGyroSettings *accelGyroSettings = AccelGyroSettings::GetInstance(getObjectManager());

    Q_ASSERT(revoCalibration);
    Q_ASSERT(homeLocation);
    RevoCalibration::DataFields revoCalibrationData     = revoCalibration->getData();
    savedSettings.revoCalibration   = revoCalibration->getData();
    HomeLocation::DataFields homeLocationData = homeLocation->getData();
    AccelGyroSettings::DataFields accelGyroSettingsData = accelGyroSettings->getData();
    savedSettings.accelGyroSettings = accelGyroSettings->getData();

    // check if Homelocation is set
    if (!homeLocationData.Set) {
        QMessageBox msgBox;
        msgBox.setInformativeText(tr("<p>HomeLocation not SET.</p><p>Please set your HomeLocation and try again. Aborting calibration!</p>"));
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.setDefaultButton(QMessageBox::Ok);
        msgBox.setIcon(QMessageBox::Information);
        msgBox.exec();
        return;
    }

    // Calibration accel
    accelGyroSettingsData.accel_scale[AccelGyroSettings::ACCEL_SCALE_X] = 1;
    accelGyroSettingsData.accel_scale[AccelGyroSettings::ACCEL_SCALE_Y] = 1;
    accelGyroSettingsData.accel_scale[AccelGyroSettings::ACCEL_SCALE_Z] = 1;
    accelGyroSettingsData.accel_bias[AccelGyroSettings::ACCEL_BIAS_X]   = 0;
    accelGyroSettingsData.accel_bias[AccelGyroSettings::ACCEL_BIAS_Y]   = 0;
    accelGyroSettingsData.accel_bias[AccelGyroSettings::ACCEL_BIAS_Z]   = 0;

    accel_accum_x.clear();
    accel_accum_y.clear();
    accel_accum_z.clear();

    // Calibration mag
    // Reset the transformation matrix to identity
    for (int i = 0; i < RevoCalibration::MAG_TRANSFORM_R2C2; i++) {
        revoCalibrationData.mag_transform[i] = 0;
    }
    revoCalibrationData.mag_transform[RevoCalibration::MAG_TRANSFORM_R0C0] = 1;
    revoCalibrationData.mag_transform[RevoCalibration::MAG_TRANSFORM_R1C1] = 1;
    revoCalibrationData.mag_transform[RevoCalibration::MAG_TRANSFORM_R2C2] = 1;
    revoCalibrationData.mag_bias[RevoCalibration::MAG_BIAS_X] = 0;
    revoCalibrationData.mag_bias[RevoCalibration::MAG_BIAS_Y] = 0;
    revoCalibrationData.mag_bias[RevoCalibration::MAG_BIAS_Z] = 0;

    // Disable adaptive mag nulling
    initialMagCorrectionRate = revoCalibrationData.MagBiasNullingRate;
    revoCalibrationData.MagBiasNullingRate = 0;

    revoCalibration->setData(revoCalibrationData);
    accelGyroSettings->setData(accelGyroSettingsData);

    QThread::usleep(100000);

    mag_accum_x.clear();
    mag_accum_y.clear();
    mag_accum_z.clear();

    UAVObject::Metadata mdata;

    /* Need to get as many accel updates as possible */
    AccelState *accelState = AccelState::GetInstance(getObjectManager());
    Q_ASSERT(accelState);

    initialAccelStateMdata = accelState->getMetadata();
    mdata = initialAccelStateMdata;
    UAVObject::SetFlightTelemetryUpdateMode(mdata, UAVObject::UPDATEMODE_PERIODIC);
    mdata.flightTelemetryUpdatePeriod = 100;
    accelState->setMetadata(mdata);

    /* Need to get as many mag updates as possible */
    MagState *mag = MagState::GetInstance(getObjectManager());
    Q_ASSERT(mag);
    initialMagStateMdata = mag->getMetadata();
    mdata = initialMagStateMdata;
    UAVObject::SetFlightTelemetryUpdateMode(mdata, UAVObject::UPDATEMODE_PERIODIC);
    mdata.flightTelemetryUpdatePeriod = 100;
    mag->setMetadata(mdata);

    /* Show instructions and enable controls */
    displayInstructions(tr("Place horizontally, nose pointing north and click save position..."), true);
    showHelp(CALIBRATION_HELPER_IMAGE_NED);
    disableAllCalibrations();
    savePositionEnabledChanged(true);
    position = 0;
}

/**
 * Saves the data from the aircraft in one of six positions.
 * This is called when they click "save position" and starts
 * averaging data for this position.
 */
void SixPointCalibrationModel::savePositionData()
{
    QMutexLocker lock(&sensorsUpdateLock);

    savePositionEnabledChanged(false);

    accel_accum_x.clear();
    accel_accum_y.clear();
    accel_accum_z.clear();
    mag_accum_x.clear();
    mag_accum_y.clear();
    mag_accum_z.clear();

    collectingData = true;

    AccelState *accelState = AccelState::GetInstance(getObjectManager());
    Q_ASSERT(accelState);
    MagState *mag = MagState::GetInstance(getObjectManager());
    Q_ASSERT(mag);

    connect(accelState, SIGNAL(objectUpdated(UAVObject *)), this, SLOT(getSample(UAVObject *)));
    connect(mag, SIGNAL(objectUpdated(UAVObject *)), this, SLOT(getSample(UAVObject *)));

    displayInstructions(tr("Hold..."), false);
}

/**
 * Grab a sample of mag (optionally accel) data while in this position and
 * store it for averaging.  When sufficient points are collected advance
 * to the next position (give message to user) or compute the scale and bias
 */
void SixPointCalibrationModel::getSample(UAVObject *obj)
{
    QMutexLocker lock(&sensorsUpdateLock);

    // This is necessary to prevent a race condition on disconnect signal and another update
    if (collectingData == true) {
        if (obj->getObjID() == AccelState::OBJID) {
            AccelState *accelState = AccelState::GetInstance(getObjectManager());
            Q_ASSERT(accelState);
            AccelState::DataFields accelStateData = accelState->getData();
            accel_accum_x.append(accelStateData.x);
            accel_accum_y.append(accelStateData.y);
            accel_accum_z.append(accelStateData.z);
        } else if (obj->getObjID() == MagState::OBJID) {
            MagState *mag = MagState::GetInstance(getObjectManager());
            Q_ASSERT(mag);
            MagState::DataFields magData = mag->getData();
            mag_accum_x.append(magData.x);
            mag_accum_y.append(magData.y);
            mag_accum_z.append(magData.z);
        } else {
            Q_ASSERT(0);
        }
    }

    if (accel_accum_x.size() >= POINT_SAMPLE_SIZE && mag_accum_x.size() >= POINT_SAMPLE_SIZE && collectingData == true) {
        collectingData = false;

        savePositionEnabledChanged(true);

        // Store the mean for this position for the accel
        AccelState *accelState = AccelState::GetInstance(getObjectManager());
        Q_ASSERT(accelState);
        disconnect(accelState, SIGNAL(objectUpdated(UAVObject *)), this, SLOT(getSample(UAVObject *)));
        accel_data_x[position] = CalibrationUtils::listMean(accel_accum_x);
        accel_data_y[position] = CalibrationUtils::listMean(accel_accum_y);
        accel_data_z[position] = CalibrationUtils::listMean(accel_accum_z);

        // Store the mean for this position for the mag
        MagState *mag = MagState::GetInstance(getObjectManager());
        Q_ASSERT(mag);
        disconnect(mag, SIGNAL(objectUpdated(UAVObject *)), this, SLOT(getSample(UAVObject *)));
        mag_data_x[position] = CalibrationUtils::listMean(mag_accum_x);
        mag_data_y[position] = CalibrationUtils::listMean(mag_accum_y);
        mag_data_z[position] = CalibrationUtils::listMean(mag_accum_z);

        position = (position + 1) % 6;
        if (position == 1) {
            displayInstructions(tr("Place with nose down, right side west and click save position..."), false);
            showHelp(CALIBRATION_HELPER_IMAGE_DWN);
        }
        if (position == 2) {
            displayInstructions(tr("Place right side down, nose west and click save position..."), false);
            showHelp(CALIBRATION_HELPER_IMAGE_WDS);
        }
        if (position == 3) {
            displayInstructions(tr("Place upside down, nose east and click save position..."), false);
            showHelp(CALIBRATION_HELPER_IMAGE_ENU);
        }
        if (position == 4) {
            displayInstructions(tr("Place with nose up, left side north and click save position..."), false);
            showHelp(CALIBRATION_HELPER_IMAGE_USE);
        }
        if (position == 5) {
            displayInstructions(tr("Place with left side down, nose south and click save position..."), false);
            showHelp(CALIBRATION_HELPER_IMAGE_SUW);
        }
        if (position == 0) {
            compute(calibratingMag, calibratingAccel);
            savePositionEnabledChanged(false);

            enableAllCalibrations();
            showHelp(CALIBRATION_HELPER_IMAGE_EMPTY);
            /* Cleanup original settings */
            accelState->setMetadata(initialAccelStateMdata);

            mag->setMetadata(initialMagStateMdata);

            // Recall saved board rotation
            recallBoardRotation();
        }
    }
}

/**
 * Computes the scale and bias for the magnetomer and (compile option)
 * for the accel once all the data has been collected in 6 positions.
 */
void SixPointCalibrationModel::compute(bool mag, bool accel)
{
    double S[3], b[3];
    double Be_length;
    AccelGyroSettings *accelGyroSettings = AccelGyroSettings::GetInstance(getObjectManager());
    RevoCalibration *revoCalibration     = RevoCalibration::GetInstance(getObjectManager());
    HomeLocation *homeLocation = HomeLocation::GetInstance(getObjectManager());

    Q_ASSERT(revoCalibration);
    Q_ASSERT(homeLocation);
    AccelGyroSettings::DataFields accelGyroSettingsData = accelGyroSettings->getData();
    RevoCalibration::DataFields revoCalibrationData     = revoCalibration->getData();
    HomeLocation::DataFields homeLocationData = homeLocation->getData();

    // Calibration accel
    if (accel) {
        OpenPilot::CalibrationUtils::SixPointInConstFieldCal(homeLocationData.g_e, accel_data_x, accel_data_y, accel_data_z, S, b);
        accelGyroSettingsData.accel_scale[AccelGyroSettings::ACCEL_SCALE_X] = fabs(S[0]);
        accelGyroSettingsData.accel_scale[AccelGyroSettings::ACCEL_SCALE_Y] = fabs(S[1]);
        accelGyroSettingsData.accel_scale[AccelGyroSettings::ACCEL_SCALE_Z] = fabs(S[2]);

        accelGyroSettingsData.accel_bias[AccelGyroSettings::ACCEL_BIAS_X]   = -sign(S[0]) * b[0];
        accelGyroSettingsData.accel_bias[AccelGyroSettings::ACCEL_BIAS_Y]   = -sign(S[1]) * b[1];
        accelGyroSettingsData.accel_bias[AccelGyroSettings::ACCEL_BIAS_Z]   = -sign(S[2]) * b[2];
    }

    // Calibration mag
    if (mag) {
        Be_length = sqrt(pow(homeLocationData.Be[0], 2) + pow(homeLocationData.Be[1], 2) + pow(homeLocationData.Be[2], 2));
        OpenPilot::CalibrationUtils::SixPointInConstFieldCal(Be_length, mag_data_x, mag_data_y, mag_data_z, S, b);
        revoCalibrationData.mag_transform[RevoCalibration::MAG_TRANSFORM_R0C0] = fabs(S[0]);
        revoCalibrationData.mag_transform[RevoCalibration::MAG_TRANSFORM_R1C1] = fabs(S[1]);
        revoCalibrationData.mag_transform[RevoCalibration::MAG_TRANSFORM_R2C2] = fabs(S[2]);

        revoCalibrationData.mag_bias[RevoCalibration::MAG_BIAS_X] = -sign(S[0]) * b[0];
        revoCalibrationData.mag_bias[RevoCalibration::MAG_BIAS_Y] = -sign(S[1]) * b[1];
        revoCalibrationData.mag_bias[RevoCalibration::MAG_BIAS_Z] = -sign(S[2]) * b[2];
    }
    // Restore the previous setting
    revoCalibrationData.MagBiasNullingRate = initialMagCorrectionRate;


    bool good_calibration = true;

    // Check the mag calibration is good
    if (mag) {
        good_calibration &= revoCalibrationData.mag_transform[RevoCalibration::MAG_TRANSFORM_R0C0] ==
                            revoCalibrationData.mag_transform[RevoCalibration::MAG_TRANSFORM_R0C0];
        good_calibration &= revoCalibrationData.mag_transform[RevoCalibration::MAG_TRANSFORM_R1C1] ==
                            revoCalibrationData.mag_transform[RevoCalibration::MAG_TRANSFORM_R1C1];
        good_calibration &= revoCalibrationData.mag_transform[RevoCalibration::MAG_TRANSFORM_R2C2] ==
                            revoCalibrationData.mag_transform[RevoCalibration::MAG_TRANSFORM_R2C2];
        good_calibration &= revoCalibrationData.mag_bias[RevoCalibration::MAG_BIAS_X] ==
                            revoCalibrationData.mag_bias[RevoCalibration::MAG_BIAS_X];
        good_calibration &= revoCalibrationData.mag_bias[RevoCalibration::MAG_BIAS_Y] ==
                            revoCalibrationData.mag_bias[RevoCalibration::MAG_BIAS_Y];
        good_calibration &= revoCalibrationData.mag_bias[RevoCalibration::MAG_BIAS_Z] ==
                            revoCalibrationData.mag_bias[RevoCalibration::MAG_BIAS_Z];
    }
    // Check the accel calibration is good
    if (accel) {
        good_calibration &= accelGyroSettingsData.accel_scale[AccelGyroSettings::ACCEL_SCALE_X] ==
                            accelGyroSettingsData.accel_scale[AccelGyroSettings::ACCEL_SCALE_X];
        good_calibration &= accelGyroSettingsData.accel_scale[AccelGyroSettings::ACCEL_SCALE_Y] ==
                            accelGyroSettingsData.accel_scale[AccelGyroSettings::ACCEL_SCALE_Y];
        good_calibration &= accelGyroSettingsData.accel_scale[AccelGyroSettings::ACCEL_SCALE_Z] ==
                            accelGyroSettingsData.accel_scale[AccelGyroSettings::ACCEL_SCALE_Z];
        good_calibration &= accelGyroSettingsData.accel_bias[AccelGyroSettings::ACCEL_BIAS_X] ==
                            accelGyroSettingsData.accel_bias[AccelGyroSettings::ACCEL_BIAS_X];
        good_calibration &= accelGyroSettingsData.accel_bias[AccelGyroSettings::ACCEL_BIAS_Y] ==
                            accelGyroSettingsData.accel_bias[AccelGyroSettings::ACCEL_BIAS_Y];
        good_calibration &= accelGyroSettingsData.accel_bias[AccelGyroSettings::ACCEL_BIAS_Z] ==
                            accelGyroSettingsData.accel_bias[AccelGyroSettings::ACCEL_BIAS_Z];
    }
    if (good_calibration) {
        if (mag) {
            revoCalibration->setData(revoCalibrationData);
        } else {
            revoCalibration->setData(savedSettings.revoCalibration);
        }

        if (accel) {
            accelGyroSettings->setData(accelGyroSettingsData);
        } else {
            accelGyroSettings->setData(savedSettings.accelGyroSettings);
        }
        displayInstructions(tr("Sensor scale and bias computed succesfully."), true);
    } else {
        displayInstructions(tr("Bad calibration. Please review the instructions and repeat."), true);
    }
    position = -1; // set to run again
}
UAVObjectManager *SixPointCalibrationModel::getObjectManager()
{
    ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
    UAVObjectManager *objMngr = pm->getObject<UAVObjectManager>();

    Q_ASSERT(objMngr);
    return objMngr;
}
void SixPointCalibrationModel::showHelp(QString image)
{
    if (image == CALIBRATION_HELPER_IMAGE_EMPTY) {
        displayVisualHelp(image);
    } else {
        if (calibratingAccel) {
            displayVisualHelp(CALIBRATION_HELPER_BOARD_PREFIX + image);
        } else {
            displayVisualHelp(CALIBRATION_HELPER_PLANE_PREFIX + image);
        }
    }
}
}

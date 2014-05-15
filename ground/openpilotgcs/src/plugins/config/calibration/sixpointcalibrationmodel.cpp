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
#include "extensionsystem/pluginmanager.h"
#include "calibration/calibrationuiutils.h"

#include "math.h"
#include <QMessageBox>
#include <QThread>
#include "QDebug"

#define POINT_SAMPLE_SIZE 50
#define GRAVITY           9.81f
#define sign(x) ((x < 0) ? -1 : 1)

#define FITTING_USING_CONTINOUS_ACQUISITION

namespace OpenPilot {

SixPointCalibrationModel::SixPointCalibrationModel(QObject *parent) :
    QObject(parent),
    calibrationStepsMag(),
    calibrationStepsAccelOnly(),
    currentSteps(0),
    position(-1),
    calibratingMag(false),
    calibratingAccel(false),
    collectingData(false)
{
    calibrationStepsMag.clear();
    calibrationStepsMag
        << CalibrationStep(CALIBRATION_HELPER_IMAGE_NED,
                           tr("Place horizontally, nose pointing north and click Save Position button..."))
        << CalibrationStep(CALIBRATION_HELPER_IMAGE_DWN,
                       tr("Place with nose down, right side west and click Save Position button..."))
        << CalibrationStep(CALIBRATION_HELPER_IMAGE_WDS,
                       tr("Place right side down, nose west and click Save Position button..."))
        << CalibrationStep(CALIBRATION_HELPER_IMAGE_ENU,
                       tr("Place upside down, nose east and click Save Position button..."))
        << CalibrationStep(CALIBRATION_HELPER_IMAGE_USE,
                       tr("Place with nose up, left side north and click Save Position button..."))
        << CalibrationStep(CALIBRATION_HELPER_IMAGE_SUW,
                       tr("Place with left side down, nose south and click Save Position button..."));

    calibrationStepsAccelOnly.clear();
    calibrationStepsAccelOnly << CalibrationStep(CALIBRATION_HELPER_IMAGE_NED,
                                                 tr("Place horizontally and click Save Position button..."))
                              << CalibrationStep(CALIBRATION_HELPER_IMAGE_DWN,
                       tr("Place with nose down and click Save Position button..."))
                              << CalibrationStep(CALIBRATION_HELPER_IMAGE_WDS,
                       tr("Place right side down and click Save Position button..."))
                              << CalibrationStep(CALIBRATION_HELPER_IMAGE_ENU,
                       tr("Place upside down and click Save Position button..."))
                              << CalibrationStep(CALIBRATION_HELPER_IMAGE_USE,
                       tr("Place with nose up and click Save Position button..."))
                              << CalibrationStep(CALIBRATION_HELPER_IMAGE_SUW,
                       tr("Place with left side down and click Save Position button..."));
}

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

    if (calibrateMag) {
        currentSteps = &calibrationStepsMag;
    } else {
        currentSteps = &calibrationStepsAccelOnly;
    }

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
        // TODO
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

    if (calibrateAccel) {
        mdata = initialAccelStateMdata;
        UAVObject::SetFlightTelemetryUpdateMode(mdata, UAVObject::UPDATEMODE_PERIODIC);
        mdata.flightTelemetryUpdatePeriod = 100;
        accelState->setMetadata(mdata);
    }
    /* Need to get as many mag updates as possible */
    MagState *mag = MagState::GetInstance(getObjectManager());
    Q_ASSERT(mag);
    initialMagStateMdata = mag->getMetadata();

    if (calibrateMag) {
        mdata = initialMagStateMdata;
        UAVObject::SetFlightTelemetryUpdateMode(mdata, UAVObject::UPDATEMODE_PERIODIC);
        mdata.flightTelemetryUpdatePeriod = 100;
        mag->setMetadata(mdata);
    }

    /* Show instructions and enable controls */
    displayInstructions((*currentSteps)[0].instructions, WizardModel::Info, true);
    showHelp((*currentSteps)[0].visualHelp);

    disableAllCalibrations();
    savePositionEnabledChanged(true);
    position = 0;
    mag_fit_x.clear();
    mag_fit_y.clear();
    mag_fit_z.clear();
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

    if (calibratingMag) {
#ifdef FITTING_USING_CONTINOUS_ACQUISITION
        // Mag samples are acquired during the whole calibration session, to be used for ellipsoid fit.
        if (!position) {
            connect(mag, SIGNAL(objectUpdated(UAVObject *)), this, SLOT(continouslyGetMagSamples(UAVObject *)));
        }
#endif // FITTING_USING_CONTINOUS_ACQUISITION
        connect(mag, SIGNAL(objectUpdated(UAVObject *)), this, SLOT(getSample(UAVObject *)));
    }
    if (calibratingAccel) {
        connect(accelState, SIGNAL(objectUpdated(UAVObject *)), this, SLOT(getSample(UAVObject *)));
    }

    displayInstructions(tr("Hold..."));
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
#ifndef FITTING_USING_CONTINOUS_ACQUISITION
            mag_fit_x.append(magData.x);
            mag_fit_y.append(magData.y);
            mag_fit_z.append(magData.z);
#endif // FITTING_USING_CONTINOUS_ACQUISITION
        } else {
            Q_ASSERT(0);
        }
    }

    if ((!calibratingAccel || (accel_accum_x.size() >= POINT_SAMPLE_SIZE)) &&
        (!calibratingMag || (mag_accum_x.size() >= POINT_SAMPLE_SIZE / 10)) &&
        (collectingData == true)) {
        collectingData = false;

        savePositionEnabledChanged(true);

        // Store the mean for this position for the accel
        AccelState *accelState = AccelState::GetInstance(getObjectManager());
        Q_ASSERT(accelState);
        if (calibratingAccel) {
            disconnect(accelState, SIGNAL(objectUpdated(UAVObject *)), this, SLOT(getSample(UAVObject *)));
            accel_data_x[position] = CalibrationUtils::listMean(accel_accum_x);
            accel_data_y[position] = CalibrationUtils::listMean(accel_accum_y);
            accel_data_z[position] = CalibrationUtils::listMean(accel_accum_z);
        }
        // Store the mean for this position for the mag
        MagState *mag = MagState::GetInstance(getObjectManager());
        Q_ASSERT(mag);
        if (calibratingMag) {
            disconnect(mag, SIGNAL(objectUpdated(UAVObject *)), this, SLOT(getSample(UAVObject *)));
            mag_data_x[position] = CalibrationUtils::listMean(mag_accum_x);
            mag_data_y[position] = CalibrationUtils::listMean(mag_accum_y);
            mag_data_z[position] = CalibrationUtils::listMean(mag_accum_z);
        }

        position = (position + 1) % 6;
        if (position != 0) {
            displayInstructions((*currentSteps)[position].instructions);
            showHelp((*currentSteps)[position].visualHelp);
        } else {
#ifdef FITTING_USING_CONTINOUS_ACQUISITION
            if (calibratingMag) {
                disconnect(mag, SIGNAL(objectUpdated(UAVObject *)), this, SLOT(continouslyGetMagSamples(UAVObject *)));
            }
#endif // FITTING_USING_CONTINOUS_ACQUISITION
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

void SixPointCalibrationModel::continouslyGetMagSamples(UAVObject *obj)
{
    QMutexLocker lock(&sensorsUpdateLock);

    if (obj->getObjID() == MagState::OBJID) {
        MagState *mag = MagState::GetInstance(getObjectManager());
        Q_ASSERT(mag);
        MagState::DataFields magData = mag->getData();
        mag_fit_x.append(magData.x);
        mag_fit_y.append(magData.y);
        mag_fit_z.append(magData.z);
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
        int vectSize = mag_fit_x.count();
        Eigen::VectorXf samples_x(vectSize);
        Eigen::VectorXf samples_y(vectSize);
        Eigen::VectorXf samples_z(vectSize);
        for (int i = 0; i < vectSize; i++) {
            samples_x(i) = mag_fit_x[i];
            samples_y(i) = mag_fit_y[i];
            samples_z(i) = mag_fit_z[i];
        }
        OpenPilot::CalibrationUtils::EllipsoidCalibrationResult result;
        OpenPilot::CalibrationUtils::EllipsoidCalibration(&samples_x, &samples_y, &samples_z, Be_length, &result, true);
        qDebug() << "-----------------------------------";
        qDebug() << "Mag Calibration results: Fit";
        qDebug() << "scale(" << result.Scale.coeff(0) << ", " << result.Scale.coeff(1) << ", " << result.Scale.coeff(2) << ")";
        qDebug() << "bias(" << result.Bias.coeff(0) << ", " << result.Bias.coeff(1) << ", " << result.Bias.coeff(2) << ")";

        OpenPilot::CalibrationUtils::SixPointInConstFieldCal(Be_length, mag_data_x, mag_data_y, mag_data_z, S, b);

        qDebug() << "-----------------------------------";
        qDebug() << "Mag Calibration results: Six Point";
        qDebug() << "scale(" << S[0] << ", " << S[1] << ", " << S[2] << ")";
        qDebug() << "bias(" << -sign(S[0]) * b[0] << ", " << -sign(S[1]) * b[1] << ", " << -sign(S[2]) * b[2] << ")";
        qDebug() << "-----------------------------------";

        revoCalibrationData.mag_transform[RevoCalibration::MAG_TRANSFORM_R0C0] = result.CalibrationMatrix.coeff(0, 0);
        revoCalibrationData.mag_transform[RevoCalibration::MAG_TRANSFORM_R0C1] = result.CalibrationMatrix.coeff(0, 1);
        revoCalibrationData.mag_transform[RevoCalibration::MAG_TRANSFORM_R0C2] = result.CalibrationMatrix.coeff(0, 2);

        revoCalibrationData.mag_transform[RevoCalibration::MAG_TRANSFORM_R1C0] = result.CalibrationMatrix.coeff(1, 0);
        revoCalibrationData.mag_transform[RevoCalibration::MAG_TRANSFORM_R1C1] = result.CalibrationMatrix.coeff(1, 1);
        revoCalibrationData.mag_transform[RevoCalibration::MAG_TRANSFORM_R1C2] = result.CalibrationMatrix.coeff(1, 2);

        revoCalibrationData.mag_transform[RevoCalibration::MAG_TRANSFORM_R2C0] = result.CalibrationMatrix.coeff(2, 0);
        revoCalibrationData.mag_transform[RevoCalibration::MAG_TRANSFORM_R2C1] = result.CalibrationMatrix.coeff(2, 1);
        revoCalibrationData.mag_transform[RevoCalibration::MAG_TRANSFORM_R2C2] = result.CalibrationMatrix.coeff(2, 2);

        revoCalibrationData.mag_bias[RevoCalibration::MAG_BIAS_X] = result.Bias.coeff(0);
        revoCalibrationData.mag_bias[RevoCalibration::MAG_BIAS_Y] = result.Bias.coeff(1);
        revoCalibrationData.mag_bias[RevoCalibration::MAG_BIAS_Z] = result.Bias.coeff(2);
    }
    // Restore the previous setting
    revoCalibrationData.MagBiasNullingRate = initialMagCorrectionRate;


    bool good_calibration = true;

    // Check the mag calibration is good
    if (mag) {
        good_calibration &= revoCalibrationData.mag_transform[RevoCalibration::MAG_TRANSFORM_R0C0] ==
                            revoCalibrationData.mag_transform[RevoCalibration::MAG_TRANSFORM_R0C0];
        good_calibration &= revoCalibrationData.mag_transform[RevoCalibration::MAG_TRANSFORM_R0C1] ==
                            revoCalibrationData.mag_transform[RevoCalibration::MAG_TRANSFORM_R0C1];
        good_calibration &= revoCalibrationData.mag_transform[RevoCalibration::MAG_TRANSFORM_R0C2] ==
                            revoCalibrationData.mag_transform[RevoCalibration::MAG_TRANSFORM_R0C2];

        good_calibration &= revoCalibrationData.mag_transform[RevoCalibration::MAG_TRANSFORM_R1C0] ==
                            revoCalibrationData.mag_transform[RevoCalibration::MAG_TRANSFORM_R1C0];
        good_calibration &= revoCalibrationData.mag_transform[RevoCalibration::MAG_TRANSFORM_R1C1] ==
                            revoCalibrationData.mag_transform[RevoCalibration::MAG_TRANSFORM_R1C1];
        good_calibration &= revoCalibrationData.mag_transform[RevoCalibration::MAG_TRANSFORM_R1C2] ==
                            revoCalibrationData.mag_transform[RevoCalibration::MAG_TRANSFORM_R1C2];

        good_calibration &= revoCalibrationData.mag_transform[RevoCalibration::MAG_TRANSFORM_R2C0] ==
                            revoCalibrationData.mag_transform[RevoCalibration::MAG_TRANSFORM_R2C0];
        good_calibration &= revoCalibrationData.mag_transform[RevoCalibration::MAG_TRANSFORM_R2C1] ==
                            revoCalibrationData.mag_transform[RevoCalibration::MAG_TRANSFORM_R2C1];
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
        displayInstructions(tr("Sensor scale and bias computed succesfully."));
    } else {
        displayInstructions(tr("Bad calibration. Please review the instructions and repeat."), WizardModel::Error);
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

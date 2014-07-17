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

#include <math.h>
#include <QThread>
#include "QDebug"

#define POINT_SAMPLE_SIZE 50
#define GRAVITY           9.81f
#define sign(x)   ((x < 0) ? -1 : 1)

#define FITTING_USING_CONTINOUS_ACQUISITION

#define IS_NAN(v) (!(v == v))

namespace OpenPilot {
SixPointCalibrationModel::SixPointCalibrationModel(QObject *parent) :
    QObject(parent),
    calibratingMag(false),
    externalMagAvailable(false),
    calibratingAccel(false),
    calibrationStepsMag(),
    calibrationStepsAccelOnly(),
    currentSteps(0),
    position(-1),
    collectingData(false),
    m_dirty(false)
{
    calibrationStepsMag.clear();
    calibrationStepsMag
        << CalibrationStep(CALIBRATION_HELPER_IMAGE_NED,
                           tr("Place horizontally, nose pointing north and press Save Position..."))
        << CalibrationStep(CALIBRATION_HELPER_IMAGE_DWN,
                       tr("Place with nose down, right side west and press Save Position..."))
        << CalibrationStep(CALIBRATION_HELPER_IMAGE_WDS,
                       tr("Place right side down, nose west and press Save Position..."))
        << CalibrationStep(CALIBRATION_HELPER_IMAGE_ENU,
                       tr("Place upside down, nose east and press Save Position..."))
        << CalibrationStep(CALIBRATION_HELPER_IMAGE_USE,
                       tr("Place with nose up, left side north and press Save Position..."))
        << CalibrationStep(CALIBRATION_HELPER_IMAGE_SUW,
                       tr("Place with left side down, nose south and press Save Position..."));

    // All mag calibration matrices have the same structure, this is also used when calculating bias/transforms
    // this is enforced using assert.
    Q_STATIC_ASSERT((int)RevoCalibration::MAG_TRANSFORM_R0C0 == (int)AuxMagSettings::MAG_TRANSFORM_R0C0 &&
                    (int)RevoCalibration::MAG_TRANSFORM_R1C0 == (int)AuxMagSettings::MAG_TRANSFORM_R1C0 &&
                    (int)RevoCalibration::MAG_TRANSFORM_R2C0 == (int)AuxMagSettings::MAG_TRANSFORM_R2C0 &&
                    (int)RevoCalibration::MAG_TRANSFORM_R0C1 == (int)AuxMagSettings::MAG_TRANSFORM_R0C1 &&
                    (int)RevoCalibration::MAG_TRANSFORM_R1C1 == (int)AuxMagSettings::MAG_TRANSFORM_R1C1 &&
                    (int)RevoCalibration::MAG_TRANSFORM_R2C1 == (int)AuxMagSettings::MAG_TRANSFORM_R2C1 &&
                    (int)RevoCalibration::MAG_TRANSFORM_R0C2 == (int)AuxMagSettings::MAG_TRANSFORM_R0C2 &&
                    (int)RevoCalibration::MAG_TRANSFORM_R1C2 == (int)AuxMagSettings::MAG_TRANSFORM_R1C2 &&
                    (int)RevoCalibration::MAG_TRANSFORM_R2C2 == (int)AuxMagSettings::MAG_TRANSFORM_R2C2 &&
                    (int)RevoCalibration::MAG_BIAS_X == (int)AuxMagSettings::MAG_BIAS_X &&
                    (int)RevoCalibration::MAG_BIAS_Y == (int)AuxMagSettings::MAG_BIAS_Y &&
                    (int)RevoCalibration::MAG_BIAS_Z == (int)AuxMagSettings::MAG_BIAS_Z);

    calibrationStepsAccelOnly.clear();
    calibrationStepsAccelOnly << CalibrationStep(CALIBRATION_HELPER_IMAGE_NED,
                                                 tr("Place horizontally and press Save Position..."))
                              << CalibrationStep(CALIBRATION_HELPER_IMAGE_DWN,
                       tr("Place with nose down and press Save Position..."))
                              << CalibrationStep(CALIBRATION_HELPER_IMAGE_WDS,
                       tr("Place right side down and press Save Position..."))
                              << CalibrationStep(CALIBRATION_HELPER_IMAGE_ENU,
                       tr("Place upside down and press Save Position..."))
                              << CalibrationStep(CALIBRATION_HELPER_IMAGE_USE,
                       tr("Place with nose up and press Save Position..."))
                              << CalibrationStep(CALIBRATION_HELPER_IMAGE_SUW,
                       tr("Place with left side down and press Save Position..."));

    revoCalibration   = RevoCalibration::GetInstance(getObjectManager());
    Q_ASSERT(revoCalibration);

    auxMagSettings = AuxMagSettings::GetInstance(getObjectManager());
    Q_ASSERT(auxMagSettings);

    accelGyroSettings = AccelGyroSettings::GetInstance(getObjectManager());
    Q_ASSERT(accelGyroSettings);

    accelState   = AccelState::GetInstance(getObjectManager());
    Q_ASSERT(accelState);

    magSensor    = MagSensor::GetInstance(getObjectManager());
    Q_ASSERT(magSensor);

    auxMagSensor = AuxMagSensor::GetInstance(getObjectManager());
    Q_ASSERT(auxMagSensor);

    homeLocation = HomeLocation::GetInstance(getObjectManager());
    Q_ASSERT(homeLocation);
}

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

    started();

    // check if Homelocation is set
    HomeLocation::DataFields homeLocationData = homeLocation->getData();
    if (!homeLocationData.Set) {
        displayInstructions(tr("Home location not set, please set your home location and retry."), WizardModel::Warn);
        displayInstructions(tr("Aborting calibration!"), WizardModel::Failure);
        stopped();
        return;
    }

    // Store and reset board rotation before calibration starts
    storeAndClearBoardRotation();

    // Calibration accel
    AccelGyroSettings::DataFields accelGyroSettingsData = accelGyroSettings->getData();
    memento.accelGyroSettingsData = accelGyroSettingsData;

    accelGyroSettingsData.accel_scale[AccelGyroSettings::ACCEL_SCALE_X] = 1;
    accelGyroSettingsData.accel_scale[AccelGyroSettings::ACCEL_SCALE_Y] = 1;
    accelGyroSettingsData.accel_scale[AccelGyroSettings::ACCEL_SCALE_Z] = 1;
    accelGyroSettingsData.accel_bias[AccelGyroSettings::ACCEL_BIAS_X]   = 0;
    accelGyroSettingsData.accel_bias[AccelGyroSettings::ACCEL_BIAS_Y]   = 0;
    accelGyroSettingsData.accel_bias[AccelGyroSettings::ACCEL_BIAS_Z]   = 0;

    accelGyroSettings->setData(accelGyroSettingsData);

    // Calibration mag
    RevoCalibration::DataFields revoCalibrationData = revoCalibration->getData();
    memento.revoCalibrationData = revoCalibrationData;

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
    revoCalibrationData.MagBiasNullingRate = 0;

    revoCalibration->setData(revoCalibrationData);

    // Calibration AuxMag
    AuxMagSettings::DataFields auxMagSettingsData = auxMagSettings->getData();
    memento.auxMagSettings = auxMagSettingsData;

    // Reset the transformation matrix to identity
    for (int i = 0; i < AuxMagSettings::MAG_TRANSFORM_R2C2; i++) {
        auxMagSettingsData.mag_transform[i] = 0;
    }
    auxMagSettingsData.mag_transform[AuxMagSettings::MAG_TRANSFORM_R0C0] = 1;
    auxMagSettingsData.mag_transform[AuxMagSettings::MAG_TRANSFORM_R1C1] = 1;
    auxMagSettingsData.mag_transform[AuxMagSettings::MAG_TRANSFORM_R2C2] = 1;
    auxMagSettingsData.mag_bias[AuxMagSettings::MAG_BIAS_X] = 0;
    auxMagSettingsData.mag_bias[AuxMagSettings::MAG_BIAS_Y] = 0;
    auxMagSettingsData.mag_bias[AuxMagSettings::MAG_BIAS_Z] = 0;

    // Disable adaptive mag nulling
    auxMagSettingsData.MagBiasNullingRate = 0;

    auxMagSettings->setData(auxMagSettingsData);

    QThread::usleep(100000);

    mag_accum_x.clear();
    mag_accum_y.clear();
    mag_accum_z.clear();

    mag_fit_x.clear();
    mag_fit_y.clear();
    mag_fit_z.clear();

    // Need to get as many accel updates as possible
    memento.accelStateMetadata = accelState->getMetadata();
    if (calibrateAccel) {
        UAVObject::Metadata mdata = accelState->getMetadata();
        UAVObject::SetFlightTelemetryUpdateMode(mdata, UAVObject::UPDATEMODE_PERIODIC);
        mdata.flightTelemetryUpdatePeriod = 100;
        accelState->setMetadata(mdata);
    }

    // Need to get as many mag updates as possible
    memento.magSensorMetadata    = magSensor->getMetadata();
    memento.auxMagSensorMetadata = auxMagSensor->getMetadata();

    if (calibrateMag) {
        UAVObject::Metadata mdata = magSensor->getMetadata();
        UAVObject::SetFlightTelemetryUpdateMode(mdata, UAVObject::UPDATEMODE_PERIODIC);
        mdata.flightTelemetryUpdatePeriod = 100;
        magSensor->setMetadata(mdata);

        mdata = auxMagSensor->getMetadata();
        UAVObject::SetFlightTelemetryUpdateMode(mdata, UAVObject::UPDATEMODE_PERIODIC);
        mdata.flightTelemetryUpdatePeriod = 100;
        auxMagSensor->setMetadata(mdata);
    }

    // reset dirty state to forget previous unsaved runs
    m_dirty = false;

    if (calibrateMag) {
        currentSteps = &calibrationStepsMag;
    } else {
        currentSteps = &calibrationStepsAccelOnly;
    }

    position = 0;

    // Show instructions and enable controls
    progressChanged(0);
    displayInstructions((*currentSteps)[0].instructions, WizardModel::Prompt);
    showHelp((*currentSteps)[0].visualHelp);
    savePositionEnabledChanged(true);
}

/**
 * Saves the data from the aircraft in one of six positions.
 * This is called when they press "save position" and starts
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
    aux_mag_accum_x.clear();
    aux_mag_accum_y.clear();
    aux_mag_accum_z.clear();

    collectingData = true;

    if (calibratingMag) {
#ifdef FITTING_USING_CONTINOUS_ACQUISITION
        // Mag samples are acquired during the whole calibration session, to be used for ellipsoid fit.
        if (!position) {
            connect(magSensor, SIGNAL(objectUpdated(UAVObject *)), this, SLOT(continouslyGetMagSamples(UAVObject *)));
            connect(auxMagSensor, SIGNAL(objectUpdated(UAVObject *)), this, SLOT(continouslyGetMagSamples(UAVObject *)));
        }
#endif // FITTING_USING_CONTINOUS_ACQUISITION
        connect(magSensor, SIGNAL(objectUpdated(UAVObject *)), this, SLOT(getSample(UAVObject *)));
        connect(auxMagSensor, SIGNAL(objectUpdated(UAVObject *)), this, SLOT(getSample(UAVObject *)));
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
            AccelState::DataFields accelStateData = accelState->getData();
            accel_accum_x.append(accelStateData.x);
            accel_accum_y.append(accelStateData.y);
            accel_accum_z.append(accelStateData.z);
        } else if (obj->getObjID() == MagSensor::OBJID) {
            MagSensor::DataFields magData = magSensor->getData();
            mag_accum_x.append(magData.x);
            mag_accum_y.append(magData.y);
            mag_accum_z.append(magData.z);
#ifndef FITTING_USING_CONTINOUS_ACQUISITION
            mag_fit_x.append(magData.x);
            mag_fit_y.append(magData.y);
            mag_fit_z.append(magData.z);
#endif // FITTING_USING_CONTINOUS_ACQUISITION
        } else if (obj->getObjID() == AuxMagSensor::OBJID) {
            AuxMagSensor::DataFields auxMagData = auxMagSensor->getData();
            if (auxMagData.Status == AuxMagSensor::STATUS_OK) {
                aux_mag_accum_x.append(auxMagData.x);
                aux_mag_accum_y.append(auxMagData.y);
                aux_mag_accum_z.append(auxMagData.z);
                externalMagAvailable = true;
#ifndef FITTING_USING_CONTINOUS_ACQUISITION
                aux_mag_fit_x.append(auxMagData.x);
                aux_mag_fit_y.append(auxMagData.y);
                aux_mag_fit_z.append(auxMagData.z);
#endif // FITTING_USING_CONTINOUS_ACQUISITION
            }
        } else {
            Q_ASSERT(0);
        }
    }

    bool done = true;
    float progress = 0;
    if (calibratingAccel) {
        done     = (accel_accum_x.size() >= POINT_SAMPLE_SIZE);
        progress = (float)accel_accum_x.size() / (float)POINT_SAMPLE_SIZE;
    }
    if (calibratingMag) {
        done     = (mag_accum_x.size() >= POINT_SAMPLE_SIZE / 10);
        progress = (float)mag_accum_x.size() / (float)(POINT_SAMPLE_SIZE / 10);
    }

    progressChanged(progress * 100);

    if (collectingData && done) {
        collectingData = false;

        savePositionEnabledChanged(true);

        // Store the mean for this position for the accel
        if (calibratingAccel) {
            disconnect(accelState, SIGNAL(objectUpdated(UAVObject *)), this, SLOT(getSample(UAVObject *)));
            accel_data_x[position] = CalibrationUtils::listMean(accel_accum_x);
            accel_data_y[position] = CalibrationUtils::listMean(accel_accum_y);
            accel_data_z[position] = CalibrationUtils::listMean(accel_accum_z);
        }

        // Store the mean for this position for the mag
        if (calibratingMag) {
            disconnect(magSensor, SIGNAL(objectUpdated(UAVObject *)), this, SLOT(getSample(UAVObject *)));
            disconnect(auxMagSensor, SIGNAL(objectUpdated(UAVObject *)), this, SLOT(getSample(UAVObject *)));
        }

        position = (position + 1) % 6;
        if (position != 0) {
            // move to next step
            displayInstructions((*currentSteps)[position].instructions, WizardModel::Prompt);
            showHelp((*currentSteps)[position].visualHelp);
        } else {
            // done...
#ifdef FITTING_USING_CONTINOUS_ACQUISITION
            if (calibratingMag) {
                disconnect(magSensor, SIGNAL(objectUpdated(UAVObject *)), this, SLOT(continouslyGetMagSamples(UAVObject *)));
                disconnect(auxMagSensor, SIGNAL(objectUpdated(UAVObject *)), this, SLOT(continouslyGetMagSamples(UAVObject *)));
            }
#endif // FITTING_USING_CONTINOUS_ACQUISITION
            compute();

            // Restore original settings
            accelState->setMetadata(memento.accelStateMetadata);
            magSensor->setMetadata(memento.magSensorMetadata);
            auxMagSensor->setMetadata(memento.auxMagSensorMetadata);
            revoCalibration->setData(memento.revoCalibrationData);
            accelGyroSettings->setData(memento.accelGyroSettingsData);
            auxMagSettings->setData(memento.auxMagSettings);
            // Recall saved board rotation
            recallBoardRotation();

            stopped();
            showHelp(CALIBRATION_HELPER_IMAGE_EMPTY);
            savePositionEnabledChanged(false);
        }
    }
}

void SixPointCalibrationModel::continouslyGetMagSamples(UAVObject *obj)
{
    QMutexLocker lock(&sensorsUpdateLock);

    if (obj->getObjID() == MagSensor::OBJID) {
        MagSensor::DataFields magSensorData = magSensor->getData();
        mag_fit_x.append(magSensorData.x);
        mag_fit_y.append(magSensorData.y);
        mag_fit_z.append(magSensorData.z);
    } else if (obj->getObjID() == AuxMagSensor::OBJID) {
        AuxMagSensor::DataFields auxMagData = auxMagSensor->getData();
        if (auxMagData.Status == AuxMagSensor::STATUS_OK) {
            aux_mag_fit_x.append(auxMagData.x);
            aux_mag_fit_y.append(auxMagData.y);
            aux_mag_fit_z.append(auxMagData.z);
            externalMagAvailable = true;
        }
    }
}

/**
 * Computes the scale and bias for the magnetomer or for the accel.
 * Called once all the data has been collected in 6 positions.
 */
void SixPointCalibrationModel::compute()
{
    double S[3], b[3];
    double Be_length;

    RevoCalibration::DataFields revoCalibrationData     = revoCalibration->getData();
    AuxMagSettings::DataFields auxCalibrationData    = auxMagSettings->getData();
    AccelGyroSettings::DataFields accelGyroSettingsData = accelGyroSettings->getData();

    HomeLocation::DataFields homeLocationData = homeLocation->getData();

    // Calibration accel
    if (calibratingAccel) {
        OpenPilot::CalibrationUtils::SixPointInConstFieldCal(homeLocationData.g_e, accel_data_x, accel_data_y, accel_data_z, S, b);
        accelGyroSettingsData.accel_scale[AccelGyroSettings::ACCEL_SCALE_X] = fabs(S[0]);
        accelGyroSettingsData.accel_scale[AccelGyroSettings::ACCEL_SCALE_Y] = fabs(S[1]);
        accelGyroSettingsData.accel_scale[AccelGyroSettings::ACCEL_SCALE_Z] = fabs(S[2]);

        accelGyroSettingsData.accel_bias[AccelGyroSettings::ACCEL_BIAS_X]   = -sign(S[0]) * b[0];
        accelGyroSettingsData.accel_bias[AccelGyroSettings::ACCEL_BIAS_Y]   = -sign(S[1]) * b[1];
        accelGyroSettingsData.accel_bias[AccelGyroSettings::ACCEL_BIAS_Z]   = -sign(S[2]) * b[2];
    }

    // Calibration mag
    if (calibratingMag) {
        Be_length = sqrt(pow(homeLocationData.Be[0], 2) + pow(homeLocationData.Be[1], 2) + pow(homeLocationData.Be[2], 2));

        qDebug() << "-----------------------------------";
        qDebug() << "Onboard Mag";
        CalcCalibration(mag_fit_x, mag_fit_y, mag_fit_z, Be_length, revoCalibrationData.mag_transform, revoCalibrationData.mag_bias);
        if (externalMagAvailable) {
            qDebug() << "Aux Mag";
            CalcCalibration(aux_mag_fit_x, aux_mag_fit_y, aux_mag_fit_z, Be_length, auxCalibrationData.mag_transform, auxCalibrationData.mag_bias);
        }
    }
    // Restore the previous setting
    revoCalibrationData.MagBiasNullingRate = memento.revoCalibrationData.MagBiasNullingRate;;

    // Check the mag calibration is good
    bool good_calibration     = true;
    bool good_aux_calibration = true;
    if (calibratingMag) {
        good_calibration &= !IS_NAN(revoCalibrationData.mag_transform[RevoCalibration::MAG_TRANSFORM_R0C0]);
        good_calibration &= !IS_NAN(revoCalibrationData.mag_transform[RevoCalibration::MAG_TRANSFORM_R0C1]);
        good_calibration &= !IS_NAN(revoCalibrationData.mag_transform[RevoCalibration::MAG_TRANSFORM_R0C2]);

        good_calibration &= !IS_NAN(revoCalibrationData.mag_transform[RevoCalibration::MAG_TRANSFORM_R1C0]);
        good_calibration &= !IS_NAN(revoCalibrationData.mag_transform[RevoCalibration::MAG_TRANSFORM_R1C1]);
        good_calibration &= !IS_NAN(revoCalibrationData.mag_transform[RevoCalibration::MAG_TRANSFORM_R1C2]);

        good_calibration &= !IS_NAN(revoCalibrationData.mag_transform[RevoCalibration::MAG_TRANSFORM_R2C0]);
        good_calibration &= !IS_NAN(revoCalibrationData.mag_transform[RevoCalibration::MAG_TRANSFORM_R2C1]);
        good_calibration &= !IS_NAN(revoCalibrationData.mag_transform[RevoCalibration::MAG_TRANSFORM_R2C2]);

        good_calibration &= !IS_NAN(revoCalibrationData.mag_bias[RevoCalibration::MAG_BIAS_X]);
        good_calibration &= !IS_NAN(revoCalibrationData.mag_bias[RevoCalibration::MAG_BIAS_Y]);
        good_calibration &= !IS_NAN(revoCalibrationData.mag_bias[RevoCalibration::MAG_BIAS_Z]);
        if (externalMagAvailable) {
            good_calibration &= !IS_NAN(auxCalibrationData.mag_transform[RevoCalibration::MAG_TRANSFORM_R0C0]);
            good_calibration &= !IS_NAN(auxCalibrationData.mag_transform[RevoCalibration::MAG_TRANSFORM_R0C1]);
            good_calibration &= !IS_NAN(auxCalibrationData.mag_transform[RevoCalibration::MAG_TRANSFORM_R0C2]);

            good_calibration &= !IS_NAN(auxCalibrationData.mag_transform[RevoCalibration::MAG_TRANSFORM_R1C0]);
            good_calibration &= !IS_NAN(auxCalibrationData.mag_transform[RevoCalibration::MAG_TRANSFORM_R1C1]);
            good_calibration &= !IS_NAN(auxCalibrationData.mag_transform[RevoCalibration::MAG_TRANSFORM_R1C2]);

            good_calibration &= !IS_NAN(auxCalibrationData.mag_transform[RevoCalibration::MAG_TRANSFORM_R2C0]);
            good_calibration &= !IS_NAN(auxCalibrationData.mag_transform[RevoCalibration::MAG_TRANSFORM_R2C1]);
            good_calibration &= !IS_NAN(auxCalibrationData.mag_transform[RevoCalibration::MAG_TRANSFORM_R2C2]);

            good_calibration &= !IS_NAN(auxCalibrationData.mag_bias[RevoCalibration::MAG_BIAS_X]);
            good_calibration &= !IS_NAN(auxCalibrationData.mag_bias[RevoCalibration::MAG_BIAS_Y]);
            good_calibration &= !IS_NAN(auxCalibrationData.mag_bias[RevoCalibration::MAG_BIAS_Z]);
        }
    }
    // Check the accel calibration is good
    if (calibratingAccel) {
        good_calibration &= !IS_NAN(accelGyroSettingsData.accel_scale[AccelGyroSettings::ACCEL_SCALE_X]);
        good_calibration &= !IS_NAN(accelGyroSettingsData.accel_scale[AccelGyroSettings::ACCEL_SCALE_Y]);
        good_calibration &= !IS_NAN(accelGyroSettingsData.accel_scale[AccelGyroSettings::ACCEL_SCALE_Z]);
        good_calibration &= !IS_NAN(accelGyroSettingsData.accel_bias[AccelGyroSettings::ACCEL_BIAS_X]);
        good_calibration &= !IS_NAN(accelGyroSettingsData.accel_bias[AccelGyroSettings::ACCEL_BIAS_Y]);
        good_calibration &= !IS_NAN(accelGyroSettingsData.accel_bias[AccelGyroSettings::ACCEL_BIAS_Z]);
    }
    if (good_calibration) {
        m_dirty = true;
        if (calibratingMag) {
            result.revoCalibrationData   = revoCalibrationData;
            result.auxMagSettingsData = auxCalibrationData;
            displayInstructions(tr("Magnetometer calibration completed successfully."), WizardModel::Success);
        }
        if (calibratingAccel) {
            result.accelGyroSettingsData = accelGyroSettingsData;
            displayInstructions(tr("Accelerometer calibration completed successfully."), WizardModel::Success);
        }
    } else {
        progressChanged(0);
        displayInstructions(tr("Calibration failed! Please review the help and retry."), WizardModel::Failure);
    }
    // set to run again
    position = -1;
}

void SixPointCalibrationModel::CalcCalibration(QList<float> x, QList<float> y, QList<float> z, double Be_length, float calibrationMatrix[], float bias[])
{
    int vectSize = x.count();
    Eigen::VectorXf samples_x(vectSize);
    Eigen::VectorXf samples_y(vectSize);
    Eigen::VectorXf samples_z(vectSize);

    for (int i = 0; i < vectSize; i++) {
        samples_x(i) = x[i];
        samples_y(i) = y[i];
        samples_z(i) = z[i];
    }
    OpenPilot::CalibrationUtils::EllipsoidCalibrationResult result;
    OpenPilot::CalibrationUtils::EllipsoidCalibration(&samples_x, &samples_y, &samples_z, Be_length, &result, true);

    qDebug() << "Mag fitting results: ";
    qDebug() << "scale(" << result.Scale.coeff(0) << ", " << result.Scale.coeff(1) << ", " << result.Scale.coeff(2) << ")";
    qDebug() << "bias(" << result.Bias.coeff(0) << ", " << result.Bias.coeff(1) << ", " << result.Bias.coeff(2) << ")";
    qDebug() << "-----------------------------------";
    calibrationMatrix[RevoCalibration::MAG_TRANSFORM_R0C0] = result.CalibrationMatrix.coeff(0, 0);
    calibrationMatrix[RevoCalibration::MAG_TRANSFORM_R0C1] = result.CalibrationMatrix.coeff(0, 1);
    calibrationMatrix[RevoCalibration::MAG_TRANSFORM_R0C2] = result.CalibrationMatrix.coeff(0, 2);

    calibrationMatrix[RevoCalibration::MAG_TRANSFORM_R1C0] = result.CalibrationMatrix.coeff(1, 0);
    calibrationMatrix[RevoCalibration::MAG_TRANSFORM_R1C1] = result.CalibrationMatrix.coeff(1, 1);
    calibrationMatrix[RevoCalibration::MAG_TRANSFORM_R1C2] = result.CalibrationMatrix.coeff(1, 2);

    calibrationMatrix[RevoCalibration::MAG_TRANSFORM_R2C0] = result.CalibrationMatrix.coeff(2, 0);
    calibrationMatrix[RevoCalibration::MAG_TRANSFORM_R2C1] = result.CalibrationMatrix.coeff(2, 1);
    calibrationMatrix[RevoCalibration::MAG_TRANSFORM_R2C2] = result.CalibrationMatrix.coeff(2, 2);

    bias[RevoCalibration::MAG_BIAS_X] = result.Bias.coeff(0);
    bias[RevoCalibration::MAG_BIAS_Y] = result.Bias.coeff(1);
    bias[RevoCalibration::MAG_BIAS_Z] = result.Bias.coeff(2);
}

void SixPointCalibrationModel::save()
{
    if (!m_dirty) {
        return;
    }
    if (calibratingMag) {
        RevoCalibration::DataFields revoCalibrationData = revoCalibration->getData();

        for (int i = 0; i < RevoCalibration::MAG_TRANSFORM_NUMELEM; i++) {
            revoCalibrationData.mag_transform[i] = result.revoCalibrationData.mag_transform[i];
        }
        for (int i = 0; i < 3; i++) {
            revoCalibrationData.mag_bias[i] = result.revoCalibrationData.mag_bias[i];
        }

        revoCalibration->setData(revoCalibrationData);
        if (externalMagAvailable) {
            AuxMagSettings::DataFields auxCalibrationData = auxMagSettings->getData();
            // Note that Revo/AuxMag MAG_TRANSFORM_RxCx are interchangeable, an assertion at initialization enforces the structs are equal
            for (int i = 0; i < RevoCalibration::MAG_TRANSFORM_R2C2; i++) {
                auxCalibrationData.mag_transform[i] = result.auxMagSettingsData.mag_transform[i];
            }
            for (int i = 0; i < 3; i++) {
                auxCalibrationData.mag_bias[i] = result.auxMagSettingsData.mag_bias[i];
            }

            auxMagSettings->setData(auxCalibrationData);
        }
    }
    if (calibratingAccel) {
        AccelGyroSettings::DataFields accelGyroSettingsData = accelGyroSettings->getData();

        for (int i = 0; i < 3; i++) {
            accelGyroSettingsData.accel_scale[i] = result.accelGyroSettingsData.accel_scale[i];
            accelGyroSettingsData.accel_bias[i]  = result.accelGyroSettingsData.accel_bias[i];
        }

        accelGyroSettings->setData(accelGyroSettingsData);
    }

    m_dirty = false;
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

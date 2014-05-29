/**
 ******************************************************************************
 *
 * @file       levelcalibrationmodel.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2014.
 * @addtogroup board level calibration
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

#include "levelcalibrationmodel.h"
#include <attitudestate.h>
#include <attitudesettings.h>
#include "extensionsystem/pluginmanager.h"
#include "calibration/calibrationuiutils.h"

static const int LEVEL_SAMPLES = 100;
namespace OpenPilot {
LevelCalibrationModel::LevelCalibrationModel(QObject *parent) :
    QObject(parent)
{}


/******* Level calibration *******/
/**
 * Starts an accelerometer bias calibration.
 */
void LevelCalibrationModel::start()
{
    // Store and reset board rotation before calibration starts

    disableAllCalibrations();
    progressChanged(0);

    rot_data_pitch = 0;
    rot_data_roll  = 0;
    UAVObject::Metadata mdata;

    AttitudeState *attitudeState = AttitudeState::GetInstance(getObjectManager());
    Q_ASSERT(attitudeState);
    initialAttitudeStateMdata = attitudeState->getMetadata();
    mdata = initialAttitudeStateMdata;
    UAVObject::SetFlightTelemetryUpdateMode(mdata, UAVObject::UPDATEMODE_PERIODIC);
    mdata.flightTelemetryUpdatePeriod = 100;
    attitudeState->setMetadata(mdata);

    /* Show instructions and enable controls */
    displayInstructions(tr("Place horizontally and click Save Position button..."), true);
    displayVisualHelp(CALIBRATION_HELPER_PLANE_PREFIX + CALIBRATION_HELPER_IMAGE_NED);
    disableAllCalibrations();
    savePositionEnabledChanged(true);
    position = 0;
}

void LevelCalibrationModel::savePosition()
{
    QMutexLocker lock(&sensorsUpdateLock);

    Q_UNUSED(lock);

    savePositionEnabledChanged(false);

    rot_accum_pitch.clear();
    rot_accum_roll.clear();

    collectingData = true;

    AttitudeState *attitudeState = AttitudeState::GetInstance(getObjectManager());
    Q_ASSERT(attitudeState);
    connect(attitudeState, SIGNAL(objectUpdated(UAVObject *)), this, SLOT(getSample(UAVObject *)));

    displayInstructions(tr("Hold..."), false);
}

/**
   Updates the accel bias raw values
 */
void LevelCalibrationModel::getSample(UAVObject *obj)
{
    QMutexLocker lock(&sensorsUpdateLock);

    Q_UNUSED(lock);

    switch (obj->getObjID()) {
    case AttitudeState::OBJID:
    {
        AttitudeState *attitudeState = AttitudeState::GetInstance(getObjectManager());
        Q_ASSERT(attitudeState);
        AttitudeState::DataFields attitudeStateData = attitudeState->getData();
        rot_accum_roll.append(attitudeStateData.Roll);
        rot_accum_pitch.append(attitudeStateData.Pitch);
        break;
    }
    default:
        Q_ASSERT(0);
    }

    // Work out the progress based on whichever has less
    double p1 = (double)rot_accum_roll.size() / (double)LEVEL_SAMPLES;
    progressChanged(p1 * 100);

    if (rot_accum_roll.size() >= LEVEL_SAMPLES &&
        collectingData == true) {
        collectingData = false;

        AttitudeState *attitudeState = AttitudeState::GetInstance(getObjectManager());

        disconnect(attitudeState, SIGNAL(objectUpdated(UAVObject *)), this, SLOT(getSample(UAVObject *)));

        position++;
        switch (position) {
        case 1:
            rot_data_pitch = OpenPilot::CalibrationUtils::listMean(rot_accum_pitch);
            rot_data_roll  = OpenPilot::CalibrationUtils::listMean(rot_accum_roll);

            displayInstructions(tr("Leave horizontally, rotate 180° along yaw axis and click Save Position button..."), false);
            displayVisualHelp(CALIBRATION_HELPER_PLANE_PREFIX + CALIBRATION_HELPER_IMAGE_SWD);

            disableAllCalibrations();

            savePositionEnabledChanged(true);
            break;
        case 2:
            rot_data_pitch += OpenPilot::CalibrationUtils::listMean(rot_accum_pitch);
            rot_data_pitch /= 2;
            rot_data_roll  += OpenPilot::CalibrationUtils::listMean(rot_accum_roll);
            rot_data_roll  /= 2;
            attitudeState->setMetadata(initialAttitudeStateMdata);
            compute();
            enableAllCalibrations();
            displayVisualHelp(CALIBRATION_HELPER_IMAGE_EMPTY);
            displayInstructions(tr("Board leveling computed successfully."), false);
            break;
        }
    }
}
void LevelCalibrationModel::compute()
{
    enableAllCalibrations();

    AttitudeSettings *attitudeSettings = AttitudeSettings::GetInstance(getObjectManager());
    Q_ASSERT(attitudeSettings);
    AttitudeSettings::DataFields attitudeSettingsData = attitudeSettings->getData();

    // Update the biases based on collected data
    // "rotate" the board in the opposite direction as the calculated offset
    attitudeSettingsData.BoardLevelTrim[AttitudeSettings::BOARDLEVELTRIM_PITCH] -= rot_data_pitch;
    attitudeSettingsData.BoardLevelTrim[AttitudeSettings::BOARDLEVELTRIM_ROLL]  -= rot_data_roll;

    attitudeSettings->setData(attitudeSettingsData);
    attitudeSettings->updated();
}
UAVObjectManager *LevelCalibrationModel::getObjectManager()
{
    ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
    UAVObjectManager *objMngr = pm->getObject<UAVObjectManager>();

    Q_ASSERT(objMngr);
    return objMngr;
}
}

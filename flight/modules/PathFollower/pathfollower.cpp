/**
 ******************************************************************************
 *
 * @file       pathfollower.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      This module compared @ref PositionActuatl to @ref ActiveWaypoint
 * and sets @ref AttitudeDesired.  It only does this when the FlightMode field
 * of @ref ManualControlCommand is Auto.
 *
 * @see        The GNU Public License (GPL) Version 3
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

/**
 * Input object: PathDesired
 * Input object: PositionState
 * Input object: ManualControlCommand
 * Output object: StabilizationDesired
 *
 * This module acts as "autopilot" - it controls the setpoints of stabilization
 * based on current flight situation and desired flight path (PathDesired) as
 * directed by flightmode selection or pathplanner
 * This is a periodic delayed callback module
 *
 * Modules have no API, all communication to other modules is done through UAVObjects.
 * However modules may use the API exposed by shared libraries.
 * See the OpenPilot wiki for more details.
 * http://www.openpilot.org/OpenPilot_Application_Architecture
 *
 */

extern "C" {
#include <openpilot.h>

#include <callbackinfo.h>

#include <math.h>
#include <pid.h>
#include <CoordinateConversions.h>
#include <sin_lookup.h>
#include <pathdesired.h>
#include <paths.h>
#include "plans.h"
#include <sanitycheck.h>

#include <groundpathfollowersettings.h>
#include <fixedwingpathfollowersettings.h>
#include <fixedwingpathfollowerstatus.h>
#include <vtolpathfollowersettings.h>
#include <flightstatus.h>
#include <flightmodesettings.h>
#include <pathstatus.h>
#include <positionstate.h>
#include <velocitystate.h>
#include <velocitydesired.h>
#include <stabilizationdesired.h>
#include <airspeedstate.h>
#include <attitudestate.h>
#include <takeofflocation.h>
#include <poilocation.h>
#include <manualcontrolcommand.h>
#include <systemsettings.h>
#include <stabilizationbank.h>
#include <vtolselftuningstats.h>
#include <pathsummary.h>
#include <pidstatus.h>
}

#include "fsm_land.h"
#include "FSMBrake.h"
#include "PathFollowerControlLanding.h"
#include "PathFollowerControlVelocityRoam.h"
#include "PathFollowerControlBrake.h"
#include "PathFollowerControlFly.h"
#include "FixedWingControlFly.h"
#include "GroundDriveController.h"

// Private constants

#define CALLBACK_PRIORITY      CALLBACK_PRIORITY_LOW
#define CBTASK_PRIORITY        CALLBACK_TASK_FLIGHTCONTROL

#define PF_IDLE_UPDATE_RATE_MS 100

#define STACK_SIZE_BYTES       2048


// Private types

// Private variables
static DelayedCallbackInfo *pathFollowerCBInfo;
static uint32_t updatePeriod = PF_IDLE_UPDATE_RATE_MS;
static FrameType_t frameType;
static PathStatusData pathStatus;
static PathDesiredData pathDesired;
static FixedWingPathFollowerSettingsData fixedWingPathFollowerSettings;
static GroundPathFollowerSettingsData groundPathFollowerSettings;
static VtolPathFollowerSettingsData vtolPathFollowerSettings;
static FlightStatusData flightStatus;
static PathFollowerControl *activeController = 0;

// Private functions
static void pathFollowerTask(void);
static void SettingsUpdatedCb(UAVObjEvent *ev);
static void airspeedStateUpdatedCb(UAVObjEvent *ev);
static void pathFollowerObjectiveUpdatedCb(UAVObjEvent *ev);
static void flightStatusUpdatedCb(UAVObjEvent *ev);
static void updateFixedAttitude(float *attitude);

/**
 * Initialise the module, called on startup
 * \returns 0 on success or -1 if initialisation failed
 */
int32_t PathFollowerStart()
{
    // Start main task
    PathStatusGet(&pathStatus);
    SettingsUpdatedCb(NULL);
    PIOS_CALLBACKSCHEDULER_Dispatch(pathFollowerCBInfo);

    return 0;
}

/**
 * Initialise the module, called on startup
 * \returns 0 on success or -1 if initialisation failed
 */
int32_t PathFollowerInitialize()
{
    // initialize objects
    GroundPathFollowerSettingsInitialize();
    FixedWingPathFollowerSettingsInitialize();
    FixedWingPathFollowerStatusInitialize();
    VtolPathFollowerSettingsInitialize();
    FlightStatusInitialize();
    FlightModeSettingsInitialize();
    PathStatusInitialize();
    PathSummaryInitialize();
    PathDesiredInitialize();
    PositionStateInitialize();
    VelocityStateInitialize();
    VelocityDesiredInitialize();
    StabilizationDesiredInitialize();
    AirspeedStateInitialize();
    AttitudeStateInitialize();
    TakeOffLocationInitialize();
    PoiLocationInitialize();
    ManualControlCommandInitialize();
    SystemSettingsInitialize();
    StabilizationBankInitialize();
    VtolSelfTuningStatsInitialize();
    PIDStatusInitialize();

    // Initialise the autopilot mode implementations that use an FSM
    FSMLand::instance()->Initialize(&vtolPathFollowerSettings, &pathDesired, &flightStatus);
    PathFollowerControlLanding::instance()->Initialize((PathFollowerFSM *)FSMLand::instance(),
                                                       &vtolPathFollowerSettings, &pathDesired, &flightStatus, &pathStatus);
    PathFollowerControlVelocityRoam::instance()->Initialize(&vtolPathFollowerSettings, &pathDesired, &pathStatus);
    PathFollowerControlFly::instance()->Initialize(&vtolPathFollowerSettings, &pathDesired, &pathStatus);
    FixedWingControlFly::instance()->Initialize(&fixedWingPathFollowerSettings, &pathDesired, &pathStatus);
    GroundDriveController::instance()->Initialize(&groundPathFollowerSettings, &pathDesired, &pathStatus);
    PathFollowerControlBrake::instance()->Initialize((PathFollowerFSM *)FSMBrake::instance(),
                                                     &vtolPathFollowerSettings,
                                                     &pathDesired,
                                                     &flightStatus,
                                                     &pathStatus);

    // Create object queue
    pathFollowerCBInfo = PIOS_CALLBACKSCHEDULER_Create(&pathFollowerTask, CALLBACK_PRIORITY, CBTASK_PRIORITY, CALLBACKINFO_RUNNING_PATHFOLLOWER, STACK_SIZE_BYTES);
    FixedWingPathFollowerSettingsConnectCallback(&SettingsUpdatedCb);
    GroundPathFollowerSettingsConnectCallback(&SettingsUpdatedCb);
    VtolPathFollowerSettingsConnectCallback(&SettingsUpdatedCb);
    PathDesiredConnectCallback(&pathFollowerObjectiveUpdatedCb);
    FlightStatusConnectCallback(&flightStatusUpdatedCb);
    SystemSettingsConnectCallback(&SettingsUpdatedCb);
    AirspeedStateConnectCallback(&airspeedStateUpdatedCb);

    return 0;
}
MODULE_INITCALL(PathFollowerInitialize, PathFollowerStart);


static void pathFollowerSetActiveController(void)
{
    if (activeController == 0) {
        switch (frameType) {
        case FRAME_TYPE_MULTIROTOR:
        case FRAME_TYPE_HELI:

            switch (pathDesired.Mode) {
            case PATHDESIRED_MODE_BRAKE: // brake then hold sequence controller
                activeController = PathFollowerControlBrake::instance();
                activeController->Activate();
                break;
            case PATHDESIRED_MODE_VELOCITY: // velocity roam controller
                activeController = PathFollowerControlVelocityRoam::instance();
                activeController->Activate();
                break;
            case PATHDESIRED_MODE_GOTOENDPOINT:
            case PATHDESIRED_MODE_FOLLOWVECTOR:
            case PATHDESIRED_MODE_CIRCLERIGHT:
            case PATHDESIRED_MODE_CIRCLELEFT:
                activeController = PathFollowerControlFly::instance();
                activeController->Activate();
                break;
            case PATHDESIRED_MODE_LAND: // land with optional velocity roam option
                activeController = PathFollowerControlLanding::instance();
                activeController->Activate();
                break;
            default:
                activeController = 0;
                break;
            }
            break;

        case FRAME_TYPE_FIXED_WING:

          switch (pathDesired.Mode) {
           case PATHDESIRED_MODE_GOTOENDPOINT:
           case PATHDESIRED_MODE_FOLLOWVECTOR:
           case PATHDESIRED_MODE_CIRCLERIGHT:
           case PATHDESIRED_MODE_CIRCLELEFT:
               activeController = FixedWingControlFly::instance();
               activeController->Activate();
               break;
           default:
               activeController = 0;
               break;
           }
           break;

        case FRAME_TYPE_GROUND:

          switch (pathDesired.Mode) {
           case PATHDESIRED_MODE_GOTOENDPOINT:
           case PATHDESIRED_MODE_FOLLOWVECTOR:
           case PATHDESIRED_MODE_CIRCLERIGHT:
           case PATHDESIRED_MODE_CIRCLELEFT:
               activeController = GroundDriveController::instance();
               activeController->Activate();
               break;
           default:
               activeController = 0;
               break;
           }
           break;

        default:
            activeController = 0;
            break;
        }
    }
}

/**
 * Module thread, should not return.
 */
static void pathFollowerTask(void)
{
    FlightStatusGet(&flightStatus);

    if (flightStatus.ControlChain.PathFollower != FLIGHTSTATUS_CONTROLCHAIN_TRUE) {
        AlarmsSet(SYSTEMALARMS_ALARM_GUIDANCE, SYSTEMALARMS_ALARM_UNINITIALISED);
        PIOS_CALLBACKSCHEDULER_Schedule(pathFollowerCBInfo, PF_IDLE_UPDATE_RATE_MS, CALLBACK_UPDATEMODE_SOONER);
        return;
    }

    pathStatus.UID    = pathDesired.UID;
    pathStatus.Status = PATHSTATUS_STATUS_INPROGRESS;

    pathFollowerSetActiveController();

    if (activeController) {
        activeController->UpdateAutoPilot();
        PIOS_CALLBACKSCHEDULER_Schedule(pathFollowerCBInfo, updatePeriod, CALLBACK_UPDATEMODE_SOONER);
        return;
    }

    switch (pathDesired.Mode) {
    case PATHDESIRED_MODE_FIXEDATTITUDE:
        updateFixedAttitude(pathDesired.ModeParameters);
        AlarmsSet(SYSTEMALARMS_ALARM_GUIDANCE, SYSTEMALARMS_ALARM_OK);
        break;
    case PATHDESIRED_MODE_DISARMALARM:
        AlarmsSet(SYSTEMALARMS_ALARM_GUIDANCE, SYSTEMALARMS_ALARM_CRITICAL);
        break;
    default:
        pathStatus.Status = PATHSTATUS_STATUS_CRITICAL;
        AlarmsSet(SYSTEMALARMS_ALARM_GUIDANCE, SYSTEMALARMS_ALARM_ERROR);
        break;
    }
    PathStatusSet(&pathStatus);

    PIOS_CALLBACKSCHEDULER_Schedule(pathFollowerCBInfo, updatePeriod, CALLBACK_UPDATEMODE_SOONER);
}


static void pathFollowerObjectiveUpdatedCb(__attribute__((unused)) UAVObjEvent *ev)
{
    PathDesiredGet(&pathDesired);

    if (activeController && pathDesired.Mode != activeController->Mode()) {
        activeController->Deactivate();
        activeController = 0;
    }

    pathFollowerSetActiveController();

    if (activeController) {
        activeController->ObjectiveUpdated();
    }
}

// we use this to deactivate active controllers as the pathdesired
// objective never gets updated with switching to a non-pathfollower flight mode
static void flightStatusUpdatedCb(__attribute__((unused)) UAVObjEvent *ev)
{
    if (!activeController) {
        return;
    }

    FlightStatusControlChainData controlChain;
    FlightStatusControlChainGet(&controlChain);

    if (controlChain.PathFollower != FLIGHTSTATUS_CONTROLCHAIN_TRUE) {
        activeController->Deactivate();
        activeController = 0;
    }
}


static void SettingsUpdatedCb(__attribute__((unused)) UAVObjEvent *ev)
{
    FixedWingPathFollowerSettingsGet(&fixedWingPathFollowerSettings);
    GroundPathFollowerSettingsGet(&groundPathFollowerSettings);
    VtolPathFollowerSettingsGet(&vtolPathFollowerSettings);

    frameType = GetCurrentFrameType();

    if (frameType == FRAME_TYPE_CUSTOM) {
        switch (vtolPathFollowerSettings.TreatCustomCraftAs) {
        case VTOLPATHFOLLOWERSETTINGS_TREATCUSTOMCRAFTAS_FIXEDWING:
            frameType = FRAME_TYPE_FIXED_WING;
            break;
        case VTOLPATHFOLLOWERSETTINGS_TREATCUSTOMCRAFTAS_VTOL:
            frameType = FRAME_TYPE_MULTIROTOR;
            break;
        case VTOLPATHFOLLOWERSETTINGS_TREATCUSTOMCRAFTAS_GROUND:
            frameType = FRAME_TYPE_GROUND;
            break;
        }
    }
    switch (frameType) {
    case FRAME_TYPE_MULTIROTOR:
    case FRAME_TYPE_HELI:
        updatePeriod = vtolPathFollowerSettings.UpdatePeriod;
        break;
    case FRAME_TYPE_FIXED_WING:
        updatePeriod = fixedWingPathFollowerSettings.UpdatePeriod;
        break;
    case FRAME_TYPE_GROUND:
        updatePeriod = groundPathFollowerSettings.UpdatePeriod;
      break;
    default:
        updatePeriod = fixedWingPathFollowerSettings.UpdatePeriod;
        break;
    }

    if (activeController) {
        activeController->SettingsUpdated();
    }
}


static void airspeedStateUpdatedCb(__attribute__((unused)) UAVObjEvent *ev)
{
  FixedWingControlFly::instance()->AirspeedStateUpdatedCb(ev);
}



/**
 * Compute desired attitude from a fixed preset
 *
 */
static void updateFixedAttitude(float *attitude)
{
    StabilizationDesiredData stabDesired;

    StabilizationDesiredGet(&stabDesired);
    stabDesired.Roll   = attitude[0];
    stabDesired.Pitch  = attitude[1];
    stabDesired.Yaw    = attitude[2];
    stabDesired.Thrust = attitude[3];
    stabDesired.StabilizationMode.Roll   = STABILIZATIONDESIRED_STABILIZATIONMODE_ATTITUDE;
    stabDesired.StabilizationMode.Pitch  = STABILIZATIONDESIRED_STABILIZATIONMODE_ATTITUDE;
    stabDesired.StabilizationMode.Yaw    = STABILIZATIONDESIRED_STABILIZATIONMODE_RATE;
    stabDesired.StabilizationMode.Thrust = STABILIZATIONDESIRED_STABILIZATIONMODE_MANUAL;
    StabilizationDesiredSet(&stabDesired);
}

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

// Private constants

#define CALLBACK_PRIORITY      CALLBACK_PRIORITY_LOW
#define CBTASK_PRIORITY        CALLBACK_TASK_FLIGHTCONTROL

#define PF_IDLE_UPDATE_RATE_MS 100

#define STACK_SIZE_BYTES       2048

#define DEADBAND_HIGH          0.10f
#define DEADBAND_LOW           -0.10f


// Private types

struct Globals {
    struct pid  PIDposH[2];
    struct pid  PIDposV;
    struct pid  PIDcourse;
    struct pid  PIDspeed;
    struct pid  PIDpower;
    float poiRadius;
    float vtolEmergencyFallback;
    FrameType_t frameType;
    bool vtolEmergencyFallbackSwitch;
};

// Private variables
static DelayedCallbackInfo *pathFollowerCBInfo;
static uint32_t updatePeriod = PF_IDLE_UPDATE_RATE_MS;
static struct Globals global;
static PathStatusData pathStatus;
static PathDesiredData pathDesired;
static FixedWingPathFollowerSettingsData fixedWingPathFollowerSettings;
static VtolPathFollowerSettingsData vtolPathFollowerSettings;
static FlightStatusData flightStatus;

// correct speed by measured airspeed
static float indicatedAirspeedStateBias = 0.0f;

static PathFollowerControlLanding *activeController = 0;


// Private functions
static void pathFollowerTask(void);
static void resetGlobals();
static void SettingsUpdatedCb(UAVObjEvent *ev);
static uint8_t updateAutoPilotFixedWing();
// static void processPOI();
static void updatePathVelocity(float kFF, bool limited);
static uint8_t updateFixedDesiredAttitude();
static void updateFixedAttitude(float *attitude);
static void airspeedStateUpdatedCb(UAVObjEvent *ev);
static void pathFollowerObjectiveUpdatedCb(UAVObjEvent *ev);
static void flightStatusUpdatedCb(UAVObjEvent *ev);
static bool correctCourse(float *C, float *V, float *F, float s);

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
    PathFollowerControlBrake::instance()->Initialize((PathFollowerFSM *)FSMBrake::instance(),
                                                     &vtolPathFollowerSettings,
                                                     &pathDesired,
                                                     &flightStatus,
                                                     &pathStatus);

    // reset integrals
    resetGlobals();

    // Create object queue
    pathFollowerCBInfo = PIOS_CALLBACKSCHEDULER_Create(&pathFollowerTask, CALLBACK_PRIORITY, CBTASK_PRIORITY, CALLBACKINFO_RUNNING_PATHFOLLOWER, STACK_SIZE_BYTES);
    FixedWingPathFollowerSettingsConnectCallback(&SettingsUpdatedCb);
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
        switch (global.frameType) {
        case FRAME_TYPE_MULTIROTOR:
        case FRAME_TYPE_HELI:

            switch (pathDesired.Mode) {
            case PATHDESIRED_MODE_BRAKE: // brake then hold sequence controller
                activeController = (PathFollowerControlLanding *)PathFollowerControlBrake::instance();
                activeController->Activate();
                break;
            case PATHDESIRED_MODE_VELOCITY: // velocity roam controller
                activeController = (PathFollowerControlLanding *)PathFollowerControlVelocityRoam::instance();
                activeController->Activate();
                break;
            case PATHDESIRED_MODE_FLYENDPOINT:
            case PATHDESIRED_MODE_FLYVECTOR:
            case PATHDESIRED_MODE_FLYCIRCLERIGHT:
            case PATHDESIRED_MODE_FLYCIRCLELEFT:
                activeController = (PathFollowerControlLanding *)PathFollowerControlFly::instance();
                activeController->Activate();
                break;
            case PATHDESIRED_MODE_LAND: // land with optional velocity roam option
                activeController = (PathFollowerControlLanding *)PathFollowerControlLanding::instance();
                activeController->Activate();
                break;
            default:
                activeController = 0;
                break;
            }


            break;
        case FRAME_TYPE_FIXED_WING:
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
        resetGlobals();
        AlarmsSet(SYSTEMALARMS_ALARM_GUIDANCE, SYSTEMALARMS_ALARM_UNINITIALISED);
        PIOS_CALLBACKSCHEDULER_Schedule(pathFollowerCBInfo, PF_IDLE_UPDATE_RATE_MS, CALLBACK_UPDATEMODE_SOONER);
        return;
    }

    pathStatus.UID    = pathDesired.UID;
    pathStatus.Status = PATHSTATUS_STATUS_INPROGRESS;

#if 0
    if (flightStatus.FlightMode == FLIGHTSTATUS_FLIGHTMODE_POI) { // TODO Hack from vtolpathfollower, move into manualcontrol!
        processPOI();
    }
#endif

    pathFollowerSetActiveController();

    if (activeController) {
        activeController->UpdateAutoPilot();
        PIOS_CALLBACKSCHEDULER_Schedule(pathFollowerCBInfo, updatePeriod, CALLBACK_UPDATEMODE_SOONER);
        return;
    }


    switch (pathDesired.Mode) {
    case PATHDESIRED_MODE_FLYENDPOINT:
    case PATHDESIRED_MODE_FLYVECTOR:
    case PATHDESIRED_MODE_FLYCIRCLERIGHT:
    case PATHDESIRED_MODE_FLYCIRCLELEFT:
    {
      if (global.frameType == FRAME_TYPE_FIXED_WING) {
        uint8_t result = updateAutoPilotFixedWing();
        if (result) {
            AlarmsSet(SYSTEMALARMS_ALARM_GUIDANCE, SYSTEMALARMS_ALARM_OK);
        } else {
            pathStatus.Status = PATHSTATUS_STATUS_CRITICAL;
            AlarmsSet(SYSTEMALARMS_ALARM_GUIDANCE, SYSTEMALARMS_ALARM_WARNING);
        }
      }
      else {
        pathStatus.Status = PATHSTATUS_STATUS_CRITICAL;
        AlarmsSet(SYSTEMALARMS_ALARM_GUIDANCE, SYSTEMALARMS_ALARM_ERROR);
      }
    }
    break;
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
    VtolPathFollowerSettingsGet(&vtolPathFollowerSettings);

    global.frameType = GetCurrentFrameType();

    if (global.frameType == FRAME_TYPE_CUSTOM || global.frameType == FRAME_TYPE_GROUND) {
        switch (vtolPathFollowerSettings.TreatCustomCraftAs) {
        case VTOLPATHFOLLOWERSETTINGS_TREATCUSTOMCRAFTAS_FIXEDWING:
            global.frameType = FRAME_TYPE_FIXED_WING;
            break;
        case VTOLPATHFOLLOWERSETTINGS_TREATCUSTOMCRAFTAS_VTOL:
            global.frameType = FRAME_TYPE_MULTIROTOR;
            break;
        }
    }
    switch (global.frameType) {
    case FRAME_TYPE_MULTIROTOR:
    case FRAME_TYPE_HELI:
        updatePeriod = vtolPathFollowerSettings.UpdatePeriod;
        break;
    case FRAME_TYPE_FIXED_WING:
    default:
        updatePeriod = fixedWingPathFollowerSettings.UpdatePeriod;
        break;
    }

    // fixed wing PID only
    pid_configure(&global.PIDposH[0], fixedWingPathFollowerSettings.HorizontalPosP, 0.0f, 0.0f, 0.0f);
    pid_configure(&global.PIDposH[1], fixedWingPathFollowerSettings.HorizontalPosP, 0.0f, 0.0f, 0.0f);
    pid_configure(&global.PIDposV, fixedWingPathFollowerSettings.VerticalPosP, 0.0f, 0.0f, 0.0f);

    pid_configure(&global.PIDcourse, fixedWingPathFollowerSettings.CoursePI.Kp, fixedWingPathFollowerSettings.CoursePI.Ki, 0.0f, fixedWingPathFollowerSettings.CoursePI.ILimit);
    pid_configure(&global.PIDspeed, fixedWingPathFollowerSettings.SpeedPI.Kp, fixedWingPathFollowerSettings.SpeedPI.Ki, 0.0f, fixedWingPathFollowerSettings.SpeedPI.ILimit);
    pid_configure(&global.PIDpower, fixedWingPathFollowerSettings.PowerPI.Kp, fixedWingPathFollowerSettings.PowerPI.Ki, 0.0f, fixedWingPathFollowerSettings.PowerPI.ILimit);

    if (activeController) {
        activeController->SettingsUpdated();
    }
}


static void airspeedStateUpdatedCb(__attribute__((unused)) UAVObjEvent *ev)
{
    AirspeedStateData airspeedState;
    VelocityStateData velocityState;

    AirspeedStateGet(&airspeedState);
    VelocityStateGet(&velocityState);
    float airspeedVector[2];
    float yaw;
    AttitudeStateYawGet(&yaw);
    airspeedVector[0] = cos_lookup_deg(yaw);
    airspeedVector[1] = sin_lookup_deg(yaw);
    // vector projection of groundspeed on airspeed vector to handle both forward and backwards movement
    float groundspeedProjection = velocityState.North * airspeedVector[0] + velocityState.East * airspeedVector[1];

    indicatedAirspeedStateBias = airspeedState.CalibratedAirspeed - groundspeedProjection;
    // note - we do fly by Indicated Airspeed (== calibrated airspeed) however
    // since airspeed is updated less often than groundspeed, we use sudden
    // changes to groundspeed to offset the airspeed by the same measurement.
    // This has a side effect that in the absence of any airspeed updates, the
    // pathfollower will fly using groundspeed.
}


/**
 * reset integrals
 */
static void resetGlobals()
{
    pid_zero(&global.PIDposH[0]);
    pid_zero(&global.PIDposH[1]);
    pid_zero(&global.PIDposV);
    pid_zero(&global.PIDcourse);
    pid_zero(&global.PIDspeed);
    pid_zero(&global.PIDpower);
    global.poiRadius = 0.0f;
    global.vtolEmergencyFallback = 0;
    global.vtolEmergencyFallbackSwitch = false;

    pathStatus.path_time = 0.0f;
}

/**
 * fixed wing autopilot:
 * straight forward:
 * 1. update path velocity for limited motion crafts
 * 2. update attitude according to default fixed wing pathfollower algorithm
 */
static uint8_t updateAutoPilotFixedWing()
{
    updatePathVelocity(fixedWingPathFollowerSettings.CourseFeedForward, true);
    return updateFixedDesiredAttitude();
}

#if 0
/**
 * process POI control logic TODO: this should most likely go into manualcontrol!!!!
 * TODO: the whole process of POI handling likely needs cleanup and rethinking, might be broken since manualcontrol was refactored currently
 **/
static void processPOI()
{
    const float dT = updatePeriod / 1000.0f;


    PositionStateData positionState;

    PositionStateGet(&positionState);
    // TODO put commented out camera feature code back in place either
    // permanently or optionally or remove it
    // CameraDesiredData cameraDesired;
    // CameraDesiredGet(&cameraDesired);
    StabilizationDesiredData stabDesired;
    StabilizationDesiredGet(&stabDesired);
    PoiLocationData poi;
    PoiLocationGet(&poi);

    float dLoc[3];
    float yaw = 0;
    // TODO camera feature
    /*float elevation = 0;*/

    dLoc[0] = positionState.North - poi.North;
    dLoc[1] = positionState.East - poi.East;
    dLoc[2] = positionState.Down - poi.Down;

    if (dLoc[1] < 0) {
        yaw = RAD2DEG(atan2f(dLoc[1], dLoc[0])) + 180.0f;
    } else {
        yaw = RAD2DEG(atan2f(dLoc[1], dLoc[0])) - 180.0f;
    }

    // distance
    float distance = sqrtf(powf(dLoc[0], 2.0f) + powf(dLoc[1], 2.0f));

    ManualControlCommandData manualControlData;
    ManualControlCommandGet(&manualControlData);

    float changeRadius = 0;
    // Move closer or further, radially
    if (manualControlData.Pitch > DEADBAND_HIGH) {
        changeRadius = (manualControlData.Pitch - DEADBAND_HIGH) * dT * 100.0f;
    } else if (manualControlData.Pitch < DEADBAND_LOW) {
        changeRadius = (manualControlData.Pitch - DEADBAND_LOW) * dT * 100.0f;
    }

    // move along circular path
    float pathAngle = 0;
    if (manualControlData.Roll > DEADBAND_HIGH) {
        pathAngle = -(manualControlData.Roll - DEADBAND_HIGH) * dT * 300.0f;
    } else if (manualControlData.Roll < DEADBAND_LOW) {
        pathAngle = -(manualControlData.Roll - DEADBAND_LOW) * dT * 300.0f;
    } else if (manualControlData.Roll >= DEADBAND_LOW && manualControlData.Roll <= DEADBAND_HIGH) {
        // change radius only when not circling
        global.poiRadius = distance + changeRadius;
    }

    // don't try to move any closer
    if (global.poiRadius >= 3.0f || changeRadius > 0) {
        if (fabsf(pathAngle) > 0.0f || fabsf(changeRadius) > 0.0f) {
            pathDesired.End.North = poi.North + (global.poiRadius * cosf(DEG2RAD(pathAngle + yaw - 180.0f)));
            pathDesired.End.East  = poi.East + (global.poiRadius * sinf(DEG2RAD(pathAngle + yaw - 180.0f)));
            pathDesired.StartingVelocity = 1.0f;
            pathDesired.EndingVelocity   = 0.0f;
            pathDesired.Mode = PATHDESIRED_MODE_FLYENDPOINT;
            PathDesiredSet(&pathDesired);
        }
    }
    // not above
    if (distance >= 3.0f) {
        // TODO camera feature
        // You can feed this into camerastabilization
        /*elevation = RAD2DEG(atan2f(dLoc[2],distance));*/

        // cameraDesired.Yaw=yaw;
        // cameraDesired.PitchOrServo2=elevation;

        // CameraDesiredSet(&cameraDesired);
    }
}
#endif


/**
 * Compute desired velocity from the current position and path
 */
static void updatePathVelocity(float kFF, bool limited)
{
    PositionStateData positionState;

    PositionStateGet(&positionState);
    VelocityStateData velocityState;
    VelocityStateGet(&velocityState);
    VelocityDesiredData velocityDesired;

    const float dT = updatePeriod / 1000.0f;

    // look ahead kFF seconds
    float cur[3]   = { positionState.North + (velocityState.North * kFF),
                       positionState.East + (velocityState.East * kFF),
                       positionState.Down + (velocityState.Down * kFF) };
    struct path_status progress;
    path_progress(&pathDesired, cur, &progress);

    // calculate velocity - can be zero if waypoints are too close
    velocityDesired.North = progress.path_vector[0];
    velocityDesired.East  = progress.path_vector[1];
    velocityDesired.Down  = progress.path_vector[2];

    if (limited &&
        // if a plane is crossing its desired flightpath facing the wrong way (away from flight direction)
        // it would turn towards the flightpath to get on its desired course. This however would reverse the correction vector
        // once it crosses the flightpath again, which would make it again turn towards the flightpath (but away from its desired heading)
        // leading to an S-shape snake course the wrong way
        // this only happens especially if HorizontalPosP is too high, as otherwise the angle between velocity desired and path_direction won't
        // turn steep unless there is enough space complete the turn before crossing the flightpath
        // in this case the plane effectively needs to be turned around
        // indicators:
        // difference between correction_direction and velocitystate >90 degrees and
        // difference between path_direction and velocitystate >90 degrees  ( 4th sector, facing away from everything )
        // fix: ignore correction, steer in path direction until the situation has become better (condition doesn't apply anymore)
        // calculating angles < 90 degrees through dot products
        (vector_lengthf(progress.path_vector, 2) > 1e-6f) &&
        ((progress.path_vector[0] * velocityState.North + progress.path_vector[1] * velocityState.East) < 0.0f) &&
        ((progress.correction_vector[0] * velocityState.North + progress.correction_vector[1] * velocityState.East) < 0.0f)) {
        ;
    } else {
        // calculate correction
        velocityDesired.North += pid_apply(&global.PIDposH[0], progress.correction_vector[0], dT);
        velocityDesired.East  += pid_apply(&global.PIDposH[1], progress.correction_vector[1], dT);
    }
    velocityDesired.Down += pid_apply(&global.PIDposV, progress.correction_vector[2], dT);

    // update pathstatus
    pathStatus.error      = progress.error;
    pathStatus.fractional_progress  = progress.fractional_progress;
    pathStatus.path_direction_north = progress.path_vector[0];
    pathStatus.path_direction_east  = progress.path_vector[1];
    pathStatus.path_direction_down  = progress.path_vector[2];

    pathStatus.correction_direction_north = progress.correction_vector[0];
    pathStatus.correction_direction_east  = progress.correction_vector[1];
    pathStatus.correction_direction_down  = progress.correction_vector[2];


    VelocityDesiredSet(&velocityDesired);
}


/**
 * Compute desired attitude from the desired velocity for fixed wing craft
 */
static uint8_t updateFixedDesiredAttitude()
{
    uint8_t result = 1;

    const float dT = updatePeriod / 1000.0f; // Convert from [ms] to [s]

    VelocityDesiredData velocityDesired;
    VelocityStateData velocityState;
    StabilizationDesiredData stabDesired;
    AttitudeStateData attitudeState;
    FixedWingPathFollowerStatusData fixedWingPathFollowerStatus;
    AirspeedStateData airspeedState;
    SystemSettingsData systemSettings;

    float groundspeedProjection;
    float indicatedAirspeedState;
    float indicatedAirspeedDesired;
    float airspeedError;

    float pitchCommand;

    float descentspeedDesired;
    float descentspeedError;
    float powerCommand;

    float airspeedVector[2];
    float fluidMovement[2];
    float courseComponent[2];
    float courseError;
    float courseCommand;

    FixedWingPathFollowerStatusGet(&fixedWingPathFollowerStatus);

    VelocityStateGet(&velocityState);
    StabilizationDesiredGet(&stabDesired);
    VelocityDesiredGet(&velocityDesired);
    AttitudeStateGet(&attitudeState);
    AirspeedStateGet(&airspeedState);
    SystemSettingsGet(&systemSettings);

    /**
     * Compute speed error and course
     */
    // missing sensors for airspeed-direction we have to assume within
    // reasonable error that measured airspeed is actually the airspeed
    // component in forward pointing direction
    // airspeedVector is normalized
    airspeedVector[0]     = cos_lookup_deg(attitudeState.Yaw);
    airspeedVector[1]     = sin_lookup_deg(attitudeState.Yaw);

    // current ground speed projected in forward direction
    groundspeedProjection = velocityState.North * airspeedVector[0] + velocityState.East * airspeedVector[1];

    // note that airspeedStateBias is ( calibratedAirspeed - groundspeedProjection ) at the time of measurement,
    // but thanks to accelerometers,  groundspeedProjection reacts faster to changes in direction
    // than airspeed and gps sensors alone
    indicatedAirspeedState = groundspeedProjection + indicatedAirspeedStateBias;

    // fluidMovement is a vector describing the aproximate movement vector of
    // the surrounding fluid in 2d space (aka wind vector)
    fluidMovement[0] = velocityState.North - (indicatedAirspeedState * airspeedVector[0]);
    fluidMovement[1] = velocityState.East - (indicatedAirspeedState * airspeedVector[1]);

    // calculate the movement vector we need to fly to reach velocityDesired -
    // taking fluidMovement into account
    courseComponent[0] = velocityDesired.North - fluidMovement[0];
    courseComponent[1] = velocityDesired.East - fluidMovement[1];

    indicatedAirspeedDesired = boundf(sqrtf(courseComponent[0] * courseComponent[0] + courseComponent[1] * courseComponent[1]),
                                      fixedWingPathFollowerSettings.HorizontalVelMin,
                                      fixedWingPathFollowerSettings.HorizontalVelMax);

    // if we could fly at arbitrary speeds, we'd just have to move towards the
    // courseComponent vector as previously calculated and we'd be fine
    // unfortunately however we are bound by min and max air speed limits, so
    // we need to recalculate the correct course to meet at least the
    // velocityDesired vector direction at our current speed
    // this overwrites courseComponent
    bool valid = correctCourse(courseComponent, (float *)&velocityDesired.North, fluidMovement, indicatedAirspeedDesired);

    // Error condition: wind speed too high, we can't go where we want anymore
    fixedWingPathFollowerStatus.Errors.Wind = 0;
    if ((!valid) &&
        fixedWingPathFollowerSettings.Safetymargins.Wind > 0.5f) { // alarm switched on
        fixedWingPathFollowerStatus.Errors.Wind = 1;
        result = 0;
    }

    // Airspeed error
    airspeedError = indicatedAirspeedDesired - indicatedAirspeedState;

    // Vertical speed error
    descentspeedDesired = boundf(
        velocityDesired.Down,
        -fixedWingPathFollowerSettings.VerticalVelMax,
        fixedWingPathFollowerSettings.VerticalVelMax);
    descentspeedError = descentspeedDesired - velocityState.Down;

    // Error condition: plane too slow or too fast
    fixedWingPathFollowerStatus.Errors.Highspeed = 0;
    fixedWingPathFollowerStatus.Errors.Lowspeed  = 0;
    if (indicatedAirspeedState > systemSettings.AirSpeedMax * fixedWingPathFollowerSettings.Safetymargins.Overspeed) {
        fixedWingPathFollowerStatus.Errors.Overspeed = 1;
        result = 0;
    }
    if (indicatedAirspeedState > fixedWingPathFollowerSettings.HorizontalVelMax * fixedWingPathFollowerSettings.Safetymargins.Highspeed) {
        fixedWingPathFollowerStatus.Errors.Highspeed = 1;
        result = 0;
    }
    if (indicatedAirspeedState < fixedWingPathFollowerSettings.HorizontalVelMin * fixedWingPathFollowerSettings.Safetymargins.Lowspeed) {
        fixedWingPathFollowerStatus.Errors.Lowspeed = 1;
        result = 0;
    }
    if (indicatedAirspeedState < systemSettings.AirSpeedMin * fixedWingPathFollowerSettings.Safetymargins.Stallspeed) {
        fixedWingPathFollowerStatus.Errors.Stallspeed = 1;
        result = 0;
    }

    /**
     * Compute desired thrust command
     */

    // Compute the cross feed from vertical speed to pitch, with saturation
    float speedErrorToPowerCommandComponent = boundf(
        (airspeedError / fixedWingPathFollowerSettings.HorizontalVelMin) * fixedWingPathFollowerSettings.AirspeedToPowerCrossFeed.Kp,
        -fixedWingPathFollowerSettings.AirspeedToPowerCrossFeed.Max,
        fixedWingPathFollowerSettings.AirspeedToPowerCrossFeed.Max
        );

    // Compute final thrust response
    powerCommand = pid_apply(&global.PIDpower, -descentspeedError, dT) +
                   speedErrorToPowerCommandComponent;

    // Output internal state to telemetry
    fixedWingPathFollowerStatus.Error.Power    = descentspeedError;
    fixedWingPathFollowerStatus.ErrorInt.Power = global.PIDpower.iAccumulator;
    fixedWingPathFollowerStatus.Command.Power  = powerCommand;

    // set thrust
    stabDesired.Thrust = boundf(fixedWingPathFollowerSettings.ThrustLimit.Neutral + powerCommand,
                                fixedWingPathFollowerSettings.ThrustLimit.Min,
                                fixedWingPathFollowerSettings.ThrustLimit.Max);

    // Error condition: plane cannot hold altitude at current speed.
    fixedWingPathFollowerStatus.Errors.Lowpower = 0;
    if (fixedWingPathFollowerSettings.ThrustLimit.Neutral + powerCommand >= fixedWingPathFollowerSettings.ThrustLimit.Max && // thrust at maximum
        velocityState.Down > 0.0f && // we ARE going down
        descentspeedDesired < 0.0f && // we WANT to go up
        airspeedError > 0.0f && // we are too slow already
        fixedWingPathFollowerSettings.Safetymargins.Lowpower > 0.5f) { // alarm switched on
        fixedWingPathFollowerStatus.Errors.Lowpower = 1;
        result = 0;
    }
    // Error condition: plane keeps climbing despite minimum thrust (opposite of above)
    fixedWingPathFollowerStatus.Errors.Highpower = 0;
    if (fixedWingPathFollowerSettings.ThrustLimit.Neutral + powerCommand <= fixedWingPathFollowerSettings.ThrustLimit.Min && // thrust at minimum
        velocityState.Down < 0.0f && // we ARE going up
        descentspeedDesired > 0.0f && // we WANT to go down
        airspeedError < 0.0f && // we are too fast already
        fixedWingPathFollowerSettings.Safetymargins.Highpower > 0.5f) { // alarm switched on
        fixedWingPathFollowerStatus.Errors.Highpower = 1;
        result = 0;
    }

    /**
     * Compute desired pitch command
     */
    // Compute the cross feed from vertical speed to pitch, with saturation
    float verticalSpeedToPitchCommandComponent = boundf(-descentspeedError * fixedWingPathFollowerSettings.VerticalToPitchCrossFeed.Kp,
                                                        -fixedWingPathFollowerSettings.VerticalToPitchCrossFeed.Max,
                                                        fixedWingPathFollowerSettings.VerticalToPitchCrossFeed.Max
                                                        );

    // Compute the pitch command as err*Kp + errInt*Ki + X_feed.
    pitchCommand = -pid_apply(&global.PIDspeed, airspeedError, dT) + verticalSpeedToPitchCommandComponent;

    fixedWingPathFollowerStatus.Error.Speed    = airspeedError;
    fixedWingPathFollowerStatus.ErrorInt.Speed = global.PIDspeed.iAccumulator;
    fixedWingPathFollowerStatus.Command.Speed  = pitchCommand;

    stabDesired.Pitch = boundf(fixedWingPathFollowerSettings.PitchLimit.Neutral + pitchCommand,
                               fixedWingPathFollowerSettings.PitchLimit.Min,
                               fixedWingPathFollowerSettings.PitchLimit.Max);

    // Error condition: high speed dive
    fixedWingPathFollowerStatus.Errors.Pitchcontrol = 0;
    if (fixedWingPathFollowerSettings.PitchLimit.Neutral + pitchCommand >= fixedWingPathFollowerSettings.PitchLimit.Max && // pitch demand is full up
        velocityState.Down > 0.0f && // we ARE going down
        descentspeedDesired < 0.0f && // we WANT to go up
        airspeedError < 0.0f && // we are too fast already
        fixedWingPathFollowerSettings.Safetymargins.Pitchcontrol > 0.5f) { // alarm switched on
        fixedWingPathFollowerStatus.Errors.Pitchcontrol = 1;
        result = 0;
    }

    /**
     * Compute desired roll command
     */
    courseError = RAD2DEG(atan2f(courseComponent[1], courseComponent[0])) - attitudeState.Yaw;

    if (courseError < -180.0f) {
        courseError += 360.0f;
    }
    if (courseError > 180.0f) {
        courseError -= 360.0f;
    }

    // overlap calculation. Theres a dead zone behind the craft where the
    // counter-yawing of some craft while rolling could render a desired right
    // turn into a desired left turn. Making the turn direction based on
    // current roll angle keeps the plane committed to a direction once chosen
    if (courseError < -180.0f + (fixedWingPathFollowerSettings.ReverseCourseOverlap * 0.5f)
        && attitudeState.Roll > 0.0f) {
        courseError += 360.0f;
    }
    if (courseError > 180.0f - (fixedWingPathFollowerSettings.ReverseCourseOverlap * 0.5f)
        && attitudeState.Roll < 0.0f) {
        courseError -= 360.0f;
    }

    courseCommand = pid_apply(&global.PIDcourse, courseError, dT);

    fixedWingPathFollowerStatus.Error.Course    = courseError;
    fixedWingPathFollowerStatus.ErrorInt.Course = global.PIDcourse.iAccumulator;
    fixedWingPathFollowerStatus.Command.Course  = courseCommand;

    stabDesired.Roll = boundf(fixedWingPathFollowerSettings.RollLimit.Neutral +
                              courseCommand,
                              fixedWingPathFollowerSettings.RollLimit.Min,
                              fixedWingPathFollowerSettings.RollLimit.Max);

    // TODO: find a check to determine loss of directional control. Likely needs some check of derivative


    /**
     * Compute desired yaw command
     */
    // TODO implement raw control mode for yaw and base on Accels.Y
    stabDesired.Yaw = 0.0f;


    stabDesired.StabilizationMode.Roll   = STABILIZATIONDESIRED_STABILIZATIONMODE_ATTITUDE;
    stabDesired.StabilizationMode.Pitch  = STABILIZATIONDESIRED_STABILIZATIONMODE_ATTITUDE;
    stabDesired.StabilizationMode.Yaw    = STABILIZATIONDESIRED_STABILIZATIONMODE_MANUAL;
    stabDesired.StabilizationMode.Thrust = STABILIZATIONDESIRED_STABILIZATIONMODE_MANUAL;

    StabilizationDesiredSet(&stabDesired);

    FixedWingPathFollowerStatusSet(&fixedWingPathFollowerStatus);

    return result;
}


/**
 * Function to calculate course vector C based on airspeed s, fluid movement F
 * and desired movement vector V
 * parameters in: V,F,s
 * parameters out: C
 * returns true if a valid solution could be found for V,F,s, false if not
 * C will be set to a best effort attempt either way
 */
static bool correctCourse(float *C, float *V, float *F, float s)
{
    // Approach:
    // Let Sc be a circle around origin marking possible movement vectors
    // of the craft with airspeed s (all possible options for C)
    // Let Vl be a line through the origin along movement vector V where fr any
    // point v on line Vl v = k * (V / |V|) = k' * V
    // Let Wl be a line parallel to Vl where for any point v on line Vl exists
    // a point w on WL with w = v - F
    // Then any intersection between circle Sc and line Wl represents course
    // vector which would result in a movement vector
    // V' = k * ( V / |V|) = k' * V
    // If there is no intersection point, S is insufficient to compensate
    // for F and we can only try to fly in direction of V (thus having wind drift
    // but at least making progress orthogonal to wind)

    s = fabsf(s);
    float f = vector_lengthf(F, 2);

    // normalize Cn=V/|V|, |V| must be >0
    float v = vector_lengthf(V, 2);
    if (v < 1e-6f) {
        // if |V|=0, we aren't supposed to move, turn into the wind
        // (this allows hovering)
        C[0] = -F[0];
        C[1] = -F[1];
        // if desired airspeed matches fluidmovement a hover is actually
        // intended so return true
        return fabsf(f - s) < 1e-3f;
    }
    float Vn[2] = { V[0] / v, V[1] / v };

    // project F on V
    float fp    = F[0] * Vn[0] + F[1] * Vn[1];

    // find component Fo of F that is orthogonal to V
    // (which is exactly the distance between Vl and Wl)
    float Fo[2] = { F[0] - (fp * Vn[0]), F[1] - (fp * Vn[1]) };
    float fo2   = Fo[0] * Fo[0] + Fo[1] * Fo[1];

    // find k where k * Vn = C - Fo
    // |C|=s is the hypothenuse in any rectangular triangle formed by k * Vn and Fo
    // so k^2 + fo^2 = s^2 (since |Vn|=1)
    float k2 = s * s - fo2;
    if (k2 <= -1e-3f) {
        // there is no solution, we will be drifted off either way
        // fallback: fly stupidly in direction of V and hope for the best
        C[0] = V[0];
        C[1] = V[1];
        return false;
    } else if (k2 <= 1e-3f) {
        // there is exactly one solution: -Fo
        C[0] = -Fo[0];
        C[1] = -Fo[1];
        return true;
    }
    // we have two possible solutions k positive and k negative as there are
    // two intersection points between Wl and Sc
    // which one is better? two criteria:
    // 1. we MUST move in the right direction, if any k leads to -v its invalid
    // 2. we should minimize the speed error
    float k     = sqrt(k2);
    float C1[2] = { -k * Vn[0] - Fo[0], -k * Vn[1] - Fo[1] };
    float C2[2] = { k *Vn[0] - Fo[0], k * Vn[1] - Fo[1] };
    // project C+F on Vn to find signed resulting movement vector length
    float vp1   = (C1[0] + F[0]) * Vn[0] + (C1[1] + F[1]) * Vn[1];
    float vp2   = (C2[0] + F[0]) * Vn[0] + (C2[1] + F[1]) * Vn[1];
    if (vp1 >= 0.0f && fabsf(v - vp1) < fabsf(v - vp2)) {
        // in this case the angle between course and resulting movement vector
        // is greater than 90 degrees - so we actually fly backwards
        C[0] = C1[0];
        C[1] = C1[1];
        return true;
    }
    C[0] = C2[0];
    C[1] = C2[1];
    if (vp2 >= 0.0f) {
        // in this case the angle between course and movement vector is less than
        // 90 degrees, but we do move in the right direction
        return true;
    } else {
        // in this case we actually get driven in the opposite direction of V
        // with both solutions for C
        // this might be reached in headwind stronger than maximum allowed
        // airspeed.
        return false;
    }
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

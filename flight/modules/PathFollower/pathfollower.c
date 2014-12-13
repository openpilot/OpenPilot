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
#include <adjustments.h>
#include <pathsummary.h>


// Private constants

#define CALLBACK_PRIORITY      CALLBACK_PRIORITY_LOW
#define CBTASK_PRIORITY        CALLBACK_TASK_FLIGHTCONTROL

#define PF_IDLE_UPDATE_RATE_MS 100

#define STACK_SIZE_BYTES       2048

#define DEADBAND_HIGH          0.10f
#define DEADBAND_LOW           -0.10f
// Private types

struct Globals {
    struct pid PIDposH[2];
    struct pid PIDposV;
    struct pid PIDvel[3];
    struct pid BrakePIDvel[3];
    struct pid PIDcourse;
    struct pid PIDspeed;
    struct pid PIDpower;
    float poiRadius;
    float vtolEmergencyFallback;
    bool  vtolEmergencyFallbackSwitch;
};

struct NeutralThrustEstimation {
  uint32_t count;
  float sum;
  float average;
  float correction;
  float algo_erro_check;
  bool start_sampling;
  bool have_correction;
};
static struct NeutralThrustEstimation neutralThrustEst;


// Private variables
static DelayedCallbackInfo *pathFollowerCBInfo;
static uint32_t updatePeriod = PF_IDLE_UPDATE_RATE_MS;
static struct Globals global;
static PathStatusData pathStatus;
static PathDesiredData pathDesired;
static FixedWingPathFollowerSettingsData fixedWingPathFollowerSettings;
static VtolPathFollowerSettingsData vtolPathFollowerSettings;
static FlightStatusData flightStatus;
static PathSummaryData pathSummary;

// correct speed by measured airspeed
static float indicatedAirspeedStateBias = 0.0f;


// Private functions
static void pathFollowerTask(void);
static void resetGlobals();
static void SettingsUpdatedCb(UAVObjEvent *ev);
static uint8_t updateAutoPilotByFrameType();
static uint8_t updateAutoPilotFixedWing();
static uint8_t updateAutoPilotVtol();
static float updateTailInBearing();
static float updateCourseBearing();
static float updatePathBearing();
static float updatePOIBearing();
static void processPOI();
static void updateBrakeVelocity(float startingVelocity, float dT, float brakeRate, float currentVelocity, float *updatedVelocity);
static void updatePathVelocity(float kFF, bool limited);
static uint8_t updateFixedDesiredAttitude();
static int8_t updateVtolDesiredAttitude(bool yaw_attitude, float yaw_direction);
static void updateFixedAttitude();
static void updateVtolDesiredAttitudeEmergencyFallback();
static void airspeedStateUpdatedCb(UAVObjEvent *ev);
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
    AdjustmentsInitialize();


    // reset integrals
    resetGlobals();

    // Create object queue
    pathFollowerCBInfo = PIOS_CALLBACKSCHEDULER_Create(&pathFollowerTask, CALLBACK_PRIORITY, CBTASK_PRIORITY, CALLBACKINFO_RUNNING_PATHFOLLOWER, STACK_SIZE_BYTES);
    FixedWingPathFollowerSettingsConnectCallback(&SettingsUpdatedCb);
    VtolPathFollowerSettingsConnectCallback(&SettingsUpdatedCb);
    PathDesiredConnectCallback(SettingsUpdatedCb);
    AirspeedStateConnectCallback(airspeedStateUpdatedCb);

    return 0;
}
MODULE_INITCALL(PathFollowerInitialize, PathFollowerStart);


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

    if (flightStatus.FlightMode == FLIGHTSTATUS_FLIGHTMODE_POI) { // TODO Hack from vtolpathfollower, move into manualcontrol!
        processPOI();
    }

    int16_t old_uid = pathStatus.UID;
    pathStatus.UID    = pathDesired.UID;
    pathStatus.Status = PATHSTATUS_STATUS_INPROGRESS;
    if (pathDesired.Timeout > 0.0f) {
	if (old_uid != pathStatus.UID) {
	    pathStatus.path_time = 0.0f;
	}
	else {
	    pathStatus.path_time += updatePeriod / 1000.0f;
	}
    }

    switch (pathDesired.Mode) {
    case PATHDESIRED_MODE_FLYENDPOINT:
    case PATHDESIRED_MODE_FLYVECTOR:
    case PATHDESIRED_MODE_BRAKE:
    case PATHDESIRED_MODE_FLYCIRCLERIGHT:
    case PATHDESIRED_MODE_FLYCIRCLELEFT:
    {
        uint8_t result = updateAutoPilotByFrameType();
        if (result) {
            AlarmsSet(SYSTEMALARMS_ALARM_GUIDANCE, SYSTEMALARMS_ALARM_OK);
        } else {
            pathStatus.Status = PATHSTATUS_STATUS_CRITICAL;
            AlarmsSet(SYSTEMALARMS_ALARM_GUIDANCE, SYSTEMALARMS_ALARM_WARNING);
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


static void SettingsUpdatedCb(__attribute__((unused)) UAVObjEvent *ev)
{
    FixedWingPathFollowerSettingsGet(&fixedWingPathFollowerSettings);

    pid_configure(&global.PIDcourse, fixedWingPathFollowerSettings.CoursePI.Kp, fixedWingPathFollowerSettings.CoursePI.Ki, 0.0f, fixedWingPathFollowerSettings.CoursePI.ILimit);
    pid_configure(&global.PIDspeed, fixedWingPathFollowerSettings.SpeedPI.Kp, fixedWingPathFollowerSettings.SpeedPI.Ki, 0.0f, fixedWingPathFollowerSettings.SpeedPI.ILimit);
    pid_configure(&global.PIDpower, fixedWingPathFollowerSettings.PowerPI.Kp, fixedWingPathFollowerSettings.PowerPI.Ki, 0.0f, fixedWingPathFollowerSettings.PowerPI.ILimit);


    VtolPathFollowerSettingsGet(&vtolPathFollowerSettings);

    pid_configure(&global.PIDvel[0], vtolPathFollowerSettings.HorizontalVelPID.Kp, vtolPathFollowerSettings.HorizontalVelPID.Ki, vtolPathFollowerSettings.HorizontalVelPID.Kd, vtolPathFollowerSettings.HorizontalVelPID.ILimit);
    pid_configure(&global.PIDvel[1], vtolPathFollowerSettings.HorizontalVelPID.Kp, vtolPathFollowerSettings.HorizontalVelPID.Ki, vtolPathFollowerSettings.HorizontalVelPID.Kd, vtolPathFollowerSettings.HorizontalVelPID.ILimit);
    pid_configure(&global.PIDvel[2], vtolPathFollowerSettings.VerticalVelPID.Kp, vtolPathFollowerSettings.VerticalVelPID.Ki, vtolPathFollowerSettings.VerticalVelPID.Kd, vtolPathFollowerSettings.VerticalVelPID.ILimit);

    pid_configure(&global.BrakePIDvel[0], vtolPathFollowerSettings.BrakeHorizontalVelPID.Kp, vtolPathFollowerSettings.BrakeHorizontalVelPID.Ki, vtolPathFollowerSettings.BrakeHorizontalVelPID.Kd, vtolPathFollowerSettings.BrakeHorizontalVelPID.ILimit);
    pid_configure(&global.BrakePIDvel[1], vtolPathFollowerSettings.BrakeHorizontalVelPID.Kp, vtolPathFollowerSettings.BrakeHorizontalVelPID.Ki, vtolPathFollowerSettings.BrakeHorizontalVelPID.Kd, vtolPathFollowerSettings.BrakeHorizontalVelPID.ILimit);
    pid_configure(&global.BrakePIDvel[2], vtolPathFollowerSettings.BrakeVerticalVelPID.Kp, vtolPathFollowerSettings.BrakeVerticalVelPID.Ki, vtolPathFollowerSettings.BrakeVerticalVelPID.Kd, vtolPathFollowerSettings.BrakeVerticalVelPID.ILimit);

    PathDesiredGet(&pathDesired);
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
    pid_zero(&global.PIDvel[0]);
    pid_zero(&global.PIDvel[1]);
    pid_zero(&global.PIDvel[2]);
    pid_zero(&global.BrakePIDvel[0]);
    pid_zero(&global.BrakePIDvel[1]);
    pid_zero(&global.BrakePIDvel[2]);
    pid_zero(&global.PIDcourse);
    pid_zero(&global.PIDspeed);
    pid_zero(&global.PIDpower);
    global.poiRadius = 0.0f;
    global.vtolEmergencyFallback = 0;
    global.vtolEmergencyFallbackSwitch = false;

    // reset neutral thrust assessment. We restart this process
    // and do once for each position hold engagement
    neutralThrustEst.start_sampling = false;
    neutralThrustEst.count = 0;
    neutralThrustEst.sum = 0.0f;
    neutralThrustEst.have_correction = false;
    neutralThrustEst.average = 0.0f;
    neutralThrustEst.correction = 0.0f;

    pathStatus.path_time = 0.0f;
}

static uint8_t updateAutoPilotByFrameType()
{
    FrameType_t frameType = GetCurrentFrameType();

    if (frameType == FRAME_TYPE_CUSTOM || frameType == FRAME_TYPE_GROUND) {
        switch (vtolPathFollowerSettings.TreatCustomCraftAs) {
        case VTOLPATHFOLLOWERSETTINGS_TREATCUSTOMCRAFTAS_FIXEDWING:
            frameType = FRAME_TYPE_FIXED_WING;
            break;
        case VTOLPATHFOLLOWERSETTINGS_TREATCUSTOMCRAFTAS_VTOL:
            frameType = FRAME_TYPE_MULTIROTOR;
            break;
        }
    }
    switch (frameType) {
    case FRAME_TYPE_MULTIROTOR:
    case FRAME_TYPE_HELI:
        updatePeriod = vtolPathFollowerSettings.UpdatePeriod;
        return updateAutoPilotVtol();

        break;
    case FRAME_TYPE_FIXED_WING:
    default:
        updatePeriod = fixedWingPathFollowerSettings.UpdatePeriod;
        return updateAutoPilotFixedWing();

        break;
    }
}

/**
 * fixed wing autopilot:
 * straight forward:
 * 1. update path velocity for limited motion crafts
 * 2. update attitude according to default fixed wing pathfollower algorithm
 */
static uint8_t updateAutoPilotFixedWing()
{
    pid_configure(&global.PIDposH[0], fixedWingPathFollowerSettings.HorizontalPosP, 0.0f, 0.0f, 0.0f);
    pid_configure(&global.PIDposH[1], fixedWingPathFollowerSettings.HorizontalPosP, 0.0f, 0.0f, 0.0f);
    pid_configure(&global.PIDposV, fixedWingPathFollowerSettings.VerticalPosP, 0.0f, 0.0f, 0.0f);
    updatePathVelocity(fixedWingPathFollowerSettings.CourseFeedForward, true);
    return updateFixedDesiredAttitude();
}

/**
 * vtol autopilot
 * use hover capable algorithm with unlimeted movement calculation. if that fails (flyaway situation due to compass failure)
 * fall back to emergency fallback autopilot to keep minimum amount of flight control
 */
static uint8_t updateAutoPilotVtol()
{
    enum { RETURN_0 = 0, RETURN_1 = 1, RETURN_RESULT } returnmode;
    enum { FOLLOWER_REGULAR, FOLLOWER_FALLBACK } followermode;
    uint8_t result = 0;

    // decide on behaviour based on settings and system state
    if (global.vtolEmergencyFallbackSwitch) {
        returnmode   = RETURN_0;
        followermode = FOLLOWER_FALLBACK;
    } else {
        if (vtolPathFollowerSettings.FlyawayEmergencyFallback == VTOLPATHFOLLOWERSETTINGS_FLYAWAYEMERGENCYFALLBACK_ALWAYS) {
            returnmode   = RETURN_1;
            followermode = FOLLOWER_FALLBACK;
        } else {
            returnmode   = RETURN_RESULT;
            followermode = FOLLOWER_REGULAR;
        }
    }

    // vertical positon control PID loops works the same in both regular and fallback modes, setup according to settings
    pid_configure(&global.PIDposV, vtolPathFollowerSettings.VerticalPosP, 0.0f, 0.0f, 0.0f);

    switch (followermode) {
    case FOLLOWER_REGULAR:
    {
        // horizontal position control PID loop works according to settings in regular mode, allowing integral terms
        pid_configure(&global.PIDposH[0], vtolPathFollowerSettings.HorizontalPosP, 0.0f, 0.0f, 0.0f);
        pid_configure(&global.PIDposH[1], vtolPathFollowerSettings.HorizontalPosP, 0.0f, 0.0f, 0.0f);
        updatePathVelocity(vtolPathFollowerSettings.CourseFeedForward, false);

        // yaw behaviour is configurable in vtolpathfollower, select yaw control algorithm
        bool yaw_attitude = true;
        float yaw = 0.0f;

        if (pathDesired.Mode == PATHDESIRED_MODE_BRAKE) {
	      yaw_attitude = false;
        }
        else {
	  switch (vtolPathFollowerSettings.YawControl) {
	  case VTOLPATHFOLLOWERSETTINGS_YAWCONTROL_MANUAL:
	      yaw_attitude = false;
	      break;
	  case VTOLPATHFOLLOWERSETTINGS_YAWCONTROL_TAILIN:
	      yaw = updateTailInBearing();
	      break;
	  case VTOLPATHFOLLOWERSETTINGS_YAWCONTROL_MOVEMENTDIRECTION:
	      yaw = updateCourseBearing();
	      break;
	  case VTOLPATHFOLLOWERSETTINGS_YAWCONTROL_PATHDIRECTION:
	      yaw = updatePathBearing();
	      break;
	  case VTOLPATHFOLLOWERSETTINGS_YAWCONTROL_POI:
	      yaw = updatePOIBearing();
	      break;
	  }
        }
        result = updateVtolDesiredAttitude(yaw_attitude, yaw);

        if (!result) {
            if (pathDesired.Mode == PATHDESIRED_MODE_BRAKE) {
        	plan_setup_assistedcontrol(true); // revert braking to position hold, user can always stick override
            }
            else if (vtolPathFollowerSettings.FlyawayEmergencyFallback != VTOLPATHFOLLOWERSETTINGS_FLYAWAYEMERGENCYFALLBACK_DISABLED) {
                // switch to emergency follower if follower indicates problems
                global.vtolEmergencyFallbackSwitch = true;
            }
        }
    }
    break;
    case FOLLOWER_FALLBACK:
    {
        // fallback loop only cares about intended horizontal flight direction, simplify control behaviour accordingly
        pid_configure(&global.PIDposH[0], 1.0f, 0.0f, 0.0f, 0.0f);
        pid_configure(&global.PIDposH[1], 1.0f, 0.0f, 0.0f, 0.0f);
        updatePathVelocity(vtolPathFollowerSettings.CourseFeedForward, true);

        // emergency follower has no return value
        updateVtolDesiredAttitudeEmergencyFallback();
    }
    break;
    }


    // Brake mode end condition checks
    // todo capture enter-hold reason code
    if (pathDesired.Mode == PATHDESIRED_MODE_BRAKE) {

	bool exit_brake = false;
        if (pathStatus.path_time > pathDesired.Timeout) { // enter hold on timeout
            pathSummary.brake_exit_reason = PATHSUMMARY_BRAKE_EXIT_REASON_TIMEOUT;
            exit_brake = true;
        }
        else if (pathStatus.fractional_progress > 0.9f) {   // enter hold if achieved 90% reduction in start velocity
            pathSummary.brake_exit_reason = PATHSUMMARY_BRAKE_EXIT_REASON_PATHCOMPLETED;
            exit_brake = true;
        }

        if (exit_brake) {
            // Calculate the distance error between the originally desired
            // stopping point and the actual brake-exit point.

            PositionStateData p;
            PositionStateGet(&p);
            float north_offset = pathDesired.End.North - p.North;
            float east_offset = pathDesired.End.East - p.East;
            float down_offset = pathDesired.End.Down - p.Down;
            pathSummary.brake_distance_offset = sqrtf(north_offset * north_offset + east_offset*east_offset + down_offset*down_offset);
            pathSummary.time_remaining = pathDesired.Timeout - pathStatus.path_time;
            pathSummary.fractional_progress = pathStatus.fractional_progress;
            VelocityStateData velocityState;
            VelocityStateGet(&velocityState);
            float cur_velocity = velocityState.North * velocityState.North + velocityState.East * velocityState.East + velocityState.Down * velocityState.Down;
            cur_velocity = sqrtf(cur_velocity);
            pathSummary.decelrate = (pathDesired.StartingVelocity - cur_velocity) / pathStatus.path_time;
            float brakeRate;
            FlightModeSettingsAssistedControlBrakeRateGet(&brakeRate);
            pathSummary.brakeRateActualDesiredRatio = pathSummary.decelrate / brakeRate;
            pathSummary.velocityIntoHold = cur_velocity;
            pathSummary.UID = pathStatus.UID;
            PathSummarySet(&pathSummary);

            plan_setup_assistedcontrol(true); // braking timeout true
        }
    }

    switch (returnmode) {
    case RETURN_RESULT:
        return result;

    default:
        // returns either 0 or 1 according to enum definition above
        return returnmode;
    }
}


/**
 * Compute bearing of current takeoff location
 */
static float updateTailInBearing()
{
    PositionStateData p;

    PositionStateGet(&p);
    TakeOffLocationData t;
    TakeOffLocationGet(&t);
    // atan2f always returns in between + and - 180 degrees
    return RAD2DEG(atan2f(p.East - t.East, p.North - t.North));
}

/**
 * Compute bearing of current movement direction
 */
static float updateCourseBearing()
{
    VelocityStateData v;

    VelocityStateGet(&v);
    // atan2f always returns in between + and - 180 degrees
    return RAD2DEG(atan2f(v.East, v.North));
}

/**
 * Compute bearing of current path direction
 */
static float updatePathBearing()
{
    PositionStateData positionState;

    PositionStateGet(&positionState);

    float cur[3] = { positionState.North,
                     positionState.East,
                     positionState.Down };
    struct path_status progress;

    path_progress(&pathDesired, cur, &progress);

    // atan2f always returns in between + and - 180 degrees
    return RAD2DEG(atan2f(progress.path_vector[1], progress.path_vector[0]));
}

/**
 * Compute bearing between current position and POI
 */
static float updatePOIBearing()
{
    PoiLocationData poi;

    PoiLocationGet(&poi);
    PositionStateData positionState;
    PositionStateGet(&positionState);

    const float dT = updatePeriod / 1000.0f;
    float dLoc[3];
    float yaw = 0;
    /*float elevation = 0;*/

    dLoc[0] = positionState.North - poi.North;
    dLoc[1] = positionState.East - poi.East;
    dLoc[2] = positionState.Down - poi.Down;

    if (dLoc[1] < 0) {
        yaw = RAD2DEG(atan2f(dLoc[1], dLoc[0])) + 180.0f;
    } else {
        yaw = RAD2DEG(atan2f(dLoc[1], dLoc[0])) - 180.0f;
    }
    ManualControlCommandData manualControlData;
    ManualControlCommandGet(&manualControlData);

    float pathAngle = 0;
    if (manualControlData.Roll > DEADBAND_HIGH) {
        pathAngle = -(manualControlData.Roll - DEADBAND_HIGH) * dT * 300.0f;
    } else if (manualControlData.Roll < DEADBAND_LOW) {
        pathAngle = -(manualControlData.Roll - DEADBAND_LOW) * dT * 300.0f;
    }

    return yaw + (pathAngle / 2.0f);
}

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

static void updateBrakeVelocity(float startingVelocity, float dT, float brakeRate, float currentVelocity, float *updatedVelocity)
{
    if (startingVelocity >= 0.0f) {
	*updatedVelocity = startingVelocity - dT*brakeRate;
	if (*updatedVelocity > currentVelocity) {
	    *updatedVelocity = currentVelocity;
	}
	if (*updatedVelocity < 0.0f) {
	    *updatedVelocity = 0.0f;
	}
    }
    else {
	*updatedVelocity = startingVelocity + dT*brakeRate;
	if (*updatedVelocity < currentVelocity) {
	    *updatedVelocity = currentVelocity;
	}
	if (*updatedVelocity > 0.0f) {
	    *updatedVelocity = 0.0f;
	}
    }
    return;
}

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

    if (pathDesired.Mode == PATHDESIRED_MODE_BRAKE) {
        float brakeRate;
        FlightModeSettingsAssistedControlBrakeRateGet(&brakeRate);
        if (brakeRate < 0.2f) brakeRate = 0.2f;  // set a minimum to avoid a divide by zero potential below
        updateBrakeVelocity(pathDesired.StartingVelocityVector.North, pathStatus.path_time, brakeRate, velocityState.North, &velocityDesired.North);
        updateBrakeVelocity(pathDesired.StartingVelocityVector.East,  pathStatus.path_time, brakeRate, velocityState.East, &velocityDesired.East);
        updateBrakeVelocity(pathDesired.StartingVelocityVector.Down,  pathStatus.path_time, brakeRate, velocityState.Down, &velocityDesired.Down);

        float cur_velocity = velocityState.North * velocityState.North + velocityState.East * velocityState.East + velocityState.Down * velocityState.Down;
        cur_velocity = sqrtf(cur_velocity);
        float desired_velocity = velocityDesired.North * velocityDesired.North + velocityDesired.East * velocityDesired.East + velocityDesired.Down * velocityDesired.Down;
        desired_velocity = sqrtf(desired_velocity);

	// update pathstatus
	pathStatus.error      = cur_velocity - desired_velocity;
	pathStatus.fractional_progress  = 1.0f;
	if (pathDesired.StartingVelocity > 0.0f) {
	    pathStatus.fractional_progress  = ( pathDesired.StartingVelocity - cur_velocity) / pathDesired.StartingVelocity;

	    //sometimes current velocity can exceed starting velocity due to initial acceleration
	    if (pathStatus.fractional_progress < 0.0f) pathStatus.fractional_progress = 0.0f;
	}
	pathStatus.path_direction_north = velocityDesired.North;
	pathStatus.path_direction_east  = velocityDesired.East;
	pathStatus.path_direction_down  = velocityDesired.Down;

	pathStatus.correction_direction_north = velocityDesired.North - velocityState.North;
	pathStatus.correction_direction_east  = velocityDesired.East - velocityState.East;
	pathStatus.correction_direction_down  = velocityDesired.Down - velocityState.Down;
    }
    else {
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
    }


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
 * Compute desired attitude from the desired velocity
 *
 * Takes in @ref NedState which has the acceleration in the
 * NED frame as the feedback term and then compares the
 * @ref VelocityState against the @ref VelocityDesired
 */
static int8_t updateVtolDesiredAttitude(bool yaw_attitude, float yaw_direction)
{
    const float dT = updatePeriod / 1000.0f;
    uint8_t result = 1;
    bool manual_thrust = false;

    VelocityDesiredData velocityDesired;
    VelocityStateData velocityState;
    StabilizationDesiredData stabDesired;
    AttitudeStateData attitudeState;
    StabilizationBankData stabSettings;
    SystemSettingsData systemSettings;
    AdjustmentsData adjustments;

    float northError;
    float northCommand;

    float eastError;
    float eastCommand;

    float downError;
    float downCommand;

    SystemSettingsGet(&systemSettings);
    VelocityStateGet(&velocityState);
    VelocityDesiredGet(&velocityDesired);
    StabilizationDesiredGet(&stabDesired);
    VelocityDesiredGet(&velocityDesired);
    AttitudeStateGet(&attitudeState);
    StabilizationBankGet(&stabSettings);
    AdjustmentsGet(&adjustments);


    if (pathDesired.Mode != PATHDESIRED_MODE_BRAKE) {
      // scale velocity if it is above configured maximum
      // for braking, we can not help it if initial velocity was greater
      float velH = sqrtf(velocityDesired.North * velocityDesired.North + velocityDesired.East * velocityDesired.East);
      if (velH > vtolPathFollowerSettings.HorizontalVelMax) {
	  velocityDesired.North *= vtolPathFollowerSettings.HorizontalVelMax / velH;
	  velocityDesired.East  *= vtolPathFollowerSettings.HorizontalVelMax / velH;
      }
      if (fabsf(velocityDesired.Down) > vtolPathFollowerSettings.VerticalVelMax) {
	  velocityDesired.Down *= vtolPathFollowerSettings.VerticalVelMax / fabsf(velocityDesired.Down);
      }
    }

    // calculate the velocity errors between desired and actual
    northError   = velocityDesired.North - velocityState.North;
    eastError    = velocityDesired.East - velocityState.East;
    downError    = velocityDesired.Down - velocityState.Down;

    // Must flip this sign
    downError    = -downError;

    // Compute desired commands
    if (pathDesired.Mode != PATHDESIRED_MODE_BRAKE) {
	northCommand = pid_apply(&global.PIDvel[0], northError, dT) + velocityDesired.North * vtolPathFollowerSettings.VelocityFeedforward;
	eastCommand  = pid_apply(&global.PIDvel[1], eastError, dT) + velocityDesired.East * vtolPathFollowerSettings.VelocityFeedforward;
	downCommand  = pid_apply(&global.PIDvel[2], downError, dT);
    }
    else {
	northCommand = pid_apply(&global.BrakePIDvel[0], northError, dT) + velocityDesired.North * vtolPathFollowerSettings.BrakeVelocityFeedforward;
	eastCommand  = pid_apply(&global.BrakePIDvel[1], eastError, dT) + velocityDesired.East * vtolPathFollowerSettings.BrakeVelocityFeedforward;
	downCommand  = pid_apply(&global.BrakePIDvel[2], downError, dT);
    }


    if ((vtolPathFollowerSettings.ThrustControl == VTOLPATHFOLLOWERSETTINGS_THRUSTCONTROL_MANUAL &&
	flightStatus.FlightModeAssist == FLIGHTSTATUS_FLIGHTMODEASSIST_NONE ) ||
	(flightStatus.FlightModeAssist && flightStatus.AssistedThrottleState == FLIGHTSTATUS_ASSISTEDTHROTTLESTATE_MANUAL) ) {
	manual_thrust = true;
    }

    if (!manual_thrust && neutralThrustEst.have_correction != true) {
	// reassess the correction value for this position hold period if not already done

	// Assess if position hold state running
	bool ph_active =
	    ( (flightStatus.FlightMode == FLIGHTSTATUS_FLIGHTMODE_POSITIONHOLD  &&
			      flightStatus.FlightModeAssist == FLIGHTSTATUS_FLIGHTMODEASSIST_NONE) ||
	    (flightStatus.FlightModeAssist != FLIGHTSTATUS_FLIGHTMODEASSIST_NONE  &&
			      flightStatus.AssistedControlState == FLIGHTSTATUS_ASSISTEDCONTROLSTATE_HOLD) );

	bool stable = (fabsf(velocityDesired.Down) < 0.2f && fabsf(velocityState.Down) < 0.3f && fabsf(downError) < 0.3f);

	if (ph_active && stable) {

	    if (neutralThrustEst.start_sampling) {
		neutralThrustEst.count++;
		neutralThrustEst.sum += downCommand;

		if (neutralThrustEst.count >= 60) {
		    // 3 seconds have past
		    // lets take an average
		    neutralThrustEst.average = neutralThrustEst.sum/60.0f;

		    // Let the size of the correction to something reasonable.  The largest
		    float checked_average = neutralThrustEst.average;
		    if (checked_average > 0.2f) checked_average = 0.2f;
		    else if (checked_average < -0.2f) checked_average = -0.2f;

		    // We always limit the correction to the size of the current i term
		    float iterm = global.PIDvel[2].iAccumulator/1000.0f;
		    if (fabsf(checked_average) > fabsf(iterm)) {
			neutralThrustEst.correction = iterm;
			global.PIDvel[2].iAccumulator = 0.0f;
		    }
		    else {
			neutralThrustEst.correction = checked_average;
			global.PIDvel[2].iAccumulator -= neutralThrustEst.correction * 1000.0f;

		    }
		    // recalculate downCommand now that i is adjusted
		    float new_downCommand  = pid_apply(&global.PIDvel[2], downError, 0.0f);  // we generally don't have a d controller
		    neutralThrustEst.algo_erro_check = (new_downCommand + neutralThrustEst.correction) - downCommand;
		    downCommand = new_downCommand;
		    neutralThrustEst.start_sampling = false;
		    neutralThrustEst.have_correction = true;

		    // Write a new adjustment value
		    adjustments.NeutralThrustOffset = neutralThrustEst.correction;
		    adjustments.NeutralThrustAlgoError = neutralThrustEst.algo_erro_check;
		    AdjustmentsNeutralThrustOffsetSet(&adjustments.NeutralThrustOffset);
		    AdjustmentsNeutralThrustAlgoErrorSet(&adjustments.NeutralThrustAlgoError);
		}
	    }
	    else {
		// start a tick count
		neutralThrustEst.start_sampling = true;
		neutralThrustEst.count = 0;
		neutralThrustEst.sum = 0.0f;
	    }
	}
	else {
	    // reset sampling as we did't get 3 continuous seconds
	    neutralThrustEst.start_sampling = false;
	}
    } //else we already have a correction for this PH run

    // Generally in braking the downError will be an increased altitude.  We really will rely on cruisecontrol to backoff.
    stabDesired.Thrust = boundf(adjustments.NeutralThrustOffset + downCommand + vtolPathFollowerSettings.ThrustLimits.Neutral, vtolPathFollowerSettings.ThrustLimits.Min, vtolPathFollowerSettings.ThrustLimits.Max);


    // DEBUG HACK: allow user to skew compass on purpose to see if emergency failsafe kicks in
    if (vtolPathFollowerSettings.FlyawayEmergencyFallback == VTOLPATHFOLLOWERSETTINGS_FLYAWAYEMERGENCYFALLBACK_DEBUGTEST) {
        attitudeState.Yaw += 120.0f;
        if (attitudeState.Yaw > 180.0f) {
            attitudeState.Yaw -= 360.0f;
        }
    }


    if ( // emergency flyaway detection
        ( // integral already at its limit
            vtolPathFollowerSettings.HorizontalVelPID.ILimit - fabsf(global.PIDvel[0].iAccumulator) < 1e-6f ||
            vtolPathFollowerSettings.HorizontalVelPID.ILimit - fabsf(global.PIDvel[1].iAccumulator) < 1e-6f
        ) &&
        // angle between desired and actual velocity >90 degrees (by dot product)
        (velocityDesired.North * velocityState.North + velocityDesired.East * velocityState.East < 0.0f) &&
        // quad is moving at significant speed (during flyaway it would keep speeding up)
        squaref(velocityState.North) + squaref(velocityState.East) > 1.0f
        ) {
        global.vtolEmergencyFallback += dT;
        if (global.vtolEmergencyFallback >= vtolPathFollowerSettings.FlyawayEmergencyFallbackTriggerTime) {
            // after emergency timeout, trigger alarm - everything else is handled by callers
            // (switch to emergency algorithm, switch to emergency waypoint in pathplanner, alarms, ...)
            result = 0;
        }
    } else {
        global.vtolEmergencyFallback = 0.0f;
    }

    // Project the north and east command signals into the pitch and roll based on yaw.  For this to behave well the
    // craft should move similarly for 5 deg roll versus 5 deg pitch

    float maxPitch =  vtolPathFollowerSettings.MaxRollPitch;
    if (pathDesired.Mode == PATHDESIRED_MODE_BRAKE) {
	maxPitch = vtolPathFollowerSettings.MaxBrakePitch;
    }

    stabDesired.Pitch = boundf(-northCommand * cosf(DEG2RAD(attitudeState.Yaw)) +
                               -eastCommand * sinf(DEG2RAD(attitudeState.Yaw)),
                               -maxPitch, maxPitch);
    stabDesired.Roll  = boundf(-northCommand * sinf(DEG2RAD(attitudeState.Yaw)) +
                               eastCommand * cosf(DEG2RAD(attitudeState.Yaw)),
                               -maxPitch, maxPitch);



    if (manual_thrust) {
	ManualControlCommandData manualControl;
	ManualControlCommandGet(&manualControl);
        stabDesired.Thrust = manualControl.Thrust;
    }

    stabDesired.StabilizationMode.Roll  = STABILIZATIONDESIRED_STABILIZATIONMODE_ATTITUDE;
    stabDesired.StabilizationMode.Pitch = STABILIZATIONDESIRED_STABILIZATIONMODE_ATTITUDE;
    if (yaw_attitude) {
        stabDesired.StabilizationMode.Yaw = STABILIZATIONDESIRED_STABILIZATIONMODE_ATTITUDE;
        stabDesired.Yaw = yaw_direction;
    } else {
        stabDesired.StabilizationMode.Yaw = STABILIZATIONDESIRED_STABILIZATIONMODE_AXISLOCK;
	ManualControlCommandData manualControl;
	ManualControlCommandGet(&manualControl);
        stabDesired.Yaw = stabSettings.MaximumRate.Yaw * manualControl.Yaw;
    }
    stabDesired.StabilizationMode.Thrust = STABILIZATIONDESIRED_STABILIZATIONMODE_CRUISECONTROL;
    StabilizationDesiredSet(&stabDesired);

    return result;
}

/**
 * Compute desired attitude for vtols - emergency fallback
 */
static void updateVtolDesiredAttitudeEmergencyFallback()
{
    const float dT = updatePeriod / 1000.0f;

    VelocityDesiredData velocityDesired;
    VelocityStateData velocityState;
    StabilizationDesiredData stabDesired;

    float courseError;
    float courseCommand;

    float downError;
    float downCommand;

    VelocityStateGet(&velocityState);
    VelocityDesiredGet(&velocityDesired);

    ManualControlCommandData manualControlData;
    ManualControlCommandGet(&manualControlData);

    courseError = RAD2DEG(atan2f(velocityDesired.East, velocityDesired.North) - atan2f(velocityState.East, velocityState.North));

    if (courseError < -180.0f) {
        courseError += 360.0f;
    }
    if (courseError > 180.0f) {
        courseError -= 360.0f;
    }


    courseCommand   = (courseError * vtolPathFollowerSettings.EmergencyFallbackYawRate.kP);

    stabDesired.Yaw = boundf(courseCommand, -vtolPathFollowerSettings.EmergencyFallbackYawRate.Max, vtolPathFollowerSettings.EmergencyFallbackYawRate.Max);

    // Compute desired down command
    downError   = velocityDesired.Down - velocityState.Down;
    // Must flip this sign
    downError   = -downError;
    downCommand = pid_apply(&global.PIDvel[2], downError, dT);

    stabDesired.Thrust = boundf(downCommand + vtolPathFollowerSettings.ThrustLimits.Neutral, vtolPathFollowerSettings.ThrustLimits.Min, vtolPathFollowerSettings.ThrustLimits.Max);


    stabDesired.Roll   = vtolPathFollowerSettings.EmergencyFallbackAttitude.Roll;
    stabDesired.Pitch  = vtolPathFollowerSettings.EmergencyFallbackAttitude.Pitch;

    if (vtolPathFollowerSettings.ThrustControl == VTOLPATHFOLLOWERSETTINGS_THRUSTCONTROL_MANUAL) {
        // For now override thrust with manual control.  Disable at your risk, quad goes to China.
        ManualControlCommandData manualControl;
        ManualControlCommandGet(&manualControl);
        stabDesired.Thrust = manualControl.Thrust;
    }

    stabDesired.StabilizationMode.Roll   = STABILIZATIONDESIRED_STABILIZATIONMODE_ATTITUDE;
    stabDesired.StabilizationMode.Pitch  = STABILIZATIONDESIRED_STABILIZATIONMODE_ATTITUDE;
    stabDesired.StabilizationMode.Yaw    = STABILIZATIONDESIRED_STABILIZATIONMODE_RATE;
    stabDesired.StabilizationMode.Thrust = STABILIZATIONDESIRED_STABILIZATIONMODE_CRUISECONTROL;
    StabilizationDesiredSet(&stabDesired);
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

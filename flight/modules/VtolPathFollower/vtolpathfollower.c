/**
 ******************************************************************************
 *
 * @file       vtolpathfollower.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2012.
 * @brief      This module compared @ref PositionState to @ref PathDesired
 * and sets @ref Stabilization.  It only does this when the FlightMode field
 * of @ref FlightStatus is PathPlanner or RTH.
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
 * Input object: FlightStatus
 * Input object: PathDesired
 * Input object: PositionState
 * Output object: StabilizationDesired
 *
 * This module will periodically update the value of the @ref StabilizationDesired object based on
 * @ref PathDesired and @PositionState when the Flight Mode selected in @FlightStatus is supported
 * by this module.  Otherwise another module (e.g. @ref ManualControlCommand) is expected to be
 * writing to @ref StabilizationDesired.
 *
 * The module executes in its own thread in this example.
 *
 * Modules have no API, all communication to other modules is done through UAVObjects.
 * However modules may use the API exposed by shared libraries.
 * See the OpenPilot wiki for more details.
 * http://www.openpilot.org/OpenPilot_Application_Architecture
 *
 */

#include <openpilot.h>

#include "vtolpathfollower.h"

#include "accelstate.h"
#include "attitudestate.h"
#include "hwsettings.h"
#include "pathdesired.h" // object that will be updated by the module
#include "positionstate.h"
#include "manualcontrol.h"
#include "flightstatus.h"
#include "pathstatus.h"
#include "gpsvelocitysensor.h"
#include "gpspositionsensor.h"
#include "homelocation.h"
#include "vtolpathfollowersettings.h"
#include "nedaccel.h"
#include "stabilizationdesired.h"
#include "stabilizationsettings.h"
#include "systemsettings.h"
#include "velocitydesired.h"
#include "velocitystate.h"
#include "taskinfo.h"

#include "paths.h"
#include "CoordinateConversions.h"

#include "cameradesired.h"
#include "poilearnsettings.h"
#include "poilocation.h"
#include "accessorydesired.h"

// Private constants
#define MAX_QUEUE_SIZE   4
#define STACK_SIZE_BYTES 1548
#define TASK_PRIORITY    (tskIDLE_PRIORITY + 2)

// Private types

// Private variables
static xTaskHandle pathfollowerTaskHandle;
static PathStatusData pathStatus;
static VtolPathFollowerSettingsData vtolpathfollowerSettings;
static float poiRadius;

// Private functions
static void vtolPathFollowerTask(void *parameters);
static void SettingsUpdatedCb(UAVObjEvent *ev);
static void updateNedAccel();
static void updatePOIBearing();
static void updatePathVelocity();
static void updateEndpointVelocity();
static void updateFixedAttitude(float *attitude);
static void updateVtolDesiredAttitude(bool yaw_attitude);
static float bound(float val, float min, float max);
static bool vtolpathfollower_enabled;
static void accessoryUpdated(UAVObjEvent *ev);

/**
 * Initialise the module, called on startup
 * \returns 0 on success or -1 if initialisation failed
 */
int32_t VtolPathFollowerStart()
{
    if (vtolpathfollower_enabled) {
        // Start main task
        xTaskCreate(vtolPathFollowerTask, (signed char *)"VtolPathFollower", STACK_SIZE_BYTES / 4, NULL, TASK_PRIORITY, &pathfollowerTaskHandle);
        PIOS_TASK_MONITOR_RegisterTask(TASKINFO_RUNNING_PATHFOLLOWER, pathfollowerTaskHandle);
    }

    return 0;
}

/**
 * Initialise the module, called on startup
 * \returns 0 on success or -1 if initialisation failed
 */
int32_t VtolPathFollowerInitialize()
{
    uint8_t optionalModules[HWSETTINGS_OPTIONALMODULES_NUMELEM];

    HwSettingsOptionalModulesGet(optionalModules);

    if (optionalModules[HWSETTINGS_OPTIONALMODULES_VTOLPATHFOLLOWER] == HWSETTINGS_OPTIONALMODULES_ENABLED) {
        VtolPathFollowerSettingsInitialize();
        NedAccelInitialize();
        PathDesiredInitialize();
        PathStatusInitialize();
        VelocityDesiredInitialize();
        CameraDesiredInitialize();
        AccessoryDesiredInitialize();
        PoiLearnSettingsInitialize();
        PoiLocationInitialize();
        vtolpathfollower_enabled = true;
    } else {
        vtolpathfollower_enabled = false;
    }

    return 0;
}

MODULE_INITCALL(VtolPathFollowerInitialize, VtolPathFollowerStart);

static float northVelIntegral = 0;
static float eastVelIntegral  = 0;
static float downVelIntegral  = 0;

static float northPosIntegral = 0;
static float eastPosIntegral  = 0;
static float downPosIntegral  = 0;

static float throttleOffset   = 0;
/**
 * Module thread, should not return.
 */
static void vtolPathFollowerTask(__attribute__((unused)) void *parameters)
{
    SystemSettingsData systemSettings;
    FlightStatusData flightStatus;

    portTickType lastUpdateTime;

    VtolPathFollowerSettingsConnectCallback(SettingsUpdatedCb);
    AccessoryDesiredConnectCallback(accessoryUpdated);

    VtolPathFollowerSettingsGet(&vtolpathfollowerSettings);

    // Main task loop
    lastUpdateTime = xTaskGetTickCount();
    while (1) {
        // Conditions when this runs:
        // 1. Must have VTOL type airframe
        // 2. Flight mode is PositionHold and PathDesired.Mode is Endpoint  OR
        // FlightMode is PathPlanner and PathDesired.Mode is Endpoint or Path

        SystemSettingsGet(&systemSettings);
        if ((systemSettings.AirframeType != SYSTEMSETTINGS_AIRFRAMETYPE_VTOL) && (systemSettings.AirframeType != SYSTEMSETTINGS_AIRFRAMETYPE_QUADP)
            && (systemSettings.AirframeType != SYSTEMSETTINGS_AIRFRAMETYPE_OCTOCOAXX) && (systemSettings.AirframeType != SYSTEMSETTINGS_AIRFRAMETYPE_QUADX)
            && (systemSettings.AirframeType != SYSTEMSETTINGS_AIRFRAMETYPE_HEXA) && (systemSettings.AirframeType != SYSTEMSETTINGS_AIRFRAMETYPE_HEXAX)
            && (systemSettings.AirframeType != SYSTEMSETTINGS_AIRFRAMETYPE_HEXACOAX) && (systemSettings.AirframeType != SYSTEMSETTINGS_AIRFRAMETYPE_OCTO)
            && (systemSettings.AirframeType != SYSTEMSETTINGS_AIRFRAMETYPE_OCTOV) && (systemSettings.AirframeType != SYSTEMSETTINGS_AIRFRAMETYPE_OCTOCOAXP)
            && (systemSettings.AirframeType != SYSTEMSETTINGS_AIRFRAMETYPE_TRI)) {
            AlarmsSet(SYSTEMALARMS_ALARM_GUIDANCE, SYSTEMALARMS_ALARM_WARNING);
            vTaskDelay(1000);
            continue;
        }

        // Continue collecting data if not enough time
        vTaskDelayUntil(&lastUpdateTime, vtolpathfollowerSettings.UpdatePeriod / portTICK_RATE_MS);

        // Convert the accels into the NED frame
        updateNedAccel();

        FlightStatusGet(&flightStatus);
        PathStatusGet(&pathStatus);
        PathDesiredData pathDesired;
        PathDesiredGet(&pathDesired);

        // Check the combinations of flightmode and pathdesired mode
        switch (flightStatus.FlightMode) {
        case FLIGHTSTATUS_FLIGHTMODE_LAND:
        case FLIGHTSTATUS_FLIGHTMODE_POSITIONHOLD:
        case FLIGHTSTATUS_FLIGHTMODE_RETURNTOBASE:
            if (pathDesired.Mode == PATHDESIRED_MODE_FLYENDPOINT) {
                updateEndpointVelocity();
                updateVtolDesiredAttitude(false);
                AlarmsSet(SYSTEMALARMS_ALARM_GUIDANCE, SYSTEMALARMS_ALARM_OK);
            } else {
                AlarmsSet(SYSTEMALARMS_ALARM_GUIDANCE, SYSTEMALARMS_ALARM_ERROR);
            }
            break;
        case FLIGHTSTATUS_FLIGHTMODE_PATHPLANNER:
            pathStatus.UID    = pathDesired.UID;
            pathStatus.Status = PATHSTATUS_STATUS_INPROGRESS;
            switch (pathDesired.Mode) {
            // TODO: Make updateVtolDesiredAttitude and velocity report success and update PATHSTATUS_STATUS accordingly
            case PATHDESIRED_MODE_FLYENDPOINT:
            case PATHDESIRED_MODE_FLYVECTOR:
            case PATHDESIRED_MODE_FLYCIRCLERIGHT:
            case PATHDESIRED_MODE_FLYCIRCLELEFT:
                updatePathVelocity();
                updateVtolDesiredAttitude(false);
                AlarmsSet(SYSTEMALARMS_ALARM_GUIDANCE, SYSTEMALARMS_ALARM_OK);
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
            break;
        case FLIGHTSTATUS_FLIGHTMODE_POI:
            if (pathDesired.Mode == PATHDESIRED_MODE_FLYENDPOINT) {
                updateEndpointVelocity();
                updateVtolDesiredAttitude(true);
                updatePOIBearing();
            } else {
                AlarmsSet(SYSTEMALARMS_ALARM_GUIDANCE, SYSTEMALARMS_ALARM_ERROR);
            }
            break;
        default:
            // Be cleaner and get rid of global variables
            northVelIntegral = 0;
            eastVelIntegral  = 0;
            downVelIntegral  = 0;
            northPosIntegral = 0;
            eastPosIntegral  = 0;
            downPosIntegral  = 0;

            // Track throttle before engaging this mode.  Cheap system ident
            StabilizationDesiredData stabDesired;
            StabilizationDesiredGet(&stabDesired);
            throttleOffset = stabDesired.Throttle;

            break;
        }

        AlarmsClear(SYSTEMALARMS_ALARM_GUIDANCE);
    }
}

/**
 * Compute bearing and elevation between current position and POI
 */
static void updatePOIBearing()
{
    const float DEADBAND_HIGH = 0.10f;
    const float DEADBAND_LOW  = -0.10f;
    float dT = vtolpathfollowerSettings.UpdatePeriod / 1000.0f;

    PathDesiredData pathDesired;

    PathDesiredGet(&pathDesired);
    PositionStateData positionState;
    PositionStateGet(&positionState);
    CameraDesiredData cameraDesired;
    CameraDesiredGet(&cameraDesired);
    StabilizationDesiredData stabDesired;
    StabilizationDesiredGet(&stabDesired);
    PoiLocationData poi;
    PoiLocationGet(&poi);

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
        poiRadius = distance + changeRadius;
    }

    // don't try to move any closer
    if (poiRadius >= 3.0f || changeRadius > 0) {
        if (fabsf(pathAngle) > 0.0f || fabsf(changeRadius) > 0.0f) {
            pathDesired.End.fields.North = poi.North + (poiRadius * cosf(DEG2RAD(pathAngle + yaw - 180.0f)));
            pathDesired.End.fields.East  = poi.East + (poiRadius * sinf(DEG2RAD(pathAngle + yaw - 180.0f)));
            pathDesired.StartingVelocity = 1.0f;
            pathDesired.EndingVelocity   = 0.0f;
            pathDesired.Mode = PATHDESIRED_MODE_FLYENDPOINT;
            PathDesiredSet(&pathDesired);
        }
    }
    // not above
    if (distance >= 3.0f) {
        // You can feed this into camerastabilization
        /*elevation = RAD2DEG(atan2f(dLoc[2],distance));*/

        stabDesired.Yaw = yaw + (pathAngle / 2.0f);
        stabDesired.StabilizationMode.fields.Yaw = STABILIZATIONDESIRED_STABILIZATIONMODE_ATTITUDE;

        // cameraDesired.Yaw=yaw;
        // cameraDesired.PitchOrServo2=elevation;

        CameraDesiredSet(&cameraDesired);
        StabilizationDesiredSet(&stabDesired);
    }
}

/**
 * Compute desired velocity from the current position and path
 *
 * Takes in @ref PositionState and compares it to @ref PathDesired
 * and computes @ref VelocityDesired
 */
static void updatePathVelocity()
{
    float dT = vtolpathfollowerSettings.UpdatePeriod / 1000.0f;
    float downCommand;

    PathDesiredData pathDesired;

    PathDesiredGet(&pathDesired);
    PositionStateData positionState;
    PositionStateGet(&positionState);

    float cur[3] =
    { positionState.North, positionState.East, positionState.Down };
    struct path_status progress;

    path_progress(pathDesired.Start.data, pathDesired.End.data, cur, &progress, pathDesired.Mode);

    float groundspeed;
    switch (pathDesired.Mode) {
    case PATHDESIRED_MODE_FLYCIRCLERIGHT:
    case PATHDESIRED_MODE_DRIVECIRCLERIGHT:
    case PATHDESIRED_MODE_FLYCIRCLELEFT:
    case PATHDESIRED_MODE_DRIVECIRCLELEFT:
        groundspeed = pathDesired.EndingVelocity;
        break;
    case PATHDESIRED_MODE_FLYENDPOINT:
    case PATHDESIRED_MODE_DRIVEENDPOINT:
        groundspeed = pathDesired.EndingVelocity - pathDesired.EndingVelocity * bound(progress.fractional_progress, 0, 1);
        if (progress.fractional_progress > 1) {
            groundspeed = 0;
        }
        break;
    case PATHDESIRED_MODE_FLYVECTOR:
    case PATHDESIRED_MODE_DRIVEVECTOR:
    default:
        groundspeed = pathDesired.StartingVelocity
                      + (pathDesired.EndingVelocity - pathDesired.StartingVelocity) * bound(progress.fractional_progress, 0, 1);
        if (progress.fractional_progress > 1) {
            groundspeed = 0;
        }
        break;
    }

    VelocityDesiredData velocityDesired;
    velocityDesired.North = progress.path_direction[0] * groundspeed;
    velocityDesired.East  = progress.path_direction[1] * groundspeed;

    float error_speed = progress.error * vtolpathfollowerSettings.HorizontalPosPI.fields.Kp;
    float correction_velocity[2] =
    { progress.correction_direction[0] * error_speed, progress.correction_direction[1] * error_speed };

    float total_vel = sqrtf(powf(correction_velocity[0], 2) + powf(correction_velocity[1], 2));
    float scale     = 1;
    if (total_vel > vtolpathfollowerSettings.HorizontalVelMax) {
        scale = vtolpathfollowerSettings.HorizontalVelMax / total_vel;
    }

    velocityDesired.North += progress.correction_direction[0] * error_speed * scale;
    velocityDesired.East  += progress.correction_direction[1] * error_speed * scale;

    float altitudeSetpoint = pathDesired.Start.fields.Down + (pathDesired.End.fields.Down - pathDesired.Start.fields.Down) * bound(progress.fractional_progress, 0, 1);

    float downError = altitudeSetpoint - positionState.Down;
    downPosIntegral = bound(downPosIntegral + downError * dT * vtolpathfollowerSettings.VerticalPosPI.fields.Ki,
                            -vtolpathfollowerSettings.VerticalPosPI.fields.ILimit,
                            vtolpathfollowerSettings.VerticalPosPI.fields.ILimit);
    downCommand     = (downError * vtolpathfollowerSettings.VerticalPosPI.fields.Kp + downPosIntegral);
    velocityDesired.Down = bound(downCommand, -vtolpathfollowerSettings.VerticalVelMax, vtolpathfollowerSettings.VerticalVelMax);

    // update pathstatus
    pathStatus.error     = progress.error;
    pathStatus.fractional_progress = progress.fractional_progress;

    VelocityDesiredSet(&velocityDesired);
}

/**
 * Compute desired velocity from the current position
 *
 * Takes in @ref PositionState and compares it to @ref PositionDesired
 * and computes @ref VelocityDesired
 */
void updateEndpointVelocity()
{
    float dT = vtolpathfollowerSettings.UpdatePeriod / 1000.0f;

    PathDesiredData pathDesired;

    PathDesiredGet(&pathDesired);

    PositionStateData positionState;
    VelocityDesiredData velocityDesired;

    PositionStateGet(&positionState);
    VelocityDesiredGet(&velocityDesired);

    float northError;
    float eastError;
    float downError;
    float northCommand;
    float eastCommand;
    float downCommand;

    float northPos = 0, eastPos = 0, downPos = 0;
    switch (vtolpathfollowerSettings.PositionSource) {
    case VTOLPATHFOLLOWERSETTINGS_POSITIONSOURCE_EKF:
        northPos = positionState.North;
        eastPos  = positionState.East;
        downPos  = positionState.Down;
        break;
    case VTOLPATHFOLLOWERSETTINGS_POSITIONSOURCE_GPSPOS:
    {
        // this used to work with the NEDposition UAVObject
        // however this UAVObject has been removed
        GPSPositionSensorData gpsPosition;
        GPSPositionSensorGet(&gpsPosition);
        HomeLocationData homeLocation;
        HomeLocationGet(&homeLocation);
        float lat    = DEG2RAD(homeLocation.Latitude / 10.0e6f);
        float alt    = homeLocation.Altitude;
        float T[3]   = { alt + 6.378137E6f,
                         cosf(lat) * (alt + 6.378137E6f),
                         -1.0f };
        float NED[3] = { T[0] * (DEG2RAD((gpsPosition.Latitude - homeLocation.Latitude) / 10.0e6f)),
                         T[1] * (DEG2RAD((gpsPosition.Longitude - homeLocation.Longitude) / 10.0e6f)),
                         T[2] * ((gpsPosition.Altitude + gpsPosition.GeoidSeparation - homeLocation.Altitude)) };

        northPos = NED[0];
        eastPos  = NED[1];
        downPos  = NED[2];
    }
    break;
    default:
        PIOS_Assert(0);
        break;
    }

    // Compute desired north command
    northError = pathDesired.End.fields.North - northPos;
    northPosIntegral = bound(northPosIntegral + northError * dT * vtolpathfollowerSettings.HorizontalPosPI.fields.Ki,
                             -vtolpathfollowerSettings.HorizontalPosPI.fields.ILimit,
                             vtolpathfollowerSettings.HorizontalPosPI.fields.ILimit);
    northCommand     = (northError * vtolpathfollowerSettings.HorizontalPosPI.fields.Kp + northPosIntegral);

    eastError = pathDesired.End.fields.East - eastPos;
    eastPosIntegral  = bound(eastPosIntegral + eastError * dT * vtolpathfollowerSettings.HorizontalPosPI.fields.Ki,
                             -vtolpathfollowerSettings.HorizontalPosPI.fields.ILimit,
                             vtolpathfollowerSettings.HorizontalPosPI.fields.ILimit);
    eastCommand = (eastError * vtolpathfollowerSettings.HorizontalPosPI.fields.Kp + eastPosIntegral);

    // Limit the maximum velocity
    float total_vel = sqrtf(powf(northCommand, 2) + powf(eastCommand, 2));
    float scale     = 1;
    if (total_vel > vtolpathfollowerSettings.HorizontalVelMax) {
        scale = vtolpathfollowerSettings.HorizontalVelMax / total_vel;
    }

    velocityDesired.North = northCommand * scale;
    velocityDesired.East  = eastCommand * scale;

    downError = pathDesired.End.fields.Down - downPos;
    downPosIntegral = bound(downPosIntegral + downError * dT * vtolpathfollowerSettings.VerticalPosPI.fields.Ki,
                            -vtolpathfollowerSettings.VerticalPosPI.fields.ILimit,
                            vtolpathfollowerSettings.VerticalPosPI.fields.ILimit);
    downCommand     = (downError * vtolpathfollowerSettings.VerticalPosPI.fields.Kp + downPosIntegral);
    velocityDesired.Down = bound(downCommand, -vtolpathfollowerSettings.VerticalVelMax, vtolpathfollowerSettings.VerticalVelMax);

    VelocityDesiredSet(&velocityDesired);
}

/**
 * Compute desired attitude from a fixed preset
 *
 */
static void updateFixedAttitude(float *attitude)
{
    StabilizationDesiredData stabDesired;

    StabilizationDesiredGet(&stabDesired);
    stabDesired.Roll     = attitude[0];
    stabDesired.Pitch    = attitude[1];
    stabDesired.Yaw      = attitude[2];
    stabDesired.Throttle = attitude[3];
    stabDesired.StabilizationMode.fields.Roll  = STABILIZATIONDESIRED_STABILIZATIONMODE_ATTITUDE;
    stabDesired.StabilizationMode.fields.Pitch = STABILIZATIONDESIRED_STABILIZATIONMODE_ATTITUDE;
    stabDesired.StabilizationMode.fields.Yaw   = STABILIZATIONDESIRED_STABILIZATIONMODE_AXISLOCK;
    StabilizationDesiredSet(&stabDesired);
}

/**
 * Compute desired attitude from the desired velocity
 *
 * Takes in @ref NedState which has the acceleration in the
 * NED frame as the feedback term and then compares the
 * @ref VelocityState against the @ref VelocityDesired
 */
static void updateVtolDesiredAttitude(bool yaw_attitude)
{
    float dT = vtolpathfollowerSettings.UpdatePeriod / 1000.0f;

    VelocityDesiredData velocityDesired;
    VelocityStateData velocityState;
    StabilizationDesiredData stabDesired;
    AttitudeStateData attitudeState;
    NedAccelData nedAccel;
    StabilizationSettingsData stabSettings;
    SystemSettingsData systemSettings;

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
    StabilizationSettingsGet(&stabSettings);
    NedAccelGet(&nedAccel);

    float northVel = 0, eastVel = 0, downVel = 0;
    switch (vtolpathfollowerSettings.VelocitySource) {
    case VTOLPATHFOLLOWERSETTINGS_VELOCITYSOURCE_EKF:
        northVel = velocityState.North;
        eastVel  = velocityState.East;
        downVel  = velocityState.Down;
        break;
    case VTOLPATHFOLLOWERSETTINGS_VELOCITYSOURCE_NEDVEL:
    {
        GPSVelocitySensorData gpsVelocity;
        GPSVelocitySensorGet(&gpsVelocity);
        northVel = gpsVelocity.North;
        eastVel  = gpsVelocity.East;
        downVel  = gpsVelocity.Down;
    }
    break;
    case VTOLPATHFOLLOWERSETTINGS_VELOCITYSOURCE_GPSPOS:
    {
        GPSPositionSensorData gpsPosition;
        GPSPositionSensorGet(&gpsPosition);
        northVel = gpsPosition.Groundspeed * cosf(DEG2RAD(gpsPosition.Heading));
        eastVel  = gpsPosition.Groundspeed * sinf(DEG2RAD(gpsPosition.Heading));
        downVel  = velocityState.Down;
    }
    break;
    default:
        PIOS_Assert(0);
        break;
    }

    // Testing code - refactor into manual control command
    ManualControlCommandData manualControlData;
    ManualControlCommandGet(&manualControlData);

    // Compute desired north command
    northError = velocityDesired.North - northVel;
    northVelIntegral = bound(northVelIntegral + northError * dT * vtolpathfollowerSettings.HorizontalVelPID.fields.Ki,
                             -vtolpathfollowerSettings.HorizontalVelPID.fields.ILimit,
                             vtolpathfollowerSettings.HorizontalVelPID.fields.ILimit);
    northCommand     = (northError * vtolpathfollowerSettings.HorizontalVelPID.fields.Kp + northVelIntegral
                        - nedAccel.North * vtolpathfollowerSettings.HorizontalVelPID.fields.Kd
                        + velocityDesired.North * vtolpathfollowerSettings.VelocityFeedforward);

    // Compute desired east command
    eastError = velocityDesired.East - eastVel;
    eastVelIntegral = bound(eastVelIntegral + eastError * dT * vtolpathfollowerSettings.HorizontalVelPID.fields.Ki,
                            -vtolpathfollowerSettings.HorizontalVelPID.fields.ILimit,
                            vtolpathfollowerSettings.HorizontalVelPID.fields.ILimit);
    eastCommand     = (eastError * vtolpathfollowerSettings.HorizontalVelPID.fields.Kp + eastVelIntegral
                       - nedAccel.East * vtolpathfollowerSettings.HorizontalVelPID.fields.Kd
                       + velocityDesired.East * vtolpathfollowerSettings.VelocityFeedforward);

    // Compute desired down command
    downError = velocityDesired.Down - downVel;
    // Must flip this sign
    downError = -downError;
    downVelIntegral = bound(downVelIntegral + downError * dT * vtolpathfollowerSettings.VerticalVelPID.fields.Ki,
                            -vtolpathfollowerSettings.VerticalVelPID.fields.ILimit,
                            vtolpathfollowerSettings.VerticalVelPID.fields.ILimit);
    downCommand     = (downError * vtolpathfollowerSettings.VerticalVelPID.fields.Kp + downVelIntegral
                       - nedAccel.Down * vtolpathfollowerSettings.VerticalVelPID.fields.Kd);

    stabDesired.Throttle = bound(downCommand + throttleOffset, 0, 1);

    // Project the north and east command signals into the pitch and roll based on yaw.  For this to behave well the
    // craft should move similarly for 5 deg roll versus 5 deg pitch
    stabDesired.Pitch = bound(-northCommand * cosf(DEG2RAD(attitudeState.Yaw)) +
                              -eastCommand * sinf(DEG2RAD(attitudeState.Yaw)),
                              -vtolpathfollowerSettings.MaxRollPitch, vtolpathfollowerSettings.MaxRollPitch);
    stabDesired.Roll  = bound(-northCommand * sinf(DEG2RAD(attitudeState.Yaw)) +
                              eastCommand * cosf(DEG2RAD(attitudeState.Yaw)),
                              -vtolpathfollowerSettings.MaxRollPitch, vtolpathfollowerSettings.MaxRollPitch);

    if (vtolpathfollowerSettings.ThrottleControl == VTOLPATHFOLLOWERSETTINGS_THROTTLECONTROL_FALSE) {
        // For now override throttle with manual control.  Disable at your risk, quad goes to China.
        ManualControlCommandData manualControl;
        ManualControlCommandGet(&manualControl);
        stabDesired.Throttle = manualControl.Throttle;
    }

    stabDesired.StabilizationMode.fields.Roll  = STABILIZATIONDESIRED_STABILIZATIONMODE_ATTITUDE;
    stabDesired.StabilizationMode.fields.Pitch = STABILIZATIONDESIRED_STABILIZATIONMODE_ATTITUDE;
    if (yaw_attitude) {
        stabDesired.StabilizationMode.fields.Yaw = STABILIZATIONDESIRED_STABILIZATIONMODE_ATTITUDE;
    } else {
        stabDesired.StabilizationMode.fields.Yaw = STABILIZATIONDESIRED_STABILIZATIONMODE_AXISLOCK;
        stabDesired.Yaw = stabSettings.MaximumRate.fields.Yaw * manualControlData.Yaw;
    }
    StabilizationDesiredSet(&stabDesired);
}

/**
 * Keep a running filtered version of the acceleration in the NED frame
 */
static void updateNedAccel()
{
    float accel[3];
    float q[4];
    float Rbe[3][3];
    float accel_ned[3];

    // Collect downsampled attitude data
    AccelStateData accelState;

    AccelStateGet(&accelState);
    accel[0] = accelState.x;
    accel[1] = accelState.y;
    accel[2] = accelState.z;

    // rotate avg accels into earth frame and store it
    AttitudeStateData attitudeState;
    AttitudeStateGet(&attitudeState);
    q[0] = attitudeState.q1;
    q[1] = attitudeState.q2;
    q[2] = attitudeState.q3;
    q[3] = attitudeState.q4;
    Quaternion2R(q, Rbe);
    for (uint8_t i = 0; i < 3; i++) {
        accel_ned[i] = 0;
        for (uint8_t j = 0; j < 3; j++) {
            accel_ned[i] += Rbe[j][i] * accel[j];
        }
    }
    accel_ned[2] += 9.81f;

    NedAccelData accelData;
    NedAccelGet(&accelData);
    accelData.North = accel_ned[0];
    accelData.East  = accel_ned[1];
    accelData.Down  = accel_ned[2];
    NedAccelSet(&accelData);
}

/**
 * Bound input value between limits
 */
static float bound(float val, float min, float max)
{
    if (val < min) {
        val = min;
    } else if (val > max) {
        val = max;
    }
    return val;
}

static void SettingsUpdatedCb(__attribute__((unused)) UAVObjEvent *ev)
{
    VtolPathFollowerSettingsGet(&vtolpathfollowerSettings);
}

static void accessoryUpdated(UAVObjEvent *ev)
{
    if (ev->obj != AccessoryDesiredHandle()) {
        return;
    }

    AccessoryDesiredData accessory;
    PoiLearnSettingsData poiLearn;
    PoiLearnSettingsGet(&poiLearn);

    if (poiLearn.Input != POILEARNSETTINGS_INPUT_NONE) {
        if (AccessoryDesiredInstGet(poiLearn.Input - POILEARNSETTINGS_INPUT_ACCESSORY0, &accessory) == 0) {
            if (accessory.AccessoryVal < -0.5f) {
                PositionStateData positionState;
                PositionStateGet(&positionState);
                PoiLocationData poi;
                PoiLocationGet(&poi);
                poi.North = positionState.North;
                poi.East  = positionState.East;
                poi.Down  = positionState.Down;
                PoiLocationSet(&poi);
            }
        }
    }
}

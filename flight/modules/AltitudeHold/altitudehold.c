/**
 ******************************************************************************
 *
 * @file       guidance.c
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
 * Input object: ActiveWaypoint
 * Input object: PositionState
 * Input object: ManualControlCommand
 * Output object: AttitudeDesired
 *
 * This module will periodically update the value of the AttitudeDesired object.
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

#include <math.h>
#include <CoordinateConversions.h>
#include <altholdsmoothed.h>
#include <attitudestate.h>
#include <altitudeholdsettings.h>
#include <altitudeholddesired.h> // object that will be updated by the module
#include <barosensor.h>
#include <positionstate.h>
#include <flightstatus.h>
#include <stabilizationdesired.h>
#include <accelstate.h>
#include <taskinfo.h>
#include <pios_constants.h>
#include <velocitystate.h>
#include <positionstate.h>
// Private constants
#define MAX_QUEUE_SIZE   2
#define STACK_SIZE_BYTES 1024
#define TASK_PRIORITY    (tskIDLE_PRIORITY + 1)
#define ACCEL_DOWNSAMPLE 4
#define TIMEOUT_TRESHOLD 200000
// Private types

// Private variables
static xTaskHandle altitudeHoldTaskHandle;
static xQueueHandle queue;
static AltitudeHoldSettingsData altitudeHoldSettings;

// Private functions
static void altitudeHoldTask(void *parameters);
static void SettingsUpdatedCb(UAVObjEvent *ev);

/**
 * Initialise the module, called on startup
 * \returns 0 on success or -1 if initialisation failed
 */
int32_t AltitudeHoldStart()
{
    // Start main task
    xTaskCreate(altitudeHoldTask, (signed char *)"AltitudeHold", STACK_SIZE_BYTES / 4, NULL, TASK_PRIORITY, &altitudeHoldTaskHandle);
    PIOS_TASK_MONITOR_RegisterTask(TASKINFO_RUNNING_ALTITUDEHOLD, altitudeHoldTaskHandle);

    return 0;
}

/**
 * Initialise the module, called on startup
 * \returns 0 on success or -1 if initialisation failed
 */
int32_t AltitudeHoldInitialize()
{
    AltitudeHoldSettingsInitialize();
    AltitudeHoldDesiredInitialize();
    AltHoldSmoothedInitialize();

    // Create object queue
    queue = xQueueCreate(MAX_QUEUE_SIZE, sizeof(UAVObjEvent));

    AltitudeHoldSettingsConnectCallback(&SettingsUpdatedCb);

    return 0;
}
MODULE_INITCALL(AltitudeHoldInitialize, AltitudeHoldStart);

float throttleIntegral;
float switchThrottle;
float velocity;
bool running = false;
float error;
float velError;
uint32_t timeval;

bool posUpdated;

/**
 * Module thread, should not return.
 */
static void altitudeHoldTask(__attribute__((unused)) void *parameters)
{
    AltitudeHoldDesiredData altitudeHoldDesired;
    StabilizationDesiredData stabilizationDesired;
    AltHoldSmoothedData altHold;
    AttitudeStateData attitudeState;
    AccelStateData accelState;
    VelocityStateData velocityData;
    float dT;
    float q[4], Rbe[3][3];

    portTickType thisTime, lastUpdateTime;
    UAVObjEvent ev;

    timeval = 0;
    lastUpdateTime = 0;
    // Force update of the settings
    SettingsUpdatedCb(&ev);
    // Failsafe handling
    uint32_t lastAltitudeHoldDesiredUpdate = 0;
    bool enterFailSafe = false;
    // Listen for updates.
    AltitudeHoldDesiredConnectQueue(queue);
    AccelStateConnectQueue(queue);
    PositionStateConnectQueue(queue);
    FlightStatusConnectQueue(queue);
    VelocityStateConnectQueue(queue);
    bool altitudeHoldFlightMode = false;
    running = false;
    enum init_state { WAITING_BARO, WAITIING_INIT, INITED } init = WAITING_BARO;

    uint8_t flightMode;
    FlightStatusFlightModeGet(&flightMode);
    // initialize enable flag
    altitudeHoldFlightMode = flightMode == FLIGHTSTATUS_FLIGHTMODE_ALTITUDEHOLD || flightMode == FLIGHTSTATUS_FLIGHTMODE_ALTITUDEVARIO;
    // Main task loop
    while (1) {
        enterFailSafe = PIOS_DELAY_DiffuS(lastAltitudeHoldDesiredUpdate) > TIMEOUT_TRESHOLD;
        // Wait until the AttitudeRaw object is updated, if a timeout then go to failsafe
        if (xQueueReceive(queue, &ev, 100 / portTICK_RATE_MS) != pdTRUE) {
            if (!running) {
                throttleIntegral = 0;
            }

            // Todo: Add alarm if it should be running
            continue;
        } else if (ev.obj == PositionStateHandle()) {
            posUpdated = true;

            init = (init == WAITING_BARO) ? WAITIING_INIT : init;
        } else if (ev.obj == FlightStatusHandle()) {
            FlightStatusFlightModeGet(&flightMode);
            altitudeHoldFlightMode = flightMode == FLIGHTSTATUS_FLIGHTMODE_ALTITUDEHOLD || flightMode == FLIGHTSTATUS_FLIGHTMODE_ALTITUDEVARIO;
            if (altitudeHoldFlightMode && !running) {
                // Copy the current throttle as a starting point for integral
                StabilizationDesiredThrottleGet(&throttleIntegral);
                switchThrottle = throttleIntegral;
                throttleIntegral *= Rbe[2][2]; // rotate into earth frame
                if (throttleIntegral > 1) {
                    throttleIntegral = 1;
                } else if (throttleIntegral < 0) {
                    throttleIntegral = 0;
                }
                error    = 0;
                velocity = 0;
                altitudeHoldDesired.Altitude = altHold.Altitude;
                running  = true;
            } else if (!altitudeHoldFlightMode) {
                running = false;
                lastAltitudeHoldDesiredUpdate = PIOS_DELAY_GetRaw();
            }
        } else if (ev.obj == AccelStateHandle()) {
            dT = PIOS_DELAY_DiffuS(timeval) / 1.0e6f;
            timeval = PIOS_DELAY_GetRaw();

            AltHoldSmoothedGet(&altHold);

            AttitudeStateGet(&attitudeState);

            q[0] = attitudeState.q1;
            q[1] = attitudeState.q2;
            q[2] = attitudeState.q3;
            q[3] = attitudeState.q4;
            Quaternion2R(q, Rbe);

            AccelStateGet(&accelState);
            altHold.Accel = 0.95f * altHold.Accel + 0.05f * (Rbe[0][2] * accelState.x + Rbe[1][2] * accelState.y + Rbe[2][2] * accelState.z + 9.81f);

            VelocityStateGet(&velocityData);

            altHold.Velocity = 0.95f * altHold.Velocity + 0.05f * (Rbe[0][2] * velocityData.East + Rbe[1][2] * velocityData.North + Rbe[2][2] * velocityData.Down);

            PositionStateDownGet(&altHold.Altitude);

            AltHoldSmoothedSet(&altHold);

            // Verify that we are in altitude hold mode
            uint8_t armed;
            FlightStatusArmedGet(&armed);
            if (!altitudeHoldFlightMode || armed != FLIGHTSTATUS_ARMED_ARMED) {
                running = false;
            }

            if (!running) {
                lastAltitudeHoldDesiredUpdate = PIOS_DELAY_GetRaw();
                continue;
            }

            // Compute the altitude error
            error = altitudeHoldDesired.Altitude - altHold.Altitude;
            velError = altitudeHoldDesired.Velocity - altHold.Velocity;

            if(fabsf(altitudeHoldDesired.Velocity) < 1e-3f) {
                // Compute integral off altitude error
                throttleIntegral += error * altitudeHoldSettings.Ki * dT;
            }
            thisTime = xTaskGetTickCount();
            // Only update stabilizationDesired less frequently
            if ((thisTime - lastUpdateTime) < 20) {
                continue;
            }

            lastUpdateTime = thisTime;

            // Instead of explicit limit on integral you output limit feedback
            StabilizationDesiredGet(&stabilizationDesired);
            if (!enterFailSafe) {
                if(fabsf(altitudeHoldDesired.Velocity) < 1e-3f) {
                    stabilizationDesired.Throttle = - error * altitudeHoldSettings.Kp + throttleIntegral +
                                                    altHold.Velocity * altitudeHoldSettings.Kd;// - altHold.Accel * altitudeHoldSettings.Ka;
                    // scale up throttle to compensate for roll/pitch angle but limit this to 60 deg (cos(60) == 0.5) to prevent excessive scaling
                    float throttlescale = Rbe[2][2] < 0.5f ? 0.5f : Rbe[2][2];
                    stabilizationDesired.Throttle /= throttlescale;
                } else {
                    stabilizationDesired.Throttle = -velError * altitudeHoldSettings.Kv + throttleIntegral;
                }

                if (stabilizationDesired.Throttle > 1) {
                    throttleIntegral -= (stabilizationDesired.Throttle - 1);
                    stabilizationDesired.Throttle = 1;
                } else if (stabilizationDesired.Throttle < 0) {
                    throttleIntegral -= stabilizationDesired.Throttle;
                    stabilizationDesired.Throttle = 0;
                }
            } else {
                // shutdown motors
                stabilizationDesired.Throttle = -1;
            }
            stabilizationDesired.StabilizationMode[STABILIZATIONDESIRED_STABILIZATIONMODE_ROLL]  = STABILIZATIONDESIRED_STABILIZATIONMODE_ATTITUDE;
            stabilizationDesired.StabilizationMode[STABILIZATIONDESIRED_STABILIZATIONMODE_PITCH] = STABILIZATIONDESIRED_STABILIZATIONMODE_ATTITUDE;
            stabilizationDesired.StabilizationMode[STABILIZATIONDESIRED_STABILIZATIONMODE_YAW]   = STABILIZATIONDESIRED_STABILIZATIONMODE_AXISLOCK;
            stabilizationDesired.Roll  = altitudeHoldDesired.Roll;
            stabilizationDesired.Pitch = altitudeHoldDesired.Pitch;
            stabilizationDesired.Yaw   = altitudeHoldDesired.Yaw;

            StabilizationDesiredSet(&stabilizationDesired);
        } else if (ev.obj == AltitudeHoldDesiredHandle()) {
            // reset the failsafe timer
            lastAltitudeHoldDesiredUpdate = PIOS_DELAY_GetRaw();
            AltitudeHoldDesiredGet(&altitudeHoldDesired);
        }
    }
}

static void SettingsUpdatedCb(__attribute__((unused)) UAVObjEvent *ev)
{
    AltitudeHoldSettingsGet(&altitudeHoldSettings);
}

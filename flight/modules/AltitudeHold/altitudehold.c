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

float tau;
float altitudeIntegral;
float velocityIntegral;
float decay;
float velocity_decay;
float velocity;
float accelAlpha;
float velAlpha;
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
    VelocityStateData velocityData;
    float dT;
    float q[4], Rbe[3][3], fblimit = 0;
    float lastVertVelocity;
    portTickType thisTime, lastUpdateTime;
    UAVObjEvent ev;

    dT = 0;
    lastVertVelocity = 0;
    timeval = 0;
    lastUpdateTime   = 0;
    // Force update of the settings
    SettingsUpdatedCb(&ev);
    // Failsafe handling
    uint32_t lastAltitudeHoldDesiredUpdate = 0;
    bool enterFailSafe = false;
    // Listen for updates.
    AltitudeHoldDesiredConnectQueue(queue);
    // PositionStateConnectQueue(queue);
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
                altitudeIntegral = 0;
            }

            // Todo: Add alarm if it should be running
            continue;
        } else if (ev.obj == FlightStatusHandle()) {
            FlightStatusFlightModeGet(&flightMode);
            altitudeHoldFlightMode = flightMode == FLIGHTSTATUS_FLIGHTMODE_ALTITUDEHOLD || flightMode == FLIGHTSTATUS_FLIGHTMODE_ALTITUDEVARIO;
            if (altitudeHoldFlightMode && !running) {
                AttitudeStateGet(&attitudeState);
                q[0] = attitudeState.q1;
                q[1] = attitudeState.q2;
                q[2] = attitudeState.q3;
                q[3] = attitudeState.q4;
                Quaternion2R(q, Rbe);
                // Copy the current throttle as a starting point for integral
                float initThrottle;
                StabilizationDesiredThrottleGet(&initThrottle);
                initThrottle *= Rbe[2][2]; // rotate into earth frame
                if (initThrottle > 1) {
                    initThrottle = 1;
                } else if (initThrottle < 0) {
                    initThrottle = 0;
                }
                error    = 0;
                altitudeHoldDesired.Velocity = 0;
                altitudeHoldDesired.Altitude = altHold.Altitude;
                altitudeIntegral = altHold.Altitude * altitudeHoldSettings.Kp + initThrottle;
                velocityIntegral = 0;
                running  = true;
            } else if (!altitudeHoldFlightMode) {
                running = false;
                lastAltitudeHoldDesiredUpdate = PIOS_DELAY_GetRaw();
            }
        } else if (ev.obj == VelocityStateHandle()) {
            init    = (init == WAITING_BARO) ? WAITIING_INIT : init;
            dT      = 0.1f * PIOS_DELAY_DiffuS(timeval) / 1.0e6f + 0.9f * dT;
            timeval = PIOS_DELAY_GetRaw();

            AltHoldSmoothedGet(&altHold);

            VelocityStateGet(&velocityData);

            altHold.Velocity = -(velAlpha * altHold.Velocity + (1 - velAlpha) * velocityData.Down);
            float vertAccel = (velocityData.Down - lastVertVelocity) / dT;

            lastVertVelocity = velocityData.Down;

            altHold.Accel    = -(accelAlpha * altHold.Accel + (1 - accelAlpha) * vertAccel);
            altHold.StateUpdateInterval = dT;

            PositionStateDownGet(&altHold.Altitude);
            altHold.Altitude = -altHold.Altitude;

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

            // Compute altitude and velocity integral
            altitudeIntegral += (altitudeHoldDesired.Altitude - altHold.Altitude - fblimit) * altitudeHoldSettings.AltitudeKi * dT;
            velocityIntegral += (altitudeHoldDesired.Velocity - altHold.Velocity - fblimit) * altitudeHoldSettings.VelocityKi * dT;

            thisTime = xTaskGetTickCount();
            // Only update stabilizationDesired less frequently
            if ((thisTime - lastUpdateTime) < 20) {
                continue;
            }
            altHold.ThrottleUpdateInterval = thisTime - lastUpdateTime;
            lastUpdateTime = thisTime;

            // Instead of explicit limit on integral you output limit feedback
            StabilizationDesiredGet(&stabilizationDesired);
            if (!enterFailSafe) {

                stabilizationDesired.Throttle = altitudeIntegral + velocityIntegral
                                                + error * fabsf(error) * altitudeHoldSettings.Kp2
                                                - altHold.Altitude * altitudeHoldSettings.Kp
                                                - altHold.Velocity * altitudeHoldSettings.Kd
                                                - altHold.Accel * altitudeHoldSettings.Ka;
                // scale up throttle to compensate for roll/pitch angle but limit this to 60 deg (cos(60) == 0.5) to prevent excessive scaling
                AttitudeStateGet(&attitudeState);
                q[0] = attitudeState.q1;
                q[1] = attitudeState.q2;
                q[2] = attitudeState.q3;
                q[3] = attitudeState.q4;
                Quaternion2R(q, Rbe);
                float throttlescale = Rbe[2][2] < 0.5f ? 0.5f : Rbe[2][2];
                stabilizationDesired.Throttle /= throttlescale;
                fblimit = 0;

                if (stabilizationDesired.Throttle > 1) {
                    fblimit = stabilizationDesired.Throttle - 1;
                    stabilizationDesired.Throttle = 1;
                } else if (stabilizationDesired.Throttle < 0) {
                    fblimit = stabilizationDesired.Throttle;
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
    accelAlpha = expf(-(1000.0f / 666.0f * ACCEL_DOWNSAMPLE) / altitudeHoldSettings.AccelTau);
    velAlpha   = expf(-(1000.0f / 666.0f * ACCEL_DOWNSAMPLE) / altitudeHoldSettings.VelocityTau);
}

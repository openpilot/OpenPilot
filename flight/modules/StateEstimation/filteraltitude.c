/**
 ******************************************************************************
 * @addtogroup OpenPilotModules OpenPilot Modules
 * @{
 * @addtogroup State Estimation
 * @brief Acquires sensor data and computes state estimate
 * @{
 *
 * @file       filteraltitude.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2013.
 * @brief      Barometric altitude filter, calculates vertical speed and true
 *             altitude based on Barometric altitude and accelerometers
 *
 * @see        The GNU Public License (GPL) Version 3
 *
 ******************************************************************************/
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

#include "inc/stateestimation.h"
#include <attitudestate.h>
#include <altitudefiltersettings.h>

#include <CoordinateConversions.h>

// Private constants

// Acceldrift ki value used at startup to let it quickly converge
// to its target value
#define INITIALIZATION_ACCELDRIFT_KI 0.2f
// lenght of accel bias initialization phase
#define INITIALIZATION_DURATION 5000

#define STACK_REQUIRED 128

#define DT_ALPHA       1e-2f
#define DT_MIN         1e-6f
#define DT_MAX         1.0f
#define DT_AVERAGE     1e-3f
static volatile bool reloadSettings;

// Private types
struct data {
    float altitudeState; // state = altitude,velocity,accel_offset,accel
    float velocityState;
    float accelBiasState;
    float accelState;
    float pos[3]; // position updates from other filters
    float vel[3]; // position updates from other filters

    PiOSDeltatimeConfig dt1config;
    PiOSDeltatimeConfig dt2config;
    float accelLast;
    float baroLast;
    bool  first_run;
    portTickType initTimer;
    AltitudeFilterSettingsData settings;
};

// Private variables

// Private functions

static int32_t init(stateFilter *self);
static int32_t filter(stateFilter *self, stateEstimation *state);
static void settingsUpdatedCb(UAVObjEvent *ev);


int32_t filterAltitudeInitialize(stateFilter *handle)
{
    handle->init      = &init;
    handle->filter    = &filter;
    handle->localdata = pvPortMalloc(sizeof(struct data));
    AttitudeStateInitialize();
    AltitudeFilterSettingsInitialize();
    AltitudeFilterSettingsConnectCallback(&settingsUpdatedCb);
    reloadSettings = true;
    return STACK_REQUIRED;
}

static int32_t init(stateFilter *self)
{
    struct data *this = (struct data *)self->localdata;

    this->altitudeState  = 0.0f;
    this->velocityState  = 0.0f;
    this->accelBiasState = 0.0f;
    this->accelState     = 0.0f;
    this->pos[0]    = 0.0f;
    this->pos[1]    = 0.0f;
    this->pos[2]    = 0.0f;
    this->vel[0]    = 0.0f;
    this->vel[1]    = 0.0f;
    this->vel[2]    = 0.0f;
    PIOS_DELTATIME_Init(&this->dt1config, DT_AVERAGE, DT_MIN, DT_MAX, DT_ALPHA);
    PIOS_DELTATIME_Init(&this->dt2config, DT_AVERAGE, DT_MIN, DT_MAX, DT_ALPHA);
    this->baroLast  = 0.0f;
    this->accelLast = 0.0f;
    this->first_run = 1;
    return 0;
}

static int32_t filter(stateFilter *self, stateEstimation *state)
{
    struct data *this = (struct data *)self->localdata;

    if (reloadSettings) {
        reloadSettings = false;
        AltitudeFilterSettingsGet(&this->settings);
    }

    if (this->first_run) {
        // Initialize to current altitude reading at initial location
        if (IS_SET(state->updated, SENSORUPDATES_baro)) {
            this->first_run = 0;
            this->initTimer = xTaskGetTickCount();
        }
    } else {
        // save existing position and velocity updates so GPS will still work
        if (IS_SET(state->updated, SENSORUPDATES_pos)) {
            this->pos[0]  = state->pos[0];
            this->pos[1]  = state->pos[1];
            this->pos[2]  = state->pos[2];
            state->pos[2] = -this->altitudeState;
        }
        if (IS_SET(state->updated, SENSORUPDATES_vel)) {
            this->vel[0]  = state->vel[0];
            this->vel[1]  = state->vel[1];
            this->vel[2]  = state->vel[2];
            state->vel[2] = -this->velocityState;
        }
        if (IS_SET(state->updated, SENSORUPDATES_accel)) {
            // rotate accels into global coordinate frame
            AttitudeStateData att;
            AttitudeStateGet(&att);
            float Rbe[3][3];
            Quaternion2R(&att.q1, Rbe);
            float current = -(Rbe[0][2] * state->accel[0] + Rbe[1][2] * state->accel[1] + Rbe[2][2] * state->accel[2] + 9.81f);

            // low pass filter accelerometers
            this->accelState = (1.0f - this->settings.AccelLowPassKp) * this->accelState + this->settings.AccelLowPassKp * current;
            if (((xTaskGetTickCount() - this->initTimer) / portTICK_RATE_MS) < INITIALIZATION_DURATION) {
                // allow the offset to reach quickly the target value in case of small AccelDriftKi
                this->accelBiasState = (1.0f - INITIALIZATION_ACCELDRIFT_KI) * this->accelBiasState + INITIALIZATION_ACCELDRIFT_KI * this->accelState;
            } else {
                // correct accel offset (low pass zeroing)
                this->accelBiasState = (1.0f - this->settings.AccelDriftKi) * this->accelBiasState + this->settings.AccelDriftKi * this->accelState;
            }
            // correct velocity and position state (integration)
            // low pass for average dT, compensate timing jitter from scheduler
            //
            float dT = PIOS_DELTATIME_GetAverageSeconds(&this->dt1config);
            float speedLast = this->velocityState;

            this->velocityState += 0.5f * (this->accelLast + (this->accelState - this->accelBiasState)) * dT;
            this->accelLast      = this->accelState - this->accelBiasState;

            this->altitudeState += 0.5f * (speedLast + this->velocityState) * dT;


            state->pos[0]   = this->pos[0];
            state->pos[1]   = this->pos[1];
            state->pos[2]   = -this->altitudeState;
            state->updated |= SENSORUPDATES_pos;

            state->vel[0]   = this->vel[0];
            state->vel[1]   = this->vel[1];
            state->vel[2]   = -this->velocityState;
            state->updated |= SENSORUPDATES_vel;
        }
        if (IS_SET(state->updated, SENSORUPDATES_baro)) {
            // correct the altitude state (simple low pass)
            this->altitudeState = (1.0f - this->settings.BaroKp) * this->altitudeState + this->settings.BaroKp * state->baro[0];

            // correct the velocity state (low pass differentiation)
            // low pass for average dT, compensate timing jitter from scheduler
            float dT = PIOS_DELTATIME_GetAverageSeconds(&this->dt2config);
            this->velocityState = (1.0f - (this->settings.BaroKp * this->settings.BaroKp)) * this->velocityState + (this->settings.BaroKp * this->settings.BaroKp) * (state->baro[0] - this->baroLast) / dT;
            this->baroLast  = state->baro[0];

            state->pos[0]   = this->pos[0];
            state->pos[1]   = this->pos[1];
            state->pos[2]   = -this->altitudeState;
            state->updated |= SENSORUPDATES_pos;

            state->vel[0]   = this->vel[0];
            state->vel[1]   = this->vel[1];
            state->vel[2]   = -this->velocityState;
            state->updated |= SENSORUPDATES_vel;
        }
    }

    return 0;
}

void settingsUpdatedCb(UAVObjEvent *ev)
{
    if (ev->obj == AltitudeFilterSettingsHandle()) {
        reloadSettings = true;
    }
}

/**
 * @}
 * @}
 */

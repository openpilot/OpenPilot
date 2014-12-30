/**
 ******************************************************************************
 * @addtogroup OpenPilotModules OpenPilot Modules
 * @{
 * @addtogroup State Estimation
 * @brief Acquires sensor data and computes state estimate
 * @{
 *
 * @file       filtervelocity.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2013.
 * @brief      Barometric altitude filter, calculates altitude offset based on
 *             GPS altitude offset if available
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

#include <revosettings.h>

// Private constants

#define STACK_REQUIRED 128
#define DT_ALPHA       1e-3f
#define DT_MIN         1e-6f
#define DT_MAX         1.0f
#define DT_INIT        (1.0f / PIOS_SENSOR_RATE) // initialize with board sensor rate

// Private types
struct data {
    float   vel[3];
    float   velocityBias[3];
    float   oldPos[3];
    float   alpha;
    uint8_t inited;
    PiOSDeltatimeConfig dtconfig;
};

// Private variables

// Private functions

static int32_t init(stateFilter *self);
static filterResult filter(stateFilter *self, stateEstimation *state);


int32_t filterVelocityInitialize(stateFilter *handle)
{
    handle->init      = &init;
    handle->filter    = &filter;
    handle->localdata = pios_malloc(sizeof(struct data));
    return STACK_REQUIRED;
}

static int32_t init(stateFilter *self)
{
    struct data *this = (struct data *)self->localdata;

    this->vel[0] = 0.0f;
    this->vel[1] = 0.0f;
    this->vel[2] = 0.0f;
    this->velocityBias[0] = 0.0f;
    this->velocityBias[1] = 0.0f;
    this->velocityBias[2] = 0.0f;
    this->oldPos[0] = 0.0f;
    this->oldPos[1] = 0.0f;
    this->oldPos[2] = 0.0f;
    this->inited    = 0;

    RevoSettingsInitialize();
    RevoSettingsVelocityPostProcessingLowPassAlphaGet(&this->alpha);

    PIOS_DELTATIME_Init(&this->dtconfig, DT_INIT, DT_MIN, DT_MAX, DT_ALPHA);

    return 0;
}

static filterResult filter(stateFilter *self, stateEstimation *state)
{
    struct data *this = (struct data *)self->localdata;


    if (IS_SET(state->updated, SENSORUPDATES_pos)) {
        float dT;
        dT = PIOS_DELTATIME_GetAverageSeconds(&this->dtconfig);

        // position updates allow us to calculate an ad-hoc velocity estimate
        // it will be very noisy and likely quite wrong at every given point in time,
        // but its long term average error should be quite low

        if (this->inited >= 1) { // only calculate velocity estimate if previous position is known
            this->vel[0] = (state->pos[0] - this->oldPos[0]) / dT;
            this->vel[1] = (state->pos[1] - this->oldPos[1]) / dT;
            this->vel[2] = (state->pos[2] - this->oldPos[2]) / dT;
            this->inited = 2;
        } else {
            // mark previous position as known
            this->inited = 1;
        }
        this->oldPos[0] = state->pos[0];
        this->oldPos[1] = state->pos[1];
        this->oldPos[2] = state->pos[2];
    }
    if (IS_SET(state->updated, SENSORUPDATES_vel)) {
        // if we have both a velocity estimate from filter and a velocity estimate from postion
        // we can calculate a bias estimate. It must be heavily low pass filtered to become useful
        // this assumes that the bias is relatively constant over time
        if (this->inited >= 2) { // only calculate bias if velocity estimate is calculated
            this->velocityBias[0] *= this->alpha;
            this->velocityBias[0] += (1.0f - this->alpha) * (this->vel[0] - state->vel[0]);
            this->velocityBias[1] *= this->alpha;
            this->velocityBias[1] += (1.0f - this->alpha) * (this->vel[1] - state->vel[1]);
            this->velocityBias[2] *= this->alpha;
            this->velocityBias[2] += (1.0f - this->alpha) * (this->vel[2] - state->vel[2]);
        }
        state->vel[0] += this->velocityBias[0];
        state->vel[1] += this->velocityBias[1];
        state->vel[2] += this->velocityBias[2];
    }
    return FILTERRESULT_OK;
}


/**
 * @}
 * @}
 */

/**
 ******************************************************************************
 * @addtogroup OpenPilotModules OpenPilot Modules
 * @{
 * @addtogroup State Estimation
 * @brief Acquires sensor data and computes state estimate
 * @{
 *
 * @file       filterbaro.c
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

// Private constants

#define STACK_REQUIRED 64

// low pass filter configuration to calculate offset
// of barometric altitude sensor
// reasoning: updates at: 10 Hz, tau= 300 s settle time
// exp(-(1/f) / tau ) ~=~ 0.9997
#define BARO_OFFSET_LOWPASS_ALPHA 0.9997f

// Private types
struct data {
    float baroOffset;
    float baroAlt;
    bool  first_run;
};

// Private variables

// Private functions

static int32_t init(stateFilter *self);
static int32_t filter(stateFilter *self, stateEstimation *state);


int32_t filterBaroInitialize(stateFilter *handle)
{
    handle->init      = &init;
    handle->filter    = &filter;
    handle->localdata = pvPortMalloc(sizeof(struct data));
    return STACK_REQUIRED;
}

static int32_t init(stateFilter *self)
{
    struct data *this = (struct data *)self->localdata;

    this->baroOffset = 0.0f;
    this->first_run  = 1;
    return 0;
}

static int32_t filter(stateFilter *self, stateEstimation *state)
{
    struct data *this = (struct data *)self->localdata;

    if (this->first_run) {
        // Initialize to current altitude reading at initial location
        if (IS_SET(state->updated, SENSORUPDATES_baro)) {
            this->first_run  = 0;
            this->baroOffset = state->baro[0];
            this->baroAlt    = state->baro[0];
        }
    } else {
        // Track barometric altitude offset with a low pass filter
        // based on GPS altitude if available
        if (IS_SET(state->updated, SENSORUPDATES_pos)) {
            this->baroOffset = BARO_OFFSET_LOWPASS_ALPHA * this->baroOffset +
                               (1.0f - BARO_OFFSET_LOWPASS_ALPHA)
                               * (this->baroAlt + state->pos[2]);
        }
        // calculate bias corrected altitude
        if (IS_SET(state->updated, SENSORUPDATES_baro)) {
            this->baroAlt   = state->baro[0];
            state->baro[0] -= this->baroOffset;
        }
    }

    return 0;
}


/**
 * @}
 * @}
 */

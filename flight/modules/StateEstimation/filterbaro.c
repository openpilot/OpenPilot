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

#include <revosettings.h>

// Private constants

#define STACK_REQUIRED 64

// Private types
struct data {
    float   baroOffset;
    float   baroGPSOffsetCorrectionAlpha;
    float   baroAlt;
    int16_t first_run;
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
    this->first_run  = 100;

    RevoSettingsInitialize();
    RevoSettingsBaroGPSOffsetCorrectionAlphaGet(&this->baroGPSOffsetCorrectionAlpha);

    return 0;
}

static int32_t filter(stateFilter *self, stateEstimation *state)
{
    struct data *this = (struct data *)self->localdata;

    if (this->first_run) {
        // Initialize to current altitude reading at initial location
        if (IS_SET(state->updated, SENSORUPDATES_baro)) {
            this->baroOffset = (100.f - this->first_run) / 100.f * this->baroOffset + (this->first_run / 100.f) * state->baro[0];
            this->baroAlt    = this->baroOffset;
            this->first_run--;
            UNSET_MASK(state->updated, SENSORUPDATES_baro);
        }
    } else {
        // Track barometric altitude offset with a low pass filter
        // based on GPS altitude if available
        if (IS_SET(state->updated, SENSORUPDATES_pos)) {
            this->baroOffset = this->baroOffset * this->baroGPSOffsetCorrectionAlpha +
                               (1.0f - this->baroGPSOffsetCorrectionAlpha) * (this->baroAlt + state->pos[2]);
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

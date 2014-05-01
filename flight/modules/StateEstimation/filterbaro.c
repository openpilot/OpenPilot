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

#define STACK_REQUIRED 128
#define INIT_CYCLES    100

// Private types
struct data {
    float   baroOffset;
    float   baroGPSOffsetCorrectionAlpha;
    float   baroAlt;
    float   gpsAlt;
    int16_t first_run;
    bool    useGPS;
};

// Private variables

// Private functions

static int32_t initwithgps(stateFilter *self);
static int32_t initwithoutgps(stateFilter *self);
static int32_t maininit(stateFilter *self);
static int32_t filter(stateFilter *self, stateEstimation *state);


int32_t filterBaroInitialize(stateFilter *handle)
{
    handle->init      = &initwithgps;
    handle->filter    = &filter;
    handle->localdata = pvPortMalloc(sizeof(struct data));
    return STACK_REQUIRED;
}

int32_t filterBaroiInitialize(stateFilter *handle)
{
    handle->init      = &initwithoutgps;
    handle->filter    = &filter;
    handle->localdata = pvPortMalloc(sizeof(struct data));
    return STACK_REQUIRED;
}

static int32_t initwithgps(stateFilter *self)
{
    struct data *this = (struct data *)self->localdata;

    this->useGPS = 1;
    return maininit(self);
}

static int32_t initwithoutgps(stateFilter *self)
{
    struct data *this = (struct data *)self->localdata;

    this->useGPS = 0;
    return maininit(self);
}

static int32_t maininit(stateFilter *self)
{
    struct data *this = (struct data *)self->localdata;

    this->baroOffset = 0.0f;
    this->gpsAlt     = 0.0f;
    this->first_run  = INIT_CYCLES;

    RevoSettingsInitialize();
    RevoSettingsBaroGPSOffsetCorrectionAlphaGet(&this->baroGPSOffsetCorrectionAlpha);

    return 0;
}

static int32_t filter(stateFilter *self, stateEstimation *state)
{
    struct data *this = (struct data *)self->localdata;

    if (this->first_run) {
        // Make sure initial location is initialized properly before continuing
        if (this->useGPS && IS_SET(state->updated, SENSORUPDATES_pos)) {
            if (this->first_run == INIT_CYCLES) {
                this->gpsAlt = state->pos[2];
                this->first_run--;
            }
        }
        // Initialize to current altitude reading at initial location
        if (IS_SET(state->updated, SENSORUPDATES_baro)) {
            if (this->first_run < INIT_CYCLES || !this->useGPS) {
                this->baroOffset = (((float)(INIT_CYCLES)-this->first_run) / (float)(INIT_CYCLES)) * this->baroOffset + (this->first_run / (float)(INIT_CYCLES)) * (state->baro[0] + this->gpsAlt);
                this->baroAlt    = state->baro[0];
                this->first_run--;
            }
            UNSET_MASK(state->updated, SENSORUPDATES_baro);
        }
        // make sure we raise an error until properly initialized - would not be good if people arm and
        // use altitudehold without initialized barometer filter
        return 2;
    } else {
        // Track barometric altitude offset with a low pass filter
        // based on GPS altitude if available
        if (this->useGPS && IS_SET(state->updated, SENSORUPDATES_pos)) {
            this->baroOffset = this->baroOffset * this->baroGPSOffsetCorrectionAlpha +
                               (1.0f - this->baroGPSOffsetCorrectionAlpha) * (this->baroAlt + state->pos[2]);
        }
        // calculate bias corrected altitude
        if (IS_SET(state->updated, SENSORUPDATES_baro)) {
            this->baroAlt   = state->baro[0];
            state->baro[0] -= this->baroOffset;
        }
        return 0;
    }
}


/**
 * @}
 * @}
 */

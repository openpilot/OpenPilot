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

// low pass filter configuration to calculate offset
// of barometric altitude sensor
// reasoning: updates at: 10 Hz, tau= 300 s settle time
// exp(-(1/f) / tau ) ~=~ 0.9997
#define BARO_OFFSET_LOWPASS_ALPHA 0.9997f

// Private types

// Private variables
static float baroOffset = 0.0f;
static bool first_run   = 1;

// Private functions

static int32_t init(void);
static int32_t filter(stateEstimation *state);


void filterBaroInitialize(stateFilter *handle)
{
    handle->init   = &init;
    handle->filter = &filter;
}

static int32_t init(void)
{
    baroOffset = 0.0f;
    first_run  = 1;
    return 0;
}

static int32_t filter(stateEstimation *state)
{
    if (first_run) {
        // Initialize to current altitude reading at initial location
        if (ISSET(state->updated, bar_UPDATED)) {
            first_run  = 0;
            baroOffset = state->bar[0];
        }
    } else {
        // Track barometric altitude offset with a low pass filter
        // based on GPS altitude if available
        if (ISSET(state->updated, pos_UPDATED)) {
            baroOffset = BARO_OFFSET_LOWPASS_ALPHA * baroOffset +
                         (1.0f - BARO_OFFSET_LOWPASS_ALPHA)
                         * (-state->pos[2] - state->bar[0]);
        }
        // calculate bias corrected altitude
        if (ISSET(state->updated, bar_UPDATED)) {
            state->bar[0] -= baroOffset;
        }
    }

    return 0;
}


/**
 * @}
 * @}
 */

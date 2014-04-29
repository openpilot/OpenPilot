/**
 ******************************************************************************
 * @addtogroup OpenPilotModules OpenPilot Modules
 * @{
 * @addtogroup State Estimation
 * @brief Acquires sensor data and computes state estimate
 * @{
 *
 * @file       filtermag.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2013.
 * @brief      Magnetometer drift compensation, uses previous cycles
 *             AttitudeState for estimation
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
#include <revocalibration.h>
#include <homelocation.h>

#include <CoordinateConversions.h>

// Private constants
//
#define STACK_REQUIRED 256

// Private types
struct data {
    HomeLocationData    homeLocation;
    RevoCalibrationData revoCalibration;
    float magBias[3];
};

// Private variables

// Private functions

static int32_t init(stateFilter *self);
static filterResult filter(stateFilter *self, stateEstimation *state);
static void magOffsetEstimation(struct data *this, float mag[3]);


int32_t filterMagInitialize(stateFilter *handle)
{
    handle->init      = &init;
    handle->filter    = &filter;
    handle->localdata = pvPortMalloc(sizeof(struct data));
    HomeLocationInitialize();
    return STACK_REQUIRED;
}

static int32_t init(stateFilter *self)
{
    struct data *this = (struct data *)self->localdata;

    this->magBias[0] = this->magBias[1] = this->magBias[2] = 0.0f;
    HomeLocationGet(&this->homeLocation);
    RevoCalibrationGet(&this->revoCalibration);
    return 0;
}

static filterResult filter(stateFilter *self, stateEstimation *state)
{
    struct data *this = (struct data *)self->localdata;

    if (IS_SET(state->updated, SENSORUPDATES_mag)) {
        if (this->revoCalibration.MagBiasNullingRate > 0) {
            magOffsetEstimation(this, state->mag);
        }
    }

    return FILTERRESULT_OK;
}

/**
 * Perform an update of the @ref MagBias based on
 * Magmeter Offset Cancellation: Theory and Implementation,
 * revisited William Premerlani, October 14, 2011
 */
static void magOffsetEstimation(struct data *this, float mag[3])
{
#if 0
    // Constants, to possibly go into a UAVO
    static const float MIN_NORM_DIFFERENCE = 50;

    static float B2[3] = { 0, 0, 0 };

    MagBiasData magBias;
    MagBiasGet(&magBias);

    // Remove the current estimate of the bias
    mag->x -= magBias.x;
    mag->y -= magBias.y;
    mag->z -= magBias.z;

    // First call
    if (B2[0] == 0 && B2[1] == 0 && B2[2] == 0) {
        B2[0] = mag->x;
        B2[1] = mag->y;
        B2[2] = mag->z;
        return;
    }

    float B1[3]     = { mag->x, mag->y, mag->z };
    float norm_diff = sqrtf(powf(B2[0] - B1[0], 2) + powf(B2[1] - B1[1], 2) + powf(B2[2] - B1[2], 2));
    if (norm_diff > MIN_NORM_DIFFERENCE) {
        float norm_b1    = sqrtf(B1[0] * B1[0] + B1[1] * B1[1] + B1[2] * B1[2]);
        float norm_b2    = sqrtf(B2[0] * B2[0] + B2[1] * B2[1] + B2[2] * B2[2]);
        float scale      = cal.MagBiasNullingRate * (norm_b2 - norm_b1) / norm_diff;
        float b_error[3] = { (B2[0] - B1[0]) * scale, (B2[1] - B1[1]) * scale, (B2[2] - B1[2]) * scale };

        magBias.x += b_error[0];
        magBias.y += b_error[1];
        magBias.z += b_error[2];

        MagBiasSet(&magBias);

        // Store this value to compare against next update
        B2[0] = B1[0]; B2[1] = B1[1]; B2[2] = B1[2];
    }
#else // if 0

    const float Rxy  = sqrtf(this->homeLocation.Be[0] * this->homeLocation.Be[0] + this->homeLocation.Be[1] * this->homeLocation.Be[1]);
    const float Rz   = this->homeLocation.Be[2];

    const float rate = this->revoCalibration.MagBiasNullingRate;
    float Rot[3][3];
    float B_e[3];
    float xy[2];
    float delta[3];

    AttitudeStateData attitude;
    AttitudeStateGet(&attitude);

    // Get the rotation matrix
    Quaternion2R(&attitude.q1, Rot);

    // Rotate the mag into the NED frame
    B_e[0] = Rot[0][0] * mag[0] + Rot[1][0] * mag[1] + Rot[2][0] * mag[2];
    B_e[1] = Rot[0][1] * mag[0] + Rot[1][1] * mag[1] + Rot[2][1] * mag[2];
    B_e[2] = Rot[0][2] * mag[0] + Rot[1][2] * mag[1] + Rot[2][2] * mag[2];

    float cy = cosf(DEG2RAD(attitude.Yaw));
    float sy = sinf(DEG2RAD(attitude.Yaw));

    xy[0] = cy * B_e[0] + sy * B_e[1];
    xy[1] = -sy * B_e[0] + cy * B_e[1];

    float xy_norm = sqrtf(xy[0] * xy[0] + xy[1] * xy[1]);

    delta[0] = -rate * (xy[0] / xy_norm * Rxy - xy[0]);
    delta[1] = -rate * (xy[1] / xy_norm * Rxy - xy[1]);
    delta[2] = -rate * (Rz - B_e[2]);

    if (!isnan(delta[0]) && !isinf(delta[0]) &&
        !isnan(delta[1]) && !isinf(delta[1]) &&
        !isnan(delta[2]) && !isinf(delta[2])) {
        this->magBias[0] += delta[0];
        this->magBias[1] += delta[1];
        this->magBias[2] += delta[2];
    }

    // Add bias to state estimation
    mag[0] += this->magBias[0];
    mag[1] += this->magBias[1];
    mag[2] += this->magBias[2];

#endif // if 0
}


/**
 * @}
 * @}
 */

/**
 ******************************************************************************
 * @addtogroup OpenPilotModules OpenPilot Modules
 * @{
 * @addtogroup State Estimation
 * @brief Acquires sensor data and computes state estimate
 * @{
 *
 * @file       filtercf.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2013.
 * @brief      Complementary filter to calculate Attitude from Accels and Gyros
 *             and optionally magnetometers:
 *             WARNING: Will drift if the mean acceleration force doesn't point
 *             to ground, unsafe on fixed wing, since position hold is
 *             implemented as continuous circular flying!
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

#include <attitudesettings.h>
#include <attitudestate.h>
#include <flightstatus.h>
#include <homelocation.h>

#include <CoordinateConversions.h>

// Private constants


// Private types

// Private variables
static AttitudeSettingsData attitudeSettings;
static FlightStatusData flightStatus;
static HomeLocationData homeLocation;

static bool first_run = 1;
static bool useMag    = 0;
static float currentAccel[3];
static float currentMag[3];
static float gyroBias[3];
static bool accelUpdated = 0;
static bool magUpdated   = 0;


static float accel_alpha = 0;
static bool accel_filter_enabled = false;

// Private functions

static int32_t initwithmag(void);
static int32_t initwithoutmag(void);
static int32_t maininit(void);
static int32_t filter(stateEstimation *state);
static int32_t complementaryFilter(float gyro[3], float accel[3], float mag[3], float q[4]);

static void flightStatusUpdatedCb(UAVObjEvent *ev);


void filterCFInitialize(stateFilter *handle)
{
    handle->init   = &initwithoutmag;
    handle->filter = &filter;
    FlightStatusConnectCallback(&flightStatusUpdatedCb);
    HomeLocationConnectCallback(&flightStatusUpdatedCb);
    flightStatusUpdatedCb(NULL);
}

void filterCFMInitialize(stateFilter *handle)
{
    handle->init   = &initwithmag;
    handle->filter = &filter;
}

static int32_t initwithmag(void)
{
    useMag = 1;
    return maininit();
}

static int32_t initwithoutmag(void)
{
    useMag = 0;
    return maininit();
}

static int32_t maininit(void)
{
    first_run    = 1;
    accelUpdated = 0;
    AttitudeSettingsGet(&attitudeSettings);

    const float fakeDt = 0.0025f;
    if (attitudeSettings.AccelTau < 0.0001f) {
        accel_alpha = 0; // not trusting this to resolve to 0
        accel_filter_enabled = false;
    } else {
        accel_alpha = expf(-fakeDt / attitudeSettings.AccelTau);
        accel_filter_enabled = true;
    }

    // reset gyro Bias
    gyroBias[0] = 0.0f;
    gyroBias[1] = 0.0f;
    gyroBias[2] = 0.0f;

    return 0;
}

/**
 * Collect all required state variables, then run complementary filter
 */
static int32_t filter(stateEstimation *state)
{
    int32_t result = 0;

    if (ISSET(state->updated, mag_UPDATED)) {
        magUpdated    = 1;
        currentMag[0] = state->mag[0];
        currentMag[1] = state->mag[1];
        currentMag[2] = state->mag[2];
    }
    if (ISSET(state->updated, acc_UPDATED)) {
        accelUpdated    = 1;
        currentAccel[0] = state->acc[0];
        currentAccel[1] = state->acc[1];
        currentAccel[2] = state->acc[2];
    }
    if (ISSET(state->updated, gyr_UPDATED)) {
        if (accelUpdated) {
            float att[4];
            result = complementaryFilter(state->gyr, currentAccel, currentMag, att);
            if (!result) {
                state->att[0]   = att[0];
                state->att[1]   = att[1];
                state->att[2]   = att[2];
                state->att[3]   = att[3];
                state->updated |= att_UPDATED;
            }
            accelUpdated = 0;
            magUpdated   = 0;
        }
        state->gyr[0] -= gyroBias[0];
        state->gyr[1] -= gyroBias[1];
        state->gyr[2] -= gyroBias[2];
    }
    return result;
}


static inline void apply_accel_filter(const float *raw, float *filtered)
{
    if (accel_filter_enabled) {
        filtered[0] = filtered[0] * accel_alpha + raw[0] * (1 - accel_alpha);
        filtered[1] = filtered[1] * accel_alpha + raw[1] * (1 - accel_alpha);
        filtered[2] = filtered[2] * accel_alpha + raw[2] * (1 - accel_alpha);
    } else {
        filtered[0] = raw[0];
        filtered[1] = raw[1];
        filtered[2] = raw[2];
    }
}

static int32_t complementaryFilter(float gyro[3], float accel[3], float mag[3], float q[4])
{
    static int32_t timeval;
    float dT;
    static uint8_t init = 0;
    float magKp = 0.0f; // TODO: make this non hardcoded at some point
    float magKi = 0.000001f;

    // During initialization and
    if (first_run) {
#if defined(PIOS_INCLUDE_HMC5883)
        // wait until mags have been updated
        if (!mag_updated) {
            return 1;
        }
#else
        mag[0] = 100.0f;
        mag[1] = 0.0f;
        mag[2] = 0.0f;
#endif
        AttitudeStateData attitudeState; // base on previous state
        AttitudeStateGet(&attitudeState);
        init = 0;

        // Set initial attitude. Use accels to determine roll and pitch, rotate magnetic measurement accordingly,
        // so pseudo "north" vector can be estimated even if the board is not level
        attitudeState.Roll = atan2f(-accel[2], -accel[3]);
        float zn  = cosf(attitudeState.Roll) * mag[2] + sinf(attitudeState.Roll) * mag[1];
        float yn  = cosf(attitudeState.Roll) * mag[1] - sinf(attitudeState.Roll) * mag[2];

        // rotate accels z vector according to roll
        float azn = cosf(attitudeState.Roll) * accel[2] + sinf(attitudeState.Roll) * accel[1];
        attitudeState.Pitch = atan2f(accel[0], -azn);

        float xn  = cosf(attitudeState.Pitch) * mag[0] + sinf(attitudeState.Pitch) * zn;

        attitudeState.Yaw = atan2f(-yn, xn);
        // TODO: This is still a hack
        // Put this in a proper generic function in CoordinateConversion.c
        // should take 4 vectors: g (0,0,-9.81), accels, Be (or 1,0,0 if no home loc) and magnetometers (or 1,0,0 if no mags)
        // should calculate the rotation in 3d space using proper cross product math
        // SUBTODO: formulate the math required

        attitudeState.Roll  = RAD2DEG(attitudeState.Roll);
        attitudeState.Pitch = RAD2DEG(attitudeState.Pitch);
        attitudeState.Yaw   = RAD2DEG(attitudeState.Yaw);

        RPY2Quaternion(&attitudeState.Roll, &attitudeState.q1);
        q[0]    = attitudeState.q1;
        q[1]    = attitudeState.q1;
        q[2]    = attitudeState.q1;
        q[3]    = attitudeState.q1;

        timeval = PIOS_DELAY_GetRaw();

        return 0;
    }

    if ((init == 0 && xTaskGetTickCount() < 7000) && (xTaskGetTickCount() > 1000)) {
        // For first 7 seconds use accels to get gyro bias
        attitudeSettings.AccelKp     = 1.0f;
        attitudeSettings.AccelKi     = 0.9f;
        attitudeSettings.YawBiasRate = 0.23f;
        magKp = 1.0f;
    } else if ((attitudeSettings.ZeroDuringArming == ATTITUDESETTINGS_ZERODURINGARMING_TRUE) && (flightStatus.Armed == FLIGHTSTATUS_ARMED_ARMING)) {
        attitudeSettings.AccelKp     = 1.0f;
        attitudeSettings.AccelKi     = 0.9f;
        attitudeSettings.YawBiasRate = 0.23f;
        magKp = 1.0f;
        init = 0;
    } else if (init == 0) {
        // Reload settings (all the rates)
        AttitudeSettingsGet(&attitudeSettings);
        magKp = 0.01f;
        init  = 1;
    }

    // Compute the dT using the cpu clock
    dT = PIOS_DELAY_DiffuS(timeval) / 1000000.0f;
    timeval = PIOS_DELAY_GetRaw();
    if (dT < 0.001f) { // safe bounds
        dT = 0.001f;
    }

    AttitudeStateData attitudeState; // base on previous state
    AttitudeStateGet(&attitudeState);

    // Get the current attitude estimate
    quat_copy(&attitudeState.q1, q);

    float accels_filtered[3];
    // Apply smoothing to accel values, to reduce vibration noise before main calculations.
    apply_accel_filter(accel, accels_filtered);

    // Rotate gravity to body frame and cross with accels
    float grot[3];
    grot[0] = -(2.0f * (q[1] * q[3] - q[0] * q[2]));
    grot[1] = -(2.0f * (q[2] * q[3] + q[0] * q[1]));
    grot[2] = -(q[0] * q[0] - q[1] * q[1] - q[2] * q[2] + q[3] * q[3]);

    float grot_filtered[3];
    float accel_err[3];
    apply_accel_filter(grot, grot_filtered);
    CrossProduct((const float *)accels_filtered, (const float *)grot_filtered, accel_err);

    // Account for accel magnitude
    float accel_mag = sqrtf(accels_filtered[0] * accels_filtered[0] + accels_filtered[1] * accels_filtered[1] + accels_filtered[2] * accels_filtered[2]);
    if (accel_mag < 1.0e-3f) {
        return 2; // safety feature copied from CC
    }

    // Account for filtered gravity vector magnitude
    float grot_mag;
    if (accel_filter_enabled) {
        grot_mag = sqrtf(grot_filtered[0] * grot_filtered[0] + grot_filtered[1] * grot_filtered[1] + grot_filtered[2] * grot_filtered[2]);
    } else {
        grot_mag = 1.0f;
    }
    if (grot_mag < 1.0e-3f) {
        return 2;
    }

    accel_err[0] /= (accel_mag * grot_mag);
    accel_err[1] /= (accel_mag * grot_mag);
    accel_err[2] /= (accel_mag * grot_mag);

    float mag_err[3] = { 0.0f };
    if (magUpdated && useMag) {
        // Rotate gravity to body frame and cross with accels
        float brot[3];
        float Rbe[3][3];

        Quaternion2R(q, Rbe);

        rot_mult(Rbe, homeLocation.Be, brot);

        float mag_len = sqrtf(mag[0] * mag[0] + mag[1] * mag[1] + mag[2] * mag[2]);
        mag[0]  /= mag_len;
        mag[1]  /= mag_len;
        mag[2]  /= mag_len;

        float bmag = sqrtf(brot[0] * brot[0] + brot[1] * brot[1] + brot[2] * brot[2]);
        brot[0] /= bmag;
        brot[1] /= bmag;
        brot[2] /= bmag;

        // Only compute if neither vector is null
        if (bmag < 1.0f || mag_len < 1.0f) {
            mag_err[0] = mag_err[1] = mag_err[2] = 0.0f;
        } else {
            CrossProduct((const float *)mag, (const float *)brot, mag_err);
        }
    } else {
        mag_err[0] = mag_err[1] = mag_err[2] = 0.0f;
    }

    // Accumulate integral of error.  Scale here so that units are (deg/s) but Ki has units of s
    gyroBias[0] -= accel_err[0] * attitudeSettings.AccelKi;
    gyroBias[1] -= accel_err[1] * attitudeSettings.AccelKi;
    if (useMag) {
        gyroBias[2] -= mag_err[2] * magKi;
    }

    // Correct rates based on integral coefficient
    gyro[0] -= gyroBias[0];
    gyro[1] -= gyroBias[1];
    gyro[2] -= gyroBias[2];

    float gyrotmp[3] = { gyro[0], gyro[1], gyro[2] };
    // Correct rates based on proportional coefficient
    gyrotmp[0] += accel_err[0] * attitudeSettings.AccelKp / dT;
    gyrotmp[1] += accel_err[1] * attitudeSettings.AccelKp / dT;
    if (useMag) {
        gyrotmp[2] += accel_err[2] * attitudeSettings.AccelKp / dT + mag_err[2] * magKp / dT;
    } else {
        gyrotmp[2] += accel_err[2] * attitudeSettings.AccelKp / dT;
    }

    // Work out time derivative from INSAlgo writeup
    // Also accounts for the fact that gyros are in deg/s
    float qdot[4];
    qdot[0] = DEG2RAD(-q[1] * gyrotmp[0] - q[2] * gyrotmp[1] - q[3] * gyrotmp[2]) * dT / 2;
    qdot[1] = DEG2RAD(q[0] * gyrotmp[0] - q[3] * gyrotmp[1] + q[2] * gyrotmp[2]) * dT / 2;
    qdot[2] = DEG2RAD(q[3] * gyrotmp[0] + q[0] * gyrotmp[1] - q[1] * gyrotmp[2]) * dT / 2;
    qdot[3] = DEG2RAD(-q[2] * gyrotmp[0] + q[1] * gyrotmp[1] + q[0] * gyrotmp[2]) * dT / 2;

    // Take a time step
    q[0]    = q[0] + qdot[0];
    q[1]    = q[1] + qdot[1];
    q[2]    = q[2] + qdot[2];
    q[3]    = q[3] + qdot[3];

    if (q[0] < 0.0f) {
        q[0] = -q[0];
        q[1] = -q[1];
        q[2] = -q[2];
        q[3] = -q[3];
    }

    // Renomalize
    float qmag = sqrtf(q[0] * q[0] + q[1] * q[1] + q[2] * q[2] + q[3] * q[3]);
    q[0] = q[0] / qmag;
    q[1] = q[1] / qmag;
    q[2] = q[2] / qmag;
    q[3] = q[3] / qmag;

    // If quaternion has become inappropriately short or is nan reinit.
    // THIS SHOULD NEVER ACTUALLY HAPPEN
    if ((fabsf(qmag) < 1.0e-3f) || isnan(qmag)) {
        q[0] = 1.0f;
        q[1] = 0.0f;
        q[2] = 0.0f;
        q[3] = 0.0f;
        return 2;
    }

    return 0;
}

static void flightStatusUpdatedCb(UAVObjEvent *ev)
{
    FlightStatusGet(&flightStatus);
    HomeLocationGet(&homeLocation);
}

/**
 * @}
 * @}
 */

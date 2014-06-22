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
#include <revocalibration.h>

#include <CoordinateConversions.h>
#include <pios_notify.h>
// Private constants

#define STACK_REQUIRED          512

#define CALIBRATION_DELAY_MS    4000
#define CALIBRATION_DURATION_MS 6000

// Private types
struct data {
    AttitudeSettingsData attitudeSettings;
    HomeLocationData     homeLocation;
    bool    first_run;
    bool    useMag;
    float   currentAccel[3];
    float   currentMag[3];
    float   accels_filtered[3];
    float   grot_filtered[3];
    float   gyroBias[3];
    bool    accelUpdated;
    bool    magUpdated;
    float   accel_alpha;
    bool    accel_filter_enabled;
    float   rollPitchBiasRate;
    int32_t timeval;
    int32_t starttime;
    uint8_t init;
    bool    magCalibrated;
};

// Private variables
bool initialized = 0;
static FlightStatusData flightStatus;

// Private functions

static int32_t initwithmag(stateFilter *self);
static int32_t initwithoutmag(stateFilter *self);
static int32_t maininit(stateFilter *self);
static int32_t filter(stateFilter *self, stateEstimation *state);
static int32_t complementaryFilter(struct data *this, float gyro[3], float accel[3], float mag[3], float attitude[4]);

static void flightStatusUpdatedCb(UAVObjEvent *ev);

static void globalInit(void);


static void globalInit(void)
{
    if (!initialized) {
        initialized = 1;
        FlightStatusInitialize();
        HomeLocationInitialize();
        RevoCalibrationInitialize();
        FlightStatusConnectCallback(&flightStatusUpdatedCb);
        flightStatusUpdatedCb(NULL);
    }
}

int32_t filterCFInitialize(stateFilter *handle)
{
    globalInit();
    handle->init      = &initwithoutmag;
    handle->filter    = &filter;
    handle->localdata = pios_malloc(sizeof(struct data));
    return STACK_REQUIRED;
}

int32_t filterCFMInitialize(stateFilter *handle)
{
    globalInit();
    handle->init      = &initwithmag;
    handle->filter    = &filter;
    handle->localdata = pios_malloc(sizeof(struct data));
    return STACK_REQUIRED;
}

static int32_t initwithmag(stateFilter *self)
{
    struct data *this = (struct data *)self->localdata;

    this->useMag = 1;
    return maininit(self);
}

static int32_t initwithoutmag(stateFilter *self)
{
    struct data *this = (struct data *)self->localdata;

    this->useMag = 0;
    return maininit(self);
}

static int32_t maininit(stateFilter *self)
{
    struct data *this = (struct data *)self->localdata;

    this->first_run     = 1;
    this->accelUpdated  = 0;
    this->magCalibrated = true;
    AttitudeSettingsGet(&this->attitudeSettings);
    HomeLocationGet(&this->homeLocation);

    const float fakeDt = 0.0025f;
    if (this->attitudeSettings.AccelTau < 0.0001f) {
        this->accel_alpha = 0; // not trusting this to resolve to 0
        this->accel_filter_enabled = false;
    } else {
        this->accel_alpha = expf(-fakeDt / this->attitudeSettings.AccelTau);
        this->accel_filter_enabled = true;
    }

    // reset gyro Bias
    this->gyroBias[0] = 0.0f;
    this->gyroBias[1] = 0.0f;
    this->gyroBias[2] = 0.0f;

    return 0;
}

/**
 * Collect all required state variables, then run complementary filter
 */
static int32_t filter(stateFilter *self, stateEstimation *state)
{
    struct data *this = (struct data *)self->localdata;

    int32_t result    = 0;

    if (IS_SET(state->updated, SENSORUPDATES_mag)) {
        this->magUpdated    = 1;
        this->currentMag[0] = state->mag[0];
        this->currentMag[1] = state->mag[1];
        this->currentMag[2] = state->mag[2];
    }
    if (IS_SET(state->updated, SENSORUPDATES_accel)) {
        this->accelUpdated    = 1;
        this->currentAccel[0] = state->accel[0];
        this->currentAccel[1] = state->accel[1];
        this->currentAccel[2] = state->accel[2];
    }
    if (IS_SET(state->updated, SENSORUPDATES_gyro)) {
        if (this->accelUpdated) {
            float attitude[4];
            result = complementaryFilter(this, state->gyro, this->currentAccel, this->currentMag, attitude);
            if (!result) {
                state->attitude[0] = attitude[0];
                state->attitude[1] = attitude[1];
                state->attitude[2] = attitude[2];
                state->attitude[3] = attitude[3];
                state->updated    |= SENSORUPDATES_attitude;
            }
            this->accelUpdated = 0;
            this->magUpdated   = 0;
        }
    }
    return result;
}


static inline void apply_accel_filter(const struct data *this, const float *raw, float *filtered)
{
    if (this->accel_filter_enabled) {
        filtered[0] = filtered[0] * this->accel_alpha + raw[0] * (1 - this->accel_alpha);
        filtered[1] = filtered[1] * this->accel_alpha + raw[1] * (1 - this->accel_alpha);
        filtered[2] = filtered[2] * this->accel_alpha + raw[2] * (1 - this->accel_alpha);
    } else {
        filtered[0] = raw[0];
        filtered[1] = raw[1];
        filtered[2] = raw[2];
    }
}

static int32_t complementaryFilter(struct data *this, float gyro[3], float accel[3], float mag[3], float attitude[4])
{
    float dT;

    // During initialization and
    if (this->first_run) {
#if defined(PIOS_INCLUDE_HMC5883)
        // wait until mags have been updated
        if (!this->magUpdated) {
            return 1;
        }
#else
        mag[0] = 100.0f;
        mag[1] = 0.0f;
        mag[2] = 0.0f;
#endif
        float magBias[3];
        RevoCalibrationmag_biasArrayGet(magBias);
        // don't trust Mag for initial orientation if it has not been calibrated
        if (magBias[0] < 1e-6f && magBias[1] < 1e-6f && magBias[2] < 1e-6f) {
            this->magCalibrated = false;
            mag[0] = 100.0f;
            mag[1] = 0.0f;
            mag[2] = 0.0f;
        }

        AttitudeStateData attitudeState; // base on previous state
        AttitudeStateGet(&attitudeState);
        this->init = 0;

        // Set initial attitude. Use accels to determine roll and pitch, rotate magnetic measurement accordingly,
        // so pseudo "north" vector can be estimated even if the board is not level
        attitudeState.Roll = atan2f(-accel[1], -accel[2]);
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

        RPY2Quaternion(&attitudeState.Roll, attitude);

        this->first_run = 0;
        this->accels_filtered[0] = 0.0f;
        this->accels_filtered[1] = 0.0f;
        this->accels_filtered[2] = 0.0f;
        this->grot_filtered[0]   = 0.0f;
        this->grot_filtered[1]   = 0.0f;
        this->grot_filtered[2]   = 0.0f;
        this->timeval   = PIOS_DELAY_GetRaw(); // Cycle counter used for precise timing
        this->starttime = xTaskGetTickCount(); // Tick counter used for long time intervals

        return 0; // must return zero on initial initialization, so attitude will init with a valid quaternion
    }

    if (this->init == 0 && xTaskGetTickCount() - this->starttime < CALIBRATION_DELAY_MS / portTICK_RATE_MS) {
        // wait 4 seconds for the user to get his hands off in case the board was just powered
        this->timeval = PIOS_DELAY_GetRaw();
        return 1;
    } else if (this->init == 0 && xTaskGetTickCount() - this->starttime < (CALIBRATION_DELAY_MS + CALIBRATION_DURATION_MS) / portTICK_RATE_MS) {
        // For first 6 seconds use accels to get gyro bias
        this->attitudeSettings.AccelKp     = 1.0f;
        this->attitudeSettings.AccelKi     = 0.0f;
        this->attitudeSettings.YawBiasRate = 0.23f;
        this->accel_filter_enabled   = false;
        this->rollPitchBiasRate      = 0.01f;
        this->attitudeSettings.MagKp = this->magCalibrated ? 1.0f : 0.0f;
        PIOS_NOTIFY_StartNotification(NOTIFY_DRAW_ATTENTION, NOTIFY_PRIORITY_REGULAR);
    } else if ((this->attitudeSettings.ZeroDuringArming == ATTITUDESETTINGS_ZERODURINGARMING_TRUE) && (flightStatus.Armed == FLIGHTSTATUS_ARMED_ARMING)) {
        this->attitudeSettings.AccelKp     = 1.0f;
        this->attitudeSettings.AccelKi     = 0.0f;
        this->attitudeSettings.YawBiasRate = 0.23f;
        this->accel_filter_enabled   = false;
        this->rollPitchBiasRate      = 0.01f;
        this->attitudeSettings.MagKp = this->magCalibrated ? 1.0f : 0.0f;
        this->init = 0;
        PIOS_NOTIFY_StartNotification(NOTIFY_DRAW_ATTENTION, NOTIFY_PRIORITY_REGULAR);
    } else if (this->init == 0) {
        // Reload settings (all the rates)
        AttitudeSettingsGet(&this->attitudeSettings);
        this->rollPitchBiasRate = 0.0f;
        if (this->accel_alpha > 0.0f) {
            this->accel_filter_enabled = true;
        }
        this->init = 1;
    }

    // Compute the dT using the cpu clock
    dT = PIOS_DELAY_DiffuS(this->timeval) / 1000000.0f;
    this->timeval = PIOS_DELAY_GetRaw();
    if (dT < 0.001f) { // safe bounds
        dT = 0.001f;
    }

    AttitudeStateData attitudeState; // base on previous state
    AttitudeStateGet(&attitudeState);

    // Get the current attitude estimate
    quat_copy(&attitudeState.q1, attitude);

    // Apply smoothing to accel values, to reduce vibration noise before main calculations.
    apply_accel_filter(this, accel, this->accels_filtered);

    // Rotate gravity to body frame and cross with accels
    float grot[3];
    grot[0] = -(2.0f * (attitude[1] * attitude[3] - attitude[0] * attitude[2]));
    grot[1] = -(2.0f * (attitude[2] * attitude[3] + attitude[0] * attitude[1]));
    grot[2] = -(attitude[0] * attitude[0] - attitude[1] * attitude[1] - attitude[2] * attitude[2] + attitude[3] * attitude[3]);

    float accel_err[3];
    apply_accel_filter(this, grot, this->grot_filtered);

    CrossProduct((const float *)this->accels_filtered, (const float *)this->grot_filtered, accel_err);

    // Account for accel magnitude
    float accel_mag = sqrtf(this->accels_filtered[0] * this->accels_filtered[0] + this->accels_filtered[1] * this->accels_filtered[1] + this->accels_filtered[2] * this->accels_filtered[2]);
    if (accel_mag < 1.0e-3f) {
        return 2; // safety feature copied from CC
    }

    // Account for filtered gravity vector magnitude
    float grot_mag;
    if (this->accel_filter_enabled) {
        grot_mag = sqrtf(this->grot_filtered[0] * this->grot_filtered[0] + this->grot_filtered[1] * this->grot_filtered[1] + this->grot_filtered[2] * this->grot_filtered[2]);
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
    if (this->magUpdated && this->useMag) {
        // Rotate gravity to body frame and cross with accels
        float brot[3];
        float Rbe[3][3];

        Quaternion2R(attitude, Rbe);

        rot_mult(Rbe, this->homeLocation.Be, brot);

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

    // Correct rates based on integral coefficient
    gyro[0] -= this->gyroBias[0];
    gyro[1] -= this->gyroBias[1];
    gyro[2] -= this->gyroBias[2];

    // Accumulate integral of error.  Scale here so that units are (deg/s) but Ki has units of s
    this->gyroBias[0] -= accel_err[0] * this->attitudeSettings.AccelKi - gyro[0] * this->rollPitchBiasRate;
    this->gyroBias[1] -= accel_err[1] * this->attitudeSettings.AccelKi - gyro[1] * this->rollPitchBiasRate;
    if (this->useMag) {
        this->gyroBias[2] -= -mag_err[2] * this->attitudeSettings.MagKi - gyro[2] * this->rollPitchBiasRate;
    } else {
        this->gyroBias[2] -= -gyro[2] * this->rollPitchBiasRate;
    }

    float gyrotmp[3] = { gyro[0], gyro[1], gyro[2] };
    // Correct rates based on proportional coefficient
    gyrotmp[0] += accel_err[0] * this->attitudeSettings.AccelKp / dT;
    gyrotmp[1] += accel_err[1] * this->attitudeSettings.AccelKp / dT;
    if (this->useMag) {
        gyrotmp[2] += accel_err[2] * this->attitudeSettings.AccelKp / dT + mag_err[2] * this->attitudeSettings.MagKp / dT;
    } else {
        gyrotmp[2] += accel_err[2] * this->attitudeSettings.AccelKp / dT;
    }

    // Work out time derivative from INSAlgo writeup
    // Also accounts for the fact that gyros are in deg/s
    float qdot[4];
    qdot[0]     = DEG2RAD(-attitude[1] * gyrotmp[0] - attitude[2] * gyrotmp[1] - attitude[3] * gyrotmp[2]) * dT / 2;
    qdot[1]     = DEG2RAD(attitude[0] * gyrotmp[0] - attitude[3] * gyrotmp[1] + attitude[2] * gyrotmp[2]) * dT / 2;
    qdot[2]     = DEG2RAD(attitude[3] * gyrotmp[0] + attitude[0] * gyrotmp[1] - attitude[1] * gyrotmp[2]) * dT / 2;
    qdot[3]     = DEG2RAD(-attitude[2] * gyrotmp[0] + attitude[1] * gyrotmp[1] + attitude[0] * gyrotmp[2]) * dT / 2;

    // Take a time step
    attitude[0] = attitude[0] + qdot[0];
    attitude[1] = attitude[1] + qdot[1];
    attitude[2] = attitude[2] + qdot[2];
    attitude[3] = attitude[3] + qdot[3];

    if (attitude[0] < 0.0f) {
        attitude[0] = -attitude[0];
        attitude[1] = -attitude[1];
        attitude[2] = -attitude[2];
        attitude[3] = -attitude[3];
    }

    // Renomalize
    float qmag = sqrtf(attitude[0] * attitude[0] + attitude[1] * attitude[1] + attitude[2] * attitude[2] + attitude[3] * attitude[3]);
    attitude[0] = attitude[0] / qmag;
    attitude[1] = attitude[1] / qmag;
    attitude[2] = attitude[2] / qmag;
    attitude[3] = attitude[3] / qmag;

    // If quaternion has become inappropriately short or is nan reinit.
    // THIS SHOULD NEVER ACTUALLY HAPPEN
    if ((fabsf(qmag) < 1.0e-3f) || isnan(qmag)) {
        this->first_run = 1;
        return 2;
    }

    if (this->init) {
        return 0;
    } else {
        return 2; // return "critical" for now, so users can see the zeroing period, switch to more graceful notification later
    }
}

static void flightStatusUpdatedCb(__attribute__((unused)) UAVObjEvent *ev)
{
    FlightStatusGet(&flightStatus);
}

/**
 * @}
 * @}
 */

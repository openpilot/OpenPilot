/**
 ******************************************************************************
 * @addtogroup OpenPilotModules OpenPilot Modules
 * @{
 * @addtogroup State Estimation
 * @brief Acquires sensor data and computes state estimate
 * @{
 *
 * @file       filterekf.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2013.
 * @brief      Extended Kalman Filter. Calculates complete system state except
 *             accelerometer drift.
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
#include <pios_struct_helper.h>

#include <ekfconfiguration.h>
#include <ekfstatevariance.h>
#include <attitudestate.h>
#include <systemalarms.h>
#include <homelocation.h>

#include <insgps.h>
#include <CoordinateConversions.h>

// Private constants

#define STACK_REQUIRED 2048
#define DT_ALPHA       1e-3f
#define DT_MIN         1e-6f
#define DT_MAX         1.0f
#define DT_INIT        (1.0f / 666.0f) // initialize with 666 Hz (default sensor update rate on revo)

#define IMPORT_SENSOR_IF_UPDATED(shortname, num) \
    if (IS_SET(state->updated, SENSORUPDATES_##shortname)) { \
        uint8_t t; \
        for (t = 0; t < num; t++) { \
            this->work.shortname[t] = state->shortname[t]; \
        } \
    }

// Private types
struct data {
    EKFConfigurationData ekfConfiguration;
    HomeLocationData     homeLocation;

    bool    usePos;

    int32_t init_stage;

    stateEstimation work;

    bool inited;

    PiOSDeltatimeConfig dtconfig;
};

// Private variables
static bool initialized = 0;


// Private functions

static int32_t init13i(stateFilter *self);
static int32_t init13(stateFilter *self);
static int32_t maininit(stateFilter *self);
static filterResult filter(stateFilter *self, stateEstimation *state);
static inline bool invalid_var(float data);

static void globalInit(void);


static void globalInit(void)
{
    if (!initialized) {
        initialized = 1;
        EKFConfigurationInitialize();
        EKFStateVarianceInitialize();
        HomeLocationInitialize();
    }
}

int32_t filterEKF13iInitialize(stateFilter *handle)
{
    globalInit();
    handle->init      = &init13i;
    handle->filter    = &filter;
    handle->localdata = pios_malloc(sizeof(struct data));
    return STACK_REQUIRED;
}
int32_t filterEKF13Initialize(stateFilter *handle)
{
    globalInit();
    handle->init      = &init13;
    handle->filter    = &filter;
    handle->localdata = pios_malloc(sizeof(struct data));
    return STACK_REQUIRED;
}
// XXX
// TODO: Until the 16 state EKF is implemented, run 13 state, so compilation runs through
// XXX
int32_t filterEKF16iInitialize(stateFilter *handle)
{
    globalInit();
    handle->init      = &init13i;
    handle->filter    = &filter;
    handle->localdata = pios_malloc(sizeof(struct data));
    return STACK_REQUIRED;
}
int32_t filterEKF16Initialize(stateFilter *handle)
{
    globalInit();
    handle->init      = &init13;
    handle->filter    = &filter;
    handle->localdata = pios_malloc(sizeof(struct data));
    return STACK_REQUIRED;
}


static int32_t init13i(stateFilter *self)
{
    struct data *this = (struct data *)self->localdata;

    this->usePos = 0;
    return maininit(self);
}

static int32_t init13(stateFilter *self)
{
    struct data *this = (struct data *)self->localdata;

    this->usePos = 1;
    return maininit(self);
}

static int32_t maininit(stateFilter *self)
{
    struct data *this = (struct data *)self->localdata;

    this->inited       = false;
    this->init_stage   = 0;
    this->work.updated = 0;
    PIOS_DELTATIME_Init(&this->dtconfig, DT_INIT, DT_MIN, DT_MAX, DT_ALPHA);

    EKFConfigurationGet(&this->ekfConfiguration);
    int t;
    // plausibility check
    for (t = 0; t < EKFCONFIGURATION_P_NUMELEM; t++) {
        if (invalid_var(cast_struct_to_array(this->ekfConfiguration.P, this->ekfConfiguration.P.AttitudeQ1)[t])) {
            return 2;
        }
    }
    for (t = 0; t < EKFCONFIGURATION_Q_NUMELEM; t++) {
        if (invalid_var(cast_struct_to_array(this->ekfConfiguration.Q, this->ekfConfiguration.Q.AccelX)[t])) {
            return 2;
        }
    }
    for (t = 0; t < EKFCONFIGURATION_R_NUMELEM; t++) {
        if (invalid_var(cast_struct_to_array(this->ekfConfiguration.R, this->ekfConfiguration.R.BaroZ)[t])) {
            return 2;
        }
    }
    HomeLocationGet(&this->homeLocation);
    // Don't require HomeLocation.Set to be true but at least require a mag configuration (allows easily
    // switching between indoor and outdoor mode with Set = false)
    if ((this->homeLocation.Be[0] * this->homeLocation.Be[0] + this->homeLocation.Be[1] * this->homeLocation.Be[1] + this->homeLocation.Be[2] * this->homeLocation.Be[2] < 1e-5f)) {
        return 2;
    }


    return 0;
}

/**
 * Collect all required state variables, then run complementary filter
 */
static filterResult filter(stateFilter *self, stateEstimation *state)
{
    struct data *this    = (struct data *)self->localdata;

    const float zeros[3] = { 0.0f, 0.0f, 0.0f };

    // Perform the update
    float dT;
    uint16_t sensors = 0;

    this->work.updated |= state->updated;

    // check magnetometer alarm, discard any magnetometer readings if not OK
    // during initialization phase (but let them through afterwards)
    SystemAlarmsAlarmData alarms;
    SystemAlarmsAlarmGet(&alarms);
    if (alarms.Magnetometer != SYSTEMALARMS_ALARM_OK && !this->inited) {
        UNSET_MASK(state->updated, SENSORUPDATES_mag);
        UNSET_MASK(this->work.updated, SENSORUPDATES_mag);
    }

    // Get most recent data
    IMPORT_SENSOR_IF_UPDATED(gyro, 3);
    IMPORT_SENSOR_IF_UPDATED(accel, 3);
    IMPORT_SENSOR_IF_UPDATED(mag, 3);
    IMPORT_SENSOR_IF_UPDATED(baro, 1);
    IMPORT_SENSOR_IF_UPDATED(pos, 3);
    IMPORT_SENSOR_IF_UPDATED(vel, 3);
    IMPORT_SENSOR_IF_UPDATED(airspeed, 2);

    // check whether mandatory updates are present accels must have been supplied already,
    // and gyros must be supplied just now for a prediction step to take place
    // ("gyros last" rule for multi object synchronization)
    if (!(IS_SET(this->work.updated, SENSORUPDATES_accel) && IS_SET(state->updated, SENSORUPDATES_gyro))) {
        UNSET_MASK(state->updated, SENSORUPDATES_pos);
        UNSET_MASK(state->updated, SENSORUPDATES_vel);
        UNSET_MASK(state->updated, SENSORUPDATES_attitude);
        UNSET_MASK(state->updated, SENSORUPDATES_gyro);
        return FILTERRESULT_OK;
    }

    dT = PIOS_DELTATIME_GetAverageSeconds(&this->dtconfig);

    if (!this->inited && IS_SET(this->work.updated, SENSORUPDATES_mag) && IS_SET(this->work.updated, SENSORUPDATES_baro) && IS_SET(this->work.updated, SENSORUPDATES_pos)) {
        // Don't initialize until all sensors are read
        if (this->init_stage == 0) {
            // Reset the INS algorithm
            INSGPSInit();
            // variance is measured in mGaus, but internally the EKF works with a normalized  vector. Scale down by Be^2
            float Be2 = this->homeLocation.Be[0] * this->homeLocation.Be[0] + this->homeLocation.Be[1] * this->homeLocation.Be[1] + this->homeLocation.Be[2] * this->homeLocation.Be[2];
            INSSetMagVar((float[3]) { this->ekfConfiguration.R.MagX / Be2,
                                      this->ekfConfiguration.R.MagY / Be2,
                                      this->ekfConfiguration.R.MagZ / Be2 }
                         );
            INSSetAccelVar((float[3]) { this->ekfConfiguration.Q.AccelX,
                                        this->ekfConfiguration.Q.AccelY,
                                        this->ekfConfiguration.Q.AccelZ }
                           );
            INSSetGyroVar((float[3]) { this->ekfConfiguration.Q.GyroX,
                                       this->ekfConfiguration.Q.GyroY,
                                       this->ekfConfiguration.Q.GyroZ }
                          );
            INSSetGyroBiasVar((float[3]) { this->ekfConfiguration.Q.GyroDriftX,
                                           this->ekfConfiguration.Q.GyroDriftY,
                                           this->ekfConfiguration.Q.GyroDriftZ }
                              );
            INSSetBaroVar(this->ekfConfiguration.R.BaroZ);

            // Initialize the gyro bias
            float gyro_bias[3] = { 0.0f, 0.0f, 0.0f };
            INSSetGyroBias(gyro_bias);

            AttitudeStateData attitudeState;
            AttitudeStateGet(&attitudeState);

            // Set initial attitude. Use accels to determine roll and pitch, rotate magnetic measurement accordingly,
            // so pseudo "north" vector can be estimated even if the board is not level
            attitudeState.Roll = atan2f(-this->work.accel[1], -this->work.accel[2]);
            float zn  = cosf(attitudeState.Roll) * this->work.mag[2] + sinf(attitudeState.Roll) * this->work.mag[1];
            float yn  = cosf(attitudeState.Roll) * this->work.mag[1] - sinf(attitudeState.Roll) * this->work.mag[2];

            // rotate accels z vector according to roll
            float azn = cosf(attitudeState.Roll) * this->work.accel[2] + sinf(attitudeState.Roll) * this->work.accel[1];
            attitudeState.Pitch = atan2f(this->work.accel[0], -azn);

            float xn  = cosf(attitudeState.Pitch) * this->work.mag[0] + sinf(attitudeState.Pitch) * zn;

            attitudeState.Yaw = atan2f(-yn, xn);
            // TODO: This is still a hack
            // Put this in a proper generic function in CoordinateConversion.c
            // should take 4 vectors: g (0,0,-9.81), accels, Be (or 1,0,0 if no home loc) and magnetometers (or 1,0,0 if no mags)
            // should calculate the rotation in 3d space using proper cross product math
            // SUBTODO: formulate the math required

            attitudeState.Roll  = RAD2DEG(attitudeState.Roll);
            attitudeState.Pitch = RAD2DEG(attitudeState.Pitch);
            attitudeState.Yaw   = RAD2DEG(attitudeState.Yaw);

            RPY2Quaternion(&attitudeState.Roll, this->work.attitude);

            INSSetState(this->work.pos, (float *)zeros, this->work.attitude, (float *)zeros, (float *)zeros);

            INSResetP(cast_struct_to_array(this->ekfConfiguration.P, this->ekfConfiguration.P.AttitudeQ1));
        } else {
            // Run prediction a bit before any corrections

            float gyros[3] = { DEG2RAD(this->work.gyro[0]), DEG2RAD(this->work.gyro[1]), DEG2RAD(this->work.gyro[2]) };
            INSStatePrediction(gyros, this->work.accel, dT);

            // Copy the attitude into the state
            // NOTE: updating gyr correctly is valid, because this code is reached only when SENSORUPDATES_gyro is already true
            state->attitude[0] = Nav.q[0];
            state->attitude[1] = Nav.q[1];
            state->attitude[2] = Nav.q[2];
            state->attitude[3] = Nav.q[3];
            state->gyro[0]    -= RAD2DEG(Nav.gyro_bias[0]);
            state->gyro[1]    -= RAD2DEG(Nav.gyro_bias[1]);
            state->gyro[2]    -= RAD2DEG(Nav.gyro_bias[2]);
            state->pos[0]   = Nav.Pos[0];
            state->pos[1]   = Nav.Pos[1];
            state->pos[2]   = Nav.Pos[2];
            state->vel[0]   = Nav.Vel[0];
            state->vel[1]   = Nav.Vel[1];
            state->vel[2]   = Nav.Vel[2];
            state->updated |= SENSORUPDATES_attitude | SENSORUPDATES_pos | SENSORUPDATES_vel;
        }

        this->init_stage++;
        if (this->init_stage > 10) {
            this->inited = true;
        }

        return FILTERRESULT_OK;
    }

    if (!this->inited) {
        return FILTERRESULT_CRITICAL;
    }

    float gyros[3] = { DEG2RAD(this->work.gyro[0]), DEG2RAD(this->work.gyro[1]), DEG2RAD(this->work.gyro[2]) };

    // Advance the state estimate
    INSStatePrediction(gyros, this->work.accel, dT);

    // Copy the attitude into the state
    // NOTE: updating gyr correctly is valid, because this code is reached only when SENSORUPDATES_gyro is already true
    state->attitude[0] = Nav.q[0];
    state->attitude[1] = Nav.q[1];
    state->attitude[2] = Nav.q[2];
    state->attitude[3] = Nav.q[3];
    state->gyro[0]    -= RAD2DEG(Nav.gyro_bias[0]);
    state->gyro[1]    -= RAD2DEG(Nav.gyro_bias[1]);
    state->gyro[2]    -= RAD2DEG(Nav.gyro_bias[2]);
    state->pos[0]   = Nav.Pos[0];
    state->pos[1]   = Nav.Pos[1];
    state->pos[2]   = Nav.Pos[2];
    state->vel[0]   = Nav.Vel[0];
    state->vel[1]   = Nav.Vel[1];
    state->vel[2]   = Nav.Vel[2];
    state->updated |= SENSORUPDATES_attitude | SENSORUPDATES_pos | SENSORUPDATES_vel;

    // Advance the covariance estimate
    INSCovariancePrediction(dT);

    if (IS_SET(this->work.updated, SENSORUPDATES_mag)) {
        sensors |= MAG_SENSORS;
    }

    if (IS_SET(this->work.updated, SENSORUPDATES_baro)) {
        sensors |= BARO_SENSOR;
    }

    INSSetMagNorth(this->homeLocation.Be);

    if (!this->usePos) {
        // position and velocity variance used in indoor mode
        INSSetPosVelVar((float[3]) { this->ekfConfiguration.FakeR.FakeGPSPosIndoor,
                                     this->ekfConfiguration.FakeR.FakeGPSPosIndoor,
                                     this->ekfConfiguration.FakeR.FakeGPSPosIndoor },
                        (float[3]) { this->ekfConfiguration.FakeR.FakeGPSVelIndoor,
                                     this->ekfConfiguration.FakeR.FakeGPSVelIndoor,
                                     this->ekfConfiguration.FakeR.FakeGPSVelIndoor }
                        );
    } else {
        // position and velocity variance used in outdoor mode
        INSSetPosVelVar((float[3]) { this->ekfConfiguration.R.GPSPosNorth,
                                     this->ekfConfiguration.R.GPSPosEast,
                                     this->ekfConfiguration.R.GPSPosDown },
                        (float[3]) { this->ekfConfiguration.R.GPSVelNorth,
                                     this->ekfConfiguration.R.GPSVelEast,
                                     this->ekfConfiguration.R.GPSVelDown }
                        );
    }

    if (IS_SET(this->work.updated, SENSORUPDATES_pos)) {
        sensors |= POS_SENSORS;
    }

    if (IS_SET(this->work.updated, SENSORUPDATES_vel)) {
        sensors |= HORIZ_SENSORS | VERT_SENSORS;
    }

    if (IS_SET(this->work.updated, SENSORUPDATES_airspeed) && ((!IS_SET(this->work.updated, SENSORUPDATES_vel) && !IS_SET(this->work.updated, SENSORUPDATES_pos)) | !this->usePos)) {
        // HACK: feed airspeed into EKF as velocity, treat wind as 1e2 variance
        sensors |= HORIZ_SENSORS | VERT_SENSORS;
        INSSetPosVelVar((float[3]) { this->ekfConfiguration.FakeR.FakeGPSPosIndoor,
                                     this->ekfConfiguration.FakeR.FakeGPSPosIndoor,
                                     this->ekfConfiguration.FakeR.FakeGPSPosIndoor },
                        (float[3]) { this->ekfConfiguration.FakeR.FakeGPSVelAirspeed,
                                     this->ekfConfiguration.FakeR.FakeGPSVelAirspeed,
                                     this->ekfConfiguration.FakeR.FakeGPSVelAirspeed }
                        );
        // rotate airspeed vector into NED frame - airspeed is measured in X axis only
        float R[3][3];
        Quaternion2R(Nav.q, R);
        float vtas[3] = { this->work.airspeed[1], 0.0f, 0.0f };
        rot_mult(R, vtas, this->work.vel);
    }

    /*
     * TODO: Need to add a general sanity check for all the inputs to make sure their kosher
     * although probably should occur within INS itself
     */
    if (sensors) {
        INSCorrection(this->work.mag, this->work.pos, this->work.vel, this->work.baro[0], sensors);
    }

    EKFStateVarianceData vardata;
    EKFStateVarianceGet(&vardata);
    INSGetP(cast_struct_to_array(vardata.P, vardata.P.AttitudeQ1));
    EKFStateVarianceSet(&vardata);
    int t;
    for (t = 0; t < EKFSTATEVARIANCE_P_NUMELEM; t++) {
        if (!IS_REAL(cast_struct_to_array(vardata.P, vardata.P.AttitudeQ1)[t]) || cast_struct_to_array(vardata.P, vardata.P.AttitudeQ1)[t] <= 0.0f) {
            INSResetP(cast_struct_to_array(this->ekfConfiguration.P, this->ekfConfiguration.P.AttitudeQ1));
            this->init_stage = -1;
            break;
        }
    }

    // all sensor data has been used, reset!
    this->work.updated = 0;

    if (this->init_stage < 0) {
        return FILTERRESULT_WARNING;
    } else {
        return FILTERRESULT_OK;
    }
}

// check for invalid variance values
static inline bool invalid_var(float data)
{
    if (isnan(data) || isinf(data)) {
        return true;
    }
    if (data < 1e-15f) { // var should not be close to zero. And not negative either.
        return true;
    }
    return false;
}

/**
 * @}
 * @}
 */

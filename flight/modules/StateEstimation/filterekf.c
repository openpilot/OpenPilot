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

#include <ekfconfiguration.h>
#include <ekfstatevariance.h>
#include <gpsposition.h>
#include <attitudestate.h>
#include <homelocation.h>


#include <insgps.h>
#include <CoordinateConversions.h>

// Private constants


// Private types

// Private variables
static EKFConfigurationData ekfConfiguration;
static HomeLocationData homeLocation;

static bool initialized = 0;
static bool first_run   = 1;
static bool usePos = 0;


// Private functions

static int32_t init13i(void);
static int32_t init13(void);
static int32_t maininit(void);
static int32_t filter(stateEstimation *state);
static inline bool invalid(float data);
static inline bool invalid_var(float data);

static void globalInit(void);


static void globalInit(void)
{
    if (!initialized) {
        initialized = 1;
        EKFConfigurationInitialize();
        EKFStateVarianceInitialize();
    }
}

void filterEKF13iInitialize(stateFilter *handle)
{
    globalInit();
    handle->init   = &init13i;
    handle->filter = &filter;
}
void filterEKF13Initialize(stateFilter *handle)
{
    globalInit();
    handle->init   = &init13;
    handle->filter = &filter;
}
// XXX
// TODO: Until the 16 state EKF is implemented, run 13 state, so compilation runs through
// XXX
void filterEKF16iInitialize(stateFilter *handle)
{
    globalInit();
    handle->init   = &init13i;
    handle->filter = &filter;
}
void filterEKF16Initialize(stateFilter *handle)
{
    globalInit();
    handle->init   = &init13;
    handle->filter = &filter;
}


static int32_t init13i(void)
{
    usePos = 0;
    return maininit();
}

static int32_t init13(void)
{
    usePos = 1;
    return maininit();
}

static int32_t maininit(void)
{
    first_run = 1;

    EKFConfigurationGet(&ekfConfiguration);
    int t;
    // plausibility check
    for (t = 0; t < EKFCONFIGURATION_P_NUMELEM; t++) {
        if (invalid_var(ekfConfiguration.P[t])) {
            return 2;
        }
    }
    for (t = 0; t < EKFCONFIGURATION_Q_NUMELEM; t++) {
        if (invalid_var(ekfConfiguration.Q[t])) {
            return 2;
        }
    }
    for (t = 0; t < EKFCONFIGURATION_R_NUMELEM; t++) {
        if (invalid_var(ekfConfiguration.R[t])) {
            return 2;
        }
    }
    HomeLocationGet(&homeLocation);
    // Don't require HomeLocation.Set to be true but at least require a mag configuration (allows easily
    // switching between indoor and outdoor mode with Set = false)
    if ((homeLocation.Be[0] * homeLocation.Be[0] + homeLocation.Be[1] * homeLocation.Be[1] + homeLocation.Be[2] * homeLocation.Be[2] < 1e-5f)) {
        return 2;
    }


    return 0;
}

/**
 * Collect all required state variables, then run complementary filter
 */
static int32_t filter(stateEstimation *state)
{
    static int32_t init_stage;

    static stateEstimation work;

    static uint32_t ins_last_time = 0;
    static bool inited;

    const float zeros[3] = { 0.0f, 0.0f, 0.0f };

    // Perform the update
    float dT;
    uint16_t sensors = 0;

    if (inited) {
        work.updated = 0;
    }

    if (first_run) {
        first_run     = false;
        inited        = false;
        init_stage    = 0;

        work.updated  = 0;

        ins_last_time = PIOS_DELAY_GetRaw();

        return 1;
    }

    work.updated |= state->updated;

    // Get most recent data
#define UPDATE(shortname, num) \
    if (ISSET(state->updated, shortname##_UPDATED)) { \
        uint8_t t; \
        for (t = 0; t < num; t++) { \
            work.shortname[t] = state->shortname[t]; \
        } \
    }
    UPDATE(gyr, 3);
    UPDATE(acc, 3);
    UPDATE(mag, 3);
    UPDATE(bar, 1);
    UPDATE(pos, 3);
    UPDATE(vel, 3);
    UPDATE(air, 2);


    if (usePos) {
        GPSPositionData gpsData;
        GPSPositionGet(&gpsData);
        // Have a minimum requirement for gps usage
        if ((gpsData.Satellites < 7) ||
            (gpsData.PDOP > 4.0f) ||
            (gpsData.Latitude == 0 && gpsData.Longitude == 0) ||
            (homeLocation.Set != HOMELOCATION_SET_TRUE)) {
            UNSET(state->updated, pos_UPDATED);
            UNSET(state->updated, vel_UPDATED);
            UNSET(work.updated, pos_UPDATED);
            UNSET(work.updated, vel_UPDATED);
        }
    }

    dT = PIOS_DELAY_DiffuS(ins_last_time) / 1.0e6f;
    ins_last_time = PIOS_DELAY_GetRaw();

    // This should only happen at start up or at mode switches
    if (dT > 0.01f) {
        dT = 0.01f;
    } else if (dT <= 0.001f) {
        dT = 0.001f;
    }

    if (!inited && ISSET(work.updated, mag_UPDATED) && ISSET(work.updated, bar_UPDATED) && ISSET(work.updated, pos_UPDATED)) {
        // Don't initialize until all sensors are read
        if (init_stage == 0) {
            // Reset the INS algorithm
            INSGPSInit();
            INSSetMagVar((float[3]) { ekfConfiguration.R[EKFCONFIGURATION_R_MAGX],
                                      ekfConfiguration.R[EKFCONFIGURATION_R_MAGY],
                                      ekfConfiguration.R[EKFCONFIGURATION_R_MAGZ] }
                         );
            INSSetAccelVar((float[3]) { ekfConfiguration.Q[EKFCONFIGURATION_Q_ACCELX],
                                        ekfConfiguration.Q[EKFCONFIGURATION_Q_ACCELY],
                                        ekfConfiguration.Q[EKFCONFIGURATION_Q_ACCELZ] }
                           );
            INSSetGyroVar((float[3]) { ekfConfiguration.Q[EKFCONFIGURATION_Q_GYROX],
                                       ekfConfiguration.Q[EKFCONFIGURATION_Q_GYROY],
                                       ekfConfiguration.Q[EKFCONFIGURATION_Q_GYROZ] }
                          );
            INSSetGyroBiasVar((float[3]) { ekfConfiguration.Q[EKFCONFIGURATION_Q_GYRODRIFTX],
                                           ekfConfiguration.Q[EKFCONFIGURATION_Q_GYRODRIFTY],
                                           ekfConfiguration.Q[EKFCONFIGURATION_Q_GYRODRIFTZ] }
                              );
            INSSetBaroVar(ekfConfiguration.R[EKFCONFIGURATION_R_BAROZ]);

            // Initialize the gyro bias
            float gyro_bias[3] = { 0.0f, 0.0f, 0.0f };
            INSSetGyroBias(gyro_bias);

            AttitudeStateData attitudeState;
            AttitudeStateGet(&attitudeState);

            // Set initial attitude. Use accels to determine roll and pitch, rotate magnetic measurement accordingly,
            // so pseudo "north" vector can be estimated even if the board is not level
            attitudeState.Roll = atan2f(-work.acc[1], -work.acc[2]);
            float zn  = cosf(attitudeState.Roll) * work.mag[2] + sinf(attitudeState.Roll) * work.mag[1];
            float yn  = cosf(attitudeState.Roll) * work.mag[1] - sinf(attitudeState.Roll) * work.mag[2];

            // rotate accels z vector according to roll
            float azn = cosf(attitudeState.Roll) * work.acc[2] + sinf(attitudeState.Roll) * work.acc[1];
            attitudeState.Pitch = atan2f(work.acc[0], -azn);

            float xn  = cosf(attitudeState.Pitch) * work.mag[0] + sinf(attitudeState.Pitch) * zn;

            attitudeState.Yaw = atan2f(-yn, xn);
            // TODO: This is still a hack
            // Put this in a proper generic function in CoordinateConversion.c
            // should take 4 vectors: g (0,0,-9.81), accels, Be (or 1,0,0 if no home loc) and magnetometers (or 1,0,0 if no mags)
            // should calculate the rotation in 3d space using proper cross product math
            // SUBTODO: formulate the math required

            attitudeState.Roll  = RAD2DEG(attitudeState.Roll);
            attitudeState.Pitch = RAD2DEG(attitudeState.Pitch);
            attitudeState.Yaw   = RAD2DEG(attitudeState.Yaw);

            RPY2Quaternion(&attitudeState.Roll, work.att);

            INSSetState(work.pos, (float *)zeros, work.att, (float *)zeros, (float *)zeros);

            INSResetP(ekfConfiguration.P);
        } else {
            // Run prediction a bit before any corrections

            float gyros[3] = { DEG2RAD(work.gyr[0]), DEG2RAD(work.gyr[1]), DEG2RAD(work.gyr[2]) };
            INSStatePrediction(gyros, work.acc, dT);

            state->att[0]   = Nav.q[0];
            state->att[1]   = Nav.q[1];
            state->att[2]   = Nav.q[2];
            state->att[3]   = Nav.q[3];
            state->gyr[0]  += Nav.gyro_bias[0];
            state->gyr[1]  += Nav.gyro_bias[1];
            state->gyr[2]  += Nav.gyro_bias[2];
            state->pos[0]   = Nav.Pos[0];
            state->pos[1]   = Nav.Pos[1];
            state->pos[2]   = Nav.Pos[2];
            state->vel[0]   = Nav.Vel[0];
            state->vel[1]   = Nav.Vel[1];
            state->vel[2]   = Nav.Vel[2];
            state->updated |= att_UPDATED | pos_UPDATED | vel_UPDATED | gyr_UPDATED;
        }

        init_stage++;
        if (init_stage > 10) {
            inited = true;
        }

        return 0;
    }

    if (!inited) {
        return 1;
    }

    float gyros[3] = { DEG2RAD(work.gyr[0]), DEG2RAD(work.gyr[1]), DEG2RAD(work.gyr[2]) };

    // Advance the state estimate
    INSStatePrediction(gyros, work.acc, dT);

    // Copy the attitude into the UAVO
    state->att[0]   = Nav.q[0];
    state->att[1]   = Nav.q[1];
    state->att[2]   = Nav.q[2];
    state->att[3]   = Nav.q[3];
    state->gyr[0]  += Nav.gyro_bias[0];
    state->gyr[1]  += Nav.gyro_bias[1];
    state->gyr[2]  += Nav.gyro_bias[2];
    state->pos[0]   = Nav.Pos[0];
    state->pos[1]   = Nav.Pos[1];
    state->pos[2]   = Nav.Pos[2];
    state->vel[0]   = Nav.Vel[0];
    state->vel[1]   = Nav.Vel[1];
    state->vel[2]   = Nav.Vel[2];
    state->updated |= att_UPDATED | pos_UPDATED | vel_UPDATED | gyr_UPDATED;

    // Advance the covariance estimate
    INSCovariancePrediction(dT);

    if (ISSET(work.updated, mag_UPDATED)) {
        sensors |= MAG_SENSORS;
    }

    if (ISSET(work.updated, bar_UPDATED)) {
        sensors |= BARO_SENSOR;
    }

    INSSetMagNorth(homeLocation.Be);

    if (!usePos) {
        // position and velocity variance used in indoor mode
        INSSetPosVelVar((float[3]) { ekfConfiguration.FakeR[EKFCONFIGURATION_FAKER_FAKEGPSPOSINDOOR],
                                     ekfConfiguration.FakeR[EKFCONFIGURATION_FAKER_FAKEGPSPOSINDOOR],
                                     ekfConfiguration.FakeR[EKFCONFIGURATION_FAKER_FAKEGPSPOSINDOOR] },
                        (float[3]) { ekfConfiguration.FakeR[EKFCONFIGURATION_FAKER_FAKEGPSVELINDOOR],
                                     ekfConfiguration.FakeR[EKFCONFIGURATION_FAKER_FAKEGPSVELINDOOR],
                                     ekfConfiguration.FakeR[EKFCONFIGURATION_FAKER_FAKEGPSVELINDOOR] }
                        );
    } else {
        // position and velocity variance used in outdoor mode
        INSSetPosVelVar((float[3]) { ekfConfiguration.R[EKFCONFIGURATION_R_GPSPOSNORTH],
                                     ekfConfiguration.R[EKFCONFIGURATION_R_GPSPOSEAST],
                                     ekfConfiguration.R[EKFCONFIGURATION_R_GPSPOSDOWN] },
                        (float[3]) { ekfConfiguration.R[EKFCONFIGURATION_R_GPSVELNORTH],
                                     ekfConfiguration.R[EKFCONFIGURATION_R_GPSVELEAST],
                                     ekfConfiguration.R[EKFCONFIGURATION_R_GPSVELDOWN] }
                        );
    }

    if (ISSET(work.updated, pos_UPDATED)) {
        sensors |= POS_SENSORS;
    }

    if (ISSET(work.updated, vel_UPDATED)) {
        sensors |= HORIZ_SENSORS | VERT_SENSORS;
    }

    if (ISSET(work.updated, air_UPDATED) && ((!ISSET(work.updated, vel_UPDATED) && !ISSET(work.updated, pos_UPDATED)) | !usePos)) {
        // HACK: feed airspeed into EKF as velocity, treat wind as 1e2 variance
        sensors |= HORIZ_SENSORS | VERT_SENSORS;
        INSSetPosVelVar((float[3]) { ekfConfiguration.FakeR[EKFCONFIGURATION_FAKER_FAKEGPSPOSINDOOR],
                                     ekfConfiguration.FakeR[EKFCONFIGURATION_FAKER_FAKEGPSPOSINDOOR],
                                     ekfConfiguration.FakeR[EKFCONFIGURATION_FAKER_FAKEGPSPOSINDOOR] },
                        (float[3]) { ekfConfiguration.FakeR[EKFCONFIGURATION_FAKER_FAKEGPSVELAIRSPEED],
                                     ekfConfiguration.FakeR[EKFCONFIGURATION_FAKER_FAKEGPSVELAIRSPEED],
                                     ekfConfiguration.FakeR[EKFCONFIGURATION_FAKER_FAKEGPSVELAIRSPEED] }
                        );
        // rotate airspeed vector into NED frame - airspeed is measured in X axis only
        float R[3][3];
        Quaternion2R(Nav.q, R);
        float vtas[3] = { work.air[1], 0.0f, 0.0f };
        rot_mult(R, vtas, work.vel);
    }

    /*
     * TODO: Need to add a general sanity check for all the inputs to make sure their kosher
     * although probably should occur within INS itself
     */
    if (sensors) {
        INSCorrection(work.mag, work.pos, work.vel, work.bar[0], sensors);
    }

    EKFStateVarianceData vardata;
    EKFStateVarianceGet(&vardata);
    INSGetP(vardata.P);
    EKFStateVarianceSet(&vardata);

    return 0;
}

// check for invalid values
static inline bool invalid(float data)
{
    if (isnan(data) || isinf(data)) {
        return true;
    }
    return false;
}

// check for invalid variance values
static inline bool invalid_var(float data)
{
    if (invalid(data)) {
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

/**
 ******************************************************************************
 * @addtogroup OpenPilotModules OpenPilot Modules
 * @{
 * @addtogroup StateSetimation Module
 * @{
 *
 * @file       stateestimation.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2011.
 * @brief      Acquires sensor data and fuses it into attitude estimate for CC
 *
 * @see        The GNU Public License (GPL) Version 3
 *
 *****************************************************************************/
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
#ifndef STATEESTIMATION_H
#define STATEESTIMATION_H

#include <openpilot.h>

typedef enum {
    SENSORUPDATES_gyro         = 1 << 0,
        SENSORUPDATES_accel    = 1 << 1,
        SENSORUPDATES_mag      = 1 << 2,
        SENSORUPDATES_attitude = 1 << 3,
        SENSORUPDATES_pos      = 1 << 4,
        SENSORUPDATES_vel      = 1 << 5,
        SENSORUPDATES_airspeed = 1 << 6,
        SENSORUPDATES_baro     = 1 << 7,
} sensorUpdates;

typedef struct {
    float gyro[3];
    float accel[3];
    float mag[3];
    float attitude[4];
    float pos[3];
    float vel[3];
    float airspeed[2];
    float baro[1];
    sensorUpdates updated;
} stateEstimation;

typedef struct stateFilterStruct {
    int32_t (*init)(struct stateFilterStruct *self);
    int32_t (*filter)(struct stateFilterStruct *self, stateEstimation *state);
    void *localdata;
} stateFilter;


int32_t filterMagInitialize(stateFilter *handle);
int32_t filterBaroInitialize(stateFilter *handle);
int32_t filterAltitudeInitialize(stateFilter *handle);
int32_t filterAirInitialize(stateFilter *handle);
int32_t filterStationaryInitialize(stateFilter *handle);
int32_t filterCFInitialize(stateFilter *handle);
int32_t filterCFMInitialize(stateFilter *handle);
int32_t filterEKF13iInitialize(stateFilter *handle);
int32_t filterEKF13Initialize(stateFilter *handle);
int32_t filterEKF16iInitialize(stateFilter *handle);
int32_t filterEKF16Initialize(stateFilter *handle);

#endif // STATEESTIMATION_H

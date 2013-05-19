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
    gyr_UPDATED     = 1 << 0,
        acc_UPDATED = 1 << 1,
        mag_UPDATED = 1 << 2,
        pos_UPDATED = 1 << 3,
        vel_UPDATED = 1 << 4,
        air_UPDATED = 1 << 5,
        bar_UPDATED = 1 << 6,
} sensorUpdates;

typedef struct {
    float gyr[3];
    float acc[3];
    float mag[3];
    float pos[3];
    float vel[3];
    float air[2];
    float bar[1];
    sensorUpdates updated;
} stateEstimation;

#define ISSET(bitfield, bit) ((bitfield) & (bit) ? 1 : 0)
#define UNSET(bitfield, bit) (bitfield) &= ~(bit)


typedef struct stateFilterStruct {
    int32_t (*init)(void);
    int32_t (*filter)(stateEstimation *state);
} stateFilter;


void filterMagInitialize(stateFilter *handle);
void filterBaroInitialize(stateFilter *handle);
void filterAirInitialize(stateFilter *handle);
void filterStationaryInitialize(stateFilter *handle);
void filterCFInitialize(stateFilter *handle);
void filterCFMInitialize(stateFilter *handle);
void filterEKF13iInitialize(stateFilter *handle);
void filterEKF13Initialize(stateFilter *handle);
void filterEKF16iInitialize(stateFilter *handle);
void filterEKF16Initialize(stateFilter *handle);

#endif // STATEESTIMATION_H

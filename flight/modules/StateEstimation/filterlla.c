/**
 ******************************************************************************
 * @addtogroup OpenPilotModules OpenPilot Modules
 * @{
 * @addtogroup State Estimation
 * @brief Acquires sensor data and computes state estimate
 * @{
 *
 * @file       filterlla.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2013.
 * @brief      Computes NED position from GPS LLA data
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
#include <CoordinateConversions.h>

#include <homelocation.h>
#include <gpssettings.h>
#include <gpspositionsensor.h>

// Private constants

#define STACK_REQUIRED 256

// Private types
struct data {
    GPSSettingsData  settings;
    HomeLocationData home;
    double HomeECEF[3];
    float HomeRne[3][3];
};

// Private variables

// Private functions

static int32_t init(stateFilter *self);
static filterResult filter(stateFilter *self, stateEstimation *state);


int32_t filterLLAInitialize(stateFilter *handle)
{
    handle->init      = &init;
    handle->filter    = &filter;
    handle->localdata = pios_malloc(sizeof(struct data));
    GPSSettingsInitialize();
    GPSPositionSensorInitialize();
    HomeLocationInitialize();
    return STACK_REQUIRED;
}

static int32_t init(__attribute__((unused)) stateFilter *self)
{
    struct data *this = (struct data *)self->localdata;

    GPSSettingsGet(&this->settings);
    HomeLocationGet(&this->home);
    if (this->home.Set == HOMELOCATION_SET_TRUE) {
        // calculate home location coordinate reference
        int32_t LLAi[3] = {
            this->home.Latitude,
            this->home.Longitude,
            (int32_t)(this->home.Altitude * 1e4f),
        };
        LLA2ECEF(LLAi, this->HomeECEF);
        RneFromLLA(LLAi, this->HomeRne);
    }
    return 0;
}

static filterResult filter(__attribute__((unused)) stateFilter *self, stateEstimation *state)
{
    struct data *this = (struct data *)self->localdata;

    // cannot update local NED if home location is unset
    if (this->home.Set != HOMELOCATION_SET_TRUE) {
        return FILTERRESULT_WARNING;
    }

    // only do stuff if we have a valid GPS update
    if (IS_SET(state->updated, SENSORUPDATES_lla)) {
        // LLA information is not part of the state blob, due to its non standard layout (fixed point representation, quality of signal, ...)
        // this filter deals with the gory details of interpreting it and storing it in a standard Cartesian position state
        GPSPositionSensorData gpsdata;
        GPSPositionSensorGet(&gpsdata);

        // check if we have a valid GPS signal (not checked by StateEstimation istelf)
        if ((gpsdata.PDOP < this->settings.MaxPDOP) && (gpsdata.Satellites >= this->settings.MinSattelites) &&
            (gpsdata.Status == GPSPOSITIONSENSOR_STATUS_FIX3D) &&
            (gpsdata.Latitude != 0 || gpsdata.Longitude != 0)) {
            int32_t LLAi[3] = {
                gpsdata.Latitude,
                gpsdata.Longitude,
                (int32_t)((gpsdata.Altitude + gpsdata.GeoidSeparation) * 1e4f),
            };
            LLA2Base(LLAi, this->HomeECEF, this->HomeRne, state->pos);
            state->updated |= SENSORUPDATES_pos;
        }
    }

    return FILTERRESULT_OK;
}

/**
 * @}
 * @}
 */

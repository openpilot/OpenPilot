/**
 ******************************************************************************
 *
 * @file       auxmagsupport.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2014.
 * @brief      Functions to handle aux mag data and calibration.
 *             --
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

#include <stdint.h>
#include "inc/auxmagsupport.h"
#include "CoordinateConversions.h"

static float mag_bias[3] = { 0, 0, 0 };
static float mag_transform[3][3] = {
    { 1, 0, 0 }, { 0, 1, 0 }, { 0, 0, 1 }
};

AuxMagSettingsTypeOptions option;

void auxmagsupport_reload_settings()
{
    AuxMagSettingsTypeGet(&option);
    float a[3][3];
    float b[3][3];
    float rotz;
    AuxMagSettingsmag_transformArrayGet((float *)a);
    AuxMagSettingsOrientationGet(&rotz);
    rotz = DEG2RAD(rotz);
    rot_about_axis_z(rotz, b);
    matrix_mult_3x3f(a, b, mag_transform);
    AuxMagSettingsmag_biasArrayGet(mag_bias);
}

void auxmagsupport_publish_samples(float mags[3], uint8_t status)
{
    float mag_out[3];

    mags[0] -= mag_bias[0];
    mags[1] -= mag_bias[1];
    mags[2] -= mag_bias[2];
    rot_mult(mag_transform, mags, mag_out);

    AuxMagSensorData data;
    data.x = mag_out[0];
    data.y = mag_out[1];
    data.z = mag_out[2];
    data.Status = status;
    AuxMagSensorSet(&data);
}

AuxMagSettingsTypeOptions auxmagsupport_get_type()
{
    return option;
}

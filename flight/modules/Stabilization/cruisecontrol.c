/**
 ******************************************************************************
 * @addtogroup OpenPilotModules OpenPilot Modules
 * @{
 * @addtogroup StabilizationModule Stabilization Module
 * @brief cruisecontrol mode
 * @note This file implements the logic for a cruisecontrol
 * @{
 *
 * @file       cruisecontrol.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2014.
 * @brief      Attitude stabilization module.
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

#include <openpilot.h>
#include <stabilization.h>
#include <attitudestate.h>

static float cruisecontrol_factor = 1.0f;

void cruisecontrol_compute_factor(AttitudeStateData *attitude)
{
    float angleCosine;

    // get attitude state and calculate angle
    // spherical right triangle
    // 0 <= acosf() <= Pi

    // angle = RAD2DEG(acosf(cos_lookup_deg(attitudeState.Roll) * cos_lookup_deg(attitudeState.Pitch)));
    // more efficient calculation: see coordinateconversion.h Quaternion2R()
    angleCosine = (attitude->q1 * attitude->q1) - (attitude->q2 * attitude->q2) - (attitude->q3 * attitude->q3) + (attitude->q4 * attitude->q4);

    // if past the cutoff angle (60 to 180 (180 means never))
    if (angleCosine < stabSettings.cruiseControl.cruise_control_max_angle_cosine) {
        // -1 reversed collective, 0 zero power, or 1 normal power
        // these are all unboosted
        cruisecontrol_factor = stabSettings.cruiseControl.cruise_control_inverted_power_switch;
    } else {
        // avoid singularity
        if (angleCosine > -1e-4f && angleCosine < 1e-4f) {
            cruisecontrol_factor = stabSettings.cruiseControl.cruise_control_max_power_factor;
        } else {
            cruisecontrol_factor = boundf(1.0f / angleCosine,
                                          -stabSettings.cruiseControl.cruise_control_max_power_factor,
                                          stabSettings.cruiseControl.cruise_control_max_power_factor);
        }
        // factor in the power trim, no effect at 1.0, linear effect increases with factor
        cruisecontrol_factor = (cruisecontrol_factor - 1.0f) * stabSettings.cruiseControl.cruise_control_power_trim + 1.0f;
        // if inverted and they want negative boost
        if (angleCosine < 0.0f && stabSettings.cruiseControl.cruise_control_inverted_power_switch == (int8_t)-1) {
            cruisecontrol_factor = -cruisecontrol_factor;
            // WARNING: might not go well together with power_trim adjustment above, WARNING: might have bad side effects regarding wound up integral accumulators in all 3 axis at transition stage!
            // as long as thrust is getting reversed
            // we may as well do pitch and yaw for a complete "invert switch"
            // WARNING: currently not implementable in this control scheme, needs to be done elsewhere!
            // actuatorDesired.Pitch = -actuatorDesired.Pitch;
            // actuatorDesired.Yaw   = -actuatorDesired.Yaw;
        }
    }
}


float cruisecontrol_apply_factor(float raw)
{
    if (stabSettings.cruiseControl.cruise_control_max_power_factor > 0.0001f) {
        // don't adjust thrust if <= 0, leaves neg alone and zero thrust stops motors
        if (raw > stabSettings.cruiseControl.cruise_control_min_thrust) {
            // quad    example factor of 2 at hover power of 40%: (0.4 - 0.0) * 2.0 + 0.0 = 0.8
            // CP heli example factor of 2 at hover stick of 60%: (0.6 - 0.5) * 2.0 + 0.5 = 0.7
            raw = boundf((raw - stabSettings.cruiseControl.cruise_control_neutral_thrust) * cruisecontrol_factor + stabSettings.cruiseControl.cruise_control_neutral_thrust,
                         stabSettings.cruiseControl.cruise_control_min_thrust,
                         stabSettings.cruiseControl.cruise_control_max_thrust);
        }
    }
    return raw;
}

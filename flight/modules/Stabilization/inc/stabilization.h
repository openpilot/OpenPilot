/**
 ******************************************************************************
 * @addtogroup OpenPilotModules OpenPilot Modules
 * @{
 * @addtogroup StabilizationModule Stabilization Module
 * @brief Stabilization PID loops in an airframe type independent manner
 * @note This object updates the @ref ActuatorDesired "Actuator Desired" based on the
 * PID loops on the @ref AttitudeDesired "Attitude Desired" and @ref AttitudeState "Attitude State"
 * @{
 *
 * @file       stabilization.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
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
#ifndef STABILIZATION_H
#define STABILIZATION_H

#include <openpilot.h>
#include <stabilizationsettings.h>

int32_t StabilizationInitialize();

typedef struct {
    StabilizationSettingsData settings;
    float gyro_alpha;
    struct {
        float  cruise_control_min_thrust;
        float  cruise_control_max_thrust;
        float  cruise_control_max_angle_cosine;
        float  cruise_control_max_power_factor;
        float  cruise_control_power_trim;
        int8_t cruise_control_inverted_power_switch; // WARNING: currently -1 is not fully implemented !!!
        float  cruise_control_neutral_thrust;
    }     cruiseControl;
    float rattitude_mode_transition_stick_position;
} StabilizationData;


extern StabilizationData stabSettings;


#endif // STABILIZATION_H

/**
 * @}
 * @}
 */

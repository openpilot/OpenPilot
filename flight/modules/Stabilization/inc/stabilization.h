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
#include <pid.h>
#include <stabilizationsettings.h>
#include <stabilizationbank.h>


int32_t StabilizationInitialize();

typedef struct {
    StabilizationSettingsData settings;
    StabilizationBankData     stabBank;
    float gyro_alpha;
    struct {
        float min_thrust;
        float max_thrust;
        float thrust_difference;
        float power_trim;
        float half_power_delay;
        float max_power_factor_angle;
    } cruiseControl;
    struct {
        int8_t gyroupdates;
        int8_t rateupdates;
    }     monitor;
    float rattitude_mode_transition_stick_position;
    struct pid innerPids[3], outerPids[3];
    // TPS [Roll,Pitch,Yaw][P,I,D]
    bool  thrust_pid_scaling_enabled[3][3];
} StabilizationData;


extern StabilizationData stabSettings;

#define AXES                4
#define FAILSAFE_TIMEOUT_MS 30

#ifndef PIOS_STABILIZATION_STACK_SIZE
#define STACK_SIZE_BYTES    800
#else
#define STACK_SIZE_BYTES    PIOS_STABILIZATION_STACK_SIZE
#endif

// must be same as eventdispatcher to avoid needing additional mutexes
#define CBTASK_PRIORITY     CALLBACK_TASK_FLIGHTCONTROL

// outer loop only executes every 4th uavobject update to save CPU
#define OUTERLOOP_SKIPCOUNT 4

#endif // STABILIZATION_H

/**
 * @}
 * @}
 */

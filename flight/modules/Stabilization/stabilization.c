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
 * @file       stabilization.c
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

#include <openpilot.h>
#include <pios_struct_helper.h>
#include <pid.h>
#include <manualcontrolcommand.h>
#include <flightmodesettings.h>
#include <stabilizationsettings.h>
#include <stabilizationdesired.h>
#include <stabilizationstatus.h>
#include <stabilizationbank.h>
#include <stabilizationsettingsbank1.h>
#include <stabilizationsettingsbank2.h>
#include <stabilizationsettingsbank3.h>
#include <relaytuning.h>
#include <relaytuningsettings.h>
#include <ratedesired.h>
#include <sin_lookup.h>
#include <stabilization.h>
#include <innerloop.h>
#include <outerloop.h>
#include <altitudeloop.h>


// Public variables
StabilizationData stabSettings;

// Private variables
static int cur_flight_mode = -1;

// Private functions
static void SettingsUpdatedCb(UAVObjEvent *ev);
static void BankUpdatedCb(UAVObjEvent *ev);
static void SettingsBankUpdatedCb(UAVObjEvent *ev);
static void FlightModeSwitchUpdatedCb(UAVObjEvent *ev);
static void StabilizationDesiredUpdatedCb(UAVObjEvent *ev);

/**
 * Module initialization
 */
int32_t StabilizationStart()
{
    StabilizationSettingsConnectCallback(SettingsUpdatedCb);
    ManualControlCommandConnectCallback(FlightModeSwitchUpdatedCb);
    StabilizationBankConnectCallback(BankUpdatedCb);
    StabilizationSettingsBank1ConnectCallback(SettingsBankUpdatedCb);
    StabilizationSettingsBank2ConnectCallback(SettingsBankUpdatedCb);
    StabilizationSettingsBank3ConnectCallback(SettingsBankUpdatedCb);
    StabilizationDesiredConnectCallback(StabilizationDesiredUpdatedCb);
    SettingsUpdatedCb(StabilizationSettingsHandle());
    StabilizationDesiredUpdatedCb(StabilizationDesiredHandle());
    FlightModeSwitchUpdatedCb(ManualControlCommandHandle());
    BankUpdatedCb(StabilizationBankHandle());

#ifdef PIOS_INCLUDE_WDG
    PIOS_WDG_RegisterFlag(PIOS_WDG_STABILIZATION);
#endif
    return 0;
}

/**
 * Module initialization
 */
int32_t StabilizationInitialize()
{
    // Initialize variables
    StabilizationDesiredInitialize();
    StabilizationSettingsInitialize();
    StabilizationStatusInitialize();
    StabilizationBankInitialize();
    StabilizationSettingsBank1Initialize();
    StabilizationSettingsBank2Initialize();
    StabilizationSettingsBank3Initialize();
    RateDesiredInitialize();
    ManualControlCommandInitialize(); // only used for PID bank selection based on flight mode switch
    // Code required for relay tuning
    sin_lookup_initalize();
    RelayTuningSettingsInitialize();
    RelayTuningInitialize();

    stabilizationOuterloopInit();
    stabilizationInnerloopInit();
#ifdef REVOLUTION
    stabilizationAltitudeloopInit();
#endif
    pid_zero(&stabSettings.outerPids[0]);
    pid_zero(&stabSettings.outerPids[1]);
    pid_zero(&stabSettings.outerPids[2]);
    pid_zero(&stabSettings.innerPids[0]);
    pid_zero(&stabSettings.innerPids[1]);
    pid_zero(&stabSettings.innerPids[2]);
    return 0;
}

MODULE_INITCALL(StabilizationInitialize, StabilizationStart);

static void StabilizationDesiredUpdatedCb(__attribute__((unused)) UAVObjEvent *ev)
{
    StabilizationStatusData status;
    StabilizationDesiredStabilizationModeData mode;
    int t;

    StabilizationDesiredStabilizationModeGet(&mode);
    for (t = 0; t < AXES; t++) {
        switch (cast_struct_to_array(mode, mode.Roll)[t]) {
        case STABILIZATIONDESIRED_STABILIZATIONMODE_MANUAL:
            cast_struct_to_array(status.OuterLoop, status.OuterLoop.Roll)[t] = STABILIZATIONSTATUS_OUTERLOOP_DIRECT;
            cast_struct_to_array(status.InnerLoop, status.InnerLoop.Roll)[t] = STABILIZATIONSTATUS_INNERLOOP_DIRECT;
            break;
        case STABILIZATIONDESIRED_STABILIZATIONMODE_RATE:
            cast_struct_to_array(status.OuterLoop, status.OuterLoop.Roll)[t] = STABILIZATIONSTATUS_OUTERLOOP_DIRECT;
            cast_struct_to_array(status.InnerLoop, status.InnerLoop.Roll)[t] = STABILIZATIONSTATUS_INNERLOOP_RATE;
            break;
        case STABILIZATIONDESIRED_STABILIZATIONMODE_ATTITUDE:
            cast_struct_to_array(status.OuterLoop, status.OuterLoop.Roll)[t] = STABILIZATIONSTATUS_OUTERLOOP_ATTITUDE;
            cast_struct_to_array(status.InnerLoop, status.InnerLoop.Roll)[t] = STABILIZATIONSTATUS_INNERLOOP_RATE;
            break;
        case STABILIZATIONDESIRED_STABILIZATIONMODE_AXISLOCK:
            cast_struct_to_array(status.OuterLoop, status.OuterLoop.Roll)[t] = STABILIZATIONSTATUS_OUTERLOOP_DIRECT;
            cast_struct_to_array(status.InnerLoop, status.InnerLoop.Roll)[t] = STABILIZATIONSTATUS_INNERLOOP_AXISLOCK;
            break;
        case STABILIZATIONDESIRED_STABILIZATIONMODE_WEAKLEVELING:
            cast_struct_to_array(status.OuterLoop, status.OuterLoop.Roll)[t] = STABILIZATIONSTATUS_OUTERLOOP_WEAKLEVELING;
            cast_struct_to_array(status.InnerLoop, status.InnerLoop.Roll)[t] = STABILIZATIONSTATUS_INNERLOOP_RATE;
            break;
        case STABILIZATIONDESIRED_STABILIZATIONMODE_VIRTUALBAR:
            cast_struct_to_array(status.OuterLoop, status.OuterLoop.Roll)[t] = STABILIZATIONSTATUS_OUTERLOOP_DIRECT;
            cast_struct_to_array(status.InnerLoop, status.InnerLoop.Roll)[t] = STABILIZATIONSTATUS_INNERLOOP_VIRTUALFLYBAR;
            break;
        case STABILIZATIONDESIRED_STABILIZATIONMODE_RATTITUDE:
            cast_struct_to_array(status.OuterLoop, status.OuterLoop.Roll)[t] = STABILIZATIONSTATUS_OUTERLOOP_RATTITUDE;
            cast_struct_to_array(status.InnerLoop, status.InnerLoop.Roll)[t] = STABILIZATIONSTATUS_INNERLOOP_RATE;
            break;
        case STABILIZATIONDESIRED_STABILIZATIONMODE_RELAYRATE:
            cast_struct_to_array(status.OuterLoop, status.OuterLoop.Roll)[t] = STABILIZATIONSTATUS_OUTERLOOP_DIRECT;
            cast_struct_to_array(status.InnerLoop, status.InnerLoop.Roll)[t] = STABILIZATIONSTATUS_INNERLOOP_RELAYTUNING;
            break;
        case STABILIZATIONDESIRED_STABILIZATIONMODE_RELAYATTITUDE:
            cast_struct_to_array(status.OuterLoop, status.OuterLoop.Roll)[t] = STABILIZATIONSTATUS_OUTERLOOP_ATTITUDE;
            cast_struct_to_array(status.InnerLoop, status.InnerLoop.Roll)[t] = STABILIZATIONSTATUS_INNERLOOP_RELAYTUNING;
            break;
        case STABILIZATIONDESIRED_STABILIZATIONMODE_ALTITUDEHOLD:
            cast_struct_to_array(status.OuterLoop, status.OuterLoop.Roll)[t] = STABILIZATIONSTATUS_OUTERLOOP_ALTITUDE;
            cast_struct_to_array(status.InnerLoop, status.InnerLoop.Roll)[t] = STABILIZATIONSTATUS_INNERLOOP_CRUISECONTROL;
            break;
        case STABILIZATIONDESIRED_STABILIZATIONMODE_ALTITUDEVARIO:
            cast_struct_to_array(status.OuterLoop, status.OuterLoop.Roll)[t] = STABILIZATIONSTATUS_OUTERLOOP_ALTITUDEVARIO;
            cast_struct_to_array(status.InnerLoop, status.InnerLoop.Roll)[t] = STABILIZATIONSTATUS_INNERLOOP_CRUISECONTROL;
            break;
        case STABILIZATIONDESIRED_STABILIZATIONMODE_CRUISECONTROL:
            cast_struct_to_array(status.OuterLoop, status.OuterLoop.Roll)[t] = STABILIZATIONSTATUS_OUTERLOOP_DIRECT;
            cast_struct_to_array(status.InnerLoop, status.InnerLoop.Roll)[t] = STABILIZATIONSTATUS_INNERLOOP_CRUISECONTROL;
            break;
        }
    }
    StabilizationStatusSet(&status);
}

static void FlightModeSwitchUpdatedCb(__attribute__((unused)) UAVObjEvent *ev)
{
    uint8_t fm;

    ManualControlCommandFlightModeSwitchPositionGet(&fm);

    if (fm == cur_flight_mode) {
        return;
    }
    cur_flight_mode = fm;
    SettingsBankUpdatedCb(NULL);
}

static void SettingsBankUpdatedCb(__attribute__((unused)) UAVObjEvent *ev)
{
    if (cur_flight_mode < 0 || cur_flight_mode >= FLIGHTMODESETTINGS_FLIGHTMODEPOSITION_NUMELEM) {
        return;
    }
    if ((ev) && ((stabSettings.settings.FlightModeMap[cur_flight_mode] == 0 && ev->obj != StabilizationSettingsBank1Handle()) ||
                 (stabSettings.settings.FlightModeMap[cur_flight_mode] == 1 && ev->obj != StabilizationSettingsBank2Handle()) ||
                 (stabSettings.settings.FlightModeMap[cur_flight_mode] == 2 && ev->obj != StabilizationSettingsBank3Handle()) ||
                 stabSettings.settings.FlightModeMap[cur_flight_mode] > 2)) {
        return;
    }


    switch (stabSettings.settings.FlightModeMap[cur_flight_mode]) {
    case 0:
        StabilizationSettingsBank1Get((StabilizationSettingsBank1Data *)&stabSettings.stabBank);
        break;

    case 1:
        StabilizationSettingsBank2Get((StabilizationSettingsBank2Data *)&stabSettings.stabBank);
        break;

    case 2:
        StabilizationSettingsBank3Get((StabilizationSettingsBank3Data *)&stabSettings.stabBank);
        break;
    }
    StabilizationBankSet(&stabSettings.stabBank);
}

static void BankUpdatedCb(__attribute__((unused)) UAVObjEvent *ev)
{
    StabilizationBankGet(&stabSettings.stabBank);

    // Set the roll rate PID constants
    pid_configure(&stabSettings.innerPids[0], stabSettings.stabBank.RollRatePID.Kp,
                  stabSettings.stabBank.RollRatePID.Ki,
                  stabSettings.stabBank.RollRatePID.Kd,
                  stabSettings.stabBank.RollRatePID.ILimit);

    // Set the pitch rate PID constants
    pid_configure(&stabSettings.innerPids[1], stabSettings.stabBank.PitchRatePID.Kp,
                  stabSettings.stabBank.PitchRatePID.Ki,
                  stabSettings.stabBank.PitchRatePID.Kd,
                  stabSettings.stabBank.PitchRatePID.ILimit);

    // Set the yaw rate PID constants
    pid_configure(&stabSettings.innerPids[2], stabSettings.stabBank.YawRatePID.Kp,
                  stabSettings.stabBank.YawRatePID.Ki,
                  stabSettings.stabBank.YawRatePID.Kd,
                  stabSettings.stabBank.YawRatePID.ILimit);

    // Set the roll attitude PI constants
    pid_configure(&stabSettings.outerPids[0], stabSettings.stabBank.RollPI.Kp,
                  stabSettings.stabBank.RollPI.Ki,
                  0,
                  stabSettings.stabBank.RollPI.ILimit);

    // Set the pitch attitude PI constants
    pid_configure(&stabSettings.outerPids[1], stabSettings.stabBank.PitchPI.Kp,
                  stabSettings.stabBank.PitchPI.Ki,
                  0,
                  stabSettings.stabBank.PitchPI.ILimit);

    // Set the yaw attitude PI constants
    pid_configure(&stabSettings.outerPids[2], stabSettings.stabBank.YawPI.Kp,
                  stabSettings.stabBank.YawPI.Ki,
                  0,
                  stabSettings.stabBank.YawPI.ILimit);
}


static void SettingsUpdatedCb(__attribute__((unused)) UAVObjEvent *ev)
{
    // needs no mutex, as long as eventdispatcher and Stabilization are both TASK_PRIORITY_CRITICAL
    StabilizationSettingsGet(&stabSettings.settings);

    // Set up the derivative term
    pid_configure_derivative(stabSettings.settings.DerivativeCutoff, stabSettings.settings.DerivativeGamma);

    // The dT has some jitter iteration to iteration that we don't want to
    // make thie result unpredictable.  Still, it's nicer to specify the constant
    // based on a time (in ms) rather than a fixed multiplier.  The error between
    // update rates on OP (~300 Hz) and CC (~475 Hz) is negligible for this
    // calculation
    const float fakeDt = 0.0025f;
    if (stabSettings.settings.GyroTau < 0.0001f) {
        stabSettings.gyro_alpha = 0; // not trusting this to resolve to 0
    } else {
        stabSettings.gyro_alpha = expf(-fakeDt / stabSettings.settings.GyroTau);
    }

    // force flight mode update
    cur_flight_mode = -1;

    // Rattitude stick angle where the attitude to rate transition happens
    if (stabSettings.settings.RattitudeModeTransition < (uint8_t)10) {
        stabSettings.rattitude_mode_transition_stick_position = 10.0f / 100.0f;
    } else {
        stabSettings.rattitude_mode_transition_stick_position = (float)stabSettings.settings.RattitudeModeTransition / 100.0f;
    }

    stabSettings.cruiseControl.min_thrust = (float)stabSettings.settings.CruiseControlMinThrust / 100.0f;
    stabSettings.cruiseControl.max_thrust = (float)stabSettings.settings.CruiseControlMaxThrust / 100.0f;
    stabSettings.cruiseControl.thrust_difference = stabSettings.cruiseControl.max_thrust - stabSettings.cruiseControl.min_thrust;

    stabSettings.cruiseControl.power_trim = stabSettings.settings.CruiseControlPowerTrim / 100.0f;
    stabSettings.cruiseControl.half_power_delay = stabSettings.settings.CruiseControlPowerDelayComp / 2.0f;
    stabSettings.cruiseControl.max_power_factor_angle = RAD2DEG(acosf(1.0f / stabSettings.settings.CruiseControlMaxPowerFactor));
}

/**
 * @}
 * @}
 */

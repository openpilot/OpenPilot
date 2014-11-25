/**
 ******************************************************************************
 * @addtogroup OpenPilotModules OpenPilot Modules
 * @{
 * @addtogroup ManualControl
 * @brief Interpretes the control input in ManualControlCommand
 * @{
 *
 * @file       stabilizedhandler.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2014.
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

#include "inc/manualcontrol.h"
#include <mathmisc.h>
#include <manualcontrolcommand.h>
#include <stabilizationdesired.h>
#include <flightmodesettings.h>
#include <stabilizationbank.h>

// Private constants

// Private types

// Private functions
static float applyExpo(float value, float expo);


static float applyExpo(float value, float expo)
{
    // note: fastPow makes a small error, therefore result needs to be bound
    float exp = boundf(fastPow(1.00695f, expo), 0.5f, 2.0f);

    // magic number scales expo
    // so that
    // expo=100 yields value**10
    // expo=0 yields value**1
    // expo=-100 yields value**(1/10)
    // (pow(2.0,1/100)~=1.00695)
    if (value > 0.0f) {
        return boundf(fastPow(value, exp), 0.0f, 1.0f);
    } else if (value < -0.0f) {
        return boundf(-fastPow(-value, exp), -1.0f, 0.0f);
    } else {
        return 0.0f;
    }
}


/**
 * @brief Handler to control Stabilized flightmodes. FlightControl is governed by "Stabilization"
 * @input: ManualControlCommand
 * @output: StabilizationDesired
 */
void stabilizedHandler(bool newinit)
{
    if (newinit) {
        StabilizationDesiredInitialize();
        StabilizationBankInitialize();
    }
    ManualControlCommandData cmd;
    ManualControlCommandGet(&cmd);

    FlightModeSettingsData settings;
    FlightModeSettingsGet(&settings);

    StabilizationDesiredData stabilization;
    StabilizationDesiredGet(&stabilization);

    StabilizationBankData stabSettings;
    StabilizationBankGet(&stabSettings);

    cmd.Roll  = applyExpo(cmd.Roll, stabSettings.StickExpo.Roll);
    cmd.Pitch = applyExpo(cmd.Pitch, stabSettings.StickExpo.Pitch);
    cmd.Yaw   = applyExpo(cmd.Yaw, stabSettings.StickExpo.Yaw);
    uint8_t *stab_settings;
    FlightStatusData flightStatus;
    FlightStatusGet(&flightStatus);

    switch (flightStatus.FlightMode) {
    case FLIGHTSTATUS_FLIGHTMODE_STABILIZED1:
        stab_settings = FlightModeSettingsStabilization1SettingsToArray(settings.Stabilization1Settings);
        break;
    case FLIGHTSTATUS_FLIGHTMODE_STABILIZED2:
        stab_settings = FlightModeSettingsStabilization2SettingsToArray(settings.Stabilization2Settings);
        break;
    case FLIGHTSTATUS_FLIGHTMODE_STABILIZED3:
        stab_settings = FlightModeSettingsStabilization3SettingsToArray(settings.Stabilization3Settings);
        break;
    case FLIGHTSTATUS_FLIGHTMODE_STABILIZED4:
        stab_settings = FlightModeSettingsStabilization4SettingsToArray(settings.Stabilization4Settings);
        break;
    case FLIGHTSTATUS_FLIGHTMODE_STABILIZED5:
        stab_settings = FlightModeSettingsStabilization5SettingsToArray(settings.Stabilization5Settings);
        break;
    case FLIGHTSTATUS_FLIGHTMODE_STABILIZED6:
        stab_settings = FlightModeSettingsStabilization6SettingsToArray(settings.Stabilization6Settings);
        break;
    default:
        // Major error, this should not occur because only enter this block when one of these is true
        AlarmsSet(SYSTEMALARMS_ALARM_MANUALCONTROL, SYSTEMALARMS_ALARM_CRITICAL);
        stab_settings = FlightModeSettingsStabilization1SettingsToArray(settings.Stabilization1Settings);
        return;
    }

    stabilization.Roll =
        (stab_settings[0] == STABILIZATIONDESIRED_STABILIZATIONMODE_MANUAL) ? cmd.Roll :
        (stab_settings[0] == STABILIZATIONDESIRED_STABILIZATIONMODE_RATE) ? cmd.Roll * stabSettings.ManualRate.Roll :
        (stab_settings[0] == STABILIZATIONDESIRED_STABILIZATIONMODE_WEAKLEVELING) ? cmd.Roll * stabSettings.RollMax :
        (stab_settings[0] == STABILIZATIONDESIRED_STABILIZATIONMODE_ATTITUDE) ? cmd.Roll * stabSettings.RollMax :
        (stab_settings[0] == STABILIZATIONDESIRED_STABILIZATIONMODE_AXISLOCK) ? cmd.Roll * stabSettings.ManualRate.Roll :
        (stab_settings[0] == STABILIZATIONDESIRED_STABILIZATIONMODE_VIRTUALBAR) ? cmd.Roll :
        (stab_settings[0] == STABILIZATIONDESIRED_STABILIZATIONMODE_ACRO) ? cmd.Roll * stabSettings.ManualRate.Roll :
        (stab_settings[0] == STABILIZATIONDESIRED_STABILIZATIONMODE_RATTITUDE) ? cmd.Roll * stabSettings.RollMax :
        0; // this is an invalid mode

    stabilization.Pitch =
        (stab_settings[1] == STABILIZATIONDESIRED_STABILIZATIONMODE_MANUAL) ? cmd.Pitch :
        (stab_settings[1] == STABILIZATIONDESIRED_STABILIZATIONMODE_RATE) ? cmd.Pitch * stabSettings.ManualRate.Pitch :
        (stab_settings[1] == STABILIZATIONDESIRED_STABILIZATIONMODE_WEAKLEVELING) ? cmd.Pitch * stabSettings.PitchMax :
        (stab_settings[1] == STABILIZATIONDESIRED_STABILIZATIONMODE_ATTITUDE) ? cmd.Pitch * stabSettings.PitchMax :
        (stab_settings[1] == STABILIZATIONDESIRED_STABILIZATIONMODE_AXISLOCK) ? cmd.Pitch * stabSettings.ManualRate.Pitch :
        (stab_settings[1] == STABILIZATIONDESIRED_STABILIZATIONMODE_VIRTUALBAR) ? cmd.Pitch :
        (stab_settings[1] == STABILIZATIONDESIRED_STABILIZATIONMODE_ACRO) ? cmd.Pitch * stabSettings.ManualRate.Pitch :
        (stab_settings[1] == STABILIZATIONDESIRED_STABILIZATIONMODE_RATTITUDE) ? cmd.Pitch * stabSettings.PitchMax :
        0; // this is an invalid mode

    // TOOD: Add assumption about order of stabilization desired and manual control stabilization mode fields having same order
    stabilization.StabilizationMode.Roll  = stab_settings[0];
    stabilization.StabilizationMode.Pitch = stab_settings[1];
    // Other axes (yaw) cannot be Rattitude, so use Rate
    // Should really do this for Attitude mode as well?
    if (stab_settings[2] == STABILIZATIONDESIRED_STABILIZATIONMODE_RATTITUDE) {
        stabilization.StabilizationMode.Yaw = STABILIZATIONDESIRED_STABILIZATIONMODE_RATE;
        stabilization.Yaw = cmd.Yaw * stabSettings.ManualRate.Yaw;
    } else {
        stabilization.StabilizationMode.Yaw = stab_settings[2];
        stabilization.Yaw =
            (stab_settings[2] == STABILIZATIONDESIRED_STABILIZATIONMODE_MANUAL) ? cmd.Yaw :
            (stab_settings[2] == STABILIZATIONDESIRED_STABILIZATIONMODE_RATE) ? cmd.Yaw * stabSettings.ManualRate.Yaw :
            (stab_settings[2] == STABILIZATIONDESIRED_STABILIZATIONMODE_WEAKLEVELING) ? cmd.Yaw * stabSettings.YawMax :
            (stab_settings[2] == STABILIZATIONDESIRED_STABILIZATIONMODE_ATTITUDE) ? cmd.Yaw * stabSettings.YawMax :
            (stab_settings[2] == STABILIZATIONDESIRED_STABILIZATIONMODE_AXISLOCK) ? cmd.Yaw * stabSettings.ManualRate.Yaw :
            (stab_settings[2] == STABILIZATIONDESIRED_STABILIZATIONMODE_VIRTUALBAR) ? cmd.Yaw :
            (stab_settings[2] == STABILIZATIONDESIRED_STABILIZATIONMODE_ACRO) ? cmd.Yaw * stabSettings.ManualRate.Yaw :
            (stab_settings[2] == STABILIZATIONDESIRED_STABILIZATIONMODE_RATTITUDE) ? cmd.Yaw * stabSettings.YawMax :
            0; // this is an invalid mode
    }

    // Adjust thrust mode as required if position roam
    if (flightStatus.PositionRoamState != FLIGHTSTATUS_POSITIONROAMSTATE_NONE) {
        //   are we manual thrust in vtol?
	if (flightStatus.PositionRoamThrustMode == FLIGHTSTATUS_POSITIONROAMTHRUSTMODE_MANUAL) {
            stabilization.StabilizationMode.Thrust = STABILIZATIONDESIRED_STABILIZATIONMODE_MANUAL;
	}
	else {
            stabilization.StabilizationMode.Thrust = STABILIZATIONDESIRED_STABILIZATIONMODE_ALTITUDEVARIO;
	}
    }
    else {
        stabilization.StabilizationMode.Thrust = stab_settings[3];
    }
    stabilization.Thrust = cmd.Thrust;
    StabilizationDesiredSet(&stabilization);
}


/**
 * @}
 * @}
 */

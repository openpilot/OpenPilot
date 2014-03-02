/**
 ******************************************************************************
 * @addtogroup OpenPilotModules OpenPilot Modules
 * @{
 * @addtogroup ManualControl
 * @brief Interpretes the control input in ManualControlCommand
 * @{
 *
 * @file       altitudehandler.c
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
#include <manualcontrolcommand.h>
#include <stabilizationbank.h>
#include <altitudeholddesired.h>
#include <altitudeholdsettings.h>
#include <positionstate.h>

#if defined(REVOLUTION)
// Private constants

// Private types

// Private functions

/**
 * @brief Handler to control deprecated flight modes controlled by AltitudeHold module
 * @input: ManualControlCommand
 * @output: AltitudeHoldDesired
 */
void altitudeHandler(bool newinit)
{
    const float DEADBAND      = 0.20f;
    const float DEADBAND_HIGH = 1.0f / 2 + DEADBAND / 2;
    const float DEADBAND_LOW  = 1.0f / 2 - DEADBAND / 2;

    if (newinit) {
        StabilizationBankInitialize();
        AltitudeHoldDesiredInitialize();
        AltitudeHoldSettingsInitialize();
        PositionStateInitialize();
    }


    // this is the max speed in m/s at the extents of thrust
    float thrustRate;
    uint8_t thrustExp;

    static uint8_t flightMode;
    static bool newaltitude = true;

    ManualControlCommandData cmd;
    ManualControlCommandGet(&cmd);

    FlightStatusFlightModeGet(&flightMode);

    AltitudeHoldDesiredData altitudeHoldDesiredData;
    AltitudeHoldDesiredGet(&altitudeHoldDesiredData);

    AltitudeHoldSettingsThrustExpGet(&thrustExp);
    AltitudeHoldSettingsThrustRateGet(&thrustRate);

    StabilizationBankData stabSettings;
    StabilizationBankGet(&stabSettings);

    PositionStateData posState;
    PositionStateGet(&posState);

    altitudeHoldDesiredData.Roll  = cmd.Roll * stabSettings.RollMax;
    altitudeHoldDesiredData.Pitch = cmd.Pitch * stabSettings.PitchMax;
    altitudeHoldDesiredData.Yaw   = cmd.Yaw * stabSettings.ManualRate.Yaw;

    if (newinit) {
        newaltitude = true;
    }

    uint8_t cutOff;
    AltitudeHoldSettingsCutThrustWhenZeroGet(&cutOff);
    if (cutOff && cmd.Thrust < 0) {
        // Cut thrust if desired
        altitudeHoldDesiredData.SetPoint    = cmd.Thrust;
        altitudeHoldDesiredData.ControlMode = ALTITUDEHOLDDESIRED_CONTROLMODE_THRUST;
        newaltitude = true;
    } else if (flightMode == FLIGHTSTATUS_FLIGHTMODE_ALTITUDEVARIO && cmd.Thrust > DEADBAND_HIGH) {
        // being the two band symmetrical I can divide by DEADBAND_LOW to scale it to a value betweeon 0 and 1
        // then apply an "exp" f(x,k) = (k*x*x*x + (255-k)*x) / 255
        altitudeHoldDesiredData.SetPoint    = -((thrustExp * powf((cmd.Thrust - DEADBAND_HIGH) / (DEADBAND_LOW), 3) + (255 - thrustExp) * (cmd.Thrust - DEADBAND_HIGH) / DEADBAND_LOW) / 255 * thrustRate);
        altitudeHoldDesiredData.ControlMode = ALTITUDEHOLDDESIRED_CONTROLMODE_VELOCITY;
        newaltitude = true;
    } else if (flightMode == FLIGHTSTATUS_FLIGHTMODE_ALTITUDEVARIO && cmd.Thrust < DEADBAND_LOW) {
        altitudeHoldDesiredData.SetPoint    = -(-(thrustExp * powf((DEADBAND_LOW - (cmd.Thrust < 0 ? 0 : cmd.Thrust)) / DEADBAND_LOW, 3) + (255 - thrustExp) * (DEADBAND_LOW - cmd.Thrust) / DEADBAND_LOW) / 255 * thrustRate);
        altitudeHoldDesiredData.ControlMode = ALTITUDEHOLDDESIRED_CONTROLMODE_VELOCITY;
        newaltitude = true;
    } else if (newaltitude == true) {
        altitudeHoldDesiredData.SetPoint    = posState.Down;
        altitudeHoldDesiredData.ControlMode = ALTITUDEHOLDDESIRED_CONTROLMODE_ALTITUDE;
        newaltitude = false;
    }

    AltitudeHoldDesiredSet(&altitudeHoldDesiredData);
}

#else /* if defined(REVOLUTION) */
void altitudeHandler(__attribute__((unused)) bool newinit)
{
    PIOS_Assert(0); // should not be called
}
#endif // REVOLUTION


/**
 * @}
 * @}
 */

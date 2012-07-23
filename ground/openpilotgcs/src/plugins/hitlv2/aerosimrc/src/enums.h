/**
 ******************************************************************************
 *
 * @file       enums.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010-2012.
 * @addtogroup 3rdParty Third-party integration
 * @{
 * @addtogroup AeroSimRC AeroSimRC proxy plugin
 * @{
 * @brief AeroSimRC simulator to HITL proxy plugin
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

#ifndef ENUMS_H
#define ENUMS_H

// Custom Menu Item masks
enum MenuMasks {
    MenuEnable      = (1 << 0),
    MenuTx          = (1 << 1),
    MenuRx          = (1 << 2),
    MenuScreen      = (1 << 3),
    MenuNextWpt     = (1 << 4),
    MenuCmdReset    = (1 << 5),
    MenuLedBlue     = (1 << 6),
    MenuLedGreen    = (1 << 7),
    MenuFMode1      = (1 << 8),
    MenuFMode2      = (1 << 9),
    MenuFMode3      = (1 << 10)
};

enum EOverrideFlags
{
    OVR_POS             = (1 << 0),
    OVR_VEL             = (1 << 1),
    OVR_ANG_VEL         = (1 << 2),
    OVR_HPR             = (1 << 3),  // Override Heading, Pitch and Roll
    OVR_WIND_VEL        = (1 << 4),  // Override Wind velocity at model
    OVR_ENGINE_RPM      = (1 << 5),  // Override RPM of all Engines or Motors
    OVR_BAT_VOLT        = (1 << 6),  // Override motor Battery Voltage
    OVR_BAT_AMP         = (1 << 7),  // Override motor Battery current
    OVR_BAT_AH_CONSUMED = (1 << 8),  // Override motor Battery AmpsHour consumed
    OVR_FUEL_CONSUMED   = (1 << 9)   // Override Fuel consumed (gas & jet engines)
};

enum Channels {
    Ch1Aileron,
    Ch2Elevator,
    Ch3Throttle,
    Ch4Rudder,
    Ch5,
    Ch6,
    Ch7,
    Ch8,
    Ch9,
    Ch10Retracts,
    Ch11Flaps,
    Ch12FPVCamPan,
    Ch13FPVCamTilt,
    Ch14Brakes,
    Ch15Spoilers,
    Ch16Smoke,
    Ch17Fire,
    Ch18FlightMode,
    Ch19ALTHold,
    Ch20FPVTiltHold,
    Ch21ResetModel,
    Ch22MouseTX,
    Ch23Plugin1,
    Ch24Plugin2,
    Ch25ThrottleHold,
    Ch26CareFree,
    Ch27FPVCamRoll,
    Ch28LMotorDual,
    Ch29RMotorDual,
    Ch30Mix,
    Ch31Mix,
    Ch32Mix,
    Ch33Mix,
    Ch34Mix,
    Ch35Mix,
    Ch36Mix,
    Ch37Mix,
    Ch38Mix,
    Ch39Mix
};

#endif // ENUMS_H

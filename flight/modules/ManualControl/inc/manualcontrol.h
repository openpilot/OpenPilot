/**
 ******************************************************************************
 * @addtogroup OpenPilotModules OpenPilot Modules
 * @{
 * @addtogroup ManualControlModule Manual Control Module
 * @{
 *
 * @file       manualcontrol.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2014.
 * @brief      ManualControl module. Handles safety R/C link and flight mode.
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
#ifndef MANUALCONTROL_H
#define MANUALCONTROL_H

#include <openpilot.h>
#include <flightstatus.h>

typedef void (*handlerFunc)(bool newinit);

typedef struct controlHandlerStruct {
    FlightStatusControlChainData controlChain;
    handlerFunc handler;
} controlHandler;

/**
 * @brief Handler to interprete Command inputs in regards to arming/disarming (called in all flight modes)
 * @input: ManualControlCommand, AccessoryDesired
 * @output: FlightStatus.Arming
 */
void armHandler(bool newinit);

/**
 * @brief Handler to control Manual flightmode - input directly steers actuators
 * @input: ManualControlCommand
 * @output: ActuatorDesired
 */
void manualHandler(bool newinit);

/**
 * @brief Handler to control Stabilized flightmodes. FlightControl is governed by "Stabilization"
 * @input: ManualControlCommand
 * @output: StabilizationDesired
 */
void stabilizedHandler(bool newinit);

/**
 * @brief Handler to control Guided flightmodes. FlightControl is governed by PathFollower, control via PathDesired
 * @input: NONE: fully automated mode -- TODO recursively call handler for advanced stick commands
 * @output: PathDesired
 */
void pathFollowerHandler(bool newinit);

/**
 * @brief Handler to control Navigated flightmodes. FlightControl is governed by PathFollower, controlled indirectly via PathPlanner
 * @input: NONE: fully automated mode -- TODO recursively call handler for advanced stick commands to affect navigation
 * @output: NONE
 */
void pathPlannerHandler(bool newinit);

/**
 * @brief Handler to setup takeofflocation on arming. it is set up during Arming
 * @input: NONE:
 * @output: NONE
 */
void takeOffLocationHandler();

/**
 * @brief Initialize TakeoffLocationHanlder
 */
void takeOffLocationHandlerInit();


#define STABILIZATIONMODE_TABLE(ENTRY) \
    ENTRY(MANUAL) \
    ENTRY(RATE) \
    ENTRY(ATTITUDE) \
    ENTRY(AXISLOCK) \
    ENTRY(WEAKLEVELING) \
    ENTRY(VIRTUALBAR) \
    ENTRY(ACRO) \
    ENTRY(RATTITUDE) \
    ENTRY(ALTITUDEHOLD) \
    ENTRY(ALTITUDEVARIO) \
    ENTRY(CRUISECONTROL)

#define EXPAND_FLIGHTSETTINGS_STAB1(x) ((int)FLIGHTMODESETTINGS_STABILIZATION1SETTINGS_##x == (int)STABILIZATIONDESIRED_STABILIZATIONMODE_##x) &&
#define EXPAND_FLIGHTSETTINGS_STAB2(x) ((int)FLIGHTMODESETTINGS_STABILIZATION2SETTINGS_##x == (int)STABILIZATIONDESIRED_STABILIZATIONMODE_##x) &&
#define EXPAND_FLIGHTSETTINGS_STAB3(x) ((int)FLIGHTMODESETTINGS_STABILIZATION3SETTINGS_##x == (int)STABILIZATIONDESIRED_STABILIZATIONMODE_##x) &&
#define EXPAND_FLIGHTSETTINGS_STAB4(x) ((int)FLIGHTMODESETTINGS_STABILIZATION4SETTINGS_##x == (int)STABILIZATIONDESIRED_STABILIZATIONMODE_##x) &&
#define EXPAND_FLIGHTSETTINGS_STAB5(x) ((int)FLIGHTMODESETTINGS_STABILIZATION5SETTINGS_##x == (int)STABILIZATIONDESIRED_STABILIZATIONMODE_##x) &&
#define EXPAND_FLIGHTSETTINGS_STAB6(x) ((int)FLIGHTMODESETTINGS_STABILIZATION6SETTINGS_##x == (int)STABILIZATIONDESIRED_STABILIZATIONMODE_##x) &&

#define FLIGHTMODE_TABLE(ENTRY) \
    ENTRY(MANUAL) \
    ENTRY(STABILIZED1) \
    ENTRY(STABILIZED2) \
    ENTRY(STABILIZED3) \
    ENTRY(STABILIZED4) \
    ENTRY(STABILIZED5) \
    ENTRY(STABILIZED6) \
    ENTRY(POSITIONHOLD) \
    ENTRY(RETURNTOBASE) \
    ENTRY(LAND) \
    ENTRY(PATHPLANNER) \
    ENTRY(POI)

#define EXPAND_FLIGHTMODE(x)           ((int)FLIGHTMODESETTINGS_FLIGHTMODEPOSITION_##x == (int)FLIGHTSTATUS_FLIGHTMODE_##x) &&
/*
 * These are assumptions we make in the flight code about the order of settings and their consistency between
 * objects.  Please keep this synchronized to the UAVObjects
 */
#define assumptions1 \
    ( \
        STABILIZATIONMODE_TABLE(EXPAND_FLIGHTSETTINGS_STAB1) \
        1 \
    )

#define assumptions2 \
    ( \
        STABILIZATIONMODE_TABLE(EXPAND_FLIGHTSETTINGS_STAB2) \
        1 \
    )

#define assumptions3 \
    ( \
        STABILIZATIONMODE_TABLE(EXPAND_FLIGHTSETTINGS_STAB3) \
        1 \
    )

#define assumptions4 \
    ( \
        STABILIZATIONMODE_TABLE(EXPAND_FLIGHTSETTINGS_STAB4) \
        1 \
    )

#define assumptions5 \
    ( \
        STABILIZATIONMODE_TABLE(EXPAND_FLIGHTSETTINGS_STAB5) \
        1 \
    )

#define assumptions6 \
    ( \
        STABILIZATIONMODE_TABLE(EXPAND_FLIGHTSETTINGS_STAB6) \
        1 \
    )

#define assumptions7 \
    ( \
        ((int)FLIGHTMODESETTINGS_STABILIZATION1SETTINGS_NUMELEM == (int)FLIGHTMODESETTINGS_STABILIZATION2SETTINGS_NUMELEM) && \
        ((int)FLIGHTMODESETTINGS_STABILIZATION1SETTINGS_NUMELEM == (int)FLIGHTMODESETTINGS_STABILIZATION3SETTINGS_NUMELEM) && \
        ((int)FLIGHTMODESETTINGS_STABILIZATION1SETTINGS_NUMELEM == (int)FLIGHTMODESETTINGS_STABILIZATION4SETTINGS_NUMELEM) && \
        ((int)FLIGHTMODESETTINGS_STABILIZATION1SETTINGS_NUMELEM == (int)FLIGHTMODESETTINGS_STABILIZATION5SETTINGS_NUMELEM) && \
        ((int)FLIGHTMODESETTINGS_STABILIZATION1SETTINGS_NUMELEM == (int)FLIGHTMODESETTINGS_STABILIZATION6SETTINGS_NUMELEM) && \
        1 \
    )

#define assumptions_flightmode \
    ( \
        FLIGHTMODE_TABLE(EXPAND_FLIGHTMODE) \
        1 \
    )

#endif // MANUALCONTROL_H

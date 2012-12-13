/**
 ******************************************************************************
 *
 * @file       recovery.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      Recovery module to be used as a template for actual modules.
 *             Event callback version.
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

/**
 * Input objects: FlightStatus, RecoverySettings
 * Output object: StabilizationDesired
 *
 */

#include "openpilot.h"
#include "flightstatus.h"
#include "recoverysettings.h"
#include "stabilizationdesired.h"

static void RecoverySettingsUpdatedCb(UAVObjEvent * ev);
static void FlightStatusUpdatedCb(UAVObjEvent * ev);

static RecoverySettingsData settings;

int32_t RecoveryStart(void)
{
	// Listen for RecoveryObject1 updates, connect a callback function
	RecoverySettingsConnectCallback(&RecoverySettingsUpdatedCb);
	FlightStatusConnectCallback(&FlightStatusUpdatedCb);
	return 0;
}

int32_t RecoveryInitialize(void)
{
	RecoverySettingsInitialize();
	FlightStatusInitialize();
	StabilizationDesiredInitialize();
	return 0;
}
MODULE_INITCALL(RecoveryInitialize, RecoveryStart)

/**
 * This function is called each time RecoverySettings is updated
 */
static void RecoverySettingsUpdatedCb(UAVObjEvent * ev)
{
	RecoverySettingsGet(&settings);
}

/**
 * This function is called each time FlightStatus is updated
 */
static void FlightStatusUpdatedCb(UAVObjEvent * ev)
{
	FlightStatusData flightStatus;
	StabilizationDesiredData stabDesired;
	FlightStatusGet(&flightStatus);

	if (flightStatus.FlightMode != FLIGHTSTATUS_FLIGHTMODE_RECOVERY) return;

	StabilizationDesiredGet(&stabDesired);

	stabDesired.Roll = settings.Roll;
	stabDesired.Pitch = settings.Pitch;
	stabDesired.Yaw = 0;
	stabDesired.Throttle = settings.Throttle;

	stabDesired.StabilizationMode[STABILIZATIONDESIRED_STABILIZATIONMODE_ROLL] = STABILIZATIONDESIRED_STABILIZATIONMODE_ATTITUDE;
	stabDesired.StabilizationMode[STABILIZATIONDESIRED_STABILIZATIONMODE_PITCH] = STABILIZATIONDESIRED_STABILIZATIONMODE_ATTITUDE;
	stabDesired.StabilizationMode[STABILIZATIONDESIRED_STABILIZATIONMODE_YAW] = STABILIZATIONDESIRED_STABILIZATIONMODE_AXISLOCK;
	
	StabilizationDesiredSet(&stabDesired);

}


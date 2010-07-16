/**
 ******************************************************************************
 *
 * @file       manualcontrol.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
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

#include "openpilot.h"
#include "manualcontrol.h"
#include "manualcontrolsettings.h"
#include "stabilizationsettings.h"
#include "manualcontrolcommand.h"
#include "actuatordesired.h"
#include "attitudedesired.h"

/**
 * @addtogroup OpenPilotModules OpenPilot Modules
 * @{ 
 */

/**
 * @addtogroup ManualControlModule Manual Control Module
 * @brief Provide manual control or allow it alter flight mode
 *
 * Reads in the ManualControlCommand FlightMode setting from receiver then either 
 * pass the settings straght to ActuatorDesired object (manual mode) or to
 * AttitudeDesired object (stabilized mode)
 * @{ 
 */


// Private constants
#define STACK_SIZE configMINIMAL_STACK_SIZE
#define TASK_PRIORITY (tskIDLE_PRIORITY+4)
#define UPDATE_PERIOD_MS 20
#define THROTTLE_FAILSAFE -0.1
#define FLIGHT_MODE_LIMIT 1.0/3.0

// Private types

// Private variables
static xTaskHandle taskHandle;

// Private functions
static void manualControlTask(void* parameters);
static float scaleChannel(int16_t value, int16_t max, int16_t min, int16_t neutral);

/**
 * Module initialization
 */
int32_t ManualControlInitialize()
{
	// Start main task
	xTaskCreate(manualControlTask, (signed char*)"ManualControl", STACK_SIZE, NULL, TASK_PRIORITY, &taskHandle);

	return 0;
}

/**
 * Module task
 */
static void manualControlTask(void* parameters)
{
	ManualControlSettingsData settings;
	StabilizationSettingsData stabSettings;
	ManualControlCommandData cmd;
	ActuatorDesiredData actuator;
	AttitudeDesiredData attitude;
	portTickType lastSysTime;
	float flightMode;

	// Main task loop
	lastSysTime = xTaskGetTickCount();
	while (1)
	{
		// Wait until next update
		vTaskDelayUntil(&lastSysTime, UPDATE_PERIOD_MS / portTICK_RATE_MS );

		// Read settings
		ManualControlSettingsGet(&settings);
		StabilizationSettingsGet(&stabSettings);

		// Check settings, if error raise alarm
		if ( settings.Roll >= MANUALCONTROLSETTINGS_ROLL_NONE ||
		     settings.Pitch >= MANUALCONTROLSETTINGS_PITCH_NONE ||
		     settings.Yaw >= MANUALCONTROLSETTINGS_YAW_NONE ||
		     settings.Throttle >= MANUALCONTROLSETTINGS_THROTTLE_NONE ||
		     settings.FlightMode >= MANUALCONTROLSETTINGS_FLIGHTMODE_NONE )
		{
			AlarmsSet(SYSTEMALARMS_ALARM_MANUALCONTROL, SYSTEMALARMS_ALARM_CRITICAL);
			cmd.FlightMode = MANUALCONTROLCOMMAND_FLIGHTMODE_AUTO;
			cmd.Connected = MANUALCONTROLCOMMAND_CONNECTED_FALSE;
			ManualControlCommandSet(&cmd);
			continue;
		}

		// Read channel values in us
		// TODO: settings.InputMode is currently ignored because PIOS will not allow runtime
		// selection of PWM and PPM. The configuration is currently done at compile time in
		// the pios_config.h file.
		for (int n = 0; n < MANUALCONTROLCOMMAND_CHANNEL_NUMELEM; ++n)
		{
#if defined(PIOS_INCLUDE_PWM)
			cmd.Channel[n] = PIOS_PWM_Get(n);
#elif defined(PIOS_INCLUDE_PPM)
			cmd.Channel[n] = PIOS_PPM_Get(n);
#elif defined(PIOS_INCLUDE_SPEKTRUM)
			cmd.Channel[n] = PIOS_SPEKTRUM_Get(n);
#endif
		}

		// Calculate roll command in range +1 to -1
		cmd.Roll = scaleChannel( cmd.Channel[settings.Roll], settings.ChannelMax[settings.Roll],
				                 settings.ChannelMin[settings.Roll], settings.ChannelNeutral[settings.Roll] );

		// Calculate pitch command in range +1 to -1
		cmd.Pitch = scaleChannel( cmd.Channel[settings.Pitch], settings.ChannelMax[settings.Pitch],
				                  settings.ChannelMin[settings.Pitch], settings.ChannelNeutral[settings.Pitch] );

		// Calculate yaw command in range +1 to -1
		cmd.Yaw = scaleChannel( cmd.Channel[settings.Yaw], settings.ChannelMax[settings.Yaw],
				                  settings.ChannelMin[settings.Yaw], settings.ChannelNeutral[settings.Yaw] );

		// Calculate throttle command in range +1 to -1
		cmd.Throttle = scaleChannel( cmd.Channel[settings.Throttle], settings.ChannelMax[settings.Throttle],
				                     settings.ChannelMin[settings.Throttle], settings.ChannelNeutral[settings.Throttle] );

		// Update flight mode
		flightMode = scaleChannel( cmd.Channel[settings.FlightMode], settings.ChannelMax[settings.FlightMode],
				                   settings.ChannelMin[settings.FlightMode], settings.ChannelNeutral[settings.FlightMode] );
		if (flightMode < -FLIGHT_MODE_LIMIT)
		{
			cmd.FlightMode = MANUALCONTROLCOMMAND_FLIGHTMODE_MANUAL;
		}
		else if (flightMode > FLIGHT_MODE_LIMIT)
		{
			cmd.FlightMode = MANUALCONTROLCOMMAND_FLIGHTMODE_AUTO;
		}
		else
		{
			cmd.FlightMode = MANUALCONTROLCOMMAND_FLIGHTMODE_STABILIZED;
		}

		// Check for connection status (negative throttle values)
		// The receiver failsafe for the throttle channel should be set to a value below the channel NEUTRAL
		if ( cmd.Throttle < THROTTLE_FAILSAFE )
		{
			cmd.Connected = MANUALCONTROLCOMMAND_CONNECTED_FALSE;
			cmd.FlightMode = MANUALCONTROLCOMMAND_FLIGHTMODE_AUTO;
			AlarmsSet(SYSTEMALARMS_ALARM_MANUALCONTROL, SYSTEMALARMS_ALARM_WARNING);
		}
		else
		{
			cmd.Connected = MANUALCONTROLCOMMAND_CONNECTED_TRUE;
			AlarmsClear(SYSTEMALARMS_ALARM_MANUALCONTROL);
			if ( cmd.Throttle < 0 )
			{
				cmd.Throttle = 0;
			}
		}

		// Update the ManualControlCommand object
		ManualControlCommandSet(&cmd);

		// Depending on the mode update the Stabilization or Actuator objects
		if ( cmd.FlightMode == MANUALCONTROLCOMMAND_FLIGHTMODE_MANUAL )
		{
			actuator.Roll = cmd.Roll;
			actuator.Pitch = cmd.Pitch;
			actuator.Yaw = cmd.Yaw;
			actuator.Throttle = cmd.Throttle;
			ActuatorDesiredSet(&actuator);
		}
		else if ( cmd.FlightMode == MANUALCONTROLCOMMAND_FLIGHTMODE_STABILIZED )
		{
			attitude.Roll = cmd.Roll*stabSettings.RollMax;
			attitude.Pitch = cmd.Pitch*stabSettings.PitchMax;
			attitude.Yaw = cmd.Yaw*180.0;
			attitude.Throttle = cmd.Throttle*stabSettings.ThrottleMax;
			AttitudeDesiredSet(&attitude);
		}
	}
}

/**
 * Convert channel from servo pulse duration (microseconds) to scaled -1/+1 range.
 */
static float scaleChannel(int16_t value, int16_t max, int16_t min, int16_t neutral)
{
	float valueScaled;
	// Scale
	if ( (max > min && value >= neutral) || (min > max && value <= neutral) )
	{
		if ( max != neutral )
		{
			valueScaled = (float)(value-neutral)/(float)(max-neutral);
		}
		else
		{
			valueScaled = 0;
		}
	}
	else
	{
		if ( min != neutral )
		{
			valueScaled = (float)(value-neutral)/(float)(neutral-min);
		}
		else
		{
			valueScaled = 0;
		}
	}
	// Bound
	if ( valueScaled > 1.0 )
	{
		valueScaled = 1.0;
	}
	else if ( valueScaled < -1.0 )
	{
		valueScaled = -1.0;
	}
	return valueScaled;
}

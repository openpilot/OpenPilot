/**
 ******************************************************************************
 *
 * @file       actuator.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      Actuator module. Drives the actuators (servos, motors etc).
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
#include "actuator.h"
#include "actuatorsettings.h"
#include "systemsettings.h"
#include "actuatordesired.h"
#include "actuatorcommand.h"

// Private constants
#define MAX_QUEUE_SIZE 2
#define STACK_SIZE configMINIMAL_STACK_SIZE
#define TASK_PRIORITY (tskIDLE_PRIORITY+4)
#define FAILSAFE_TIMEOUT_MS 100

// Private types

// Private variables
static xQueueHandle queue;
static xTaskHandle taskHandle;

// Private functions
static void actuatorTask(void* parameters);
static int32_t mixerFixedWing(const ActuatorSettingsData* settings, const ActuatorDesiredData* desired, ActuatorCommandData* cmd);
static int32_t mixerFixedWingElevon(const ActuatorSettingsData* settings, const ActuatorDesiredData* desired, ActuatorCommandData* cmd);
static int32_t mixerVTOL(const ActuatorSettingsData* settings, const ActuatorDesiredData* desired, ActuatorCommandData* cmd);
static int16_t scaleChannel(float value, int16_t max, int16_t min, int16_t neutral);
static void setFailsafe();

/**
 * Module initialization
 */
int32_t ActuatorInitialize()
{
	// Create object queue
	queue = xQueueCreate(MAX_QUEUE_SIZE, sizeof(UAVObjEvent));

	// Listen for ExampleObject1 updates
	ActuatorDesiredConnectQueue(queue);

	// Start main task
	xTaskCreate(actuatorTask, (signed char*)"Actuator", STACK_SIZE, NULL, TASK_PRIORITY, &taskHandle);

	return 0;
}

/**
 * Main module task
 */
static void actuatorTask(void* parameters)
{
	UAVObjEvent ev;
	ActuatorSettingsData settings;
	SystemSettingsData sysSettings;
	ActuatorDesiredData desired;
	ActuatorCommandData cmd;

	// Set servo update frequency (done only on start-up)
	ActuatorSettingsGet(&settings);
	PIOS_Servo_SetHz(settings.ChannelUpdateFreq[0], settings.ChannelUpdateFreq[1]);

	// Go to the neutral (failsafe) values until an ActuatorDesired update is received
	setFailsafe();

	// Main task loop
	while (1)
	{
		// Wait until the ActuatorDesired object is updated, if a timeout then go to failsafe
		if ( xQueueReceive(queue, &ev, FAILSAFE_TIMEOUT_MS / portTICK_RATE_MS) != pdTRUE )
		{
			setFailsafe();
			continue;
		}

		// Read settings
		ActuatorSettingsGet(&settings);
		SystemSettingsGet(&sysSettings);

		// Reset ActuatorCommand to neutral values
		for (int n = 0; n < ACTUATORCOMMAND_CHANNEL_NUMELEM; ++n)
		{
			cmd.Channel[n] = settings.ChannelNeutral[n];
		}

		// Read input object
		ActuatorDesiredGet(&desired);

		// Call appropriate mixer depending on the airframe configuration
		if ( sysSettings.AirframeType == SYSTEMSETTINGS_AIRFRAMETYPE_FIXEDWING )
		{
			if ( mixerFixedWing(&settings, &desired, &cmd) == -1 )
			{
				AlarmsSet(SYSTEMALARMS_ALARM_ACTUATOR, SYSTEMALARMS_ALARM_CRITICAL);
			}
			else
			{
				AlarmsClear(SYSTEMALARMS_ALARM_ACTUATOR);
			}
		}
		else if ( sysSettings.AirframeType == SYSTEMSETTINGS_AIRFRAMETYPE_FIXEDWINGELEVON )
		{
			if ( mixerFixedWingElevon(&settings, &desired, &cmd) == -1 )
			{
				AlarmsSet(SYSTEMALARMS_ALARM_ACTUATOR, SYSTEMALARMS_ALARM_CRITICAL);
			}
			else
			{
				AlarmsClear(SYSTEMALARMS_ALARM_ACTUATOR);
			}
		}
		else if ( sysSettings.AirframeType == SYSTEMSETTINGS_AIRFRAMETYPE_VTOL )
		{
			if ( mixerVTOL(&settings, &desired, &cmd) == -1 )
			{
				AlarmsSet(SYSTEMALARMS_ALARM_ACTUATOR, SYSTEMALARMS_ALARM_CRITICAL);
			}
			else
			{
				AlarmsClear(SYSTEMALARMS_ALARM_ACTUATOR);
			}
		}

		// Update servo outputs
		for (int n = 0; n < ACTUATORCOMMAND_CHANNEL_NUMELEM; ++n)
		{
			PIOS_Servo_Set( n+1, cmd.Channel[n] );
		}

		// Update output object
		ActuatorCommandSet(&cmd);
	}
}

/**
 * Mixer for Fixed Wing airframes. Converts desired roll,pitch,yaw and throttle to servo outputs.
 * @return -1 if error, 0 if success
 */
static int32_t mixerFixedWing(const ActuatorSettingsData* settings, const ActuatorDesiredData* desired, ActuatorCommandData* cmd)
{
	// Check settings
	if ( settings->FixedWingPitch1 == ACTUATORSETTINGS_FIXEDWINGPITCH1_NONE ||
		 settings->FixedWingRoll1 == ACTUATORSETTINGS_FIXEDWINGROLL1_NONE ||
		 settings->FixedWingThrottle == ACTUATORSETTINGS_FIXEDWINGTHROTTLE_NONE )
	{
		return -1;
	}

	// Set pitch servo command
	cmd->Channel[ settings->FixedWingPitch1 ] = scaleChannel(desired->Pitch, settings->ChannelMax[ settings->FixedWingPitch1 ],
			                                                 settings->ChannelMin[ settings->FixedWingPitch1 ],
			                                                 settings->ChannelNeutral[ settings->FixedWingPitch1 ]);
	if ( settings->FixedWingPitch2 != ACTUATORSETTINGS_FIXEDWINGPITCH2_NONE )
	{
		cmd->Channel[ settings->FixedWingPitch2 ] = scaleChannel(desired->Pitch, settings->ChannelMax[ settings->FixedWingPitch2 ],
																 settings->ChannelMin[ settings->FixedWingPitch2 ],
																 settings->ChannelNeutral[ settings->FixedWingPitch2 ]);
	}

	// Set roll servo command
	cmd->Channel[ settings->FixedWingRoll1 ] = scaleChannel(desired->Roll, settings->ChannelMax[ settings->FixedWingRoll1 ],
			                                                 settings->ChannelMin[ settings->FixedWingRoll1 ],
			                                                 settings->ChannelNeutral[ settings->FixedWingRoll1 ]);
	if ( settings->FixedWingRoll2 != ACTUATORSETTINGS_FIXEDWINGROLL2_NONE )
	{
		cmd->Channel[ settings->FixedWingRoll2 ] = scaleChannel(desired->Roll, settings->ChannelMax[ settings->FixedWingRoll2 ],
															    settings->ChannelMin[ settings->FixedWingRoll2 ],
																settings->ChannelNeutral[ settings->FixedWingRoll2 ]);
	}

	// Set yaw servo command
	if ( settings->FixedWingYaw != ACTUATORSETTINGS_FIXEDWINGYAW_NONE )
	{
		cmd->Channel[ settings->FixedWingYaw ] = scaleChannel(desired->Yaw, settings->ChannelMax[ settings->FixedWingYaw ],
														      settings->ChannelMin[ settings->FixedWingYaw ],
															  settings->ChannelNeutral[ settings->FixedWingYaw ]);
	}

	// Set throttle servo command
	cmd->Channel[ settings->FixedWingThrottle ] = scaleChannel(desired->Throttle, settings->ChannelMax[ settings->FixedWingThrottle ],
			                                                 settings->ChannelMin[ settings->FixedWingThrottle ],
			                                                 settings->ChannelNeutral[ settings->FixedWingThrottle ]);

	// Done
	return 0;
}

/**
 * Mixer for Fixed Wing airframes with elevons. Converts desired roll,pitch,yaw and throttle to servo outputs.
 * @return -1 if error, 0 if success
 */
static int32_t mixerFixedWingElevon(const ActuatorSettingsData* settings, const ActuatorDesiredData* desired, ActuatorCommandData* cmd)
{
	// TODO: Implement elevon mixer
	return -1;
}

/**
 * Mixer for VTOL (quads and octo copters). Converts desired roll,pitch,yaw and throttle to servo outputs.
 * @return -1 if error, 0 if success
 */
static int32_t mixerVTOL(const ActuatorSettingsData* settings, const ActuatorDesiredData* desired, ActuatorCommandData* cmd)
{
	// TODO: Implement VTOL mixer
	return -1;
}

/**
 * Convert channel from -1/+1 to servo pulse duration in microseconds
 */
static int16_t scaleChannel(float value, int16_t max, int16_t min, int16_t neutral)
{
	int16_t valueScaled;
	// Scale
	if ( value >= 0.0)
	{
		valueScaled = (int16_t)(value*((float)(max-neutral))) + neutral;
	}
	else
	{
		valueScaled = (int16_t)(value*((float)(neutral-min))) + neutral;
	}
	return valueScaled;
}

/**
 * Set actuator output to the neutral values (failsafe)
 */
static void setFailsafe()
{
	ActuatorSettingsData settings;
	ActuatorCommandData cmd;

	// Read settings
	ActuatorSettingsGet(&settings);

	// Reset ActuatorCommand to neutral values
	for (int n = 0; n < ACTUATORCOMMAND_CHANNEL_NUMELEM; ++n)
	{
		cmd.Channel[n] = settings.ChannelNeutral[n];
	}

	// Set alarm
	AlarmsSet(SYSTEMALARMS_ALARM_ACTUATOR, SYSTEMALARMS_ALARM_CRITICAL);

	// Update servo outputs
	for (int n = 0; n < ACTUATORCOMMAND_CHANNEL_NUMELEM; ++n)
	{
		PIOS_Servo_Set( n+1, cmd.Channel[n] );
	}

	// Update output object
	ActuatorCommandSet(&cmd);
}

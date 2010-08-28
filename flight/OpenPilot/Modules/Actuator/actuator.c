/**
 ******************************************************************************
 * @addtogroup OpenPilotModules OpenPilot Modules
 * @{ 
 * @addtogroup ActuatorModule Actuator Module
 * @brief Compute servo/motor settings based on @ref ActuatorDesired "desired actuator positions" and aircraft type.
 * This is where all the mixing of channels is computed.
 * @{ 
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
static int32_t InterpolateCCPMCurve(float *Output, float input, const float *Curve);
static int32_t mixerCCPM( ActuatorSettingsData* settings, const ActuatorDesiredData* desired, ActuatorCommandData* cmd);
static int16_t scaleChannel(float value, int16_t max, int16_t min, int16_t neutral);
static void setFailsafe();

/**
 * @brief Module initialization
 * @return 0
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
 * @brief Main module task
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
		else if ( sysSettings.AirframeType == SYSTEMSETTINGS_AIRFRAMETYPE_HELICP )
		{
			if ( mixerCCPM(&settings, &desired, &cmd) == -1 )
			{
				AlarmsSet(SYSTEMALARMS_ALARM_ACTUATOR, SYSTEMALARMS_ALARM_CRITICAL);
			}
			else
			{
				AlarmsClear(SYSTEMALARMS_ALARM_ACTUATOR);
			}
		}

		// Update output object
		ActuatorCommandSet(&cmd);
		// Update in case read only (eg. during servo configuration)    
		ActuatorCommandGet(&cmd);
    
		// Update servo outputs
		for (int n = 0; n < ACTUATORCOMMAND_CHANNEL_NUMELEM; ++n)
		{
			PIOS_Servo_Set( n, cmd.Channel[n] );
		}
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
	// Check settings
	if ( settings->FixedWingRoll1 == ACTUATORSETTINGS_FIXEDWINGROLL1_NONE ||
      settings->FixedWingRoll2 == ACTUATORSETTINGS_FIXEDWINGROLL2_NONE ||
      settings->FixedWingThrottle == ACTUATORSETTINGS_FIXEDWINGTHROTTLE_NONE )
	{
		return -1;
	}
  
	// Set first elevon servo command
	cmd->Channel[ settings->FixedWingRoll1 ] = scaleChannel(desired->Pitch + desired->Roll, settings->ChannelMax[ settings->FixedWingRoll1 ],
                                                           settings->ChannelMin[ settings->FixedWingRoll1 ],
                                                           settings->ChannelNeutral[ settings->FixedWingRoll1 ]);

  // Set second elevon servo command
  cmd->Channel[ settings->FixedWingRoll2 ] = scaleChannel(desired->Pitch - desired->Roll, settings->ChannelMax[ settings->FixedWingRoll2 ],
                                                             settings->ChannelMin[ settings->FixedWingRoll2 ],
                                                             settings->ChannelNeutral[ settings->FixedWingRoll2 ]);
  
	// Set throttle servo command
	cmd->Channel[ settings->FixedWingThrottle ] = scaleChannel(desired->Throttle, settings->ChannelMax[ settings->FixedWingThrottle ],
                                                             settings->ChannelMin[ settings->FixedWingThrottle ],
                                                             settings->ChannelNeutral[ settings->FixedWingThrottle ]);
  
	// Done
	return 0;
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

static int32_t InterpolateCCPMCurve(float *output, float input, const float *curve)
{
	int8_t curvIndex;
	float slope;
	float offset;
	float value;

	//determine which section of the 5 point curve we are on
	if (input <= .25)
	{
		curvIndex=0;
		offset = 0;
	}
	else if (input <= .50)
	{
		curvIndex=1;
		offset = 0.25;
	}
	else if (input <= .75)
	{
		curvIndex=2;
		offset = 0.50;
	}
	else
	{
		curvIndex=3;
		offset = 0.75;
	}


	//calculate the linear interpolation for the selected segment
	slope = (curve[curvIndex+1]-curve[curvIndex])/0.25;
	value=curve[curvIndex] + ((input-offset) * slope);

	//bound the output to valid percentage values
	if( value > 100.0 ) value = 100.0;
	if( value < 0.0 ) value = 0.0;

	*output=value;

	return 0;

}

/**
 * Mixer for CCPM (single rotor helicopters). Converts desired roll,pitch,yaw and throttle to servo outputs.
 * @return -1 if error, 0 if success
 */
static int32_t mixerCCPM( ActuatorSettingsData* settings, const ActuatorDesiredData* desired, ActuatorCommandData* cmd)
{
#define degtorad  0.0174532925 //pi/180
	// TODO: Implement CCPM mixers
	int configOK;
	float curveValue;
	float throttle;
	float bladePitch;
	float CCPMCyclicConstant;
	float servoW;
	float servoX;
	float servoY;
	float servoZ;

	configOK=0;

	//calculate the throttle value based on the curve - scale to 0.0 -> +1.0
	InterpolateCCPMCurve(&curveValue, desired->Throttle, settings->CCPMThrottleCurve);
	throttle = curveValue/100.0;

	//calculate the Blade Pitch value based on the curve - scale to -1.0 -> +1.0
	InterpolateCCPMCurve(&curveValue, desired->Throttle, settings->CCPMPitchCurve);
	bladePitch = (curveValue/50.0) -1.0;

	//calculate how much Cyclic to apply to the mixing
	CCPMCyclicConstant=1-settings->CCPMCollectiveConstant;

	// Set ServoW servo command
	if ( settings->CCPMServoW != ACTUATORSETTINGS_CCPMSERVOW_NONE )
	{
		servoW=(CCPMCyclicConstant * ((sin((settings->CCPMAngleW + settings->CCPMCorrectionAngle)*degtorad)*desired->Roll)
										+(cos((settings->CCPMAngleW + settings->CCPMCorrectionAngle)*degtorad)*desired->Pitch)))
										+ (settings->CCPMCollectiveConstant * bladePitch);

		cmd->Channel[ settings->CCPMServoW ] = scaleChannel(servoW, settings->ChannelMax[ settings->CCPMServoW ],
			                                                 settings->ChannelMin[ settings->CCPMServoW ],
			                                                 settings->ChannelNeutral[ settings->CCPMServoW ]);
		configOK++;
	}
	// Set ServoX servo command
	if ( settings->CCPMServoX != ACTUATORSETTINGS_CCPMSERVOX_NONE )
	{
		servoX=(CCPMCyclicConstant * ((sin((settings->CCPMAngleX + settings->CCPMCorrectionAngle)*degtorad)*desired->Roll)
										+(cos((settings->CCPMAngleX + settings->CCPMCorrectionAngle)*degtorad)*desired->Pitch)))
										+ (settings->CCPMCollectiveConstant * bladePitch);

		cmd->Channel[ settings->CCPMServoX ] = scaleChannel(servoX, settings->ChannelMax[ settings->CCPMServoX ],
			                                                 settings->ChannelMin[ settings->CCPMServoX ],
			                                                 settings->ChannelNeutral[ settings->CCPMServoX ]);
		configOK++;
	}
	// Set ServoY servo command
	if ( settings->CCPMServoY != ACTUATORSETTINGS_CCPMSERVOY_NONE )
	{
		servoY=(CCPMCyclicConstant * ((sin((settings->CCPMAngleY + settings->CCPMCorrectionAngle)*degtorad)*desired->Roll)
										+(cos((settings->CCPMAngleY + settings->CCPMCorrectionAngle)*degtorad)*desired->Pitch)))
										+ (settings->CCPMCollectiveConstant * bladePitch);

		cmd->Channel[ settings->CCPMServoY ] = scaleChannel(servoY, settings->ChannelMax[ settings->CCPMServoY ],
			                                                 settings->ChannelMin[ settings->CCPMServoY ],
			                                                 settings->ChannelNeutral[ settings->CCPMServoY ]);
		configOK++;
	}
	// Set ServoZ servo command
	if ( settings->CCPMServoZ != ACTUATORSETTINGS_CCPMSERVOZ_NONE )
	{
		servoZ=(CCPMCyclicConstant * ((sin((settings->CCPMAngleZ + settings->CCPMCorrectionAngle)*degtorad)*desired->Roll)
										+(cos((settings->CCPMAngleZ + settings->CCPMCorrectionAngle)*degtorad)*desired->Pitch)))
										+ (settings->CCPMCollectiveConstant * bladePitch);

		cmd->Channel[ settings->CCPMServoZ ] = scaleChannel(servoZ, settings->ChannelMax[ settings->CCPMServoZ ],
			                                                 settings->ChannelMin[ settings->CCPMServoZ ],
			                                                 settings->ChannelNeutral[ settings->CCPMServoZ ]);
		configOK++;
	}


	// Set throttle servo command
	if ( settings->CCPMThrottle != ACTUATORSETTINGS_CCPMTHROTTLE_NONE )
	{
	cmd->Channel[ settings->CCPMThrottle ] = scaleChannel(throttle, settings->ChannelMax[ settings->CCPMThrottle ],
			                                                 settings->ChannelMin[ settings->CCPMThrottle ],
			                                                 settings->ChannelNeutral[ settings->CCPMThrottle ]);
	}
	else
	{
		configOK=0;
	}

	// Set TailRotor servo command
	if (settings->CCPMYawStabilizationInManualMode == ACTUATORSETTINGS_CCPMYAWSTABILIZATIONINMANUALMODE_FALSE)
	{//update the servo as per the desired control mode (manual or stabalized)
		if ( settings->CCPMTailRotor != ACTUATORSETTINGS_CCPMTAILROTOR_NONE )
		{
			cmd->Channel[ settings->CCPMTailRotor ] = scaleChannel(desired->Yaw, settings->ChannelMax[ settings->CCPMTailRotor ],
																  settings->ChannelMin[ settings->CCPMTailRotor ],
																  settings->ChannelNeutral[ settings->CCPMTailRotor ]);
		}
		else
		{
			configOK=0;
		}
	}
	else
	{//TODO need to implement the stabilization PID loop to provide heading hold gyro functionality in manual control mode

	}
	if (configOK<2)return -1;
	return 0;
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
  
	if (max>min)
	{
		if( valueScaled > max ) valueScaled = max;
		if( valueScaled < min ) valueScaled = min;
	}
	else
	{
		if( valueScaled < max ) valueScaled = max;
		if( valueScaled > min ) valueScaled = min;
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
		PIOS_Servo_Set( n, cmd.Channel[n] );
	}

	// Update output object
	ActuatorCommandSet(&cmd);
}

/** 
 * @}
  * @}
  */

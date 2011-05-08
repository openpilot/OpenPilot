/**
 ******************************************************************************
 * @addtogroup OpenPilotModules OpenPilot Modules
 * @{
 * @addtogroup ManualControlModule Manual Control Module
 * @brief Provide manual control or allow it alter flight mode.
 * @{
 *
 * Reads in the ManualControlCommand FlightMode setting from receiver then either
 * pass the settings straght to ActuatorDesired object (manual mode) or to
 * AttitudeDesired object (stabilized mode)
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
#include "stabilizationdesired.h"
#include "flighttelemetrystats.h"

// Private constants
#if defined(PIOS_MANUAL_STACK_SIZE)
#define STACK_SIZE_BYTES PIOS_MANUAL_STACK_SIZE
#else
#define STACK_SIZE_BYTES 824
#endif

#define TASK_PRIORITY (tskIDLE_PRIORITY+4)
#define UPDATE_PERIOD_MS 20
#define THROTTLE_FAILSAFE -0.1
#define FLIGHT_MODE_LIMIT 1.0/3.0
#define ARMED_TIME_MS      1000
//safe band to allow a bit of calibration error or trim offset (in microseconds)
#define CONNECTION_OFFSET 150

// Private types
typedef enum
{
	ARM_STATE_DISARMED,
	ARM_STATE_ARMING_MANUAL,
	ARM_STATE_ARMED,
	ARM_STATE_DISARMING_MANUAL,
	ARM_STATE_DISARMING_TIMEOUT
} ArmState_t;

// Private variables
static xTaskHandle taskHandle;
static ArmState_t armState;

// Private functions
static void updateActuatorDesired(ManualControlCommandData * cmd);
static void updateStabilizationDesired(ManualControlCommandData * cmd, ManualControlSettingsData * settings);

static void manualControlTask(void *parameters);
static float scaleChannel(int16_t value, int16_t max, int16_t min, int16_t neutral, int16_t deadband_percent);
static uint32_t timeDifferenceMs(portTickType start_time, portTickType end_time);
static bool okToArm(void);
static bool validInputRange(int16_t min, int16_t max, uint16_t value);

#define assumptions1 ( \
		((int)MANUALCONTROLSETTINGS_STABILIZATION1SETTINGS_NONE      == (int)STABILIZATIONDESIRED_STABILIZATIONMODE_NONE)       && \
		((int)MANUALCONTROLSETTINGS_STABILIZATION1SETTINGS_RATE      == (int)STABILIZATIONDESIRED_STABILIZATIONMODE_RATE)       && \
		((int)MANUALCONTROLSETTINGS_STABILIZATION1SETTINGS_ATTITUDE  == (int)STABILIZATIONDESIRED_STABILIZATIONMODE_ATTITUDE)      \
		)

#define assumptions3 ( \
		((int)MANUALCONTROLSETTINGS_STABILIZATION2SETTINGS_NONE      == (int)STABILIZATIONDESIRED_STABILIZATIONMODE_NONE)      && \
		((int)MANUALCONTROLSETTINGS_STABILIZATION2SETTINGS_RATE      == (int)STABILIZATIONDESIRED_STABILIZATIONMODE_RATE)      && \
		((int)MANUALCONTROLSETTINGS_STABILIZATION2SETTINGS_ATTITUDE  == (int)STABILIZATIONDESIRED_STABILIZATIONMODE_ATTITUDE)     \
		)

#define assumptions5 ( \
		((int)MANUALCONTROLSETTINGS_STABILIZATION3SETTINGS_NONE      == (int)STABILIZATIONDESIRED_STABILIZATIONMODE_NONE)      && \
		((int)MANUALCONTROLSETTINGS_STABILIZATION3SETTINGS_RATE      == (int)STABILIZATIONDESIRED_STABILIZATIONMODE_RATE)      && \
		((int)MANUALCONTROLSETTINGS_STABILIZATION3SETTINGS_ATTITUDE  == (int)STABILIZATIONDESIRED_STABILIZATIONMODE_ATTITUDE)     \
		)



#define ARMING_CHANNEL_ROLL     0
#define ARMING_CHANNEL_PITCH    1
#define ARMING_CHANNEL_YAW      2

#define assumptions7 ( \
		( ((int)MANUALCONTROLSETTINGS_ARMING_ROLLLEFT           -(int)MANUALCONTROLSETTINGS_ARMING_ROLLLEFT)/2 	== ARMING_CHANNEL_ROLL) 	&& \
		( ((int)MANUALCONTROLSETTINGS_ARMING_ROLLRIGHT          -(int)MANUALCONTROLSETTINGS_ARMING_ROLLLEFT)/2 	== ARMING_CHANNEL_ROLL) 	&& \
		( ((int)MANUALCONTROLSETTINGS_ARMING_PITCHFORWARD       -(int)MANUALCONTROLSETTINGS_ARMING_ROLLLEFT)/2 	== ARMING_CHANNEL_PITCH) 	&& \
		( ((int)MANUALCONTROLSETTINGS_ARMING_PITCHAFT           -(int)MANUALCONTROLSETTINGS_ARMING_ROLLLEFT)/2 	== ARMING_CHANNEL_PITCH) 	&& \
		( ((int)MANUALCONTROLSETTINGS_ARMING_YAWLEFT            -(int)MANUALCONTROLSETTINGS_ARMING_ROLLLEFT)/2 	== ARMING_CHANNEL_YAW) 		&& \
		( ((int)MANUALCONTROLSETTINGS_ARMING_YAWRIGHT           -(int)MANUALCONTROLSETTINGS_ARMING_ROLLLEFT)/2 	== ARMING_CHANNEL_YAW)		\
		)

#define assumptions8 ( \
		( ((int)MANUALCONTROLSETTINGS_ARMING_ROLLLEFT           -(int)MANUALCONTROLSETTINGS_ARMING_ROLLLEFT)%2 	== 0) 	&& \
		( ((int)MANUALCONTROLSETTINGS_ARMING_ROLLRIGHT          -(int)MANUALCONTROLSETTINGS_ARMING_ROLLLEFT)%2 	!= 0) 	&& \
		( ((int)MANUALCONTROLSETTINGS_ARMING_PITCHFORWARD       -(int)MANUALCONTROLSETTINGS_ARMING_ROLLLEFT)%2 	== 0) 	&& \
		( ((int)MANUALCONTROLSETTINGS_ARMING_PITCHAFT           -(int)MANUALCONTROLSETTINGS_ARMING_ROLLLEFT)%2 	!= 0) 	&& \
		( ((int)MANUALCONTROLSETTINGS_ARMING_YAWLEFT            -(int)MANUALCONTROLSETTINGS_ARMING_ROLLLEFT)%2 	== 0)	&& \
		( ((int)MANUALCONTROLSETTINGS_ARMING_YAWRIGHT           -(int)MANUALCONTROLSETTINGS_ARMING_ROLLLEFT)%2 	!= 0)	\
		)


#define assumptions_flightmode ( \
		( (int)MANUALCONTROLSETTINGS_FLIGHTMODEPOSITION_MANUAL == (int) MANUALCONTROLCOMMAND_FLIGHTMODE_MANUAL) && \
		( (int)MANUALCONTROLSETTINGS_FLIGHTMODEPOSITION_STABILIZED1 == (int) MANUALCONTROLCOMMAND_FLIGHTMODE_STABILIZED1) && \
		( (int)MANUALCONTROLSETTINGS_FLIGHTMODEPOSITION_STABILIZED2 == (int) MANUALCONTROLCOMMAND_FLIGHTMODE_STABILIZED2) && \
		( (int)MANUALCONTROLSETTINGS_FLIGHTMODEPOSITION_STABILIZED3 == (int) MANUALCONTROLCOMMAND_FLIGHTMODE_STABILIZED3) && \
		( (int)MANUALCONTROLSETTINGS_FLIGHTMODEPOSITION_VELOCITYCONTROL == (int) MANUALCONTROLCOMMAND_FLIGHTMODE_VELOCITYCONTROL) && \
		( (int)MANUALCONTROLSETTINGS_FLIGHTMODEPOSITION_POSITIONHOLD == (int) MANUALCONTROLCOMMAND_FLIGHTMODE_POSITIONHOLD) \
		)

#define assumptions (assumptions1 && assumptions3 && assumptions5 && assumptions7 && assumptions8 && assumptions_flightmode)

/**
 * Module initialization
 */
int32_t ManualControlInitialize()
{
	/* Check the assumptions about uavobject enum's are correct */
	if(!assumptions) 
		return -1;
	// Start main task
	xTaskCreate(manualControlTask, (signed char *)"ManualControl", STACK_SIZE_BYTES/4, NULL, TASK_PRIORITY, &taskHandle);
	TaskMonitorAdd(TASKINFO_RUNNING_MANUALCONTROL, taskHandle);
	PIOS_WDG_RegisterFlag(PIOS_WDG_MANUAL);


	return 0;
}

/**
 * Module task
 */
static void manualControlTask(void *parameters)
{
	ManualControlSettingsData settings;
	ManualControlCommandData cmd;
	portTickType lastSysTime;
	
	float flightMode = 0;

	uint8_t disconnected_count = 0;
	uint8_t connected_count = 0;
	enum { CONNECTED, DISCONNECTED } connection_state = DISCONNECTED;

	// Make sure unarmed on power up
	ManualControlCommandGet(&cmd);
	cmd.Armed = MANUALCONTROLCOMMAND_ARMED_FALSE;
	ManualControlCommandSet(&cmd);
	armState = ARM_STATE_DISARMED;

	// Main task loop
	lastSysTime = xTaskGetTickCount();
	while (1) {
		float scaledChannel[MANUALCONTROLCOMMAND_CHANNEL_NUMELEM];

		// Wait until next update
		vTaskDelayUntil(&lastSysTime, UPDATE_PERIOD_MS / portTICK_RATE_MS);
		PIOS_WDG_UpdateFlag(PIOS_WDG_MANUAL);

		// Read settings
		ManualControlSettingsGet(&settings);

		if (ManualControlCommandReadOnly(&cmd)) {
			FlightTelemetryStatsData flightTelemStats;
			FlightTelemetryStatsGet(&flightTelemStats);
			if(flightTelemStats.Status != FLIGHTTELEMETRYSTATS_STATUS_CONNECTED) {
				/* trying to fly via GCS and lost connection.  fall back to transmitter */
				UAVObjMetadata metadata;
				UAVObjGetMetadata(&cmd, &metadata);
				metadata.access = ACCESS_READWRITE;
				UAVObjSetMetadata(&cmd, &metadata);
			}
		}

		if (!ManualControlCommandReadOnly(&cmd)) {

			// Read channel values in us
			// TODO: settings.InputMode is currently ignored because PIOS will not allow runtime
			// selection of PWM and PPM. The configuration is currently done at compile time in
			// the pios_config.h file.
			for (int n = 0; n < MANUALCONTROLCOMMAND_CHANNEL_NUMELEM; ++n) {
#if defined(PIOS_INCLUDE_PWM)
				cmd.Channel[n] = PIOS_PWM_Get(n);
#elif defined(PIOS_INCLUDE_PPM)
				cmd.Channel[n] = PIOS_PPM_Get(n);
#elif defined(PIOS_INCLUDE_SPEKTRUM)
				cmd.Channel[n] = PIOS_SPEKTRUM_Get(n);
#endif
				scaledChannel[n] = scaleChannel(cmd.Channel[n], settings.ChannelMax[n],	settings.ChannelMin[n], settings.ChannelNeutral[n], 0);
			}

			// Check settings, if error raise alarm
			if (settings.Roll >= MANUALCONTROLSETTINGS_ROLL_NONE ||
			    settings.Pitch >= MANUALCONTROLSETTINGS_PITCH_NONE ||
			    settings.Yaw >= MANUALCONTROLSETTINGS_YAW_NONE ||
			    settings.Throttle >= MANUALCONTROLSETTINGS_THROTTLE_NONE ||
			    settings.FlightMode >= MANUALCONTROLSETTINGS_FLIGHTMODE_NONE) {
				AlarmsSet(SYSTEMALARMS_ALARM_MANUALCONTROL, SYSTEMALARMS_ALARM_CRITICAL);
				cmd.Connected = MANUALCONTROLCOMMAND_CONNECTED_FALSE;
				ManualControlCommandSet(&cmd);
				continue;
			}

			// decide if we have valid manual input or not
			bool valid_input_detected = TRUE;
			if (!validInputRange(settings.ChannelMin[settings.Throttle], settings.ChannelMax[settings.Throttle], cmd.Channel[settings.Throttle]))
				valid_input_detected = FALSE;
			if (!validInputRange(settings.ChannelMin[settings.Roll], settings.ChannelMax[settings.Roll], cmd.Channel[settings.Roll]))
				valid_input_detected = FALSE;
			if (!validInputRange(settings.ChannelMin[settings.Yaw], settings.ChannelMax[settings.Yaw], cmd.Channel[settings.Yaw]))
				valid_input_detected = FALSE;
			if (!validInputRange(settings.ChannelMin[settings.Pitch], settings.ChannelMax[settings.Pitch], cmd.Channel[settings.Pitch]))
				valid_input_detected = FALSE;

			// Implement hysteresis loop on connection status
			if (valid_input_detected)
			{
				if (++connected_count > 10)
				{
					connection_state = CONNECTED;
					connected_count = 0;
					disconnected_count = 0;
				}
			}
			else
			{
				if (++disconnected_count > 10)
				{
					connection_state = DISCONNECTED;
					connected_count = 0;
					disconnected_count = 0;
				}
			}

			if (connection_state == DISCONNECTED) {
				cmd.Connected = MANUALCONTROLCOMMAND_CONNECTED_FALSE;
				cmd.Throttle = -1;	// Shut down engine with no control
				cmd.Roll = 0;
				cmd.Yaw = 0;
				cmd.Pitch = 0;
				//cmd.FlightMode = MANUALCONTROLCOMMAND_FLIGHTMODE_AUTO; // don't do until AUTO implemented and functioning
				// Important: Throttle < 0 will reset Stabilization coefficients among other things. Either change this,
				// or leave throttle at IDLE speed or above when going into AUTO-failsafe.
				AlarmsSet(SYSTEMALARMS_ALARM_MANUALCONTROL, SYSTEMALARMS_ALARM_WARNING);
				ManualControlCommandSet(&cmd);
			} else {
				cmd.Connected = MANUALCONTROLCOMMAND_CONNECTED_TRUE;
				AlarmsClear(SYSTEMALARMS_ALARM_MANUALCONTROL);

				// Scale channels to -1 -> +1 range
				cmd.Roll 		= scaledChannel[settings.Roll];
				cmd.Pitch 		= scaledChannel[settings.Pitch];
				cmd.Yaw 		= scaledChannel[settings.Yaw];
				cmd.Throttle 	= scaledChannel[settings.Throttle];
				flightMode 		= scaledChannel[settings.FlightMode];

				if (settings.Accessory1 != MANUALCONTROLSETTINGS_ACCESSORY1_NONE)
					cmd.Accessory1 = scaledChannel[settings.Accessory1];
				else
					cmd.Accessory1 = 0;

				if (settings.Accessory2 != MANUALCONTROLSETTINGS_ACCESSORY2_NONE)
					cmd.Accessory2 = scaledChannel[settings.Accessory2];
				else
					cmd.Accessory2 = 0;

				if (settings.Accessory3 != MANUALCONTROLSETTINGS_ACCESSORY3_NONE)
					cmd.Accessory3 = scaledChannel[settings.Accessory3];
				else
					cmd.Accessory3 = 0;

				// Note here the code is ass
				if (flightMode < -FLIGHT_MODE_LIMIT) 
					cmd.FlightMode = settings.FlightModePosition[0];
				else if (flightMode > FLIGHT_MODE_LIMIT) 
					cmd.FlightMode = settings.FlightModePosition[2];
				else 
					cmd.FlightMode = settings.FlightModePosition[1];
				

				//
				// Arming and Disarming mechanism
				//

				if (settings.Arming == MANUALCONTROLSETTINGS_ARMING_ALWAYSDISARMED) {
					// In this configuration we always disarm
					cmd.Armed = MANUALCONTROLCOMMAND_ARMED_FALSE;
				} else {
					// In all other cases, we will not change the arm state when disconnected
					if (connection_state == CONNECTED)
					{
						if (settings.Arming == MANUALCONTROLSETTINGS_ARMING_ALWAYSARMED) {
							// In this configuration, we go into armed state as soon as the throttle is low, never disarm
							if (cmd.Throttle < 0) {
								cmd.Armed = MANUALCONTROLCOMMAND_ARMED_TRUE;
							}
						} else {
							// When the configuration is not "Always armed" and no "Always disarmed",
							// the state will not be changed when the throttle is not low
							if (cmd.Throttle < 0) {
								static portTickType armedDisarmStart;
								float armingInputLevel = 0;

								// Calc channel see assumptions7
								switch ( (settings.Arming-MANUALCONTROLSETTINGS_ARMING_ROLLLEFT)/2 ) {
								case ARMING_CHANNEL_ROLL: 	armingInputLevel = cmd.Roll; 	break;
								case ARMING_CHANNEL_PITCH: 	armingInputLevel = cmd.Pitch; 	break;
								case ARMING_CHANNEL_YAW: 	armingInputLevel = cmd.Yaw; 	break;
								}

								bool manualArm = false;
								bool manualDisarm = false;

								if (connection_state == CONNECTED) {
									// Should use RC input only if RX is connected
									if (armingInputLevel <= -0.50)
										manualArm = true;
									else if (armingInputLevel >= +0.50)
										manualDisarm = true;
								}

								// Swap arm-disarming see assumptions8
								if ((settings.Arming-MANUALCONTROLSETTINGS_ARMING_ROLLLEFT)%2) {
									bool temp = manualArm;
									manualArm = manualDisarm;
									manualDisarm = temp;
								}

								switch(armState) {
								case ARM_STATE_DISARMED:
									cmd.Armed = MANUALCONTROLCOMMAND_ARMED_FALSE;

									if (manualArm)
									{
										if (okToArm())	// only allow arming if it's OK too
										{
											armedDisarmStart = lastSysTime;
											armState = ARM_STATE_ARMING_MANUAL;
										}
									}
									break;

								case ARM_STATE_ARMING_MANUAL:
									if (manualArm) {
										if (timeDifferenceMs(armedDisarmStart, lastSysTime) > ARMED_TIME_MS)
											armState = ARM_STATE_ARMED;
									}
									else
										armState = ARM_STATE_DISARMED;
									break;

								case ARM_STATE_ARMED:
									// When we get here, the throttle is low,
									// we go immediately to disarming due to timeout, also when the disarming mechanism is not enabled
									armedDisarmStart = lastSysTime;
									armState = ARM_STATE_DISARMING_TIMEOUT;
									cmd.Armed = MANUALCONTROLCOMMAND_ARMED_TRUE;
									break;

								case ARM_STATE_DISARMING_TIMEOUT:
									// We get here when armed while throttle low, even when the arming timeout is not enabled
									if (settings.ArmedTimeout != 0)
										if (timeDifferenceMs(armedDisarmStart, lastSysTime) > settings.ArmedTimeout)
											armState = ARM_STATE_DISARMED;
									// Switch to disarming due to manual control when needed
									if (manualDisarm) {
										armedDisarmStart = lastSysTime;
										armState = ARM_STATE_DISARMING_MANUAL;
									}
									break;

								case ARM_STATE_DISARMING_MANUAL:
									if (manualDisarm) {
										if (timeDifferenceMs(armedDisarmStart, lastSysTime) > ARMED_TIME_MS)
											armState = ARM_STATE_DISARMED;
									}
									else
										armState = ARM_STATE_ARMED;
									break;
								}	// End Switch
							} else {
								// The throttle is not low, in case we where arming or disarming, abort
								switch(armState) {
									case ARM_STATE_DISARMING_MANUAL:
									case ARM_STATE_DISARMING_TIMEOUT:
										armState = ARM_STATE_ARMED;
										break;
									case ARM_STATE_ARMING_MANUAL:
										armState = ARM_STATE_DISARMED;
										break;
									default:
										// Nothing needs to be done in the other states
										break;
								}
							}
						}
					}
				}
				//
				// End of arming/disarming
				//

				// Update cmd object
				ManualControlCommandSet(&cmd);

			}
			
		} else {
			ManualControlCommandGet(&cmd);	/* Under GCS control */
		}


		// Depending on the mode update the Stabilization or Actuator objects
		switch(PARSE_FLIGHT_MODE(cmd.FlightMode)) {
			case FLIGHTMODE_UNDEFINED:
				// This reflects a bug in the code architecture!
				AlarmsSet(SYSTEMALARMS_ALARM_MANUALCONTROL, SYSTEMALARMS_ALARM_CRITICAL);
				break;
			case FLIGHTMODE_MANUAL:
				updateActuatorDesired(&cmd);
				break;
			case FLIGHTMODE_STABILIZED:
				updateStabilizationDesired(&cmd, &settings);
				break;
			case FLIGHTMODE_GUIDANCE:
				// TODO: Implement
				break;
		}				
	}
}

static void updateActuatorDesired(ManualControlCommandData * cmd) 
{
	ActuatorDesiredData actuator;
	ActuatorDesiredGet(&actuator);
	actuator.Roll = cmd->Roll;
	actuator.Pitch = cmd->Pitch;
	actuator.Yaw = cmd->Yaw;
	actuator.Throttle = (cmd->Throttle < 0) ? -1 : cmd->Throttle;
	ActuatorDesiredSet(&actuator);	
}

static void updateStabilizationDesired(ManualControlCommandData * cmd, ManualControlSettingsData * settings)
{
	StabilizationDesiredData stabilization;
	StabilizationDesiredGet(&stabilization);
	
	StabilizationSettingsData stabSettings;
	StabilizationSettingsGet(&stabSettings);
		
	uint8_t * stab_settings;
	switch(cmd->FlightMode) {
		case MANUALCONTROLCOMMAND_FLIGHTMODE_STABILIZED1:
			stab_settings = settings->Stabilization1Settings;
			break;
		case MANUALCONTROLCOMMAND_FLIGHTMODE_STABILIZED2:
			stab_settings = settings->Stabilization2Settings;
			break;
		case MANUALCONTROLCOMMAND_FLIGHTMODE_STABILIZED3:
			stab_settings = settings->Stabilization3Settings;
			break;
		default:
			// Major error, this should not occur because only enter this block when one of these is true
			AlarmsSet(SYSTEMALARMS_ALARM_MANUALCONTROL, SYSTEMALARMS_ALARM_CRITICAL);
			return;	
	}
	
	// TOOD: Add assumption about order of stabilization desired and manual control stabilization mode fields having same order
	stabilization.StabilizationMode[STABILIZATIONDESIRED_STABILIZATIONMODE_ROLL]  = stab_settings[0];
	stabilization.StabilizationMode[STABILIZATIONDESIRED_STABILIZATIONMODE_PITCH] = stab_settings[1];
	stabilization.StabilizationMode[STABILIZATIONDESIRED_STABILIZATIONMODE_YAW]   = stab_settings[2];
	
	stabilization.Roll = (stab_settings[0] == STABILIZATIONDESIRED_STABILIZATIONMODE_NONE) ? cmd->Roll :
	     (stab_settings[0] == STABILIZATIONDESIRED_STABILIZATIONMODE_RATE) ? cmd->Roll * stabSettings.ManualRate[STABILIZATIONSETTINGS_MANUALRATE_ROLL] :
	     (stab_settings[0] == STABILIZATIONDESIRED_STABILIZATIONMODE_ATTITUDE) ? cmd->Roll * stabSettings.RollMax :
	     0; // this is an invalid mode
					      ;
	stabilization.Pitch = (stab_settings[1] == STABILIZATIONDESIRED_STABILIZATIONMODE_NONE) ? cmd->Pitch :
	     (stab_settings[1] == STABILIZATIONDESIRED_STABILIZATIONMODE_RATE) ? cmd->Pitch * stabSettings.ManualRate[STABILIZATIONSETTINGS_MANUALRATE_PITCH] :
	     (stab_settings[1] == STABILIZATIONDESIRED_STABILIZATIONMODE_ATTITUDE) ? cmd->Pitch * stabSettings.PitchMax :
	     0; // this is an invalid mode

	stabilization.Yaw = (stab_settings[2] == STABILIZATIONDESIRED_STABILIZATIONMODE_NONE) ? cmd->Yaw :
	     (stab_settings[2] == STABILIZATIONDESIRED_STABILIZATIONMODE_RATE) ? cmd->Yaw * stabSettings.ManualRate[STABILIZATIONSETTINGS_MANUALRATE_YAW] :
	     (stab_settings[2] == STABILIZATIONDESIRED_STABILIZATIONMODE_ATTITUDE) ? fmod(cmd->Yaw * 180.0, 360) :
	     0; // this is an invalid mode
	
	stabilization.Throttle = (cmd->Throttle < 0) ? -1 : cmd->Throttle; 
	StabilizationDesiredSet(&stabilization);
}

/**
 * Convert channel from servo pulse duration (microseconds) to scaled -1/+1 range.
 */
static float scaleChannel(int16_t value, int16_t max, int16_t min, int16_t neutral, int16_t deadband_percent)
{
	float valueScaled;

	// Scale
	if ((max > min && value >= neutral) || (min > max && value <= neutral))
	{
		if (max != neutral)
			valueScaled = (float)(value - neutral) / (float)(max - neutral);
		else
			valueScaled = 0;
	}
	else
	{
		if (min != neutral)
			valueScaled = (float)(value - neutral) / (float)(neutral - min);
		else
			valueScaled = 0;
	}

	// Neutral RC stick position dead band
	if (deadband_percent > 0)
	{
		if (deadband_percent > 50) deadband_percent = 50;   // limit deadband to a maximum of 50%
		float deadband = (float)deadband_percent / 100;
		if (fabs(valueScaled) <= deadband)
			valueScaled = 0;                                // deadband the value
		else
		if (valueScaled < 0)
			valueScaled = (valueScaled + deadband) / (1.0 - deadband);	// value scales 0.0 to -1.0 after deadband
		else
			valueScaled = (valueScaled - deadband) / (1.0 - deadband);  // value scales 0.0 to +1.0 after deadband
	}

	// Bound
	if (valueScaled >  1.0) valueScaled =  1.0;
	else
	if (valueScaled < -1.0) valueScaled = -1.0;

	return valueScaled;
}

static uint32_t timeDifferenceMs(portTickType start_time, portTickType end_time) {
	if(end_time > start_time)
		return (end_time - start_time) * portTICK_RATE_MS;
	return ((((portTICK_RATE_MS) -1) - start_time) + end_time) * portTICK_RATE_MS;
}

/**
 * @brief Determine if the aircraft is safe to arm
 * @returns True if safe to arm, false otherwise
 */
static bool okToArm(void)
{
	// read alarms
	SystemAlarmsData alarms;
	SystemAlarmsGet(&alarms);


	// Check each alarm
	for (int i = 0; i < SYSTEMALARMS_ALARM_NUMELEM; i++)
	{
		if (alarms.Alarm[i] >= SYSTEMALARMS_ALARM_ERROR)
		{	// found an alarm thats set
			if (i == SYSTEMALARMS_ALARM_GPS || i == SYSTEMALARMS_ALARM_TELEMETRY)
				continue;

			return false;
		}
	}

	return true;
}

/**
 * @brief Determine if the manual input value is within acceptable limits
 * @returns return TRUE if so, otherwise return FALSE
 */
bool validInputRange(int16_t min, int16_t max, uint16_t value)
{
	if (min > max)
	{
		int16_t tmp = min;
		min = max;
		max = tmp;
	}
	return (value >= min - CONNECTION_OFFSET && value <= max + CONNECTION_OFFSET);
}

//
//static void armingMechanism(uint8_t* armingState, const ManualControlSettingsData* settings, const ManualControlCommandData* cmd)
//{
//	if (settings->Arming == MANUALCONTROLSETTINGS_ARMING_ALWAYSDISARMED) {
//		*armingState = MANUALCONTROLCOMMAND_ARMED_FALSE;
//		return;
//	}
//
//
//}
/**
  * @}
  * @}
  */

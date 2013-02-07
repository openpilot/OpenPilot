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
#include "accessorydesired.h"
#include "actuatordesired.h"
#include "altitudeholddesired.h"
#include "baroaltitude.h"
#include "flighttelemetrystats.h"
#include "flightstatus.h"
#include "manualcontrol.h"
#include "manualcontrolsettings.h"
#include "manualcontrolcommand.h"
#include "positionactual.h"
#include "pathdesired.h"
#include "stabilizationsettings.h"
#include "stabilizationdesired.h"
#include "receiveractivity.h"

#if defined(PIOS_INCLUDE_USB_RCTX)
#include "pios_usb_rctx.h"
#endif	/* PIOS_INCLUDE_USB_RCTX */

// Private constants
#if defined(PIOS_MANUAL_STACK_SIZE)
#define STACK_SIZE_BYTES PIOS_MANUAL_STACK_SIZE
#else
#define STACK_SIZE_BYTES 1024
#endif

#define TASK_PRIORITY (tskIDLE_PRIORITY+4)
#define UPDATE_PERIOD_MS 20
#define THROTTLE_FAILSAFE -0.1f
#define ARMED_TIME_MS      1000
#define ARMED_THRESHOLD    0.50f
//safe band to allow a bit of calibration error or trim offset (in microseconds)
#define CONNECTION_OFFSET 250

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
static portTickType lastSysTime;

#ifdef USE_INPUT_LPF
static portTickType lastSysTimeLPF;
static float inputFiltered[MANUALCONTROLSETTINGS_RESPONSETIME_NUMELEM];
#endif

// Private functions
static void updateActuatorDesired(ManualControlCommandData * cmd);
static void updateStabilizationDesired(ManualControlCommandData * cmd, ManualControlSettingsData * settings);
static void altitudeHoldDesired(ManualControlCommandData * cmd, bool changed);
static void updatePathDesired(ManualControlCommandData * cmd, bool changed, bool home);
static void processFlightMode(ManualControlSettingsData * settings, float flightMode);
static void processArm(ManualControlCommandData * cmd, ManualControlSettingsData * settings);
static void setArmedIfChanged(uint8_t val);

static void manualControlTask(void *parameters);
static float scaleChannel(int16_t value, int16_t max, int16_t min, int16_t neutral);
static uint32_t timeDifferenceMs(portTickType start_time, portTickType end_time);
static bool okToArm(void);
static bool validInputRange(int16_t min, int16_t max, uint16_t value);
static void applyDeadband(float *value, float deadband);

#ifdef USE_INPUT_LPF
static void applyLPF(float *value, ManualControlSettingsResponseTimeElem channel, ManualControlSettingsData *settings, float dT);
#endif

#define RCVR_ACTIVITY_MONITOR_CHANNELS_PER_GROUP 12
#define RCVR_ACTIVITY_MONITOR_MIN_RANGE 10
struct rcvr_activity_fsm {
	ManualControlSettingsChannelGroupsOptions group;
	uint16_t prev[RCVR_ACTIVITY_MONITOR_CHANNELS_PER_GROUP];
	uint8_t sample_count;
};
static struct rcvr_activity_fsm activity_fsm;

static void resetRcvrActivity(struct rcvr_activity_fsm * fsm);
static bool updateRcvrActivity(struct rcvr_activity_fsm * fsm);

#define assumptions (assumptions1 && assumptions3 && assumptions5 && assumptions7 && assumptions8 && assumptions_flightmode && assumptions_channelcount)

/**
 * Module starting
 */
int32_t ManualControlStart()
{
	// Start main task
	xTaskCreate(manualControlTask, (signed char *)"ManualControl", STACK_SIZE_BYTES/4, NULL, TASK_PRIORITY, &taskHandle);
	TaskMonitorAdd(TASKINFO_RUNNING_MANUALCONTROL, taskHandle);
	PIOS_WDG_RegisterFlag(PIOS_WDG_MANUAL);

	return 0;
}

/**
 * Module initialization
 */
int32_t ManualControlInitialize()
{

	/* Check the assumptions about uavobject enum's are correct */
	if(!assumptions)
		return -1;

	AccessoryDesiredInitialize();
	ManualControlCommandInitialize();
	FlightStatusInitialize();
	StabilizationDesiredInitialize();
	ReceiverActivityInitialize();
	ManualControlSettingsInitialize();

	return 0;
}
MODULE_INITCALL(ManualControlInitialize, ManualControlStart)

/**
 * Module task
 */
static void manualControlTask(void *parameters)
{
	ManualControlSettingsData settings;
	ManualControlCommandData cmd;
	FlightStatusData flightStatus;
	float flightMode = 0;

	uint8_t disconnected_count = 0;
	uint8_t connected_count = 0;

	// For now manual instantiate extra instances of Accessory Desired.  In future should be done dynamically
	// this includes not even registering it if not used
	AccessoryDesiredCreateInstance();
	AccessoryDesiredCreateInstance();

	// Make sure unarmed on power up
	ManualControlCommandGet(&cmd);
	FlightStatusGet(&flightStatus);
	flightStatus.Armed = FLIGHTSTATUS_ARMED_DISARMED;
	armState = ARM_STATE_DISARMED;

	/* Initialize the RcvrActivty FSM */
	portTickType lastActivityTime = xTaskGetTickCount();
	resetRcvrActivity(&activity_fsm);

	// Main task loop
	lastSysTime = xTaskGetTickCount();
	while (1) {
		float scaledChannel[MANUALCONTROLSETTINGS_CHANNELGROUPS_NUMELEM];

		// Wait until next update
		vTaskDelayUntil(&lastSysTime, UPDATE_PERIOD_MS / portTICK_RATE_MS);
		PIOS_WDG_UpdateFlag(PIOS_WDG_MANUAL);

		// Read settings
		ManualControlSettingsGet(&settings);

		/* Update channel activity monitor */
		if (flightStatus.Armed == ARM_STATE_DISARMED) {
			if (updateRcvrActivity(&activity_fsm)) {
				/* Reset the aging timer because activity was detected */
				lastActivityTime = lastSysTime;
			}
		}
		if (timeDifferenceMs(lastActivityTime, lastSysTime) > 5000) {
			resetRcvrActivity(&activity_fsm);
			lastActivityTime = lastSysTime;
		}

		if (ManualControlCommandReadOnly()) {
			FlightTelemetryStatsData flightTelemStats;
			FlightTelemetryStatsGet(&flightTelemStats);
			if(flightTelemStats.Status != FLIGHTTELEMETRYSTATS_STATUS_CONNECTED) {
				/* trying to fly via GCS and lost connection.  fall back to transmitter */
				UAVObjMetadata metadata;
				ManualControlCommandGetMetadata(&metadata);
				UAVObjSetAccess(&metadata, ACCESS_READWRITE);
				ManualControlCommandSetMetadata(&metadata);
			}
		}

		if (!ManualControlCommandReadOnly()) {

			bool valid_input_detected = true;
			
			// Read channel values in us
			for (uint8_t n = 0; 
			     n < MANUALCONTROLSETTINGS_CHANNELGROUPS_NUMELEM && n < MANUALCONTROLCOMMAND_CHANNEL_NUMELEM;
			     ++n) {
				extern uint32_t pios_rcvr_group_map[];

				if (settings.ChannelGroups[n] >= MANUALCONTROLSETTINGS_CHANNELGROUPS_NONE) {
					cmd.Channel[n] = PIOS_RCVR_INVALID;
				} else {
					cmd.Channel[n] = PIOS_RCVR_Read(pios_rcvr_group_map[settings.ChannelGroups[n]],
									settings.ChannelNumber[n]);
				}
				
				// If a channel has timed out this is not valid data and we shouldn't update anything
				// until we decide to go to failsafe
				if(cmd.Channel[n] == PIOS_RCVR_TIMEOUT)
					valid_input_detected = false;
				else
					scaledChannel[n] = scaleChannel(cmd.Channel[n], settings.ChannelMax[n],	settings.ChannelMin[n], settings.ChannelNeutral[n]);
			}

			// Check settings, if error raise alarm
			if (settings.ChannelGroups[MANUALCONTROLSETTINGS_CHANNELGROUPS_ROLL] >= MANUALCONTROLSETTINGS_CHANNELGROUPS_NONE ||
				settings.ChannelGroups[MANUALCONTROLSETTINGS_CHANNELGROUPS_PITCH] >= MANUALCONTROLSETTINGS_CHANNELGROUPS_NONE ||
				settings.ChannelGroups[MANUALCONTROLSETTINGS_CHANNELGROUPS_YAW] >= MANUALCONTROLSETTINGS_CHANNELGROUPS_NONE ||
				settings.ChannelGroups[MANUALCONTROLSETTINGS_CHANNELGROUPS_THROTTLE] >= MANUALCONTROLSETTINGS_CHANNELGROUPS_NONE ||
				// Check all channel mappings are valid
				cmd.Channel[MANUALCONTROLSETTINGS_CHANNELGROUPS_ROLL] == (uint16_t) PIOS_RCVR_INVALID ||
				cmd.Channel[MANUALCONTROLSETTINGS_CHANNELGROUPS_PITCH] == (uint16_t) PIOS_RCVR_INVALID ||
				cmd.Channel[MANUALCONTROLSETTINGS_CHANNELGROUPS_YAW] == (uint16_t) PIOS_RCVR_INVALID ||
				cmd.Channel[MANUALCONTROLSETTINGS_CHANNELGROUPS_THROTTLE] == (uint16_t) PIOS_RCVR_INVALID ||
				// Check the driver exists
				cmd.Channel[MANUALCONTROLSETTINGS_CHANNELGROUPS_ROLL] == (uint16_t) PIOS_RCVR_NODRIVER ||
				cmd.Channel[MANUALCONTROLSETTINGS_CHANNELGROUPS_PITCH] == (uint16_t) PIOS_RCVR_NODRIVER ||
				cmd.Channel[MANUALCONTROLSETTINGS_CHANNELGROUPS_YAW] == (uint16_t) PIOS_RCVR_NODRIVER ||
				cmd.Channel[MANUALCONTROLSETTINGS_CHANNELGROUPS_THROTTLE] == (uint16_t) PIOS_RCVR_NODRIVER ||
				// Check the FlightModeNumber is valid
				settings.FlightModeNumber < 1 || settings.FlightModeNumber > MANUALCONTROLSETTINGS_FLIGHTMODEPOSITION_NUMELEM ||
				// Similar checks for FlightMode channel but only if more than one flight mode has been set. Otherwise don't care
				((settings.FlightModeNumber > 1) && (
					settings.ChannelGroups[MANUALCONTROLSETTINGS_CHANNELGROUPS_FLIGHTMODE] >= MANUALCONTROLSETTINGS_CHANNELGROUPS_NONE ||
					cmd.Channel[MANUALCONTROLSETTINGS_CHANNELGROUPS_FLIGHTMODE] == (uint16_t) PIOS_RCVR_INVALID ||
					cmd.Channel[MANUALCONTROLSETTINGS_CHANNELGROUPS_FLIGHTMODE] == (uint16_t) PIOS_RCVR_NODRIVER))) {

				AlarmsSet(SYSTEMALARMS_ALARM_MANUALCONTROL, SYSTEMALARMS_ALARM_CRITICAL);
				cmd.Connected = MANUALCONTROLCOMMAND_CONNECTED_FALSE;
				ManualControlCommandSet(&cmd);

				// Need to do this here since we don't process armed status.  Since this shouldn't happen in flight (changed config) 
				// immediately disarm
				setArmedIfChanged(FLIGHTSTATUS_ARMED_DISARMED);

				continue;
			}

			// decide if we have valid manual input or not
			valid_input_detected &= validInputRange(settings.ChannelMin[MANUALCONTROLSETTINGS_CHANNELGROUPS_THROTTLE], settings.ChannelMax[MANUALCONTROLSETTINGS_CHANNELGROUPS_THROTTLE], cmd.Channel[MANUALCONTROLSETTINGS_CHANNELGROUPS_THROTTLE]) &&
			     validInputRange(settings.ChannelMin[MANUALCONTROLSETTINGS_CHANNELGROUPS_ROLL], settings.ChannelMax[MANUALCONTROLSETTINGS_CHANNELGROUPS_ROLL], cmd.Channel[MANUALCONTROLSETTINGS_CHANNELGROUPS_ROLL]) &&
			     validInputRange(settings.ChannelMin[MANUALCONTROLSETTINGS_CHANNELGROUPS_YAW], settings.ChannelMax[MANUALCONTROLSETTINGS_CHANNELGROUPS_YAW], cmd.Channel[MANUALCONTROLSETTINGS_CHANNELGROUPS_YAW]) &&
			     validInputRange(settings.ChannelMin[MANUALCONTROLSETTINGS_CHANNELGROUPS_PITCH], settings.ChannelMax[MANUALCONTROLSETTINGS_CHANNELGROUPS_PITCH], cmd.Channel[MANUALCONTROLSETTINGS_CHANNELGROUPS_PITCH]);

			// Implement hysteresis loop on connection status
			if (valid_input_detected && (++connected_count > 10)) {
				cmd.Connected = MANUALCONTROLCOMMAND_CONNECTED_TRUE;
				connected_count = 0;
				disconnected_count = 0;
			} else if (!valid_input_detected && (++disconnected_count > 10)) {
				cmd.Connected = MANUALCONTROLCOMMAND_CONNECTED_FALSE;
				connected_count = 0;
				disconnected_count = 0;
			}

			if (cmd.Connected == MANUALCONTROLCOMMAND_CONNECTED_FALSE) {
				cmd.Throttle = -1;	// Shut down engine with no control
				cmd.Roll = 0;
				cmd.Yaw = 0;
				cmd.Pitch = 0;
				cmd.Collective = 0;
				//cmd.FlightMode = MANUALCONTROLCOMMAND_FLIGHTMODE_AUTO; // don't do until AUTO implemented and functioning
				// Important: Throttle < 0 will reset Stabilization coefficients among other things. Either change this,
				// or leave throttle at IDLE speed or above when going into AUTO-failsafe.
				AlarmsSet(SYSTEMALARMS_ALARM_MANUALCONTROL, SYSTEMALARMS_ALARM_WARNING);
				
				AccessoryDesiredData accessory;
				// Set Accessory 0
				if (settings.ChannelGroups[MANUALCONTROLSETTINGS_CHANNELGROUPS_ACCESSORY0] != 
					MANUALCONTROLSETTINGS_CHANNELGROUPS_NONE) {
					accessory.AccessoryVal = 0;
					if(AccessoryDesiredInstSet(0, &accessory) != 0)
						AlarmsSet(SYSTEMALARMS_ALARM_MANUALCONTROL, SYSTEMALARMS_ALARM_WARNING);
				}
				// Set Accessory 1
				if (settings.ChannelGroups[MANUALCONTROLSETTINGS_CHANNELGROUPS_ACCESSORY1] != 
					MANUALCONTROLSETTINGS_CHANNELGROUPS_NONE) {
					accessory.AccessoryVal = 0;
					if(AccessoryDesiredInstSet(1, &accessory) != 0)
						AlarmsSet(SYSTEMALARMS_ALARM_MANUALCONTROL, SYSTEMALARMS_ALARM_WARNING);
				}
				// Set Accessory 2
				if (settings.ChannelGroups[MANUALCONTROLSETTINGS_CHANNELGROUPS_ACCESSORY2] != 
					MANUALCONTROLSETTINGS_CHANNELGROUPS_NONE) {
					accessory.AccessoryVal = 0;
					if(AccessoryDesiredInstSet(2, &accessory) != 0)
						AlarmsSet(SYSTEMALARMS_ALARM_MANUALCONTROL, SYSTEMALARMS_ALARM_WARNING);
				}

			} else if (valid_input_detected) {
				AlarmsClear(SYSTEMALARMS_ALARM_MANUALCONTROL);

				// Scale channels to -1 -> +1 range
				cmd.Roll           = scaledChannel[MANUALCONTROLSETTINGS_CHANNELGROUPS_ROLL];
				cmd.Pitch          = scaledChannel[MANUALCONTROLSETTINGS_CHANNELGROUPS_PITCH];
				cmd.Yaw            = scaledChannel[MANUALCONTROLSETTINGS_CHANNELGROUPS_YAW];
				cmd.Throttle       = scaledChannel[MANUALCONTROLSETTINGS_CHANNELGROUPS_THROTTLE];
				flightMode         = scaledChannel[MANUALCONTROLSETTINGS_CHANNELGROUPS_FLIGHTMODE];

				// Apply deadband for Roll/Pitch/Yaw stick inputs
				if (settings.Deadband) {
					applyDeadband(&cmd.Roll, settings.Deadband);
					applyDeadband(&cmd.Pitch, settings.Deadband);
					applyDeadband(&cmd.Yaw, settings.Deadband);
				}
#ifdef USE_INPUT_LPF
				// Apply Low Pass Filter to input channels, time delta between calls in ms
				portTickType thisSysTime = xTaskGetTickCount();
				float dT = (thisSysTime > lastSysTimeLPF) ?
						(float)((thisSysTime - lastSysTimeLPF) * portTICK_RATE_MS) :
						(float)UPDATE_PERIOD_MS;
				lastSysTimeLPF = thisSysTime;

				applyLPF(&cmd.Roll, MANUALCONTROLSETTINGS_RESPONSETIME_ROLL, &settings, dT);
				applyLPF(&cmd.Pitch, MANUALCONTROLSETTINGS_RESPONSETIME_PITCH, &settings, dT);
				applyLPF(&cmd.Yaw, MANUALCONTROLSETTINGS_RESPONSETIME_YAW, &settings, dT);
#endif // USE_INPUT_LPF
				if(cmd.Channel[MANUALCONTROLSETTINGS_CHANNELGROUPS_COLLECTIVE] != (uint16_t) PIOS_RCVR_INVALID &&
				   cmd.Channel[MANUALCONTROLSETTINGS_CHANNELGROUPS_COLLECTIVE] != (uint16_t) PIOS_RCVR_NODRIVER &&
				   cmd.Channel[MANUALCONTROLSETTINGS_CHANNELGROUPS_COLLECTIVE] != (uint16_t) PIOS_RCVR_TIMEOUT)
					cmd.Collective = scaledChannel[MANUALCONTROLSETTINGS_CHANNELGROUPS_COLLECTIVE];
				   
				AccessoryDesiredData accessory;
				// Set Accessory 0
				if (settings.ChannelGroups[MANUALCONTROLSETTINGS_CHANNELGROUPS_ACCESSORY0] != 
					MANUALCONTROLSETTINGS_CHANNELGROUPS_NONE) {
					accessory.AccessoryVal = scaledChannel[MANUALCONTROLSETTINGS_CHANNELGROUPS_ACCESSORY0];
#ifdef USE_INPUT_LPF
					applyLPF(&accessory.AccessoryVal, MANUALCONTROLSETTINGS_RESPONSETIME_ACCESSORY0, &settings, dT);
#endif
					if(AccessoryDesiredInstSet(0, &accessory) != 0)
						AlarmsSet(SYSTEMALARMS_ALARM_MANUALCONTROL, SYSTEMALARMS_ALARM_WARNING);
				}
				// Set Accessory 1
				if (settings.ChannelGroups[MANUALCONTROLSETTINGS_CHANNELGROUPS_ACCESSORY1] != 
					MANUALCONTROLSETTINGS_CHANNELGROUPS_NONE) {
					accessory.AccessoryVal = scaledChannel[MANUALCONTROLSETTINGS_CHANNELGROUPS_ACCESSORY1];
#ifdef USE_INPUT_LPF
					applyLPF(&accessory.AccessoryVal, MANUALCONTROLSETTINGS_RESPONSETIME_ACCESSORY1, &settings, dT);
#endif
					if(AccessoryDesiredInstSet(1, &accessory) != 0)
						AlarmsSet(SYSTEMALARMS_ALARM_MANUALCONTROL, SYSTEMALARMS_ALARM_WARNING);
				}
				// Set Accessory 2
				if (settings.ChannelGroups[MANUALCONTROLSETTINGS_CHANNELGROUPS_ACCESSORY2] != 
					MANUALCONTROLSETTINGS_CHANNELGROUPS_NONE) {
					accessory.AccessoryVal = scaledChannel[MANUALCONTROLSETTINGS_CHANNELGROUPS_ACCESSORY2];
#ifdef USE_INPUT_LPF
					applyLPF(&accessory.AccessoryVal, MANUALCONTROLSETTINGS_RESPONSETIME_ACCESSORY2, &settings, dT);
#endif
					if(AccessoryDesiredInstSet(2, &accessory) != 0)
						AlarmsSet(SYSTEMALARMS_ALARM_MANUALCONTROL, SYSTEMALARMS_ALARM_WARNING);
				}

				processFlightMode(&settings, flightMode);

			}

			// Process arming outside conditional so system will disarm when disconnected
			processArm(&cmd, &settings);
			
			// Update cmd object
			ManualControlCommandSet(&cmd);
#if defined(PIOS_INCLUDE_USB_RCTX)
			if (pios_usb_rctx_id) {
				PIOS_USB_RCTX_Update(pios_usb_rctx_id,
						cmd.Channel,
						settings.ChannelMin,
						settings.ChannelMax,
						NELEMENTS(cmd.Channel));
			}
#endif	/* PIOS_INCLUDE_USB_RCTX */

		} else {
			ManualControlCommandGet(&cmd);	/* Under GCS control */
		}

		FlightStatusGet(&flightStatus);

		// Depending on the mode update the Stabilization or Actuator objects
		static uint8_t lastFlightMode = FLIGHTSTATUS_FLIGHTMODE_MANUAL;
		switch(PARSE_FLIGHT_MODE(flightStatus.FlightMode)) {
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
			case FLIGHTMODE_TUNING:
				// Tuning takes settings directly from manualcontrolcommand.  No need to
				// call anything else.  This just avoids errors.
				break;
			case FLIGHTMODE_GUIDANCE:
				switch(flightStatus.FlightMode) {
					case FLIGHTSTATUS_FLIGHTMODE_ALTITUDEHOLD:
						altitudeHoldDesired(&cmd, lastFlightMode != flightStatus.FlightMode);
						break;
					case FLIGHTSTATUS_FLIGHTMODE_POSITIONHOLD:
						updatePathDesired(&cmd, lastFlightMode != flightStatus.FlightMode, false);
						break;
					case FLIGHTSTATUS_FLIGHTMODE_RETURNTOBASE:
						updatePathDesired(&cmd, lastFlightMode != flightStatus.FlightMode, true);
						break;
					default:
						AlarmsSet(SYSTEMALARMS_ALARM_MANUALCONTROL, SYSTEMALARMS_ALARM_CRITICAL);
				}
				break;
		}
		lastFlightMode = flightStatus.FlightMode;
	}
}

static void resetRcvrActivity(struct rcvr_activity_fsm * fsm)
{
	ReceiverActivityData data;
	bool updated = false;

	/* Clear all channel activity flags */
	ReceiverActivityGet(&data);
	if (data.ActiveGroup != RECEIVERACTIVITY_ACTIVEGROUP_NONE &&
		data.ActiveChannel != 255) {
		data.ActiveGroup = RECEIVERACTIVITY_ACTIVEGROUP_NONE;
		data.ActiveChannel = 255;
		updated = true;
	}
	if (updated) {
		ReceiverActivitySet(&data);
	}

	/* Reset the FSM state */
	fsm->group        = 0;
	fsm->sample_count = 0;
}

static void updateRcvrActivitySample(uint32_t rcvr_id, uint16_t samples[], uint8_t max_channels) {
	for (uint8_t channel = 1; channel <= max_channels; channel++) {
		// Subtract 1 because channels are 1 indexed
		samples[channel - 1] = PIOS_RCVR_Read(rcvr_id, channel);
	}
}

static bool updateRcvrActivityCompare(uint32_t rcvr_id, struct rcvr_activity_fsm * fsm)
{
	bool activity_updated = false;

	/* Compare the current value to the previous sampled value */
	for (uint8_t channel = 1;
	     channel <= RCVR_ACTIVITY_MONITOR_CHANNELS_PER_GROUP;
	     channel++) {
		uint16_t delta;
		uint16_t prev = fsm->prev[channel - 1];   // Subtract 1 because channels are 1 indexed
		uint16_t curr = PIOS_RCVR_Read(rcvr_id, channel); 
		if (curr > prev) {
			delta = curr - prev;
		} else {
			delta = prev - curr;
		}

		if (delta > RCVR_ACTIVITY_MONITOR_MIN_RANGE) {
			/* Mark this channel as active */
			ReceiverActivityActiveGroupOptions group;

			/* Don't assume manualcontrolsettings and receiveractivity are in the same order. */
			switch (fsm->group) {
			case MANUALCONTROLSETTINGS_CHANNELGROUPS_PWM: 
				group = RECEIVERACTIVITY_ACTIVEGROUP_PWM;
				break;
			case MANUALCONTROLSETTINGS_CHANNELGROUPS_PPM:
				group = RECEIVERACTIVITY_ACTIVEGROUP_PPM;
				break;
			case MANUALCONTROLSETTINGS_CHANNELGROUPS_DSMMAINPORT:
				group = RECEIVERACTIVITY_ACTIVEGROUP_DSMMAINPORT;
				break;
			case MANUALCONTROLSETTINGS_CHANNELGROUPS_DSMFLEXIPORT:
				group = RECEIVERACTIVITY_ACTIVEGROUP_DSMFLEXIPORT;
				break;
			case MANUALCONTROLSETTINGS_CHANNELGROUPS_SBUS:
				group = RECEIVERACTIVITY_ACTIVEGROUP_SBUS;
				break;
			case MANUALCONTROLSETTINGS_CHANNELGROUPS_GCS:
				group = RECEIVERACTIVITY_ACTIVEGROUP_GCS;
				break;
			case MANUALCONTROLSETTINGS_CHANNELGROUPS_OPLINK:
				group = RECEIVERACTIVITY_ACTIVEGROUP_OPLINK;
				break;
			default:
				PIOS_Assert(0);
				break;
			}

			ReceiverActivityActiveGroupSet((uint8_t*)&group);
			ReceiverActivityActiveChannelSet(&channel);
			activity_updated = true;
		}
	}
	return (activity_updated);
}

static bool updateRcvrActivity(struct rcvr_activity_fsm * fsm)
{
	bool activity_updated = false;

	if (fsm->group >= MANUALCONTROLSETTINGS_CHANNELGROUPS_NONE) {
		/* We're out of range, reset things */
		resetRcvrActivity(fsm);
	}

	extern uint32_t pios_rcvr_group_map[];
	if (!pios_rcvr_group_map[fsm->group]) {
		/* Unbound group, skip it */
		goto group_completed;
	}

	if (fsm->sample_count == 0) {
		/* Take a sample of each channel in this group */
		updateRcvrActivitySample(pios_rcvr_group_map[fsm->group],
					fsm->prev,
					NELEMENTS(fsm->prev));
		fsm->sample_count++;
		return (false);
	}

	/* Compare with previous sample */
	activity_updated = updateRcvrActivityCompare(pios_rcvr_group_map[fsm->group], fsm);

group_completed:
	/* Reset the sample counter */
	fsm->sample_count = 0;

	/* Find the next active group, but limit search so we can't loop forever here */
	for (uint8_t i = 0; i < MANUALCONTROLSETTINGS_CHANNELGROUPS_NONE; i++) {
		/* Move to the next group */
		fsm->group++;
		if (fsm->group >= MANUALCONTROLSETTINGS_CHANNELGROUPS_NONE) {
			/* Wrap back to the first group */
			fsm->group = 0;
		}
		if (pios_rcvr_group_map[fsm->group]) {
			/* 
			 * Found an active group, take a sample here to avoid an
			 * extra 20ms delay in the main thread so we can speed up
			 * this algorithm.
			 */
			updateRcvrActivitySample(pios_rcvr_group_map[fsm->group],
						fsm->prev,
						NELEMENTS(fsm->prev));
			fsm->sample_count++;
			break;
		}
	}

	return (activity_updated);
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
	FlightStatusData flightStatus;
	FlightStatusGet(&flightStatus);
	switch(flightStatus.FlightMode) {
		case FLIGHTSTATUS_FLIGHTMODE_STABILIZED1:
			stab_settings = settings->Stabilization1Settings;
			break;
		case FLIGHTSTATUS_FLIGHTMODE_STABILIZED2:
			stab_settings = settings->Stabilization2Settings;
			break;
		case FLIGHTSTATUS_FLIGHTMODE_STABILIZED3:
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
	     (stab_settings[0] == STABILIZATIONDESIRED_STABILIZATIONMODE_WEAKLEVELING) ? cmd->Roll * stabSettings.ManualRate[STABILIZATIONSETTINGS_MANUALRATE_ROLL] :
	     (stab_settings[0] == STABILIZATIONDESIRED_STABILIZATIONMODE_ATTITUDE) ? cmd->Roll * stabSettings.RollMax :
	     (stab_settings[0] == STABILIZATIONDESIRED_STABILIZATIONMODE_AXISLOCK) ? cmd->Roll * stabSettings.ManualRate[STABILIZATIONSETTINGS_MANUALRATE_ROLL] :
	     (stab_settings[0] == STABILIZATIONDESIRED_STABILIZATIONMODE_VIRTUALBAR) ? cmd->Roll :
	     (stab_settings[0] == STABILIZATIONDESIRED_STABILIZATIONMODE_RELAYRATE) ? cmd->Roll * stabSettings.ManualRate[STABILIZATIONSETTINGS_MANUALRATE_ROLL] :
	     (stab_settings[0] == STABILIZATIONDESIRED_STABILIZATIONMODE_RELAYATTITUDE) ? cmd->Roll * stabSettings.RollMax :
	     0; // this is an invalid mode
					      ;
	stabilization.Pitch = (stab_settings[1] == STABILIZATIONDESIRED_STABILIZATIONMODE_NONE) ? cmd->Pitch :
	     (stab_settings[1] == STABILIZATIONDESIRED_STABILIZATIONMODE_RATE) ? cmd->Pitch * stabSettings.ManualRate[STABILIZATIONSETTINGS_MANUALRATE_PITCH] :
	     (stab_settings[1] == STABILIZATIONDESIRED_STABILIZATIONMODE_WEAKLEVELING) ? cmd->Pitch * stabSettings.ManualRate[STABILIZATIONSETTINGS_MANUALRATE_PITCH] :
	     (stab_settings[1] == STABILIZATIONDESIRED_STABILIZATIONMODE_ATTITUDE) ? cmd->Pitch * stabSettings.PitchMax :
	     (stab_settings[1] == STABILIZATIONDESIRED_STABILIZATIONMODE_AXISLOCK) ? cmd->Pitch * stabSettings.ManualRate[STABILIZATIONSETTINGS_MANUALRATE_PITCH] :
	     (stab_settings[1] == STABILIZATIONDESIRED_STABILIZATIONMODE_VIRTUALBAR) ? cmd->Pitch :
	     (stab_settings[1] == STABILIZATIONDESIRED_STABILIZATIONMODE_RELAYRATE) ? cmd->Pitch * stabSettings.ManualRate[STABILIZATIONSETTINGS_MANUALRATE_PITCH] :
	     (stab_settings[1] == STABILIZATIONDESIRED_STABILIZATIONMODE_RELAYATTITUDE) ? cmd->Pitch * stabSettings.PitchMax :
	     0; // this is an invalid mode

	stabilization.Yaw = (stab_settings[2] == STABILIZATIONDESIRED_STABILIZATIONMODE_NONE) ? cmd->Yaw :
	     (stab_settings[2] == STABILIZATIONDESIRED_STABILIZATIONMODE_RATE) ? cmd->Yaw * stabSettings.ManualRate[STABILIZATIONSETTINGS_MANUALRATE_YAW] :
	     (stab_settings[2] == STABILIZATIONDESIRED_STABILIZATIONMODE_WEAKLEVELING) ? cmd->Yaw * stabSettings.ManualRate[STABILIZATIONSETTINGS_MANUALRATE_YAW] :
	     (stab_settings[2] == STABILIZATIONDESIRED_STABILIZATIONMODE_ATTITUDE) ? cmd->Yaw * stabSettings.YawMax :
	     (stab_settings[2] == STABILIZATIONDESIRED_STABILIZATIONMODE_AXISLOCK) ? cmd->Yaw * stabSettings.ManualRate[STABILIZATIONSETTINGS_MANUALRATE_YAW] :
	     (stab_settings[2] == STABILIZATIONDESIRED_STABILIZATIONMODE_VIRTUALBAR) ? cmd->Yaw :
	     (stab_settings[2] == STABILIZATIONDESIRED_STABILIZATIONMODE_RELAYRATE) ? cmd->Yaw * stabSettings.ManualRate[STABILIZATIONSETTINGS_MANUALRATE_YAW] :
	     (stab_settings[2] == STABILIZATIONDESIRED_STABILIZATIONMODE_RELAYATTITUDE) ? cmd->Yaw * stabSettings.YawMax :
	     0; // this is an invalid mode

	stabilization.Throttle = (cmd->Throttle < 0) ? -1 : cmd->Throttle;
	StabilizationDesiredSet(&stabilization);
}

#if defined(REVOLUTION)
// TODO: Need compile flag to exclude this from copter control
/**
 * @brief Update the position desired to current location when
 * enabled and allow the waypoint to be moved by transmitter
 */
static void updatePathDesired(ManualControlCommandData * cmd, bool changed,bool home)
{
	static portTickType lastSysTime;
	portTickType thisSysTime;
	float dT;

	thisSysTime = xTaskGetTickCount();
	dT = (thisSysTime - lastSysTime) / portTICK_RATE_MS / 1000.0f;
	lastSysTime = thisSysTime;

	if (home && changed) {
		// Simple Return To Base mode - keep altitude the same, fly to home position
		PositionActualData positionActual;
		PositionActualGet(&positionActual);
		
		PathDesiredData pathDesired;
		PathDesiredGet(&pathDesired);
		pathDesired.Start[PATHDESIRED_START_NORTH] = 0;
		pathDesired.Start[PATHDESIRED_START_EAST] = 0;
		pathDesired.Start[PATHDESIRED_START_DOWN] = positionActual.Down - 10;
		pathDesired.End[PATHDESIRED_END_NORTH] = 0;
		pathDesired.End[PATHDESIRED_END_EAST] = 0;
		pathDesired.End[PATHDESIRED_END_DOWN] = positionActual.Down - 10;
		pathDesired.StartingVelocity=1;
		pathDesired.EndingVelocity=0;
		pathDesired.Mode = PATHDESIRED_MODE_FLYENDPOINT;
		PathDesiredSet(&pathDesired);
	} else if(changed) {
		// After not being in this mode for a while init at current height
		PositionActualData positionActual;
		PositionActualGet(&positionActual);
		
		PathDesiredData pathDesired;
		PathDesiredGet(&pathDesired);
		pathDesired.Start[PATHDESIRED_END_NORTH] = positionActual.North;
		pathDesired.Start[PATHDESIRED_END_EAST] = positionActual.East;
		pathDesired.Start[PATHDESIRED_END_DOWN] = positionActual.Down - 10;
		pathDesired.End[PATHDESIRED_END_NORTH] = positionActual.North;
		pathDesired.End[PATHDESIRED_END_EAST] = positionActual.East;
		pathDesired.End[PATHDESIRED_END_DOWN] = positionActual.Down - 10;
		pathDesired.StartingVelocity=1;
		pathDesired.EndingVelocity=0;
		pathDesired.Mode = PATHDESIRED_MODE_FLYENDPOINT;
		PathDesiredSet(&pathDesired);
	} else {
		
/*Disable this section, until such time as proper discussion can be had about how to implement it for all types of crafts.		
		PathDesiredData pathDesired;
		PathDesiredGet(&pathDesired);
		pathDesired.End[PATHDESIRED_END_NORTH] += dT * -cmd->Pitch;
		pathDesired.End[PATHDESIRED_END_EAST] += dT * cmd->Roll;
		pathDesired.Mode = PATHDESIRED_MODE_FLYENDPOINT;
		PathDesiredSet(&pathDesired);
*/ 
	}
}

/**
 * @brief Update the altitude desired to current altitude when
 * enabled and enable altitude mode for stabilization
 * @todo: Need compile flag to exclude this from copter control
 */
static void altitudeHoldDesired(ManualControlCommandData * cmd, bool changed)
{
	const float DEADBAND_HIGH = 0.55;
	const float DEADBAND_LOW = 0.45;
	
	static portTickType lastSysTime;
	static bool zeroed = false;
	portTickType thisSysTime;
	float dT;
	AltitudeHoldDesiredData altitudeHoldDesired;
	AltitudeHoldDesiredGet(&altitudeHoldDesired);

	StabilizationSettingsData stabSettings;
	StabilizationSettingsGet(&stabSettings);

	thisSysTime = xTaskGetTickCount();
	dT = (thisSysTime - lastSysTime) / portTICK_RATE_MS / 1000.0f;
	lastSysTime = thisSysTime;

	altitudeHoldDesired.Roll = cmd->Roll * stabSettings.RollMax;
	altitudeHoldDesired.Pitch = cmd->Pitch * stabSettings.PitchMax;
	altitudeHoldDesired.Yaw = cmd->Yaw * stabSettings.ManualRate[STABILIZATIONSETTINGS_MANUALRATE_YAW];
	
	float currentDown;
	PositionActualDownGet(&currentDown);
	if(changed) {
		// After not being in this mode for a while init at current height
		altitudeHoldDesired.Altitude = 0;
		zeroed = false;
	} else if (cmd->Throttle > DEADBAND_HIGH && zeroed)
		altitudeHoldDesired.Altitude += (cmd->Throttle - DEADBAND_HIGH) * dT;
	else if (cmd->Throttle < DEADBAND_LOW && zeroed)
		altitudeHoldDesired.Altitude += (cmd->Throttle - DEADBAND_LOW) * dT;
	else if (cmd->Throttle >= DEADBAND_LOW && cmd->Throttle <= DEADBAND_HIGH)  // Require the stick to enter the dead band before they can move height
		zeroed = true;
	
	AltitudeHoldDesiredSet(&altitudeHoldDesired);
}
#else

// TODO: These functions should never be accessible on CC.  Any configuration that
// could allow them to be called sholud already throw an error to prevent this happening
// in flight
static void updatePathDesired(ManualControlCommandData * cmd, bool changed, bool home)
{
	AlarmsSet(SYSTEMALARMS_ALARM_MANUALCONTROL, SYSTEMALARMS_ALARM_ERROR);
}

static void altitudeHoldDesired(ManualControlCommandData * cmd, bool changed)
{
	AlarmsSet(SYSTEMALARMS_ALARM_MANUALCONTROL, SYSTEMALARMS_ALARM_ERROR);
}
#endif
/**
 * Convert channel from servo pulse duration (microseconds) to scaled -1/+1 range.
 */
static float scaleChannel(int16_t value, int16_t max, int16_t min, int16_t neutral)
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
 * @brief Determine if the aircraft is forced to disarm by an explicit alarm
 * @returns True if safe to arm, false otherwise
 */
static bool forcedDisArm(void)
{
	// read alarms
	SystemAlarmsData alarms;
	SystemAlarmsGet(&alarms);

	if (alarms.Alarm[SYSTEMALARMS_ALARM_GUIDANCE] == SYSTEMALARMS_ALARM_CRITICAL) {
		return true;
	}
	return false;
}

/**
 * @brief Update the flightStatus object only if value changed.  Reduces callbacks
 * @param[in] val The new value
 */
static void setArmedIfChanged(uint8_t val) {
	FlightStatusData flightStatus;
	FlightStatusGet(&flightStatus);

	if(flightStatus.Armed != val) {
		flightStatus.Armed = val;
		FlightStatusSet(&flightStatus);
	}
}

/**
 * @brief Process the inputs and determine whether to arm or not
 * @param[out] cmd The structure to set the armed in
 * @param[in] settings Settings indicating the necessary position
 */
static void processArm(ManualControlCommandData * cmd, ManualControlSettingsData * settings)
{

	bool lowThrottle = cmd->Throttle <= 0;

	if (forcedDisArm()) {
		// PathPlanner forces explicit disarming due to error condition (crash, impact, fire, ...)
		setArmedIfChanged(FLIGHTSTATUS_ARMED_DISARMED);
		return;
	}

	if (settings->Arming == MANUALCONTROLSETTINGS_ARMING_ALWAYSDISARMED) {
		// In this configuration we always disarm
		setArmedIfChanged(FLIGHTSTATUS_ARMED_DISARMED);
	} else {
		// Not really needed since this function not called when disconnected
		if (cmd->Connected == MANUALCONTROLCOMMAND_CONNECTED_FALSE)
			lowThrottle = true;

		// The throttle is not low, in case we where arming or disarming, abort
		if (!lowThrottle) {
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
			return;
		}

		// The rest of these cases throttle is low
		if (settings->Arming == MANUALCONTROLSETTINGS_ARMING_ALWAYSARMED) {
			// In this configuration, we go into armed state as soon as the throttle is low, never disarm
			setArmedIfChanged(FLIGHTSTATUS_ARMED_ARMED);
			return;
		}


		// When the configuration is not "Always armed" and no "Always disarmed",
		// the state will not be changed when the throttle is not low
		static portTickType armedDisarmStart;
		float armingInputLevel = 0;

		// Calc channel see assumptions7
		int8_t sign = ((settings->Arming-MANUALCONTROLSETTINGS_ARMING_ROLLLEFT)%2) ? -1 : 1;
		switch ( (settings->Arming-MANUALCONTROLSETTINGS_ARMING_ROLLLEFT)/2 ) {
			case ARMING_CHANNEL_ROLL:    armingInputLevel = sign * cmd->Roll;    break;
			case ARMING_CHANNEL_PITCH:   armingInputLevel = sign * cmd->Pitch;   break;
			case ARMING_CHANNEL_YAW:     armingInputLevel = sign * cmd->Yaw;     break;
		}

		bool manualArm = false;
		bool manualDisarm = false;

		if (armingInputLevel <= -ARMED_THRESHOLD)
			manualArm = true;
		else if (armingInputLevel >= +ARMED_THRESHOLD)
			manualDisarm = true;

		switch(armState) {
			case ARM_STATE_DISARMED:
				setArmedIfChanged(FLIGHTSTATUS_ARMED_DISARMED);

				// only allow arming if it's OK too
				if (manualArm && okToArm()) {
					armedDisarmStart = lastSysTime;
					armState = ARM_STATE_ARMING_MANUAL;
				}
				break;

			case ARM_STATE_ARMING_MANUAL:
				setArmedIfChanged(FLIGHTSTATUS_ARMED_ARMING);

				if (manualArm && (timeDifferenceMs(armedDisarmStart, lastSysTime) > ARMED_TIME_MS))
					armState = ARM_STATE_ARMED;
				else if (!manualArm)
					armState = ARM_STATE_DISARMED;
				break;

			case ARM_STATE_ARMED:
				// When we get here, the throttle is low,
				// we go immediately to disarming due to timeout, also when the disarming mechanism is not enabled
				armedDisarmStart = lastSysTime;
				armState = ARM_STATE_DISARMING_TIMEOUT;
				setArmedIfChanged(FLIGHTSTATUS_ARMED_ARMED);
				break;

			case ARM_STATE_DISARMING_TIMEOUT:
				// We get here when armed while throttle low, even when the arming timeout is not enabled
				if ((settings->ArmedTimeout != 0) && (timeDifferenceMs(armedDisarmStart, lastSysTime) > settings->ArmedTimeout))
					armState = ARM_STATE_DISARMED;

				// Switch to disarming due to manual control when needed
				if (manualDisarm) {
					armedDisarmStart = lastSysTime;
					armState = ARM_STATE_DISARMING_MANUAL;
				}
				break;

			case ARM_STATE_DISARMING_MANUAL:
				if (manualDisarm &&(timeDifferenceMs(armedDisarmStart, lastSysTime) > ARMED_TIME_MS))
					armState = ARM_STATE_DISARMED;
				else if (!manualDisarm)
					armState = ARM_STATE_ARMED;
				break;
		}	// End Switch
	}
}

/**
 * @brief Determine which of N positions the flight mode switch is in and set flight mode accordingly
 * @param[out] cmd Pointer to the command structure to set the flight mode in
 * @param[in] settings The settings which indicate which position is which mode
 * @param[in] flightMode the value of the switch position
 */
static void processFlightMode(ManualControlSettingsData *settings, float flightMode)
{
	FlightStatusData flightStatus;
	FlightStatusGet(&flightStatus);

	// Convert flightMode value into the switch position in the range [0..N-1]
	uint8_t pos = ((int16_t)(flightMode * 256.0f) + 256) * settings->FlightModeNumber >> 9;
	if (pos >= settings->FlightModeNumber)
		pos = settings->FlightModeNumber - 1;

	uint8_t newMode = settings->FlightModePosition[pos];

	if (flightStatus.FlightMode != newMode) {
		flightStatus.FlightMode = newMode;
		FlightStatusSet(&flightStatus);
	}
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

/**
 * @brief Apply deadband to Roll/Pitch/Yaw channels
 */
static void applyDeadband(float *value, float deadband)
{
	if (fabs(*value) < deadband)
		*value = 0.0f;
	else
		if (*value > 0.0f)
			*value -= deadband;
		else
			*value += deadband;
}

#ifdef USE_INPUT_LPF
/**
 * @brief Apply Low Pass Filter to Throttle/Roll/Pitch/Yaw or Accessory channel
 */
static void applyLPF(float *value, ManualControlSettingsResponseTimeElem channel, ManualControlSettingsData *settings, float dT)
{
	if (settings->ResponseTime[channel]) {
		float rt = (float)settings->ResponseTime[channel];
		inputFiltered[channel] = ((rt * inputFiltered[channel]) + (dT * (*value))) / (rt + dT);
		*value = inputFiltered[channel];
	}
}
#endif // USE_INPUT_LPF

/**
  * @}
  * @}
  */

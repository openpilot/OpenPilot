/**
 ******************************************************************************
 * @addtogroup OpenPilotModules OpenPilot Modules
 * @{
 * @addtogroup ReceiverModule Manual Control Module
 * @brief Provide manual control or allow it alter flight mode.
 * @{
 *
 * Reads in the ManualControlCommand from receiver then
 * pass it to ManualControl
 *
 * @file       receiver.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2014.
 * @brief      Receiver module. Handles safety R/C link and flight mode.
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

#include <openpilot.h>
#include <pios_struct_helper.h>
#include <accessorydesired.h>
#include <manualcontrolsettings.h>
#include <manualcontrolcommand.h>
#include <receiveractivity.h>
#include <flightstatus.h>
#include <flighttelemetrystats.h>
#include <flightmodesettings.h>
#include <systemsettings.h>
#include <taskinfo.h>

#if defined(PIOS_INCLUDE_USB_RCTX)
#include "pios_usb_rctx.h"
#endif /* PIOS_INCLUDE_USB_RCTX */

// Private constants
#if defined(PIOS_RECEIVER_STACK_SIZE)
#define STACK_SIZE_BYTES  PIOS_RECEIVER_STACK_SIZE
#else
#define STACK_SIZE_BYTES  1152
#endif

#define TASK_PRIORITY     (tskIDLE_PRIORITY + 3) // 3 = flight control
#define UPDATE_PERIOD_MS  20
#define THROTTLE_FAILSAFE -0.1f
#define ARMED_THRESHOLD   0.50f
// safe band to allow a bit of calibration error or trim offset (in microseconds)
#define CONNECTION_OFFSET 250

// Private types

// Private variables
static xTaskHandle taskHandle;
static portTickType lastSysTime;

#ifdef USE_INPUT_LPF
static portTickType lastSysTimeLPF;
static float inputFiltered[MANUALCONTROLSETTINGS_RESPONSETIME_NUMELEM];
#endif

// Private functions
static void receiverTask(void *parameters);
static float scaleChannel(int16_t value, int16_t max, int16_t min, int16_t neutral);
static uint32_t timeDifferenceMs(portTickType start_time, portTickType end_time);
static bool validInputRange(int16_t min, int16_t max, uint16_t value);
static void applyDeadband(float *value, float deadband);

#ifdef USE_INPUT_LPF
static void applyLPF(float *value, ManualControlSettingsResponseTimeElem channel, ManualControlSettingsData *settings, float dT);
#endif

#define RCVR_ACTIVITY_MONITOR_CHANNELS_PER_GROUP 12
#define RCVR_ACTIVITY_MONITOR_MIN_RANGE          10
struct rcvr_activity_fsm {
    ManualControlSettingsChannelGroupsOptions group;
    uint16_t prev[RCVR_ACTIVITY_MONITOR_CHANNELS_PER_GROUP];
    uint8_t sample_count;
};
static struct rcvr_activity_fsm activity_fsm;

static void resetRcvrActivity(struct rcvr_activity_fsm *fsm);
static bool updateRcvrActivity(struct rcvr_activity_fsm *fsm);

#define assumptions \
    ( \
        ((int)MANUALCONTROLCOMMAND_CHANNEL_NUMELEM == (int)MANUALCONTROLSETTINGS_CHANNELGROUPS_NUMELEM) && \
        ((int)MANUALCONTROLCOMMAND_CHANNEL_NUMELEM == (int)MANUALCONTROLSETTINGS_CHANNELNUMBER_NUMELEM) && \
        ((int)MANUALCONTROLCOMMAND_CHANNEL_NUMELEM == (int)MANUALCONTROLSETTINGS_CHANNELMIN_NUMELEM) && \
        ((int)MANUALCONTROLCOMMAND_CHANNEL_NUMELEM == (int)MANUALCONTROLSETTINGS_CHANNELMAX_NUMELEM) && \
        ((int)MANUALCONTROLCOMMAND_CHANNEL_NUMELEM == (int)MANUALCONTROLSETTINGS_CHANNELNEUTRAL_NUMELEM))

/**
 * Module starting
 */
int32_t ReceiverStart()
{
    // Start main task
    xTaskCreate(receiverTask, (signed char *)"Receiver", STACK_SIZE_BYTES / 4, NULL, TASK_PRIORITY, &taskHandle);
    PIOS_TASK_MONITOR_RegisterTask(TASKINFO_RUNNING_RECEIVER, taskHandle);
#ifdef PIOS_INCLUDE_WDG
    PIOS_WDG_RegisterFlag(PIOS_WDG_MANUAL);
#endif

    return 0;
}

/**
 * Module initialization
 */
int32_t ReceiverInitialize()
{
    /* Check the assumptions about uavobject enum's are correct */
    PIOS_STATIC_ASSERT(assumptions);

    ManualControlCommandInitialize();
    ReceiverActivityInitialize();
    ManualControlSettingsInitialize();

    return 0;
}
MODULE_INITCALL(ReceiverInitialize, ReceiverStart);

/**
 * Module task
 */
static void receiverTask(__attribute__((unused)) void *parameters)
{
    ManualControlSettingsData settings;
    ManualControlCommandData cmd;
    FlightStatusData flightStatus;

    uint8_t disconnected_count = 0;
    uint8_t connected_count    = 0;

    // For now manual instantiate extra instances of Accessory Desired.  In future should be done dynamically
    // this includes not even registering it if not used
    AccessoryDesiredCreateInstance();
    AccessoryDesiredCreateInstance();

    // Whenever the configuration changes, make sure it is safe to fly

    ManualControlCommandGet(&cmd);
    FlightStatusGet(&flightStatus);

    /* Initialize the RcvrActivty FSM */
    portTickType lastActivityTime = xTaskGetTickCount();
    resetRcvrActivity(&activity_fsm);

    // Main task loop
    lastSysTime = xTaskGetTickCount();

    float scaledChannel[MANUALCONTROLSETTINGS_CHANNELGROUPS_NUMELEM] = { 0 };
    SystemSettingsThrustControlOptions thrustType;

    while (1) {
        // Wait until next update
        vTaskDelayUntil(&lastSysTime, UPDATE_PERIOD_MS / portTICK_RATE_MS);
#ifdef PIOS_INCLUDE_WDG
        PIOS_WDG_UpdateFlag(PIOS_WDG_MANUAL);
#endif

        // Read settings
        ManualControlSettingsGet(&settings);
        SystemSettingsThrustControlGet(&thrustType);

        /* Update channel activity monitor */
        if (flightStatus.Armed == FLIGHTSTATUS_ARMED_DISARMED) {
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
            if (flightTelemStats.Status != FLIGHTTELEMETRYSTATS_STATUS_CONNECTED) {
                /* trying to fly via GCS and lost connection.  fall back to transmitter */
                UAVObjMetadata metadata;
                ManualControlCommandGetMetadata(&metadata);
                UAVObjSetAccess(&metadata, ACCESS_READWRITE);
                ManualControlCommandSetMetadata(&metadata);
            }
            AlarmsSet(SYSTEMALARMS_ALARM_RECEIVER, SYSTEMALARMS_ALARM_WARNING);
            continue;
        }

        bool valid_input_detected = true;

        // Read channel values in us
        for (uint8_t n = 0; n < MANUALCONTROLSETTINGS_CHANNELGROUPS_NUMELEM && n < MANUALCONTROLCOMMAND_CHANNEL_NUMELEM; ++n) {
            extern uint32_t pios_rcvr_group_map[];

            if (cast_struct_to_array(settings.ChannelGroups, settings.ChannelGroups.Roll)[n] >= MANUALCONTROLSETTINGS_CHANNELGROUPS_NONE) {
                cmd.Channel[n] = PIOS_RCVR_INVALID;
            } else {
                cmd.Channel[n] = PIOS_RCVR_Read(pios_rcvr_group_map[
                                                    cast_struct_to_array(settings.ChannelGroups, settings.ChannelGroups.Pitch)[n]],
                                                cast_struct_to_array(settings.ChannelNumber, settings.ChannelNumber.Pitch)[n]);
            }

            // If a channel has timed out this is not valid data and we shouldn't update anything
            // until we decide to go to failsafe
            if (cmd.Channel[n] == (uint16_t)PIOS_RCVR_TIMEOUT) {
                valid_input_detected = false;
            } else {
                scaledChannel[n] = scaleChannel(cmd.Channel[n],
                                                cast_struct_to_array(settings.ChannelMax, settings.ChannelMax.Pitch)[n],
                                                cast_struct_to_array(settings.ChannelMin, settings.ChannelMin.Pitch)[n],
                                                cast_struct_to_array(settings.ChannelNeutral, settings.ChannelNeutral.Pitch)[n]);
            }
        }

        // Check settings, if error raise alarm
        if (settings.ChannelGroups.Roll >= MANUALCONTROLSETTINGS_CHANNELGROUPS_NONE
            || settings.ChannelGroups.Pitch >= MANUALCONTROLSETTINGS_CHANNELGROUPS_NONE
            || settings.ChannelGroups.Yaw >= MANUALCONTROLSETTINGS_CHANNELGROUPS_NONE
            || settings.ChannelGroups.Throttle >= MANUALCONTROLSETTINGS_CHANNELGROUPS_NONE
            ||
            // Check all channel mappings are valid
            cmd.Channel[MANUALCONTROLSETTINGS_CHANNELGROUPS_ROLL] == (uint16_t)PIOS_RCVR_INVALID
            || cmd.Channel[MANUALCONTROLSETTINGS_CHANNELGROUPS_PITCH] == (uint16_t)PIOS_RCVR_INVALID
            || cmd.Channel[MANUALCONTROLSETTINGS_CHANNELGROUPS_YAW] == (uint16_t)PIOS_RCVR_INVALID
            || cmd.Channel[MANUALCONTROLSETTINGS_CHANNELGROUPS_THROTTLE] == (uint16_t)PIOS_RCVR_INVALID
            ||
            // Check the driver exists
            cmd.Channel[MANUALCONTROLSETTINGS_CHANNELGROUPS_ROLL] == (uint16_t)PIOS_RCVR_NODRIVER
            || cmd.Channel[MANUALCONTROLSETTINGS_CHANNELGROUPS_PITCH] == (uint16_t)PIOS_RCVR_NODRIVER
            || cmd.Channel[MANUALCONTROLSETTINGS_CHANNELGROUPS_YAW] == (uint16_t)PIOS_RCVR_NODRIVER
            || cmd.Channel[MANUALCONTROLSETTINGS_CHANNELGROUPS_THROTTLE] == (uint16_t)PIOS_RCVR_NODRIVER
            ||
            // Check collective if required
            (thrustType == SYSTEMSETTINGS_THRUSTCONTROL_COLLECTIVE && (
                 settings.ChannelGroups.Collective >= MANUALCONTROLSETTINGS_CHANNELGROUPS_NONE
                 || cmd.Channel[MANUALCONTROLSETTINGS_CHANNELGROUPS_COLLECTIVE] == (uint16_t)PIOS_RCVR_INVALID
                 || cmd.Channel[MANUALCONTROLSETTINGS_CHANNELGROUPS_COLLECTIVE] == (uint16_t)PIOS_RCVR_NODRIVER))
            ||
            // Check the FlightModeNumber is valid
            settings.FlightModeNumber < 1 || settings.FlightModeNumber > FLIGHTMODESETTINGS_FLIGHTMODEPOSITION_NUMELEM
            ||
            // Similar checks for FlightMode channel but only if more than one flight mode has been set. Otherwise don't care
            ((settings.FlightModeNumber > 1)
             && (settings.ChannelGroups.FlightMode >= MANUALCONTROLSETTINGS_CHANNELGROUPS_NONE
                 || cmd.Channel[MANUALCONTROLSETTINGS_CHANNELGROUPS_FLIGHTMODE] == (uint16_t)PIOS_RCVR_INVALID
                 || cmd.Channel[MANUALCONTROLSETTINGS_CHANNELGROUPS_FLIGHTMODE] == (uint16_t)PIOS_RCVR_NODRIVER))) {
            AlarmsSet(SYSTEMALARMS_ALARM_RECEIVER, SYSTEMALARMS_ALARM_CRITICAL);
            cmd.Connected = MANUALCONTROLCOMMAND_CONNECTED_FALSE;
            ManualControlCommandSet(&cmd);

            continue;
        }

        // decide if we have valid manual input or not
        valid_input_detected &= validInputRange(settings.ChannelMin.Throttle,
                                                settings.ChannelMax.Throttle, cmd.Channel[MANUALCONTROLSETTINGS_CHANNELGROUPS_THROTTLE])
                                && validInputRange(settings.ChannelMin.Roll,
                                                   settings.ChannelMax.Roll, cmd.Channel[MANUALCONTROLSETTINGS_CHANNELGROUPS_ROLL])
                                && validInputRange(settings.ChannelMin.Yaw,
                                                   settings.ChannelMax.Yaw, cmd.Channel[MANUALCONTROLSETTINGS_CHANNELGROUPS_YAW])
                                && validInputRange(settings.ChannelMin.Pitch,
                                                   settings.ChannelMax.Pitch, cmd.Channel[MANUALCONTROLSETTINGS_CHANNELGROUPS_PITCH]);

        if (settings.ChannelGroups.Collective != MANUALCONTROLSETTINGS_CHANNELGROUPS_NONE) {
            valid_input_detected &= validInputRange(settings.ChannelMin.Collective,
                                                    settings.ChannelMax.Collective, cmd.Channel[MANUALCONTROLSETTINGS_CHANNELGROUPS_COLLECTIVE]);
        }
        if (settings.ChannelGroups.Accessory0 != MANUALCONTROLSETTINGS_CHANNELGROUPS_NONE) {
            valid_input_detected &= validInputRange(settings.ChannelMin.Accessory0,
                                                    settings.ChannelMax.Accessory0, cmd.Channel[MANUALCONTROLSETTINGS_CHANNELGROUPS_ACCESSORY0]);
        }
        if (settings.ChannelGroups.Accessory1 != MANUALCONTROLSETTINGS_CHANNELGROUPS_NONE) {
            valid_input_detected &= validInputRange(settings.ChannelMin.Accessory1,
                                                    settings.ChannelMax.Accessory1, cmd.Channel[MANUALCONTROLSETTINGS_CHANNELGROUPS_ACCESSORY1]);
        }
        if (settings.ChannelGroups.Accessory2 != MANUALCONTROLSETTINGS_CHANNELGROUPS_NONE) {
            valid_input_detected &= validInputRange(settings.ChannelMin.Accessory2,
                                                    settings.ChannelMax.Accessory2, cmd.Channel[MANUALCONTROLSETTINGS_CHANNELGROUPS_ACCESSORY2]);
        }

        // Implement hysteresis loop on connection status
        if (valid_input_detected && (++connected_count > 10)) {
            cmd.Connected      = MANUALCONTROLCOMMAND_CONNECTED_TRUE;
            connected_count    = 0;
            disconnected_count = 0;
        } else if (!valid_input_detected && (++disconnected_count > 10)) {
            cmd.Connected      = MANUALCONTROLCOMMAND_CONNECTED_FALSE;
            connected_count    = 0;
            disconnected_count = 0;
        }

        if (cmd.Connected == MANUALCONTROLCOMMAND_CONNECTED_FALSE) {
            cmd.Throttle   = settings.FailsafeChannel.Throttle;
            cmd.Roll       = settings.FailsafeChannel.Roll;
            cmd.Pitch      = settings.FailsafeChannel.Pitch;
            cmd.Yaw = settings.FailsafeChannel.Yaw;
            cmd.Collective = settings.FailsafeChannel.Collective;
            switch (thrustType) {
            case SYSTEMSETTINGS_THRUSTCONTROL_THROTTLE:
                cmd.Thrust = cmd.Throttle;
                break;
            case SYSTEMSETTINGS_THRUSTCONTROL_COLLECTIVE:
                cmd.Thrust = cmd.Collective;
                break;
            default:
                break;
            }
            if (settings.FailsafeFlightModeSwitchPosition >= 0 && settings.FailsafeFlightModeSwitchPosition < settings.FlightModeNumber) {
                cmd.FlightModeSwitchPosition = (uint8_t)settings.FailsafeFlightModeSwitchPosition;
            }
            AlarmsSet(SYSTEMALARMS_ALARM_RECEIVER, SYSTEMALARMS_ALARM_WARNING);

            AccessoryDesiredData accessory;
            // Set Accessory 0
            if (settings.ChannelGroups.Accessory0 != MANUALCONTROLSETTINGS_CHANNELGROUPS_NONE) {
                accessory.AccessoryVal = settings.FailsafeChannel.Accessory0;
                if (AccessoryDesiredInstSet(0, &accessory) != 0) {
                    AlarmsSet(SYSTEMALARMS_ALARM_RECEIVER, SYSTEMALARMS_ALARM_WARNING);
                }
            }
            // Set Accessory 1
            if (settings.ChannelGroups.Accessory1 != MANUALCONTROLSETTINGS_CHANNELGROUPS_NONE) {
                accessory.AccessoryVal = settings.FailsafeChannel.Accessory1;
                if (AccessoryDesiredInstSet(1, &accessory) != 0) {
                    AlarmsSet(SYSTEMALARMS_ALARM_RECEIVER, SYSTEMALARMS_ALARM_WARNING);
                }
            }
            // Set Accessory 2
            if (settings.ChannelGroups.Accessory2 != MANUALCONTROLSETTINGS_CHANNELGROUPS_NONE) {
                accessory.AccessoryVal = settings.FailsafeChannel.Accessory2;
                if (AccessoryDesiredInstSet(2, &accessory) != 0) {
                    AlarmsSet(SYSTEMALARMS_ALARM_RECEIVER, SYSTEMALARMS_ALARM_WARNING);
                }
            }
        } else if (valid_input_detected) {
            AlarmsClear(SYSTEMALARMS_ALARM_RECEIVER);

            // Scale channels to -1 -> +1 range
            cmd.Roll     = scaledChannel[MANUALCONTROLSETTINGS_CHANNELGROUPS_ROLL];
            cmd.Pitch    = scaledChannel[MANUALCONTROLSETTINGS_CHANNELGROUPS_PITCH];
            cmd.Yaw      = scaledChannel[MANUALCONTROLSETTINGS_CHANNELGROUPS_YAW];
            cmd.Throttle = scaledChannel[MANUALCONTROLSETTINGS_CHANNELGROUPS_THROTTLE];
            // Convert flightMode value into the switch position in the range [0..N-1]
            cmd.FlightModeSwitchPosition = ((int16_t)(scaledChannel[MANUALCONTROLSETTINGS_CHANNELGROUPS_FLIGHTMODE] * 256.0f) + 256) * settings.FlightModeNumber >> 9;
            if (cmd.FlightModeSwitchPosition >= settings.FlightModeNumber) {
                cmd.FlightModeSwitchPosition = settings.FlightModeNumber - 1;
            }

            // Apply deadband for Roll/Pitch/Yaw stick inputs
            if (settings.Deadband > 0.0f) {
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
            if (cmd.Channel[MANUALCONTROLSETTINGS_CHANNELGROUPS_COLLECTIVE] != (uint16_t)PIOS_RCVR_INVALID
                && cmd.Channel[MANUALCONTROLSETTINGS_CHANNELGROUPS_COLLECTIVE] != (uint16_t)PIOS_RCVR_NODRIVER
                && cmd.Channel[MANUALCONTROLSETTINGS_CHANNELGROUPS_COLLECTIVE] != (uint16_t)PIOS_RCVR_TIMEOUT) {
                cmd.Collective = scaledChannel[MANUALCONTROLSETTINGS_CHANNELGROUPS_COLLECTIVE];
                if (settings.Deadband > 0.0f) {
                    applyDeadband(&cmd.Collective, settings.Deadband);
                }
#ifdef USE_INPUT_LPF
                applyLPF(&cmd.Collective, MANUALCONTROLSETTINGS_RESPONSETIME_COLLECTIVE, &settings, dT);
#endif // USE_INPUT_LPF
            }

            switch (thrustType) {
            case SYSTEMSETTINGS_THRUSTCONTROL_THROTTLE:
                cmd.Thrust = cmd.Throttle;
                break;
            case SYSTEMSETTINGS_THRUSTCONTROL_COLLECTIVE:
                cmd.Thrust = cmd.Collective;
                break;
            default:
                break;
            }

            AccessoryDesiredData accessory;
            // Set Accessory 0
            if (settings.ChannelGroups.Accessory0 != MANUALCONTROLSETTINGS_CHANNELGROUPS_NONE) {
                accessory.AccessoryVal = scaledChannel[MANUALCONTROLSETTINGS_CHANNELGROUPS_ACCESSORY0];
#ifdef USE_INPUT_LPF
                applyLPF(&accessory.AccessoryVal, MANUALCONTROLSETTINGS_RESPONSETIME_ACCESSORY0, &settings, dT);
#endif
                if (AccessoryDesiredInstSet(0, &accessory) != 0) {
                    AlarmsSet(SYSTEMALARMS_ALARM_RECEIVER, SYSTEMALARMS_ALARM_WARNING);
                }
            }
            // Set Accessory 1
            if (settings.ChannelGroups.Accessory1 != MANUALCONTROLSETTINGS_CHANNELGROUPS_NONE) {
                accessory.AccessoryVal = scaledChannel[MANUALCONTROLSETTINGS_CHANNELGROUPS_ACCESSORY1];
#ifdef USE_INPUT_LPF
                applyLPF(&accessory.AccessoryVal, MANUALCONTROLSETTINGS_RESPONSETIME_ACCESSORY1, &settings, dT);
#endif
                if (AccessoryDesiredInstSet(1, &accessory) != 0) {
                    AlarmsSet(SYSTEMALARMS_ALARM_RECEIVER, SYSTEMALARMS_ALARM_WARNING);
                }
            }
            // Set Accessory 2
            if (settings.ChannelGroups.Accessory2 != MANUALCONTROLSETTINGS_CHANNELGROUPS_NONE) {
                accessory.AccessoryVal = scaledChannel[MANUALCONTROLSETTINGS_CHANNELGROUPS_ACCESSORY2];
#ifdef USE_INPUT_LPF
                applyLPF(&accessory.AccessoryVal, MANUALCONTROLSETTINGS_RESPONSETIME_ACCESSORY2, &settings, dT);
#endif

                if (AccessoryDesiredInstSet(2, &accessory) != 0) {
                    AlarmsSet(SYSTEMALARMS_ALARM_RECEIVER, SYSTEMALARMS_ALARM_WARNING);
                }
            }
        }

        // Update cmd object
        ManualControlCommandSet(&cmd);
#if defined(PIOS_INCLUDE_USB_RCTX)
        if (pios_usb_rctx_id) {
            PIOS_USB_RCTX_Update(pios_usb_rctx_id,
                                 cmd.Channel,
                                 cast_struct_to_array(settings.ChannelMin, settings.ChannelMin.Roll),
                                 cast_struct_to_array(settings.ChannelMax, settings.ChannelMax.Roll),
                                 NELEMENTS(cmd.Channel));
        }
#endif /* PIOS_INCLUDE_USB_RCTX */
    }
}

static void resetRcvrActivity(struct rcvr_activity_fsm *fsm)
{
    ReceiverActivityData data;
    bool updated = false;

    /* Clear all channel activity flags */
    ReceiverActivityGet(&data);
    if (data.ActiveGroup != RECEIVERACTIVITY_ACTIVEGROUP_NONE && data.ActiveChannel != 255) {
        data.ActiveGroup   = RECEIVERACTIVITY_ACTIVEGROUP_NONE;
        data.ActiveChannel = 255;
        updated = true;
    }
    if (updated) {
        ReceiverActivitySet(&data);
    }

    /* Reset the FSM state */
    fsm->group = 0;
    fsm->sample_count = 0;
}

static void updateRcvrActivitySample(uint32_t rcvr_id, uint16_t samples[], uint8_t max_channels)
{
    for (uint8_t channel = 1; channel <= max_channels; channel++) {
        // Subtract 1 because channels are 1 indexed
        samples[channel - 1] = PIOS_RCVR_Read(rcvr_id, channel);
    }
}

static bool updateRcvrActivityCompare(uint32_t rcvr_id, struct rcvr_activity_fsm *fsm)
{
    bool activity_updated = false;

    /* Compare the current value to the previous sampled value */
    for (uint8_t channel = 1; channel <= RCVR_ACTIVITY_MONITOR_CHANNELS_PER_GROUP; channel++) {
        uint16_t delta;
        uint16_t prev = fsm->prev[channel - 1]; // Subtract 1 because channels are 1 indexed
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

            ReceiverActivityActiveGroupSet((uint8_t *)&group);
            ReceiverActivityActiveChannelSet(&channel);
            activity_updated = true;
        }
    }
    return activity_updated;
}

static bool updateRcvrActivity(struct rcvr_activity_fsm *fsm)
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
        updateRcvrActivitySample(pios_rcvr_group_map[fsm->group], fsm->prev, NELEMENTS(fsm->prev));
        fsm->sample_count++;
        return false;
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
            updateRcvrActivitySample(pios_rcvr_group_map[fsm->group], fsm->prev, NELEMENTS(fsm->prev));
            fsm->sample_count++;
            break;
        }
    }

    return activity_updated;
}

/**
 * Convert channel from servo pulse duration (microseconds) to scaled -1/+1 range.
 */
static float scaleChannel(int16_t value, int16_t max, int16_t min, int16_t neutral)
{
    float valueScaled;

    // Scale
    if ((max > min && value >= neutral) || (min > max && value <= neutral)) {
        if (max != neutral) {
            valueScaled = (float)(value - neutral) / (float)(max - neutral);
        } else {
            valueScaled = 0;
        }
    } else {
        if (min != neutral) {
            valueScaled = (float)(value - neutral) / (float)(neutral - min);
        } else {
            valueScaled = 0;
        }
    }

    // Bound
    if (valueScaled > 1.0f) {
        valueScaled = 1.0f;
    } else if (valueScaled < -1.0f) {
        valueScaled = -1.0f;
    }

    return valueScaled;
}

static uint32_t timeDifferenceMs(portTickType start_time, portTickType end_time)
{
    return (end_time - start_time) * portTICK_RATE_MS;
}


/**
 * @brief Determine if the manual input value is within acceptable limits
 * @returns return TRUE if so, otherwise return FALSE
 */
bool validInputRange(int16_t min, int16_t max, uint16_t value)
{
    if (min > max) {
        int16_t tmp = min;
        min = max;
        max = tmp;
    }
    return value >= min - CONNECTION_OFFSET && value <= max + CONNECTION_OFFSET;
}

/**
 * @brief Apply deadband to Roll/Pitch/Yaw channels
 */
static void applyDeadband(float *value, float deadband)
{
    if (fabsf(*value) < deadband) {
        *value = 0.0f;
    } else if (*value > 0.0f) {
        *value -= deadband;
    } else {
        *value += deadband;
    }
}

#ifdef USE_INPUT_LPF
/**
 * @brief Apply Low Pass Filter to Throttle/Roll/Pitch/Yaw or Accessory channel
 */
static void applyLPF(float *value, ManualControlSettingsResponseTimeElem channel, ManualControlSettingsData *settings, float dT)
{
    if (cast_struct_to_array(settings->ResponseTime, settings->ResponseTime.Roll)[channel]) {
        float rt = (float)cast_struct_to_array(settings->ResponseTime, settings->ResponseTime.Roll)[channel];
        inputFiltered[channel] = ((rt * inputFiltered[channel]) + (dT * (*value))) / (rt + dT);
        *value = inputFiltered[channel];
    }
}
#endif // USE_INPUT_LPF

/**
 * @}
 * @}
 */

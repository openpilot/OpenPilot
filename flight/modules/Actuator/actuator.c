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


#include <openpilot.h>

#include "accessorydesired.h"
#include "actuator.h"
#include "actuatorsettings.h"
#include "systemsettings.h"
#include "actuatordesired.h"
#include "actuatorcommand.h"
#include "flightstatus.h"
#include <flightmodesettings.h>
#include "mixersettings.h"
#include "mixerstatus.h"
#include "cameradesired.h"
#include "manualcontrolcommand.h"
#include "taskinfo.h"
#include <systemsettings.h>
#include <sanitycheck.h>
#ifndef PIOS_EXCLUDE_ADVANCED_FEATURES
#include <vtolpathfollowersettings.h>
#endif
#undef PIOS_INCLUDE_INSTRUMENTATION
#ifdef PIOS_INCLUDE_INSTRUMENTATION
#include <pios_instrumentation.h>
static int8_t counter;
// Counter 0xAC700001 total Actuator body execution time(excluding queue waits etc).
#endif

// Private constants
#define MAX_QUEUE_SIZE                  2

#if defined(PIOS_ACTUATOR_STACK_SIZE)
#define STACK_SIZE_BYTES                PIOS_ACTUATOR_STACK_SIZE
#else
#define STACK_SIZE_BYTES                1312
#endif

#define TASK_PRIORITY                   (tskIDLE_PRIORITY + 4) // device driver
#define FAILSAFE_TIMEOUT_MS             100
#define MAX_MIX_ACTUATORS               ACTUATORCOMMAND_CHANNEL_NUMELEM

#define CAMERA_BOOT_DELAY_MS            7000

#define ACTUATOR_ONESHOT125_CLOCK       2000000
#define ACTUATOR_ONESHOT125_PULSE_SCALE 4
#define ACTUATOR_PWM_CLOCK              1000000
// Private types


// Private variables
static xQueueHandle queue;
static xTaskHandle taskHandle;
static FrameType_t frameType = FRAME_TYPE_MULTIROTOR;
static SystemSettingsThrustControlOptions thrustType = SYSTEMSETTINGS_THRUSTCONTROL_THROTTLE;

static float lastResult[MAX_MIX_ACTUATORS] = { 0 };
static float filterAccumulator[MAX_MIX_ACTUATORS] = { 0 };
static uint8_t pinsMode[MAX_MIX_ACTUATORS];
// used to inform the actuator thread that actuator update rate is changed
static ActuatorSettingsData actuatorSettings;
static bool spinWhileArmed;

// used to inform the actuator thread that mixer settings are changed
static MixerSettingsData mixerSettings;
static int mixer_settings_count = 2;

// Private functions
static void actuatorTask(void *parameters);
static int16_t scaleChannel(float value, int16_t max, int16_t min, int16_t neutral);
static int16_t scaleMotor(float value, int16_t max, int16_t min, int16_t neutral, float maxMotor, float minMotor, bool armed, bool AlwaysStabilizeWhenArmed, float throttleDesired);
static void setFailsafe();
static float MixerCurveFullRangeProportional(const float input, const float *curve, uint8_t elements, bool multirotor);
static float MixerCurveFullRangeAbsolute(const float input, const float *curve, uint8_t elements, bool multirotor);
static bool set_channel(uint8_t mixer_channel, uint16_t value);
static void actuator_update_rate_if_changed(bool force_update);
static void MixerSettingsUpdatedCb(UAVObjEvent *ev);
static void ActuatorSettingsUpdatedCb(UAVObjEvent *ev);
static void SettingsUpdatedCb(UAVObjEvent *ev);
float ProcessMixer(const int index, const float curve1, const float curve2,
                   ActuatorDesiredData *desired,
                   const float period, bool multirotor);

// this structure is equivalent to the UAVObjects for one mixer.
typedef struct {
    uint8_t type;
    int8_t  matrix[5];
} __attribute__((packed)) Mixer_t;

/**
 * @brief Module initialization
 * @return 0
 */
int32_t ActuatorStart()
{
    // Start main task
    xTaskCreate(actuatorTask, "Actuator", STACK_SIZE_BYTES / 4, NULL, TASK_PRIORITY, &taskHandle);
    PIOS_TASK_MONITOR_RegisterTask(TASKINFO_RUNNING_ACTUATOR, taskHandle);
#ifdef PIOS_INCLUDE_WDG
    PIOS_WDG_RegisterFlag(PIOS_WDG_ACTUATOR);
#endif
    SettingsUpdatedCb(NULL);
    MixerSettingsUpdatedCb(NULL);
    ActuatorSettingsUpdatedCb(NULL);
    return 0;
}

/**
 * @brief Module initialization
 * @return 0
 */
int32_t ActuatorInitialize()
{
    // Register for notification of changes to ActuatorSettings
    ActuatorSettingsInitialize();
    ActuatorSettingsConnectCallback(ActuatorSettingsUpdatedCb);

    // Register for notification of changes to MixerSettings
    MixerSettingsInitialize();
    MixerSettingsConnectCallback(MixerSettingsUpdatedCb);

    // Listen for ActuatorDesired updates (Primary input to this module)
    ActuatorDesiredInitialize();
    queue = xQueueCreate(MAX_QUEUE_SIZE, sizeof(UAVObjEvent));
    ActuatorDesiredConnectQueue(queue);

    // Register AccessoryDesired (Secondary input to this module)
    AccessoryDesiredInitialize();

    // Primary output of this module
    ActuatorCommandInitialize();

#ifdef DIAG_MIXERSTATUS
    // UAVO only used for inspecting the internal status of the mixer during debug
    MixerStatusInitialize();
#endif

#ifndef PIOS_EXCLUDE_ADVANCED_FEATURES
    VtolPathFollowerSettingsInitialize();
    VtolPathFollowerSettingsConnectCallback(&SettingsUpdatedCb);
#endif
    SystemSettingsInitialize();
    SystemSettingsConnectCallback(&SettingsUpdatedCb);

    return 0;
}
MODULE_INITCALL(ActuatorInitialize, ActuatorStart);

/**
 * @brief Main Actuator module task
 *
 * Universal matrix based mixer for VTOL, helis and fixed wing.
 * Converts desired roll,pitch,yaw and throttle to servo/ESC outputs.
 *
 * Because of how the Throttle ranges from 0 to 1, the motors should too!
 *
 * Note this code depends on the UAVObjects for the mixers being all being the same
 * and in sequence. If you change the object definition, make sure you check the code!
 *
 * @return -1 if error, 0 if success
 */
static void actuatorTask(__attribute__((unused)) void *parameters)
{
    UAVObjEvent ev;
    portTickType lastSysTime;
    portTickType thisSysTime;
    float dTSeconds;
    uint32_t dTMilliseconds;

    ActuatorCommandData command;
    ActuatorDesiredData desired;
    MixerStatusData mixerStatus;
    FlightModeSettingsData settings;
    FlightStatusData flightStatus;
    float throttleDesired;
    float collectiveDesired;

#ifdef PIOS_INCLUDE_INSTRUMENTATION
    counter = PIOS_Instrumentation_CreateCounter(0xAC700001);
#endif
    /* Read initial values of ActuatorSettings */

    ActuatorSettingsGet(&actuatorSettings);

    /* Read initial values of MixerSettings */
    MixerSettingsGet(&mixerSettings);

    /* Force an initial configuration of the actuator update rates */
    actuator_update_rate_if_changed(true);

    // Go to the neutral (failsafe) values until an ActuatorDesired update is received
    setFailsafe();

    // Main task loop
    lastSysTime = xTaskGetTickCount();
    while (1) {
#ifdef PIOS_INCLUDE_WDG
        PIOS_WDG_UpdateFlag(PIOS_WDG_ACTUATOR);
#endif

        // Wait until the ActuatorDesired object is updated
        uint8_t rc = xQueueReceive(queue, &ev, FAILSAFE_TIMEOUT_MS / portTICK_RATE_MS);
#ifdef PIOS_INCLUDE_INSTRUMENTATION
        PIOS_Instrumentation_TimeStart(counter);
#endif

        if (rc != pdTRUE) {
            /* Update of ActuatorDesired timed out.  Go to failsafe */
            setFailsafe();
            continue;
        }

        // Check how long since last update
        thisSysTime    = xTaskGetTickCount();
        dTMilliseconds = (thisSysTime == lastSysTime) ? 1 : (thisSysTime - lastSysTime) * portTICK_RATE_MS;
        lastSysTime    = thisSysTime;
        dTSeconds = dTMilliseconds * 0.001f;

        FlightStatusGet(&flightStatus);
        FlightModeSettingsGet(&settings);
        ActuatorDesiredGet(&desired);
        ActuatorCommandGet(&command);

        // read in throttle and collective -demultiplex thrust
        switch (thrustType) {
        case SYSTEMSETTINGS_THRUSTCONTROL_THROTTLE:
            throttleDesired = desired.Thrust;
            ManualControlCommandCollectiveGet(&collectiveDesired);
            break;
        case SYSTEMSETTINGS_THRUSTCONTROL_COLLECTIVE:
            ManualControlCommandThrottleGet(&throttleDesired);
            collectiveDesired = desired.Thrust;
            break;
        default:
            ManualControlCommandThrottleGet(&throttleDesired);
            ManualControlCommandCollectiveGet(&collectiveDesired);
        }

        bool armed = flightStatus.Armed == FLIGHTSTATUS_ARMED_ARMED;
        bool activeThrottle   = (throttleDesired < -0.001f || throttleDesired > 0.001f); // for ground and reversible motors
        bool positiveThrottle = (throttleDesired > 0.00f);
        bool multirotor  = (GetCurrentFrameType() == FRAME_TYPE_MULTIROTOR); // check if frame is a multirotor.
        bool alwaysArmed = settings.Arming == FLIGHTMODESETTINGS_ARMING_ALWAYSARMED;
        bool AlwaysStabilizeWhenArmed = settings.AlwaysStabilizeWhenArmed == FLIGHTMODESETTINGS_ALWAYSSTABILIZEWHENARMED_TRUE;

        if (alwaysArmed) {
            AlwaysStabilizeWhenArmed = false; // Do not allow always stabilize when alwaysArmed is active. This is dangerous.
        }
        // safety settings
        if (!armed) {
            throttleDesired = 0.00f; // this also happens in scaleMotors as a per axis check
        }

        if ((frameType == FRAME_TYPE_GROUND && !activeThrottle) || (frameType != FRAME_TYPE_GROUND && throttleDesired <= 0.00f) || !armed) {
            // throttleDesired should never be 0 or go below 0.
            // force set all other controls to zero if throttle is cut (previously set in Stabilization)
            if (multirotor && AlwaysStabilizeWhenArmed && armed) { // we don't do this if this is a multirotor AND AlwaysStabilizeWhenArmed is true and the model is armed
                if (actuatorSettings.LowThrottleZeroAxis.Roll == ACTUATORSETTINGS_LOWTHROTTLEZEROAXIS_TRUE) {
                    desired.Roll = 0.00f;
                }
                if (actuatorSettings.LowThrottleZeroAxis.Pitch == ACTUATORSETTINGS_LOWTHROTTLEZEROAXIS_TRUE) {
                    desired.Pitch = 0.00f;
                }
                if (actuatorSettings.LowThrottleZeroAxis.Yaw == ACTUATORSETTINGS_LOWTHROTTLEZEROAXIS_TRUE) {
                    desired.Yaw = 0.00f;
                }
            }
        }

#ifdef DIAG_MIXERSTATUS
        MixerStatusGet(&mixerStatus);
#endif

        if ((mixer_settings_count < 2) && !ActuatorCommandReadOnly()) { // Nothing can fly with less than two mixers.
            setFailsafe();
            continue;
        }

        AlarmsClear(SYSTEMALARMS_ALARM_ACTUATOR);

        float curve1 = 0.0f; // curve 1 is the throttle curve applied to all motors.
        float curve2 = 0.0f;

        // Interpolate curve 1 from throttleDesired as input.
        // assume reversible motor/mixer initially. We can later reverse this. The difference is simply that -ve throttleDesired values
        // map differently
        curve1 = MixerCurveFullRangeProportional(throttleDesired, mixerSettings.ThrottleCurve1, MIXERSETTINGS_THROTTLECURVE1_NUMELEM, multirotor);

        // The source for the secondary curve is selectable
        AccessoryDesiredData accessory;
        uint8_t curve2Source = mixerSettings.Curve2Source;
        switch (curve2Source) {
        case MIXERSETTINGS_CURVE2SOURCE_THROTTLE:
            // assume reversible motor/mixer initially
            curve2 = MixerCurveFullRangeProportional(throttleDesired, mixerSettings.ThrottleCurve2, MIXERSETTINGS_THROTTLECURVE2_NUMELEM, multirotor);
            break;
        case MIXERSETTINGS_CURVE2SOURCE_ROLL:
            // Throttle curve contribution the same for +ve vs -ve roll
            if (multirotor) {
                curve2 = MixerCurveFullRangeProportional(desired.Roll, mixerSettings.ThrottleCurve2, MIXERSETTINGS_THROTTLECURVE2_NUMELEM, multirotor);
            } else {
                curve2 = MixerCurveFullRangeAbsolute(desired.Roll, mixerSettings.ThrottleCurve2, MIXERSETTINGS_THROTTLECURVE2_NUMELEM, multirotor);
            }
            break;
        case MIXERSETTINGS_CURVE2SOURCE_PITCH:
            // Throttle curve contribution the same for +ve vs -ve pitch
            if (multirotor) {
                curve2 = MixerCurveFullRangeProportional(desired.Pitch, mixerSettings.ThrottleCurve2,
                                                         MIXERSETTINGS_THROTTLECURVE2_NUMELEM, multirotor);
            } else {
                curve2 = MixerCurveFullRangeAbsolute(desired.Pitch, mixerSettings.ThrottleCurve2,
                                                     MIXERSETTINGS_THROTTLECURVE2_NUMELEM, multirotor);
            }
            break;
        case MIXERSETTINGS_CURVE2SOURCE_YAW:
            // Throttle curve contribution the same for +ve vs -ve yaw
            if (multirotor) {
                curve2 = MixerCurveFullRangeProportional(desired.Yaw, mixerSettings.ThrottleCurve2, MIXERSETTINGS_THROTTLECURVE2_NUMELEM, multirotor);
            } else {
                curve2 = MixerCurveFullRangeAbsolute(desired.Yaw, mixerSettings.ThrottleCurve2, MIXERSETTINGS_THROTTLECURVE2_NUMELEM, multirotor);
            }
            break;
        case MIXERSETTINGS_CURVE2SOURCE_COLLECTIVE:
            // assume reversible motor/mixer initially
            curve2 = MixerCurveFullRangeProportional(collectiveDesired, mixerSettings.ThrottleCurve2,
                                                     MIXERSETTINGS_THROTTLECURVE2_NUMELEM, multirotor);
            break;
        case MIXERSETTINGS_CURVE2SOURCE_ACCESSORY0:
        case MIXERSETTINGS_CURVE2SOURCE_ACCESSORY1:
        case MIXERSETTINGS_CURVE2SOURCE_ACCESSORY2:
        case MIXERSETTINGS_CURVE2SOURCE_ACCESSORY3:
        case MIXERSETTINGS_CURVE2SOURCE_ACCESSORY4:
        case MIXERSETTINGS_CURVE2SOURCE_ACCESSORY5:
            if (AccessoryDesiredInstGet(mixerSettings.Curve2Source - MIXERSETTINGS_CURVE2SOURCE_ACCESSORY0, &accessory) == 0) {
                // Throttle curve contribution the same for +ve vs -ve accessory....maybe not want we want.
                curve2 = MixerCurveFullRangeAbsolute(accessory.AccessoryVal, mixerSettings.ThrottleCurve2, MIXERSETTINGS_THROTTLECURVE2_NUMELEM, multirotor);
            } else {
                curve2 = 0.0f;
            }
            break;
        default:
            curve2 = 0.0f;
            break;
        }

        float *status   = (float *)&mixerStatus; // access status objects as an array of floats
        Mixer_t *mixers = (Mixer_t *)&mixerSettings.Mixer1Type;
        float maxMotor  = -1.0f; // highest motor value. Addition method needs this to be -1.0f, division method needs this to be 1.0f
        float minMotor  = 1.0f; // lowest motor value Addition method needs this to be 1.0f, division method needs this to be -1.0f

        for (int ct = 0; ct < MAX_MIX_ACTUATORS; ct++) {
            // During boot all camera actuators should be completely disabled (PWM pulse = 0).
            // command.Channel[i] is reused below as a channel PWM activity flag:
            // 0 - PWM disabled, >0 - PWM set to real mixer value using scaleChannel() later.
            // Setting it to 1 by default means "Rescale this channel and enable PWM on its output".
            command.Channel[ct] = 1;

            uint8_t mixer_type = mixers[ct].type;

            if (mixer_type == MIXERSETTINGS_MIXER1TYPE_DISABLED) {
                // Set to minimum if disabled.  This is not the same as saying PWM pulse = 0 us
                status[ct] = -1;
                continue;
            }

            if ((mixer_type == MIXERSETTINGS_MIXER1TYPE_MOTOR)) {
                float nonreversible_curve1 = curve1;
                float nonreversible_curve2 = curve2;
                if (nonreversible_curve1 < 0.0f) {
                    nonreversible_curve1 = 0.0f;
                }
                if (nonreversible_curve2 < 0.0f) {
                    nonreversible_curve2 = 0.0f;
                }
                status[ct] = ProcessMixer(ct, nonreversible_curve1, nonreversible_curve2, &desired, dTSeconds, multirotor);
                // If not armed or motors aren't meant to spin all the time
                if (!armed ||
                    (!spinWhileArmed && !positiveThrottle)) {
                    filterAccumulator[ct] = 0;
                    lastResult[ct] = 0;
                    status[ct] = -1; // force min throttle
                }
                // If armed meant to keep spinning,
                else if ((spinWhileArmed && !positiveThrottle) ||
                         (status[ct] < 0)) {
                    if (!multirotor) {
                        status[ct] = 0;
                        // allow throttle values lower than 0 if multirotor.
                        // Values will be scaled to 0 if they need to be in the scaleMotor function
                    }
                }
            } else if (mixer_type == MIXERSETTINGS_MIXER1TYPE_REVERSABLEMOTOR) {
                status[ct] = ProcessMixer(ct, curve1, curve2, &desired, dTSeconds, multirotor);
                // Reversable Motors are like Motors but go to neutral instead of minimum
                // If not armed or motor is inactive - no "spinwhilearmed" for this engine type
                if (!armed || !activeThrottle) {
                    filterAccumulator[ct] = 0;
                    lastResult[ct] = 0;
                    status[ct] = 0; // force neutral throttle
                }
            } else if (mixer_type == MIXERSETTINGS_MIXER1TYPE_SERVO) {
                status[ct] = ProcessMixer(ct, curve1, curve2, &desired, dTSeconds, multirotor);
            } else {
                status[ct] = -1;

                // If an accessory channel is selected for direct bypass mode
                // In this configuration the accessory channel is scaled and mapped
                // directly to output.  Note: THERE IS NO SAFETY CHECK HERE FOR ARMING
                // these also will not be updated in failsafe mode.  I'm not sure what
                // the correct behavior is since it seems domain specific.  I don't love
                // this code
                if ((mixer_type >= MIXERSETTINGS_MIXER1TYPE_ACCESSORY0) &&
                    (mixer_type <= MIXERSETTINGS_MIXER1TYPE_ACCESSORY5)) {
                    if (AccessoryDesiredInstGet(mixer_type - MIXERSETTINGS_MIXER1TYPE_ACCESSORY0, &accessory) == 0) {
                        status[ct] = accessory.AccessoryVal;
                    } else {
                        status[ct] = -1;
                    }
                }

                if ((mixer_type >= MIXERSETTINGS_MIXER1TYPE_CAMERAROLLORSERVO1) &&
                    (mixer_type <= MIXERSETTINGS_MIXER1TYPE_CAMERAYAW)) {
                    CameraDesiredData cameraDesired;
                    if (CameraDesiredGet(&cameraDesired) == 0) {
                        switch (mixer_type) {
                        case MIXERSETTINGS_MIXER1TYPE_CAMERAROLLORSERVO1:
                            status[ct] = cameraDesired.RollOrServo1;
                            break;
                        case MIXERSETTINGS_MIXER1TYPE_CAMERAPITCHORSERVO2:
                            status[ct] = cameraDesired.PitchOrServo2;
                            break;
                        case MIXERSETTINGS_MIXER1TYPE_CAMERAYAW:
                            status[ct] = cameraDesired.Yaw;
                            break;
                        default:
                            break;
                        }
                    } else {
                        status[ct] = -1;
                    }

                    // Disable camera actuators for CAMERA_BOOT_DELAY_MS after boot
                    if (thisSysTime < (CAMERA_BOOT_DELAY_MS / portTICK_RATE_MS)) {
                        command.Channel[ct] = 0;
                    }
                }
            }

            // If mixer type is motor we need to find which motor has the highest value and which motor has the lowest value.
            // For use in function scaleMotor
            if (mixers[ct].type == MIXERSETTINGS_MIXER1TYPE_MOTOR) {
                if (maxMotor < status[ct]) {
                    maxMotor = status[ct];
                }
                if (minMotor > status[ct]) {
                    minMotor = status[ct];
                }
            }
        }

        // Set real actuator output values scaling them from mixers. All channels
        // will be set except explicitly disabled (which will have PWM pulse = 0).
        for (int i = 0; i < MAX_MIX_ACTUATORS; i++) {
            if (command.Channel[i]) {
                if (mixers[i].type == MIXERSETTINGS_MIXER1TYPE_MOTOR) { // If mixer is for a motor we need to find the highest value of all motors
                    command.Channel[i] = scaleMotor(status[i],
                                                    actuatorSettings.ChannelMax[i],
                                                    actuatorSettings.ChannelMin[i],
                                                    actuatorSettings.ChannelNeutral[i],
                                                    maxMotor,
                                                    minMotor,
                                                    armed,
                                                    AlwaysStabilizeWhenArmed,
                                                    throttleDesired);
                } else { // else we scale the channel
                    command.Channel[i] = scaleChannel(status[i],
                                                      actuatorSettings.ChannelMax[i],
                                                      actuatorSettings.ChannelMin[i],
                                                      actuatorSettings.ChannelNeutral[i]);
                }
            }
        }

        // Store update time
        command.UpdateTime = dTMilliseconds;
        if (command.UpdateTime > command.MaxUpdateTime) {
            command.MaxUpdateTime = command.UpdateTime;
        }
        // Update output object
        ActuatorCommandSet(&command);
        // Update in case read only (eg. during servo configuration)
        ActuatorCommandGet(&command);

#ifdef DIAG_MIXERSTATUS
        MixerStatusSet(&mixerStatus);
#endif


        // Update servo outputs
        bool success = true;

        for (int n = 0; n < ACTUATORCOMMAND_CHANNEL_NUMELEM; ++n) {
            success &= set_channel(n, command.Channel[n]);
        }

        PIOS_Servo_Update();

        if (!success) {
            command.NumFailedUpdates++;
            ActuatorCommandSet(&command);
            AlarmsSet(SYSTEMALARMS_ALARM_ACTUATOR, SYSTEMALARMS_ALARM_CRITICAL);
        }
#ifdef PIOS_INCLUDE_INSTRUMENTATION
        PIOS_Instrumentation_TimeEnd(counter);
#endif
    }
}


/**
 * Process mixing for one actuator
 */
float ProcessMixer(const int index, const float curve1, const float curve2,
                   ActuatorDesiredData *desired, const float period, bool multirotor)
{
    static float lastFilteredResult[MAX_MIX_ACTUATORS];
    const Mixer_t *mixers = (Mixer_t *)&mixerSettings.Mixer1Type; // pointer to array of mixers in UAVObjects
    const Mixer_t *mixer  = &mixers[index];

    float result = ((((float)mixer->matrix[MIXERSETTINGS_MIXER1VECTOR_THROTTLECURVE1]) * curve1) +
                    (((float)mixer->matrix[MIXERSETTINGS_MIXER1VECTOR_THROTTLECURVE2]) * curve2) +
                    (((float)mixer->matrix[MIXERSETTINGS_MIXER1VECTOR_ROLL]) * desired->Roll) +
                    (((float)mixer->matrix[MIXERSETTINGS_MIXER1VECTOR_PITCH]) * desired->Pitch) +
                    (((float)mixer->matrix[MIXERSETTINGS_MIXER1VECTOR_YAW]) * desired->Yaw)) / 128.0f;

    // note: no feedforward for reversable motors yet for safety reasons
    if (mixer->type == MIXERSETTINGS_MIXER1TYPE_MOTOR) {
        if (!multirotor) { // we allow negative throttle with a multirotor
            if (result < 0.0f) { // zero throttle
                result = 0.0f;
            }
        }

        // feed forward
        float accumulator = filterAccumulator[index];
        accumulator += (result - lastResult[index]) * mixerSettings.FeedForward;
        lastResult[index] = result;
        result += accumulator;
        if (period > 0.0f) {
            if (accumulator > 0.0f) {
                float invFilter = period / mixerSettings.AccelTime;
                if (invFilter > 1) {
                    invFilter = 1;
                }
                accumulator -= accumulator * invFilter;
            } else {
                float invFilter = period / mixerSettings.DecelTime;
                if (invFilter > 1) {
                    invFilter = 1;
                }
                accumulator -= accumulator * invFilter;
            }
        }
        filterAccumulator[index] = accumulator;
        result += accumulator;

        // acceleration limit
        float dt    = result - lastFilteredResult[index];
        float maxDt = mixerSettings.MaxAccel * period;
        if (dt > maxDt) { // we are accelerating too hard
            result = lastFilteredResult[index] + maxDt;
        }
        lastFilteredResult[index] = result;
    }

    return result;
}


/**
 * Interpolate a throttle curve
 * Full range input (-1 to 1) for yaw, roll, pitch
 * Output range (-1 to 1) reversible motor/throttle curve
 *
 * Input of -1 -> -lookup(1)
 * Input of 0  ->  lookup(0)
 * Input of 1  ->  lookup(1)
 */
static float MixerCurveFullRangeProportional(const float input, const float *curve, uint8_t elements, bool multirotor)
{
    float unsigned_value = MixerCurveFullRangeAbsolute(input, curve, elements, multirotor);

    if (input < 0.0f) {
        return -unsigned_value;
    } else {
        return unsigned_value;
    }
}

/**
 * Interpolate a throttle curve
 * Full range input (-1 to 1) for yaw, roll, pitch
 * Output range (0 to 1) non-reversible motor/throttle curve
 *
 * Input of -1 -> lookup(1)
 * Input of 0  -> lookup(0)
 * Input of 1  -> lookup(1)
 */
static float MixerCurveFullRangeAbsolute(const float input, const float *curve, uint8_t elements, bool multirotor)
{
    float abs_input = fabsf(input);
    float scale     = abs_input * (float)(elements - 1);
    int idx1 = scale;

    scale -= (float)idx1; // remainder
    if (curve[0] < -1) {
        return abs_input;
    }
    int idx2 = idx1 + 1;
    if (idx2 >= elements) {
        idx2 = elements - 1; // clamp to highest entry in table
        if (idx1 >= elements) {
            if (multirotor) {
                // if multirotor frame we can return throttle values higher than 100%.
                // Since the we don't have elements in the curve higher than 100% we return
                // the last element multiplied by the throttle float
                if (input < 2.0f) { // this limits positive throttle to 200% of max value in table (Maybe this is too much allowance)
                    return curve[idx2] * input;
                } else {
                    return curve[idx2] * 2.0f; // return 200% of max value in table
                }
            }
            idx1 = elements - 1;
        }
    }

    float unsigned_value = curve[idx1] * (1.0f - scale) + curve[idx2] * scale;
    return unsigned_value;
}


/**
 * Convert channel from -1/+1 to servo pulse duration in microseconds
 */
static int16_t scaleChannel(float value, int16_t max, int16_t min, int16_t neutral)
{
    int16_t valueScaled;

    // Scale
    if (value >= 0.0f) {
        valueScaled = (int16_t)(value * ((float)(max - neutral))) + neutral;
    } else {
        valueScaled = (int16_t)(value * ((float)(neutral - min))) + neutral;
    }

    if (max > min) {
        if (valueScaled > max) {
            valueScaled = max;
        }
        if (valueScaled < min) {
            valueScaled = min;
        }
    } else {
        if (valueScaled < max) {
            valueScaled = max;
        }
        if (valueScaled > min) {
            valueScaled = min;
        }
    }

    return valueScaled;
}

/**
 * Constrain motor values to keep any one motor value from going too far out of range of another motor
 */
static int16_t scaleMotor(float value, int16_t max, int16_t min, int16_t neutral, float maxMotor, float minMotor, bool armed, bool AlwaysStabilizeWhenArmed, float throttleDesired)
{
    int16_t valueScaled;
    int16_t maxMotorScaled;
    int16_t minMotorScaled;
    int16_t diff;

    // Scale
    if (value >= 0.0f) {
        valueScaled    = (int16_t)(value * ((float)(max - neutral))) + neutral;
        maxMotorScaled = (int16_t)(maxMotor * ((float)(max - neutral))) + neutral;
        minMotorScaled = (int16_t)(minMotor * ((float)(max - neutral))) + neutral;
    } else {
        valueScaled    = (int16_t)(value * ((float)(neutral - min))) + neutral;
        maxMotorScaled = (int16_t)(maxMotor * ((float)(neutral - min))) + neutral;
        minMotorScaled = (int16_t)(minMotor * ((float)(neutral - min))) + neutral;
    }

    if (max > min) {
        diff = max - maxMotorScaled; // difference between max allowed and actual max motor
        if (diff < 0) { // if the difference is smaller than 0 we add it to the scaled value
            valueScaled += diff;
            if (valueScaled > max) {
                valueScaled = max; // clamp to max value only after scaling is done.
            }
        }
        diff = min - minMotorScaled; // difference between min allowed and actual min motor
        if (diff > 0) { // if the difference is larger than 0 we add it to the scaled value
            valueScaled += diff;
            if (valueScaled < min) {
                valueScaled = min; // clamp to min value only after scaling is done.
            }
        }
    } else {
        // not sure what to do about reversed polarity right now. Why would anyone do this?
        if (valueScaled < max) {
            valueScaled = max; // clamp to max value only after scaling is done.
        }
        if (valueScaled > min) {
            valueScaled = min; // clamp to min value only after scaling is done.
        }
    }

    // I've added the bool AlwaysStabilizeWhenArmed to this function. Right now we command the motors at min or a range between neutral and max.
    // NEVER should a motor be command at between min and neutral. I don't like the idea of stabilization ever commanding a motor to min, but we give people the option
    // This prevents motors startup sync issues causing possible ESC failures.

    // safety checks
    if (!armed) {
        // if not armed return min EVERYTIME!
        valueScaled = min;
    } else if (!AlwaysStabilizeWhenArmed && (throttleDesired <= 0.0f) && spinWhileArmed) {
        // stabilize when armed?
        valueScaled = neutral;
    } else if (!spinWhileArmed && (throttleDesired <= 0.0f)) {
        // soft disarm
        valueScaled = min;
    }

    return valueScaled;
}

/**
 * Set actuator output to the neutral values (failsafe)
 */
static void setFailsafe()
{
    /* grab only the parts that we are going to use */
    int16_t Channel[ACTUATORCOMMAND_CHANNEL_NUMELEM];

    ActuatorCommandChannelGet(Channel);

    const Mixer_t *mixers = (Mixer_t *)&mixerSettings.Mixer1Type; // pointer to array of mixers in UAVObjects

    // Reset ActuatorCommand to safe values
    for (int n = 0; n < ACTUATORCOMMAND_CHANNEL_NUMELEM; ++n) {
        if (mixers[n].type == MIXERSETTINGS_MIXER1TYPE_MOTOR) {
            Channel[n] = actuatorSettings.ChannelMin[n];
        } else if (mixers[n].type == MIXERSETTINGS_MIXER1TYPE_SERVO || mixers[n].type == MIXERSETTINGS_MIXER1TYPE_REVERSABLEMOTOR) {
            // reversible motors need calibration wizard that allows channel neutral to be the 0 velocity point
            Channel[n] = actuatorSettings.ChannelNeutral[n];
        } else {
            Channel[n] = 0;
        }
    }

    // Set alarm
    AlarmsSet(SYSTEMALARMS_ALARM_ACTUATOR, SYSTEMALARMS_ALARM_CRITICAL);

    // Update servo outputs
    for (int n = 0; n < ACTUATORCOMMAND_CHANNEL_NUMELEM; ++n) {
        set_channel(n, Channel[n]);
    }
    // Send the updated command
    PIOS_Servo_Update();

    // Update output object's parts that we changed
    ActuatorCommandChannelSet(Channel);
}

/**
 * determine buzzer or blink sequence
 **/

typedef enum { BUZZ_BUZZER = 0, BUZZ_ARMING = 1, BUZZ_INFO = 2, BUZZ_MAX = 3 } buzzertype;

static inline bool buzzerState(buzzertype type)
{
    // This is for buzzers that take a PWM input

    static uint32_t tune[BUZZ_MAX] = { 0 };
    static uint32_t tunestate[BUZZ_MAX] = { 0 };


    uint32_t newTune = 0;

    if (type == BUZZ_BUZZER) {
        // Decide what tune to play
        if (AlarmsGet(SYSTEMALARMS_ALARM_BATTERY) > SYSTEMALARMS_ALARM_WARNING) {
            newTune = 0b11110110110000; // pause, short, short, short, long
        } else if (AlarmsGet(SYSTEMALARMS_ALARM_GPS) >= SYSTEMALARMS_ALARM_WARNING) {
            newTune = 0x80000000; // pause, short
        } else {
            newTune = 0;
        }
    } else { // BUZZ_ARMING || BUZZ_INFO
        uint8_t arming;
        FlightStatusArmedGet(&arming);
        // base idle tune
        newTune = 0x80000000; // 0b1000...

        // Merge the error pattern for InfoLed
        if (type == BUZZ_INFO) {
            if (AlarmsGet(SYSTEMALARMS_ALARM_BATTERY) > SYSTEMALARMS_ALARM_WARNING) {
                newTune |= 0b00000000001111111011111110000000;
            } else if (AlarmsGet(SYSTEMALARMS_ALARM_GPS) >= SYSTEMALARMS_ALARM_WARNING) {
                newTune |= 0b00000000000000110110110000000000;
            }
        }
        // fast double blink pattern if armed
        if (arming == FLIGHTSTATUS_ARMED_ARMED) {
            newTune |= 0xA0000000; // 0b101000...
        }
    }

    // Do we need to change tune?
    if (newTune != tune[type]) {
        tune[type] = newTune;
        // resynchronize all tunes on change, so they stay in sync
        for (int i = 0; i < BUZZ_MAX; i++) {
            tunestate[i] = tune[i];
        }
    }

    // Play tune
    bool buzzOn     = false;
    static portTickType lastSysTime = 0;
    portTickType thisSysTime = xTaskGetTickCount();
    portTickType dT = 0;

    // For now, only look at the battery alarm, because functions like AlarmsHasCritical() can block for some time; to be discussed
    if (tune[type]) {
        if (thisSysTime > lastSysTime) {
            dT = thisSysTime - lastSysTime;
        } else {
            lastSysTime = 0; // avoid the case where SysTimeMax-lastSysTime <80
        }

        buzzOn = (tunestate[type] & 1);

        if (dT > 80) {
            // Go to next bit in alarm_seq_state
            for (int i = 0; i < BUZZ_MAX; i++) {
                tunestate[i] >>= 1;
                if (tunestate[i] == 0) { // All done, re-start the tune
                    tunestate[i] = tune[i];
                }
            }
            lastSysTime = thisSysTime;
        }
    }
    return buzzOn;
}


#if defined(ARCH_POSIX) || defined(ARCH_WIN32)
static bool set_channel(uint8_t mixer_channel, uint16_t value)
{
    return true;
}
#else
static bool set_channel(uint8_t mixer_channel, uint16_t value)
{
    switch (actuatorSettings.ChannelType[mixer_channel]) {
    case ACTUATORSETTINGS_CHANNELTYPE_PWMALARMBUZZER:
        PIOS_Servo_Set(actuatorSettings.ChannelAddr[mixer_channel],
                       buzzerState(BUZZ_BUZZER) ? actuatorSettings.ChannelMax[mixer_channel] : actuatorSettings.ChannelMin[mixer_channel]);
        return true;

    case ACTUATORSETTINGS_CHANNELTYPE_ARMINGLED:
        PIOS_Servo_Set(actuatorSettings.ChannelAddr[mixer_channel],
                       buzzerState(BUZZ_ARMING) ? actuatorSettings.ChannelMax[mixer_channel] : actuatorSettings.ChannelMin[mixer_channel]);
        return true;

    case ACTUATORSETTINGS_CHANNELTYPE_INFOLED:
        PIOS_Servo_Set(actuatorSettings.ChannelAddr[mixer_channel],
                       buzzerState(BUZZ_INFO) ? actuatorSettings.ChannelMax[mixer_channel] : actuatorSettings.ChannelMin[mixer_channel]);
        return true;

    case ACTUATORSETTINGS_CHANNELTYPE_PWM:
    {
        uint8_t mode = pinsMode[actuatorSettings.ChannelAddr[mixer_channel]];
        switch (mode) {
        case ACTUATORSETTINGS_BANKMODE_ONESHOT125:
            // Remap 1000-2000 range to 125-250
            PIOS_Servo_Set(actuatorSettings.ChannelAddr[mixer_channel], value / ACTUATOR_ONESHOT125_PULSE_SCALE);
            break;
        default:
            PIOS_Servo_Set(actuatorSettings.ChannelAddr[mixer_channel], value);
            break;
        }
        return true;
    }

#if defined(PIOS_INCLUDE_I2C_ESC)
    case ACTUATORSETTINGS_CHANNELTYPE_MK:
        return PIOS_SetMKSpeed(actuatorSettings->ChannelAddr[mixer_channel], value);

    case ACTUATORSETTINGS_CHANNELTYPE_ASTEC4:
        return PIOS_SetAstec4Speed(actuatorSettings->ChannelAddr[mixer_channel], value);

#endif
    default:
        return false;
    }

    return false;
}
#endif /* if defined(ARCH_POSIX) || defined(ARCH_WIN32) */

/**
 * @brief Update the servo update rate
 */
static void actuator_update_rate_if_changed(bool force_update)
{
    static uint16_t prevBankUpdateFreq[ACTUATORSETTINGS_BANKUPDATEFREQ_NUMELEM];
    static uint8_t prevBankMode[ACTUATORSETTINGS_BANKMODE_NUMELEM];
    bool updateMode = force_update || (memcmp(prevBankMode, actuatorSettings.BankMode, sizeof(prevBankMode)) != 0);
    bool updateFreq = force_update || (memcmp(prevBankUpdateFreq, actuatorSettings.BankUpdateFreq, sizeof(prevBankUpdateFreq)) != 0);

    // check if any setting is changed
    if (updateMode || updateFreq) {
        /* Something has changed, apply the settings to HW */

        uint16_t freq[ACTUATORSETTINGS_BANKUPDATEFREQ_NUMELEM];
        uint32_t clock[ACTUATORSETTINGS_BANKUPDATEFREQ_NUMELEM] = { 0 };
        for (uint8_t i = 0; i < ACTUATORSETTINGS_BANKMODE_NUMELEM; i++) {
            if (force_update || (actuatorSettings.BankMode[i] != prevBankMode[i])) {
                PIOS_Servo_SetBankMode(i,
                                       actuatorSettings.BankMode[i] ==
                                       ACTUATORSETTINGS_BANKMODE_PWM ?
                                       PIOS_SERVO_BANK_MODE_PWM :
                                       PIOS_SERVO_BANK_MODE_SINGLE_PULSE
                                       );
            }
            switch (actuatorSettings.BankMode[i]) {
            case ACTUATORSETTINGS_BANKMODE_ONESHOT125:
                freq[i]  = 100; // Value must be small enough so CCr isn't update until the PIOS_Servo_Update is triggered
                clock[i] = ACTUATOR_ONESHOT125_CLOCK; // Setup an 2MHz timer clock
                break;
            case ACTUATORSETTINGS_BANKMODE_PWMSYNC:
                freq[i]  = 100;
                clock[i] = ACTUATOR_PWM_CLOCK;
                break;
            default: // PWM
                freq[i]  = actuatorSettings.BankUpdateFreq[i];
                clock[i] = ACTUATOR_PWM_CLOCK;
                break;
            }
        }

        memcpy(prevBankMode,
               actuatorSettings.BankMode,
               sizeof(prevBankMode));

        PIOS_Servo_SetHz(freq, clock, ACTUATORSETTINGS_BANKUPDATEFREQ_NUMELEM);

        memcpy(prevBankUpdateFreq,
               actuatorSettings.BankUpdateFreq,
               sizeof(prevBankUpdateFreq));
        // retrieve mode from related bank
        for (uint8_t i = 0; i < MAX_MIX_ACTUATORS; i++) {
            uint8_t bank = PIOS_Servo_GetPinBank(i);
            pinsMode[i] = actuatorSettings.BankMode[bank];
        }
    }
}

static void ActuatorSettingsUpdatedCb(__attribute__((unused)) UAVObjEvent *ev)
{
    ActuatorSettingsGet(&actuatorSettings);
    spinWhileArmed = actuatorSettings.MotorsSpinWhileArmed == ACTUATORSETTINGS_MOTORSSPINWHILEARMED_TRUE;
    if (frameType == FRAME_TYPE_GROUND) {
        spinWhileArmed = false;
    }
    actuator_update_rate_if_changed(false);
}

static void MixerSettingsUpdatedCb(__attribute__((unused)) UAVObjEvent *ev)
{
    MixerSettingsGet(&mixerSettings);
    mixer_settings_count = 0;
    Mixer_t *mixers = (Mixer_t *)&mixerSettings.Mixer1Type;
    for (int ct = 0; ct < MAX_MIX_ACTUATORS; ct++) {
        if (mixers[ct].type != MIXERSETTINGS_MIXER1TYPE_DISABLED) {
            mixer_settings_count++;
        }
    }
}
static void SettingsUpdatedCb(__attribute__((unused)) UAVObjEvent *ev)
{
    frameType = GetCurrentFrameType();
#ifndef PIOS_EXCLUDE_ADVANCED_FEATURES
    uint8_t TreatCustomCraftAs;
    VtolPathFollowerSettingsTreatCustomCraftAsGet(&TreatCustomCraftAs);

    if (frameType == FRAME_TYPE_CUSTOM) {
        switch (TreatCustomCraftAs) {
        case VTOLPATHFOLLOWERSETTINGS_TREATCUSTOMCRAFTAS_FIXEDWING:
            frameType = FRAME_TYPE_FIXED_WING;
            break;
        case VTOLPATHFOLLOWERSETTINGS_TREATCUSTOMCRAFTAS_VTOL:
            frameType = FRAME_TYPE_MULTIROTOR;
            break;
        case VTOLPATHFOLLOWERSETTINGS_TREATCUSTOMCRAFTAS_GROUND:
            frameType = FRAME_TYPE_GROUND;
            break;
        }
    }
#endif

    SystemSettingsThrustControlGet(&thrustType);
}

/**
 * @}
 * @}
 */

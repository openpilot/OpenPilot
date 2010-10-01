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
//#include "vtolsettings.h"
#include "systemsettings.h"
#include "actuatordesired.h"
#include "actuatorcommand.h"
#include "manualcontrolcommand.h"
#include "mixersettings.h"
#include "mixerstatus.h"


// Private constants
#define MAX_QUEUE_SIZE 2
#define STACK_SIZE configMINIMAL_STACK_SIZE
#define TASK_PRIORITY (tskIDLE_PRIORITY+4)
#define FAILSAFE_TIMEOUT_MS 100
#define MAX_MIX_ACTUATORS ACTUATORCOMMAND_CHANNEL_NUMELEM

// Private types


// Private variables
static xQueueHandle queue;
static xTaskHandle taskHandle;

// Private functions
static void actuatorTask(void* parameters);
static int16_t scaleChannel(float value, int16_t max, int16_t min, int16_t neutral);
static void setFailsafe();
static float MixerCurve(const float throttle, const float* curve);
float ProcessMixer(const int index, const float curve1, const float curve2,
		   MixerSettingsData* mixerSettings, ActuatorDesiredData* desired,
		   const float period);


//this structure is equivalent to the UAVObjects for one mixer.
typedef struct {
	uint8_t type;
	float matrix[5];
} __attribute__((packed)) Mixer_t;


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
 * @brieft Main Actuator module task
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
static void actuatorTask(void* parameters)
{
	UAVObjEvent ev;
	portTickType lastSysTime;
	portTickType thisSysTime;
	float dT;
	ActuatorCommandData command;
	ActuatorSettingsData settings;
	
	SystemSettingsData sysSettings;
	MixerSettingsData mixerSettings;
	ActuatorDesiredData desired;
	MixerStatusData mixerStatus;
	ManualControlCommandData manualControl;
	
	ActuatorSettingsGet(&settings);
	// Set servo update frequency (done only on start-up)
	PIOS_Servo_SetHz(settings.ChannelUpdateFreq[0], settings.ChannelUpdateFreq[1]);
		
	float * status = (float *)&mixerStatus; //access status objects as an array of floats		
	
	// Go to the neutral (failsafe) values until an ActuatorDesired update is received
	setFailsafe();

	// Main task loop
	lastSysTime = xTaskGetTickCount();
	while (1)
	{
                // Wait until the ActuatorDesired object is updated, if a timeout then go to failsafe
                if ( xQueueReceive(queue, &ev, FAILSAFE_TIMEOUT_MS / portTICK_RATE_MS) != pdTRUE )
                {
                        setFailsafe();
                        continue;
                }
		
		// Check how long since last update
		thisSysTime = xTaskGetTickCount();
		if(thisSysTime > lastSysTime) // reuse dt in case of wraparound
			dT = (thisSysTime - lastSysTime) / portTICK_RATE_MS / 1000.0f;		
		lastSysTime = thisSysTime;
		
		
		ManualControlCommandGet(&manualControl);
		SystemSettingsGet(&sysSettings);
		MixerStatusGet(&mixerStatus);
		MixerSettingsGet (&mixerSettings);
		ActuatorDesiredGet(&desired);
		ActuatorCommandGet(&command);
		ActuatorSettingsGet(&settings);
		
		int nMixers = 0;
		Mixer_t * mixers = (Mixer_t *)&mixerSettings.Mixer0Type;
		for(int ct=0; ct < MAX_MIX_ACTUATORS; ct++)
		{
			if(mixers[ct].type != MIXERSETTINGS_MIXER0TYPE_DISABLED)
			{
				nMixers ++;
			}
		}
		if(nMixers < 2) //Nothing can fly with less than two mixers.
		{
			AlarmsSet(SYSTEMALARMS_ALARM_ACTUATOR, SYSTEMALARMS_ALARM_WARNING);
			continue;			
		}
		
		AlarmsClear(SYSTEMALARMS_ALARM_ACTUATOR);
		
		bool armed = manualControl.Armed == MANUALCONTROLCOMMAND_ARMED_TRUE;
		armed &= desired.Throttle > 0.05; //zero throttle stops the motors
				
		float curve1 = MixerCurve(desired.Throttle,mixerSettings.ThrottleCurve1);
		float curve2 = MixerCurve(desired.Throttle,mixerSettings.ThrottleCurve2);
		for(int ct=0; ct < MAX_MIX_ACTUATORS; ct++)
		{
			if(mixers[ct].type != MIXERSETTINGS_MIXER0TYPE_DISABLED)
			{
				status[ct] = ProcessMixer(ct, curve1, curve2, &mixerSettings, &desired, dT);

				if(!armed &&
				   mixers[ct].type == MIXERSETTINGS_MIXER0TYPE_MOTOR)
				{
					command.Channel[ct] = settings.ChannelMin[ct]; //force zero throttle
				}else
				{
					command.Channel[ct] = scaleChannel(status[ct],
									   settings.ChannelMax[ct],
									   settings.ChannelMin[ct],
									   settings.ChannelNeutral[ct]);
				}
			}
		}
		MixerStatusSet(&mixerStatus);		
		
		// Update output object
		ActuatorCommandSet(&command);
		// Update in case read only (eg. during servo configuration)
		ActuatorCommandGet(&command);
		
		// Update servo outputs
		for (int n = 0; n < ACTUATORCOMMAND_CHANNEL_NUMELEM; ++n)
		{
			PIOS_Servo_Set( n, command.Channel[n] );
		}
	}
}



/**
 *Process mixing for one actuator
 */

float ProcessMixer(const int index, const float curve1, const float curve2,
		   MixerSettingsData* mixerSettings, ActuatorDesiredData* desired, const float period)
{
	static float lastResult[MAX_MIX_ACTUATORS]={0,0,0,0,0,0,0,0};
	static float lastFilteredResult[MAX_MIX_ACTUATORS]={0,0,0,0,0,0,0,0};
	static float filterAccumulator[MAX_MIX_ACTUATORS]={0,0,0,0,0,0,0,0};
	Mixer_t * mixers = (Mixer_t *)&mixerSettings->Mixer0Type; //pointer to array of mixers in UAVObjects
	Mixer_t * mixer = &mixers[index];
	float result = (mixer->matrix[MIXERSETTINGS_MIXER0VECTOR_THROTTLECURVE1] * curve1) +
	(mixer->matrix[MIXERSETTINGS_MIXER0VECTOR_THROTTLECURVE2] * curve2) +
	(mixer->matrix[MIXERSETTINGS_MIXER0VECTOR_ROLL] * desired->Roll) +
	(mixer->matrix[MIXERSETTINGS_MIXER0VECTOR_PITCH] * desired->Pitch) +
	(mixer->matrix[MIXERSETTINGS_MIXER0VECTOR_YAW] * desired->Yaw);
	if(mixer->type == MIXERSETTINGS_MIXER0TYPE_MOTOR)
	{
		if(result < 0) //idle throttle
		{
			result = 0;
		}
		
		//feed forward
		float accumulator = filterAccumulator[index];
		accumulator += (result - lastResult[index]) * mixerSettings->FeedForward;
		lastResult[index] = result;
		result += accumulator;
		if(accumulator > 0)
		{
			float filter = mixerSettings->AccelTime / period;
			if(filter <1)
			{
				filter = 1;
			}
			accumulator -= accumulator / filter;
		}else
		{
			float filter = mixerSettings->DecelTime / period;
			if(filter <1)
			{
				filter = 1;
			}
			accumulator -= accumulator / filter;
		}
		filterAccumulator[index] = accumulator;
		result += accumulator;
		
		//acceleration limit
		float dt = result - lastFilteredResult[index];
		float maxDt = mixerSettings->MaxAccel * period;
		if(dt > maxDt) //we are accelerating too hard
		{
			result = lastFilteredResult[index] + maxDt;
		}
		lastFilteredResult[index] = result;
	}
	return(result);
}


/**
 *Interpolate a throttle curve. Throttle input should be in the range 0 to 1.
 *Output is in the range 0 to 1.
 */

#define MIXER_CURVE_ENTRIES 5

static float MixerCurve(const float throttle, const float* curve)
{
	float scale = throttle * MIXER_CURVE_ENTRIES;
	int idx1 = scale;
	scale -= (float)idx1; //remainder
	if(curve[0] < -1)
	{
		return(throttle);
	}
	if (idx1 < 0)
	{
		idx1 = 0; //clamp to lowest entry in table
		scale = 0;
	}
	int idx2 = idx1 + 1;
	if(idx2 >= MIXER_CURVE_ENTRIES)
	{
		idx2 = MIXER_CURVE_ENTRIES -1; //clamp to highest entry in table
		if(idx1 >= MIXER_CURVE_ENTRIES)
		{
			idx1 = MIXER_CURVE_ENTRIES -1;
		}
	}
	return((curve[idx1] * (1 - scale)) + (curve[idx2] * scale));
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
	ActuatorCommandData command;
	ActuatorSettingsData settings;
	
	ActuatorCommandGet(&command);
	ActuatorSettingsGet(&settings);
	// Reset ActuatorCommand to neutral values
	for (int n = 0; n < ACTUATORCOMMAND_CHANNEL_NUMELEM; ++n)
	{
		command.Channel[n] = settings.ChannelNeutral[n];
	}
	
	// Set alarm
	AlarmsSet(SYSTEMALARMS_ALARM_ACTUATOR, SYSTEMALARMS_ALARM_CRITICAL);
	
	// Update servo outputs
	for (int n = 0; n < ACTUATORCOMMAND_CHANNEL_NUMELEM; ++n)
	{
		PIOS_Servo_Set( n, command.Channel[n] );
	}
	
	// Update output object
	ActuatorCommandSet(&command);
}


/**
 * @}
 * @}
 */

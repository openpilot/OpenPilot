/**
 ******************************************************************************
 * @addtogroup OpenPilotModules OpenPilot Modules
 * @{
 * @addtogroup StabilizationModule Stabilization Module
 * @brief Relay tuning controller
 * @note This object updates the @ref ActuatorDesired "Actuator Desired" based on the
 * PID loops on the @ref AttitudeDesired "Attitude Desired" and @ref AttitudeActual "Attitude Actual"
 * @{
 *
 * @file       stabilization.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      Attitude stabilization module.
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
#include "stabilization.h"
#include "stabilizationsettings.h"
#include "actuatordesired.h"
#include "ratedesired.h"
#include "relaytuning.h"
#include "relaytuningsettings.h"
#include "stabilizationdesired.h"
#include "attitudeactual.h"
#include "gyros.h"
#include "flightstatus.h"
#include "manualcontrol.h" // Just to get a macro
#include "CoordinateConversions.h"

//! Private variables
static float *sin_lookup; // TODO: Move this to flash
static const int SIN_RESOLUTION = 180;

//! Private methods
static float sin_l(int angle);

#define MAX_AXES 3

int stabilization_relay_init()
{
	sin_lookup = (float *) pvPortMalloc(sizeof(float) * SIN_RESOLUTION);
	if (sin_lookup == NULL)
		return -1;

	for(uint32_t i = 0; i < 180; i++)
		sin_lookup[i] = sinf((float)i * 2 * M_PI / 360.0f);

	return 0;
}

/**
 * Apply a step function for the stabilization controller and monitor the 
 * result
 *
 * Used to  Replace the rate PID with a relay to measure the critical properties of this axis
 * i.e. period and gain
 */
int stabilization_relay_rate(float error, float *output, int axis, bool reinit)
{
	RelayTuningData relay;
	RelayTuningGet(&relay);

	static bool high = false;
	static portTickType lastHighTime;
	static portTickType lastLowTime;

	static float accum_sin, accum_cos;
	static uint32_t accumulated = 0;

	const uint16_t DEGLITCH_TIME = 20; // ms
	const float AMPLITUDE_ALPHA = 0.95;
	const float PERIOD_ALPHA = 0.95;

	portTickType thisTime = xTaskGetTickCount();

	static bool rateRelayRunning[MAX_AXES];

	// On first run initialize estimates to something reasonable
	if(reinit) {
		rateRelayRunning[axis] = false;
		relay.Period[axis] = 200;
		relay.Gain[axis] = 0;

		accum_sin = 0;
		accum_cos = 0;
		accumulated = 0;

		// These should get reinitialized anyway
		high = true;
		lastHighTime = thisTime;
		lastLowTime = thisTime;
		RelayTuningSet(&relay);
	}


	RelayTuningSettingsData relaySettings;
	RelayTuningSettingsGet(&relaySettings);

	// Compute output, simple threshold on error
	*output = error > 0 ? relaySettings.Amplitude : -relaySettings.Amplitude;

	/**** The code below here is to estimate the properties of the oscillation ****/

	// Make sure the period can't go below limit
	if (relay.Period[axis] < DEGLITCH_TIME)
		relay.Period[axis] = DEGLITCH_TIME;

	// Project the error onto a sine and cosine of the same frequency
	// to accumulate the average amplitude
	int dT = thisTime - lastHighTime;
	uint32_t phase = 360 * dT / relay.Period[axis];
	if(phase >= 360)
		phase = 1;
	accum_sin += sin_l(phase) * error;
	accum_cos += sin_l(phase + 90) * error;
	accumulated ++;

	// Make sure we've had enough time since last transition then check for a change in the output
	bool hysteresis = (high ? (thisTime - lastHighTime) : (thisTime - lastLowTime)) > DEGLITCH_TIME;
	if ( !high && hysteresis && error > 0 ){ /* RISE DETECTED */
		float this_amplitude = 2 * sqrtf(accum_sin*accum_sin + accum_cos*accum_cos) / accumulated;
		float this_gain = this_amplitude / relaySettings.Amplitude;

		accumulated = 0;
		accum_sin = 0;
		accum_cos = 0;

		if(rateRelayRunning[axis] == false) {
			rateRelayRunning[axis] = true;
			relay.Period[axis] = 200;
			relay.Gain[axis] = 0;
		} else {
			// Low pass filter each amplitude and period
			relay.Gain[axis] = relay.Gain[axis] * AMPLITUDE_ALPHA + this_gain * (1 - AMPLITUDE_ALPHA);
			relay.Period[axis] = relay.Period[axis] * PERIOD_ALPHA + dT * (1 - PERIOD_ALPHA);
		}
		lastHighTime = thisTime;
		high = true;
		RelayTuningSet(&relay);
	} else if ( high && hysteresis && error < 0 ) { /* FALL DETECTED */
		lastLowTime = thisTime;
		high = false;
	}

	return 0;
}


/**
 * Uses the lookup table to calculate sine (angle is in degrees)
 * @param[in] angle in degrees
 * @returns sin(angle)
 */
static float sin_l(int angle) {
	angle = angle % 360;
	if (angle > 180)
		return - sin_lookup[angle-180];
	else
		return sin_lookup[angle];
}


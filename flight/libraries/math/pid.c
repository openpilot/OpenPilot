/**
 ******************************************************************************
 * @addtogroup OpenPilot Math Utilities
 * @{
 * @addtogroup Sine and cosine methods that use a cached lookup table
 * @{
 *
 * @file       pid.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2012.
 * @brief      Methods to work with PID structure
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
#include "pid.h"

#define F_PI ((float) M_PI)

//! Private method
static float bound(float val, float range);

//! Store the shared time constant for the derivative cutoff.
static float deriv_tau = 7.9577e-3f;

//! Store the setpoint weight to apply for the derivative term
static float deriv_gamma = 1.0;

/**
 * Update the PID computation
 * @param[in] pid The PID struture which stores temporary information
 * @param[in] err The error term
 * @param[in] dT  The time step
 * @returns Output the computed controller value
 */
float pid_apply(struct pid *pid, const float err, float dT)
{	
	// Scale up accumulator by 1000 while computing to avoid losing precision
	pid->iAccumulator += err * (pid->i * dT * 1000.0f);
	pid->iAccumulator = bound(pid->iAccumulator, pid->iLim * 1000.0f);

	// Calculate DT1 term
	float diff = (err - pid->lastErr);
	float dterm = 0;
	pid->lastErr = err;
	if(pid->d && dT)
	{
		dterm = pid->lastDer +  dT / ( dT + deriv_tau) * ((diff * pid->d / dT) - pid->lastDer);
		pid->lastDer = dterm;            //   ^ set constant to 1/(2*pi*f_cutoff)
	}	                                 //   7.9577e-3  means 20 Hz f_cutoff
 
	return ((err * pid->p) + pid->iAccumulator / 1000.0f + dterm);
}

/**
 * Update the PID computation with setpoint weighting on the derivative
 * @param[in] pid The PID struture which stores temporary information
 * @param[in] setpoint The setpoint to use
 * @param[in] measured The measured value of output
 * @param[in] dT  The time step
 * @returns Output the computed controller value
 *
 * This version of apply uses setpoint weighting for the derivative component so the gain
 * on the gyro derivative can be different than the gain on the setpoint derivative
 */
float pid_apply_setpoint(struct pid *pid, const float setpoint, const float measured, float dT)
{
	float err = setpoint - measured;
	
	// Scale up accumulator by 1000 while computing to avoid losing precision
	pid->iAccumulator += err * (pid->i * dT * 1000.0f);
	pid->iAccumulator = bound(pid->iAccumulator, pid->iLim * 1000.0f);

	// Calculate DT1 term,
	float dterm = 0;
	float diff = ((deriv_gamma * setpoint - measured) - pid->lastErr);
	pid->lastErr = (deriv_gamma * setpoint - measured);
	if(pid->d && dT)
	{
		dterm = pid->lastDer +  dT / ( dT + deriv_tau) * ((diff * pid->d / dT) - pid->lastDer);
		pid->lastDer = dterm;            //   ^ set constant to 1/(2*pi*f_cutoff)
	}	                                 //   7.9577e-3  means 20 Hz f_cutoff
 
	return ((err * pid->p) + pid->iAccumulator / 1000.0f + dterm);
}

/**
 * Reset a bit
 * @param[in] pid The pid to reset
 */
void pid_zero(struct pid *pid)
{
	if (!pid)
		return;

	pid->iAccumulator = 0;
	pid->lastErr = 0;
	pid->lastDer = 0;
}

/**
 * @brief Configure the common terms that alter ther derivative
 * @param[in] cutoff The cutoff frequency (in Hz)
 * @param[in] gamma The gamma term for setpoint shaping (unsused now)
 */
void pid_configure_derivative(float cutoff, float g)
{
	deriv_tau = 1.0f / (2 * F_PI * cutoff);
	deriv_gamma = g;
}

/**
 * Configure the settings for a pid structure
 * @param[out] pid The PID structure to configure
 * @param[in] p The proportional term
 * @param[in] i The integral term
 * @param[in] d The derivative term
 */
void pid_configure(struct pid *pid, float p, float i, float d, float iLim)
{
	if (!pid)
		return;

	pid->p = p;
	pid->i = i;
	pid->d = d;
	pid->iLim = iLim;
}

/**
 * Bound input value between limits
 */
static float bound(float val, float range)
{
	if(val < -range) {
		val = -range;
	} else if(val > range) {
		val = range;
	}
	return val;
}


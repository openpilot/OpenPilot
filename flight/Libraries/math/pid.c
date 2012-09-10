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

//! Private method
static float bound(float val, float range);

float pid_apply(struct pid *pid, const float err, float dT)
{
	float diff = (err - pid->lastErr);
	float dterm = 0;
	pid->lastErr = err;
	
	// Scale up accumulator by 1000 while computing to avoid losing precision
	pid->iAccumulator += err * (pid->i * dT * 1000.0f);
	pid->iAccumulator = bound(pid->iAccumulator, pid->iLim * 1000.0f);

	// Calculate DT1 term, fixed T1 timeconstant
	if(pid->d && dT)
	{
		dterm = pid->lastDer +  dT / ( dT + 7.9577e-3f) * ((diff * pid->d / dT) - pid->lastDer);
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


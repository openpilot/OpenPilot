/**
 ******************************************************************************
 * @addtogroup OpenPilot Math Utilities
 * @{
 * @addtogroup Sine and cosine methods that use a cached lookup table
 * @{
 *
 * @file       pid.h
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

#ifndef PID_H
#define PID_H

//! 
struct pid {
	float p;
	float i;
	float d;
	float iLim;
	float iAccumulator;
	float lastErr;
	float lastDer;
};

//! Methods to use the pid structures
float pid_apply(struct pid *pid, const float err, float dT);
float pid_apply_setpoint(struct pid *pid, const float setpoint, const float measured, float dT);
void pid_zero(struct pid *pid);
void pid_configure(struct pid *pid, float p, float i, float d, float iLim);
void pid_configure_derivative(float cutoff, float gamma);

#endif /* PID_H */
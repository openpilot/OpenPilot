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
#include <mathmisc.h>
#include <pios_math.h>

// ! Store the shared time constant for the derivative cutoff.
static float deriv_tau   = 7.9577e-3f;

// ! Store the setpoint weight to apply for the derivative term
static float deriv_gamma = 1.0f;

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
    pid->iAccumulator  = boundf(pid->iAccumulator, pid->iLim * -1000.0f, pid->iLim * 1000.0f);

    // Calculate DT1 term
    float diff  = (err - pid->lastErr);
    float dterm = 0;
    pid->lastErr = err;
    if (pid->d > 0.0f && dT > 0.0f) {
        dterm = pid->lastDer + dT / (dT + deriv_tau) * ((diff * pid->d / dT) - pid->lastDer);
        pid->lastDer = dterm; // ^ set constant to 1/(2*pi*f_cutoff)
    } // 7.9577e-3  means 20 Hz f_cutoff

    return (err * pid->p) + pid->iAccumulator / 1000.0f + dterm;
}

/**
 * Update the PID computation with setpoint weighting on the derivative
 * @param[in] pid The PID struture which stores temporary information
 * @param[in] factor A dynamic factor to scale pid's by, to compensate nonlinearities
 * @param[in] setpoint The setpoint to use
 * @param[in] measured The measured value of output
 * @param[in] dT  The time step
 * @returns Output the computed controller value
 *
 * This version of apply uses setpoint weighting for the derivative component so the gain
 * on the gyro derivative can be different than the gain on the setpoint derivative
 */
float pid_apply_setpoint(struct pid *pid, const pid_scaler *scaler, const float setpoint, const float measured, float dT)
{
    float err = setpoint - measured;

    // Scale up accumulator by 1000 while computing to avoid losing precision
    pid->iAccumulator += err * (scaler->i * pid->i * dT * 1000.0f);
    pid->iAccumulator  = boundf(pid->iAccumulator, pid->iLim * -1000.0f, pid->iLim * 1000.0f);

    // Calculate DT1 term,
    float dterm = 0;
    float diff  = ((deriv_gamma * setpoint - measured) - pid->lastErr);
    pid->lastErr = (deriv_gamma * setpoint - measured);
    if (pid->d > 0.0f && dT > 0.0f) {
        // low pass filter derivative term. below formula is the same as
        // dterm = (1-alpha)*pid->lastDer + alpha * (...)/dT
        // with alpha = dT/(deriv_tau+dT)
        dterm = pid->lastDer + dT / (dT + deriv_tau) * ((scaler->d * diff * pid->d / dT) - pid->lastDer);
        pid->lastDer = dterm;
    }

    return (err * scaler->p * pid->p) + pid->iAccumulator / 1000.0f + dterm;
}

/**
 * Reset a bit
 * @param[in] pid The pid to reset
 */
void pid_zero(struct pid *pid)
{
    if (!pid) {
        return;
    }

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
    deriv_tau   = 1.0f / (2 * M_PI_F * cutoff);
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
    if (!pid) {
        return;
    }

    pid->p    = p;
    pid->i    = i;
    pid->d    = d;
    pid->iLim = iLim;
}


/**
 * Configure the settings for a pid2 structure
 * @param[out] pid The PID2 structure to configure
 * @param[in] kp proportional gain
 * @param[in] ki integral gain.  Time constant Ti = kp/ki
 * @param[in] kd derivative gain. Time constant Td = kd/kp
 * @param[in] Tf filtering time = (kd/k)/N, N is in the range of 2 to 20
 * @param[in] kt tracking gain for anti-windup. Tt = √TiTd and Tt = (Ti + Td)/2
 * @param[in] dt delta time increment
 * @param[in] beta setpoint weight on setpoint in P component.  beta=1 error feedback. beta=0 smoothes out response to changes in setpoint
 * @param[in] u0 initial output for r=y at activation to achieve bumpless transfer
 * @param[in] va constant for compute of actuator output for check against limits for antiwindup
 * @param[in] vb multiplier for compute of actuator output for check against limits for anti-windup
 */
void pid2_configure(struct pid2 *pid, float kp, float ki, float kd, float Tf, float kt, float dT, float beta, float u0, float va, float vb)
{
    pid->reconfigure = true;
    pid->u0   = u0;
    pid->va   = va;
    pid->vb   = vb;
    pid->kp   = kp;
    pid->beta = beta; // setpoint weight on proportional term

    pid->bi   = ki * dT;
    pid->br   = kt * dT / vb;

    pid->ad   = Tf / (Tf + dT);
    pid->bd   = kd / (Tf + dT);
}

/**
 * Achieve a bumpless transfer and trigger initialisation of I term
 * @param[out] pid The PID structure to configure
 * @param[in] u0 initial output for r=y at activation to achieve bumpless transfer
 */
void pid2_transfer(struct pid2 *pid, float u0)
{
    pid->reconfigure = true;
    pid->u0 = u0;
}

/**
 * pid controller with setpoint weighting, anti-windup, with a low-pass filtered derivative on the process variable
 * See "Feedback Systems" for an explanation
 * @param[out] pid The PID structure to configure
 * @param[in] r setpoint
 * @param[in] y process variable
 * @param[in] ulow lower limit on actuator
 * @param[in] uhigh upper limit on actuator
 */
float pid2_apply(
    struct pid2 *pid,
    const float r,
    const float y,
    const float ulow,
    const float uhigh)
{
    // on reconfigure ensure bumpless transfer
    // http://www.controlguru.com/2008/021008.html
    if (pid->reconfigure) {
        pid->reconfigure = false;

        // initialise derivative terms
        pid->yold = y;
        pid->D    = 0.0f;

        // t=0, u=u0, y=y0, v=u
        pid->I    = (pid->u0 - pid->va) / pid->vb - pid->kp * (pid->beta * r - y);
    }

    // compute proportional part
    pid->P = pid->kp * (pid->beta * r - y);

    // update derivative part
    pid->D = pid->ad * pid->D - pid->bd * (y - pid->yold);

    // compute temporary output
    float v = pid->va + pid->vb * (pid->P + pid->I + pid->D);

    // simulate actuator saturation
    float u = boundf(v, ulow, uhigh);

    // update integral
    pid->I    = pid->I + pid->bi * (r - y) + pid->br * (u - v);

    // update old process output
    pid->yold = y;

    return u;
}

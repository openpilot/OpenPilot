/**
 ******************************************************************************
 * @addtogroup OpenPilot Math Utilities
 * @{
 * @addtogroup Butterworth low pass filter
 * @{
 *
 * @file       butterworth.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2014.
 * @brief      Direct form two of a second order Butterworth low pass filter
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
#include "math.h"
#include "butterworth.h"

#define SQRT2 1.414213562f

/**
 * Initialization function for coefficients of a second order Butterworth biquadratic filter in direct from 2.
 * Note that b1  = 2 * b0 and b2  = b0 is use here and in the sequel.
 * @param[in]  ff Cut-off frequency ratio
 * @param[out] filterPtr Pointer to filter coefficients
 * @returns Nothing
 */
void InitButterWorthDF2Filter(const float ff, struct ButterWorthDF2Filter *filterPtr)
{
    const float ita = 1.0f / tanf(M_PI_F * ff);
    const float b0  = 1.0f / (1.0f + SQRT2 * ita + ita * ita);
    const float a1  = 2.0f * b0 * (ita * ita - 1.0f);
    const float a2  = -b0 * (1.0f - SQRT2 * ita + ita * ita);

    filterPtr->b0 = b0;
    filterPtr->a1 = a1;
    filterPtr->a2 = a2;
}


/**
 * Initialization function for intermediate values of a second order Butterworth biquadratic filter in direct from 2.
 * Obtained by solving a linear equation system.
 * @param[in]  x0 Prescribed value
 * @param[in]  filterPtr Pointer to filter coefficients
 * @param[out] wn1Ptr Pointer to first intermediate value
 * @param[out] wn2Ptr Pointer to second intermediate value
 * @returns Nothing
 */
void InitButterWorthDF2Values(const float x0, const struct ButterWorthDF2Filter *filterPtr, float *wn1Ptr, float *wn2Ptr)
{
    const float b0   = filterPtr->b0;
    const float a1   = filterPtr->a1;
    const float a2   = filterPtr->a2;

    const float a11  = 2.0f + a1;
    const float a12  = 1.0f + a2;
    const float a21  = 2.0f + a1 * a1 + a2;
    const float a22  = 1.0f + a1 * a2;
    const float det  = a11 * a22 - a12 * a21;
    const float rhs1 = x0 / b0 - x0;
    const float rhs2 = x0 / b0 - x0 + a1 * x0;

    *wn1Ptr = (a22 * rhs1 - a12 * rhs2) / det;
    *wn2Ptr = (-a21 * rhs1 + a11 * rhs2) / det;
}


/**
 * Second order Butterworth biquadratic filter in direct from 2, such that only two values wn1=w[n-1] and wn2=w[n-2] need to be stored.
 * Function takes care of updating the values wn1 and wn2.
 * @param[in]  xn New raw value
 * @param[in]  filterPtr Pointer to filter coefficients
 * @param[out] wn1Ptr Pointer to first intermediate value
 * @param[out] wn2Ptr Pointer to second intermediate value
 * @returns Filtered value
 */
float FilterButterWorthDF2(const float xn, const struct ButterWorthDF2Filter *filterPtr, float *wn1Ptr, float *wn2Ptr)
{
    const float wn  = xn + filterPtr->a1 * (*wn1Ptr) + filterPtr->a2 * (*wn2Ptr);
    const float val = filterPtr->b0 * (wn + 2.0f * (*wn1Ptr) + (*wn2Ptr));

    *wn2Ptr = *wn1Ptr;
    *wn1Ptr = wn;
    return val;
}

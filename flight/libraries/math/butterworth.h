/**
 ******************************************************************************
 * @addtogroup OpenPilot Math Utilities
 * @{
 * @addtogroup Butterworth low pass filter
 * @{
 *
 * @file       butterworth.h
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

#ifndef BUTTERWORTH_H
#define BUTTERWORTH_H

// Coefficients of second order Butterworth biquadratic filter in direct from 2
struct ButterWorthDF2Filter {
    float b0;
    float a1;
    float a2;
};

// Function declarations
void InitButterWorthDF2Filter(const float ff, struct ButterWorthDF2Filter *filterPtr);
void InitButterWorthDF2Values(const float x0, const struct ButterWorthDF2Filter *filterPtr, float *wn1Ptr, float *wn2Ptr);
float FilterButterWorthDF2(const float xn, const struct ButterWorthDF2Filter *filterPtr, float *wn1Ptr, float *wn2Ptr);

#endif

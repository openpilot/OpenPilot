/**
 ******************************************************************************
 * @addtogroup OpenPilot Math Utilities
 * @{
 * @addtogroup Reuseable math functions
 * @{
 *
 * @file       mathmisc.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2012.
 * @brief      Reuseable math functions
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

#ifndef MATHMISC_H
#define MATHMISC_H

#include <math.h>
#include <stdint.h>

// returns min(boundary1,boundary2) if val<min(boundary1,boundary2)
// returns max(boundary1,boundary2) if val>max(boundary1,boundary2)
// returns val if min(boundary1,boundary2)<=val<=max(boundary1,boundary2)
static inline float boundf(float val, float boundary1, float boundary2)
{
    if (boundary1 > boundary2) {
        if (!(val >= boundary2)) {
            return boundary2;
        } else if (!(val <= boundary1)) {
            return boundary1;
        }
    } else {
        if (!(val >= boundary1)) {
            return boundary1;
        } else if (!(val <= boundary2)) {
            return boundary2;
        }
    }
    return val;
}

static inline float squaref(float x)
{
    return x * x;
}

static inline float vector_lengthf(float *vector, const uint8_t dim)
{
    float length = 0.0f;

    for (int t = 0; t < dim; t++) {
        length += squaref(vector[t]);
    }
    return sqrtf(length);
}

static inline void vector_normalizef(float *vector, const uint8_t dim)
{
    float length = vector_lengthf(vector, dim);

    if (length <= 0.0f || isnan(length)) {
        return;
    }
    for (int t = 0; t < dim; t++) {
        vector[t] /= length;
    }
}

typedef struct pointf {
    float x;
    float y;
} pointf;

// Returns the y value, given x, on the line passing through the points p0 and p1.
static inline float y_on_line(float x, const pointf *p0, const pointf *p1)
{
    // Setup line y = m * x + b.
    const float dY1 = p1->y - p0->y;
    const float dX1 = p1->x - p0->x;
    const float m   = dY1 / dX1; // == dY0 / dX0 == (p0.y - b) / (p0.x - 0.0f) ==>
    const float b   = p0->y - m * p0->x;

    // Get the y value on the line.
    return m * x + b;
}

// Returns the y value, given x, on the curve defined by the points array.
// The fist and last line of the curve extends beyond the first resp. last points.
static inline float y_on_curve(float x, const pointf points[], int num_points)
{
    // Find the two points x is within.
    // If x is smaller than the first point's x value, use the first line of the curve.
    // If x is larger than the last point's x value, user the last line of the curve.
    int end_point = num_points - 1;

    for (int i = 1; i < num_points; i++) {
        if (x < points[i].x) {
            end_point = i;
            break;
        }
    }

    // Find the y value on the selected line.
    return y_on_line(x, &points[end_point - 1], &points[end_point]);
}
// Fast inverse square root implementation from "quake3-1.32b/code/game/q_math.c"
// http://en.wikipedia.org/wiki/Fast_inverse_square_root

static inline float fast_invsqrtf(float number)
{
    float x2, y;
    const float threehalfs = 1.5F;

    union {
        float    f;
        uint32_t u;
    } i;

    x2  = number * 0.5F;
    y   = number;

    i.f = y; // evil floating point bit level hacking
    i.u = 0x5f3759df - (i.u >> 1); // what the fxck?
    y   = i.f;
    y   = y * (threehalfs - (x2 * y * y));   // 1st iteration
// y  = y * ( threehalfs - ( x2 * y * y ) );   // 2nd iteration, this can be removed

    return y;
}

/**
 * Ultrafast pow() aproximation needed for expo
 * Based on Algorithm by Martin Ankerl
 */
static inline float fastPow(float a, float b)
{
    union {
        double  d;
        int32_t x[2];
    } u = { (double)a };
    u.x[1] = (int32_t)(b * (u.x[1] - 1072632447) + 1072632447);
    u.x[0] = 0;
    return (float)u.d;
}

#endif /* MATHMISC_H */

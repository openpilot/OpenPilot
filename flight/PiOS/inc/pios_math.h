/**
 ******************************************************************************
 *
 * @file       pios_math.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      Header for math functions/macro definitions
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

#ifndef PIOS_MATH_H
#define PIOS_MATH_H
// Generic float math constants
#define M_E_F        2.71828182845904523536028747135f      /* e */
#define M_LOG2E_F    1.44269504088896340735992468100f      /* log_2 (e) */
#define M_LOG10E_F   0.43429448190325182765112891892f      /* log_10 (e) */
#define M_SQRT2_F    1.41421356237309504880168872421f      /* sqrt(2) */
#define M_SQRT1_2_F  0.70710678118654752440084436210f      /* sqrt(1/2) */
#define M_SQRT3_F    1.73205080756887729352744634151f      /* sqrt(3) */
#define M_PI_F       3.14159265358979323846264338328f      /* pi */
#define M_PI_2_F     1.57079632679489661923132169164f      /* pi/2 */
#define M_PI_4_F     0.78539816339744830961566084582f      /* pi/4 */
#define M_SQRTPI_F   1.77245385090551602729816748334f      /* sqrt(pi) */
#define M_2_SQRTPI_F 1.12837916709551257389615890312f      /* 2/sqrt(pi) */
#define M_1_PI_F     0.31830988618379067153776752675f      /* 1/pi */
#define M_2_PI_F     0.63661977236758134307553505349f      /* 2/pi */
#define M_LN10_F     2.30258509299404568401799145468f      /* ln(10) */
#define M_LN2_F      0.69314718055994530941723212146f      /* ln(2) */
#define M_LNPI_F     1.14472988584940017414342735135f      /* ln(pi) */
#define M_EULER_F    0.57721566490153286060651209008f      /* Euler constant */

// Conversion macro
#define RAD2DEG(rad)    ((rad)*(180.0f/M_PI_F))
#define DEG2RAD(deg)    ((deg)*(M_PI_F/180.0f))

// Useful math macros
#define MAX(a,b) ((a) > (b) ? (a) : (b))
#define MIN(a,b) ((a) < (b) ? (a) : (b))

#endif // PIOS_MATH_H

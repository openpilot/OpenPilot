/*
* Copyright (C) 2012 Invensense, Inc.
*
* This software is licensed under the terms of the GNU General Public
* License version 2, as published by the Free Software Foundation, and
* may be copied, distributed, and modified under those terms.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
*/

/**
 *  @addtogroup  DRIVERS
 *  @brief       Hardware drivers.
 *
 *  @{
 *      @file    inv_gyro_misc.c
 *      @brief   A sysfs device driver for Invensense gyroscopes.
 *      @details This file is part of inv_gyro driver code
 */

/* source code extracted from:
 * https://kernel--tegra.android-source-browsing.googlecode.com/archive/74ae38d84cf2502ca3235d04ffe76cbe386908b5.zip
 * this is cut down version of this file
 * changes:
 * #include "inv_mpu_misc.h"
 * original function declaration was:
 * static unsigned short inv_row_2_scale(const signed char *row)
 * static unsigned short inv_orientation_matrix_to_scaler
 * rest is unchanged*/

#include "inv_mpu_misc.h"

unsigned short inv_row_2_scale(const signed char *row)
{
	unsigned short b;

	if (row[0] > 0)
		b = 0;
	else if (row[0] < 0)
		b = 4;
	else if (row[1] > 0)
		b = 1;
	else if (row[1] < 0)
		b = 5;
	else if (row[2] > 0)
		b = 2;
	else if (row[2] < 0)
		b = 6;
	else
		b = 7;
	/* error */
	return b;
}

/** Converts an orientation matrix made up of 0,+1,and -1 to a scalar
*	representation.
* @param[in] mtx Orientation matrix to convert to a scalar.
* @return Description of orientation matrix. The lowest 2 bits (0 and 1)
* represent the column the one is on for the
* first row, with the bit number 2 being the sign. The next 2 bits
* (3 and 4) represent
* the column the one is on for the second row with bit number 5 being
* the sign.
* The next 2 bits (6 and 7) represent the column the one is on for the
* third row with
* bit number 8 being the sign. In binary the identity matrix would therefor
* be: 010_001_000 or 0x88 in hex.
*/
unsigned short inv_orientation_matrix_to_scaler(const signed char *mtx)
{

	unsigned short scalar;
	scalar = inv_row_2_scale(mtx);
	scalar |= inv_row_2_scale(mtx + 3) << 3;
	scalar |= inv_row_2_scale(mtx + 6) << 6;

	return scalar;
}

/**
 *  @}
 */


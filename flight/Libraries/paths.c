/**
 ******************************************************************************
 *
 * @file       paths.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2012.
 * @brief      Library path manipulation 
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

#include "pios.h"
#include "paths.h"

/**
 * @brief Compute progress along path and deviation from it
 * @param[in] start_point Starting point
 * @param[in] end_point Ending point
 * @param[in] cur_point Current location
 * @param[out] status Structure containing progress along path and deviation
 */
void path_progress(float * start_point, float * end_point, float * cur_point, struct path_status * status)
{
	float path_north, path_east, diff_north, diff_east;
	float dist_path2;
	float dot;
	float normal[2];

	// Distance to go
	path_north = end_point[0] - start_point[0];
	path_east = end_point[1] - start_point[1];

	// Current progress location relative to start
	diff_north = cur_point[0] - start_point[0];
	diff_east = cur_point[1] - start_point[1];

	dot = path_north * diff_north + path_east * diff_east;
	dist_path2 = path_north * path_north + path_east * path_east;

	if(dist_path2 < 1e-3) {
		status->fractional_progress = 1;
		status->error = 0;
		status->correction_direction[0] = status->correction_direction[1] = 0;
		status->path_direction[0] = status->path_direction[1] = 0;
		return;
	}

	// Compute the normal to the path
	normal[0] = -path_east / sqrtf(dist_path2);
	normal[1] = path_north / sqrtf(dist_path2);

	status->fractional_progress = dot / dist_path2;
	status->error = normal[0] * diff_north + normal[1] * diff_east;

	// Compute direction to correct error
	status->correction_direction[0] = -status->error * normal[0];
	status->correction_direction[1] = -status->error * normal[1];

	// Compute direction to travel
	status->path_direction[0] = path_north / sqrtf(dist_path2);
	status->path_direction[1] = path_east / sqrtf(dist_path2);

	status->error = fabs(status->error);
}


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

#include <pios.h>
#include <pios_math.h>
#include <mathmisc.h>

#include "uavobjectmanager.h" // <--.
#include "pathdesired.h" // <-- needed only for correct ENUM macro usage with path modes (PATHDESIRED_MODE_xxx,
#include "paths.h"
// no direct UAVObject usage allowed in this file

// private functions
static void path_endpoint(PathDesiredData *path, float *cur_point, struct path_status *status, bool mode);
static void path_vector(PathDesiredData *path, float *cur_point, struct path_status *status, bool mode);
static void path_circle(PathDesiredData *path, float *cur_point, struct path_status *status, bool clockwise);

/**
 * @brief Compute progress along path and deviation from it
 * @param[in] path  PathDesired structure
 * @param[in] cur_point Current location
 * @param[out] status Structure containing progress along path and deviation
 */
void path_progress(PathDesiredData *path, float *cur_point, struct path_status *status)
{
    switch (path->Mode) {
    case PATHDESIRED_MODE_BRAKE: // should never get here...
    case PATHDESIRED_MODE_FLYVECTOR:
        return path_vector(path, cur_point, status, true);

        break;
    case PATHDESIRED_MODE_DRIVEVECTOR:
        return path_vector(path, cur_point, status, false);

        break;
    case PATHDESIRED_MODE_FLYCIRCLERIGHT:
    case PATHDESIRED_MODE_DRIVECIRCLERIGHT:
        return path_circle(path, cur_point, status, 1);

        break;
    case PATHDESIRED_MODE_FLYCIRCLELEFT:
    case PATHDESIRED_MODE_DRIVECIRCLELEFT:
        return path_circle(path, cur_point, status, 0);

        break;
    case PATHDESIRED_MODE_FLYENDPOINT:
        return path_endpoint(path, cur_point, status, true);

        break;
    case PATHDESIRED_MODE_DRIVEENDPOINT:
    default:
        // use the endpoint as default failsafe if called in unknown modes
        return path_endpoint(path, cur_point, status, false);

        break;
    }
}

/**
 * @brief Compute progress towards endpoint. Deviation equals distance
 * @param[in] path PathDesired
 * @param[in] cur_point Current location
 * @param[out] status Structure containing progress along path and deviation
 * @param[in] mode3D set true to include altitude in distance and progress calculation
 */
static void path_endpoint(PathDesiredData *path, float *cur_point, struct path_status *status, bool mode3D)
{
    float diff[3];
    float dist_path, dist_diff;

    // Distance to go
    status->path_vector[0] = path->End.North - path->Start.North;
    status->path_vector[1] = path->End.East - path->Start.East;
    status->path_vector[2] = mode3D ? path->End.Down - path->Start.Down : 0.0f;

    // Current progress location relative to end
    diff[0]   = path->End.North - cur_point[0];
    diff[1]   = path->End.East - cur_point[1];
    diff[2]   = mode3D ? path->End.Down - cur_point[2] : 0.0f;

    dist_diff = vector_lengthf(diff, 3);
    dist_path = vector_lengthf(status->path_vector, 3);

    if (dist_diff < 1e-6f) {
        status->fractional_progress  = 1;
        status->error = 0.0f;
        status->correction_vector[0] = status->correction_vector[1] = status->correction_vector[2] = 0.0f;
        // we have no base movement direction in this mode
        status->path_vector[0] = status->path_vector[1] = status->path_vector[2] = 0.0f;

        return;
    }

    if (fmaxf(dist_path, 1.0f) > dist_diff) {
        status->fractional_progress = 1 - dist_diff / fmaxf(dist_path, 1.0f);
    } else {
        status->fractional_progress = 0; // we don't want fractional_progress to become negative
    }
    status->error = dist_diff;

    // Compute correction vector
    status->correction_vector[0] = diff[0];
    status->correction_vector[1] = diff[1];
    status->correction_vector[2] = diff[2];

    // base movement direction in this mode is a constant velocity offset on top of correction in the same direction
    status->path_vector[0] = path->EndingVelocity * status->correction_vector[0] / dist_diff;
    status->path_vector[1] = path->EndingVelocity * status->correction_vector[1] / dist_diff;
    status->path_vector[2] = path->EndingVelocity * status->correction_vector[2] / dist_diff;
}

/**
 * @brief Compute progress along path and deviation from it
 * @param[in] path PathDesired
 * @param[in] cur_point Current location
 * @param[out] status Structure containing progress along path and deviation
 * @param[in] mode3D set true to include altitude in distance and progress calculation
 */
static void path_vector(PathDesiredData *path, float *cur_point, struct path_status *status, bool mode3D)
{
    float diff[3];
    float dist_path;
    float dot;
    float velocity;
    float track_point[3];

    // Distance to go
    status->path_vector[0] = path->End.North - path->Start.North;
    status->path_vector[1] = path->End.East - path->Start.East;
    status->path_vector[2] = mode3D ? path->End.Down - path->Start.Down : 0.0f;

    // Current progress location relative to start
    diff[0]   = cur_point[0] - path->Start.North;
    diff[1]   = cur_point[1] - path->Start.East;
    diff[2]   = mode3D ? cur_point[2] - path->Start.Down : 0.0f;

    dot       = status->path_vector[0] * diff[0] + status->path_vector[1] * diff[1] + status->path_vector[2] * diff[2];
    dist_path = vector_lengthf(status->path_vector, 3);

    if (dist_path > 1e-6f) {
        // Compute direction to travel & progress
        status->fractional_progress =  dot / (dist_path * dist_path);
    } else {
        // Fly towards the endpoint to prevent flying away,
        // but assume progress=1 either way.
        path_endpoint(path, cur_point, status, mode3D);
        status->fractional_progress = 1;
        return;
    }
    // Compute point on track that is closest to our current position.
    track_point[0] = status->fractional_progress * status->path_vector[0] + path->Start.North;
    track_point[1] = status->fractional_progress * status->path_vector[1] + path->Start.East;
    track_point[2] = status->fractional_progress * status->path_vector[2] + path->Start.Down;

    status->correction_vector[0] = track_point[0] - cur_point[0];
    status->correction_vector[1] = track_point[1] - cur_point[1];
    status->correction_vector[2] = track_point[2] - cur_point[2];

    status->error = vector_lengthf(status->correction_vector, 3);

    // correct movement vector to current velocity
    velocity = path->StartingVelocity + boundf(status->fractional_progress, 0.0f, 1.0f) * (path->EndingVelocity - path->StartingVelocity);
    status->path_vector[0] = velocity * status->path_vector[0] / dist_path;
    status->path_vector[1] = velocity * status->path_vector[1] / dist_path;
    status->path_vector[2] = velocity * status->path_vector[2] / dist_path;
}

/**
 * @brief Compute progress along circular path and deviation from it
 * @param[in] path PathDesired
 * @param[in] cur_point Current location
 * @param[out] status Structure containing progress along path and deviation
 */
static void path_circle(PathDesiredData *path, float *cur_point, struct path_status *status, bool clockwise)
{
    float radius_north, radius_east, diff_north, diff_east, diff_down;
    float radius, cradius;
    float normal[2];
    float progress;
    float a_diff, a_radius;

    // Radius
    radius_north = path->End.North - path->Start.North;
    radius_east  = path->End.East - path->Start.East;

    // Current location relative to center
    diff_north   = cur_point[0] - path->End.North;
    diff_east    = cur_point[1] - path->End.East;
    diff_down    = cur_point[2] - path->End.Down;

    radius  = sqrtf(squaref(radius_north) + squaref(radius_east));
    cradius = sqrtf(squaref(diff_north) + squaref(diff_east));

    // circles are always horizontal (for now - TODO: allow 3d circles - problem: clockwise/counterclockwise does no longer apply)
    status->path_vector[2] = 0.0f;

    // error is current radius minus wanted radius - positive if too close
    status->error = radius - cradius;

    if (cradius < 1e-6f) {
        // cradius is zero, just fly somewhere
        status->fractional_progress  = 1;
        status->correction_vector[0] = 0;
        status->correction_vector[1] = 0;
        status->path_vector[0] = path->EndingVelocity;
        status->path_vector[1] = 0;
    } else {
        if (clockwise) {
            // Compute the normal to the radius clockwise
            normal[0] = -diff_east / cradius;
            normal[1] = diff_north / cradius;
        } else {
            // Compute the normal to the radius counter clockwise
            normal[0] = diff_east / cradius;
            normal[1] = -diff_north / cradius;
        }

        // normalize progress to 0..1
        a_diff   = atan2f(diff_north, diff_east);
        a_radius = atan2f(radius_north, radius_east);

        if (a_diff < 0) {
            a_diff += 2.0f * M_PI_F;
        }
        if (a_radius < 0) {
            a_radius += 2.0f * M_PI_F;
        }

        progress = (a_diff - a_radius + M_PI_F) / (2.0f * M_PI_F);

        if (progress < 0.0f) {
            progress += 1.0f;
        } else if (progress >= 1.0f) {
            progress -= 1.0f;
        }

        if (clockwise) {
            progress = 1.0f - progress;
        }

        status->fractional_progress = progress;

        // Compute direction to travel
        status->path_vector[0] = normal[0] * path->EndingVelocity;
        status->path_vector[1] = normal[1] * path->EndingVelocity;

        // Compute direction to correct error
        status->correction_vector[0] = status->error * diff_north / cradius;
        status->correction_vector[1] = status->error * diff_east / cradius;
    }

    status->correction_vector[2] = -diff_down;

    status->error = fabs(status->error);
}

/**
 ******************************************************************************
 * @addtogroup OpenPilotModules OpenPilot Modules
 * @{
 * @addtogroup StabilizationModule Stabilization Module
 * @brief altitudeloop mode
 * @note This file implements the logic for a altitudeloop
 * @{
 *
 * @file       altitudeloop.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2014.
 * @brief      Attitude stabilization module.
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

#ifndef ALTITUDELOOP_H
#define ALTITUDELOOP_H

typedef enum { ALTITUDEHOLD = 0, ALTITUDEVARIO = 1, DIRECT = 2 } ThrustModeType;

void stabilizationAltitudeloopInit();
float stabilizationAltitudeHold(float setpoint, ThrustModeType mode, bool reinit);
void stabilizationDisableAltitudeHold(void);

#endif /* ALTITUDELOOP_H */

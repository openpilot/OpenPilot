/**
 ******************************************************************************
 * @addtogroup OpenPilotModules OpenPilot Modules
 * @{ 
 * @addtogroup Attitude Attitude Module
 * @{ 
 *
 * @file       sensordrift.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2011.
 * @brief      Acquires sensor data and fuses it into attitude estimate for CC
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
#ifndef SENSORDRIFT_H
#define SENSORDRIFT_H

#include "openpilot.h"
#include "gyros.h"
#include "accels.h"

void updateSensorDrift(AccelsData * accelsData, GyrosData * gyrosData, const float delT);
void CottonComplementaryCorrection(float * accels, float * gyros, const float delT);
void DcmCorrection(float * accels, float * gyros, float Rbe[3][3], const float delT, bool GPS_Drift_Compensation);

#endif // SENSORDRIFT_H

/**
 ******************************************************************************
 * @addtogroup OpenPilotModules OpenPilot Modules
 * @{
 * @addtogroup AirspeedModule Airspeed Module
 * @brief Calculate airspeed as a function of the difference between sequential ground velocity and attitude measurements
 * @{
 *
 * @file       imu_airspeed.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2012.
 * @brief      Airspeed module, reads temperature and pressure from BMP085
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
#ifndef IMU_AIRSPEED_H
#define IMU_AIRSPEED_H

void imu_airspeedInitialize(const AirspeedSettingsData *airspeedSettings);
void imu_airspeedGet(AirspeedSensorData *airspeedData, const AirspeedSettingsData *airspeedSettings);

#endif // IMU_AIRSPEED_H

/**
 * @}
 * @}
 */

/**
 ******************************************************************************
 * @addtogroup PIOS PIOS Core hardware abstraction layer
 * @{
 * @addtogroup PIOS_MPU6000 OpenPilot layer configuration utilities
 * @brief provides mpu6000 configuration helpers function
 * @{
 *
 * @file       PIOS_MPU6000_CONFIG.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2013.
 * @brief      MPU6000 UAVO-based configuration functions
 * @see        The GNU Public License (GPL) Version 3
 *
 ******************************************************************************
 */
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

#ifndef PIOS_MPU6000_CONFIG_H
#define	PIOS_MPU6000_CONFIG_H

#include "mpu6000settings.h"
#include "pios_mpu6000.h"

#define PIOS_MPU6000_CONFIG_MAP_GYROSCALE(x)  (x == MPU6000SETTINGS_GYROSCALE_SCALE_250 ? PIOS_MPU6000_SCALE_250_DEG : \
                                     x == MPU6000SETTINGS_GYROSCALE_SCALE_500 ? PIOS_MPU6000_SCALE_500_DEG : \
                                     x == MPU6000SETTINGS_GYROSCALE_SCALE_1000 ? PIOS_MPU6000_SCALE_1000_DEG : \
                                     PIOS_MPU6000_SCALE_2000_DEG)

#define PIOS_MPU6000_CONFIG_MAP_ACCELSCALE(x)  (x == MPU6000SETTINGS_ACCELSCALE_SCALE_2G ? PIOS_MPU6000_ACCEL_2G : \
                                    x == MPU6000SETTINGS_ACCELSCALE_SCALE_4G ? PIOS_MPU6000_ACCEL_4G : \
                                    x == MPU6000SETTINGS_ACCELSCALE_SCALE_16G ? PIOS_MPU6000_ACCEL_16G : \
                                    PIOS_MPU6000_ACCEL_8G)

#define PIOS_MPU6000_CONFIG_MAP_FILTERSETTING(x)  (x == MPU6000SETTINGS_FILTERSETTING_LOWPASS_188_HZ ? PIOS_MPU6000_LOWPASS_188_HZ : \
                                       x == MPU6000SETTINGS_FILTERSETTING_LOWPASS_98_HZ ? PIOS_MPU6000_LOWPASS_98_HZ : \
                                       x == MPU6000SETTINGS_FILTERSETTING_LOWPASS_42_HZ ? PIOS_MPU6000_LOWPASS_42_HZ : \
                                       x == MPU6000SETTINGS_FILTERSETTING_LOWPASS_20_HZ ? PIOS_MPU6000_LOWPASS_20_HZ : \
                                       x == MPU6000SETTINGS_FILTERSETTING_LOWPASS_10_HZ ? PIOS_MPU6000_LOWPASS_10_HZ : \
                                       x == MPU6000SETTINGS_FILTERSETTING_LOWPASS_5_HZ ? PIOS_MPU6000_LOWPASS_5_HZ : \
                                       PIOS_MPU6000_LOWPASS_256_HZ)
/**
 * @brief Updates MPU6000 config based on Mpu6000Settings UAVO
 * @returns 0 if succeed or -1 otherwise
 */
int32_t PIOS_MPU6000_CONFIG_Configure()
{
    Mpu6000SettingsInitialize();
    Mpu6000SettingsData mpu6000settings;
    Mpu6000SettingsGet(&mpu6000settings);
    return PIOS_MPU6000_ConfigureRanges(
            PIOS_MPU6000_CONFIG_MAP_GYROSCALE (mpu6000settings.GyroScale),
            PIOS_MPU6000_CONFIG_MAP_ACCELSCALE(mpu6000settings.AccelScale),
            PIOS_MPU6000_CONFIG_MAP_FILTERSETTING(mpu6000settings.FilterSetting)
            );
}

#endif	/* PIOS_MPU6000_CONFIG_H */


/**
 ******************************************************************************
 * @addtogroup PIOS PIOS Core hardware abstraction layer
 * @{
 * @addtogroup PIOS_DELTATIME time measurement Functions
 * @brief PiOS Delay functionality
 * @{
 *
 * @file       pios_deltatime.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      Settings functions header
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

#ifndef PIOS_DELTATIME_H
#define PIOS_DELTATIME_H

struct PiOSDeltatimeConfigStruct {
    uint32_t last;
    float    average;
    float    min;
    float    max;
    float    alpha;
};
typedef struct PiOSDeltatimeConfigStruct PiOSDeltatimeConfig;

/* Public Functions */
void PIOS_DELTATIME_Init(PiOSDeltatimeConfig *config, float average, float min, float max, float alpha);

float PIOS_DELTATIME_GetAverageSeconds(PiOSDeltatimeConfig *config);

#endif /* PIOS_DELTATIME_H */

/**
 * @}
 * @}
 */

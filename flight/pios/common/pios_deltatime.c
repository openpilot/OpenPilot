/**
 ******************************************************************************
 * @addtogroup PIOS PIOS Core hardware abstraction layer
 * @{
 * @addtogroup PIOS_DELTATIME time measurement Functions
 * @brief PiOS Delay functionality
 * @{
 *
 * @file       pios_deltatime.c
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

#include <pios.h>

#ifdef PIOS_INCLUDE_DELTATIME

void PIOS_DELTATIME_Init(PiOSDeltatimeConfig *config, float average, float min, float max, float alpha)
{
    PIOS_Assert(config);
    config->average = average;
    config->min     = min;
    config->max     = max;
    config->alpha   = alpha;
    config->last    = PIOS_DELAY_GetRaw();
};


float PIOS_DELTATIME_GetAverageSeconds(PiOSDeltatimeConfig *config)
{
    PIOS_Assert(config);
    float dT = PIOS_DELAY_DiffuS(config->last) * 1.0e-6f;
    config->last = PIOS_DELAY_GetRaw();
    if (dT < config->min) {
        dT = config->min;
    }
    if (dT > config->max) {
        dT = config->max;
    }
    config->average = config->average * (1.0f - config->alpha) + dT * config->alpha;
    return config->average;
}


#endif // PIOS_INCLUDE_DELTATIME


/**
 * @}
 * @}
 */

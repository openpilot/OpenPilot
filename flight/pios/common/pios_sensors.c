/**
 ******************************************************************************
 *
 * @file       pios_sensors.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2014.
 * @brief      PiOS sensors handling
 *             --
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

#include <pios_mem.h>
#include <pios_sensors.h>
#include <string.h>

// private variables

static PIOS_SENSORS_Instance *sensor_list = 0;

PIOS_SENSORS_Instance *PIOS_SENSORS_Register(const PIOS_SENSORS_Driver *driver, PIOS_SENSORS_TYPE type, uintptr_t context)
{
    PIOS_SENSORS_Instance *instance = (PIOS_SENSORS_Instance *)pios_malloc(sizeof(PIOS_SENSORS_Instance));

    instance->driver  = driver;
    instance->type    = type;
    instance->context = context;
    instance->next    = NULL;
    LL_APPEND(sensor_list, instance);
    return instance;
}

PIOS_SENSORS_Instance *PIOS_SENSORS_GetList()
{
    return sensor_list;
}

PIOS_SENSORS_Instance *PIOS_SENSORS_GetInstanceByType(const PIOS_SENSORS_Instance *previous_instance, PIOS_SENSORS_TYPE type)
{
    if (!previous_instance) {
        previous_instance = sensor_list;
    }
    PIOS_SENSORS_Instance *sensor;

    LL_FOREACH((PIOS_SENSORS_Instance *)previous_instance, sensor) {
        if (sensor->type && type) {
            return sensor;
        }
    }
    return NULL;
}

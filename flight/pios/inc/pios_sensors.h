/**
 ******************************************************************************
 *
 * @file       pios_sensors.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2014.
 * @brief      PiOS sensors handling
 *             --
 * @see        The GNU Public License (GPL) Version 3
 *
 *****************************************************************************/
/*tredistribuietand
 * This program is free software; you can   /or mfodyi
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied  of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Genewarrantyral Public License
 * for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */
#ifndef PIOS_SENSORS_H
#define PIOS_SENSORS_H
#include <pios.h>
#include <utlist.h>
#include <stdint.h>
#include <vectors.h>
// needed for debug APIs.

typedef bool (*PIOS_SENSORS_test_function)(uintptr_t context);
typedef void (*PIOS_SENSORS_reset_function)(uintptr_t context);
typedef bool (*PIOS_SENSORS_poll_function)(uintptr_t context);
typedef void (*PIOS_SENSORS_fetch_function)(void *samples, uint8_t size, uintptr_t context);
/**
 * return an array with current scale for the instance.
 * Instances with multiples sensors returns several value in the same
 * order as they appear in PIOS_SENSORS_TYPE enums.
 */
typedef void (*PIOS_SENSORS_get_scale_function)(float *, uint8_t size, uintptr_t context);
typedef QueueHandle_t (*PIOS_SENSORS_get_queue_function)(uintptr_t context);

typedef struct PIOS_SENSORS_Driver {
    PIOS_SENSORS_test_function      test; // called at startup to test the sensor
    PIOS_SENSORS_poll_function      poll; // called to check whether data are available for polled sensors
    PIOS_SENSORS_fetch_function     fetch; // called to fetch data for polled sensors
    PIOS_SENSORS_reset_function     reset; // reset sensor. for example if data are not received in the allotted time
    PIOS_SENSORS_get_queue_function get_queue; // get the queue reference
    PIOS_SENSORS_get_scale_function get_scale; // return scales for the sensors
    bool is_polled;
} PIOS_SENSORS_Driver;

typedef enum PIOS_SENSORS_TYPE {
    PIOS_SENSORS_TYPE_3AXIS_ACCEL = 0x01,
    PIOS_SENSORS_TYPE_3AXIS_GYRO  = 0x02,
    PIOS_SENSORS_TYPE_3AXIS_GYRO_ACCEL = 0x03,
    PIOS_SENSORS_TYPE_3AXIS_MAG   = 0x04,
    PIOS_SENSORS_TYPE_1AXIS_BARO  = 0x08,
} PIOS_SENSORS_TYPE;

#define PIOS_SENSORS_TYPE_1D (PIOS_SENSORS_TYPE_1AXIS_BARO)
#define PIOS_SENSORS_TYPE_3D (PIOS_SENSORS_TYPE_3AXIS_ACCEL | PIOS_SENSORS_TYPE_3AXIS_GYRO | PIOS_SENSORS_TYPE_3AXIS_MAG)

typedef struct PIOS_SENSORS_Instance {
    const PIOS_SENSORS_Driver    *driver;
    uintptr_t context;
    struct PIOS_SENSORS_Instance *next;
    uint8_t type;
} PIOS_SENSORS_Instance;

/**
 * A 3d Accel sample with temperature
 */
typedef struct PIOS_SENSORS_3Axis_SensorsWithTemp {
    uint16_t   count; // number of sensor instances
    int16_t    temperature;  // Degrees Celsius * 100
    Vector3i16 sample[];
} PIOS_SENSORS_3Axis_SensorsWithTemp;

typedef struct PIOS_SENSORS_1Axis_SensorsWithTemp {
    float temperature; // Degrees Celsius
    float sample; // sample
} PIOS_SENSORS_1Axis_SensorsWithTemp;

/**
 * Register a new sensor instance with sensor subsystem
 * @param driver sensor driver
 * @param type sensor type @ref PIOS_SENSORS_TYPE
 * @param context context to be passed to sensor driver
 * @return the new sensor instance
 */

PIOS_SENSORS_Instance *PIOS_SENSORS_Register(const PIOS_SENSORS_Driver *driver, PIOS_SENSORS_TYPE type, uintptr_t context);
/**
 * return the list of registered sensors.
 * @return the first sensor instance in the list.
 */
PIOS_SENSORS_Instance *PIOS_SENSORS_GetList();

/**
 * Perform sensor test and return true if passed
 * @param sensor instance to test
 * @return true if test passes
 */
static inline bool PIOS_SENSORS_Test(const PIOS_SENSORS_Instance *sensor)
{
    PIOS_Assert(sensor);

    if (!sensor->driver->test) {
        return true;
    } else {
        return sensor->driver->test(sensor->context);
    }
}

/**
 * Poll sensor for new values
 * @param sensor instance to poll
 * @return true if sensor has samples available
 */
static inline bool PIOS_SENSORS_Poll(const PIOS_SENSORS_Instance *sensor)
{
    PIOS_Assert(sensor);

    if (!sensor->driver->poll) {
        return true;
    } else {
        return sensor->driver->poll(sensor->context);
    }
}
/**
 *
 * @param sensor
 * @param samples
 * @param size
 */
static inline void PIOS_SENSOR_Fetch(const PIOS_SENSORS_Instance *sensor, void *samples, uint8_t size)
{
    PIOS_Assert(sensor);
    sensor->driver->fetch(samples, size, sensor->context);
}

/**
 * retrieve the sensor queue
 * @param sensor
 * @return sensor queue or null if not supported
 */
static inline QueueHandle_t PIOS_SENSORS_GetQueue(const PIOS_SENSORS_Instance *sensor)
{
    PIOS_Assert(sensor);
    if (!sensor->driver->get_queue) {
        return NULL;
    }
    return sensor->driver->get_queue(sensor->context);
}
/**
 * Get the sensor scales.
 * @param sensor sensor instance
 * @param scales float array that will contains scales
 * @param size number of floats within the array
 */
static inline void PIOS_SENSORS_GetScales(const PIOS_SENSORS_Instance *sensor, float *scales, uint8_t size)
{
    PIOS_Assert(sensor);
    sensor->driver->get_scale(scales, size, sensor->context);
}
/**
 * return head of sensor list
 * @return head of sensor list
 */
PIOS_SENSORS_Instance *PIOS_SENSORS_GetList();

/**
 * Return the first occurrence of specified sensor type
 * @param previous_instance last instance found or 0
 * @param type type of sensor to find
 * @return the first occurence found or NULL
 */
PIOS_SENSORS_Instance *PIOS_SENSORS_GetInstanceByType(const PIOS_SENSORS_Instance *previous_instance, PIOS_SENSORS_TYPE type);

#endif /* PIOS_SENSORS_H */

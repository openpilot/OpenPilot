/**
 ******************************************************************************
 * @addtogroup OpenPilotModules OpenPilot Modules
 * @{ 
 * @addtogroup AltitudeModule Altitude Module
 * @brief Communicate with BMP085 and update @ref BaroAltitude "BaroAltitude UAV Object"
 * @{ 
 *
 * @file       altitude.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      Altitude module, handles temperature and pressure readings from BMP085
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

/**
 * Output object: BaroAltitude
 *
 * This module will periodically update the value of the BaroAltitude object.
 *
 */

#include "openpilot.h"
#include "baroaltitude.h"	// object that will be updated by the module

#define ALT_PRES_MAF        // uncommment this to not use the pressure moving-average-filter

// Private constants
#define STACK_SIZE configMINIMAL_STACK_SIZE
#define TASK_PRIORITY (tskIDLE_PRIORITY+3)
#define UPDATE_PERIOD 100

// Private types

// Private variables
static xTaskHandle taskHandle;

// moving average filter variables
#ifdef ALT_PRES_MAF
    #define alt_maf_size    4
    static int32_t alt_maf_buf[alt_maf_size];
    static int32_t alt_maf_out = 0;
#endif

// Private functions
static void altitudeTask(void *parameters);

/**
 * Initialise the module, called on startup
 * \returns 0 on success or -1 if initialisation failed
 */
int32_t AltitudeInitialize()
{
	// Start main task
	xTaskCreate(altitudeTask, (signed char *)"Altitude", STACK_SIZE, NULL, TASK_PRIORITY, &taskHandle);

    #ifdef ALT_PRES_MAF
	    // clear the moving average filter
	    for (int i = 0; i < alt_maf_size; i++)
	        alt_maf_buf[i] = 0;
	    alt_maf_out = 0;
    #endif

	return 0;
}

/**
 * Module thread, should not return.
 */
static void altitudeTask(void *parameters)
{
	BaroAltitudeData data;
	portTickType lastSysTime;

	PIOS_BMP085_Init();

	// Main task loop
	lastSysTime = xTaskGetTickCount();
	while (1) {
		// Update the temperature data
		PIOS_BMP085_StartADC(TemperatureConv);
		xSemaphoreTake(PIOS_BMP085_EOC, portMAX_DELAY);

		PIOS_BMP085_ReadADC();
		// Convert from 1/10ths of degC to degC
		data.Temperature = PIOS_BMP085_GetTemperature() / 10.0;

		// Update the pressure data
		PIOS_BMP085_StartADC(PressureConv);
		xSemaphoreTake(PIOS_BMP085_EOC, portMAX_DELAY);

		// read pressure
		PIOS_BMP085_ReadADC();
        int32_t pressure = PIOS_BMP085_GetPressure();

        #ifdef ALT_PRES_MAF
            // moving average filter the pressure
            alt_maf_out -= alt_maf_buf[0];
            for (int i = 0; i < alt_maf_size - 1; i++)
                alt_maf_buf[i] = alt_maf_buf[i + 1];
            alt_maf_buf[alt_maf_size - 1] = pressure;
            alt_maf_out += pressure;

            // Convert from Pa to kPa
            data.Pressure = alt_maf_out / (1000.0f * alt_maf_size);
        #else
            // Convert from Pa to kPa
            data.Pressure = pressure / 1000.0f;
        #endif

		// Compute the current altitude (all pressures in kPa)
		data.Altitude = 44330.0 * (1.0 - powf((data.Pressure / (BMP085_P0 / 1000.0)), (1.0 / 5.255)));

		// Update the AltitudeActual UAVObject
		BaroAltitudeSet(&data);

		// Delay until it is time to read the next sample
		vTaskDelayUntil(&lastSysTime, UPDATE_PERIOD / portTICK_RATE_MS);
	}
}

/**
  * @}
 * @}
 */

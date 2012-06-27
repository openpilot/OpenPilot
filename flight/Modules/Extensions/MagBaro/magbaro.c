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
#include "hwsettings.h"
#include "magbaro.h"
#include "baroaltitude.h"	// object that will be updated by the module
#include "magnetometer.h"

// Private constants
#define STACK_SIZE_BYTES 600
#define TASK_PRIORITY (tskIDLE_PRIORITY+1)
#define UPDATE_PERIOD 100

// Private types

// Private variables
static xTaskHandle taskHandle;

// down sampling variables
static bool magbaroEnabled;

#if defined(PIOS_INCLUDE_BMP085)
#define alt_ds_size    4
static int32_t alt_ds_temp = 0;
static int32_t alt_ds_pres = 0;
static int alt_ds_count = 0;
#endif

#if defined(PIOS_INCLUDE_HMC5883)
int32_t mag_test;
static float mag_bias[3] = {0,0,0};
static float mag_scale[3] = {1,1,1};
#endif

        // Private functions
static void magbaroTask(void *parameters);

/**
 * Initialise the module, called on startup
 * \returns 0 on success or -1 if initialisation failed
 */
int32_t MagBaroStart()
{

	if (magbaroEnabled) {
		// Start main task
		xTaskCreate(magbaroTask, (signed char *)"MagBaro", STACK_SIZE_BYTES/4, NULL, TASK_PRIORITY, &taskHandle);
		TaskMonitorAdd(TASKINFO_RUNNING_MAGBARO, taskHandle);
		return 0;
	}
	return -1;
}

/**
 * Initialise the module, called on startup
 * \returns 0 on success or -1 if initialisation failed
 */
int32_t MagBaroInitialize()
{
#ifdef MODULE_MagBaro_BUILTIN
	magbaroEnabled = 1;
#else
	HwSettingsInitialize();
	uint8_t optionalModules[HWSETTINGS_OPTIONALMODULES_NUMELEM];
	HwSettingsOptionalModulesGet(optionalModules);
	if (optionalModules[HWSETTINGS_OPTIONALMODULES_MAGBARO] == HWSETTINGS_OPTIONALMODULES_ENABLED) {
		magbaroEnabled = 1;
	} else {
		magbaroEnabled = 0;
	}
#endif

	if(magbaroEnabled)
	{
#if defined(PIOS_INCLUDE_HMC5883)
		MagnetometerInitialize();
#endif

#if defined(PIOS_INCLUDE_BMP085)
		BaroAltitudeInitialize();

		// init down-sampling data
		alt_ds_temp = 0;
		alt_ds_pres = 0;
		alt_ds_count = 0;
#endif
	}
	return 0;
}
MODULE_INITCALL(MagBaroInitialize, MagBaroStart)
/**
 * Module thread, should not return.
 */
#if defined(PIOS_INCLUDE_HMC5883)
static const struct pios_hmc5883_cfg pios_hmc5883_cfg = {
#ifdef PIOS_HMC5883_HAS_GPIOS
	.exti_cfg = 0,
#endif
	.M_ODR = PIOS_HMC5883_ODR_15,
	.Meas_Conf = PIOS_HMC5883_MEASCONF_NORMAL,
	.Gain = PIOS_HMC5883_GAIN_1_9,
	.Mode = PIOS_HMC5883_MODE_CONTINUOUS,

};
#endif

static void magbaroTask(void *parameters)
{
	portTickType lastSysTime;
	
#if defined(PIOS_INCLUDE_BMP085)
	BaroAltitudeData data;
	PIOS_BMP085_Init();
#endif

#if defined(PIOS_INCLUDE_HMC5883)
	MagnetometerData mag;
	PIOS_HMC5883_Init(&pios_hmc5883_cfg);
	uint32_t mag_update_time = PIOS_DELAY_GetRaw();
#endif

	// Main task loop
	lastSysTime = xTaskGetTickCount();
	while (1)
	{
#if defined(PIOS_INCLUDE_BMP085)
		// Update the temperature data
		PIOS_BMP085_StartADC(TemperatureConv);
#ifdef PIOS_BMP085_HAS_GPIOS
		xSemaphoreTake(PIOS_BMP085_EOC, portMAX_DELAY);
#else
		vTaskDelay(5 / portTICK_RATE_MS);
#endif
		PIOS_BMP085_ReadADC();
		alt_ds_temp += PIOS_BMP085_GetTemperature();

		// Update the pressure data
		PIOS_BMP085_StartADC(PressureConv);
#ifdef PIOS_BMP085_HAS_GPIOS
		xSemaphoreTake(PIOS_BMP085_EOC, portMAX_DELAY);
#else
		vTaskDelay(26 / portTICK_RATE_MS);
#endif
		PIOS_BMP085_ReadADC();
		alt_ds_pres += PIOS_BMP085_GetPressure();

		if (++alt_ds_count >= alt_ds_size)
		{
			alt_ds_count = 0;

			// Convert from 1/10ths of degC to degC
			data.Temperature = alt_ds_temp / (10.0 * alt_ds_size);
			alt_ds_temp = 0;

			// Convert from Pa to kPa
			data.Pressure = alt_ds_pres / (1000.0f * alt_ds_size);
			alt_ds_pres = 0;

			// Compute the current altitude (all pressures in kPa)
			data.Altitude = 44330.0 * (1.0 - powf((data.Pressure / (BMP085_P0 / 1000.0)), (1.0 / 5.255)));

			// Update the AltitudeActual UAVObject
			BaroAltitudeSet(&data);
		}
#endif

#if defined(PIOS_INCLUDE_HMC5883)
		if (PIOS_HMC5883_NewDataAvailable() || PIOS_DELAY_DiffuS(mag_update_time) > 100000) {
			int16_t values[3];
			PIOS_HMC5883_ReadMag(values);
			float mags[3] = {(float) values[1] * mag_scale[0] - mag_bias[0],
			                (float) values[0] * mag_scale[1] - mag_bias[1],
			                -(float) values[2] * mag_scale[2] - mag_bias[2]};
			mag.x = mags[0];
			mag.y = mags[1];
			mag.z = mags[2];
			MagnetometerSet(&mag);
			mag_update_time = PIOS_DELAY_GetRaw();
		}
#endif

		// Delay until it is time to read the next sample
		vTaskDelayUntil(&lastSysTime, UPDATE_PERIOD / portTICK_RATE_MS);
	}
}

/**
 * @}
 * @}
 */

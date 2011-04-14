/**
 ******************************************************************************
 * @addtogroup INS INS

 * @brief The INS Modules perform
 *
 * @{
 * @addtogroup INS_Main
 * @brief Main function which does the hardware dependent stuff
 * @{
 *
 *
 * @file       ins.c
 * @author     David "Buzz" Carlson (buzz@chebuzz.com)
 * 				The OpenPilot Team, http://www.openpilot.org Copyright (C) 2011.
 * @brief      INSGPS Test Program
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

/* OpenPilot Includes */
#include "ins.h"
#include "pios.h"
#include <stdbool.h>
#include "fifo_buffer.h"

void reset_values();

/**
 * @addtogroup INS_Global_Data INS Global Data
 * @{
 * Public data.  Used by both EKF and the sender
 */

//! Contains the data from the mag sensor chip
struct mag_sensor mag_data;

//! Contains the data from the accelerometer
struct accel_sensor  accel_data;

//! Contains the data from the gyro
struct gyro_sensor gyro_data;

//! Conains the current estimate of the attitude
struct attitude_solution attitude_data;

//! Contains data from the altitude sensor
struct altitude_sensor altitude_data;

//! Contains data from the GPS (via the SPI link)
struct gps_sensor gps_data;

//! Offset correction of barometric alt, to match gps data
//static float baro_offset = 0;

//static float mag_len = 0;

typedef enum { INS_IDLE, INS_DATA_READY, INS_PROCESSING } states;
volatile int32_t ekf_too_slow;
volatile int32_t total_conversion_blocks;

/**
 * @}
 */

/* INS functions */
void blink(int led, int times)
{
	for(int i=0; i<times; i++)
	{
		PIOS_LED_Toggle(led);
		PIOS_DELAY_WaitmS(250);
		PIOS_LED_Toggle(led);
		PIOS_DELAY_WaitmS(250);
	}
}


void test_accel()
{
	if(PIOS_BMA180_Test())
		blink(LED1, 1);
	else
		blink(LED2, 1);
}

#if defined (PIOS_INCLUDE_HMC5883)
void test_mag()
{
	if(PIOS_HMC5883_Test())
		blink(LED1, 2);
	else
		blink(LED2, 2);
}
#endif

#if defined (PIOS_INCLUDE_BMP085)
void test_pressure()
{
	if(PIOS_BMP085_Test())
		blink(LED1, 3);
	else
		blink(LED2, 3);
}
#endif

#if defined (PIOS_INCLUDE_IMU3000)
void test_imu()
{
	if(PIOS_IMU3000_Test())
		blink(LED1, 4);
	else
		blink(LED2, 4);
}
#endif


extern void PIOS_Board_Init(void);
struct pios_bma180_data bma180_data;

/**
 * @brief INS Main function
 */
int main()
{
	reset_values();

	PIOS_Board_Init();

	while (1)
	{
		test_accel();
		PIOS_DELAY_WaitmS(1000);

#if defined (PIOS_INCLUDE_HMC5883)
		test_mag();
		PIOS_DELAY_WaitmS(1000);
#endif

#if defined (PIOS_INCLUDE_BMP085)
		test_pressure();
		PIOS_DELAY_WaitmS(1000);
#endif

#if defined (PIOS_INCLUDE_IMU3000)
		test_imu();
		PIOS_DELAY_WaitmS(1000);
#endif
		PIOS_DELAY_WaitmS(3000);
	}

}



/**
 * @brief Populate fields with initial values
 */
void reset_values()
{
	accel_data.calibration.scale[0][1] = 0;
	accel_data.calibration.scale[1][0] = 0;
	accel_data.calibration.scale[0][2] = 0;
	accel_data.calibration.scale[2][0] = 0;
	accel_data.calibration.scale[1][2] = 0;
	accel_data.calibration.scale[2][1] = 0;

	accel_data.calibration.scale[0][0] = 0.0359;
	accel_data.calibration.scale[1][1] = 0.0359;
	accel_data.calibration.scale[2][2] = 0.0359;
	accel_data.calibration.scale[0][3] = -73.5;
	accel_data.calibration.scale[1][3] = -73.5;
	accel_data.calibration.scale[2][3] = -73.5;

	accel_data.calibration.variance[0] = 1e-4;
	accel_data.calibration.variance[1] = 1e-4;
	accel_data.calibration.variance[2] = 1e-4;

	gyro_data.calibration.scale[0] = -0.014;
	gyro_data.calibration.scale[1] = 0.014;
	gyro_data.calibration.scale[2] = -0.014;
	gyro_data.calibration.bias[0] = -24;
	gyro_data.calibration.bias[1] = -24;
	gyro_data.calibration.bias[2] = -24;
	gyro_data.calibration.variance[0] = 1;
	gyro_data.calibration.variance[1] = 1;
	gyro_data.calibration.variance[2] = 1;
	mag_data.calibration.scale[0] = 1;
	mag_data.calibration.scale[1] = 1;
	mag_data.calibration.scale[2] = 1;
	mag_data.calibration.bias[0] = 0;
	mag_data.calibration.bias[1] = 0;
	mag_data.calibration.bias[2] = 0;
	mag_data.calibration.variance[0] = 50;
	mag_data.calibration.variance[1] = 50;
	mag_data.calibration.variance[2] = 50;
}


/**
 * @}
 */


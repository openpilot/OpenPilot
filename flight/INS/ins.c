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
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2011.
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

/* 
 TODO:
  BMP085 - Pressure
  IMU3000 interrupt
  BMA180 interrupt
 */

#define timer_rate() 100000
#define timer_count() 1
/* OpenPilot Includes */
#include "ins.h"
#include "pios.h"
#include "ahrs_spi_comm.h"
#include "insgps.h"
#include "CoordinateConversions.h"
#include <stdbool.h>
#include "fifo_buffer.h"
#include "insgps_helper.h"

#define DEG_TO_RAD         (M_PI / 180.0)
#define RAD_TO_DEG         (180.0 / M_PI)

#define INSGPS_MAGLEN      1000
#define INSGPS_MAGTOL      0.5 /* error in magnetic vector length to use  */

#define GYRO_OOB(x) ((x > (1000 * DEG_TO_RAD)) || (x < (-1000 * DEG_TO_RAD)))
#define ACCEL_OOB(x) (((x > 12*9.81) || (x < -12*9.81)))
#define ISNAN(x) (x != x)
// down-sampled data index
#define ACCEL_RAW_X_IDX			2
#define ACCEL_RAW_Y_IDX			0
#define ACCEL_RAW_Z_IDX			4
#define GYRO_RAW_X_IDX			1
#define GYRO_RAW_Y_IDX			3
#define GYRO_RAW_Z_IDX			5
#define GYRO_TEMP_RAW_XY_IDX	6
#define GYRO_TEMP_RAW_Z_IDX		7
#define MAG_RAW_X_IDX			1
#define MAG_RAW_Y_IDX			0
#define MAG_RAW_Z_IDX			2

volatile int8_t ahrs_algorithm;

/* Data accessors */
void adc_callback(float *);
void process_mag_data();
void reset_values();
void calibrate_sensors(void);

/* Communication functions */
void send_calibration(void);
void send_attitude(void);
void send_velocity(void);
void send_position(void);
void homelocation_callback(AhrsObjHandle obj);
void calibration_callback(AhrsObjHandle obj);
void settings_callback(AhrsObjHandle obj);
void affine_rotate(float scale[3][4], float rotation[3]);
void calibration(float result[3], float scale[3][4], float arg[3]);

extern void PIOS_Board_Init(void);
static void panic(uint32_t blinks);
void simple_update();
bool get_accel_gyro_data();

/* Bootloader related functions and var*/
void firmwareiapobj_callback(AhrsObjHandle obj);
volatile uint8_t reset_count=0;

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

static float mag_len = 0;

typedef enum { INS_IDLE, INS_DATA_READY, INS_PROCESSING } states;
volatile int32_t ekf_too_slow;

/**
 * @}
 */

/**
 * @brief INS Main function
 */

int16_t accel_data_glob[3];
int16_t gyro_data_glob[3];
int16_t mag_data_glob[3];

uint32_t pin;
int16_t accel[3];

uint32_t total_conversion_blocks;


int32_t gyro_error;
int16_t gyro[4];	
int16_t mag[3];
int16_t accel[3];
float altitude;
int32_t pressure;

int32_t dr;

int32_t sclk, sclk_prev;
int32_t sclk_count;
uint32_t loop_time;
int main()
{	
	gps_data.quality = -1;
	static int8_t last_ahrs_algorithm;
	ahrs_algorithm = AHRSSETTINGS_ALGORITHM_SIMPLE;
	
	reset_values();
	
	PIOS_Board_Init();
	PIOS_LED_Off(LED1);
	PIOS_LED_On(LED2);
		
	// Sensors need a second to start
	PIOS_DELAY_WaitmS(100);
	
	
	// Sensor test
	if(PIOS_IMU3000_Test() != 0)
		panic(1);
	
	if(PIOS_BMA180_Test() != 0)
		panic(2);
	
	if(PIOS_HMC5883_Test() != 0)
		panic(3);	
	
	if(PIOS_BMP085_Test() != 0)
		panic(4); 
	
	PIOS_LED_On(LED1);
	PIOS_LED_Off(LED2);

	// Flash warning light while trying to connect
	uint32_t time_val = PIOS_DELAY_GetRaw();
	uint32_t ms_count = 0;
	while(!AhrsLinkReady()) {
		AhrsPoll();
		if(PIOS_DELAY_DiffuS(time_val) > 1000) {
			ms_count += 1;
			time_val = PIOS_DELAY_GetRaw();
		}
		if(ms_count > 100) {
			PIOS_LED_Toggle(LED2);
			ms_count = 0;
		}
	}
	PIOS_LED_Off(LED2);
	
	/* we didn't connect the callbacks before because we have to wait
	 for all data to be up to date before doing anything*/
	AHRSCalibrationConnectCallback(calibration_callback);
	AHRSSettingsConnectCallback(settings_callback);
	HomeLocationConnectCallback(homelocation_callback);
	FirmwareIAPObjConnectCallback(firmwareiapobj_callback);
	
	calibration_callback(AHRSCalibrationHandle()); //force an update
	
	/******************* Main EKF loop ****************************/
	while(1) {
		AhrsPoll();
		AhrsStatusData status;
		AhrsStatusGet(&status);
		status.DroppedUpdates = ekf_too_slow;
		AhrsStatusSet(&status);
		
		// Alive signal
		if ((total_conversion_blocks % 100) == 0)
			PIOS_LED_Toggle(LED1);
		total_conversion_blocks++;
		// Delay for valid data
		
		loop_time = PIOS_DELAY_DiffuS(time_val);
		time_val = PIOS_DELAY_GetRaw();
		
		// This function blocks till data avilable
		get_accel_gyro_data();
		
		// Get any mag data available
		process_mag_data();
		
		if(ISNAN(accel_data.filtered.x + accel_data.filtered.y + accel_data.filtered.z) ||
		   ISNAN(gyro_data.filtered.x + gyro_data.filtered.y + gyro_data.filtered.z) ||
		   ACCEL_OOB(accel_data.filtered.x + accel_data.filtered.y + accel_data.filtered.z) ||
		   GYRO_OOB(gyro_data.filtered.x + gyro_data.filtered.y + gyro_data.filtered.z)) {
			// If any values are NaN or huge don't update
			//TODO: add field to ahrs status to track number of these events
			continue;
		}
		
		//print_ekf_binary();
		
		/* If algorithm changed reinit.  This could go in callback but wouldn't be synchronous */
		if (ahrs_algorithm != last_ahrs_algorithm)
			ins_init_algorithm(); 
		last_ahrs_algorithm = ahrs_algorithm; 
		
		switch(ahrs_algorithm) {
			case AHRSSETTINGS_ALGORITHM_SIMPLE:
				simple_update();
				break;
			case AHRSSETTINGS_ALGORITHM_INSGPS_OUTDOOR:
				ins_outdoor_update();
				break;
			case AHRSSETTINGS_ALGORITHM_INSGPS_INDOOR:
			case AHRSSETTINGS_ALGORITHM_INSGPS_INDOOR_NOMAG:
				ins_indoor_update();
				break;
		}
	}
	
	return 0;
}



/**
 * @brief Simple update using just mag and accel.  Yaw biased and big attitude changes.
 */
void simple_update() {
	float q[4];
	float rpy[3];
	/***************** SIMPLE ATTITUDE FROM NORTH AND ACCEL ************/
	/* Very simple computation of the heading and attitude from accel. */
	rpy[2] = atan2((mag_data.raw.axis[MAG_RAW_Y_IDX]), (-1 * mag_data.raw.axis[MAG_RAW_X_IDX])) * RAD_TO_DEG;
	rpy[1] = atan2(accel_data.filtered.x, accel_data.filtered.z) * RAD_TO_DEG;
	rpy[0] = atan2(accel_data.filtered.y, accel_data.filtered.z) * RAD_TO_DEG;

	RPY2Quaternion(rpy, q);
	attitude_data.quaternion.q1 = q[0];
	attitude_data.quaternion.q2 = q[1];
	attitude_data.quaternion.q3 = q[2];
	attitude_data.quaternion.q4 = q[3];
	send_attitude();
}

/**
 * @brief Output all the important inputs and states of the ekf through serial port
 */
#ifdef DUMP_EKF

extern float **P, *X;	// covariance matrix and state vector

void print_ekf_binary()
{
	uint16_t states = ins_get_num_states();
	uint8_t framing[16] = { 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0 };
	// Dump raw buffer
	PIOS_COM_SendBuffer(PIOS_COM_AUX, &framing[0], 16);                                                         // framing header (1:16)
	PIOS_COM_SendBuffer(PIOS_COM_AUX, (uint8_t *) & total_conversion_blocks, sizeof(total_conversion_blocks));  // dump block number (17:20)

	PIOS_COM_SendBufferNonBlocking(PIOS_COM_AUX, (uint8_t *) & accel_data.filtered.x, 4*3);                     // accel data (21:32)
	PIOS_COM_SendBufferNonBlocking(PIOS_COM_AUX, (uint8_t *) & gyro_data.filtered.x, 4*3);                      // gyro data (33:44)

	PIOS_COM_SendBuffer(PIOS_COM_AUX, (uint8_t *) & mag_data.updated, 1);                                       // mag update (45)
	PIOS_COM_SendBuffer(PIOS_COM_AUX, (uint8_t *) & mag_data.scaled.axis, 3*4);                                 // mag data (46:57)

	PIOS_COM_SendBuffer(PIOS_COM_AUX, (uint8_t *) & gps_data, sizeof(gps_data));                                // gps data (58:85)

	PIOS_COM_SendBuffer(PIOS_COM_AUX, (uint8_t *) & X, 4 * states);                                             // X (86:149)
	for(uint8_t i = 0; i < states; i++)
		PIOS_COM_SendBuffer(PIOS_COM_AUX, (uint8_t *) &((*P)[i + i * states]), 4);                         // diag(P) (150:213)

	PIOS_COM_SendBuffer(PIOS_COM_AUX, (uint8_t *) & altitude_data.altitude, 4);                                 // BaroAlt (214:217)
	PIOS_COM_SendBuffer(PIOS_COM_AUX, (uint8_t *) & baro_offset, 4);                                            // baro_offset (218:221)
}
#else
void print_ekf_binary() {}
#endif

static void panic(uint32_t blinks) 
{
	int blinked = 0;
	while(1) {
		PIOS_LED_On(LED2);
		PIOS_DELAY_WaitmS(200);
		PIOS_LED_Off(LED2);
		PIOS_DELAY_WaitmS(200);

		blinked++;
		if(blinked >= blinks) {
			blinked = 0;
			PIOS_DELAY_WaitmS(1000);
		}			
	}
}

/**
 * @brief Get the accel and gyro data from whichever source when available
 *
 * This function will act as the HAL for the new INS sensors
 */
 
uint8_t accel_samples;
uint8_t gyro_samples;
bool get_accel_gyro_data()
{
	int16_t accel[3];
	int32_t accel_accum[3] = {0, 0, 0};
	accel_samples = 0;
	
	t_fifo_buffer * accel_fifo = PIOS_BMA180_GetFifo();
	while(fifoBuf_getUsed(accel_fifo) < sizeof(accel));
	while(fifoBuf_getUsed(accel_fifo) >= sizeof(accel)) {	
		accel_samples++;
		fifoBuf_getData(accel_fifo, (uint8_t *) accel, sizeof(accel));
		accel_accum[0] += accel[0];
		accel_accum[1] += accel[1];
		accel_accum[2] += accel[2];
	}
	accel[0] = accel_accum[0] / accel_samples;
	accel[1] = accel_accum[1] / accel_samples;
	accel[2] = accel_accum[2] / accel_samples;

	int32_t gyro_accum[3] = {0,0,0};
	int16_t gyro[3];
	int32_t read_good;
	
	// Make sure we get one sample
	while((read_good = PIOS_IMU3000_ReadFifo(gyro)) != 0);
	while(read_good == 0) {
		gyro_samples++;

		gyro_accum[0] += gyro[0];
		gyro_accum[1] += gyro[1];
		gyro_accum[2] += gyro[2];
		read_good = PIOS_IMU3000_ReadFifo(gyro);
	}
	gyro[0] = gyro_accum[0] / gyro_samples;
	gyro[1] = gyro_accum[1] / gyro_samples;
	gyro[2] = gyro_accum[2] / gyro_samples;

	// Not the swaping of channel orders
	accel_data.filtered.x = accel[0] * PIOS_BMA180_GetScale();
	accel_data.filtered.y = accel[1] * PIOS_BMA180_GetScale();
	accel_data.filtered.z = accel[2] * PIOS_BMA180_GetScale();
	gyro_data.filtered.x = -gyro[1] * 0.00763 * DEG_TO_RAD;;
	gyro_data.filtered.y = -gyro[0] * 0.00763 * DEG_TO_RAD;;
	gyro_data.filtered.z = -gyro[2] * 0.00763 * DEG_TO_RAD;;
	
	AttitudeRawData raw;
		
	raw.gyros[0] = gyro_data.filtered.x * RAD_TO_DEG;
	raw.gyros[1] = gyro_data.filtered.y * RAD_TO_DEG;
	raw.gyros[2] = gyro_data.filtered.z * RAD_TO_DEG;
	
	raw.accels[0] = accel_data.filtered.x;
	raw.accels[1] = accel_data.filtered.y;
	raw.accels[2] = accel_data.filtered.z;
	
	raw.magnetometers[0] = mag_data.scaled.axis[0];
	raw.magnetometers[1] = mag_data.scaled.axis[1];
	raw.magnetometers[2] = mag_data.scaled.axis[2];
	
	AttitudeRawSet(&raw);
	
	return true;
}

/**
 * @brief Perform calibration of a 3-axis field sensor using an affine transformation
 * matrix.
 *
 * Computes result = scale * arg.
 *
 * @param result[out] The three-axis resultant field.
 * @param scale[in] The 4x4 affine transformation matrix.  The fourth row is implicitly
 * 	[0 0 0 1]
 * @param arg[in] The 3-axis input field.  The 'w' component is implicitly 1.
 */
void calibration(float result[3], float scale[3][4], float arg[3])
{
	for (int row = 0; row < 3; ++row) {
		result[row] = 0.0f;
		int col;
		for (col = 0; col < 3; ++col) {
			result[row] += scale[row][col] * arg[col];
		}
		// fourth column: arg has an implicit w value of 1.0f.
		result[row] += scale[row][col];
	}
}

/**
 * @brief Scale an affine transformation matrix by a rotation, defined by a
 * rotation vector.  scale <- rotation * scale
 *
 * @param scale[in,out] The affine transformation matrix to be rotated
 * @param rotation[in] The rotation vector defining the rotation
 */
void affine_rotate(float scale[3][4], float rotation[3])
{
	// Rotate the scale factor matrix in-place
	float rmatrix[3][3];
	Rv2Rot(rotation, rmatrix);

	float ret[3][4];
	for (int row = 0; row < 3; ++row) {
		for (int col = 0; col < 4; ++col) {
			ret[row][col] = 0.0f;
			for (int i = 0; i < 3; ++i) {
				ret[row][col] += rmatrix[row][i] * scale[i][col];
			}
		}
	}
	// copy output to argument
	for (int row = 0; row < 3; ++row) {
		for (int col = 0; col < 4; ++col) {
			scale[row][col] = ret[row][col];
		}
	}
}

#if defined(PIOS_INCLUDE_HMC5883) && defined(PIOS_INCLUDE_I2C)
/**
 * @brief Get the mag data from the I2C sensor and load into structure
 * @return none
 *
 * This function also considers if the home location is set and has a valid
 * magnetic field before updating the mag data to prevent data being used that
 * cannot be interpreted.  In addition the mag data is not used for the first
 * five seconds to allow the filter to start to converge
 */
void process_mag_data()
{
	// Get magnetic readings
	// For now don't use mags until the magnetic field is set AND until 5 seconds
	// after initialization otherwise it seems to have problems
	// TODO: Follow up this initialization issue
	HomeLocationData home;
	HomeLocationGet(&home);
	if (PIOS_HMC5883_NewDataAvailable()) {
		PIOS_HMC5883_ReadMag(mag_data.raw.axis);

		// Swap the axis here to acount for orientation of mag chip (notice 0 and 1 swapped in raw)
		mag_data.scaled.axis[0] = (mag_data.raw.axis[MAG_RAW_X_IDX] * mag_data.calibration.scale[0]) + mag_data.calibration.bias[0];
		mag_data.scaled.axis[1] = (mag_data.raw.axis[MAG_RAW_Y_IDX] * mag_data.calibration.scale[1]) + mag_data.calibration.bias[1];
		mag_data.scaled.axis[2] = (mag_data.raw.axis[MAG_RAW_Z_IDX] * mag_data.calibration.scale[2]) + mag_data.calibration.bias[2];

		// Only use if magnetic length reasonable
		float Blen = sqrt(pow(mag_data.scaled.axis[0],2) + pow(mag_data.scaled.axis[1],2) + pow(mag_data.scaled.axis[2],2));

		mag_data.updated =  (home.Set == HOMELOCATION_SET_TRUE) &&
			((home.Be[0] != 0) || (home.Be[1] != 0) || (home.Be[2] != 0)) &&
			((mag_data.raw.axis[MAG_RAW_X_IDX] != 0) || (mag_data.raw.axis[MAG_RAW_Y_IDX] != 0) || (mag_data.raw.axis[MAG_RAW_Z_IDX] != 0)) &&
			((Blen < mag_len * (1 + INSGPS_MAGTOL)) && (Blen > mag_len * (1 - INSGPS_MAGTOL)));
	}
}
#else
void process_mag_data() { }
#endif


/**
 * @brief Assumes board is not moving computes biases and variances of sensors
 * @returns None
 *
 * All data is stored in global structures.  This function should be called from OP when
 * aircraft is in stable state and then the data stored to SD card.
 *
 * After this function the bias for each sensor will be the mean value.  This doesn't make
 * sense for the z accel so make sure 6 point calibration is also run and those values set
 * after these read.
 */
#define NBIAS 100
#define NVAR  500
void calibrate_sensors()
{
	int i,j;
	float accel_bias[3] = {0, 0, 0};
	float gyro_bias[3] = {0, 0, 0};
	float mag_bias[3] = {0, 0, 0};


	for (i = 0, j = 0; i < NBIAS; i++) {

		get_accel_gyro_data();

		gyro_bias[0] += gyro_data.filtered.x / NBIAS;
		gyro_bias[1] += gyro_data.filtered.y / NBIAS;
		gyro_bias[2] += gyro_data.filtered.z / NBIAS;
		accel_bias[0] += accel_data.filtered.x / NBIAS;
		accel_bias[1] += accel_data.filtered.y / NBIAS;
		accel_bias[2] += accel_data.filtered.z / NBIAS;

#if defined(PIOS_INCLUDE_HMC5883) && defined(PIOS_INCLUDE_I2C)
		if(PIOS_HMC5883_NewDataAvailable()) {
			j ++;
			PIOS_HMC5883_ReadMag(mag_data.raw.axis);
			mag_data.scaled.axis[0] = (mag_data.raw.axis[MAG_RAW_X_IDX] * mag_data.calibration.scale[0]) + mag_data.calibration.bias[0];
			mag_data.scaled.axis[1] = (mag_data.raw.axis[MAG_RAW_Y_IDX] * mag_data.calibration.scale[1]) + mag_data.calibration.bias[1];
			mag_data.scaled.axis[2] = (mag_data.raw.axis[MAG_RAW_Z_IDX] * mag_data.calibration.scale[2]) + mag_data.calibration.bias[2];
			mag_bias[0] += mag_data.scaled.axis[0];
			mag_bias[1] += mag_data.scaled.axis[1];
			mag_bias[2] += mag_data.scaled.axis[2];
		}
#endif

	}
	mag_bias[0] /= j;
	mag_bias[1] /= j;
	mag_bias[2] /= j;

	gyro_data.calibration.variance[0] = 0;
	gyro_data.calibration.variance[1] = 0;
	gyro_data.calibration.variance[2] = 0;
	mag_data.calibration.variance[0] = 0;
	mag_data.calibration.variance[1] = 0;
	mag_data.calibration.variance[2] = 0;
	accel_data.calibration.variance[0] = 0;
	accel_data.calibration.variance[1] = 0;
	accel_data.calibration.variance[2] = 0;

	for (i = 0, j = 0; j < NVAR; j++) {
		get_accel_gyro_data();

		gyro_data.calibration.variance[0] += pow(gyro_data.filtered.x-gyro_bias[0],2) / NVAR;
		gyro_data.calibration.variance[1] += pow(gyro_data.filtered.y-gyro_bias[1],2) / NVAR;
		gyro_data.calibration.variance[2] += pow(gyro_data.filtered.z-gyro_bias[2],2) / NVAR;
		accel_data.calibration.variance[0] += pow(accel_data.filtered.x-accel_bias[0],2) / NVAR;
		accel_data.calibration.variance[1] += pow(accel_data.filtered.y-accel_bias[1],2) / NVAR;
		accel_data.calibration.variance[2] += pow(accel_data.filtered.z-accel_bias[2],2) / NVAR;

#if defined(PIOS_INCLUDE_HMC5883) && defined(PIOS_INCLUDE_I2C)
		if(PIOS_HMC5883_NewDataAvailable()) {
			j ++;
			PIOS_HMC5883_ReadMag(mag_data.raw.axis);
			mag_data.scaled.axis[0] = (mag_data.raw.axis[MAG_RAW_X_IDX] * mag_data.calibration.scale[0]) + mag_data.calibration.bias[0];
			mag_data.scaled.axis[1] = (mag_data.raw.axis[MAG_RAW_Y_IDX] * mag_data.calibration.scale[1]) + mag_data.calibration.bias[1];
			mag_data.scaled.axis[2] = (mag_data.raw.axis[MAG_RAW_Z_IDX] * mag_data.calibration.scale[2]) + mag_data.calibration.bias[2];
			mag_data.calibration.variance[0] += pow(mag_data.scaled.axis[0]-mag_bias[0],2);
			mag_data.calibration.variance[1] += pow(mag_data.scaled.axis[1]-mag_bias[1],2);
			mag_data.calibration.variance[2] += pow(mag_data.scaled.axis[2]-mag_bias[2],2);
		}
#endif

	}

	mag_data.calibration.variance[0] /= j;
	mag_data.calibration.variance[1] /= j;
	mag_data.calibration.variance[2] /= j;

	gyro_data.calibration.bias[0] -= gyro_bias[0];
	gyro_data.calibration.bias[1] -= gyro_bias[1];
	gyro_data.calibration.bias[2] -= gyro_bias[2];
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
	
	ahrs_algorithm = AHRSSETTINGS_ALGORITHM_SIMPLE;
}

void send_attitude(void)
{
	AttitudeActualData attitude;
	AHRSSettingsData settings;
	AHRSSettingsGet(&settings);

	attitude.q1 = attitude_data.quaternion.q1;
	attitude.q2 = attitude_data.quaternion.q2;
	attitude.q3 = attitude_data.quaternion.q3;
	attitude.q4 = attitude_data.quaternion.q4;
	float rpy[3];
	Quaternion2RPY(&attitude_data.quaternion.q1, rpy);
	attitude.Roll = rpy[0] + settings.RollBias;
	attitude.Pitch = rpy[1] + settings.PitchBias;
	attitude.Yaw = rpy[2] + settings.YawBias;
	if(attitude.Yaw > 360)
		attitude.Yaw -= 360;
	AttitudeActualSet(&attitude);
}

void send_velocity(void)
{
	VelocityActualData velocityActual;
	VelocityActualGet(&velocityActual);

	// convert into cm
	velocityActual.North = Nav.Vel[0] * 100;
	velocityActual.East = Nav.Vel[1] * 100;
	velocityActual.Down = Nav.Vel[2] * 100;

	VelocityActualSet(&velocityActual);
}

void send_position(void)
{
	PositionActualData positionActual;
	PositionActualGet(&positionActual);

	// convert into cm
	positionActual.North = Nav.Pos[0] * 100;
	positionActual.East = Nav.Pos[1] * 100;
	positionActual.Down = Nav.Pos[2] * 100;

	PositionActualSet(&positionActual);
}

void send_calibration(void)
{
	AHRSCalibrationData cal;
	AHRSCalibrationGet(&cal);
	for(int ct=0; ct<3; ct++)
	{
		cal.accel_var[ct] = accel_data.calibration.variance[ct];
		cal.gyro_bias[ct] = gyro_data.calibration.bias[ct];
		cal.gyro_var[ct] = gyro_data.calibration.variance[ct];
		cal.mag_var[ct] = mag_data.calibration.variance[ct];
	}
	cal.measure_var = AHRSCALIBRATION_MEASURE_VAR_SET;
	AHRSCalibrationSet(&cal);
}

/**
 * @brief INS calibration callback
 *
 * Called when the OP board sets the calibration
 */
void calibration_callback(AhrsObjHandle obj)
{
	AHRSCalibrationData cal;
	AHRSCalibrationGet(&cal);
	if(cal.measure_var == AHRSCALIBRATION_MEASURE_VAR_SET){

		accel_data.calibration.scale[0][1] = cal.accel_ortho[0];
		accel_data.calibration.scale[1][0] = cal.accel_ortho[0];

		accel_data.calibration.scale[0][2] = cal.accel_ortho[1];
		accel_data.calibration.scale[2][0] = cal.accel_ortho[1];

		accel_data.calibration.scale[1][2] = cal.accel_ortho[2];
		accel_data.calibration.scale[2][1] = cal.accel_ortho[2];

#if 0
		// TODO: Enable after v1.0 feature freeze.
		float rotation[3] = { cal.accel_rotation[0],
				cal.accel_rotation[1],
				cal.accel_rotation[2],
		};

		affine_rotate(accel_data.calibration.scale, rotation);
#endif

		for(int ct=0; ct<3; ct++)
		{
			accel_data.calibration.scale[ct][ct] = cal.accel_scale[ct];
			accel_data.calibration.scale[ct][3] = cal.accel_bias[ct];
			accel_data.calibration.variance[ct] = cal.accel_var[ct];

			gyro_data.calibration.scale[ct] = cal.gyro_scale[ct];
			gyro_data.calibration.bias[ct] = cal.gyro_bias[ct];
			gyro_data.calibration.variance[ct] = cal.gyro_var[ct];
#if 1
			gyro_data.calibration.tempcompfactor[ct] = cal.gyro_tempcompfactor[ct];
#endif
			mag_data.calibration.bias[ct] = cal.mag_bias[ct];
			mag_data.calibration.scale[ct] = cal.mag_scale[ct];
			mag_data.calibration.variance[ct] = cal.mag_var[ct];
		}
		// Note: We need the divided by 1000^2 since we scale mags to have a norm of 1000 and they are scaled to
		// one in code
		float mag_var[3] = {mag_data.calibration.variance[0] / INSGPS_MAGLEN / INSGPS_MAGLEN,
			mag_data.calibration.variance[1] / INSGPS_MAGLEN / INSGPS_MAGLEN,
			mag_data.calibration.variance[2] / INSGPS_MAGLEN / INSGPS_MAGLEN};
		INSSetMagVar(mag_var);
		INSSetAccelVar(accel_data.calibration.variance);
		INSSetGyroVar(gyro_data.calibration.variance);
	}
	else if(cal.measure_var == AHRSCALIBRATION_MEASURE_VAR_MEASURE) {
		calibrate_sensors();
		send_calibration();
	}

	INSSetPosVelVar(cal.pos_var, cal.vel_var);

}

void settings_callback(AhrsObjHandle obj)
{
	AHRSSettingsData settings;
	AHRSSettingsGet(&settings);

	ahrs_algorithm = settings.Algorithm;
}

void homelocation_callback(AhrsObjHandle obj)
{
	HomeLocationData data;
	HomeLocationGet(&data);

	mag_len = sqrt(pow(data.Be[0],2) + pow(data.Be[1],2) + pow(data.Be[2],2));
	float Be[3] = {data.Be[0] / mag_len, data.Be[1] / mag_len, data.Be[2] / mag_len};

	INSSetMagNorth(Be);
}

void firmwareiapobj_callback(AhrsObjHandle obj) 
{
#if 0
	const struct pios_board_info * bdinfo = &pios_board_info_blob;
	
	FirmwareIAPObjData firmwareIAPObj;
	FirmwareIAPObjGet(&firmwareIAPObj);
	if(firmwareIAPObj.ArmReset==0)
		reset_count=0;
	if(firmwareIAPObj.ArmReset==1)
	{
		
		if((firmwareIAPObj.BoardType==bdinfo->board_type) || (firmwareIAPObj.BoardType==0xFF))
		{
			
			++reset_count;
			if(reset_count>2)
			{
				PIOS_IAP_SetRequest1();
				PIOS_IAP_SetRequest2();
				PIOS_SYS_Reset();
			}
		}
	}
	else if(firmwareIAPObj.BoardType==bdinfo->board_type && firmwareIAPObj.crc!=PIOS_BL_HELPER_CRC_Memory_Calc())
	{
		PIOS_BL_HELPER_FLASH_Read_Description(firmwareIAPObj.Description,bdinfo->desc_size);
		firmwareIAPObj.crc=PIOS_BL_HELPER_CRC_Memory_Calc();
		firmwareIAPObj.BoardRevision=bdinfo->board_rev;
		FirmwareIAPObjSet(&firmwareIAPObj);
	}
#endif
}


/**
 * @}
 */


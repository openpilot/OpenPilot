/**
 ******************************************************************************
 * @addtogroup AHRS AHRS
 * @brief The AHRS Modules perform
 *
 * @{
 * @addtogroup AHRS_Main
 * @brief Main function which does the hardware dependent stuff
 * @{
 *
 *
 * @file       ahrs.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
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
#include "ahrs.h"
#include "ahrs_adc.h"
#include "ahrs_timer.h"
#include "pios_opahrs_proto.h"
//#include "ahrs_fsm.h"		/* lfsm_state */
#include "insgps.h"
#include "CoordinateConversions.h"
#include "ahrs_spi_comm.h"

// For debugging the raw sensors
//#define DUMP_RAW
//#define DUMP_FRIENDLY
//#define DUMP_EKF

#ifdef DUMP_EKF
#define NUMX 13			// number of states, X is the state vector
#define NUMW 9			// number of plant noise inputs, w is disturbance noise vector
#define NUMV 10			// number of measurements, v is the measurement noise vector
#define NUMU 6			// number of deterministic inputs, U is the input vector
extern float F[NUMX][NUMX], G[NUMX][NUMW], H[NUMV][NUMX];	// linearized system matrices
extern float P[NUMX][NUMX], X[NUMX];	// covariance matrix and state vector
extern float Q[NUMW], R[NUMV];	// input noise and measurement noise variances
extern float K[NUMX][NUMV];	// feedback gain matrix
#endif

volatile enum algorithms ahrs_algorithm;

/**
 * @addtogroup AHRS_Structures Local Structres
 * @{
 */

//! Contains the data from the mag sensor chip
struct mag_sensor {
	uint8_t id[4];
	uint8_t updated;
	struct {
		int16_t axis[3];
	} raw;
	struct {
		float axis[3];
	} scaled;
	struct {
		float bias[3];
		float scale[3];
		float variance[3];
	} calibration;
} mag_data;

//! Contains the data from the accelerometer
struct accel_sensor {
	struct {
		uint16_t x;
		uint16_t y;
		uint16_t z;
	} raw;
	struct {
		float x;
		float y;
		float z;
	} filtered;
	struct {
		float bias[3];
		float scale[3];
		float variance[3];
	} calibration;
} accel_data;

//! Contains the data from the gyro
struct gyro_sensor {
	struct {
		uint16_t x;
		uint16_t y;
		uint16_t z;
	} raw;
	struct {
		float x;
		float y;
		float z;
	} filtered;
	struct {
		float bias[3];
		float scale[3];
		float variance[3];
	} calibration;
	struct {
		uint16_t xy;
		uint16_t z;
	} temp;
} gyro_data;

//! Conains the current estimate of the attitude
struct attitude_solution {
	struct {
		float q1;
		float q2;
		float q3;
		float q4;
	} quaternion;
} attitude_data;

//! Contains data from the altitude sensor
struct altitude_sensor {
	float altitude;
	bool updated;
} altitude_data;

//! Contains data from the GPS (via the SPI link)
struct gps_sensor {
	float NED[3];
	float heading;
	float groundspeed;
	float quality;
	bool updated;
} gps_data;

/**
 * @}
 */

/* Function Prototypes */
void process_spi_request(void);
void downsample_data(void);
void calibrate_sensors(void);
void reset_values();
void send_calibration(void);
void altitude_callback(AhrsObjHandle obj);
void calibration_callback(AhrsObjHandle obj);
void gps_callback(AhrsObjHandle obj);
void settings_callback(AhrsObjHandle obj);

volatile uint32_t last_counter_idle_start = 0;
volatile uint32_t last_counter_idle_end = 0;
volatile uint32_t idle_counts;
volatile uint32_t running_counts;
uint32_t counter_val;

/**
 * @addtogroup AHRS_Global_Data AHRS Global Data
 * @{
 * Public data.  Used by both EKF and the sender
 */
//! Filter coefficients used in decimation.  Limited order so filter can't run between samples
int16_t fir_coeffs[50];

//! Indicates the communications are requesting a calibration
uint8_t calibration_pending = FALSE;

//! The oversampling rate, ekf is 2k / this
static uint8_t adc_oversampling = 17;


/**
 * @}
 */

/**
 * @brief AHRS Main function
 */
int main()
{
	float gyro[3], accel[3], mag[3];
	float vel[3] = { 0, 0, 0 };
	gps_data.quality = -1;

	ahrs_algorithm = INSGPS_Algo;

	/* Brings up System using CMSIS functions, enables the LEDs. */
	PIOS_SYS_Init();

	/* Delay system */
	PIOS_DELAY_Init();

	/* Communication system */
	PIOS_COM_Init();

	/* ADC system */
	AHRS_ADC_Config(adc_oversampling);

	/* Setup the Accelerometer FS (Full-Scale) GPIO */
	PIOS_GPIO_Enable(0);
	SET_ACCEL_2G;
#if defined(PIOS_INCLUDE_HMC5843) && defined(PIOS_INCLUDE_I2C)
	/* Magnetic sensor system */
	PIOS_I2C_Init();
	PIOS_HMC5843_Init();
	// Get 3 ID bytes
	strcpy((char *)mag_data.id, "ZZZ");
	PIOS_HMC5843_ReadID(mag_data.id);
#endif

	/* SPI link to master */
//	PIOS_SPI_Init();

//	lfsm_init();
	reset_values();

	ahrs_state = AHRS_IDLE;
	AhrsInitComms();
	ahrs_state = AHRS_IDLE;
	while(!AhrsLinkReady()) {
		AhrsPoll();
		while(ahrs_state != AHRS_DATA_READY) ;
		ahrs_state = AHRS_PROCESSING;
		downsample_data();
		ahrs_state = AHRS_IDLE;
		if((total_conversion_blocks % 10) == 0)
			PIOS_LED_Toggle(LED1);
	}
/* we didn't connect the callbacks before because we have to wait
for all data to be up to date before doing anything*/
	AHRSCalibrationConnectCallback(calibration_callback);
	GPSPositionConnectCallback(gps_callback);
	BaroAltitudeConnectCallback(altitude_callback);
	AHRSSettingsConnectCallback(settings_callback);

	calibration_callback(AHRSCalibrationHandle()); //force an update


	/* Use simple averaging filter for now */
	for (int i = 0; i < adc_oversampling; i++)
		fir_coeffs[i] = 1;
	fir_coeffs[adc_oversampling] = adc_oversampling;

	INSGPSInit();

#ifdef DUMP_RAW
	int previous_conversion;
	while (1) {
		AhrsPoll();
		int result;
		uint8_t framing[16] =
		    { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14,
	  15 };
		while (ahrs_state != AHRS_DATA_READY) ;
		ahrs_state = AHRS_PROCESSING;

		if (total_conversion_blocks != previous_conversion + 1)
			PIOS_LED_On(LED1);	// not keeping up
		else
			PIOS_LED_Off(LED1);
		previous_conversion = total_conversion_blocks;

		downsample_data();
		ahrs_state = AHRS_IDLE;;

		// Dump raw buffer
		result = PIOS_COM_SendBuffer(PIOS_COM_AUX, &framing[0], 16);	// framing header
		result += PIOS_COM_SendBuffer(PIOS_COM_AUX, (uint8_t *) & total_conversion_blocks, sizeof(total_conversion_blocks));	// dump block number
		result +=
		    PIOS_COM_SendBuffer(PIOS_COM_AUX,
					(uint8_t *) & valid_data_buffer[0],
					adc_oversampling *
					ADC_CONTINUOUS_CHANNELS *
					sizeof(valid_data_buffer[0]));
		if (result == 0)
			PIOS_LED_Off(LED1);
		else {
			PIOS_LED_On(LED1);
		}
	}
#endif

	timer_start();

	/******************* Main EKF loop ****************************/
	while(1) {
		AhrsPoll();
		AhrsStatus2Data status;
		AhrsStatus2Get(&status);
		status.CPULoad = ((float)running_counts /
						  (float)(idle_counts + running_counts)) * 100;
		status.IdleTimePerCyle = idle_counts / (TIMER_RATE / 10000);
		status.RunningTimePerCyle = running_counts / (TIMER_RATE / 10000);
		status.DroppedUpdates = ekf_too_slow;
		AhrsStatus2Set(&status);

		// Alive signal
		if ((total_conversion_blocks % 100) == 0)
			PIOS_LED_Toggle(LED1);

		if (calibration_pending) {
			calibrate_sensors();
			calibration_pending = FALSE;
		}
#if defined(PIOS_INCLUDE_HMC5843) && defined(PIOS_INCLUDE_I2C)
		// Get magnetic readings
		if (PIOS_HMC5843_NewDataAvailable()) {
			PIOS_HMC5843_ReadMag(mag_data.raw.axis);
			mag_data.scaled.axis[0] = (mag_data.raw.axis[0] * mag_data.calibration.scale[0]) + mag_data.calibration.bias[0];
			mag_data.scaled.axis[1] = (mag_data.raw.axis[1] * mag_data.calibration.scale[1]) + mag_data.calibration.bias[1];
			mag_data.scaled.axis[2] = (mag_data.raw.axis[2] * mag_data.calibration.scale[2]) + mag_data.calibration.bias[2];
			mag_data.updated = 1;
		}

#endif
		// Delay for valid data

		counter_val = timer_count();
		running_counts = counter_val - last_counter_idle_end;
		last_counter_idle_start = counter_val;

		while (ahrs_state != AHRS_DATA_READY) ;

		counter_val = timer_count();
		idle_counts = counter_val - last_counter_idle_start;
		last_counter_idle_end = counter_val;

		ahrs_state = AHRS_PROCESSING;

		downsample_data();

		/******************** INS ALGORITHM **************************/
		if (ahrs_algorithm == INSGPS_Algo) {

			// format data for INS algo
			gyro[0] = gyro_data.filtered.x;
			gyro[1] = gyro_data.filtered.y;
			gyro[2] = gyro_data.filtered.z;
			accel[0] = accel_data.filtered.x,
			accel[1] = accel_data.filtered.y,
			accel[2] = accel_data.filtered.z,
			// Note: The magnetometer driver returns registers X,Y,Z from the chip which are
			// (left, backward, up).  Remapping to (forward, right, down).
			mag[0] = -mag_data.scaled.axis[1];
			mag[1] = -mag_data.scaled.axis[0];
			mag[2] = -mag_data.scaled.axis[2];

			INSStatePrediction(gyro, accel, 1 / (float)EKF_RATE);
			process_spi_request();  // get message out quickly
			INSCovariancePrediction(1 / (float)EKF_RATE);

			if (gps_data.updated && gps_data.quality == 1) {
				// Compute velocity from Heading and groundspeed
				vel[0] =
				    gps_data.groundspeed *
				    cos(gps_data.heading * M_PI / 180);
				vel[1] =
				    gps_data.groundspeed *
				    sin(gps_data.heading * M_PI / 180);

				INSSetPosVelVar(0.004);
				if (gps_data.updated) {
					//TOOD: add check for altitude updates
					FullCorrection(mag, gps_data.NED,
						       vel,
						       altitude_data.
						       altitude);
					gps_data.updated = 0;
				} else {
					GpsBaroCorrection(gps_data.NED,
							  vel,
							  altitude_data.
							  altitude);
				}

				gps_data.updated = false;
				mag_data.updated = 0;
			} else if (gps_data.quality != -1
				   && mag_data.updated == 1) {
				float mag_var[3] = {mag_data.calibration.variance[1], mag_data.calibration.variance[0], mag_data.calibration.variance[2]};
				INSSetMagVar(mag_var);
				MagCorrection(mag);	// only trust mags if outdoors
				mag_data.updated = 0;
			} else {
				// Indoors, update with zero position and velocity and high covariance
				INSSetPosVelVar(0.1);
				vel[0] = 0;
				vel[1] = 0;
				vel[2] = 0;

				if(mag_data.updated == 1) {
					float mag_var[3] = {10,10,10};
					INSSetMagVar(mag_var);
					MagVelBaroCorrection(mag,vel,altitude_data.altitude);  // only trust mags if outdoors
					mag_data.updated = 0;
				} else {
					VelBaroCorrection(vel, altitude_data.altitude);
				}
			}

			attitude_data.quaternion.q1 = Nav.q[0];
			attitude_data.quaternion.q2 = Nav.q[1];
			attitude_data.quaternion.q3 = Nav.q[2];
			attitude_data.quaternion.q4 = Nav.q[3];
		} else if (ahrs_algorithm == SIMPLE_Algo) {
			float q[4];
			float rpy[3];
	    /***************** SIMPLE ATTITUDE FROM NORTH AND ACCEL ************/
			/* Very simple computation of the heading and attitude from accel. */
			rpy[2] =
			    atan2((mag_data.raw.axis[0]),
				  (-1 * mag_data.raw.axis[1])) * 180 /
			    M_PI;
			rpy[1] =
			    atan2(accel_data.filtered.x,
				  accel_data.filtered.z) * 180 / M_PI;
			rpy[0] =
			    atan2(accel_data.filtered.y,
				  accel_data.filtered.z) * 180 / M_PI;

			RPY2Quaternion(rpy, q);
			attitude_data.quaternion.q1 = q[0];
			attitude_data.quaternion.q2 = q[1];
			attitude_data.quaternion.q3 = q[2];
			attitude_data.quaternion.q4 = q[3];
			process_spi_request();

		}

		ahrs_state = AHRS_IDLE;

#ifdef DUMP_FRIENDLY
		PIOS_COM_SendFormattedStringNonBlocking(PIOS_COM_AUX, "b: %d\r\n",
							total_conversion_blocks);
		PIOS_COM_SendFormattedStringNonBlocking(PIOS_COM_AUX,"a: %d %d %d\r\n",
							(int16_t) (accel_data.filtered.x * 1000),
							(int16_t) (accel_data.filtered.y * 1000),
							(int16_t) (accel_data.filtered.z * 1000));
		PIOS_COM_SendFormattedStringNonBlocking(PIOS_COM_AUX, "g: %d %d %d\r\n",
							(int16_t) (gyro_data.filtered.x * 1000),
							(int16_t) (gyro_data.filtered.y * 1000),
							(int16_t) (gyro_data.filtered.z * 1000));
		PIOS_COM_SendFormattedStringNonBlocking(PIOS_COM_AUX,"m: %d %d %d\r\n",
							mag_data.raw.axis[0],
							mag_data.raw.axis[1],
							mag_data.raw.axis[2]);
		PIOS_COM_SendFormattedStringNonBlocking(PIOS_COM_AUX,
							"q: %d %d %d %d\r\n",
							(int16_t) (Nav.q[0] * 1000),
							(int16_t) (Nav.q[1] * 1000),
							(int16_t) (Nav.q[2] * 1000),
							(int16_t) (Nav.q[3] * 1000));
#endif
#ifdef DUMP_EKF
		uint8_t framing[16] =
		    { 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1,
	  0 };
		extern float F[NUMX][NUMX], G[NUMX][NUMW], H[NUMV][NUMX];	// linearized system matrices
		extern float P[NUMX][NUMX], X[NUMX];	// covariance matrix and state vector
		extern float Q[NUMW], R[NUMV];	// input noise and measurement noise variances
		extern float K[NUMX][NUMV];	// feedback gain matrix

		// Dump raw buffer
		int8_t result;
		result = PIOS_COM_SendBuffer(PIOS_COM_AUX, &framing[0], 16);	// framing header
		result += PIOS_COM_SendBuffer(PIOS_COM_AUX, (uint8_t *) & total_conversion_blocks, sizeof(total_conversion_blocks));	// dump block number
		result +=
		    PIOS_COM_SendBuffer(PIOS_COM_AUX,
					(uint8_t *) & mag_data,
					sizeof(mag_data));
		result +=
		    PIOS_COM_SendBuffer(PIOS_COM_AUX,
					(uint8_t *) & gps_data,
					sizeof(gps_data));
		result +=
		    PIOS_COM_SendBuffer(PIOS_COM_AUX,
					(uint8_t *) & accel_data,
					sizeof(accel_data));
		result +=
		    PIOS_COM_SendBuffer(PIOS_COM_AUX,
					(uint8_t *) & gyro_data,
					sizeof(gyro_data));
		result +=
		    PIOS_COM_SendBuffer(PIOS_COM_AUX, (uint8_t *) & Q,
					sizeof(float) * NUMX * NUMX);
		result +=
		    PIOS_COM_SendBuffer(PIOS_COM_AUX, (uint8_t *) & K,
					sizeof(float) * NUMX * NUMV);
		result +=
		    PIOS_COM_SendBuffer(PIOS_COM_AUX, (uint8_t *) & X,
					sizeof(float) * NUMX * NUMX);

		if (result == 0)
			PIOS_LED_Off(LED1);
		else {
			PIOS_LED_On(LED1);
		}
#endif

	}

	return 0;
}

/**
 * @brief Downsample the analog data
 * @return none
 *
 * Tried to make as much of the filtering fixed point when possible.  Need to account
 * for offset for each sample before the multiplication if filter not a boxcar.  Could
 * precompute fixed offset as sum[fir_coeffs[i]] * ACCEL_OFFSET.  Puts data into global
 * data structures @ref accel_data and @ref gyro_data.
 *
 * The accel_data values are converted into a coordinate system where X is forwards along
 * the fuselage, Y is along right the wing, and Z is down.
 */
void downsample_data()
{
	uint16_t i;

	// Get the Y data.  Third byte in.  Convert to m/s
	accel_data.filtered.y = 0;
	for (i = 0; i < adc_oversampling; i++)
		accel_data.filtered.y += valid_data_buffer[0 + i * PIOS_ADC_NUM_PINS] * fir_coeffs[i];
	accel_data.filtered.y /= (float) fir_coeffs[adc_oversampling];
	accel_data.filtered.y = (accel_data.filtered.y * accel_data.calibration.scale[1]) + accel_data.calibration.bias[1];

	// Get the X data which projects forward/backwards.  Fifth byte in.  Convert to m/s
	accel_data.filtered.x = 0;
	for (i = 0; i < adc_oversampling; i++)
		accel_data.filtered.x += valid_data_buffer[2 + i * PIOS_ADC_NUM_PINS] * fir_coeffs[i];
	accel_data.filtered.x /= (float) fir_coeffs[adc_oversampling];
	accel_data.filtered.x = (accel_data.filtered.x * accel_data.calibration.scale[0]) + accel_data.calibration.bias[0];

	// Get the Z data.  Third byte in.  Convert to m/s
	accel_data.filtered.z = 0;
	for (i = 0; i < adc_oversampling; i++)
		accel_data.filtered.z += valid_data_buffer[4 + i * PIOS_ADC_NUM_PINS] * fir_coeffs[i];
	accel_data.filtered.z /= (float) fir_coeffs[adc_oversampling];
	accel_data.filtered.z = (accel_data.filtered.z * accel_data.calibration.scale[2]) + accel_data.calibration.bias[2];

	// Get the X gyro data.  Seventh byte in.  Convert to deg/s.
	gyro_data.filtered.x = 0;
	for (i = 0; i < adc_oversampling; i++)
		gyro_data.filtered.x  += valid_data_buffer[1 + i * PIOS_ADC_NUM_PINS] * fir_coeffs[i];
	gyro_data.filtered.x /= fir_coeffs[adc_oversampling];
	gyro_data.filtered.x = (gyro_data.filtered.x * gyro_data.calibration.scale[0]) + gyro_data.calibration.bias[0];

	// Get the Y gyro data.  Second byte in.  Convert to deg/s.
	gyro_data.filtered.y = 0;
	for (i = 0; i < adc_oversampling; i++)
		gyro_data.filtered.y += valid_data_buffer[3 + i * PIOS_ADC_NUM_PINS] * fir_coeffs[i];
	gyro_data.filtered.y /= fir_coeffs[adc_oversampling];
	gyro_data.filtered.y = (gyro_data.filtered.y * gyro_data.calibration.scale[1]) + gyro_data.calibration.bias[1];

	// Get the Z gyro data.  Fifth byte in.  Convert to deg/s.
	gyro_data.filtered.z = 0;
	for (i = 0; i < adc_oversampling; i++)
		gyro_data.filtered.z += valid_data_buffer[5 + i * PIOS_ADC_NUM_PINS] * fir_coeffs[i];
	gyro_data.filtered.z /= fir_coeffs[adc_oversampling];
	gyro_data.filtered.z = (gyro_data.filtered.z * gyro_data.calibration.scale[2]) + gyro_data.calibration.bias[2];

	AttitudeRawData raw;

	raw.gyros[0] = valid_data_buffer[1];
	raw.gyros[1] = valid_data_buffer[3];
	raw.gyros[2] = valid_data_buffer[5];

	raw.gyros_filtered[0] = gyro_data.filtered.x;
	raw.gyros_filtered[1] = gyro_data.filtered.y;
	raw.gyros_filtered[2] = gyro_data.filtered.z;

	raw.accels[0] = valid_data_buffer[2];
	raw.accels[1] = valid_data_buffer[0];
	raw.accels[2] = valid_data_buffer[4];

	raw.accels_filtered[0] = accel_data.filtered.x;
	raw.accels_filtered[1] = accel_data.filtered.y;
	raw.accels_filtered[2] = accel_data.filtered.z;

	raw.magnetometers[0] = mag_data.scaled.axis[0];
	raw.magnetometers[1] = mag_data.scaled.axis[1];
	raw.magnetometers[2] = mag_data.scaled.axis[2];

	AttitudeRawSet(&raw);
}

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
		while (ahrs_state != AHRS_DATA_READY) ;
		ahrs_state = AHRS_PROCESSING;
		downsample_data();
		gyro_bias[0] += gyro_data.filtered.x / NBIAS;
		gyro_bias[1] += gyro_data.filtered.y / NBIAS;
		gyro_bias[2] += gyro_data.filtered.z / NBIAS;
		accel_bias[0] += accel_data.filtered.x / NBIAS;
		accel_bias[1] += accel_data.filtered.y / NBIAS;
		accel_bias[2] += accel_data.filtered.z / NBIAS;
		ahrs_state = AHRS_IDLE;

#if defined(PIOS_INCLUDE_HMC5843) && defined(PIOS_INCLUDE_I2C)
		if(PIOS_HMC5843_NewDataAvailable()) {
			j ++;
			PIOS_HMC5843_ReadMag(mag_data.raw.axis);
			mag_data.scaled.axis[0] = (mag_data.raw.axis[0] * mag_data.calibration.scale[0]) + mag_data.calibration.bias[0];
			mag_data.scaled.axis[1] = (mag_data.raw.axis[1] * mag_data.calibration.scale[1]) + mag_data.calibration.bias[1];
			mag_data.scaled.axis[2] = (mag_data.raw.axis[2] * mag_data.calibration.scale[2]) + mag_data.calibration.bias[2];
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
		while (ahrs_state != AHRS_DATA_READY) ;
		ahrs_state = AHRS_PROCESSING;
		downsample_data();
		gyro_data.calibration.variance[0] += pow(gyro_data.filtered.x-gyro_bias[0],2) / NVAR;
		gyro_data.calibration.variance[1] += pow(gyro_data.filtered.y-gyro_bias[1],2) / NVAR;
		gyro_data.calibration.variance[2] += pow(gyro_data.filtered.z-gyro_bias[2],2) / NVAR;
		accel_data.calibration.variance[0] += pow(accel_data.filtered.x-accel_bias[0],2) / NVAR;
		accel_data.calibration.variance[1] += pow(accel_data.filtered.y-accel_bias[1],2) / NVAR;
		accel_data.calibration.variance[2] += pow(accel_data.filtered.z-accel_bias[2],2) / NVAR;
		ahrs_state = AHRS_IDLE;
#if defined(PIOS_INCLUDE_HMC5843) && defined(PIOS_INCLUDE_I2C)
		if(PIOS_HMC5843_NewDataAvailable()) {
			j ++;
			PIOS_HMC5843_ReadMag(mag_data.raw.axis);
			mag_data.scaled.axis[0] = (mag_data.raw.axis[0] * mag_data.calibration.scale[0]) + mag_data.calibration.bias[0];
			mag_data.scaled.axis[1] = (mag_data.raw.axis[1] * mag_data.calibration.scale[1]) + mag_data.calibration.bias[1];
			mag_data.scaled.axis[2] = (mag_data.raw.axis[2] * mag_data.calibration.scale[2]) + mag_data.calibration.bias[2];
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
	AHRSCalibrationData cal;
	AHRSCalibrationGet(&cal);
	cal.measure_var = AHRSCALIBRATION_MEASURE_VAR_SET;
	AHRSCalibrationSet(&cal);
}

/**
 * @brief Populate fields with initial values
 */
void reset_values() {
	accel_data.calibration.scale[0] = 0.012;
	accel_data.calibration.scale[1] = 0.012;
	accel_data.calibration.scale[2] = -0.012;
	accel_data.calibration.bias[0] = 24;
	accel_data.calibration.bias[1] = 24;
	accel_data.calibration.bias[2] = -24;
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
	mag_data.calibration.variance[0] = 1;
	mag_data.calibration.variance[1] = 1;
	mag_data.calibration.variance[2] = 1;
}


void process_spi_request(void)
{
	AttitudeActualData attitude;
	attitude.q1 = attitude_data.quaternion.q1;
	attitude.q2 = attitude_data.quaternion.q2;
	attitude.q3 = attitude_data.quaternion.q3;
	attitude.q4 = attitude_data.quaternion.q4;
	float rpy[3];
	Quaternion2RPY(&attitude_data.quaternion.q1, rpy);
	attitude.Roll = rpy[0];
	attitude.Pitch = rpy[1];
	attitude.Yaw = rpy[2];
	AttitudeActualSet(&attitude);
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
 * @brief AHRS calibration callback
 *
 * Called when the OP board sets the calibration
 */
void calibration_callback(AhrsObjHandle obj)
{
	AHRSCalibrationData cal;
	AHRSCalibrationGet(&cal);
	if(cal.measure_var == AHRSCALIBRATION_MEASURE_VAR_SET){
		for(int ct=0; ct<3; ct++)
		{
			accel_data.calibration.scale[ct] = cal.accel_scale[ct];
			accel_data.calibration.bias[ct] = cal.accel_bias[ct];
			accel_data.calibration.variance[ct] = cal.accel_var[ct];
			gyro_data.calibration.scale[ct] = cal.gyro_scale[ct];
			gyro_data.calibration.bias[ct] = cal.gyro_bias[ct];
			gyro_data.calibration.variance[ct] = cal.gyro_var[ct];
			mag_data.calibration.bias[ct] = cal.mag_bias[ct];
			mag_data.calibration.scale[ct] = cal.mag_scale[ct];
			mag_data.calibration.variance[ct] = cal.mag_var[ct];
		}
	}else if(cal.measure_var == AHRSCALIBRATION_MEASURE_VAR_MEASURE){
		calibration_pending = true;
	}else if(cal.measure_var == AHRSCALIBRATION_MEASURE_VAR_ECHO){
		send_calibration();
	}
}

void gps_callback(AhrsObjHandle obj)
{
	GPSPositionData pos;
	GPSPositionGet(&pos);
	HomeLocationData home;
	HomeLocationGet(&home);

	if(home.Set == HOMELOCATION_SET_FALSE || home.Indoor == HOMELOCATION_INDOOR_TRUE) {
		gps_data.NED[0] = 0;
		gps_data.NED[1] = 0;
		gps_data.NED[2] = 0;
		gps_data.groundspeed = 0;
		gps_data.heading = 0;
		gps_data.quality = -1; // indicates indoor mode, high variance zeros update
		gps_data.updated = true;
		return;
	}

	if(pos.Status != GPSPOSITION_STATUS_FIX3D) //FIXME: Will this work? the old ahrs_comms does it differently.
	{
		gps_data.quality = 0;
		gps_data.updated = true;
		return;
	}

	double LLA[3] = {(double) pos.Latitude / 1e7, (double) pos.Longitude / 1e7, (double) (pos.GeoidSeparation + pos.Altitude)};
	// convert from cm back to meters
	double ECEF[3] = {(double) (home.ECEF[0] / 100), (double) (home.ECEF[1] / 100), (double) (home.ECEF[2] / 100)};
		LLA2Base(LLA, ECEF, (float (*)[3]) home.RNE, gps_data.NED);

	gps_data.heading = pos.Heading;
	gps_data.groundspeed = pos.Groundspeed;
	gps_data.quality = 1;
	gps_data.updated = true;
}

void altitude_callback(AhrsObjHandle obj)
{
	BaroAltitudeData alt;
	BaroAltitudeGet(&alt);
	altitude_data.altitude = alt.Altitude;
	altitude_data.updated = true;
}

void settings_callback(AhrsObjHandle obj)
{
	AHRSSettingsData settings;
	AHRSSettingsGet(&settings);

	if(settings.Algorithm ==  AHRSSETTINGS_ALGORITHM_INSGPS)
	{
		ahrs_algorithm = INSGPS_Algo;
	}else
	{
		ahrs_algorithm = SIMPLE_Algo;
	}
}



/**
 * @}
 */

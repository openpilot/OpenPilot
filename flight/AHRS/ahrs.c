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
#include "pios.h"
#include "ahrs_timer.h"
#include "ahrs_spi_comm.h"
#include "insgps.h"
#include "CoordinateConversions.h"
#include <stdbool.h>
#include "fifo_buffer.h"

#define DEG_TO_RAD         (M_PI / 180.0)
#define RAD_TO_DEG         (180.0 / M_PI)

#define INSGPS_GPS_TIMEOUT 2   /* 2 seconds triggers reinit of position */
#define INSGPS_GPS_MINSAT  6   /* 2 seconds triggers reinit of position */
#define INSGPS_GPS_MINPDOP 3.5 /* minimum PDOP for postition updates    */
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

// For debugging the raw sensors
//#define DUMP_RAW
//#define DUMP_EKF
//#define PIP_DUMP_RAW

volatile int8_t ahrs_algorithm;

/* INS functions */
void ins_outdoor_update();
void ins_indoor_update();
void simple_update();

/* Data accessors */
void adc_callback(float *);
bool get_accel_gyro_data();
void process_mag_data();
void reset_values();
void calibrate_sensors(void);

/* Communication functions */
void send_calibration(void);
void send_attitude(void);
void send_velocity(void);
void send_position(void);
void homelocation_callback(AhrsObjHandle obj);
void altitude_callback(AhrsObjHandle obj);
void calibration_callback(AhrsObjHandle obj);
void gps_callback(AhrsObjHandle obj);
void settings_callback(AhrsObjHandle obj);
void affine_rotate(float scale[3][4], float rotation[3]);
void calibration(float result[3], float scale[3][4], float arg[3]);

/* Bootloader related functions and var*/
void firmwareiapobj_callback(AhrsObjHandle obj);
volatile uint8_t reset_count=0;

/**
 * @addtogroup AHRS_Global_Data AHRS Global Data
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

//! The oversampling rate, ekf is 2k / this
uint8_t adc_oversampling = 15;

//! Offset correction of barometric alt, to match gps data
static float baro_offset = 0;

static float mag_len = 0;

typedef enum { AHRS_IDLE, AHRS_DATA_READY, AHRS_PROCESSING } states;
volatile int32_t ekf_too_slow;
volatile int32_t total_conversion_blocks;

//! Buffer to allow ADC to run a bit faster than EKF
t_fifo_buffer adc_fifo_buffer;


/**
 * @}
 */

/* INS functions */

/**
 * @brief Update the EKF when in outdoor mode.  The primary difference is using the GPS values.
 */
void ins_outdoor_update() 
{
	float gyro[3], accel[3], vel[3];
	static uint32_t last_gps_time = 0;
	uint16_t sensors;
		
	// format data for INS algo
	gyro[0] = gyro_data.filtered.x;
	gyro[1] = gyro_data.filtered.y;
	gyro[2] = gyro_data.filtered.z;
	accel[0] = accel_data.filtered.x,
	accel[1] = accel_data.filtered.y,
	accel[2] = accel_data.filtered.z,
		
	INSStatePrediction(gyro, accel, 1 / (float)EKF_RATE);
	attitude_data.quaternion.q1 = Nav.q[0];
	attitude_data.quaternion.q2 = Nav.q[1];
	attitude_data.quaternion.q3 = Nav.q[2];
	attitude_data.quaternion.q4 = Nav.q[3];			
	send_attitude();  // get message out quickly
	send_velocity();
	send_position();
	INSCovariancePrediction(1 / (float)EKF_RATE);
		
	sensors = 0;
	
	/* 
	 * Detect if greater than certain time since last gps update and if so
	 * reset EKF to that position since probably drifted too far for safe
	 * update 
	 */
	uint32_t this_gps_time = timer_count();
	float gps_delay;
	
	if (this_gps_time < last_gps_time)
		gps_delay = ((0xFFFF - last_gps_time) - this_gps_time) / timer_rate();
	else 
		gps_delay = (this_gps_time - last_gps_time) / timer_rate();
	last_gps_time = this_gps_time;
	
	if (gps_data.updated)
	{
		vel[0] = gps_data.groundspeed * cos(gps_data.heading * DEG_TO_RAD);
		vel[1] = gps_data.groundspeed * sin(gps_data.heading * DEG_TO_RAD);
		vel[2] = 0;
				
		if(gps_delay > INSGPS_GPS_TIMEOUT)
			INSPosVelReset(gps_data.NED,vel); // position stale, reset		
		else  {
			sensors |= HORIZ_SENSORS | POS_SENSORS;						
		}
			
		/* 
		 * When using gps need to make sure that barometer is brought into NED frame   
		 * we should try and see if the altitude from the home location is good enough 
		 * to use for the offset but for now starting with this conservative filter    
		 */
		if(fabs(gps_data.NED[2] + (altitude_data.altitude - baro_offset)) > 10) {
			baro_offset = gps_data.NED[2] + altitude_data.altitude;
		} else {
			/* IIR filter with 100 second or so tau to keep them crudely in the same frame */
			baro_offset = baro_offset * 0.999 + (gps_data.NED[2] + altitude_data.altitude) * 0.001;
		}
		gps_data.updated = false;
	} else if (gps_delay > INSGPS_GPS_TIMEOUT) {
		vel[0] = 0;
		vel[1] = 0;
		vel[2] = 0;
		sensors |= VERT_SENSORS | HORIZ_SENSORS;
	}
	
	if(mag_data.updated) {
		sensors |= MAG_SENSORS;
		mag_data.updated = false;
	}
	
	if(altitude_data.updated) {
		sensors |= BARO_SENSOR;
		altitude_data.updated = false;
	}
		
	/* 
	 * TODO: Need to add a general sanity check for all the inputs to make sure their kosher 
	 * although probably should occur within INS itself 
	 */
	INSCorrection(mag_data.scaled.axis, gps_data.NED, vel, altitude_data.altitude - baro_offset, sensors);	
}

/**
 * @brief Update the EKF when in indoor mode
 */
void ins_indoor_update()
{
	float gyro[3], accel[3], vel[3];
	static uint32_t last_indoor_time = 0;
	uint16_t sensors = 0;

	// format data for INS algo
	gyro[0] = gyro_data.filtered.x;
	gyro[1] = gyro_data.filtered.y;
	gyro[2] = gyro_data.filtered.z;
	accel[0] = accel_data.filtered.x,
	accel[1] = accel_data.filtered.y,
	accel[2] = accel_data.filtered.z,
	
	INSStatePrediction(gyro, accel, 1 / (float)EKF_RATE);
	attitude_data.quaternion.q1 = Nav.q[0];
	attitude_data.quaternion.q2 = Nav.q[1];
	attitude_data.quaternion.q3 = Nav.q[2];
	attitude_data.quaternion.q4 = Nav.q[3];			
	send_attitude();  // get message out quickly
	send_velocity();
	send_position();
	INSCovariancePrediction(1 / (float)EKF_RATE);
	
	/* Indoors, update with zero position and velocity and high covariance */
	vel[0] = 0;
	vel[1] = 0;
	vel[2] = 0;
	
	uint32_t this_indoor_time = timer_count();
	float indoor_delay;
	
	/*
	 * Detect if greater than certain time since last gps update and if so
	 * reset EKF to that position since probably drifted too far for safe
	 * update
	 */
	if (this_indoor_time < last_indoor_time)
		indoor_delay = ((0xFFFF - last_indoor_time) - this_indoor_time) / timer_rate();
	else 
		indoor_delay = (this_indoor_time - last_indoor_time) / timer_rate();
	last_indoor_time = this_indoor_time;
		
	if(indoor_delay > INSGPS_GPS_TIMEOUT)
		INSPosVelReset(vel,vel);
	else 
		sensors = HORIZ_SENSORS | VERT_SENSORS;

	if(mag_data.updated && (ahrs_algorithm == AHRSSETTINGS_ALGORITHM_INSGPS_INDOOR)) {
		sensors |= MAG_SENSORS;
		mag_data.updated = false;
	}
	
	if(altitude_data.updated) {
		sensors |= BARO_SENSOR;
		altitude_data.updated = false;
	}
	
	/* 
	 * TODO: Need to add a general sanity check for all the inputs to make sure their kosher 
	 * although probably should occur within INS itself 
	 */
	INSCorrection(mag_data.scaled.axis, gps_data.NED, vel, altitude_data.altitude, sensors | HORIZ_SENSORS | VERT_SENSORS);	
}

/**
 * @brief Initialize the EKF assuming stationary
 */
void ins_init_algorithm()
{
	float Rbe[3][3], q[4], accels[3], rpy[3], mag;
	float ge[3]={0,0,-9.81}, zeros[3]={0,0,0}, Pdiag[16]={25,25,25,5,5,5,1e-5,1e-5,1e-5,1e-5,1e-5,1e-5,1e-5,1e-4,1e-4,1e-4};
	bool using_mags, using_gps;
	
	INSGPSInit();
	
	HomeLocationData home;
	HomeLocationGet(&home);
	
	accels[0]=accel_data.filtered.x;
	accels[1]=accel_data.filtered.y;
	accels[2]=accel_data.filtered.z;
	
	using_mags = (ahrs_algorithm == AHRSSETTINGS_ALGORITHM_INSGPS_OUTDOOR) || (ahrs_algorithm == AHRSSETTINGS_ALGORITHM_INSGPS_INDOOR);
	using_mags &= (home.Be[0] != 0) || (home.Be[1] != 0) || (home.Be[2] != 0);  /* only use mags when valid home location */
	
	using_gps = (ahrs_algorithm == AHRSSETTINGS_ALGORITHM_INSGPS_OUTDOOR) && (gps_data.quality != 0);

	/* Block till a data update */
	get_accel_gyro_data();

	/* Ensure we get mag data in a timely manner */
	uint16_t fail_count = 50; // 50 at 200 Hz is up to 0.25 sec
	while(using_mags && !mag_data.updated && fail_count--) {
		get_accel_gyro_data();
		AhrsPoll();
	}
	using_mags &= mag_data.updated;		
	
	if (using_mags) {
		/* Spin waiting for mag data */
		while(!mag_data.updated) {
			get_accel_gyro_data();
			AhrsPoll();
		}
		mag_data.updated = false;

		RotFrom2Vectors(accels, ge, mag_data.scaled.axis, home.Be, Rbe);
		R2Quaternion(Rbe,q);
		if (using_gps)
			INSSetState(gps_data.NED, zeros, q, zeros, zeros);
		else
			INSSetState(zeros, zeros, q, zeros, zeros);
	} else {		
		// assume yaw = 0
		mag = VectorMagnitude(accels);
		rpy[1] = asinf(-accels[0]/mag);
		rpy[0] = atan2(accels[1]/mag,accels[2]/mag);
		rpy[2] = 0;
		RPY2Quaternion(rpy,q);
		if (using_gps)
			INSSetState(gps_data.NED, zeros, q, zeros, zeros);
		else
			INSSetState(zeros, zeros, q, zeros, zeros);
	}
	
	INSResetP(Pdiag);
	
	// TODO: include initial estimate of gyro bias?
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
 
/**
 * @brief Debugging function to output all the ADC samples 
 */
#if defined(DUMP_RAW)
void print_ahrs_raw() 
{
	int result;
	static int previous_conversion = 0;
	int16_t * valid_data_buffer;
	
	uint8_t framing[16] =
	{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14,
		15 };
	
	get_accel_gyro_data();
	
	valid_data_buffer = PIOS_ADC_GetRawBuffer();

	if (total_conversion_blocks != previous_conversion + 1)
		PIOS_LED_On(LED1);	// not keeping up
	else
		PIOS_LED_Off(LED1);
	previous_conversion = total_conversion_blocks;
	
	
	// Dump raw buffer
	result = PIOS_COM_SendBufferNonBlocking(PIOS_COM_AUX, &framing[0], 16);	// framing header
	result += PIOS_COM_SendBufferNonBlocking(PIOS_COM_AUX, (uint8_t *) & total_conversion_blocks, sizeof(total_conversion_blocks));	// dump block number
	result +=
	PIOS_COM_SendBufferNonBlocking(PIOS_COM_AUX,
			    (uint8_t *) & valid_data_buffer[0],
			    adc_oversampling *
			    PIOS_ADC_NUM_PINS *
			    sizeof(valid_data_buffer[0]));
	if (result == 0)
		PIOS_LED_Off(LED1);
	else {
		PIOS_LED_On(LED1);
	}	
}
#endif

#if defined(PIP_DUMP_RAW)

	#define MAX_OVERSAMPLING	PIOS_ADC_MAX_OVERSAMPLING

	void print_ahrs_raw()
	{
		int16_t accel_x[MAX_OVERSAMPLING], accel_y[MAX_OVERSAMPLING], accel_z[MAX_OVERSAMPLING];
		int16_t  gyro_x[MAX_OVERSAMPLING],  gyro_y[MAX_OVERSAMPLING],  gyro_z[MAX_OVERSAMPLING];
		#if defined(PIOS_INCLUDE_HMC5843) && defined(PIOS_INCLUDE_I2C)
			int16_t mag[3];
			int16_t mag_x, mag_y, mag_z;
		#endif

		static int previous_conversion = 0;

		uint8_t framing[3] = {0xD2, 0x73, 0x00};

		// wait for new raw samples
		while (previous_conversion == total_conversion_blocks);
		if ((previous_conversion + 1) != total_conversion_blocks)
			PIOS_LED_On(LED1);                              // we are not keeping up
		previous_conversion = total_conversion_blocks;

		// fetch the buffer address for the new samples
		int16_t *valid_data_buffer = PIOS_ADC_GetRawBuffer();

		// fetch number of raw samples in the buffer (per channel)
		int over_sampling = PIOS_ADC_GetOverSampling();

		framing[2] = over_sampling;

		// copy the raw samples into their own buffers
		for (uint16_t i = 0, j = 0; i < over_sampling; i++, j += PIOS_ADC_NUM_CHANNELS)
		{
			accel_x[i] = valid_data_buffer[ACCEL_RAW_X_IDX + j];
			accel_y[i] = valid_data_buffer[ACCEL_RAW_Y_IDX + j];
			accel_z[i] = valid_data_buffer[ACCEL_RAW_Z_IDX + j];

			gyro_x[i] = valid_data_buffer[GYRO_RAW_X_IDX + j];
			gyro_y[i] = valid_data_buffer[GYRO_RAW_Y_IDX + j];
			gyro_z[i] = valid_data_buffer[GYRO_RAW_Z_IDX + j];
		}

		#if defined(PIOS_INCLUDE_HMC5843) && defined(PIOS_INCLUDE_I2C)
			if (PIOS_HMC5843_NewDataAvailable())
			{
				PIOS_HMC5843_ReadMag(mag);

				mag_x = mag[MAG_RAW_X_IDX];
				mag_y = mag[MAG_RAW_Y_IDX];
				mag_z = mag[MAG_RAW_Z_IDX];
			}
		#endif

		// send the raw samples
		int result = PIOS_COM_SendBufferNonBlocking(PIOS_COM_AUX, framing, sizeof(framing));
		result += PIOS_COM_SendBufferNonBlocking(PIOS_COM_AUX, (uint8_t *)accel_x, over_sampling * sizeof(accel_x[0]));
		result += PIOS_COM_SendBufferNonBlocking(PIOS_COM_AUX, (uint8_t *)accel_y, over_sampling * sizeof(accel_y[0]));
		result += PIOS_COM_SendBufferNonBlocking(PIOS_COM_AUX, (uint8_t *)accel_z, over_sampling * sizeof(accel_z[0]));
		result += PIOS_COM_SendBufferNonBlocking(PIOS_COM_AUX, (uint8_t *)gyro_x, over_sampling * sizeof(gyro_x[0]));
		result += PIOS_COM_SendBufferNonBlocking(PIOS_COM_AUX, (uint8_t *)gyro_y, over_sampling * sizeof(gyro_y[0]));
		result += PIOS_COM_SendBufferNonBlocking(PIOS_COM_AUX, (uint8_t *)gyro_z, over_sampling * sizeof(gyro_z[0]));

		if (result != 0)
			PIOS_LED_On(LED1);          // all data not sent
		else
			PIOS_LED_Off(LED1);
	}
#endif

extern void PIOS_Board_Init(void);

/**
 * @brief AHRS Main function
 */
int main()
{
	gps_data.quality = -1;
	uint32_t up_time_real = 0;
	uint32_t up_time = 0;
	uint32_t last_up_time = 0;
	static int8_t last_ahrs_algorithm;
	uint32_t last_counter_idle_start = 0;
	uint32_t last_counter_idle_end = 0;
	uint32_t idle_counts = 0;
	uint32_t running_counts = 0;
	uint32_t counter_val = 0;
	ahrs_algorithm = AHRSSETTINGS_ALGORITHM_SIMPLE;

	reset_values();

	PIOS_Board_Init();

#if defined(PIOS_INCLUDE_HMC5843) && defined(PIOS_INCLUDE_I2C)
	// Get 3 ID bytes
	strcpy((char *)mag_data.id, "ZZZ");
	PIOS_HMC5843_ReadID(mag_data.id);
#endif
	
	while(!AhrsLinkReady()) {
		AhrsPoll();
	}
/* we didn't connect the callbacks before because we have to wait
for all data to be up to date before doing anything*/
	
	AHRSCalibrationConnectCallback(calibration_callback);
	GPSPositionConnectCallback(gps_callback);
	BaroAltitudeConnectCallback(altitude_callback);
	AHRSSettingsConnectCallback(settings_callback);
	HomeLocationConnectCallback(homelocation_callback);
	FirmwareIAPObjConnectCallback(firmwareiapobj_callback);

	calibration_callback(AHRSCalibrationHandle()); //force an update

#if defined(DUMP_RAW) || defined(PIP_DUMP_RAW)
	while (1) {
		AhrsPoll();
		print_ahrs_raw();
	}
#endif

	timer_start();

	/******************* Main EKF loop ****************************/
	while(1) {


		AhrsPoll();
		AhrsStatusData status;
		AhrsStatusGet(&status);
		status.CPULoad = ((float)running_counts /
						  (float)(idle_counts + running_counts)) * 100;
		status.IdleTimePerCyle = idle_counts / (timer_rate() / 10000);
		status.RunningTimePerCyle = running_counts / (timer_rate() / 10000);
		status.DroppedUpdates = ekf_too_slow;
		up_time = timer_count();
		if(up_time >= last_up_time) // normal condition
			up_time_real += ((up_time - last_up_time) * 1000) / timer_rate();
		else
			up_time_real += ((0xFFFF - last_up_time + up_time) * 1000) / timer_rate(); 
		last_up_time = up_time;
		status.RunningTime = up_time_real;
		AhrsStatusSet(&status);

		// Alive signal
		if (((total_conversion_blocks % 100) & 0xFFFE) == 0)
			PIOS_LED_Toggle(LED1);

		// Delay for valid data

		counter_val = timer_count();
		running_counts = counter_val - last_counter_idle_end;
		last_counter_idle_start = counter_val;

		// This function blocks till data avilable
		get_accel_gyro_data();

		// Get any mag data available
		process_mag_data();

		counter_val = timer_count();
		idle_counts = counter_val - last_counter_idle_start;
		last_counter_idle_end = counter_val;
		
		
		if(ISNAN(accel_data.filtered.x + accel_data.filtered.y + accel_data.filtered.z) ||
		   ISNAN(gyro_data.filtered.x + gyro_data.filtered.y + gyro_data.filtered.z) ||
		   ACCEL_OOB(accel_data.filtered.x + accel_data.filtered.y + accel_data.filtered.z) ||
		   GYRO_OOB(gyro_data.filtered.x + gyro_data.filtered.y + gyro_data.filtered.z)) {	
			// If any values are NaN or huge don't update
//TODO: add field to ahrs status to track number of these events
			continue;
		}
		
		print_ekf_binary();

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
 * @brief Get the accel and gyro data from whichever source when available
 *
 * This function will act as the HAL for the new INS sensors
 */
bool get_accel_gyro_data() 
{
	float accel[6];
	float gyro[6];
	uint16_t spin_count = 1;
	while(fifoBuf_getUsed(&adc_fifo_buffer) < (sizeof(accel) + sizeof(gyro))) {
		if(spin_count++ == 0) 
			AhrsPoll();
	}
	
	fifoBuf_getData(&adc_fifo_buffer, &accel[0], sizeof(float) * 3);
	fifoBuf_getData(&adc_fifo_buffer, &gyro[0], sizeof(float) * 3);	
	fifoBuf_getData(&adc_fifo_buffer, &accel[3], sizeof(float) * 3);
	fifoBuf_getData(&adc_fifo_buffer, &gyro[3], sizeof(float) * 3);	
	
	accel_data.filtered.x = (accel[0] + accel[3]) / 2;
	accel_data.filtered.y = (accel[1] + accel[4]) / 2;
	accel_data.filtered.z = (accel[2] + accel[5]) / 2;
	gyro_data.filtered.x = (gyro[0] + gyro[3]) / 2;
	gyro_data.filtered.y = (gyro[1] + gyro[4]) / 2;
	gyro_data.filtered.z = (gyro[2] + gyro[5]) / 2;
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
void adc_callback(float * downsampled_data)
{	
	AHRSSettingsData settings;
	AHRSSettingsGet(&settings);

#if 0
	// Get the Y data.  Third byte in.  Convert to m/s
	float accel_filtered[3];
	accel_filtered[1] = 0;
	for (i = 0; i < adc_oversampling; i++)
		accel_filtered[1] += valid_data_buffer[0 + i * PIOS_ADC_NUM_PINS] * fir_coeffs[i];
	accel_filtered[1] /= (float) fir_coeffs[adc_oversampling];
	// Get the X data which projects forward/backwards.  Fifth byte in.  Convert to m/s
	accel_filtered[0] = 0;
	for (i = 0; i < adc_oversampling; i++)
		accel_filtered[0] += valid_data_buffer[2 + i * PIOS_ADC_NUM_PINS] * fir_coeffs[i];
	accel_filtered[0] /= (float) fir_coeffs[adc_oversampling];

	// Get the Z data.  Third byte in.  Convert to m/s
	accel_filtered[2] = 0;
	for (i = 0; i < adc_oversampling; i++)
		accel_filtered[2] += valid_data_buffer[4 + i * PIOS_ADC_NUM_PINS] * fir_coeffs[i];
	accel_filtered[2] /= (float) fir_coeffs[adc_oversampling];

	float accel_scaled[3];
	calibration(accel_scaled, accel_data.calibration.scale, accel_filtered);
	accel_data.filtered.x = accel_scaled[0];
	accel_data.filtered.y = accel_scaled[1];
	accel_data.filtered.z = accel_scaled[2];
#else
	float accel[3], gyro[3];

	float raw_accel[3] = {
			downsampled_data[ACCEL_RAW_X_IDX],
			downsampled_data[ACCEL_RAW_Y_IDX],
			-downsampled_data[ACCEL_RAW_Z_IDX]
	};

	// Accel data is (y,x,z) in first third and fifth byte.  Convert to m/s^2
	calibration(accel, accel_data.calibration.scale, raw_accel);

	// Gyro data is (x,y,z) in second, fifth and seventh byte.  Convert to rad/s
	gyro[0] = ( ( downsampled_data[GYRO_RAW_X_IDX] + gyro_data.calibration.tempcompfactor[0] * downsampled_data[GYRO_TEMP_RAW_XY_IDX] ) * gyro_data.calibration.scale[0]) + gyro_data.calibration.bias[0];
	gyro[1] = ( ( downsampled_data[GYRO_RAW_Y_IDX] + gyro_data.calibration.tempcompfactor[1] * downsampled_data[GYRO_TEMP_RAW_XY_IDX] ) * gyro_data.calibration.scale[1]) + gyro_data.calibration.bias[1];
	gyro[2] = ( ( downsampled_data[GYRO_RAW_Z_IDX] + gyro_data.calibration.tempcompfactor[2] * downsampled_data[GYRO_TEMP_RAW_Z_IDX]  ) * gyro_data.calibration.scale[2]) + gyro_data.calibration.bias[2];

#endif
#if 0
	static float gravity_tracking[3] = {0,0,0};
	const float tau = 0.9999;
	gravity_tracking[0] = tau * gravity_tracking[0]  + (1-tau) * accel[0];
	gravity_tracking[1] = tau * gravity_tracking[1] + (1-tau) * accel[1];
	gravity_tracking[2] = tau * gravity_tracking[2] + (1-tau) * (accel[2] + 9.81);
	if(settings.BiasCorrectedRaw == AHRSSETTINGS_BIASCORRECTEDRAW_TRUE) {
		accel[0] -= gravity_tracking[0];
		accel[1] -= gravity_tracking[1];
		accel[2] -= gravity_tracking[2];
	}
#endif
	if(fifoBuf_getFree(&adc_fifo_buffer) >= (sizeof(accel) + sizeof(gyro))) {
		fifoBuf_putData(&adc_fifo_buffer, (uint8_t *) &accel[0], sizeof(accel));
		fifoBuf_putData(&adc_fifo_buffer, (uint8_t *) &gyro[0], sizeof(gyro));
	} else {
		ekf_too_slow++;
	}
	
	AttitudeRawData raw;

	raw.gyrotemp[0] = downsampled_data[GYRO_TEMP_RAW_XY_IDX];
	raw.gyrotemp[1] = downsampled_data[GYRO_TEMP_RAW_Z_IDX];

	raw.gyros[0] = gyro[0] * RAD_TO_DEG;
	raw.gyros[1] = gyro[1] * RAD_TO_DEG;
	raw.gyros[2] = gyro[2] * RAD_TO_DEG;

	raw.accels[0] = accel[0];
	raw.accels[1] = accel[1];
	raw.accels[2] = accel[2];

	raw.magnetometers[0] = mag_data.scaled.axis[0];
	raw.magnetometers[1] = mag_data.scaled.axis[1];
	raw.magnetometers[2] = mag_data.scaled.axis[2];

	if (settings.BiasCorrectedRaw == AHRSSETTINGS_BIASCORRECTEDRAW_TRUE)
	{
		raw.gyros[0] -= Nav.gyro_bias[0] * RAD_TO_DEG;
		raw.gyros[1] -= Nav.gyro_bias[1] * RAD_TO_DEG;
		raw.gyros[2] -= Nav.gyro_bias[2] * RAD_TO_DEG;

		raw.accels[0] -= Nav.accel_bias[0];
		raw.accels[1] -= Nav.accel_bias[1];
		raw.accels[2] -= Nav.accel_bias[2];
	}

	AttitudeRawSet(&raw);
	
	total_conversion_blocks++;
}

#if defined(PIOS_INCLUDE_HMC5843) && defined(PIOS_INCLUDE_I2C)
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
	if (PIOS_HMC5843_NewDataAvailable()) {
		PIOS_HMC5843_ReadMag(mag_data.raw.axis);

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

#if defined(PIOS_INCLUDE_HMC5843) && defined(PIOS_INCLUDE_I2C)
		if(PIOS_HMC5843_NewDataAvailable()) {
			j ++;
			PIOS_HMC5843_ReadMag(mag_data.raw.axis);
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

#if defined(PIOS_INCLUDE_HMC5843) && defined(PIOS_INCLUDE_I2C)
		if(PIOS_HMC5843_NewDataAvailable()) {
			j ++;
			PIOS_HMC5843_ReadMag(mag_data.raw.axis);
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
 * @brief AHRS calibration callback
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

void gps_callback(AhrsObjHandle obj)
{
	GPSPositionData pos;
	GPSPositionGet(&pos);
	HomeLocationData home;
	HomeLocationGet(&home);

	// convert from cm back to meters
	double LLA[3] = {(double) pos.Latitude / 1e7, (double) pos.Longitude / 1e7, (double) (pos.GeoidSeparation + pos.Altitude)};
	// put in local NED frame
	double ECEF[3] = {(double) (home.ECEF[0] / 100), (double) (home.ECEF[1] / 100), (double) (home.ECEF[2] / 100)};
	LLA2Base(LLA, ECEF, (float (*)[3]) home.RNE, gps_data.NED);

	gps_data.heading = pos.Heading;
	gps_data.groundspeed = pos.Groundspeed;
	gps_data.quality = 1;  /* currently unused */
	gps_data.updated = true;

	// if poor don't use this update
	if((ahrs_algorithm != AHRSSETTINGS_ALGORITHM_INSGPS_OUTDOOR) ||
	   (pos.Satellites < INSGPS_GPS_MINSAT) || 
	   (pos.PDOP >= INSGPS_GPS_MINPDOP) || 
	   (home.Set == FALSE) ||
	   ((home.ECEF[0] == 0) && (home.ECEF[1] == 0) && (home.ECEF[2] == 0)))
	{
		gps_data.quality = 0;
		gps_data.updated = false;
	}

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

	ahrs_algorithm = settings.Algorithm;
	
	if(settings.Downsampling != adc_oversampling) {
		adc_oversampling = settings.Downsampling;
		PIOS_ADC_Config(adc_oversampling);				
	}	
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
	FirmwareIAPObjData firmwareIAPObj;
	FirmwareIAPObjGet(&firmwareIAPObj);
	CRC_Ini();
	if(firmwareIAPObj.ArmReset==0)
		reset_count=0;
	if(firmwareIAPObj.ArmReset==1)
	{

		if((firmwareIAPObj.BoardType==BOARD_TYPE) || (firmwareIAPObj.BoardType==0xFF))
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
	else if(firmwareIAPObj.BoardType==BOARD_TYPE && firmwareIAPObj.crc!=FLASH_crc_memory_calc())
	{
		FLASH_read_description(firmwareIAPObj.Description,SIZE_OF_DESCRIPTION);
		CRC_Ini();
		firmwareIAPObj.crc=FLASH_crc_memory_calc();
		firmwareIAPObj.BoardRevision=BOARD_REVISION;
		FirmwareIAPObjSet(&firmwareIAPObj);
	}
}

/**
 * @}
 */


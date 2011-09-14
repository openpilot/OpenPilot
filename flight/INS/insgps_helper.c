
#include "ins.h"
#include "pios.h"
#include "ahrs_spi_comm.h"
#include "insgps.h"
#include "CoordinateConversions.h"

#define DEG_TO_RAD         (M_PI / 180.0)
#define RAD_TO_DEG         (180.0 / M_PI)

#define INSGPS_GPS_TIMEOUT 2   /* 2 seconds triggers reinit of position */
#define INSGPS_GPS_MINSAT  6   /* 2 seconds triggers reinit of position */
#define INSGPS_GPS_MINPDOP 3.5 /* minimum PDOP for postition updates    */
 /* If GPS is more than this distance on any dimension then wait a few updates */
 /* and reinitialize there */
#define INSGPS_GPS_FAR     10 

//! Contains the data from the mag sensor chip
extern struct mag_sensor mag_data;

//! Contains the data from the accelerometer
extern struct accel_sensor  accel_data;

//! Contains the data from the gyro
extern struct gyro_sensor gyro_data;

//! Conains the current estimate of the attitude
extern struct attitude_solution attitude_data;

//! Contains data from the altitude sensor
extern struct altitude_sensor altitude_data;

//! Contains data from the GPS (via the SPI link)
extern struct gps_sensor gps_data;

//! Offset correction of barometric alt, to match gps data
static float baro_offset = 0;

extern void send_calibration(void);
extern void send_attitude(void);
extern void send_velocity(void);
extern void send_position(void);
extern volatile int8_t ahrs_algorithm;
extern void get_accel_gyro_data();
extern void get_mag_data();

/* INS functions */
/**
 * @brief Update the EKF when in outdoor mode.  The primary difference is using the GPS values.
 */
uint32_t total_far_count = 0;
uint32_t relocated = 0;
void ins_outdoor_update()
{
	static uint32_t ins_last_time;
	float gyro[3], accel[3], vel[3];
	float dT;
	uint16_t sensors;
	static uint32_t gps_far_count = 0;
	
	dT = PIOS_DELAY_DiffuS(ins_last_time) / 1e6;
	ins_last_time = PIOS_DELAY_GetRaw();
	
	// This should only happen at start up or at mode switches
	if(dT > 0.01)
		dT = 0.01;
		
	// format data for INS algo
	gyro[0] = gyro_data.filtered.x;
	gyro[1] = gyro_data.filtered.y;
	gyro[2] = gyro_data.filtered.z;
	accel[0] = accel_data.filtered.x,
	accel[1] = accel_data.filtered.y,
	accel[2] = accel_data.filtered.z,
	
	INSStatePrediction(gyro, accel, dT);
	attitude_data.quaternion.q1 = Nav.q[0];
	attitude_data.quaternion.q2 = Nav.q[1];
	attitude_data.quaternion.q3 = Nav.q[2];
	attitude_data.quaternion.q4 = Nav.q[3];
	send_attitude();  // get message out quickly
	INSCovariancePrediction(dT);

	PositionActualData positionActual;
	PositionActualGet(&positionActual);
	positionActual.North = Nav.Pos[0];
	positionActual.East = Nav.Pos[1];
	positionActual.Down = Nav.Pos[2];
	PositionActualSet(&positionActual);
	
	VelocityActualData velocityActual;
	VelocityActualGet(&velocityActual);
	velocityActual.North = Nav.Vel[0];
	velocityActual.East = Nav.Vel[1];
	velocityActual.Down = Nav.Vel[2];
	VelocityActualSet(&velocityActual);

	sensors = 0;
	
	if (gps_data.updated)
	{
		vel[0] = gps_data.groundspeed * cos(gps_data.heading * DEG_TO_RAD);
		vel[1] = gps_data.groundspeed * sin(gps_data.heading * DEG_TO_RAD);
		vel[2] = 0;
		
		if (abs(gps_data.NED[0] - Nav.Pos[0]) > INSGPS_GPS_FAR ||
		    abs(gps_data.NED[1] - Nav.Pos[1]) > INSGPS_GPS_FAR ||
		    abs(gps_data.NED[2] - Nav.Pos[2]) > INSGPS_GPS_FAR ||
		    abs(vel[0] - Nav.Vel[0]) > INSGPS_GPS_FAR ||
		    abs(vel[1] - Nav.Vel[1]) > INSGPS_GPS_FAR) {
			gps_far_count++;
			total_far_count++;
			gps_data.updated = false;
			
			if(gps_far_count > 30) {
				INSPosVelReset(gps_data.NED,vel);
				relocated++;
			}
		}
		else {			
			sensors |= HORIZ_SENSORS | POS_SENSORS;
			
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
		}
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
	
	if(fabs(Nav.gyro_bias[0]) > 0.1 || fabs(Nav.gyro_bias[1]) > 0.1 || fabs(Nav.gyro_bias[2]) > 0.1) {
		float zeros[3] = {0,0,0};
		INSSetGyroBias(zeros);
	}
}

/**
 * @brief Update the EKF when in indoor mode
 */
void ins_indoor_update()
{
	static uint32_t updated_without_gps = 0;
	
	float gyro[3], accel[3];
	float zeros[3] = {0, 0, 0};
	static uint32_t ins_last_time = 0;
	uint16_t sensors = 0;
	float dT;

	dT = PIOS_DELAY_DiffuS(ins_last_time) / 1e6;
	ins_last_time = PIOS_DELAY_GetRaw();
	
	// This should only happen at start up or at mode switches
	if(dT > 0.01)
		dT = 0.01;

	// format data for INS algo
	gyro[0] = gyro_data.filtered.x;
	gyro[1] = gyro_data.filtered.y;
	gyro[2] = gyro_data.filtered.z;
	accel[0] = accel_data.filtered.x,
	accel[1] = accel_data.filtered.y,
	accel[2] = accel_data.filtered.z,
	
	INSStatePrediction(gyro, accel, dT);
	attitude_data.quaternion.q1 = Nav.q[0];
	attitude_data.quaternion.q2 = Nav.q[1];
	attitude_data.quaternion.q3 = Nav.q[2];
	attitude_data.quaternion.q4 = Nav.q[3];
	send_attitude();  // get message out quickly
	INSCovariancePrediction(dT);
	
	/* Indoors, update with zero position and velocity and high covariance */
	sensors = HORIZ_SENSORS | VERT_SENSORS;
	
	if(mag_data.updated && (ahrs_algorithm == INSSETTINGS_ALGORITHM_INSGPS_INDOOR)) {
		sensors |= MAG_SENSORS;
		mag_data.updated = false;
	}
	
	if(altitude_data.updated) {
		sensors |= BARO_SENSOR;
		altitude_data.updated = false;
	}
	
	if(gps_data.updated) {
		PositionActualData positionActual;
		PositionActualGet(&positionActual);
		positionActual.North = gps_data.NED[0];
		positionActual.East = gps_data.NED[1];
		positionActual.Down = Nav.Pos[2];
		PositionActualSet(&positionActual);
		
		VelocityActualData velocityActual;
		VelocityActualGet(&velocityActual);
		velocityActual.North = gps_data.groundspeed * cos(gps_data.heading * DEG_TO_RAD);
		velocityActual.East = gps_data.groundspeed * sin(gps_data.heading * DEG_TO_RAD);
		velocityActual.Down = Nav.Vel[2];
		VelocityActualSet(&velocityActual);
		
		updated_without_gps = 0;
		gps_data.updated = false;
	} else {
		PositionActualData positionActual;
		PositionActualGet(&positionActual);

		VelocityActualData velocityActual;
		VelocityActualGet(&velocityActual);

		positionActual.Down = Nav.Pos[2];
		velocityActual.Down = Nav.Vel[2];

		if(updated_without_gps > 500) {
			// After 2-3 seconds without a GPS update set velocity estimate to NAN
			positionActual.North = NAN;
			positionActual.East = NAN;
			velocityActual.North = NAN;
			velocityActual.East = NAN;
		} else
			updated_without_gps++;

		PositionActualSet(&positionActual);
		VelocityActualSet(&velocityActual);
	}
	
	/*
	 * TODO: Need to add a general sanity check for all the inputs to make sure their kosher
	 * although probably should occur within INS itself
	 */
	INSCorrection(mag_data.scaled.axis, zeros, zeros, altitude_data.altitude, sensors);
	
	if(fabs(Nav.gyro_bias[0]) > 0.1 || fabs(Nav.gyro_bias[1]) > 0.1 || fabs(Nav.gyro_bias[2]) > 0.1) {
		float zeros[3] = {0,0,0};
		INSSetGyroBias(zeros);
	}

}

/**
 * @brief Initialize the EKF assuming stationary
 */
bool inited = false;
float init_q[4];
void ins_init_algorithm()
{
	inited = true;
	float Rbe[3][3], q[4], accels[3], rpy[3], mag;
	float ge[3]={0,0,-9.81}, zeros[3]={0,0,0}, Pdiag[16]={25,25,25,5,5,5,1e-5,1e-5,1e-5,1e-5,1e-5,1e-5,1e-5,1e-4,1e-4,1e-4};
	bool using_mags, using_gps;
	
	INSGPSInit();
	
	HomeLocationData home;
	HomeLocationGet(&home);
	
	accels[0]=accel_data.filtered.x;
	accels[1]=accel_data.filtered.y;
	accels[2]=accel_data.filtered.z;
	
	using_mags = (ahrs_algorithm == INSSETTINGS_ALGORITHM_INSGPS_OUTDOOR) || (ahrs_algorithm == INSSETTINGS_ALGORITHM_INSGPS_INDOOR);
	using_mags &= (home.Be[0] != 0) || (home.Be[1] != 0) || (home.Be[2] != 0);  /* only use mags when valid home location */
	
	using_gps = (ahrs_algorithm == INSSETTINGS_ALGORITHM_INSGPS_OUTDOOR) && (gps_data.quality >= INSGPS_GPS_MINSAT);
	
	/* Block till a data update */
	get_accel_gyro_data();
	
	/* Ensure we get mag data in a timely manner */
	uint16_t fail_count = 50; // 50 at 200 Hz is up to 0.25 sec
	while(using_mags && !mag_data.updated && fail_count--) {
		get_mag_data();
		get_accel_gyro_data();
		AhrsPoll();
		PIOS_DELAY_WaituS(2000);
	}
	using_mags &= mag_data.updated;
	
	if (using_mags) {		
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
		RPY2Quaternion(rpy,init_q);
		if (using_gps)
			INSSetState(gps_data.NED, zeros, init_q, zeros, zeros);
		else {
			for (uint32_t i = 0; i < 5; i++) {
				INSSetState(zeros, zeros, init_q, zeros, zeros);
				ins_indoor_update();
			}
		}
	}
	
	INSResetP(Pdiag);
	
	// TODO: include initial estimate of gyro bias?
}

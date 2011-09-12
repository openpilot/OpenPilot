
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

#define timer_rate() 100000
#define timer_count() 1

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

static uint32_t ins_last_time;

/* INS functions */
/**
 * @brief Update the EKF when in outdoor mode.  The primary difference is using the GPS values.
 */

void ins_outdoor_update()
{
	float gyro[3], accel[3], vel[3];
	static uint32_t last_gps_time = 0;
	float dT;
	uint16_t sensors;
	
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

uint32_t time_val_a;
uint32_t indoor_time;
/**
 * @brief Update the EKF when in indoor mode
 */
void ins_indoor_update()
{
	time_val_a = PIOS_DELAY_GetRaw();
	
	float gyro[3], accel[3], vel[3];
	static uint32_t last_indoor_time = 0;
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

		gps_data.updated = false;
	} else {
		PositionActualData positionActual;
		PositionActualGet(&positionActual);
		positionActual.North = NAN;
		positionActual.East = NAN;
		positionActual.Down = Nav.Pos[2];
		PositionActualSet(&positionActual);

		VelocityActualData velocityActual;
		VelocityActualGet(&velocityActual);
		velocityActual.North = NAN;
		velocityActual.East = NAN;
		velocityActual.Down = Nav.Vel[2];
		VelocityActualSet(&velocityActual);
	}
	
	/*
	 * TODO: Need to add a general sanity check for all the inputs to make sure their kosher
	 * although probably should occur within INS itself
	 */
	INSCorrection(mag_data.scaled.axis, gps_data.NED, vel, altitude_data.altitude, sensors | HORIZ_SENSORS | VERT_SENSORS);
	indoor_time = PIOS_DELAY_DiffuS(time_val_a);
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
	
	using_gps = (ahrs_algorithm == INSSETTINGS_ALGORITHM_INSGPS_OUTDOOR) && (gps_data.quality != 0);
	
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

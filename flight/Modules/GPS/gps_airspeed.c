/**
 ******************************************************************************
 * @addtogroup OpenPilotModules OpenPilot Modules
 * @{ 
 * @addtogroup AirspeedModule Airspeed Module
 * @brief Use GPS data to estimate airspeed
 * @{ 
 *
 * @file       gps_airspeed.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2012.
 * @brief      Airspeed module, handles temperature and pressure readings from BMP085
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


#include "openpilot.h"
#include "gps_airspeed.h"
#include "airspeedactual.h"
#include "attitudeactual.h"
#include "CoordinateConversions.h"

// Private constants
#define GPS_AIRSPEED_BIAS_KP           0.1f   //Needs to be settable in a UAVO
#define GPS_AIRSPEED_BIAS_KI           0.1f   //Needs to be settable in a UAVO
#define SAMPLING_DELAY_MS_GPS          100    //Needs to be settable in a UAVO
#define GPS_AIRSPEED_TIME_CONSTANT_MS  500.0f //Needs to be settable in a UAVO

#define STANDARD_AIR_DENSITY 1.225f //Density of sea level air, at 15C, 20% relative humidy, in [kg/m^3].
#define F_PI 3.141526535897932f
#define DEG2RAD (F_PI/180.0f)

// Private types
struct GPSGlobals {
	float RbeCol1_old[3];
	float gpsVelOld_N;
	float gpsVelOld_E;
	float gpsVelOld_D;
};

// Private variables
static struct GPSGlobals *gps;

// Private functions

static void compute_rbe(float Rbe[3][3])
{
	AttitudeActualData attData;
	AttitudeActualGet(&attData);

	float q[4] = {
		attData.q1,
		attData.q2,
		attData.q3,
		attData.q4
	};

	//Calculate rotation matrix
	Quaternion2R(q, Rbe);
}


/*
 * Initialize function loads first data sets, and allocates memory for structure.
 */
void gps_airspeed_initialize(void)
{
	//This method saves memory in case we don't use the GPS module.
	gps = (struct GPSGlobals *)pvPortMalloc(sizeof(struct GPSGlobals));

	gps->gpsVelOld_N = 0;
	gps->gpsVelOld_E = 0;
	gps->gpsVelOld_D = 0;

	float Rbe[3][3];
	compute_rbe(Rbe);

	gps->RbeCol1_old[0] = Rbe[0][0];
	gps->RbeCol1_old[1] = Rbe[0][1];
	gps->RbeCol1_old[2] = Rbe[0][2];
}

/*
 * Calculate airspeed as a function of GPS groundspeed and vehicle attitude.
 *  From "IMU Wind Estimation (Theory)", by William Premerlani.
 *  The idea is that V_gps=V_air+V_wind. If we assume wind constant, => 
 *  V_gps_2-V_gps_1 = (V_air_2+V_wind_2) -(V_air_1+V_wind_1) = V_air_2 - V_air_1.
 *  If we assume airspeed constant, => V_gps_2-V_gps_1 = |V|*(f_2 - f1),
 *  where "f" is the fuselage vector in earth coordinates.
 *  We then solve for |V| = |V_gps_2-V_gps_1|/ |f_2 - f1|.
 */
void gps_airspeed_update(const GPSVelocityData *gpsVelData, float staticAirDensity)
{	
	float Rbe[3][3];
	compute_rbe(Rbe);

	//Calculate the cos(angle) between the two fuselage basis vectors
	float cosDiff = (Rbe[0][0] * gps->RbeCol1_old[0]) + 
			(Rbe[0][1] * gps->RbeCol1_old[1]) +
			(Rbe[0][2] * gps->RbeCol1_old[2]);

	//If there's more than a 5 degree difference between two fuselage measurements, then we have sufficient delta to continue.
	if (fabs(cosDiff) < cos(5.0f * DEG2RAD)) {
		float gps_airspeed;

		GPSVelocityData gpsVelData;
		GPSVelocityGet(&gpsVelData);

		//Calculate the norm^2 of the difference between the two GPS vectors
		float normDiffGPS2 = powf(gpsVelData.North - gps->gpsVelOld_N, 2.0f) +
			             powf(gpsVelData.East  - gps->gpsVelOld_E, 2.0f) +
			             powf(gpsVelData.Down  - gps->gpsVelOld_D, 2.0f);

		//Calculate the norm^2 of the difference between the two fuselage vectors
		float normDiffAttitude2 = powf(Rbe[0][0] - gps->RbeCol1_old[0], 2.0f) +
			                  powf(Rbe[0][1] - gps->RbeCol1_old[1], 2.0f) +
			                  powf(Rbe[0][2] - gps->RbeCol1_old[2], 2.0f);

		//Airspeed magnitude is the ratio between the two difference norms
		gps_airspeed = sqrtf(normDiffGPS2 / normDiffAttitude2);

		//Check to see if gps airspeed estimate is reasonable (remember, it can only be positive, so only need to check upper limit)
		if (gps_airspeed < 300) { //NEED TO SATURATE, BUT NOT VERY GOOD TO SATURATE LIKE THIS. PROBABLY SHOULD THROW OUT ANY READINGS THAT ARE TOO FAR OUTSIDE THE CURRENT SPEED
			//Save old variables for next pass
			gps->gpsVelOld_N = gpsVelData.North;
			gps->gpsVelOld_E = gpsVelData.East;
			gps->gpsVelOld_D = gpsVelData.Down;
			
			gps->RbeCol1_old[0] = Rbe[0][0];
			gps->RbeCol1_old[1] = Rbe[0][1];
			gps->RbeCol1_old[2] = Rbe[0][2];
			
			//Low pass filter
			const float alpha = .2;
			float gps_airspeed_old;
			AirspeedActualTrueAirspeedGet(&gps_airspeed_old);			
			
			gps_airspeed=gps_airspeed*alpha + (1-alpha)*gps_airspeed_old;
			
			// Do not update airspeed data in simulation mode
			if (!AirspeedActualReadOnly()) {
				AirspeedActualTrueAirspeedSet(&gps_airspeed);

				//Calculate calibrated airspeed, http://en.wikipedia.org/wiki/True_airspeed
				gps_airspeed*=sqrtf(staticAirDensity/STANDARD_AIR_DENSITY);
				AirspeedActualCalibratedAirspeedSet(&gps_airspeed);
			}
		}
		else {
			// Do not update airspeed data in simulation mode
			if (!AirspeedActualReadOnly()) {
				AirspeedActualTrueAirspeedGet(&gps_airspeed); //<--Why do we get the airspeed just to set it? So that the object is listed as updated?
				AirspeedActualTrueAirspeedSet(&gps_airspeed);
			}
		}

	}
}



/**
 * @}
 * @}
 */

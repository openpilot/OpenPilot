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
#include "gpsvelocity.h"
#include "attitudeactual.h"
#include "CoordinateConversions.h"



// Private constants
#define GPS_AIRSPEED_BIAS_KP           0.1f   //Needs to be settable in a UAVO
#define GPS_AIRSPEED_BIAS_KI           0.1f   //Needs to be settable in a UAVO
#define SAMPLING_DELAY_MS_GPS          100    //Needs to be settable in a UAVO
#define GPS_AIRSPEED_TIME_CONSTANT_MS  500.0f //Needs to be settable in a UAVO

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

/*
 * Initialize function loads first data sets, and allocates memory for structure.
 */
void gps_airspeedInitialize()
{
	//This method saves memory in case we don't use the GPS module.
	gps=(struct GPSGlobals *)pvPortMalloc(sizeof(struct GPSGlobals));
	
	//GPS airspeed calculation variables
	GPSVelocityData gpsVelData;
	GPSVelocityGet(&gpsVelData);
	
	gps->gpsVelOld_N=gpsVelData.North;
	gps->gpsVelOld_E=gpsVelData.East;
	gps->gpsVelOld_D=gpsVelData.Down;
	
	AttitudeActualData attData;
	AttitudeActualGet(&attData);
	
	float Rbe[3][3];
	float q[4] ={attData.q1, attData.q2, attData.q3, attData.q4};
	
	//Calculate rotation matrix
	Quaternion2R(q, Rbe);
	
	gps->RbeCol1_old[0]=Rbe[0][0];
	gps->RbeCol1_old[1]=Rbe[0][1];
	gps->RbeCol1_old[2]=Rbe[0][2];
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
void gps_airspeedGet(float *v_air_GPS)
{	
	float Rbe[3][3];
	
	{ //Scoping to save memory. We really just need Rbe.
		AttitudeActualData attData;
		AttitudeActualGet(&attData);
		
		float q[4] ={attData.q1, attData.q2, attData.q3, attData.q4};
		
		//Calculate rotation matrix
		Quaternion2R(q, Rbe);
	}
	
	//Calculate the cos(angle) between the two fuselage basis vectors
	float cosDiff=(Rbe[0][0]*gps->RbeCol1_old[0]) + (Rbe[0][1]*gps->RbeCol1_old[1]) + (Rbe[0][2]*gps->RbeCol1_old[2]);
	
	//If there's more than a 5 degree difference between two fuselage measurements, then we have sufficient delta to continue.
	if (fabs(cosDiff) < cos(5.0f*DEG2RAD)) {
		GPSVelocityData gpsVelData;
		GPSVelocityGet(&gpsVelData);
		
		//Calculate the norm^2 of the difference between the two GPS vectors
		float normDiffGPS2 = powf(gpsVelData.North-gps->gpsVelOld_N,2.0f) + powf(gpsVelData.East-gps->gpsVelOld_E,2.0f) + powf(gpsVelData.Down-gps->gpsVelOld_D,2.0f);

		//Calculate the norm^2 of the difference between the two fuselage vectors
		float normDiffAttitude2=powf(Rbe[0][0]-gps->RbeCol1_old[0],2.0f) + powf(Rbe[0][1]-gps->RbeCol1_old[1],2.0f) + powf(Rbe[0][2]-gps->RbeCol1_old[2],2.0f);
		
		//Airspeed magnitude is the ratio between the two difference norms
		*v_air_GPS = sqrtf(normDiffGPS2/normDiffAttitude2);
		
		//Save old variables for next pass
		gps->gpsVelOld_N=gpsVelData.North;
		gps->gpsVelOld_E=gpsVelData.East;
		gps->gpsVelOld_D=gpsVelData.Down;
		
		gps->RbeCol1_old[0]=Rbe[0][0];
		gps->RbeCol1_old[1]=Rbe[0][1];
		gps->RbeCol1_old[2]=Rbe[0][2];
	}
	else {
		
	}

	
}



/**
 * @}
 * @}
 */

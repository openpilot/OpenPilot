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
#include "velocitystate.h"
#include "attitudestate.h"
#include "airspeedsensor.h"
#include "airspeedsettings.h"
#include "gps_airspeed.h"
#include "CoordinateConversions.h"
#include <pios_math.h>


// Private constants
#define GPS_AIRSPEED_BIAS_KP          0.1f   // Needs to be settable in a UAVO
#define GPS_AIRSPEED_BIAS_KI          0.1f   // Needs to be settable in a UAVO
#define SAMPLING_DELAY_MS_GPS         100    // Needs to be settable in a UAVO
#define GPS_AIRSPEED_TIME_CONSTANT_MS 500.0f // Needs to be settable in a UAVO
#define COSINE_OF_5_DEG               0.9961947f

// Private types
struct GPSGlobals {
    float RbeCol1_old[3];
    float gpsVelOld_N;
    float gpsVelOld_E;
    float gpsVelOld_D;
    float oldAirspeed;
};

// Private variables
static struct GPSGlobals *gps;

// Private functions
// a simple square inline function based on multiplication faster than powf(x,2.0f)
static inline float Sq(float x)
{
    return x * x;
}


/*
 * Initialize function loads first data sets, and allocates memory for structure.
 */
void gps_airspeedInitialize()
{
    // This method saves memory in case we don't use the GPS module.
    gps = (struct GPSGlobals *)pvPortMalloc(sizeof(struct GPSGlobals));

    // GPS airspeed calculation variables
    VelocityStateInitialize();
    VelocityStateData gpsVelData;
    VelocityStateGet(&gpsVelData);

    gps->gpsVelOld_N = gpsVelData.North;
    gps->gpsVelOld_E = gpsVelData.East;
    gps->gpsVelOld_D = gpsVelData.Down;

    gps->oldAirspeed = 0.0f;

    AttitudeStateData attData;
    AttitudeStateGet(&attData);

    float Rbe[3][3];
    float q[4] = { attData.q1, attData.q2, attData.q3, attData.q4 };

    // Calculate rotation matrix
    Quaternion2R(q, Rbe);

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
/* Remark regarding "IMU Wind Estimation": The approach includes errors when |V| is
 * not constant, i.e. when the change in V_gps does not solely come from a reorientation
 * this error depends strongly on the time scale considered. Is the time between t1 and t2 too
 * small, "spikes" absorving unconsidred acceleration will arise
 */
void gps_airspeedGet(AirspeedSensorData *airspeedData, AirspeedSettingsData *airspeedSettings)
{
    float Rbe[3][3];

    { // Scoping to save memory. We really just need Rbe.
        AttitudeStateData attData;
        AttitudeStateGet(&attData);

        float q[4] = { attData.q1, attData.q2, attData.q3, attData.q4 };

        // Calculate rotation matrix
        Quaternion2R(q, Rbe);
    }

    // Calculate the cos(angle) between the two fuselage basis vectors
    float cosDiff = (Rbe[0][0] * gps->RbeCol1_old[0]) + (Rbe[0][1] * gps->RbeCol1_old[1]) + (Rbe[0][2] * gps->RbeCol1_old[2]);

    // If there's more than a 5 degree difference between two fuselage measurements, then we have sufficient delta to continue.
    if (fabsf(cosDiff) < COSINE_OF_5_DEG) {
        VelocityStateData gpsVelData;
        VelocityStateGet(&gpsVelData);

        if (gpsVelData.North * gpsVelData.North + gpsVelData.East * gpsVelData.East + gpsVelData.Down * gpsVelData.Down < 1.0f) {
            airspeedData->CalibratedAirspeed = 0;
            airspeedData->SensorConnected    = AIRSPEEDSENSOR_SENSORCONNECTED_FALSE;
            AlarmsSet(SYSTEMALARMS_ALARM_AIRSPEED, SYSTEMALARMS_ALARM_WARNING);
            return; // do not calculate if gps velocity is insufficient...
        }

        // Calculate the norm^2 of the difference between the two GPS vectors
        float normDiffGPS2 = Sq(gpsVelData.North - gps->gpsVelOld_N) + Sq(gpsVelData.East - gps->gpsVelOld_E) + Sq(gpsVelData.Down - gps->gpsVelOld_D);

        // Calculate the norm^2 of the difference between the two fuselage vectors
        float normDiffAttitude2 = Sq(Rbe[0][0] - gps->RbeCol1_old[0]) + Sq(Rbe[0][1] - gps->RbeCol1_old[1]) + Sq(Rbe[0][2] - gps->RbeCol1_old[2]);

        // Airspeed magnitude is the ratio between the two difference norms
        float airspeed = sqrtf(normDiffGPS2 / normDiffAttitude2);
        if (!IS_REAL(airspeed)) {
            airspeedData->CalibratedAirspeed = 0;
            airspeedData->SensorConnected    = AIRSPEEDSENSOR_SENSORCONNECTED_FALSE;
            AlarmsSet(SYSTEMALARMS_ALARM_AIRSPEED, SYSTEMALARMS_ALARM_WARNING);
        } else {
            // need a low pass filter to filter out spikes in non coordinated maneuvers
            airspeedData->CalibratedAirspeed = (1.0f - airspeedSettings->GroundSpeedBasedEstimationLowPassAlpha) * gps->oldAirspeed + airspeedSettings->GroundSpeedBasedEstimationLowPassAlpha * airspeed;
            gps->oldAirspeed = airspeedData->CalibratedAirspeed;
            airspeedData->SensorConnected    = AIRSPEEDSENSOR_SENSORCONNECTED_TRUE;
            AlarmsSet(SYSTEMALARMS_ALARM_AIRSPEED, SYSTEMALARMS_ALARM_OK);
        }

        // Save old variables for next pass
        gps->gpsVelOld_N    = gpsVelData.North;
        gps->gpsVelOld_E    = gpsVelData.East;
        gps->gpsVelOld_D    = gpsVelData.Down;

        gps->RbeCol1_old[0] = Rbe[0][0];
        gps->RbeCol1_old[1] = Rbe[0][1];
        gps->RbeCol1_old[2] = Rbe[0][2];
    }
}


/**
 * @}
 * @}
 */

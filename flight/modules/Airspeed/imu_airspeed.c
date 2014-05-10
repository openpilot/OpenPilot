/**
 ******************************************************************************
 * @addtogroup OpenPilotModules OpenPilot Modules
 * @{
 * @addtogroup AirspeedModule Airspeed Module
 * @brief Use attitude and velocity data to estimate airspeed
 * @{
 *
 * @file       imu_airspeed.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2012.
 * @brief      IMU based airspeed calculation
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
#include "imu_airspeed.h"
#include "CoordinateConversions.h"
#include <pios_math.h>


// Private constants
#define EPS_REORIENTATION 1e-8f
#define EPS_VELOCITY      1.f

// Private types
// structure with smoothed fuselage orientation, ground speed and their changes in time
struct IMUGlobals {
    float xB[3];
    float dxB[3];
    float Vel[3];
    float dVel[3];
};

// Private variables
static struct IMUGlobals *imu;

// Private functions
// a simple square inline function based on multiplication faster than powf(x,2.0f)
static inline float Sq(float x)
{
    return x * x;
}

/*
 * Initialize function loads first data sets, and allocates memory for structure.
 */
void imu_airspeedInitialize()
{
    // This method saves memory in case we don't use the module.
    imu = (struct IMUGlobals *)pvPortMalloc(sizeof(struct IMUGlobals));

    // airspeed calculation variables
    VelocityStateInitialize();
    VelocityStateData velData;
    VelocityStateGet(&velData);

    AttitudeStateData attData;
    AttitudeStateGet(&attData);

    // for Holt-Winters double exponential smoothing (s smooth variable, b smooth trend)
    // s1 = x1
    // b1 = x1 - x0
    // Calculate x of body frame
    QuaternionC2xB(attData.q1, attData.q2, attData.q3, attData.q4, imu->xB);

    // ground speed
    imu->Vel[0]  = velData.North;
    imu->Vel[1]  = velData.East;
    imu->Vel[2]  = velData.Down;

    // trend assumed to be zero
    imu->dxB[0]  = imu->dxB[1] = imu->dxB[2] = 0.f;
    imu->dVel[0] = imu->dVel[1] = imu->dVel[2] = 0.f;
}

/*
 * Calculate airspeed as a function of groundspeed and vehicle attitude.
 *  Adapted from "IMU Wind Estimation (Theory)", by William Premerlani.
 *  The idea is that V_gps=V_air+V_wind. If we assume wind constant, =>
 *  V_gps_2-V_gps_1 = (V_air_2+V_wind_2) -(V_air_1+V_wind_1) = V_air_2 - V_air_1.
 *  If we assume airspeed constant, => V_gps_2-V_gps_1 = |V|*(f_2 - f1),
 *  where "f" is the fuselage vector in earth coordinates.
 *  We then solve for |V| = |V_gps_2-V_gps_1|/ |f_2 - f1|.
 *  Adapted to: |V| = (V_gps_2-V_gps_1) dot (f2_-f_1) / |f_2 - f1|^2.
 *
 * See OP-1317 imu_wind_estimation.pdf for details on the adaptation
 * Need a low pass filter to filter out spikes in non coordinated maneuvers
 * Note: filtering of xB and gpsV is more effective than of the airspeed itself. Reason: derivative of oscillating part is scaled
 * by 1/period, i.e. fast oscillation => small period => large oscillation in derivative => large oscillation in airspeed
 * Idea: treat gpsV and xB as noisy time series with trend and
 * apply Holt-Winters double exponential smoothing to avoid smoothing out of trend (=derivative)
 * s1 = x1
 * b1 = x1 - x0
 * s_{k+1} = alpha*x_{k+1} + (1-alpha)*(s_k + b_k)
 * b_{k+1} = beta*(s_{k+1} - s_k) + (1-beta)b_k
 */
void imu_airspeedGet(AirspeedSensorData *airspeedData, AirspeedSettingsData *airspeedSettings)
{
    const float alpha = airspeedSettings->GroundSpeedBasedEstimationLowPassAlpha;
    const float beta  = airspeedSettings->GroundSpeedBasedEstimationLowPassAlpha;

    // get values and conduct smoothing of ground speed and orientation independently of the calculation of airspeed
    { // Scoping to save memory
        float xB[3];
        AttitudeStateData attData;
        AttitudeStateGet(&attData);
        VelocityStateData velData;
        VelocityStateGet(&velData);


        // Calculate rotation matrix
        QuaternionC2xB(attData.q1, attData.q2, attData.q3, attData.q4, xB);

        // Holt-Winters double exponential smoothing
        // Orientation xB
        float sk = imu->xB[0];
        imu->xB[0]  = alpha * xB[0] + (1.f - alpha) * (sk + imu->dxB[0]);
        imu->dxB[0] = beta * (imu->xB[0] - sk) + (1.f - beta) * imu->dxB[0];

        sk = imu->xB[1];
        imu->xB[1]  = alpha * xB[1] + (1.f - alpha) * (sk + imu->dxB[1]);
        imu->dxB[1] = beta * (imu->xB[1] - sk) + (1.f - beta) * imu->dxB[1];

        sk = imu->xB[2];
        imu->xB[2]  = alpha * xB[2] + (1.f - alpha) * (sk + imu->dxB[2]);
        imu->dxB[2] = beta * (imu->xB[2] - sk) + (1.f - beta) * imu->dxB[2];

        // Ground speed Vel
        sk = imu->Vel[0];
        imu->Vel[0]  = alpha * velData.North + (1.f - alpha) * (sk + imu->dVel[0]);
        imu->dVel[0] = beta * (imu->Vel[0] - sk) + (1.f - beta) * imu->dVel[0];

        sk = imu->Vel[1];
        imu->Vel[1]  = alpha * velData.East + (1.f - alpha) * (sk + imu->dVel[1]);
        imu->dVel[1] = beta * (imu->Vel[1] - sk) + (1.f - beta) * imu->dVel[1];

        sk = imu->Vel[2];
        imu->Vel[2]  = alpha * velData.Down + (1.f - alpha) * (sk + imu->dVel[2]);
        imu->dVel[2] = beta * (imu->Vel[2] - sk) + (1.f - beta) * imu->dVel[2];

        /////// for debugging purposes only! ////////////
        airspeedData->f[0]    = imu->xB[0];
        airspeedData->f[1]    = imu->xB[1];
        airspeedData->f[2]    = imu->xB[2];

        airspeedData->v[0]    = imu->Vel[0];
        airspeedData->v[1]    = imu->Vel[1];
        airspeedData->v[2]    = imu->Vel[2];

        airspeedData->df[0]   = imu->dxB[0];
        airspeedData->df[1]   = imu->dxB[1];
        airspeedData->df[2]   = imu->dxB[2];

        airspeedData->dv[0]   = imu->dVel[0];
        airspeedData->dv[1]   = imu->dVel[1];
        airspeedData->dv[2]   = imu->dVel[2];
        airspeedData->absdf   = Sq(imu->dxB[0]) + Sq(imu->dxB[1]) + Sq(imu->dxB[2]);
        airspeedData->dvdotdf = imu->dVel[0] * imu->dxB[0] + imu->dVel[1] * imu->dxB[1] + imu->dVel[2] * imu->dxB[2];
        //////////////////////////////////////////////////
    }

    // Calculate the norm^2 of the difference between the two fuselage vectors
    const float normDiffAttitude2 = Sq(imu->dxB[0]) + Sq(imu->dxB[1]) + Sq(imu->dxB[2]);
    const float normVel2 = Sq(imu->Vel[0]) + Sq(imu->Vel[1]) + Sq(imu->Vel[2]);

    // Some reorientation needed to be able to calculate airspeed and calculate only for sufficient velocity
    if (normDiffAttitude2 > EPS_REORIENTATION && normVel2 > EPS_VELOCITY) {
        // Calculate scalar product of difference vectors
        const float dvdtDotdfdt = imu->dVel[0] * imu->dxB[0] + imu->dVel[1] * imu->dxB[1] + imu->dVel[2] * imu->dxB[2];

        // Airspeed modulus: |v| = dv/dt * dxB/dt / |dxB/dt|^2
        // airspeed is always REAL because  normDiffAttitude2 > EPS_REORIENTATION > 0 and REAL dvdtDotdfdt
        const float airspeed = dvdtDotdfdt / normDiffAttitude2;

        if (!IS_REAL(airspeedData->CalibratedAirspeed)) {
            airspeedData->CalibratedAirspeed = 0;
            airspeedData->SensorConnected    = AIRSPEEDSENSOR_SENSORCONNECTED_FALSE;
            AlarmsSet(SYSTEMALARMS_ALARM_AIRSPEED, SYSTEMALARMS_ALARM_ERROR);
        } else {
            airspeedData->CalibratedAirspeed = airspeed;
            airspeedData->SensorConnected    = AIRSPEEDSENSOR_SENSORCONNECTED_TRUE;
            AlarmsSet(SYSTEMALARMS_ALARM_AIRSPEED, SYSTEMALARMS_ALARM_OK);
        }
    } else {
        airspeedData->CalibratedAirspeed = 0;
        airspeedData->SensorConnected    = AIRSPEEDSENSOR_SENSORCONNECTED_FALSE;
        AlarmsSet(SYSTEMALARMS_ALARM_AIRSPEED, SYSTEMALARMS_ALARM_WARNING);
    }
}


/**
 * @}
 * @}
 */

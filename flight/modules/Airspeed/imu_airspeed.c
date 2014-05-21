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
    // storage variables for Butterworth filter
    float pn1, pn2;
    float yn1, yn2;
    float v1n1, v1n2;
    float v2n1, v2n2;
    float v3n1, v3n2;
    float Vn1,Vn2;
    
    // storage variables for derivative calculation 
    float pOld, yOld;
    float v1Old, v2Old, v3Old;
};

// Private variables
static struct IMUGlobals *imu;

// Private functions
// a simple square inline function based on multiplication faster than powf(x,2.0f)
static inline float Sq(float x)
{
    return x * x;
}

// ****** find pitch, yaw from quaternion ********
static void Quaternion2PY(const float q0, const float q1, const float q2, const float q3, float *pPtr, float *yPtr, bool principalArg)
{
    float R13, R11, R12;
    const float q0s = q0 * q0;
    const float q1s = q1 * q1;
    const float q2s = q2 * q2;
    const float q3s = q3 * q3;

    R13    = 2.0f * (q1 * q3 - q0 * q2);
    R11    = q0s + q1s - q2s - q3s;
    R12    = 2.0f * (q1 * q2 + q0 * q3);

    *pPtr = asinf(-R13); // pitch always between -pi/2 to pi/2

    const float y_=atan2f(R12, R11);
    // use old yaw contained in y to add multiples of 2pi to have a continuous yaw if user does not want the principal argument
    // else simply copy atan2 result into result
    if(principalArg){
        *yPtr = y_;
    }else{
        const int mod=(int)((y_-*yPtr)/(2.0f*M_PI_F*0.9f));
        *yPtr = y_- 2.0f*M_PI_F*mod;
    }
}

static void PY2xB(float p, float y, float x[3])
{
  const float cosp=cosf(p);
  x[0]=cosp*cosf(y);
  x[1]=cosp*sinf(y);
  x[2]=-sinf(p);
}


//second order Butterworth filter with cut-off frequency ratio ff
// filter is writen in direct from 2, such that only two values wn1=w[n-1] and wn2=w[n-2] need to be stored
// function takes care of updating the values wn1 and wn2
float FilterButterWorthDF2(const float ff, float xn, float *wn1Ptr, float *wn2Ptr)
{
    // TODO: we need to think about storing the filter instead of calculating it again and again
    const float ita =1.0f/ tanf(M_PI_F*ff);
    const float q=sqrtf(2.0f);
    const float b0 = 1.0f / (1.0f + q*ita + Sq(ita));
    const float b1= 2.0f*b0;
    const float b2= b0;
    const float a1 = 2.0f * (Sq(ita) - 1.0f) * b0;
    const float a2 = -(1.0f - q*ita + Sq(ita)) * b0;
    
    const float wn=xn + a1*(*wn1Ptr) + a2*(*wn2Ptr);
    const float val=b0*wn + b1*(*wn1Ptr) + b2*(*wn2Ptr);
    *wn2Ptr=*wn1Ptr;
    *wn1Ptr=wn;
    return val;
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

    // get pitch and yaw from quarternion; principal argument for yaw
    Quaternion2PY(attData.q1, attData.q2, attData.q3, attData.q4, &(imu->pOld),&(imu->yOld),true);
        
    imu->pn1 = imu->pn2 = imu->pOld;
    imu->yn1 = imu->yn2 = imu->yOld;
    
    imu->v1n1 = imu->v1n2 = imu->v1Old = velData.North;
    imu->v2n1 = imu->v2n2 = imu->v2Old = velData.East;
    imu->v3n1 = imu->v3n2 = imu->v3Old = velData.Down;

    // initial guess for airspeed is modulus of groundspeed
    imu->Vn1 = imu->Vn2 = sqrt(Sq(velData.North) + Sq(velData.East) + Sq(velData.Down));
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
 */
void imu_airspeedGet(AirspeedSensorData *airspeedData, AirspeedSettingsData *airspeedSettings)
{
    //pre-filter frequency rate
    //corresponds to a cut-off frequency of 0.04 Hz or a period of 25 sec
    const float ff=0.04f * 1000.0f/airspeedSettings->SamplePeriod;
    // good values for turbulent situation: cut-off 0.01 Hz or a period of 100 sec
    const float ffV=0.01f * 1000.0f/airspeedSettings->SamplePeriod;
    // good values for steady situation: cut-off 0.05 Hz or a period of 20 sec
//     const float ffV=0.05 * 1000.0f/airspeedSettings->SamplePeriod;
        
    float dxB[3], dVel[3];
    float normVel2;
    
    // get values and conduct smoothing of ground speed and orientation independently of the calculation of airspeed
    { // Scoping to save memory
        AttitudeStateData attData;
        AttitudeStateGet(&attData);
        VelocityStateData velData;
        VelocityStateGet(&velData);
        float p=imu->pOld, y=imu->yOld;
        float xB[3], xBOld[3];

        // get pitch and roll Euler angles from quaternion
        // do not calculate the principlal argument of yaw, i.e. use old yaw to add multiples of 2pi to have a continuous yaw
        Quaternion2PY(attData.q1, attData.q2, attData.q3, attData.q4,&p,&y,false);

        // filter pitch and roll Euler angles instead of fuselage vector to guarantee a unit length at all times
        p=FilterButterWorthDF2(ff, p, &(imu->pn1), &(imu->pn2));
        y=FilterButterWorthDF2(ff, y, &(imu->yn1), &(imu->yn2));
        // transform pitch and yaw into fuselage vector xB
        PY2xB(imu->pOld,imu->yOld,xBOld);
        PY2xB(p,y,xB);
        // calculate change in fuselage vector
        dxB[0]=xB[0]-xBOld[0];
        dxB[1]=xB[1]-xBOld[1];
        dxB[2]=xB[2]-xBOld[2];
        
        // filter ground speed from VelocityState
        const float fv1n = FilterButterWorthDF2(ff, velData.North, &(imu->v1n1), &(imu->v1n2));
        const float fv2n = FilterButterWorthDF2(ff, velData.East,  &(imu->v2n1), &(imu->v2n2));
        const float fv3n = FilterButterWorthDF2(ff, velData.Down,  &(imu->v3n1), &(imu->v3n2));
            // calculate change in ground velocity
        dVel[0] = fv1n - imu->v1Old;
        dVel[1] = fv2n - imu->v2Old;
        dVel[2] = fv3n - imu->v3Old;

        // calculate norm of ground speed
        normVel2 = Sq(fv1n) + Sq(fv2n) + Sq(fv3n);
        
        // actualise old values
        imu->pOld = p;  imu->yOld = y;
        imu->v1Old = fv1n;  imu->v2Old = fv2n;  imu->v3Old = fv3n;
    }

    // Calculate the norm^2 of the difference between the two fuselage vectors
    const float normDiffAttitude2 = Sq(dxB[0]) + Sq(dxB[1]) + Sq(dxB[2]);
    // Calculate scalar product of difference vectors
    const float dvdtDotdfdt = dVel[0] * dxB[0] + dVel[1] * dxB[1] + dVel[2] * dxB[2];

    // Some reorientation needed to be able to calculate airspeed, calculate only for sufficient velocity
    // a negative scalar product is a clear sign that we are not really able to calculate the airspeed
    if (normDiffAttitude2 > EPS_REORIENTATION && normVel2 > EPS_VELOCITY && dvdtDotdfdt > 0.f) {
        // Airspeed modulus: |v| = dv/dt * dxB/dt / |dxB/dt|^2
        // airspeed is always REAL because  normDiffAttitude2 > EPS_REORIENTATION > 0 and REAL dvdtDotdfdt
        const float airspeed = dvdtDotdfdt / normDiffAttitude2;
        // filter raw airspeed
        const float fVn=FilterButterWorthDF2(ffV,airspeed,&(imu->Vn1),&(imu->Vn2));
        
        airspeedData->CalibratedAirspeed = fVn;
        airspeedData->SensorConnected    = AIRSPEEDSENSOR_SENSORCONNECTED_TRUE;
        AlarmsSet(SYSTEMALARMS_ALARM_AIRSPEED, SYSTEMALARMS_ALARM_OK);
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

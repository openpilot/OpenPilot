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
#include "butterworth.h"
#include <pios_math.h>


// Private constants
#define TWO_PI            6.283185308f
#define EPS               1e-6f
#define EPS_REORIENTATION 1e-10f
#define EPS_VELOCITY      1.f

// Private types
// structure with smoothed fuselage orientation, ground speed, wind vector and their changes in time
struct IMUGlobals {
    // Butterworth filters
    struct ButterWorthDF2Filter filter;
    struct ButterWorthDF2Filter prefilter;
    float ff, ffV;


    // storage variables for Butterworth filter
    float pn1, pn2;
    float yn1, yn2;
    float v1n1, v1n2;
    float v2n1, v2n2;
    float v3n1, v3n2;
    float Vw1n1, Vw1n2;
    float Vw2n1, Vw2n2;
    float Vw3n1, Vw3n2;
    float Vw1, Vw2, Vw3;

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

    R13   = 2.0f * (q1 * q3 - q0 * q2);
    R11   = q0s + q1s - q2s - q3s;
    R12   = 2.0f * (q1 * q2 + q0 * q3);

    *pPtr = asinf(-R13); // pitch always between -pi/2 to pi/2

    const float y_ = atan2f(R12, R11);
    // use old yaw contained in y to add multiples of 2pi to have a continuous yaw if user does not want the principal argument
    // else simply copy atan2 result into result
    if (principalArg) {
        *yPtr = y_;
    } else {
        // calculate needed mutliples of 2pi to avoid jumps
        // number of cycles accumulated in old yaw
        const int32_t cycles = (int32_t)(*yPtr / TWO_PI);
        // look for a jump by substracting the modulus, i.e. there is maximally one jump.
        // take slightly less than 2pi, because the jump will always be lower than 2pi
        const int32_t mod    = (int32_t)((y_ - (*yPtr - cycles * TWO_PI)) / (TWO_PI * 0.8f));
        *yPtr = y_ + TWO_PI * (cycles - mod);
    }
}

static void PY2xB(const float p, const float y, float x[3])
{
    const float cosp = cosf(p);

    x[0] = cosp * cosf(y);
    x[1] = cosp * sinf(y);
    x[2] = -sinf(p);
}


static void PY2DeltaxB(const float p, const float y, const float xB[3], float x[3])
{
    const float cosp = cosf(p);

    x[0] = xB[0] - cosp * cosf(y);
    x[1] = xB[1] - cosp * sinf(y);
    x[2] = xB[2] - -sinf(p);
}


// second order Butterworth filter with cut-off frequency ratio ff
// Biquadratic filter in direct from 2, such that only two values wn1=w[n-1] and wn2=w[n-2] need to be stored
// function takes care of updating the values wn1 and wn2
/*float FilterButterWorthDF2(const float ff, const float xn, float *wn1Ptr, float *wn2Ptr)
   {
    const float ita = 1.0f / tanf(M_PI_F * ff);
    const float b0  = 1.0f / (1.0f + SQRT2 * ita + Sq(ita));
    const float a1  = 2.0f * b0 * (Sq(ita) - 1.0f);
    const float a2  = -b0 * (1.0f - SQRT2 * ita + Sq(ita));

    const float wn  = xn + a1 * (*wn1Ptr) + a2 * (*wn2Ptr);
    const float val = b0 * (wn + 2.0f * (*wn1Ptr) + (*wn2Ptr));

 * wn2Ptr = *wn1Ptr;
 * wn1Ptr = wn;
    return val;
   }*/


// second order Butterworth filter with cut-off frequency ratio ff
// Biquadratic filter in direct from 2, such that only two values wn1=w[n-1] and wn2=w[n-2] need to be stored
// function takes care of updating the values wn1 and wn2
// float FilterButterWorthDF2(const float xn, const ButterWorthDF2Filter *filter, float *wn1Ptr, float *wn2Ptr)
// {
//
// const float wn  = xn + filter->a1 * (*wn1Ptr) + filter->a2 * (*wn2Ptr);
// const float val = filter->b0 * (wn + 2.0f * (*wn1Ptr) + (*wn2Ptr));
//
// *wn2Ptr = *wn1Ptr;
// *wn1Ptr = wn;
// return val;
// }


// Initialization function for direct form 2 Butterworth filter
/*void InitButterWorthDF2(const float ff, const float x0, float *wn1Ptr, float *wn2Ptr)
   {
    const float ita  = 1.0f / tanf(M_PI_F * ff);
    const float b0   = 1.0f / (1.0f + SQRT2 * ita + Sq(ita));
    const float b1   = 2.0f * b0;
    const float b2   = b0;
    const float a1   = 2.0f * b0 * (Sq(ita) - 1.0f);
    const float a2   = -b0 * (1.0f - SQRT2 * ita + Sq(ita));

    const float a11  = b1 + b0 * a1;
    const float a12  = b2 + b0 * a2;
    const float a21  = b1 + b0 * (Sq(a1) + a2);
    const float a22  = b2 + b0 * a1 * a2;
    const float det  = a11 * a22 - a12 * a21;

    const float rhs1 = x0 - b0 * x0;
    const float rhs2 = x0 - b0 * (x0 + a1 * x0);

 * wn1Ptr = (a22 * rhs1 - a12 * rhs2) / det;
 * wn2Ptr = (-a21 * rhs1 + a11 * rhs2) / det;
   }*/


// Initialization function for direct form 2 Butterworth filter
// initialize filter constants. Note that the following is used:
// b1  = 2 * b0
// b2  = b0
// void InitButterWorthDF2Filter(const float ff, ButterWorthDF2Filter *filter)
// {
// const float ita = 1.0f / tanf(M_PI_F * ff);
// const float b0  = 1.0f / (1.0f + SQRT2 * ita + Sq(ita));
// const float a1  = 2.0f * b0 * (Sq(ita) - 1.0f);
// const float a2  = -b0 * (1.0f - SQRT2 * ita + Sq(ita));
//
// filter->ff = ff;
// filter->b0 = b0;
// filter->a1 = a1;
// filter->a2 = a2;
// }


//// Initialization function for direct form 2 Butterworth filter
//// initialize intermediate values for starting value x0
// void InitButterWorthDF2Values(const float x0, const ButterWorthDF2Filter *filter, float *wn1Ptr, float *wn2Ptr)
// {
// const float b0  = filter->b0;
// const float a1  = filter->a1;
// const float a2  = filter->a2;
//
// const float a11  = 2.0f + a1;
// const float a12  = 1.0f + a2;
// const float a21  = 2.0f + Sq(a1) + a2;
// const float a22  = 1.0f + a1 * a2;
// const float det  = a11 * a22 - a12 * a21;
// const float rhs1 = x0 / b0 - x0;
// const float rhs2 = x0 / b0 - x0 + a1 * x0;
//
// *wn1Ptr = ( a22 * rhs1 - a12 * rhs2) / det;
// *wn2Ptr = (-a21 * rhs1 + a11 * rhs2) / det;
// }


/*
 * Initialize function loads first data sets, and allocates memory for structure.
 */
void imu_airspeedInitialize(const AirspeedSettingsData *airspeedSettings)
{
    // pre-filter frequency rate
    const float ff  = (float)(airspeedSettings->SamplePeriod) / 1000.0f / airspeedSettings->IMUBasedEstimationLowPassPeriod1;
    // filter frequency rate
    const float ffV = (float)(airspeedSettings->SamplePeriod) / 1000.0f / airspeedSettings->IMUBasedEstimationLowPassPeriod2;

    // This method saves memory in case we don't use the module.
    imu = (struct IMUGlobals *)pvPortMalloc(sizeof(struct IMUGlobals));

    // airspeed calculation variables
    VelocityStateInitialize();
    VelocityStateData velData;
    VelocityStateGet(&velData);

    AttitudeStateData attData;
    AttitudeStateGet(&attData);

    // initialize filters for given ff and ffV
    InitButterWorthDF2Filter(ffV, &(imu->filter));
    InitButterWorthDF2Filter(ff, &(imu->prefilter));
    imu->ffV = ffV;
    imu->ff  = ff;

    // get pitch and yaw from quarternion; principal argument for yaw
    Quaternion2PY(attData.q1, attData.q2, attData.q3, attData.q4, &(imu->pOld), &(imu->yOld), true);
    InitButterWorthDF2Values(imu->pOld, &(imu->prefilter), &(imu->pn1), &(imu->pn2));
    InitButterWorthDF2Values(imu->yOld, &(imu->prefilter), &(imu->yn1), &(imu->yn2));

    // use current NED speed as vOld vector and as initial value for filter
    imu->v1Old = velData.North;
    imu->v2Old = velData.East;
    imu->v3Old = velData.Down;
    InitButterWorthDF2Values(imu->v1Old, &(imu->prefilter), &(imu->v1n1), &(imu->v1n2));
    InitButterWorthDF2Values(imu->v2Old, &(imu->prefilter), &(imu->v2n1), &(imu->v2n2));
    InitButterWorthDF2Values(imu->v3Old, &(imu->prefilter), &(imu->v3n1), &(imu->v3n2));

    // initial guess for windspeed is zero
    imu->Vw3   = imu->Vw2 = imu->Vw1 = 0.0f;
    InitButterWorthDF2Values(0.0f, &(imu->filter), &(imu->Vw1n1), &(imu->Vw1n2));
    imu->Vw3n1 = imu->Vw2n1 = imu->Vw1n1;
    imu->Vw3n2 = imu->Vw2n2 = imu->Vw1n2;
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
 * A two step Butterworth second order filter is used. In the first step fuselage vector xB
 * and ground speed vector Vel are filtered. The fuselage vector is filtered through its pitch
 * and yaw to keep a unit length. After building the differenced dxB and dVel are produced and
 * the airspeed calculated. The calculated airspeed is filtered again with a Butterworth filter
 */
void imu_airspeedGet(AirspeedSensorData *airspeedData, const AirspeedSettingsData *airspeedSettings)
{
    // pre-filter frequency rate
    const float ff  = (float)(airspeedSettings->SamplePeriod) / 1000.0f / airspeedSettings->IMUBasedEstimationLowPassPeriod1;
    // filter frequency rate
    const float ffV = (float)(airspeedSettings->SamplePeriod) / 1000.0f / airspeedSettings->IMUBasedEstimationLowPassPeriod2;

    // check for a change in filter frequency rate. if yes, then actualize filter constants and intermediate values
    if (fabsf(ffV - imu->ffV) > EPS) {
        InitButterWorthDF2Filter(ffV, &(imu->filter));
        InitButterWorthDF2Values(imu->Vw1, &(imu->filter), &(imu->Vw1n1), &(imu->Vw1n2));
        InitButterWorthDF2Values(imu->Vw2, &(imu->filter), &(imu->Vw2n1), &(imu->Vw2n2));
        InitButterWorthDF2Values(imu->Vw3, &(imu->filter), &(imu->Vw3n1), &(imu->Vw3n2));
    }
    if (fabsf(ff - imu->ff) > EPS) {
        InitButterWorthDF2Filter(ff, &(imu->prefilter));
        InitButterWorthDF2Values(imu->pOld, &(imu->prefilter), &(imu->pn1), &(imu->pn2));
        InitButterWorthDF2Values(imu->yOld, &(imu->prefilter), &(imu->yn1), &(imu->yn2));
        InitButterWorthDF2Values(imu->v1Old, &(imu->prefilter), &(imu->v1n1), &(imu->v1n2));
        InitButterWorthDF2Values(imu->v2Old, &(imu->prefilter), &(imu->v2n1), &(imu->v2n2));
        InitButterWorthDF2Values(imu->v3Old, &(imu->prefilter), &(imu->v3n1), &(imu->v3n2));
    }

    float normVel2;
    float normDiffAttitude2;
    float dvdtDotdfdt;

    float xB[3];
    // get values and conduct smoothing of ground speed and orientation independently of the calculation of airspeed
    { // Scoping to save memory
        AttitudeStateData attData;
        AttitudeStateGet(&attData);
        VelocityStateData velData;
        VelocityStateGet(&velData);
        float p = imu->pOld, y = imu->yOld;
        float dxB[3];

        // get pitch and roll Euler angles from quaternion
        // do not calculate the principlal argument of yaw, i.e. use old yaw to add multiples of 2pi to have a continuous yaw
        Quaternion2PY(attData.q1, attData.q2, attData.q3, attData.q4, &p, &y, false);

        // filter pitch and roll Euler angles instead of fuselage vector to guarantee a unit length at all times
        p = FilterButterWorthDF2(p, &(imu->prefilter), &(imu->pn1), &(imu->pn2));
        y = FilterButterWorthDF2(y, &(imu->prefilter), &(imu->yn1), &(imu->yn2));
        // transform pitch and yaw into fuselage vector xB and xBold
        PY2xB(p, y, xB);
        // calculate change in fuselage vector by substraction of old value
        PY2DeltaxB(imu->pOld, imu->yOld, xB, dxB);

        // filter ground speed from VelocityState
        const float fv1n = FilterButterWorthDF2(velData.North, &(imu->prefilter), &(imu->v1n1), &(imu->v1n2));
        const float fv2n = FilterButterWorthDF2(velData.East, &(imu->prefilter), &(imu->v2n1), &(imu->v2n2));
        const float fv3n = FilterButterWorthDF2(velData.Down, &(imu->prefilter), &(imu->v3n1), &(imu->v3n2));

        // calculate norm of ground speed
        normVel2 = Sq(fv1n) + Sq(fv2n) + Sq(fv3n);
        // calculate norm of orientation change
        normDiffAttitude2 = Sq(dxB[0]) + Sq(dxB[1]) + Sq(dxB[2]);
        // cauclate scalar product between groundspeed change and orientation change
        dvdtDotdfdt = (fv1n - imu->v1Old) * dxB[0] + (fv2n - imu->v2Old) * dxB[1] + (fv3n - imu->v3Old) * dxB[2];

        // actualise old values
        imu->pOld   = p;
        imu->yOld   = y;
        imu->v1Old  = fv1n;
        imu->v2Old  = fv2n;
        imu->v3Old  = fv3n;
    }

    // Some reorientation needed to be able to calculate airspeed, calculate only for sufficient velocity
    // a negative scalar product is a clear sign that we are not really able to calculate the airspeed
    // NOTE: normVel2 check against EPS_VELOCITY might make problems during hovering maneuvers in fixed wings
    if (normDiffAttitude2 > EPS_REORIENTATION && normVel2 > EPS_VELOCITY && dvdtDotdfdt > 0.f) {
        // Airspeed modulus: |v| = dv/dt * dxB/dt / |dxB/dt|^2
        // airspeed is always REAL because  normDiffAttitude2 > EPS_REORIENTATION > 0 and REAL dvdtDotdfdt
        const float airspeed = dvdtDotdfdt / normDiffAttitude2;

        // groundspeed = airspeed + wind ---> wind = groundspeed - airspeed
        const float wind[3]  = { imu->v1Old - xB[0] * airspeed,
                                 imu->v2Old - xB[1] * airspeed,
                                 imu->v3Old - xB[2] * airspeed };
        // filter raw wind
        imu->Vw1 = FilterButterWorthDF2(wind[0], &(imu->filter), &(imu->Vw1n1), &(imu->Vw1n2));
        imu->Vw2 = FilterButterWorthDF2(wind[1], &(imu->filter), &(imu->Vw2n1), &(imu->Vw2n2));
        imu->Vw3 = FilterButterWorthDF2(wind[2], &(imu->filter), &(imu->Vw3n1), &(imu->Vw3n2));
    } // else leave wind estimation unchanged

    { // Scoping to save memory
      // airspeed = groundspeed - wind
        const float Vair[3] = {
            imu->v1Old - imu->Vw1,
            imu->v2Old - imu->Vw2,
            imu->v3Old - imu->Vw3
        };

        // project airspeed into fuselage vector
        airspeedData->CalibratedAirspeed = Vair[0] * xB[0] + Vair[1] * xB[1] + Vair[2] * xB[2];
    }

    airspeedData->SensorConnected = AIRSPEEDSENSOR_SENSORCONNECTED_TRUE;
    AlarmsClear(SYSTEMALARMS_ALARM_AIRSPEED);
}


/**
 * @}
 * @}
 */

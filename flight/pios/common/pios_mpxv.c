/**
 ******************************************************************************
 * @addtogroup PIOS PIOS Core hardware abstraction layer
 * @{
 * @addtogroup PIOS_MPXV Functions
 * @brief Hardware functions to deal with the DIYDrones airspeed kit, using MPXV*. 
 *    This is a differential sensor, so the value returned is first converted into 
 *    calibrated airspeed, using http://en.wikipedia.org/wiki/Calibrated_airspeed
 * @{
 *
 * @file       pios_mpxv.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2012.
 * @brief      ETASV3 Airspeed Sensor Driver
 * @see        The GNU Public License (GPL) Version 3
 *
 ******************************************************************************/
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

#include "pios.h"

#ifdef PIOS_INCLUDE_MPXV

#define A0 340.27f        //speed of sound at standard sea level in [m/s]
#define P0 101.325f       //static air pressure at standard sea level in kPa
#define POWER (2.0f/7.0f)

#include "pios_mpxv.h"

/*
 * Reads ADC.
 */
uint16_t PIOS_MPXV_Measure(PIOS_MPXV_descriptor *desc)
{
    if (desc)
    return PIOS_ADC_PinGet(desc->airspeedADCPin);
    return 0;
}

/*
 *Returns zeroPoint so that the user can inspect the calibration vs. the sensor value
 */
uint16_t PIOS_MPXV_Calibrate(PIOS_MPXV_descriptor *desc,uint16_t measurement) {
    desc->calibrationSum += measurement;
    desc->calibrationCount++;
    desc->zeroPoint = (uint16_t)(((float)desc->calibrationSum) / desc->calibrationCount);
    return desc->zeroPoint;
}

/*
 * Reads the airspeed and returns CAS (calibrated airspeed) in the case of success. 
 * In the case of a failed read, returns -1.
 */
float PIOS_MPXV_CalcAirspeed(PIOS_MPXV_descriptor *desc,uint16_t measurement)
{
    //Calculate dynamic pressure, as per docs
    float Qc = 3.3f/4096.0f * (float)(measurement - desc->zeroPoint);

    //Saturate Qc on the lower bound, in order to make sure we don't have negative airspeeds. No need
    // to saturate on the upper bound, we'll handle that later with calibratedAirspeed.
    if (Qc < 0) {
        Qc=0;
    }

    //Compute calibrated airspeed, as per http://en.wikipedia.org/wiki/Calibrated_airspeed
    float calibratedAirspeed = A0 * sqrtf( 5.0f * (powf(Qc / P0 + 1.0f, POWER) - 1.0f));

    //Upper bound airspeed. No need to lower bound it, that comes from Qc
    //in [m/s]
    if (calibratedAirspeed > desc->maxSpeed) {
        calibratedAirspeed=desc->maxSpeed;
    }

    return calibratedAirspeed;
}

#endif /* PIOS_INCLUDE_MPXV */

/**
 ******************************************************************************
 * @addtogroup PIOS PIOS Core hardware abstraction layer
 * @{
 * @addtogroup PIOS_MPXV5004 MPXV5004 Functions
 * @brief Hardware functions to deal with the DIYDrones airspeed kit, using MPXV5004. 
 *    This is a differential sensor, so the value returned is first converted into 
 *    calibrated airspeed, using http://en.wikipedia.org/wiki/Calibrated_airspeed
 * @{
 *
 * @file       pios_mpxv5004.h
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

/* Project Includes */
#include "pios.h"

#if defined(PIOS_INCLUDE_MPXV5004)

#include "pios_mpxv5004.h"

static uint32_t calibrationSum = 0; //static?
static int16_t calibrationOffset; //static?


/*
 * Reads ADC.
 */
uint16_t PIOS_MPXV5004_Measure(uint8_t airspeedADCPin)
{
	return PIOS_ADC_PinGet(airspeedADCPin);
}

/*
 *Returns zeroPoint so that the user can inspect the calibration vs. the sensor value
 */
uint16_t PIOS_MPXV5004_Calibrate(uint8_t airspeedADCPin, uint16_t calibrationCount){
	calibrationSum +=  PIOS_MPXV5004_Measure(airspeedADCPin);
	uint16_t zeroPoint = (uint16_t)(((float)calibrationSum) / calibrationCount + 0.5f);	

	calibrationOffset = zeroPoint - (int16_t)(1.0f/3.3f*4096.0f+0.5f); //The offset should set the zero point to 1.0V
	
	return zeroPoint;
}


/*
 * Updates the calibration when zero point is manually set by user.
 */
void PIOS_MPXV5004_UpdateCalibration(uint16_t zeroPoint){
	calibrationOffset = zeroPoint - (int16_t)(1.0f/3.3f*4096.0f+0.5f); //The offset should set the zero point to 1.0V
}


/*
 * Reads the airspeed and returns CAS (calibrated airspeed) in the case of success. 
 * In the case of a failed read, returns -1.
 */
float PIOS_MPXV5004_ReadAirspeed(uint8_t airspeedADCPin)
{
	float sensorVal = PIOS_MPXV5004_Measure(airspeedADCPin);
	
	//Calculate dynamic pressure, as per docs
	float Qc = 5.0f*(((sensorVal - calibrationOffset)/4096.0f*3.3f)/VCC - 0.2f);

	//Saturate Qc on the lower bound, in order to make sure we don't have negative airspeeds. No need
	// to saturate on the upper bound, we'll handle that later with calibratedAirspeed.
	if (Qc < 0) {
		Qc=0;
	}
	
	//Compute calibraterd airspeed, as per http://en.wikipedia.org/wiki/Calibrated_airspeed
	float calibratedAirspeed = A0*sqrt(5.0f*(pow(Qc/P0+1.0f,POWER)-1.0f));
	
	//Upper bound airspeed. No need to lower bound it, that comes from Qc
	if (calibratedAirspeed > 80) { //in [m/s]
		calibratedAirspeed=80;
	}
	
	
	return calibratedAirspeed;
}

#endif /* PIOS_INCLUDE_MPXV5004 */

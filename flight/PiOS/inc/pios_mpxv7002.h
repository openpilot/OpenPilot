/**
  ******************************************************************************
  * @addtogroup PIOS PIOS Core hardware abstraction layer
  * @{
  * @addtogroup PIOS_MPXV7002 MPXV7002 Functions
  * @brief Hardware functions to deal with the DIYDrones airspeed kit, using MPXV7002
  * @{
  *
  * @file       pios_mpxv7002.h
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

#ifndef PIOS_MPXV7002_H__

#define A0 340.27f        //speed of sound at standard sea level in [m/s]
#define P0 101.325f       //static air pressure at standard sea level in kPa
#define VCC 5.0f           //Supply voltage in V
#define POWER (2.0f/7.0f)

uint16_t PIOS_MPXV7002_Measure(uint8_t airspeedADCPin);
uint16_t PIOS_MPXV7002_Calibrate(uint8_t airspeedADCPin, uint16_t calibrationCount);
void PIOS_MPXV7002_UpdateCalibration(uint16_t zeroPoint);
float PIOS_MPXV7002_ReadAirspeed (uint8_t airspeedADCPin);

#endif // PIOS_MPXV7002_H__
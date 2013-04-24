/**
  ******************************************************************************
  * @addtogroup PIOS PIOS Core hardware abstraction layer
  * @{
  * @addtogroup PIOS_MPXV MPXV* Functions
  * @brief Hardware functions to deal with the DIYDrones airspeed kit, using MPXV5004, 7002 or similar
  * @{
  *
  * @file       pios_mpxv.h
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

#ifndef PIOS_MPXV_H
#define PIOS_MPXV_H

typedef enum{ PIOS_MPXV_UNKNOWN,PIOS_MPXV_5004,PIOS_MPXV_7002 } PIOS_MPXV_sensortype;
typedef struct{
	PIOS_MPXV_sensortype type;
	uint8_t airspeedADCPin;
	uint16_t calibrationCount;
	uint32_t calibrationSum;
	uint16_t zeroPoint;
	float maxSpeed;
} PIOS_MPXV_descriptor;

#define PIOS_MPXV_5004_DESC(pin) \
	(PIOS_MPXV_descriptor){ \
		.type           = PIOS_MPXV_5004, \
		.airspeedADCPin = pin, \
		.maxSpeed       = 80.0f, \
		.calibrationCount = 0, \
		.calibrationSum = 0, \
	}
#define PIOS_MPXV_7002_DESC(pin) \
	(PIOS_MPXV_descriptor){ \
		.type           = PIOS_MPXV_7002, \
		.airspeedADCPin = pin, \
		.maxSpeed       = 56.0f, \
		.calibrationCount = 0, \
		.calibrationSum = 0, \
	}


uint16_t PIOS_MPXV_Measure(PIOS_MPXV_descriptor *desc);
uint16_t PIOS_MPXV_Calibrate(PIOS_MPXV_descriptor *desc, uint16_t measurement);
float PIOS_MPXV_CalcAirspeed (PIOS_MPXV_descriptor *desc,uint16_t measurement);

#endif /* PIOS_MPXV_H */

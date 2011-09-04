/**
 ******************************************************************************
 *
 * @file       ahrs_comm_objects.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      AHRS SPI comms UAVObject definitions.
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

 #ifndef AHRS_COMM_OBJECTS_H
#define AHRS_COMM_OBJECTS_H

#include "attitudeactual.h"
#include "attituderaw.h"
#include "baroaltitude.h"
#include "gpsposition.h"
#include "homelocation.h"
#include "insstatus.h"
#include "inssettings.h"
#include "positionactual.h"
#include "velocityactual.h"
#include "firmwareiapobj.h"
#include "gpsposition.h"
#include "gpssatellites.h"
#include "gpstime.h"
/** union that will fit any UAVObject.
*/

typedef union {
	AttitudeRawData AttitudeRaw;
	AttitudeActualData AttitudeActual;
	InsStatusData AhrsStatus;
	BaroAltitudeData BaroAltitude;
	GPSPositionData GPSPosition;
	PositionActualData PositionActual;
	VelocityActualData VelocityActual;
	HomeLocationData HomeLocation;
	InsSettingsData InsSettings;
	FirmwareIAPObjData FirmwareIAPObj;
	GPSSatellitesData GPSSatellites;
	GPSTimeData GPSTime;
} __attribute__ ((packed)) AhrsSharedData;

/** The number of UAVObjects we will be dealing with.
*/
#define MAX_AHRS_OBJECTS 12

/** Our own version of a UAVObject.
*/
typedef struct {
	void *data;
	int size;
	uint8_t index;
#ifndef IN_AHRS
	UAVObjHandle uavHandle;
#endif
} AhrsSharedObject;

typedef AhrsSharedObject *AhrsObjHandle;

/** Initialise the object mapping.
It is important that this is called before any of the following functions.
*/
void AhrsInitHandles(void);

/** the AHRS object related to the given index.
Returns the AHRS object or NULL if not found
*/
AhrsObjHandle AhrsFromIndex(uint8_t index);

#ifndef IN_AHRS
/** Get the AHRS object associated with the UAVObject.
Returns the AHRS object or NULL if not found
*/
AhrsObjHandle AhrsFromUAV(UAVObjHandle obj);
#endif

#endif //#ifndef AHRS_COMMS_OBJECTS_H

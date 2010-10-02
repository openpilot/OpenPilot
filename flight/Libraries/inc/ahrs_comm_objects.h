#ifndef AHRS_COMM_OBJECTS_H
#define AHRS_COMM_OBJECTS_H

#include "attitudeactual.h"
#include "attituderaw.h"
#include "attitudesettings.h"
#include "ahrsstatus.h"
#include "baroaltitude.h"
#include "gpsposition.h"
#include "positionactual.h"
#include "homelocation.h"
#include "ahrscalibration.h"
#include "ahrssettings.h"

/** union that will fit any UAVObject.
*/

typedef union {
	AttitudeRawData AttitudeRaw;
	AttitudeActualData AttitudeActual;
	AHRSCalibrationData AHRSCalibration;
	AttitudeSettingsData AttitudeSettings;
	AhrsStatusData AhrsStatus;
	BaroAltitudeData BaroAltitude;
	GPSPositionData GPSPosition;
	PositionActualData PositionActual;
	HomeLocationData HomeLocation;
	AHRSSettingsData AHRSSettings;
} __attribute__ ((packed)) AhrsSharedData;

/** The number of UAVObjects we will be dealing with.
*/
#define MAX_AHRS_OBJECTS 10

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

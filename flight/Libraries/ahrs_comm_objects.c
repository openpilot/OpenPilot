#include "ahrs_spi_comm.h"
#include "pios_debug.h"

static AttitudeRawData AttitudeRaw;
static AttitudeActualData AttitudeActual;
static AHRSCalibrationData AHRSCalibration;
static AttitudeSettingsData AttitudeSettings;
static AhrsStatusData AhrsStatus;
static BaroAltitudeData BaroAltitude;
static GPSPositionData GPSPosition;
static PositionActualData PositionActual;
static HomeLocationData HomeLocation;
static AHRSSettingsData AHRSSettings;

AhrsSharedObject objectHandles[MAX_AHRS_OBJECTS];

#ifndef NULL
#define NULL ((void *)0)
#endif

#ifdef IN_AHRS

//slightly hacky - implement our own version of the xxxHandle() functions
#define CREATEHANDLE(n,obj) \
	void * obj##Handle() {return (&objectHandles[n]);}

CREATEHANDLE( 0, AttitudeRaw );
CREATEHANDLE( 1, AttitudeActual );
CREATEHANDLE( 2, AHRSCalibration );
CREATEHANDLE( 3, AttitudeSettings );
CREATEHANDLE( 4, AhrsStatus );
CREATEHANDLE( 5, BaroAltitude );
CREATEHANDLE( 6, GPSPosition );
CREATEHANDLE( 7, PositionActual );
CREATEHANDLE( 8, HomeLocation );
CREATEHANDLE( 9, AHRSSettings );

#if 10 != MAX_AHRS_OBJECTS //sanity check
#error We did not create the correct number of xxxHandle() functions
#endif

#define ADDHANDLE(idx,obj) {\
		int n = idx;\
		objectHandles[n].data = &obj;\
		objectHandles[n].size = sizeof(obj);\
		objectHandles[n].index = n;\
		}

#else

static void ObjectUpdatedCb(UAVObjEvent * ev);

#define ADDHANDLE(idx,obj) {\
		int n = idx;\
		objectHandles[n].data = &obj;\
		objectHandles[n].uavHandle = obj##Handle();\
		objectHandles[n].size = sizeof(obj);\
		objectHandles[n].index = n;\
		}

#endif


void AhrsInitHandles( void )
{
	int idx = 0;
//Note: the first item in this list has the highest priority
//the last has the lowest priority
	ADDHANDLE( idx++, AttitudeRaw );
	ADDHANDLE( idx++, AttitudeActual );
	ADDHANDLE( idx++, AHRSCalibration );
	ADDHANDLE( idx++, AttitudeSettings );
	ADDHANDLE( idx++, AhrsStatus );
	ADDHANDLE( idx++, BaroAltitude );
	ADDHANDLE( idx++, GPSPosition );
	ADDHANDLE( idx++, PositionActual );
	ADDHANDLE( idx++, HomeLocation );
	ADDHANDLE( idx++, AHRSSettings );
	if( idx != MAX_AHRS_OBJECTS ) {
		PIOS_DEBUG_Assert( 0 );
	}

//Note: Only connect objects that the AHRS needs to read
//When the AHRS writes to these the data does a round trip
//AHRS->OP->AHRS due to these events
#ifndef IN_AHRS
    AHRSSettingsConnectCallback(ObjectUpdatedCb);
    BaroAltitudeConnectCallback(ObjectUpdatedCb);
    GPSPositionConnectCallback(ObjectUpdatedCb);
    HomeLocationConnectCallback(ObjectUpdatedCb);
    AHRSCalibrationConnectCallback(ObjectUpdatedCb);

#endif
}

AhrsObjHandle AhrsFromIndex( uint8_t index )
{
	if( index >= MAX_AHRS_OBJECTS ) {
		return( NULL );
	}
	return( &objectHandles[index] );
}

#ifndef IN_AHRS

AhrsObjHandle AhrsFromUAV( UAVObjHandle obj )
{
	if( objectHandles[0].uavHandle == NULL ) { //Oops - we haven't been initialised!
		PIOS_DEBUG_Assert( 0 );
	}
	for( int ct = 0; ct < MAX_AHRS_OBJECTS; ct++ ) {
		if( objectHandles[ct].uavHandle == obj ) {
			return( &objectHandles[ct] );
		}
	}
	return( NULL );
}


/** Callback to update AHRS from UAVObjects
*/
static void ObjectUpdatedCb(UAVObjEvent * ev)
{
    if(!(ev->event & EV_MASK_ALL_UPDATES))
    {
        return;
    }
	AhrsObjHandle hdl = AhrsFromUAV(ev->obj);
	if(hdl)
	{
		AhrsSharedData data; //this is guaranteed to be big enough
		UAVObjGetData(ev->obj,&data);
		AhrsSetData(hdl, &data);
	}
}

#endif


#ifndef AHRSCOMMS_H_INCLUDED
#define AHRSCOMMS_H_INCLUDED


#ifdef IN_AHRS  //AHRS only
#include <stdint.h>
#include <stdbool.h>

/** Redirect UAVObjGetData call
*/
#define UAVObjGetData(obj, data) AhrsGetData(obj, data)

/** Redirect UAVObjSetData call
*/
#define UAVObjSetData(obj, data) AhrsSetData(obj, data)

/** Redirect UAVObjConnectCallback call
Note: in AHRS, mask is unused because there is only one event type
*/
#define UAVObjConnectCallback(obj, callback, mask) AhrsConnectCallBack(obj,callback)

/** define our own UAVObjHandle
*/
typedef void *UAVObjHandle;

#else

#include "openpilot.h"
#include "uavobjectmanager.h"

#endif


#define AHRS_NO_OBJECT 0xff

#include "ahrs_comm_objects.h"

/** Status of each end of the link
*/
typedef struct { //try to keep this short and in multiples of 4 bytes
	uint8_t kickStarts; //AHRS end only
	uint8_t crcErrors;
	uint8_t retries;
	uint8_t invalidPacket;
} AhrsEndStatus;

/** AHRS comms status
*/
typedef struct {
	uint8_t linkOk;
	AhrsEndStatus remote;
	AhrsEndStatus local;
} AhrsCommStatus;

/** Event callback, this function is called when an object changes.
 */
typedef void ( *AhrsEventCallback )( AhrsObjHandle obj );

/** Initialise comms.
Note: this must be called before you do anything else.
Comms will not start until the first call to AhrsPoll() or
AhrsSendObjects()
 */
void AhrsInitComms( void );


/** AHRS version of UAVObject  xxxSetData.
Returns: 0 if ok, -1 if an error
 */
int32_t AhrsSetData( AhrsObjHandle obj, const void *dataIn );

/** AHRS version of UAVObject  xxxGetData.
Returns: 0 if ok, -1 if an error
 */
int32_t AhrsGetData( AhrsObjHandle obj, void *dataOut );

/** Connect a callback for any changes to AHRS data.
Returns: 0 if ok, -1 if an error
 */
int32_t AhrsConnectCallBack( AhrsObjHandle obj, AhrsEventCallback cb );

/** Get the current link status.
Returns: the status.
Note: the remote status will only be valid if the link is up and running
 */
AhrsCommStatus AhrsGetStatus();

#ifdef IN_AHRS  //slave only
/** Send the latest objects to the OP
This also polls any pending events and kick starts the DMA
if needed
*/
void AhrsPoll();

/** Check if the link is up and we have received the first batch of updates
Returns: True if the link is up and all objects are up to date
*/
bool AhrsLinkReady();

#else //master only



/** Send the latest objects to the AHRS
This also polls any pending events
*/
void AhrsSendObjects( void );

#endif

#endif //#ifndef AHRSCOMMS_H_INCLUDED

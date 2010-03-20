/**
 ******************************************************************************
 *
 * @file       GPS.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      GPS module, handles GPS and NMEA stream
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
#include "gpsinfo.h"
#include "buffer.h"


// constants/macros/typdefs
#define NMEA_BUFFERSIZE		110

// Message Codes
#define NMEA_NODATA		0	// No data. Packet not available, bad, or not decoded
#define NMEA_GPGGA		1	// Global Positioning System Fix Data
#define NMEA_GPVTG		2	// Course over ground and ground speed
#define NMEA_GPGLL		3	// Geographic position - latitude/longitude
#define NMEA_GPGSV		4	// GPS satellites in view
#define NMEA_GPGSA		5	// GPS DOP and active satellites
#define NMEA_GPRMC		6	// Recommended minimum specific GPS data
#define NMEA_UNKNOWN	0xFF// Packet received but not known

// Debugging

//#define GPSDEBUG

#ifdef GPSDEBUG
	#define NMEA_DEBUG_PKT	///< define to enable debug of all NMEA messages
	#define NMEA_DEBUG_GGA	///< define to enable debug of GGA messages
	#define NMEA_DEBUG_VTG	///< define to enable debug of VTG messages
#endif

// functions
void nmeaInit(void);
uint8_t* nmeaGetPacketBuffer(void);
uint8_t nmeaProcess(cBuffer* rxBuffer);
void nmeaProcessGPGGA(uint8_t* packet);
void nmeaProcessGPVTG(uint8_t* packet);

// Private functions
static void gpsTask(void* parameters);
static void periodicEventHandler(UAVObjEvent* ev);
static void registerObject(UAVObjHandle obj);
static void updateObject(UAVObjHandle obj);
static int32_t addObject(UAVObjHandle obj);
static int32_t setUpdatePeriod(UAVObjHandle obj, int32_t updatePeriodMs);

cBuffer gpsRxBuffer;
static char gpsRxData[512];
// Global variables
GpsInfoType GpsInfo;
uint8_t NmeaPacket[NMEA_BUFFERSIZE];

// Private constants
#define MAX_QUEUE_SIZE 20
#define STACK_SIZE 100
#define TASK_PRIORITY (tskIDLE_PRIORITY + 4)
#define REQ_TIMEOUT_MS 500
#define MAX_RETRIES 3

// Private types

// Private variables
static COMPortTypeDef gpsPort;
static xQueueHandle queue;
static xTaskHandle gpsTaskHandle;

/**
 * Initialise the gps module
 * \return -1 if initialisation failed
 * \return 0 on success
 */
int32_t GpsInitialize(void)
{
	// Create object queue
	queue = xQueueCreate(MAX_QUEUE_SIZE, sizeof(UAVObjEvent));

	// TODO: Get gps settings object
	gpsPort = COM_USART2;

	// Init input buffer size 512
	bufferInit(&gpsRxBuffer, (unsigned char *)gpsRxData, 512);

	// Process all registered objects and connect queue for updates
	UAVObjIterate(&registerObject);

	// Start gps task
	xTaskCreate(gpsTask, (signed char*)"GPS", STACK_SIZE, NULL, TASK_PRIORITY, &gpsTaskHandle);

	return 0;
}

/**
 * Register a new object, adds object to local list and connects the queue depending on the object's
 * telemetry settings.
 * \param[in] obj Object to connect
 */
void registerObject(UAVObjHandle obj)
{
	// Setup object for periodic updates
	addObject(obj);

	// Setup object for telemetry updates
	updateObject(obj);
}

/**
 * Update object's queue connections and timer, depending on object's settings
 * \param[in] obj Object to updates
 */
void updateObject(UAVObjHandle obj)
{
	UAVObjMetadata metadata;
	int32_t eventMask;

	// Get metadata
	UAVObjGetMetadata(obj, &metadata);

	// Setup object depending on update mode
	if(metadata.telemetryUpdateMode == UPDATEMODE_PERIODIC)
	{
		// Set update period
		setUpdatePeriod(obj, metadata.telemetryUpdatePeriod);
		// Connect queue
		eventMask = EV_UPDATED_MANUAL | EV_UPDATE_REQ;
		if(UAVObjIsMetaobject(obj))
		{
			eventMask |= EV_UNPACKED; // we also need to act on remote updates (unpack events)
		}
		UAVObjConnectQueue(obj, queue, eventMask);
	}
	else if(metadata.telemetryUpdateMode == UPDATEMODE_ONCHANGE)
	{
		// Set update period
		setUpdatePeriod(obj, 0);
		// Connect queue
		eventMask = EV_UPDATED | EV_UPDATED_MANUAL | EV_UPDATE_REQ;
		if(UAVObjIsMetaobject(obj))
		{
			eventMask |= EV_UNPACKED; // we also need to act on remote updates (unpack events)
		}
		UAVObjConnectQueue(obj, queue, eventMask);
	}
	else if(metadata.telemetryUpdateMode == UPDATEMODE_MANUAL)
	{
		// Set update period
		setUpdatePeriod(obj, 0);
		// Connect queue
		eventMask = EV_UPDATED_MANUAL | EV_UPDATE_REQ;
		if(UAVObjIsMetaobject(obj))
		{
			eventMask |= EV_UNPACKED; // we also need to act on remote updates (unpack events)
		}
		UAVObjConnectQueue(obj, queue, eventMask);
	}
	else if(metadata.telemetryUpdateMode == UPDATEMODE_NEVER)
	{
		// Set update period
		setUpdatePeriod(obj, 0);
		// Disconnect queue
		UAVObjDisconnectQueue(obj, queue);
	}
}

/**
 * gps task. Processes input buffer. It does not return.
 */
static void gpsTask(void* parameters)
{
	int32_t gpsRxOverflow=0;

	// Loop forever
	while(1)
	{
		/* This blocks the task until there is something on the buffer */
		while(PIOS_COM_ReceiveBufferUsed(gpsPort) > 0)
		{
			if( !bufferAddToEnd(&gpsRxBuffer, PIOS_COM_ReceiveBuffer(gpsPort)) )
			{
				// no space in buffer
				// count overflow
				gpsRxOverflow++;
				break;
			}
		}
		nmeaProcess(&gpsRxBuffer);
	}
}

/**
 * Event handler for periodic object updates (called by the event dispatcher)
 */
static void periodicEventHandler(UAVObjEvent* ev)
{
	// Push event to the telemetry queue
	xQueueSend(queue, ev, 0); // do not wait if queue is full
}

/**
 * Setup object for periodic updates.
 * \param[in] obj The object to update
 * \return 0 Success
 * \return -1 Failure
 */
static int32_t addObject(UAVObjHandle obj)
{
	UAVObjEvent ev;

	// Add object for periodic updates
	ev.obj = obj;
	ev.instId = UAVOBJ_ALL_INSTANCES;
	ev.event = EV_UPDATED_MANUAL;
	return EventPeriodicCreate(&ev, &periodicEventHandler, 0);
}

/**
 * Set update period of object (it must be already setup for periodic updates)
 * \param[in] obj The object to update
 * \param[in] updatePeriodMs The update period in ms, if zero then periodic updates are disabled
 * \return 0 Success
 * \return -1 Failure
 */
static int32_t setUpdatePeriod(UAVObjHandle obj, int32_t updatePeriodMs)
{
	UAVObjEvent ev;

	// Add object for periodic updates
	ev.obj = obj;
	ev.instId = UAVOBJ_ALL_INSTANCES;
	ev.event = EV_UPDATED_MANUAL;
	return EventPeriodicUpdate(&ev, &periodicEventHandler, updatePeriodMs);
}

uint8_t* nmeaGetPacketBuffer(void)
{
	return NmeaPacket;
}


/**
 * Prosesses NMEA sentences
 * \param[in] cBuffer for prosessed nmea sentences
 * \return Message code for found packet
 * \return 0xFF NO packet found
 */
uint8_t nmeaProcess(cBuffer* rxBuffer)
{
	uint8_t foundpacket = NMEA_NODATA;
	uint8_t startFlag = FALSE;
	//u08 data;
	uint16_t i,j;

	// process the receive buffer
	// go through buffer looking for packets
	while(rxBuffer->datalength)
	{
		// look for a start of NMEA packet
		if(bufferGetAtIndex(rxBuffer,0) == '$')
		{
			// found start
			startFlag = TRUE;
			// when start is found, we leave it intact in the receive buffer
			// in case the full NMEA string is not completely received.  The
			// start will be detected in the next nmeaProcess iteration.

			// done looking for start
			break;
		}
		else
			bufferGetFromFront(rxBuffer);
	}

	// if we detected a start, look for end of packet
	if(startFlag)
	{
		for(i=1; i<(rxBuffer->datalength)-1; i++)
		{
			// check for end of NMEA packet <CR><LF>
			if((bufferGetAtIndex(rxBuffer,i) == '\r') && (bufferGetAtIndex(rxBuffer,i+1) == '\n'))
			{
				// have a packet end
				// dump initial '$'
				bufferGetFromFront(rxBuffer);
				// copy packet to NmeaPacket
				for(j=0; j<(i-1); j++)
				{
					// although NMEA strings should be 80 characters or less,
					// receive buffer errors can generate erroneous packets.
					// Protect against packet buffer overflow
					if(j<(NMEA_BUFFERSIZE-1))
						NmeaPacket[j] = bufferGetFromFront(rxBuffer);
					else
						bufferGetFromFront(rxBuffer);
				}
				// null terminate it
				NmeaPacket[j] = 0;
				// dump <CR><LF> from rxBuffer
				bufferGetFromFront(rxBuffer);
				bufferGetFromFront(rxBuffer);
				//DEBUG
				//iprintf("$%s\r\n",NmeaPacket);
				//
				#ifdef NMEA_DEBUG_PKT
				printf("$%s\r\n",NmeaPacket);
				#endif
				// found a packet
				// done with this processing session
				foundpacket = NMEA_UNKNOWN;
				break;
			}
		}
	}

	if(foundpacket)
	{
		// check message type and process appropriately
		if(!strncmp((char *)NmeaPacket, "GPGGA", 5))
		{
			// process packet of this type
			nmeaProcessGPGGA(NmeaPacket);
			// report packet type
			foundpacket = NMEA_GPGGA;
		}
		else if(!strncmp((char *)NmeaPacket, "GPVTG", 5))
		{
			// process packet of this type
			nmeaProcessGPVTG(NmeaPacket);
			// report packet type
			foundpacket = NMEA_GPVTG;
		}
	}
	else if(rxBuffer->datalength >= rxBuffer->size)
	{
		// if we found no packet, and the buffer is full
		// we're logjammed, flush entire buffer
		bufferFlush(rxBuffer);
	}
	return foundpacket;
}

//#define CHAR_GPS

/**
 * Prosesses NMEA GPGGA sentences
 * \param[in] Buffer for parsed nmea GPGGA sentence
 */
void nmeaProcessGPGGA(uint8_t* packet)
{
	uint8_t i,j=0;
	char* endptr;
	double degrees, minutesfrac;

	#ifdef NMEA_DEBUG_GGA
	printf("NMEA: %s\r\n",packet);
	#endif

	// start parsing just after "GPGGA,"
	i = 6;
	// attempt to reject empty packets right away
	if(packet[i]==',' && packet[i+1]==',')
		return;

	// get UTC time [hhmmss.sss]
	GpsInfo.PosLLA.TimeOfFix.f = strtod((char *)&packet[i], &endptr);
	while(packet[i++] != ',');				// next field: latitude

	// get latitude [ddmm.mmmmm]
	GpsInfo.PosLLA.lat.f = strtod((char *)&packet[i], &endptr);
	// convert to pure degrees [dd.dddd] format
	minutesfrac = modf(GpsInfo.PosLLA.lat.f/100, &degrees);
	GpsInfo.PosLLA.lat.f = degrees + (minutesfrac*100)/60;
	// convert to radians
	GpsInfo.PosLLA.lat.f *= (M_PI/180);
#ifdef CHAR_GPS
	while(packet[i++] != ',')
		GpsInfo.PosLLA.lat.c[j++]=packet[i-1];				// next field: N/S indicator
	GpsInfo.PosLLA.lat.c[j]=0;
#else
	while(packet[i++] != ',');				// next field: N/S indicator
#endif
	// correct latitute for N/S
	if(packet[i] == 'S') GpsInfo.PosLLA.lat.f = -GpsInfo.PosLLA.lat.f;
	while(packet[i++] != ',');				// next field: longitude

	// get longitude [ddmm.mmmmm]
	GpsInfo.PosLLA.lon.f = strtod((char *)&packet[i], &endptr);
	// convert to pure degrees [dd.dddd] format
	minutesfrac = modf(GpsInfo.PosLLA.lon.f/100, &degrees);
	GpsInfo.PosLLA.lon.f = degrees + (minutesfrac*100)/60;
	// convert to radians
	GpsInfo.PosLLA.lon.f *= (M_PI/180);
#ifdef CHAR_GPS
	j=0;
	while(packet[i++] != ',')
		GpsInfo.PosLLA.lon.c[j++]=packet[i-1];				// next field: E/W indicator
	GpsInfo.PosLLA.lon.c[j]=0;
#else
	while(packet[i++] != ',');				// next field: E/W indicator
#endif

	// correct latitute for E/W
	if(packet[i] == 'W') GpsInfo.PosLLA.lon.f = -GpsInfo.PosLLA.lon.f;
	while(packet[i++] != ',');				// next field: position fix status

	// position fix status
	// 0 = Invalid, 1 = Valid SPS, 2 = Valid DGPS, 3 = Valid PPS
	// check for good position fix
	if( (packet[i] != '0') && (packet[i] != ',') )
		GpsInfo.PosLLA.updates++;
	while(packet[i++] != ',');				// next field: satellites used

	// get number of satellites used in GPS solution
	GpsInfo.numSVs = atoi((char *)&packet[i]);
	while(packet[i++] != ',');				// next field: HDOP (horizontal dilution of precision)
	while(packet[i++] != ',');				// next field: altitude

	// get altitude (in meters)
	GpsInfo.PosLLA.alt.f = strtod((char *)&packet[i], &endptr);

	while(packet[i++] != ',');				// next field: altitude units, always 'M'
	while(packet[i++] != ',');				// next field: geoid seperation
	while(packet[i++] != ',');				// next field: seperation units
	while(packet[i++] != ',');				// next field: DGPS age
	while(packet[i++] != ',');				// next field: DGPS station ID
	while(packet[i++] != '*');				// next field: checksum
}

/**
 * Prosesses NMEA GPVTG sentences
 * \param[in] Buffer for parsed nmea GPVTG sentence
 */
void nmeaProcessGPVTG(uint8_t* packet)
{
	uint8_t i;
	char* endptr;

	#ifdef NMEA_DEBUG_VTG
	printf("NMEA: %s\r\n",packet);
	#endif

	// start parsing just after "GPVTG,"
	i = 6;
	// attempt to reject empty packets right away
	if(packet[i]==',' && packet[i+1]==',')
		return;

	// get course (true north ref) in degrees [ddd.dd]
	GpsInfo.VelHS.heading.f = strtod((char *)&packet[i], &endptr);
	while(packet[i++] != ',');				// next field: 'T'
	while(packet[i++] != ',');				// next field: course (magnetic north)

	// get course (magnetic north ref) in degrees [ddd.dd]
	//GpsInfo.VelHS.heading.f = strtod(&packet[i], &endptr);
	while(packet[i++] != ',');				// next field: 'M'
	while(packet[i++] != ',');				// next field: speed (knots)

	// get speed in knots
	//GpsInfo.VelHS.speed.f = strtod(&packet[i], &endptr);
	while(packet[i++] != ',');				// next field: 'N'
	while(packet[i++] != ',');				// next field: speed (km/h)

	// get speed in km/h
	GpsInfo.VelHS.speed.f = strtod((char *)&packet[i], &endptr);
	while(packet[i++] != ',');				// next field: 'K'
	while(packet[i++] != '*');				// next field: checksum

	GpsInfo.VelHS.updates++;
}


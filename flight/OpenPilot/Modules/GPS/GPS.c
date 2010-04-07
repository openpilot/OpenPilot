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
#include "buffer.h"


// constants/macros/typdefs
#define NMEA_BUFFERSIZE		128
#define MAXTOKENS 20 //token slots for nmea parser


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

// Private functions
static void gpsTask(void* parameters);

// functions
char* nmeaGetPacketBuffer(void);
char nmeaChecksum(char* gps_buffer);
uint8_t nmeaProcess(cBuffer* rxBuffer);
void nmeaProcessGPGGA(char* packet);
void nmeaProcessGPVTG(char* packet);

cBuffer gpsRxBuffer;
static char gpsRxData[1024];
// Global variables
extern GpsInfoType GpsInfo;
char NmeaPacket[NMEA_BUFFERSIZE];

// Private constants
#define MAX_QUEUE_SIZE 20
#define STACK_SIZE 100
#define TASK_PRIORITY (tskIDLE_PRIORITY + 5)
#define REQ_TIMEOUT_MS 500
#define MAX_RETRIES 3

// Private types

// Private variables
static COMPortTypeDef gpsPort;
static xTaskHandle gpsTaskHandle;

/**
 * Initialise the gps module
 * \return -1 if initialisation failed
 * \return 0 on success
 */
int32_t GpsInitialize(void)
{
	// TODO: Get gps settings object
	gpsPort = COM_USART2;

	// Init input buffer size 512
	bufferInit(&gpsRxBuffer, (unsigned char *)gpsRxData, 1024);

	// Start gps task
	xTaskCreate(gpsTask, (signed char*)"GPS", configMINIMAL_STACK_SIZE, NULL, TASK_PRIORITY, &gpsTaskHandle);

	return 0;
}


/**
 * gps task. Processes input buffer. It does not return.
 */
static void gpsTask(void* parameters)
{
	int32_t gpsRxOverflow=0;
	char c;
	portTickType xDelay = 70 / portTICK_RATE_MS;

	// Loop forever
	while(1)
	{
		/* This blocks the task until there is something on the buffer */
		while(PIOS_COM_ReceiveBufferUsed(gpsPort) > 0)
		{
			c=PIOS_COM_ReceiveBuffer(gpsPort);
			if( !bufferAddToEnd(&gpsRxBuffer, c) )
			{
				// no space in buffer
				// count overflow
				gpsRxOverflow++;
				break;
			}
			//PIOS_COM_SendFormattedStringNonBlocking(COM_USART1, "%d\r", PIOS_COM_ReceiveBufferUsed(gpsPort));
			//vTaskDelay(xDelay);
		}
		nmeaProcess(&gpsRxBuffer);
		vTaskDelay(xDelay);
	}
}

char* nmeaGetPacketBuffer(void)
{
	return NmeaPacket;
}

/**
 * Prosesses NMEA sentence checksum
 * \param[in] Buffer for parsed nmea sentence
 * \return 0 checksum not valid
 * \return 1 checksum valid
 */
char nmeaChecksum(char* gps_buffer)
{
	char checksum=0;
	char checksum_received=0;

	for(int x=0; x<NMEA_BUFFERSIZE; x++)
	{
		if(gps_buffer[x]=='*')
		{
			//Parsing received checksum...
			checksum_received = strtol(&gps_buffer[x+1],NULL,16);
			break;
		}
		else
		{
			//XOR the received data...
			checksum^=gps_buffer[x];
		}
	}
	//PIOS_COM_SendFormattedStringNonBlocking(COM_DEBUG_USART,"$%d=%d\r\n",checksum_received,checksum);
	if(checksum == checksum_received)
		return 1;
	else
		return 0;

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
				//PIOS_COM_SendFormattedStringNonBlocking(COM_DEBUG_USART,"$%s\r\n",NmeaPacket);
				#endif
				// found a packet
				// done with this processing session
				foundpacket = NMEA_UNKNOWN;
				//PIOS_LED_Toggle(LED2);
				break;
			}
		}
	}

	if(foundpacket)
	{
		// check message type and process appropriately
		if(!strncmp(NmeaPacket, "GPGGA", 5))
		{
			// process packet of this type
			nmeaProcessGPGGA(NmeaPacket);
			// report packet type
			foundpacket = NMEA_GPGGA;
		}
		else if(!strncmp(NmeaPacket, "GPVTG", 5))
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

/**
 * Prosesses NMEA GPGGA sentences
 * \param[in] Buffer for parsed nmea GPGGA sentence
 */
void nmeaProcessGPGGA(char* packet)
{
	char *tokens; //*p, *tokens[MAXTOKENS];
    char *last;
    char *delimiter = ",";
    char *pEnd;

    uint8_t i=0;
    long deg,min,desim;
	double degrees, minutesfrac;

	#ifdef NMEA_DEBUG_GGA
	//PIOS_COM_SendFormattedStringNonBlocking(COM_USART1,"NMEA: %s\r\n",packet);
	#endif

	// start parsing just after "GPGGA,"
	// attempt to reject empty packets right away
	if(packet[6]==',' && packet[7]==',')
		return;

	if(!nmeaChecksum(packet))
	{
		// checksum not valid
		//PIOS_LED_Toggle(LED2);
		return;
	}
	// tokenizer for nmea sentence
    /*for ((p = strtok_r(packet, ",", &last)); p;
        (p = strtok_r(NULL, ",", &last)), i++) {
            if (i < MAXTOKENS - 1)
                    tokens[i] = p;
    }*/

	//GPGGA header
	tokens = strtok_r(packet, delimiter, &last);

	// get UTC time [hhmmss.sss]
	tokens = strtok_r(NULL, delimiter, &last);
	strcpy(GpsInfo.TimeOfFix,tokens);

	// next field: latitude
    // get latitude [ddmm.mmmmm]
	tokens = strtok_r(NULL, delimiter, &last);
	if(strlen(tokens)==10)// 5 desimal output
	{
		deg=strtol (tokens,&pEnd,10);
		min=deg%100;
		deg=deg/100;
		desim=strtol (pEnd+1,NULL,10);
		GpsInfo.lat=deg+(min+desim/100000.0)/60.0; // to desimal degrees
	}
	else if(strlen(tokens)==9) // 4 desimal output
	{
		deg=strtol (tokens,&pEnd,10);
		min=deg%100;
		deg=deg/100;
		desim=strtol (pEnd+1,NULL,10);
		GpsInfo.lat=deg+(min+desim/10000.0)/60.0; // to desimal degrees
	}
	// convert to pure degrees [dd.dddd] format
    /*	minutesfrac = modf(GpsInfo.PosLLA.lat.f/100, &degrees);
	GpsInfo.PosLLA.lat.f = degrees + (minutesfrac*100)/60;
	// convert to radians
	GpsInfo.PosLLA.lat.f *= (M_PI/180);*/

    // next field: N/S indicator
	// correct latitute for N/S
	tokens = strtok_r(NULL, delimiter, &last);
	if(tokens[0] == 'S') GpsInfo.lat = -GpsInfo.lat;

	// next field: longitude
	// get longitude [dddmm.mmmmm]
	tokens = strtok_r(NULL, delimiter, &last);
	if(strlen(tokens)==11)// 5 desimal output
	{
		deg=strtol (tokens,&pEnd,10);
		min=deg%100;
		deg=deg/100;
		desim=strtol (pEnd+1,NULL,10);
		GpsInfo.lon=deg+(min+desim/100000.0)/60.0; // to desimal degrees
	}
	else if(strlen(tokens)==10) // 4 desimal output
	{
		deg=strtol (tokens,&pEnd,10);
		min=deg%100;
		deg=deg/100;
		desim=strtol (pEnd+1,NULL,10);
		GpsInfo.lon=deg+(min+desim/10000.0)/60.0; // to desimal degrees
	}

	// convert to pure degrees [dd.dddd] format
	/*minutesfrac = modf(GpsInfo.PosLLA.lon.f/100, &degrees);
	GpsInfo.PosLLA.lon.f = degrees + (minutesfrac*100)/60;
	// convert to radians
	GpsInfo.PosLLA.lon.f *= (M_PI/180);*/

    // next field: E/W indicator
	// correct latitute for E/W
	tokens = strtok_r(NULL, delimiter, &last);
	if(tokens[0] == 'W') GpsInfo.lon = -GpsInfo.lon;

    // next field: position fix status
	// position fix status
	// 0 = Invalid, 1 = Valid SPS, 2 = Valid DGPS, 3 = Valid PPS
	// check for good position fix
	tokens = strtok_r(NULL, delimiter, &last);
    if((tokens[0] != '0') || (tokens[0] != 0))
		GpsInfo.updates++;

    // next field: satellites used
    // get number of satellites used in GPS solution
	tokens = strtok_r(NULL, delimiter, &last);
	GpsInfo.numSVs = atoi(tokens);

	// next field: HDOP (horizontal dilution of precision)
	tokens = strtok_r(NULL, delimiter, &last);//HDOP, nein gebraucht

	// next field: altitude
	// get altitude (in meters mm.m)
	tokens = strtok_r(NULL, delimiter, &last);
	//reuse variables for alt
	deg=strtol (tokens,&pEnd,10); // always 0.1m resolution?
	desim=strtol (pEnd+1,NULL,10);
	GpsInfo.alt=deg+desim/10.0;

	// next field: altitude units, always 'M'
	// next field: geoid seperation
	// next field: seperation units
	// next field: DGPS age
	// next field: DGPS station ID
	// next field: checksum
}

/**
 * Prosesses NMEA GPVTG sentences
 * \param[in] Buffer for parsed nmea GPVTG sentence
 */
void nmeaProcessGPVTG(char* packet)
{
	char *tokens; //*p, *tokens[MAXTOKENS];
    char *last;
    char *delimiter = ",";
    char *pEnd;

	uint8_t i=0;

	#ifdef NMEA_DEBUG_VTG
	//PIOS_COM_SendFormattedStringNonBlocking(COM_USART1,"NMEA: %s\r\n",packet);
	#endif

	// start parsing just after "GPVTG,"
	// attempt to reject empty packets right away
	if(packet[6]==',' && packet[7]==',')
		return;

	if(!nmeaChecksum(packet))
	{
		// checksum not valid
		//PIOS_LED_Toggle(LED2);
		return;
	}
	// tokenizer for nmea sentence
    /*for ((p = strtok_r(packet, ",", &last)); p;
        (p = strtok_r(NULL, ",", &last)), i++) {
            if (i < MAXTOKENS - 1)
                    tokens[i] = p;
    }*/

	//GPVTG header
	tokens = strtok_r(packet, delimiter, &last);

	// get course (true north ref) in degrees [ddd.dd]
	tokens = strtok_r(NULL, delimiter, &last);
    strcpy(GpsInfo.heading,tokens);
    // next field: 'T'
	tokens = strtok_r(NULL, delimiter, &last);

    // next field: course (magnetic north)
 	// get course (magnetic north ref) in degrees [ddd.dd]
	tokens = strtok_r(NULL, delimiter, &last);
    //strcpy(GpsInfo.VelHS.heading.c,tokens[1]);
	// next field: 'M'
	tokens = strtok_r(NULL, delimiter, &last);

	// next field: speed (knots)
	// get speed in knots
	tokens = strtok_r(NULL, delimiter, &last);
    //strcpy(GpsInfo.VelHS.speed.f,tokens[5]);
	// next field: 'N'
	tokens = strtok_r(NULL, delimiter, &last);

	// next field: speed (km/h)
	// get speed in km/h
	tokens = strtok_r(NULL, delimiter, &last);
	strcpy(GpsInfo.speed,tokens);
	// next field: 'K'
	// next field: checksum

	GpsInfo.updates++;
}


/**
 ******************************************************************************
 * @addtogroup OpenPilotModules OpenPilot Modules
 * @{ 
 * @addtogroup GSPModule GPS Module
 * @brief Process GPS information
 * @{ 
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
#include "GPS.h"
#include <stdbool.h>
#include "NMEA.h"
#include "gpsposition.h"
#include "homelocation.h"
#include "WorldMagModel.h"
#include "CoordinateConversions.h"

// constants/macros/typdefs
#define NMEA_BUFFERSIZE		128

#define GPS_TIMEOUT_MS 500

// Private functions

static bool GPS_copy_sentence_from_cbuffer (char * dest, uint32_t dest_len, cBuffer * rxBuffer);
static void gpsTask(void* parameters);
static void setHomeLocation(GPSPositionData * gpsData);

// Global variables

// Private constants
// Unfortunately need a good size stack for the WMM calculation
#define STACK_SIZE configMINIMAL_STACK_SIZE + 2000
#define TASK_PRIORITY (tskIDLE_PRIORITY + 3)

// Private types

// Private variables
static uint8_t     gpsPort;
static xTaskHandle gpsTaskHandle;
static cBuffer     gpsRxBuffer;
static char        gpsRxData[512];
static char        NmeaPacket[NMEA_BUFFERSIZE];

static uint32_t    timeOfLastUpdateMs;
static uint32_t    numUpdates;
static uint32_t    numChecksumErrors;
static uint32_t    numParsingErrors;

/**
 * Initialise the gps module
 * \return -1 if initialisation failed
 * \return 0 on success
 */
int32_t GPSInitialize(void)
{
  signed portBASE_TYPE xReturn;

  // TODO: Get gps settings object
  gpsPort = PIOS_COM_GPS;

  // Init input buffer size 512
  bufferInit(&gpsRxBuffer, (unsigned char *)gpsRxData, 512);

  // Start gps task
  xReturn = xTaskCreate(gpsTask, (signed char*)"GPS", STACK_SIZE, NULL, TASK_PRIORITY, &gpsTaskHandle);

  return 0;
}

/**
 * gps task. Processes input buffer. It does not return.
 */
static void gpsTask(void* parameters)
{
  int32_t         gpsRxOverflow = 0;
  char            c;
  portTickType    xDelay = 100 / portTICK_RATE_MS;
  GPSPositionData GpsData;
  uint32_t        timeNowMs;
  
  // Loop forever
  while (1) {
    /* This blocks the task until there is something on the buffer */
    while (PIOS_COM_ReceiveBufferUsed(gpsPort) > 0) {
      c = PIOS_COM_ReceiveBuffer(gpsPort);
      if (!bufferAddToEnd(&gpsRxBuffer, c)) {
	/* 
	 * The buffer is already full and we haven't found a valid NMEA sentence.
	 * Flush the buffer and note the overflow event.
	 */
	gpsRxOverflow++;
	bufferFlush (&gpsRxBuffer);
      } else {
	/* Grab the next available complete NMEA sentence from the Rx buffer */
	if (GPS_copy_sentence_from_cbuffer (NmeaPacket, sizeof(NmeaPacket), &gpsRxBuffer)) {
	  /* Validate the checksum over the sentence */
	  if (!NMEA_checksum (NmeaPacket)) {
	    /* Invalid checksum.  May indicate dropped characters on Rx. */
	    ++numChecksumErrors;
	  } else {
	    /* Valid checksum, use this packet to update the GPS position */
	    if (!NMEA_update_position (NmeaPacket)) {
	      ++numParsingErrors;
	    } else {
	      ++numUpdates;
	    }
	    timeOfLastUpdateMs = xTaskGetTickCount() * portTICK_RATE_MS;
	  }
	}
      }
    }

    // Check for GPS timeout
    timeNowMs = xTaskGetTickCount() * portTICK_RATE_MS;
    if ((timeNowMs - timeOfLastUpdateMs) > GPS_TIMEOUT_MS) {
      GPSPositionGet(&GpsData);
      GpsData.Status = GPSPOSITION_STATUS_NOGPS;
      GPSPositionSet(&GpsData);
    } else {
      // Had an update
      HomeLocationData home;
      HomeLocationGet(&home);
      
      GPSPositionGet(&GpsData);
      if ((GpsData.Status == GPSPOSITION_STATUS_FIX3D) && 
	  (home.Set       == HOMELOCATION_SET_FALSE)) {
        setHomeLocation(&GpsData);
      }
    }

    // Block task until next update
    vTaskDelay(xDelay);
  }
}

static bool GPS_copy_sentence_from_cbuffer (char * dest, uint32_t dest_len, cBuffer * rxBuffer)
{
  /* Throw away all initial characters from the Rx buffer that are not the NMEA start flag '$' */
  bool startFlag = false;
  while (rxBuffer->datalength && !startFlag) {
    if (bufferGetAtIndex (rxBuffer, 0) == '$') {
      startFlag = true;
    } else {
      bufferGetFromFront(rxBuffer);
    }
  }

  if (!startFlag) {
    /* No start of sentence located, bail out */
    return false;
  }

  /* rxBuffer is now positioned at the start of sentence marker */

  /* Start of sentence located, look for an end of sentence marker '\r\n' */
  bool     endFlag = false;
  uint16_t end_pos;
  for (end_pos = 2; end_pos < rxBuffer->datalength; end_pos++) {
    // check for end of NMEA sentence '\r\n'
    if ((bufferGetAtIndex(rxBuffer, end_pos-1) == '\r') && 
	(bufferGetAtIndex(rxBuffer, end_pos)   == '\n')) {
      /* Found the end of the packet */
      endFlag = true;
    }
  }

  if (!endFlag) {
    /* No end of sentence located, bail out and wait for more data */
    return false;
  }

  /*
   * Our rxBuffer must look like this now:
   *   [0]           = '$'
   *   ...           = zero or more bytes of sentence payload
   *   [end_pos - 1] = '\r'
   *   [end_pos]     = '\n'
   *
   * Prepare to consume the sentence from the buffer
   */
  uint32_t payload_len = end_pos - 2; /* not counting the '$' or '\r\n' */

  /* Drop the start flag '$' */
  (void) bufferGetFromFront (rxBuffer);

  bool truncated = false;
  while (payload_len--) {
    if (dest_len > 1) {
      /* Put the payload data into the buffer as long as there is room */
      *dest = bufferGetFromFront (rxBuffer);
      dest++;
      dest_len--;
    } else {
      /* 
       * No more room in the dest buffer.  Note that the sentence was truncated and
       * continue to flush the rxBuffer.
       */
      truncated = true;
      (void) bufferGetFromFront (rxBuffer);
    }
  }

  /* Drop the end marker '\r\n' */
  (void) bufferGetFromFront (rxBuffer);
  (void) bufferGetFromFront (rxBuffer);

  /* NULL terminate the dest buffer.  We left 1 byte at the end (see 'dest_len > 1' above) */
  *dest = '\0';

  return (!truncated);
}

static void setHomeLocation(GPSPositionData * gpsData) 
{
  HomeLocationData home;
  HomeLocationGet(&home);
  
  // Store LLA
  home.Latitude  = gpsData->Latitude;
  home.Longitude = gpsData->Longitude;
  home.Altitude  = gpsData->GeoidSeparation;
  
  // Compute home ECEF coordinates and the rotation matrix into NED
  double LLA[3] = {(double) home.Latitude / 10e6, (double) home.Longitude / 10e6, (double) home.Altitude};
  double ECEF[3];
  RneFromLLA(LLA, (float (*)[3]) home.RNE);
  LLA2ECEF(LLA, ECEF);
  // TODO: Currently UAVTalk only supports float but these conversions use double
  // need to find out if they require that precision and if so extend UAVTAlk
  home.ECEF[0] = (int32_t) (ECEF[0] * 100);
  home.ECEF[1] = (int32_t) (ECEF[1] * 100);
  home.ECEF[2] = (int32_t) (ECEF[2] * 100);
  
  // Compute magnetic flux direction at home location
  WMM_GetMagVector(LLA[0], LLA[1], LLA[2], 8, 17, 2010, &home.Be[0]);
  
  home.Set = HOMELOCATION_SET_TRUE;
  HomeLocationSet(&home);
}



/** 
  * @}
  * @}
  */

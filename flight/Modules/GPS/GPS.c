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

// ****************

#include "openpilot.h"
#include "GPS.h"

#include <stdbool.h>

#include "NMEA.h"

#include "gpsposition.h"
#include "homelocation.h"
#include "gpstime.h"
#include "gpssatellites.h"
#include "WorldMagModel.h"
#include "CoordinateConversions.h"

// ****************
// Private functions

static void gpsTask(void *parameters);

#ifdef PIOS_GPS_SETS_HOMELOCATION
static void setHomeLocation(GPSPositionData * gpsData);
static float GravityAccel(float latitude, float longitude, float altitude);
#endif

// ****************
// Private constants

#define GPS_TIMEOUT_MS                  500
#define NMEA_MAX_PACKET_LENGTH          96 // 82 max NMEA msg size plus 12 margin (because some vendors add custom crap) plus CR plus Linefeed
// same as in COM buffer


#ifdef PIOS_GPS_SETS_HOMELOCATION
// Unfortunately need a good size stack for the WMM calculation
	#define STACK_SIZE_BYTES            800
#else
	#define STACK_SIZE_BYTES            650
#endif

#define TASK_PRIORITY                   (tskIDLE_PRIORITY + 1)

// ****************
// Private variables

static uint32_t gpsPort;

static xTaskHandle gpsTaskHandle;

static char* gps_rx_buffer;

static uint32_t timeOfLastCommandMs;
static uint32_t timeOfLastUpdateMs;
static uint32_t numUpdates;
static uint32_t numChecksumErrors;
static uint32_t numParsingErrors;

// ****************
/**
 * Initialise the gps module
 * \return -1 if initialisation failed
 * \return 0 on success
 */

int32_t GPSStart(void)
{
	// Start gps task
	xTaskCreate(gpsTask, (signed char *)"GPS", STACK_SIZE_BYTES/4, NULL, TASK_PRIORITY, &gpsTaskHandle);
	TaskMonitorAdd(TASKINFO_RUNNING_GPS, gpsTaskHandle);

	return 0;
}
/**
 * Initialise the gps module
 * \return -1 if initialisation failed
 * \return 0 on success
 */
int32_t GPSInitialize(void)
{
	GPSPositionInitialize();
	GPSTimeInitialize();
	GPSSatellitesInitialize();
#ifdef PIOS_GPS_SETS_HOMELOCATION
	HomeLocationInitialize();
#endif
	
	// TODO: Get gps settings object
	gpsPort = PIOS_COM_GPS;

	gps_rx_buffer = pvPortMalloc(NMEA_MAX_PACKET_LENGTH);

	return 0;
}
MODULE_INITCALL(GPSInitialize, GPSStart)

// ****************
/**
 * Main gps task. It does not return.
 */

static void gpsTask(void *parameters)
{
	portTickType xDelay = 100 / portTICK_RATE_MS;
	uint32_t timeNowMs = xTaskGetTickCount() * portTICK_RATE_MS;;
	GPSPositionData GpsData;
	
	uint8_t rx_count = 0;
	bool start_flag = false;
	bool found_cr = false;
	int32_t gpsRxOverflow = 0;
	
	numUpdates = 0;
	numChecksumErrors = 0;
	numParsingErrors = 0;

	timeOfLastUpdateMs = timeNowMs;
	timeOfLastCommandMs = timeNowMs;

	// Loop forever
	while (1)
	{
		uint8_t c;
		// NMEA or SINGLE-SENTENCE GPS mode

		// This blocks the task until there is something on the buffer
		while (PIOS_COM_ReceiveBuffer(gpsPort, &c, 1, xDelay) > 0)
		{
		
			// detect start while acquiring stream
			if (!start_flag && (c == '$'))
			{
				start_flag = true;
				found_cr = false;
				rx_count = 0;
			}
			else
			if (!start_flag)
				continue;
		
			if (rx_count >= NMEA_MAX_PACKET_LENGTH)
			{
				// The buffer is already full and we haven't found a valid NMEA sentence.
				// Flush the buffer and note the overflow event.
				gpsRxOverflow++;
				start_flag = false;
				found_cr = false;
				rx_count = 0;
			}
			else
			{
				gps_rx_buffer[rx_count] = c;
				rx_count++;
			}
		
			// look for ending '\r\n' sequence
			if (!found_cr && (c == '\r') )
				found_cr = true;
			else
			if (found_cr && (c != '\n') )
				found_cr = false;  // false end flag
			else
			if (found_cr && (c == '\n') )
			{
				// The NMEA functions require a zero-terminated string
				// As we detected \r\n, the string as for sure 2 bytes long, we will also strip the \r\n
				gps_rx_buffer[rx_count-2] = 0;

				// prepare to parse next sentence
				start_flag = false;
				found_cr = false;
				rx_count = 0;
				// Our rxBuffer must look like this now:
				//   [0]           = '$'
				//   ...           = zero or more bytes of sentence payload
				//   [end_pos - 1] = '\r'
				//   [end_pos]     = '\n'
				//
				// Prepare to consume the sentence from the buffer
			
				// Validate the checksum over the sentence
				if (!NMEA_checksum(&gps_rx_buffer[1]))
				{	// Invalid checksum.  May indicate dropped characters on Rx.
					//PIOS_DEBUG_PinHigh(2);
					++numChecksumErrors;
					//PIOS_DEBUG_PinLow(2);
				}
				else
				{	// Valid checksum, use this packet to update the GPS position
					if (!NMEA_update_position(&gps_rx_buffer[1])) {
						//PIOS_DEBUG_PinHigh(2);
						++numParsingErrors;
						//PIOS_DEBUG_PinLow(2);
					}
					else
						++numUpdates;

					timeNowMs = xTaskGetTickCount() * portTICK_RATE_MS;
					timeOfLastUpdateMs = timeNowMs;
					timeOfLastCommandMs = timeNowMs;
				}
			}
		}

		// Check for GPS timeout
		timeNowMs = xTaskGetTickCount() * portTICK_RATE_MS;
		if ((timeNowMs - timeOfLastUpdateMs) >= GPS_TIMEOUT_MS)
		{	// we have not received any valid GPS sentences for a while.
			// either the GPS is not plugged in or a hardware problem or the GPS has locked up.

			GPSPositionGet(&GpsData);
			GpsData.Status = GPSPOSITION_STATUS_NOGPS;
			GPSPositionSet(&GpsData);
			AlarmsSet(SYSTEMALARMS_ALARM_GPS, SYSTEMALARMS_ALARM_ERROR);

		}
		else
		{	// we appear to be receiving GPS sentences OK, we've had an update

			GPSPositionGet(&GpsData);

#ifdef PIOS_GPS_SETS_HOMELOCATION
			HomeLocationData home;
			HomeLocationGet(&home);

			if ((GpsData.Status == GPSPOSITION_STATUS_FIX3D) && (home.Set == HOMELOCATION_SET_FALSE))
				setHomeLocation(&GpsData);
#endif

			//criteria for GPS-OK taken from this post...
			//http://forums.openpilot.org/topic/1523-professors-insgps-in-svn/page__view__findpost__p__5220
			if ((GpsData.PDOP < 3.5) && (GpsData.Satellites >= 7))
				AlarmsClear(SYSTEMALARMS_ALARM_GPS);
			else
			if (GpsData.Status == GPSPOSITION_STATUS_FIX3D)
				AlarmsSet(SYSTEMALARMS_ALARM_GPS, SYSTEMALARMS_ALARM_WARNING);
			else
				AlarmsSet(SYSTEMALARMS_ALARM_GPS, SYSTEMALARMS_ALARM_CRITICAL);
		}

	}
}

#ifdef PIOS_GPS_SETS_HOMELOCATION
/*
 * Estimate the acceleration due to gravity for a particular location in LLA
 */
static float GravityAccel(float latitude, float longitude, float altitude)
{
	// WGS84 gravity model.  The effect of gravity over latitude is strong
	// enough to change the estimated accelerometer bias in those apps.
	double sinsq = sin((double)latitude);
	sinsq *= sinsq;
	// Likewise, over the altitude range of a high-altitude balloon, the effect
	// due to change in altitude can also affect the model.
	return (float)(9.7803267714 * (1 + 0.00193185138639*sinsq) / sqrt(1 - 0.00669437999013*sinsq)
		- 3.086e-6*altitude);
}

// ****************

static void setHomeLocation(GPSPositionData * gpsData)
{
	HomeLocationData home;
	HomeLocationGet(&home);
	GPSTimeData gps;
	GPSTimeGet(&gps);

	if (gps.Year >= 2000)
	{
		// Store LLA
		home.Latitude = gpsData->Latitude;
		home.Longitude = gpsData->Longitude;
		home.Altitude = gpsData->Altitude + gpsData->GeoidSeparation;

		// Compute home ECEF coordinates and the rotation matrix into NED
		double LLA[3] = { ((double)home.Latitude) / 10e6, ((double)home.Longitude) / 10e6, ((double)home.Altitude) };
		double ECEF[3];
		RneFromLLA(LLA, (float (*)[3])home.RNE);
		LLA2ECEF(LLA, ECEF);
		// TODO: Currently UAVTalk only supports float but these conversions use double
		// need to find out if they require that precision and if so extend UAVTAlk
		home.ECEF[0] = (int32_t) (ECEF[0] * 100);
		home.ECEF[1] = (int32_t) (ECEF[1] * 100);
		home.ECEF[2] = (int32_t) (ECEF[2] * 100);

		// Compute magnetic flux direction at home location
		if (WMM_GetMagVector(LLA[0], LLA[1], LLA[2], gps.Month, gps.Day, gps.Year, &home.Be[0]) >= 0)
		{   // calculations appeared to go OK

			// Compute local acceleration due to gravity.  Vehicles that span a very large
			// range of altitude (say, weather balloons) may need to update this during the
			// flight.
			home.g_e = GravityAccel(LLA[0], LLA[1], LLA[2]);
			home.Set = HOMELOCATION_SET_TRUE;
			HomeLocationSet(&home);
		}
	}
}
#endif

// ****************

/** 
  * @}
  * @}
  */

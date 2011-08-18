/**
 ******************************************************************************
 * @addtogroup OpenPilotModules OpenPilot Modules
 * @{
 * @addtogroup GSPModule GPS Module
 * @brief Process GPS information
 * @{
 *
 * @file       NMEA.c
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
#include "pios.h"
#include "NMEA.h"
#include "gpsposition.h"
#include "gpstime.h"
#include "gpssatellites.h"

//#define ENABLE_DEBUG_MSG						///< define to enable debug-messages
#define DEBUG_PORT		PIOS_COM_TELEM_RF		///< defines which serial port is ued for debug-messages



#if defined(ENABLE_GPS_NMEA) || defined(ENABLE_GPS_ONESENTENCE_GTOP)

// Debugging
#ifdef ENABLE_DEBUG_MSG
//#define DEBUG_MSG_IN			///< define to display the incoming NMEA messages
//#define DEBUG_PARS			///< define to display the incoming NMEA messages split into its parameters
//#define DEBUG_MGSID_IN		///< define to display the the names of the incoming NMEA messages
//#define NMEA_DEBUG_PKT		///< define to enable debug of all NMEA messages
//#define NMEA_DEBUG_GGA		///< define to enable debug of GGA messages
//#define NMEA_DEBUG_VTG		///< define to enable debug of VTG messages
//#define NMEA_DEBUG_RMC		///< define to enable debug of RMC messages
//#define NMEA_DEBUG_GSA		///< define to enable debug of GSA messages
//#define NMEA_DEBUG_GSV		///< define to enable debug of GSV messages
//#define NMEA_DEBUG_ZDA		///< define to enable debug of ZDA messages
//#define NMEA_DEBUG_PGTOP	///< define to enable debug of PGTOP messages
#define DEBUG_MSG(format, ...) PIOS_COM_SendFormattedString(DEBUG_PORT, format, ## __VA_ARGS__)
#else
#define DEBUG_MSG(format, ...)
#endif

#define MAX_NB_PARAMS 20
/* NMEA sentence parsers */

struct nmea_parser {
	const char *prefix;
	bool(*handler) (GPSPositionData * GpsData, bool* gpsDataUpdated, char* param[], uint8_t nbParam);
	uint32_t cnt;
};

	#ifdef ENABLE_GPS_NMEA
		static bool nmeaProcessGPGGA(GPSPositionData * GpsData, bool* gpsDataUpdated, char* param[], uint8_t nbParam);
		static bool nmeaProcessGPRMC(GPSPositionData * GpsData, bool* gpsDataUpdated, char* param[], uint8_t nbParam);
		static bool nmeaProcessGPVTG(GPSPositionData * GpsData, bool* gpsDataUpdated, char* param[], uint8_t nbParam);
		static bool nmeaProcessGPGSA(GPSPositionData * GpsData, bool* gpsDataUpdated, char* param[], uint8_t nbParam);
#if !defined(PIOS_GPS_PURISTIC)
		static bool nmeaProcessGPZDA(GPSPositionData * GpsData, bool* gpsDataUpdated, char* param[], uint8_t nbParam);
		static bool nmeaProcessGPGSV(GPSPositionData * GpsData, bool* gpsDataUpdated, char* param[], uint8_t nbParam);
#endif //PIOS_GPS_PURISTIC
	#endif

	static bool nmeaProcessPGTOP(GPSPositionData * GpsData, bool* gpsDataUpdated, char* param[], uint8_t nbParam);

	static struct nmea_parser nmea_parsers[] = {

	#ifdef ENABLE_GPS_NMEA
		{
			.prefix = "GPGGA",
			.handler = nmeaProcessGPGGA,
			.cnt = 0,
		},
		{
			.prefix = "GPVTG",
			.handler = nmeaProcessGPVTG,
			.cnt = 0,
		},
		{
			.prefix = "GPGSA",
			.handler = nmeaProcessGPGSA,
			.cnt = 0,
		},
		{
			.prefix = "GPRMC",
			.handler = nmeaProcessGPRMC,
			.cnt = 0,
		},
#if !defined(PIOS_GPS_PURISTIC)
		{
			.prefix = "GPZDA",
			.handler = nmeaProcessGPZDA,
			.cnt = 0,
		},
		{
			.prefix = "GPGSV",
			.handler = nmeaProcessGPGSV,
			.cnt = 0,
		},
#endif //PIOS_GPS_PURISTIC
	#endif
        {
         .prefix = "PGTOP",
         .handler = nmeaProcessPGTOP,
         .cnt = 0,
         },
};

static struct nmea_parser *NMEA_find_parser_by_prefix(const char *prefix)
{
	if (!prefix) {
		return (NULL);
	}

	for (uint8_t i = 0; i < NELEMENTS(nmea_parsers); i++) {
		struct nmea_parser *parser = &nmea_parsers[i];

		/* Use strcmp to check for exact equality over the entire prefix */
		if (!strcmp(prefix, parser->prefix)) {
			/* Found an appropriate parser */
			return (parser);
		}
	}

	/* No matching parser for this prefix */
	return (NULL);
}

/**
 * Computes NMEA sentence checksum
 * \param[in] Buffer for parsed nmea sentence
 * \return false checksum not valid
 * \return true checksum valid
 */
bool NMEA_checksum(char *nmea_sentence)
{
	uint8_t checksum_computed = 0;
	uint8_t checksum_received;

	while (*nmea_sentence != '\0' && *nmea_sentence != '*') {
		checksum_computed ^= *nmea_sentence;
		nmea_sentence++;
	}

	/* Make sure we're now pointing at the checksum */
	if (*nmea_sentence == '\0') {
		/* Buffer ran out before we found a checksum marker */
		return false;
	}

	/* Load the checksum from the buffer */
	checksum_received = strtol(nmea_sentence + 1, NULL, 16);

	//PIOS_COM_SendFormattedStringNonBlocking(COM_DEBUG_USART,"$%d=%d\r\n",checksum_received,checksum_computed);

	return (checksum_computed == checksum_received);
}

/*
 * This function only exists to deal with a linking
 * failure in the stdlib function strtof().  This
 * implementation does not rely on the _sbrk() syscall
 * like strtof() does.
 */

/* Parse a number encoded in a string of the format:
 *   [-]NN.nnnnn
 * into a signed whole part and an unsigned fractional part.
 * The fract_units field indicates the units of the fractional part as
 *   1 whole = 10^fract_units fract
 */
static bool NMEA_parse_real(int32_t * whole, uint32_t * fract, uint8_t * fract_units, char *field)
{
	char *s = field;
	char *field_w;
	char *field_f;

	PIOS_DEBUG_Assert(whole);
	PIOS_DEBUG_Assert(fract);
	PIOS_DEBUG_Assert(fract_units);

	field_w = strsep(&s, ".");
	field_f = s;

	*whole = strtol(field_w, NULL, 10);

	if (field_w) {
		/* decimal was found so we may have a fractional part */
		*fract = strtoul(field_f, NULL, 10);
		*fract_units = strlen(field_f);
	} else {
		/* no decimal was found, fractional part is zero */
		*fract = 0;
		*fract_units = 0;
	}

	return true;
}

static float NMEA_real_to_float(char *nmea_real)
{
	int32_t whole;
	uint32_t fract;
	uint8_t fract_units;

	/* Sanity checks */
	PIOS_DEBUG_Assert(nmea_real);

	if (!NMEA_parse_real(&whole, &fract, &fract_units, nmea_real)) {
		return false;
	}

	/* Convert to float */
	return (((float)whole) + fract * pow(10, -fract_units));
}

#ifdef ENABLE_GPS_NMEA
/*
 * Parse a field in the format:
 *    DD[D]MM.mmmm[mm]
 * into a fixed-point representation in units of (degrees * 1e-7)
 */
static bool NMEA_latlon_to_fixed_point(int32_t * latlon, char *nmea_latlon, bool negative)
{
	int32_t num_DDDMM;
	uint32_t num_m;
	uint8_t units;

	/* Sanity checks */
	PIOS_DEBUG_Assert(nmea_latlon);
	PIOS_DEBUG_Assert(latlon);

	if (!NMEA_parse_real(&num_DDDMM, &num_m, &units, nmea_latlon)) {
		return false;
	}

	/* scale up the mmmm[mm] field apropriately depending on # of digits */
	switch (units) {
	case 0:
		/* no digits, value is zero so no scaling */
		break;
	case 1:		/* m       */
		num_m *= 1e6;	/* m000000 */
		break;
	case 2:		/* mm      */
		num_m *= 1e5;	/* mm00000 */
		break;
	case 3:		/* mmm     */
		num_m *= 1e4;	/* mmm0000 */
		break;
	case 4:		/* mmmm    */
		num_m *= 1e3;	/* mmmm000 */
		break;
	case 5:		/* mmmmm   */
		num_m *= 1e2;	/* mmmmm00 */
		break;
	case 6:		/* mmmmmm  */
		num_m *= 1e1;	/* mmmmmm0 */
		break;
	default:
		/* unhandled format */
		num_m = 0;
		break;
	}

	*latlon = (num_DDDMM / 100) * 1e7;	/* scale the whole degrees */
	*latlon += (num_DDDMM % 100) * 1e7 / 60;	/* add in the scaled decimal whole minutes */
	*latlon += num_m / 60;	/* add in the scaled decimal fractional minutes */

	if (negative)
		*latlon *= -1;

	return true;
}
#endif	// ENABLE_GPS_NMEA


/**
 * Parses a complete NMEA sentence and updates the GPSPosition UAVObject
 * \param[in] An NMEA sentence with a valid checksum
 * \return true if the sentence was successfully parsed
 * \return false if any errors were encountered with the parsing
 */
bool NMEA_update_position(char *nmea_sentence)
{
	char* p = nmea_sentence;
	char* params[MAX_NB_PARAMS];
	uint8_t nbParams;

#ifdef DEBUG_MSG_IN
	DEBUG_MSG("\"%s\"\n", nmea_sentence);
#endif

	// Split the nmea sentence it its parameters, separated by ","
	// Sample NMEA message: "GPRMC,000131.736,V,,,,,0.00,0.00,060180,,,N*43"

	// The first parameter starts at the beginning of the message
	params[0] = p;
	nbParams = 1;
	while (*p != 0) {
		if (*p == '*') {
			// After the * comes the "CRC", we are done,
			*p = 0;		// Zero-terminate this parameter
			break;
		} else if (*p == ',') {
			// This is the end of this parameter
			*p = 0;		// Zero-terminate this parameter
			// Start new parameter
			if (nbParams==MAX_NB_PARAMS)
				break;
			params[nbParams] = p+1;	// For sure there is something at p+1 because at p there is ","
			nbParams++;
		}
		p++;
	}


#ifdef DEBUG_PARAMS
	int i;
	for (i=0;i<nbParams; i++) {
		DEBUG_MSG(" %d \"%s\"\n", i, params[i]);
	}
#endif

	// The first parameter is the message name, lets see if we find a parser for it
	struct nmea_parser *parser;
	parser = NMEA_find_parser_by_prefix(params[0]);
	if (!parser) {
		// No parser found
		DEBUG_MSG(" NO PARSER (\"%s\")\n", params[0]);
		return false;
	}

	parser->cnt++;
	#ifdef DEBUG_MGSID_IN
		DEBUG_MSG("%s %d ", params[0], parser->cnt);
	#endif
	// Send the message to then parser and get it update the GpsData
	GPSPositionData GpsData;
	GPSPositionGet(&GpsData);
	bool gpsDataUpdated;

	if (!parser->handler(&GpsData, &gpsDataUpdated, params, nbParams)) {
		// Parse failed
		DEBUG_MSG("PARSE FAILED (\"%s\")\n", params[0]);
		return false;
	}


	// All is fine :)  Update object if data has changed
	if (gpsDataUpdated) {
		#ifdef DEBUG_MGSID_IN
			DEBUG_MSG("U");
		#endif
		GPSPositionSet(&GpsData);
	}

	#ifdef DEBUG_MGSID_IN
		DEBUG_MSG("\n");
	#endif
	return true;
}

#ifdef ENABLE_GPS_NMEA

/**
 * Parse an NMEA GPGGA sentence and update the given UAVObject
 * \param[in] A pointer to a GPSPosition UAVObject to be updated.
 * \param[in] An NMEA sentence with a valid checksum
 */
static bool nmeaProcessGPGGA(GPSPositionData * GpsData, bool* gpsDataUpdated, char* param[], uint8_t nbParam)
{

	if (nbParam != 15)
		return false;

#ifdef  NMEA_DEBUG_GGA
	DEBUG_MSG("\n UTC=%s\n", param[1]);
	DEBUG_MSG(" Lat=%s %s\n", param[2], param[3]);
	DEBUG_MSG(" Long=%s %s\n", param[4], param[5]);
	DEBUG_MSG(" Fix=%s\n", param[6]);
	DEBUG_MSG(" Sat=%s\n", param[7]);
	DEBUG_MSG(" HDOP=%s\n", param[8]);
	DEBUG_MSG(" Alt=%s %s\n", param[9], param[10]);
	DEBUG_MSG(" GeoidSep=%s %s\n\n", param[11]);
#endif

	*gpsDataUpdated = true;

	// get latitude [DDMM.mmmmm] [N|S]
	if (!NMEA_latlon_to_fixed_point(&GpsData->Latitude, param[2], param[3][0] == 'S')) {
		return false;
	}

	// get longitude [dddmm.mmmmm] [E|W]
	if (!NMEA_latlon_to_fixed_point(&GpsData->Longitude, param[4], param[5][0] == 'W')) {
		return false;
	}

	// get number of satellites used in GPS solution
	GpsData->Satellites = atoi(param[7]);

	// get altitude (in meters mm.m)
	GpsData->Altitude = NMEA_real_to_float(param[9]);

	// geoid separation
	GpsData->GeoidSeparation = NMEA_real_to_float(param[11]);

	return true;
}

/**
 * Parse an NMEA GPRMC sentence and update the given UAVObject
 * \param[in] A pointer to a GPSPosition UAVObject to be updated.
 * \param[in] An NMEA sentence with a valid checksum
 */
static bool nmeaProcessGPRMC(GPSPositionData * GpsData, bool* gpsDataUpdated, char* param[], uint8_t nbParam)
{
	if (nbParam != 13)
		return false;

#ifdef NMEA_DEBUG_RMC
	DEBUG_MSG("\n UTC=%s\n", param[1]);
	DEBUG_MSG(" Lat=%s %s\n", param[3], param[4]);
	DEBUG_MSG(" Long=%s %s\n", param[5], param[6]);
	DEBUG_MSG(" Speed=%s\n", param[7]);
	DEBUG_MSG(" Course=%s\n", param[8]);
	DEBUG_MSG(" DateOfFix=%s\n\n", param[9]);
#endif

	*gpsDataUpdated = true;

#if !defined(PIOS_GPS_PURISTIC)
	GPSTimeData gpst;
	GPSTimeGet(&gpst);

	// get UTC time [hhmmss.sss]
	float hms = NMEA_real_to_float(param[1]);
	gpst.Second = (int)hms % 100;
	gpst.Minute = (((int)hms - gpst.Second) / 100) % 100;
	gpst.Hour = (int)hms / 10000;
#endif //PIOS_GPS_PURISTIC

	// get latitude [DDMM.mmmmm] [N|S]
	if (!NMEA_latlon_to_fixed_point(&GpsData->Latitude, param[3], param[4][0] == 'S')) {
		return false;
	}

	// get longitude [dddmm.mmmmm] [E|W]
	if (!NMEA_latlon_to_fixed_point(&GpsData->Longitude, param[5], param[6][0] == 'W')) {
		return false;
	}

	// get speed in knots
	GpsData->Groundspeed = NMEA_real_to_float(param[7]) * 0.51444; // to m/s

	// get True course
	GpsData->Heading = NMEA_real_to_float(param[8]);

#if !defined(PIOS_GPS_PURISTIC)
	// get Date of fix
	// TODO: Should really not use a float here to be safe
	float date = NMEA_real_to_float(param[9]);
	gpst.Year = (int)date % 100;
	gpst.Month = (((int)date - gpst.Year) / 100) % 100;
	gpst.Day = (int)(date / 10000);
	gpst.Year += 2000;
	GPSTimeSet(&gpst);
#endif //PIOS_GPS_PURISTIC

	return true;
}

/**
 * Parse an NMEA GPVTG sentence and update the given UAVObject
 * \param[in] A pointer to a GPSPosition UAVObject to be updated.
 * \param[in] An NMEA sentence with a valid checksum
 */
static bool nmeaProcessGPVTG(GPSPositionData * GpsData, bool* gpsDataUpdated, char* param[], uint8_t nbParam)
{
	if (nbParam != 9 && nbParam != 10 /*GTOP GPS seems to gemnerate an extra parameter...*/)
		return false;

#ifdef NMEA_DEBUG_RMC
	DEBUG_MSG("\n Heading=%s %s\n", param[1], param[2]);
	DEBUG_MSG(" GroundSpeed=%s %s\n", param[5], param[6]);
#endif

	*gpsDataUpdated = true;

	GpsData->Heading = NMEA_real_to_float(param[1]);
	GpsData->Groundspeed = NMEA_real_to_float(param[5]) * 0.51444; // to m/s

	return true;
}

#if !defined(PIOS_GPS_PURISTIC)
/**
 * Parse an NMEA GPZDA sentence and update the @ref GPSTime object
 * \param[in] A pointer to a GPSPosition UAVObject to be updated (unused).
 * \param[in] An NMEA sentence with a valid checksum
 */
static bool nmeaProcessGPZDA(GPSPositionData * GpsData, bool* gpsDataUpdated, char* param[], uint8_t nbParam)
{
	if (nbParam != 7)
		return false;

	#ifdef NMEA_DEBUG_ZDA
		DEBUG_MSG("\n Time=%s (hhmmss.ss)\n", param[1]);
		DEBUG_MSG(" Date=%s/%s/%s (d/m/y)\n", param[2], param[3], param[4]);
	#endif

	*gpsDataUpdated = false;	// Here we will never provide a new GPS value

	// No new data data extracted
	GPSTimeData gpst;
	GPSTimeGet(&gpst);

	// get UTC time [hhmmss.sss]
	float hms = NMEA_real_to_float(param[1]);
	gpst.Second = (int)hms % 100;
	gpst.Minute = (((int)hms - gpst.Second) / 100) % 100;
	gpst.Hour = (int)hms / 10000;

	// Get Date
	gpst.Day = atoi(param[2]);
	gpst.Month = atoi(param[3]);
	gpst.Year = atoi(param[4]);

	GPSTimeSet(&gpst);
	return true;
}

static GPSSatellitesData gsv_partial;
/* Bitmaps of which sentences we're looking for to allow us to handle out-of-order GSVs */
static uint8_t gsv_expected_mask;
static uint8_t gsv_processed_mask;
/* Error counters */
static uint16_t gsv_incomplete_error;
static uint16_t gsv_duplicate_error;

static bool nmeaProcessGPGSV(GPSPositionData * GpsData, bool* gpsDataUpdated, char* param[], uint8_t nbParam)
{
	if (nbParam < 4)
		return false;

#ifdef NMEA_DEBUG_GSV
	DEBUG_MSG("\n Sentence=%s/%s\n", param[2], param[1]);
	DEBUG_MSG(" Sats=%s\n", param[3]);
#endif

	uint8_t nbSentences = atoi(param[1]);
	uint8_t currSentence = atoi(param[2]);

	*gpsDataUpdated = true;

	if (nbSentences < 1 || nbSentences > 8 || currSentence < 1 || currSentence > nbSentences)
		return false;

	gsv_partial.SatsInView = atoi(param[3]);

	// Find out if this is the first sentence in the GSV set
	if (currSentence == 1) {
		if (gsv_expected_mask != gsv_processed_mask) {
			// We are starting over when we haven't yet finished our previous GSV group
			gsv_incomplete_error++;
		}

		// First GSV sentence in the sequence, reset our expected_mask
		gsv_expected_mask = (1 << nbSentences) - 1;
	}

	uint8_t current_sentence_id = (1 << (currSentence - 1));
	if (gsv_processed_mask & current_sentence_id) {
		/* Duplicate sentence in this GSV set */
		gsv_duplicate_error++;
	} else {
		/* Note that we've seen this sentence */
		gsv_processed_mask |= current_sentence_id;
	}

	uint8_t parIdx = 4;

#ifdef NMEA_DEBUG_GSV
	DEBUG_MSG(" PRN:");
#endif

	/* Make sure this sentence can fit in our GPSSatellites object */
	if ((currSentence * 4) <= NELEMENTS(gsv_partial.PRN)) {
		/* Process 4 blocks of satellite info */
		for (uint8_t i = 0; parIdx+4 <= nbParam && i < 4; i++) {
			uint8_t sat_index = ((currSentence - 1) * 4) + i;

			// Get sat info
			gsv_partial.PRN[sat_index] = atoi(param[parIdx++]);
			gsv_partial.Elevation[sat_index] = NMEA_real_to_float(param[parIdx++]);
			gsv_partial.Azimuth[sat_index] = NMEA_real_to_float(param[parIdx++]);
			gsv_partial.SNR[sat_index] = atoi(param[parIdx++]);
#ifdef NMEA_DEBUG_GSV
			DEBUG_MSG(" %d", gsv_partial.PRN[sat_index]);
#endif
		}
	}
#ifdef NMEA_DEBUG_GSV
	DEBUG_MSG("\n");
#endif


	/* Find out if we're finished processing all GSV sentences in the set */
	if ((gsv_expected_mask != 0) && (gsv_processed_mask == gsv_expected_mask)) {
		/* GSV set has been fully processed.  Update the GPSSatellites object. */
		GPSSatellitesSet(&gsv_partial);
		memset((void *)&gsv_partial, 0, sizeof(gsv_partial));
		gsv_expected_mask = 0;
		gsv_processed_mask = 0;
	}

	return true;
}
#endif //PIOS_GPS_PURISTIC

/**
 * Parse an NMEA GPGSA sentence and update the given UAVObject
 * \param[in] A pointer to a GPSPosition UAVObject to be updated.
 * \param[in] An NMEA sentence with a valid checksum
 */
static bool nmeaProcessGPGSA(GPSPositionData * GpsData, bool* gpsDataUpdated, char* param[], uint8_t nbParam)
{
	if (nbParam != 18)
		return false;

#ifdef NMEA_DEBUG_GSA
	DEBUG_MSG("\n Status=%s\n", param[2]);
	DEBUG_MSG(" PDOP=%s\n", param[15]);
	DEBUG_MSG(" HDOP=%s\n", param[16]);
	DEBUG_MSG(" VDOP=%s\n", param[17]);
#endif

	*gpsDataUpdated = true;

	switch (atoi(param[2])) {
	case 1:
		GpsData->Status = GPSPOSITION_STATUS_NOFIX;
		break;
	case 2:
		GpsData->Status = GPSPOSITION_STATUS_FIX2D;
		break;
	case 3:
		GpsData->Status = GPSPOSITION_STATUS_FIX3D;
		break;
	default:
		/* Unhandled */
		return false;
		break;
	}

	// next field: PDOP
	GpsData->PDOP = NMEA_real_to_float(param[15]);

	// next field: HDOP
	GpsData->HDOP = NMEA_real_to_float(param[16]);

	// next field: VDOP
	GpsData->VDOP = NMEA_real_to_float(param[17]);

	return true;
}

#endif	// ENABLE_GPS_NMEA

/**
 * Parse an NMEA PGTOP sentence and update the given UAVObject
 * \param[in] A pointer to a GPSPosition UAVObject to be updated.
 * \param[in] An NMEA sentence with a valid checksum
 */
static bool nmeaProcessPGTOP(GPSPositionData * GpsData, bool* gpsDataUpdated, char* param[], uint8_t nbParam)
{
	if (nbParam != 17)
		return false;

	*gpsDataUpdated = true;

#if !defined(PIOS_GPS_PURISTIC)
	GPSTimeData gpst;
	GPSTimeGet(&gpst);

	// get UTC time [hhmmss.sss]
	float hms = NMEA_real_to_float(param[1]);
	gpst.Second = (int)hms % 100;
	gpst.Minute = (((int)hms - gpst.Second) / 100) % 100;
	gpst.Hour = (int)hms / 10000;
#endif //PIOS_GPS_PURISTIC

	// get latitude decimal degrees
	GpsData->Latitude = NMEA_real_to_float(param[2])*1e7;
	if (param[3][0] == 'S')
		GpsData->Latitude = -GpsData->Latitude;


	// get longitude decimal degrees
	GpsData->Longitude = NMEA_real_to_float(param[4])*1e7;
	if (param[5][0] == 'W')
		GpsData->Longitude = -GpsData->Longitude;

	// get number of satellites used in GPS solution
	GpsData->Satellites = atoi(param[7]);

	// next field: HDOP
	GpsData->HDOP = NMEA_real_to_float(param[8]);

	// get altitude (in meters mm.m)
	GpsData->Altitude = NMEA_real_to_float(param[9]);

	// next field: geoid separation
	GpsData->GeoidSeparation = NMEA_real_to_float(param[10]);

	// Mode: 1=Fix not available, 2=2D, 3=3D
	switch (atoi(param[11])) {
	case 1:
			GpsData->Status = GPSPOSITION_STATUS_NOFIX;
			break;
	case 2:
			GpsData->Status = GPSPOSITION_STATUS_FIX2D;
			break;
	case 3:
			GpsData->Status = GPSPOSITION_STATUS_FIX3D;
			break;
	default:
			/* Unhandled */
			return false;
			break;
	}

	// get course over ground in degrees [ddd.dd]
	GpsData->Heading = NMEA_real_to_float(param[12]);

	// get speed in km/h
	GpsData->Groundspeed = NMEA_real_to_float(param[13]);
	// to m/s
	GpsData->Groundspeed /= 3.6;

#if !defined(PIOS_GPS_PURISTIC)
	gpst.Day = atoi(param[14]);
	gpst.Month = atoi(param[15]);
	gpst.Year = atoi(param[16]);
	GPSTimeSet(&gpst);
#endif //PIOS_GPS_PURISTIC

	return true;
}

#endif	// #if defined(ENABLE_GPS_NMEA) || defined(ENABLE_GPS_ONESENTENCE_GTOP)

#include "openpilot.h"
#include "pios.h"
#include "NMEA.h"
#include "gpsposition.h"

// Debugging
//#define GPSDEBUG
#ifdef GPSDEBUG
#define NMEA_DEBUG_PKT	///< define to enable debug of all NMEA messages
#define NMEA_DEBUG_GGA	///< define to enable debug of GGA messages
#define NMEA_DEBUG_VTG	///< define to enable debug of VTG messages
#define NMEA_DEBUG_RMC	///< define to enable debug of RMC messages
#define NMEA_DEBUG_GSA	///< define to enable debug of GSA messages
#endif

/* Utility functions */
static float NMEA_real_to_float (char * nmea_real);
static bool NMEA_latlon_to_fixed_point (int32_t * latlon, char * nmea_latlon);

/* NMEA sentence parsers */

struct nmea_parser {
  const char         * prefix;
  bool                 (*handler)(GPSPositionData * GpsData, char * sentence);
};

static bool nmeaProcessGPGGA (GPSPositionData * GpsData, char * sentence);
static bool nmeaProcessGPRMC (GPSPositionData * GpsData, char * sentence);
static bool nmeaProcessGPVTG (GPSPositionData * GpsData, char * sentence);
static bool nmeaProcessGPGSA (GPSPositionData * GpsData, char * sentence);

static struct nmea_parser nmea_parsers[] = {
  {
    .prefix  = "GPGGA",
    .handler = nmeaProcessGPGGA,
  },
  {
    .prefix  = "GPVTG",
    .handler = nmeaProcessGPVTG,
  },
  {
    .prefix  = "GPGSA",
    .handler = nmeaProcessGPGSA,
  },
  {
    .prefix  = "GPRMC",
    .handler = nmeaProcessGPRMC,
  },
};



static struct nmea_parser * NMEA_find_parser_by_prefix (char * prefix)
{
  if (!prefix) {
    return (NULL);
  }

  for (uint8_t i = 0; i < NELEMENTS(nmea_parsers); i++) {
    struct nmea_parser * parser = &nmea_parsers[i];

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
bool NMEA_checksum (char * nmea_sentence)
{
  uint8_t         checksum_computed = 0;
  uint8_t         checksum_received;

  while (*nmea_sentence != '\0' &&
	 *nmea_sentence != '*') {
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

/**
 * Parses a complete NMEA sentence and updates the GPSPosition UAVObject
 * \param[in] An NMEA sentence with a valid checksum
 * \return true if the sentence was successfully parsed
 * \return false if any errors were encountered with the parsing
 */
bool NMEA_update_position (char * nmea_sentence)
{
  char               * sentence = nmea_sentence;
  struct nmea_parser * parser;
  char               * prefix;

  /* Find out what kind of NMEA packet we're dealing with */
  prefix = strsep(&sentence, ",");

  /* Check if we have a parser for this packet type */
  parser = NMEA_find_parser_by_prefix (prefix);

  if (!parser) {
    /* Valid but unhandled packet type */
    return false;
  }

  /* Found a matching parser for this packet type */

  /* Reject empty (but valid) packets without parsing */
  if (sentence[0] == ',' &&
      sentence[1] == ',' &&
      sentence[2] == ',') {
    /* Nothing to parse,  */
    return false;
  }

  /* Parse the sentence and update the GpsData object */

  GPSPositionData GpsData;
  GPSPositionGet(&GpsData);
  if (!parser->handler(&GpsData, sentence)) {
    /* Parse failed for valid checksum.  Do not update the UAVObject. */
    return false;
  }
  GPSPositionSet(&GpsData);

  /* Tell the caller what kind of packet we just parsed */
  return true;
}

/**
 * Parse an NMEA GPGGA sentence and update the given UAVObject
 * \param[in] A pointer to a GPSPosition UAVObject to be updated.
 * \param[in] An NMEA sentence with a valid checksum
 */
static bool nmeaProcessGPGGA(GPSPositionData * GpsData, char * sentence)
{
  char * next = sentence;
  char * tokens;
  char * delimiter = ",*";

#ifdef NMEA_DEBUG_GGA
  PIOS_COM_SendFormattedStringNonBlocking(COM_DEBUG_USART,"$%s\r\n",sentence);
#endif

  // get UTC time [hhmmss.sss]
  tokens = strsep(&next, delimiter);
  //strcpy(GpsInfo.TimeOfFix,tokens);

  // next field: latitude
  // get latitude [DDMM.mmmmm]
  tokens = strsep(&next, delimiter);
  if (!NMEA_latlon_to_fixed_point(&GpsData->Latitude, tokens)) {
    return false;
  }
  
  // next field: N/S indicator
  // correct latitute for N/S
  tokens = strsep(&next, delimiter);
  if(tokens[0] == 'S') GpsData->Latitude = -GpsData->Latitude;

  // next field: longitude
  // get longitude [dddmm.mmmmm]
  tokens = strsep(&next, delimiter);
  if (!NMEA_latlon_to_fixed_point(&GpsData->Longitude, tokens)) {
    return false;
  }
  
  // next field: E/W indicator
  // correct longitude for E/W
  tokens = strsep(&next, delimiter);
  if(tokens[0] == 'W') GpsData->Longitude = -GpsData->Longitude;
  
  // next field: position fix status
  // position fix status
  // 0 = Invalid, 1 = Valid SPS, 2 = Valid DGPS, 3 = Valid PPS
  // check for good position fix
  tokens = strsep(&next, delimiter);
  //if((tokens[0] != '0') || (tokens[0] != 0))
  //	GpsData.Updates++;
  
  // next field: satellites used
  // get number of satellites used in GPS solution
  tokens = strsep(&next, delimiter);
  GpsData->Satellites=atoi(tokens);
  
  // next field: HDOP (horizontal dilution of precision)
  tokens = strsep(&next, delimiter);
  
  // next field: altitude
  // get altitude (in meters mm.m)
  tokens = strsep(&next, delimiter);
  GpsData->Altitude = NMEA_real_to_float (tokens);
  
  // next field: altitude units, always 'M'
  tokens = strsep(&next, delimiter);
  
  // next field: geoid separation
  tokens = strsep(&next, delimiter);
  GpsData->GeoidSeparation = NMEA_real_to_float (tokens);
  
  // next field: separation units
  tokens = strsep(&next, delimiter);
  
  // next field: DGPS age
  tokens = strsep(&next, delimiter);
  
  // next field: DGPS station ID
  tokens = strsep(&next, delimiter);
  
  // next field: checksum
  tokens = strsep(&next, delimiter);

  return true;
}

/**
 * Parse an NMEA GPRMC sentence and update the given UAVObject
 * \param[in] A pointer to a GPSPosition UAVObject to be updated.
 * \param[in] An NMEA sentence with a valid checksum
 */
static bool nmeaProcessGPRMC (GPSPositionData * GpsData, char * sentence)
{
  char * next = sentence;
  char * tokens;
  char * delimiter = ",*";

#ifdef NMEA_DEBUG_RMC
  PIOS_COM_SendFormattedStringNonBlocking(COM_DEBUG_USART,"$%s\r\n",sentence);
#endif
  
  // get UTC time [hhmmss.sss]
  tokens = strsep(&next, delimiter);
  
  // next field: Navigation receiver warning A = OK, V = warning
  tokens = strsep(&next, delimiter);

  // next field: latitude
  // get latitude [ddmm.mmmmm]
  tokens = strsep(&next, delimiter);
  if (!NMEA_latlon_to_fixed_point(&GpsData->Latitude, tokens)) {
    return false;
  }

  // next field: N/S indicator
  // correct latitute for N/S
  tokens = strsep(&next, delimiter);
  if(tokens[0] == 'S') GpsData->Latitude = -GpsData->Latitude;

  // next field: longitude
  // get longitude [dddmm.mmmmm]
  tokens = strsep(&next, delimiter);
  if (!NMEA_latlon_to_fixed_point(&GpsData->Longitude, tokens)) {
    return false;
  }

  // next field: E/W indicator
  // correct latitute for E/W
  tokens = strsep(&next, delimiter);
  if(tokens[0] == 'W') GpsData->Longitude = -GpsData->Longitude;
  
  // next field: speed (knots)
  // get speed in knots
  tokens = strsep(&next, delimiter);
  GpsData->Groundspeed  = NMEA_real_to_float (tokens);
  GpsData->Groundspeed *= 0.51444;

  // next field: True course
  // get True course
  tokens = strsep(&next, delimiter);
  GpsData->Heading = NMEA_real_to_float (tokens);

  // next field: Date of fix
  // get Date of fix
  tokens = strsep(&next, delimiter);

  // next field: Magnetic variation
  tokens = strsep(&next, delimiter);

  // next field: E or W
  tokens = strsep(&next, delimiter);

  // next field: Mode: A=autonomous, D=differential, E=Estimated, N=not valid, S=Simulator
  tokens = strsep(&next, delimiter);

  // next field: checksum
  tokens = strsep(&next, delimiter);

  return true;
}


/**
 * Parse an NMEA GPVTG sentence and update the given UAVObject
 * \param[in] A pointer to a GPSPosition UAVObject to be updated.
 * \param[in] An NMEA sentence with a valid checksum
 */
static bool nmeaProcessGPVTG (GPSPositionData * GpsData, char * sentence)
{
  char * next = sentence;
  char * tokens;
  char * delimiter = ",*";

#ifdef NMEA_DEBUG_VTG
  PIOS_COM_SendFormattedStringNonBlocking(COM_DEBUG_USART,"$%s\r\n",sentence);
#endif

  // get course (true north ref) in degrees [ddd.dd]
  tokens = strsep(&next, delimiter);

  // next field: 'T'
  tokens = strsep(&next, delimiter);

  // next field: course (magnetic north)
  // get course (magnetic north ref) in degrees [ddd.dd]
  tokens = strsep(&next, delimiter);
  // next field: 'M'
  tokens = strsep(&next, delimiter);

  // next field: speed (knots)
  // get speed in knots
  tokens = strsep(&next, delimiter);
  // next field: 'N'
  tokens = strsep(&next, delimiter);

  // next field: speed (km/h)
  // get speed in km/h
  tokens = strsep(&next, delimiter);

  // next field: 'K'
  tokens = strsep(&next, delimiter);

  // next field: checksum
  tokens = strsep(&next, delimiter);

  return true;
}

/**
 * Parse an NMEA GPGSA sentence and update the given UAVObject
 * \param[in] A pointer to a GPSPosition UAVObject to be updated.
 * \param[in] An NMEA sentence with a valid checksum
 */
static bool nmeaProcessGPGSA (GPSPositionData * GpsData, char * sentence)
{
  char * next = sentence;
  char * tokens;
  char * delimiter = ",*";

#ifdef NMEA_DEBUG_GSA
  PIOS_COM_SendFormattedStringNonBlocking(COM_DEBUG_USART,"$%s\r\n",sentence);
#endif

  // next field: Mode
  // Mode: M=Manual, forced to operate in 2D or 3D, A=Automatic, 3D/2D
  tokens = strsep(&next, delimiter);

  // next field: Mode
  // Mode: 1=Fix not available, 2=2D, 3=3D
  tokens = strsep(&next, delimiter);
  switch (atoi(tokens)) {
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
    PIOS_DEBUG_Assert(0);
    break;
  }

  // next field: 3-14 IDs of SVs used in position fix (null for unused fields)
  tokens = strsep(&next, delimiter);
  tokens = strsep(&next, delimiter);
  tokens = strsep(&next, delimiter);
  tokens = strsep(&next, delimiter);
  tokens = strsep(&next, delimiter);
  tokens = strsep(&next, delimiter);
  tokens = strsep(&next, delimiter);
  tokens = strsep(&next, delimiter);
  tokens = strsep(&next, delimiter);
  tokens = strsep(&next, delimiter);
  tokens = strsep(&next, delimiter);
  tokens = strsep(&next, delimiter);

  // next field: PDOP
  tokens = strsep(&next, delimiter);
  GpsData->PDOP = NMEA_real_to_float (tokens);

  // next field: HDOP
  tokens = strsep(&next, delimiter);
  GpsData->HDOP = NMEA_real_to_float (tokens);

  // next field: VDOP
  tokens = strsep(&next, delimiter);
  GpsData->VDOP = NMEA_real_to_float (tokens);

  // next field: checksum
  tokens = strsep(&next, delimiter);

  return true;
}


/* Parse a number encoded in a string of the format:
 *   [-]NN.nnnnn
 * into a signed whole part and an unsigned fractional part.
 * The fract_units field indicates the units of the fractional part as
 *   1 whole = 10^fract_units fract
 */
static inline bool NMEA_parse_real (int32_t * whole, uint32_t * fract, uint8_t * fract_units, char * field)
{
  char * s = field;
  char * field_w;
  char * field_f;

  PIOS_DEBUG_Assert (whole);
  PIOS_DEBUG_Assert (fract);
  PIOS_DEBUG_Assert (fract_units);

  field_w = strsep(&s, ".");
  field_f = s;

  *whole = strtol  (field_w, NULL, 10);

  if (field_w) {
    /* decimal was found so we may have a fractional part */
    *fract = strtoul (field_f, NULL, 10);
    *fract_units = strlen(field_f);
  } else {
    /* no decimal was found, fractional part is zero */
    *fract       = 0;
    *fract_units = 0;
  }

  return true;
}

/*
 * This function only exists to deal with a linking
 * failure in the stdlib function strtof().  This
 * implementation does not rely on the _sbrk() syscall
 * like strtof() does.
 */

float NMEA_real_to_float (char * nmea_real)
{
  int32_t  whole;
  uint32_t fract;
  uint8_t  fract_units;

  /* Sanity checks */
  PIOS_DEBUG_Assert (nmea_real);

  if (! NMEA_parse_real (&whole, &fract, &fract_units, nmea_real)) {
    return false;
  }

  /* Convert to float */
  return (((float)whole) + fract * pow(10, -fract_units));
}

/*
 * Parse a field in the format:
 *    DD[D]MM.mmmm[mm]
 * into a fixed-point representation in units of (degrees * 1e-7)
 */
bool NMEA_latlon_to_fixed_point (int32_t * latlon, char * nmea_latlon)
{
  int32_t    num_DDDMM;
  uint32_t   num_m;
  uint8_t    units;

  /* Sanity checks */
  PIOS_DEBUG_Assert (nmea_latlon);
  PIOS_DEBUG_Assert (latlon);

  if (! NMEA_parse_real (&num_DDDMM, &num_m, &units, nmea_latlon)) {
    return false;
  }

  /* scale up the mmmm[mm] field apropriately depending on # of digits */
  switch (units) {
  case 0:
    /* no digits, value is zero so no scaling */
    break;
  case 4:			/* mmmm    */
    num_m *= 1e3;		/* mmmm000 */
    break;
  case 5:			/* mmmmm   */
    num_m *= 1e2;		/* mmmmm00 */
    break;
  case 6:			/* mmmmmm  */
    num_m *= 1e1;		/* mmmmmm0 */
    break;
  default:
    /* unhandled format */
    PIOS_DEBUG_Assert(0);
    break;
  }

  *latlon  = (num_DDDMM / 100) * 1e7;      /* scale the whole degrees */
  *latlon += (num_DDDMM % 100) * 1e7 / 60; /* add in the scaled decimal whole minutes */
  *latlon += num_m                   / 60; /* add in the scaled decimal fractional minutes */

  return true;
}

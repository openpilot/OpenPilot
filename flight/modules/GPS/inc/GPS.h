/**
 ******************************************************************************
 * @addtogroup OpenPilotModules OpenPilot Modules
 * @{ 
 * @addtogroup GSPModule GPS Module
 * @brief Process GPS information
 * @{ 
 *
 * @file       GPS.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      Include file of the GPS module.
 * 	       As with all modules only the initialize function is exposed all other
 * 	       interactions with the module take place through the event queue and
 *             objects.
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

#ifndef GPS_H
#define GPS_H

#include "gpsvelocity.h"
#include "gpssatellites.h"
#include "gpsposition.h"
#include "gpstime.h"

#define	NO_PARSER	-3 // no parser available
#define	PARSER_OVERRUN	-2 // message buffer overrun before completing the message
#define	PARSER_ERROR	-1 // message unparsable by this parser
#define PARSER_INCOMPLETE	0 // parser needs more data to complete the message
#define PARSER_COMPLETE	1 // parser has received a complete message and finished processing

struct GPS_RX_STATS {
	uint16_t gpsRxReceived;
	uint16_t gpsRxChkSumError;
	uint16_t gpsRxOverflow;
	uint16_t gpsRxParserError;
};

int32_t GPSInitialize(void);

#endif // GPS_H

/**
  * @}
  * @}
  */

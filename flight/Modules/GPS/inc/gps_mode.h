/**
 ******************************************************************************
 * @addtogroup OpenPilotModules OpenPilot Modules
 * @{ 
 * @addtogroup GSPModule GPS Module
 * @brief Process GPS information
 * @{ 
 *
 * @file       gps_mode.h
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

#ifndef GPS_MODE_H
#define GPS_MODE_H

// ****************
// you MUST have one of these uncommented - and ONLY one
//
// note: the NMEA includes the one-sentance code as OP has the memory for it

//#define ENABLE_GPS_BINARY_GTOP		// uncomment this if we are using GTOP BINARY mode
//#define ENABLE_GPS_ONESENTENCE_GTOP	// uncomment this if we are using single sentance mode
#define ENABLE_GPS_NMEA				// uncomment this if we are using NMEA mode

// ****************
// make sure they have defined a protocol to use

#if !defined(ENABLE_GPS_BINARY_GTOP) && !defined(ENABLE_GPS_ONESENTENCE_GTOP) && !defined(ENABLE_GPS_NMEA)
	#error YOU MUST SELECT THE DESIRED GPS PROTOCOL IN gps_mode.h!
#endif

// ****************

#endif

/**
  * @}
  * @}
  */

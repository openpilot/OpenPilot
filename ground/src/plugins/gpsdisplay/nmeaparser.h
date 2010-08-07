/**
 ******************************************************************************
 *
 * @file       nmeaparser.h
 * @author     Sami Korhonen Copyright (C) 2010.
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup GPSGadgetPlugin GPS Gadget Plugin
 * @{
 * @brief A gadget that displays GPS status and enables basic configuration
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

#ifndef NMEAPARSER_H
#define NMEAPARSER_H

#include <QtCore>
#include <stdint.h>
#include "buffer.h"

// constants/macros/typdefs
#define NMEA_BUFFERSIZE		128

typedef struct struct_GpsData
{
        float Latitude;
        float Longitude;
        float Altitude;
        float Groundspeed;
        int SV;
        uint8_t channel;
        uint8_t value_h;
        uint8_t value_l;
        uint8_t sum;
}GpsData_t;

class NMEAParser
{
public:
   NMEAParser();
   void processInputStream(char c);
   char* nmeaGetPacketBuffer(void);
   char nmeaChecksum(char* gps_buffer);
   uint8_t nmeaProcess(cBuffer* rxBuffer);
   void nmeaProcessGPGGA(char* packet);
   void nmeaProcessGPRMC(char* packet);
   void nmeaProcessGPVTG(char* packet);
   void nmeaProcessGPGSA(char* packet);
   GpsData_t GpsData;
   cBuffer gpsRxBuffer;
   char gpsRxData[512];
   char NmeaPacket[NMEA_BUFFERSIZE];
   uint32_t numUpdates;
   uint32_t numErrors;
   int32_t gpsRxOverflow;
};

#endif // NMEAPARSER_H

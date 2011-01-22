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

#include <QObject>
#include <QtCore>
#include <stdint.h>
#include "buffer.h"
#include "gpsparser.h"

// constants/macros/typdefs
#define NMEA_BUFFERSIZE		128

typedef struct struct_GpsData
{
        double Latitude;
        double Longitude;
        double Altitude;
        double Groundspeed;
        double Heading;
        int SV;
        int Status;
        double PDOP;
        double HDOP;
        double VDOP;
        double GeoidSeparation;
        double GPStime;
        double GPSdate;

}GpsData_t;

class NMEAParser: public GPSParser
{
    Q_OBJECT

public:
   NMEAParser(QObject *parent = 0);
   ~NMEAParser();
   void processInputStream(char c);
   char* nmeaGetPacketBuffer(void);
   char nmeaChecksum(char* gps_buffer);
   void nmeaTerminateAtChecksum(char* gps_buffer);
   uint8_t nmeaProcess(cBuffer* rxBuffer);
   void nmeaProcessGPGGA(char* packet);
   void nmeaProcessGPRMC(char* packet);
   void nmeaProcessGPVTG(char* packet);
   void nmeaProcessGPGSA(char* packet);
   void nmeaProcessGPGSV(char* packet);
   void nmeaProcessGPZDA(char* packet);
   GpsData_t GpsData;
   cBuffer gpsRxBuffer;
   char gpsRxData[512];
   char NmeaPacket[NMEA_BUFFERSIZE];
   uint32_t numUpdates;
   uint32_t numErrors;
   int32_t gpsRxOverflow;

};

#endif // NMEAPARSER_H

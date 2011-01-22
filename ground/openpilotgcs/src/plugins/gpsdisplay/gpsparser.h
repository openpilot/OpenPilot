/**
 ******************************************************************************
 *
 * @file       gpsparser.h
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

#ifndef GPSPARSER_H
#define GPSPARSER_H

#include <QObject>
#include <QtCore>
#include <stdint.h>

class GPSParser: public QObject
{
    Q_OBJECT
public:
    ~GPSParser();
    virtual void processInputStream(char c);

protected:
    GPSParser(QObject *parent = 0);

signals:
   void sv(int); // Satellites in view
   void position(double,double,double); // Lat, Lon, Alt
   void datetime(double,double); // Date then time
   void speedheading(double,double);
   void packet(QString); // Raw NMEA Packet (or just info)
   void satellite(int,int,int,int,int); // Index, PRN, Elevation, Azimuth, SNR
   void fixmode(QString); // Mode of fix: "Auto", "Manual".
   void fixtype(QString); // Type of fix: "NoGPS", "NoFix", "Fix2D", "Fix3D".
   void dop(double, double, double); // HDOP, VDOP, PDOP
   void fixSVs(QList<int>); // SV's used for fix.
};

#endif // GPSPARSER_H

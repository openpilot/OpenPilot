/**
 ******************************************************************************
 *
 * @file       Antennatrackwidget.cpp
 * @author     Sami Korhonen & the OpenPilot team Copyright (C) 2010.
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup AntennaTrackGadgetPlugin Antenna Track Gadget Plugin
 * @{
 * @brief A gadget that communicates with antenna tracker and enables basic configuration
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

#include "antennatrackwidget.h"
#include "extensionsystem/pluginmanager.h"
#include "uavobjectmanager.h"


#include <iostream>
#include <QtGui>
#include <QDebug>

/*
 * Initialize the widget
 */
AntennaTrackWidget::AntennaTrackWidget(QWidget *parent) : QWidget(parent)
{
    setupUi(this);

    azimuth_old=0;
    elevation_old=0;
}

AntennaTrackWidget::~AntennaTrackWidget()
{
}
void AntennaTrackWidget::setPort(QPointer<QextSerialPort> portx)
{
    port=portx;
}

void AntennaTrackWidget::dumpPacket(const QString &packet)
{
    textBrowser->append(packet);
    if(textBrowser->document()->lineCount() > 200) {
        QTextCursor tc = textBrowser->textCursor();
        tc.movePosition(QTextCursor::Start);
        tc.movePosition(QTextCursor::Down, QTextCursor::KeepAnchor);
        tc.movePosition(QTextCursor::StartOfLine, QTextCursor::KeepAnchor);
        tc.removeSelectedText();
    }
}

void AntennaTrackWidget::setPosition(double lat, double lon, double alt)
{
    //lat *= 1E-7;
    //lon *= 1E-7;
    double deg = floor(fabs(lat));
    double min = (fabs(lat)-deg)*60;
    QString str1;
    str1.sprintf("%.0f%c%.3f' ", deg,0x00b0, min);
    if (lat>0)
        str1.append("N");
    else
        str1.append("S");
    coord_value->setText(str1);
    deg = floor(fabs(lon));
    min = (fabs(lon)-deg)*60;
    QString str2;
    str2.sprintf("%.0f%c%.3f' ", deg,0x00b0, min);
    if (lon>0)
        str2.append("E");
    else
        str2.append("W");
    coord_value_2->setText(str2);
    QString str3;
    str3.sprintf("%.2f m", alt);
    coord_value_3->setText(str3);
    TrackData.Latitude=lat;
    TrackData.Longitude=lon;
    TrackData.Altitude=alt;
    calcAntennaPosition();
}

void AntennaTrackWidget::setHomePosition(double lat, double lon, double alt)
{
    //lat *= 1E-7;
    //lon *= 1E-7;
    double deg = floor(fabs(lat));
    double min = (fabs(lat)-deg)*60;
    QString str1;
    str1.sprintf("%.0f%c%.3f' ", deg,0x00b0, min);
    if (lat>0)
        str1.append("N");
    else
        str1.append("S");
    speed_value->setText(str1);
    deg = floor(fabs(lon));
    min = (fabs(lon)-deg)*60;
    QString str2;
    str2.sprintf("%.0f%c%.3f' ", deg,0x00b0, min);
    if (lon>0)
        str2.append("E");
    else
        str2.append("W");
    bear_label->setText(str2);
    QString str3;
    str3.sprintf("%.2f m", alt);
    bear_value->setText(str3);
    TrackData.HomeLatitude=lat;
    TrackData.HomeLongitude=lon;
    TrackData.HomeAltitude=alt;
    calcAntennaPosition();
}

void AntennaTrackWidget::calcAntennaPosition(void)
{
    /** http://www.movable-type.co.uk/scripts/latlong.html **/
    double lat1, lat2, lon1, lon2, a, c, d, x, y, brng;
    double azimuth, elevation;
    double gcsAlt=TrackData.HomeAltitude; // Home MSL altitude
    double uavAlt=TrackData.Altitude; // UAV MSL altitude
    double dAlt=uavAlt-gcsAlt; // Altitude difference

    // Convert to radians
    lat1 = TrackData.HomeLatitude*(M_PI/180); // Home lat
    lon1 = TrackData.HomeLongitude*(M_PI/180); // Home lon
    lat2 = TrackData.Latitude*(M_PI/180); // UAV lat
    lon2 = TrackData.Longitude*(M_PI/180); // UAV lon

    // Bearing
    /**
    var y = Math.sin(dLon) * Math.cos(lat2);
    var x = Math.cos(lat1)*Math.sin(lat2) -
            Math.sin(lat1)*Math.cos(lat2)*Math.cos(dLon);
    var brng = Math.atan2(y, x).toDeg();
    **/
    y = sin(lon2-lon1) * cos(lat2);
    x = cos(lat1) * sin(lat2) - sin(lat1) * cos(lat2) * cos(lon2-lon1);
    brng = atan2((sin(lon2-lon1)*cos(lat2)),(cos(lat1)*sin(lat2)-sin(lat1)*cos(lat2)*cos(lon2-lon1)))*(180/M_PI);
    if(brng<0)
        brng+=360;

    // bearing to stepper
    azimuth = brng;

    // Haversine formula for distance
    /**
    var R = 6371; // km
    var dLat = (lat2-lat1).toRad();
    var dLon = (lon2-lon1).toRad();
    var a = Math.sin(dLat/2) * Math.sin(dLat/2) +
            Math.cos(lat1.toRad()) * Math.cos(lat2.toRad()) *
            Math.sin(dLon/2) * Math.sin(dLon/2);
    var c = 2 * Math.atan2(Math.sqrt(a), Math.sqrt(1-a));
    var d = R * c;
    **/
    a = sin((lat2-lat1)/2) * sin((lat2-lat1)/2) +
            cos(lat1) * cos(lat2) *
            sin((lon2-lon1)/2) * sin((lon2-lon1)/2);
    c = 2 * atan2(sqrt(a), sqrt(1-a));
    d = 6371 * 1000 * c;

    // Elevation  v depends servo direction
    if(d!=0)
        elevation = 90-(atan(dAlt/d)*(180/M_PI));
    else
        elevation = 0;
    //! TODO: sanity check

    QString str3;
    str3.sprintf("%.0f deg", azimuth);
    azimuth_value->setText(str3);

    str3.sprintf("%.0f deg", elevation);
    elevation_value->setText(str3);

    //servo value 2000-4000
    int servo = (int)(2000.0/180*elevation+2000);
    int stepper = (int)(400.0/360*(azimuth-azimuth_old));

    // send azimuth and elevation to tracker hardware
    str3.sprintf("move %d 2000 2000 2000 %d\r", stepper,servo);
    if(port->isOpen())
    {
        if(azimuth_old!=azimuth || elevation!=elevation_old)
            port->write(str3.toAscii());
        azimuth_old = azimuth;
        elevation_old = elevation;
    }

}

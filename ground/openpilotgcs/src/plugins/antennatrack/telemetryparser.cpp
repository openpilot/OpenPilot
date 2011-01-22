/**
 ******************************************************************************
 *
 * @file       telemetryparser.cpp
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


#include "telemetryparser.h"
#include <math.h>
#include <QDebug>
#include <QStringList>


/**
 * Initialize the parser
 */
TelemetryParser::TelemetryParser(QObject *parent) : GPSParser(parent)
{
    ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
    UAVObjectManager *objManager = pm->getObject<UAVObjectManager>();
    UAVDataObject *gpsObj = dynamic_cast<UAVDataObject*>(objManager->getObject("GPSPosition"));
    if (gpsObj != NULL) {
        connect(gpsObj, SIGNAL(objectUpdated(UAVObject*)), this, SLOT(updateGPS(UAVObject*)));
    } else {
        qDebug() << "Error: Object is unknown (GPSPosition).";
    }

    gpsObj = dynamic_cast<UAVDataObject*>(objManager->getObject("HomeLocation"));
    if (gpsObj != NULL) {
    connect(gpsObj, SIGNAL(objectUpdated(UAVObject*)), this, SLOT(updateHome(UAVObject*)));
    } else {
        qDebug() << "Error: Object is unknown (HomeLocation).";
    }

}

TelemetryParser::~TelemetryParser()
{

}

void TelemetryParser::updateHome( UAVObject* object1) {
    double lat = object1->getField(QString("Latitude"))->getDouble();
    double lon = object1->getField(QString("Longitude"))->getDouble();
    double alt = object1->getField(QString("Altitude"))->getDouble();
    lat *= 1E-7;
    lon *= 1E-7;
    emit home(lat,lon,alt);
}


void TelemetryParser::updateGPS( UAVObject* object1) {
    double lat = object1->getField(QString("Latitude"))->getDouble();
    double lon = object1->getField(QString("Longitude"))->getDouble();
    double alt = object1->getField(QString("Altitude"))->getDouble();
    lat *= 1E-7;
    lon *= 1E-7;
    emit position(lat,lon,alt);
}


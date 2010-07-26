/**
 ******************************************************************************
 *
 * @file       flightsituationactual.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @see        The GNU Public License (GPL) Version 3
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup UAVObjectsPlugin UAVObjects Plugin
 * @{
 *   
 * @note       Object definition file: flightsituationactual.xml. 
 *             This is an automatically generated file.
 *             DO NOT modify manually.
 *
 * @brief      The UAVUObjects GCS plugin 
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
#ifndef FLIGHTSITUATIONACTUAL_H
#define FLIGHTSITUATIONACTUAL_H

#include "uavdataobject.h"
#include "uavobjectmanager.h"

class UAVOBJECTS_EXPORT FlightSituationActual: public UAVDataObject
{
    Q_OBJECT

public:
    // Field structure
    typedef struct {
        float Latitude;
        float Longitude;
        float Altitude;
        float ATG;
        float Climbrate;
        float Heading;
        float Airspeed;
        float Course;
        float Groundspeed;

    } __attribute__((packed)) DataFields;

    // Field information
    // Field Latitude information
    // Field Longitude information
    // Field Altitude information
    // Field ATG information
    // Field Climbrate information
    // Field Heading information
    // Field Airspeed information
    // Field Course information
    // Field Groundspeed information

  
    // Constants
    static const quint32 OBJID = 499301246U;
    static const QString NAME;
    static const bool ISSINGLEINST = 1;
    static const bool ISSETTINGS = 0;
    static const quint32 NUMBYTES = sizeof(DataFields);

    // Functions
    FlightSituationActual();

    DataFields getData();
    void setData(const DataFields& data);
    Metadata getDefaultMetadata();
    UAVDataObject* clone(quint32 instID);

    static FlightSituationActual* GetInstance(UAVObjectManager* objMngr, quint32 instID = 0);
	
private:
    DataFields data;

    void setDefaultFieldValues();

};

#endif // FLIGHTSITUATIONACTUAL_H

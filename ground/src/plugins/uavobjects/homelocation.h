/**
 ******************************************************************************
 *
 * @file       homelocation.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @see        The GNU Public License (GPL) Version 3
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup UAVObjectsPlugin UAVObjects Plugin
 * @{
 *   
 * @note       Object definition file: homelocation.xml. 
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
#ifndef HOMELOCATION_H
#define HOMELOCATION_H

#include "uavdataobject.h"
#include "uavobjectmanager.h"

class UAVOBJECTS_EXPORT HomeLocation: public UAVDataObject
{
    Q_OBJECT

public:
    // Field structure
    typedef struct {
        qint32 Latitude;
        qint32 Longitude;
        float Altitude;
        float ECEF[3];
        float RNE[9];
        float Be[3];

    } __attribute__((packed)) DataFields;

    // Field information
    // Field Latitude information
    // Field Longitude information
    // Field Altitude information
    // Field ECEF information
    /* Number of elements for field ECEF */
    static const quint32 ECEF_NUMELEM = 3;
    // Field RNE information
    /* Number of elements for field RNE */
    static const quint32 RNE_NUMELEM = 9;
    // Field Be information
    /* Number of elements for field Be */
    static const quint32 BE_NUMELEM = 3;

  
    // Constants
    static const quint32 OBJID = 933172238U;
    static const QString NAME;
    static const bool ISSINGLEINST = 1;
    static const bool ISSETTINGS = 0;
    static const quint32 NUMBYTES = sizeof(DataFields);

    // Functions
    HomeLocation();

    DataFields getData();
    void setData(const DataFields& data);
    Metadata getDefaultMetadata();
    UAVDataObject* clone(quint32 instID);

    static HomeLocation* GetInstance(UAVObjectManager* objMngr, quint32 instID = 0);
	
private:
    DataFields data;

    void setDefaultFieldValues();

};

#endif // HOMELOCATION_H

/**
 ******************************************************************************
 *
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @see        The GNU Public License (GPL) Version 3
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup UAVObjectsPlugin UAVObjects Plugin
 * @{
 * @brief      The UAVUObjects GCS plugin 
 *   
 * @note       Object definition file: positionactual.xml. 
 *             This is an automatically generated file.
 *             DO NOT modify manually.
 *
 * @file       positionactual.h
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
#ifndef POSITIONACTUAL_H
#define POSITIONACTUAL_H

#include "uavdataobject.h"
#include "uavobjectmanager.h"

class UAVOBJECTS_EXPORT PositionActual: public UAVDataObject
{
    Q_OBJECT

public:
    // Field structure
    typedef struct {
        quint8 Status;
        float Latitude;
        float Longitude;
        float Altitude;
        float GeoidSeparation;
        float Heading;
        float Groundspeed;
        qint8 Satellites;
        float PDOP;
        float HDOP;
        float VDOP;

    } __attribute__((packed)) DataFields;

    // Field information
    // Field Status information
    /* Enumeration options for field Status */
    typedef enum { STATUS_NOGPS=0, STATUS_NOFIX=1, STATUS_FIX2D=2, STATUS_FIX3D=3,  } StatusOptions;
    // Field Latitude information
    // Field Longitude information
    // Field Altitude information
    // Field GeoidSeparation information
    // Field Heading information
    // Field Groundspeed information
    // Field Satellites information
    // Field PDOP information
    // Field HDOP information
    // Field VDOP information

  
    // Constants
    static const quint32 OBJID = 1265479538U;
    static const QString NAME;
    static const bool ISSINGLEINST = 1;
    static const bool ISSETTINGS = 0;
    static const quint32 NUMBYTES = sizeof(DataFields);

    // Functions
    PositionActual();

    DataFields getData();
    void setData(const DataFields& data);
    Metadata getDefaultMetadata();
    UAVDataObject* clone(quint32 instID);

    static PositionActual* GetInstance(UAVObjectManager* objMngr, quint32 instID = 0);
	
private:
    DataFields data;

    void setDefaultFieldValues();

};

#endif // POSITIONACTUAL_H

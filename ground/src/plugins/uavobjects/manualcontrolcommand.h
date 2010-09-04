/**
 ******************************************************************************
 *
 * @file       manualcontrolcommand.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @see        The GNU Public License (GPL) Version 3
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup UAVObjectsPlugin UAVObjects Plugin
 * @{
 *   
 * @note       Object definition file: manualcontrolcommand.xml. 
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
#ifndef MANUALCONTROLCOMMAND_H
#define MANUALCONTROLCOMMAND_H

#include "uavdataobject.h"
#include "uavobjectmanager.h"

class UAVOBJECTS_EXPORT ManualControlCommand: public UAVDataObject
{
    Q_OBJECT

public:
    // Field structure
    typedef struct {
        quint8 Connected;
        float Roll;
        float Pitch;
        float Yaw;
        float Throttle;
        quint8 FlightMode;
        float Accessory1;
        float Accessory2;
        qint16 Channel[8];

    } __attribute__((packed)) DataFields;

    // Field information
    // Field Connected information
    /* Enumeration options for field Connected */
    typedef enum { CONNECTED_FALSE=0, CONNECTED_TRUE=1 } ConnectedOptions;
    // Field Roll information
    // Field Pitch information
    // Field Yaw information
    // Field Throttle information
    // Field FlightMode information
    /* Enumeration options for field FlightMode */
    typedef enum { FLIGHTMODE_MANUAL=0, FLIGHTMODE_STABILIZED=1, FLIGHTMODE_AUTO=2 } FlightModeOptions;
    // Field Accessory1 information
    // Field Accessory2 information
    // Field Channel information
    /* Number of elements for field Channel */
    static const quint32 CHANNEL_NUMELEM = 8;

  
    // Constants
    static const quint32 OBJID = 540381354U;
    static const QString NAME;
    static const bool ISSINGLEINST = 1;
    static const bool ISSETTINGS = 0;
    static const quint32 NUMBYTES = sizeof(DataFields);

    // Functions
    ManualControlCommand();

    DataFields getData();
    void setData(const DataFields& data);
    Metadata getDefaultMetadata();
    UAVDataObject* clone(quint32 instID);

    static ManualControlCommand* GetInstance(UAVObjectManager* objMngr, quint32 instID = 0);
	
private:
    DataFields data;

    void setDefaultFieldValues();

};

#endif // MANUALCONTROLCOMMAND_H

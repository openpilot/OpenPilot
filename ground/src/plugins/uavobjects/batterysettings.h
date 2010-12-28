/**
 ******************************************************************************
 *
 * @file       batterysettings.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @see        The GNU Public License (GPL) Version 3
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup UAVObjectsPlugin UAVObjects Plugin
 * @{
 *   
 * @note       Object definition file: batterysettings.xml. 
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
#ifndef BATTERYSETTINGS_H
#define BATTERYSETTINGS_H

#include "uavdataobject.h"
#include "uavobjectmanager.h"

class UAVOBJECTS_EXPORT BatterySettings: public UAVDataObject
{
    Q_OBJECT

public:
    // Field structure
    typedef struct {
        float BatteryVoltage;
        quint32 BatteryCapacity;
        quint8 BatteryType;
        float Calibrations[2];

    } __attribute__((packed)) DataFields;

    // Field information
    // Field BatteryVoltage information
    // Field BatteryCapacity information
    // Field BatteryType information
    /* Enumeration options for field BatteryType */
    typedef enum { BATTERYTYPE_LIPO=0, BATTERYTYPE_A123=1, BATTERYTYPE_LICO=2, BATTERYTYPE_LIFESO4=3, BATTERYTYPE_NONE=4 } BatteryTypeOptions;
    // Field Calibrations information
    /* Array element names for field Calibrations */
    typedef enum { CALIBRATIONS_VOLTAGE=0, CALIBRATIONS_CURRENT=1 } CalibrationsElem;
    /* Number of elements for field Calibrations */
    static const quint32 CALIBRATIONS_NUMELEM = 2;

  
    // Constants
    static const quint32 OBJID = 2784959898U;
    static const QString NAME;
    static const QString DESCRIPTION;
    static const bool ISSINGLEINST = 1;
    static const bool ISSETTINGS = 1;
    static const quint32 NUMBYTES = sizeof(DataFields);

    // Functions
    BatterySettings();

    DataFields getData();
    void setData(const DataFields& data);
    Metadata getDefaultMetadata();
    UAVDataObject* clone(quint32 instID);

    static BatterySettings* GetInstance(UAVObjectManager* objMngr, quint32 instID = 0);
	
private:
    DataFields data;

    void setDefaultFieldValues();

};

#endif // BATTERYSETTINGS_H

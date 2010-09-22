/**
 ******************************************************************************
 *
 * @file       guidancesettings.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @see        The GNU Public License (GPL) Version 3
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup UAVObjectsPlugin UAVObjects Plugin
 * @{
 *   
 * @note       Object definition file: guidancesettings.xml. 
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
#ifndef GUIDANCESETTINGS_H
#define GUIDANCESETTINGS_H

#include "uavdataobject.h"
#include "uavobjectmanager.h"

class UAVOBJECTS_EXPORT GuidanceSettings: public UAVDataObject
{
    Q_OBJECT

public:
    // Field structure
    typedef struct {
        quint16 UpdatePeriod;
        float RollMax;
        float PitchMax;
        float PitchMin;
        float PitchRollEpsilon;
        float ThrottleMax;
        float ThrottleMin;
        float SpeedMax;
        float SpeedMin;
        float SpeedKp;
        float SpeedKi;
        float SpeedKd;
        float EnergyKp;
        float EnergyKi;
        float EnergyKd;
        float LateralKp;
        float LateralKi;
        float LateralKd;
        float CourseKp;
        float CourseKi;
        float CourseKd;

    } __attribute__((packed)) DataFields;

    // Field information
    // Field UpdatePeriod information
    // Field RollMax information
    // Field PitchMax information
    // Field PitchMin information
    // Field PitchRollEpsilon information
    // Field ThrottleMax information
    // Field ThrottleMin information
    // Field SpeedMax information
    // Field SpeedMin information
    // Field SpeedKp information
    // Field SpeedKi information
    // Field SpeedKd information
    // Field EnergyKp information
    // Field EnergyKi information
    // Field EnergyKd information
    // Field LateralKp information
    // Field LateralKi information
    // Field LateralKd information
    // Field CourseKp information
    // Field CourseKi information
    // Field CourseKd information

  
    // Constants
    static const quint32 OBJID = 2093064904U;
    static const QString NAME;
    static const bool ISSINGLEINST = 1;
    static const bool ISSETTINGS = 1;
    static const quint32 NUMBYTES = sizeof(DataFields);

    // Functions
    GuidanceSettings();

    DataFields getData();
    void setData(const DataFields& data);
    Metadata getDefaultMetadata();
    UAVDataObject* clone(quint32 instID);

    static GuidanceSettings* GetInstance(UAVObjectManager* objMngr, quint32 instID = 0);
	
private:
    DataFields data;

    void setDefaultFieldValues();

};

#endif // GUIDANCESETTINGS_H

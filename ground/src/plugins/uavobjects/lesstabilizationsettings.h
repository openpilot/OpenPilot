/**
 ******************************************************************************
 *
 * @file       lesstabilizationsettings.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @see        The GNU Public License (GPL) Version 3
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup UAVObjectsPlugin UAVObjects Plugin
 * @{
 *   
 * @note       Object definition file: lesstabilizationsettings.xml. 
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
#ifndef LESSTABILIZATIONSETTINGS_H
#define LESSTABILIZATIONSETTINGS_H

#include "uavdataobject.h"
#include "uavobjectmanager.h"

class UAVOBJECTS_EXPORT LesStabilizationSettings: public UAVDataObject
{
    Q_OBJECT

public:
    // Field structure
    typedef struct {
        quint8 UpdatePeriod;
        quint8 RollMax;
        quint8 PitchMax;
        quint8 YawMax;
        quint8 YawMode;
        float ManualYawRate;
        float MaximumRate[3];
        float RollRateP;
        float PitchRateP;
        float YawRatePI[3];
        float RollPI[3];
        float PitchPI[3];
        float YawPI[3];

    } __attribute__((packed)) DataFields;

    // Field information
    // Field UpdatePeriod information
    // Field RollMax information
    // Field PitchMax information
    // Field YawMax information
    // Field YawMode information
    /* Enumeration options for field YawMode */
    typedef enum { YAWMODE_RATE=0, YAWMODE_HEADING=1 } YawModeOptions;
    // Field ManualYawRate information
    // Field MaximumRate information
    /* Array element names for field MaximumRate */
    typedef enum { MAXIMUMRATE_ROLL=0, MAXIMUMRATE_PITCH=1, MAXIMUMRATE_YAW=2 } MaximumRateElem;
    /* Number of elements for field MaximumRate */
    static const quint32 MAXIMUMRATE_NUMELEM = 3;
    // Field RollRateP information
    // Field PitchRateP information
    // Field YawRatePI information
    /* Array element names for field YawRatePI */
    typedef enum { YAWRATEPI_KP=0, YAWRATEPI_KI=1, YAWRATEPI_ILIMIT=2 } YawRatePIElem;
    /* Number of elements for field YawRatePI */
    static const quint32 YAWRATEPI_NUMELEM = 3;
    // Field RollPI information
    /* Array element names for field RollPI */
    typedef enum { ROLLPI_KP=0, ROLLPI_KI=1, ROLLPI_ILIMIT=2 } RollPIElem;
    /* Number of elements for field RollPI */
    static const quint32 ROLLPI_NUMELEM = 3;
    // Field PitchPI information
    /* Array element names for field PitchPI */
    typedef enum { PITCHPI_KP=0, PITCHPI_KI=1, PITCHPI_ILIMIT=2 } PitchPIElem;
    /* Number of elements for field PitchPI */
    static const quint32 PITCHPI_NUMELEM = 3;
    // Field YawPI information
    /* Array element names for field YawPI */
    typedef enum { YAWPI_KP=0, YAWPI_KI=1, YAWPI_ILIMIT=2 } YawPIElem;
    /* Number of elements for field YawPI */
    static const quint32 YAWPI_NUMELEM = 3;

  
    // Constants
    static const quint32 OBJID = 2839831188U;
    static const QString NAME;
    static const bool ISSINGLEINST = 1;
    static const bool ISSETTINGS = 1;
    static const quint32 NUMBYTES = sizeof(DataFields);

    // Functions
    LesStabilizationSettings();

    DataFields getData();
    void setData(const DataFields& data);
    Metadata getDefaultMetadata();
    UAVDataObject* clone(quint32 instID);

    static LesStabilizationSettings* GetInstance(UAVObjectManager* objMngr, quint32 instID = 0);
	
private:
    DataFields data;

    void setDefaultFieldValues();

};

#endif // LESSTABILIZATIONSETTINGS_H

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
        float ManualRate[3];
        float MaximumRate[3];
        float RollRatePI[3];
        float PitchRatePI[3];
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
    // Field ManualRate information
    /* Array element names for field ManualRate */
    typedef enum { MANUALRATE_ROLL=0, MANUALRATE_PITCH=1, MANUALRATE_YAW=2 } ManualRateElem;
    /* Number of elements for field ManualRate */
    static const quint32 MANUALRATE_NUMELEM = 3;
    // Field MaximumRate information
    /* Array element names for field MaximumRate */
    typedef enum { MAXIMUMRATE_ROLL=0, MAXIMUMRATE_PITCH=1, MAXIMUMRATE_YAW=2 } MaximumRateElem;
    /* Number of elements for field MaximumRate */
    static const quint32 MAXIMUMRATE_NUMELEM = 3;
    // Field RollRatePI information
    /* Array element names for field RollRatePI */
    typedef enum { ROLLRATEPI_KP=0, ROLLRATEPI_KI=1, ROLLRATEPI_ILIMIT=2 } RollRatePIElem;
    /* Number of elements for field RollRatePI */
    static const quint32 ROLLRATEPI_NUMELEM = 3;
    // Field PitchRatePI information
    /* Array element names for field PitchRatePI */
    typedef enum { PITCHRATEPI_KP=0, PITCHRATEPI_KI=1, PITCHRATEPI_ILIMIT=2 } PitchRatePIElem;
    /* Number of elements for field PitchRatePI */
    static const quint32 PITCHRATEPI_NUMELEM = 3;
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
    static const quint32 OBJID = 3247121950U;
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

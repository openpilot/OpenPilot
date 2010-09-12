/**
 ******************************************************************************
 *
 * @file       vtolsettings.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @see        The GNU Public License (GPL) Version 3
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup UAVObjectsPlugin UAVObjects Plugin
 * @{
 *   
 * @note       Object definition file: vtolsettings.xml. 
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
#ifndef VTOLSETTINGS_H
#define VTOLSETTINGS_H

#include "uavdataobject.h"
#include "uavobjectmanager.h"

class UAVOBJECTS_EXPORT VTOLSettings: public UAVDataObject
{
    Q_OBJECT

public:
    // Field structure
    typedef struct {
        float MotorN[4];
        float MotorNE[4];
        float MotorE[4];
        float MotorSE[4];
        float MotorS[4];
        float MotorSW[4];
        float MotorW[4];
        float MotorNW[4];

    } __attribute__((packed)) DataFields;

    // Field information
    // Field MotorN information
    /* Array element names for field MotorN */
    typedef enum { MOTORN_THROTTLE=0, MOTORN_ROLL=1, MOTORN_PITCH=2, MOTORN_YAW=3 } MotorNElem;
    /* Number of elements for field MotorN */
    static const quint32 MOTORN_NUMELEM = 4;
    // Field MotorNE information
    /* Array element names for field MotorNE */
    typedef enum { MOTORNE_THROTTLE=0, MOTORNE_ROLL=1, MOTORNE_PITCH=2, MOTORNE_YAW=3 } MotorNEElem;
    /* Number of elements for field MotorNE */
    static const quint32 MOTORNE_NUMELEM = 4;
    // Field MotorE information
    /* Array element names for field MotorE */
    typedef enum { MOTORE_THROTTLE=0, MOTORE_ROLL=1, MOTORE_PITCH=2, MOTORE_YAW=3 } MotorEElem;
    /* Number of elements for field MotorE */
    static const quint32 MOTORE_NUMELEM = 4;
    // Field MotorSE information
    /* Array element names for field MotorSE */
    typedef enum { MOTORSE_THROTTLE=0, MOTORSE_ROLL=1, MOTORSE_PITCH=2, MOTORSE_YAW=3 } MotorSEElem;
    /* Number of elements for field MotorSE */
    static const quint32 MOTORSE_NUMELEM = 4;
    // Field MotorS information
    /* Array element names for field MotorS */
    typedef enum { MOTORS_THROTTLE=0, MOTORS_ROLL=1, MOTORS_PITCH=2, MOTORS_YAW=3 } MotorSElem;
    /* Number of elements for field MotorS */
    static const quint32 MOTORS_NUMELEM = 4;
    // Field MotorSW information
    /* Array element names for field MotorSW */
    typedef enum { MOTORSW_THROTTLE=0, MOTORSW_ROLL=1, MOTORSW_PITCH=2, MOTORSW_YAW=3 } MotorSWElem;
    /* Number of elements for field MotorSW */
    static const quint32 MOTORSW_NUMELEM = 4;
    // Field MotorW information
    /* Array element names for field MotorW */
    typedef enum { MOTORW_THROTTLE=0, MOTORW_ROLL=1, MOTORW_PITCH=2, MOTORW_YAW=3 } MotorWElem;
    /* Number of elements for field MotorW */
    static const quint32 MOTORW_NUMELEM = 4;
    // Field MotorNW information
    /* Array element names for field MotorNW */
    typedef enum { MOTORNW_THROTTLE=0, MOTORNW_ROLL=1, MOTORNW_PITCH=2, MOTORNW_YAW=3 } MotorNWElem;
    /* Number of elements for field MotorNW */
    static const quint32 MOTORNW_NUMELEM = 4;

  
    // Constants
    static const quint32 OBJID = 254340004U;
    static const QString NAME;
    static const bool ISSINGLEINST = 1;
    static const bool ISSETTINGS = 1;
    static const quint32 NUMBYTES = sizeof(DataFields);

    // Functions
    VTOLSettings();

    DataFields getData();
    void setData(const DataFields& data);
    Metadata getDefaultMetadata();
    UAVDataObject* clone(quint32 instID);

    static VTOLSettings* GetInstance(UAVObjectManager* objMngr, quint32 instID = 0);
	
private:
    DataFields data;

    void setDefaultFieldValues();

};

#endif // VTOLSETTINGS_H

/**
 ******************************************************************************
 *
 * @file       taskinfo.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @see        The GNU Public License (GPL) Version 3
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup UAVObjectsPlugin UAVObjects Plugin
 * @{
 *   
 * @note       Object definition file: taskinfo.xml. 
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
#ifndef TASKINFO_H
#define TASKINFO_H

#include "uavdataobject.h"
#include "uavobjectmanager.h"

class UAVOBJECTS_EXPORT TaskInfo: public UAVDataObject
{
    Q_OBJECT

public:
    // Field structure
    typedef struct {
        quint16 StackRemaining[12];
        quint8 Running[12];

    } __attribute__((packed)) DataFields;

    // Field information
    // Field StackRemaining information
    /* Array element names for field StackRemaining */
    typedef enum { STACKREMAINING_SYSTEM=0, STACKREMAINING_ACTUATOR=1, STACKREMAINING_TELEMETRYTX=2, STACKREMAINING_TELEMETRYTXPRI=3, STACKREMAINING_TELEMETRYRX=4, STACKREMAINING_GPS=5, STACKREMAINING_MANUALCONTROL=6, STACKREMAINING_ALTITUDE=7, STACKREMAINING_AHRSCOMMS=8, STACKREMAINING_STABILIZATION=9, STACKREMAINING_GUIDANCE=10, STACKREMAINING_WATCHDOG=11 } StackRemainingElem;
    /* Number of elements for field StackRemaining */
    static const quint32 STACKREMAINING_NUMELEM = 12;
    // Field Running information
    /* Enumeration options for field Running */
    typedef enum { RUNNING_FALSE=0, RUNNING_TRUE=1 } RunningOptions;
    /* Array element names for field Running */
    typedef enum { RUNNING_SYSTEM=0, RUNNING_ACTUATOR=1, RUNNING_TELEMETRYTX=2, RUNNING_TELEMETRYTXPRI=3, RUNNING_TELEMETRYRX=4, RUNNING_GPS=5, RUNNING_MANUALCONTROL=6, RUNNING_ALTITUDE=7, RUNNING_AHRSCOMMS=8, RUNNING_STABILIZATION=9, RUNNING_GUIDANCE=10, RUNNING_WATCHDOG=11 } RunningElem;
    /* Number of elements for field Running */
    static const quint32 RUNNING_NUMELEM = 12;

  
    // Constants
    static const quint32 OBJID = 3297598544U;
    static const QString NAME;
    static const QString DESCRIPTION;
    static const bool ISSINGLEINST = 1;
    static const bool ISSETTINGS = 0;
    static const quint32 NUMBYTES = sizeof(DataFields);

    // Functions
    TaskInfo();

    DataFields getData();
    void setData(const DataFields& data);
    Metadata getDefaultMetadata();
    UAVDataObject* clone(quint32 instID);

    static TaskInfo* GetInstance(UAVObjectManager* objMngr, quint32 instID = 0);
	
private:
    DataFields data;

    void setDefaultFieldValues();

};

#endif // TASKINFO_H

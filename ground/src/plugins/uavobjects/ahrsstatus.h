/**
 ******************************************************************************
 *
 * @file       ahrsstatus.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @see        The GNU Public License (GPL) Version 3
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup UAVObjectsPlugin UAVObjects Plugin
 * @{
 *   
 * @note       Object definition file: ahrsstatus.xml. 
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
#ifndef AHRSSTATUS_H
#define AHRSSTATUS_H

#include "uavdataobject.h"
#include "uavobjectmanager.h"

class UAVOBJECTS_EXPORT AhrsStatus: public UAVDataObject
{
    Q_OBJECT

public:
    // Field structure
    typedef struct {
        quint8 SerialNumber[8];
        quint8 CPULoad;
        quint32 RunningTime;
        quint8 IdleTimePerCyle;
        quint8 RunningTimePerCyle;
        quint8 DroppedUpdates;
        quint8 LinkRunning;
        quint8 AhrsKickstarts;
        quint8 AhrsCrcErrors;
        quint8 AhrsRetries;
        quint8 AhrsInvalidPackets;
        quint8 OpCrcErrors;
        quint8 OpRetries;
        quint8 OpInvalidPackets;

    } __attribute__((packed)) DataFields;

    // Field information
    // Field SerialNumber information
    /* Number of elements for field SerialNumber */
    static const quint32 SERIALNUMBER_NUMELEM = 8;
    // Field CPULoad information
    // Field RunningTime information
    // Field IdleTimePerCyle information
    // Field RunningTimePerCyle information
    // Field DroppedUpdates information
    // Field LinkRunning information
    /* Enumeration options for field LinkRunning */
    typedef enum { LINKRUNNING_FALSE=0, LINKRUNNING_TRUE=1 } LinkRunningOptions;
    // Field AhrsKickstarts information
    // Field AhrsCrcErrors information
    // Field AhrsRetries information
    // Field AhrsInvalidPackets information
    // Field OpCrcErrors information
    // Field OpRetries information
    // Field OpInvalidPackets information

  
    // Constants
    static const quint32 OBJID = 933623714U;
    static const QString NAME;
    static const QString DESCRIPTION;
    static const bool ISSINGLEINST = 1;
    static const bool ISSETTINGS = 0;
    static const quint32 NUMBYTES = sizeof(DataFields);

    // Functions
    AhrsStatus();

    DataFields getData();
    void setData(const DataFields& data);
    Metadata getDefaultMetadata();
    UAVDataObject* clone(quint32 instID);

    static AhrsStatus* GetInstance(UAVObjectManager* objMngr, quint32 instID = 0);
	
private:
    DataFields data;

    void setDefaultFieldValues();

};

#endif // AHRSSTATUS_H

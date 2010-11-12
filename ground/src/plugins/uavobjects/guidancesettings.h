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
        quint8 GuidanceMode;
        qint32 MaxGroundspeed;
        float GroundVelocityP;
        qint32 MaxVerticalSpeed;
        float VertVelocityP;
        float VelP;
        float VelI;
        float VelD;
        float DownP;
        float DownI;
        float DownD;
        float MaxVelIntegral;
        float MaxThrottleIntegral;
        qint32 VelUpdatePeriod;
        qint32 VelPIDUpdatePeriod;

    } __attribute__((packed)) DataFields;

    // Field information
    // Field GuidanceMode information
    /* Enumeration options for field GuidanceMode */
    typedef enum { GUIDANCEMODE_DUAL_LOOP=0, GUIDANCEMODE_VELOCITY_CONTROL=1, GUIDANCEMODE_POSITION_PID=2 } GuidanceModeOptions;
    // Field MaxGroundspeed information
    // Field GroundVelocityP information
    // Field MaxVerticalSpeed information
    // Field VertVelocityP information
    // Field VelP information
    // Field VelI information
    // Field VelD information
    // Field DownP information
    // Field DownI information
    // Field DownD information
    // Field MaxVelIntegral information
    // Field MaxThrottleIntegral information
    // Field VelUpdatePeriod information
    // Field VelPIDUpdatePeriod information

  
    // Constants
    static const quint32 OBJID = 2071403670U;
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

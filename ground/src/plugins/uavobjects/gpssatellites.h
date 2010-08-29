/**
 ******************************************************************************
 *
 * @file       gpssatellites.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @see        The GNU Public License (GPL) Version 3
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup UAVObjectsPlugin UAVObjects Plugin
 * @{
 *   
 * @note       Object definition file: gpssatellites.xml. 
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
#ifndef GPSSATELLITES_H
#define GPSSATELLITES_H

#include "uavdataobject.h"
#include "uavobjectmanager.h"

class UAVOBJECTS_EXPORT GPSSatellites: public UAVDataObject
{
    Q_OBJECT

public:
    // Field structure
    typedef struct {
        qint8 SatsInView;
        qint8 PRN[16];
        float Elevation[16];
        float Azimuth[16];
        qint8 SNR[16];

    } __attribute__((packed)) DataFields;

    // Field information
    // Field SatsInView information
    // Field PRN information
    /* Number of elements for field PRN */
    static const quint32 PRN_NUMELEM = 16;
    // Field Elevation information
    /* Number of elements for field Elevation */
    static const quint32 ELEVATION_NUMELEM = 16;
    // Field Azimuth information
    /* Number of elements for field Azimuth */
    static const quint32 AZIMUTH_NUMELEM = 16;
    // Field SNR information
    /* Number of elements for field SNR */
    static const quint32 SNR_NUMELEM = 16;

  
    // Constants
    static const quint32 OBJID = 3593446318U;
    static const QString NAME;
    static const bool ISSINGLEINST = 1;
    static const bool ISSETTINGS = 0;
    static const quint32 NUMBYTES = sizeof(DataFields);

    // Functions
    GPSSatellites();

    DataFields getData();
    void setData(const DataFields& data);
    Metadata getDefaultMetadata();
    UAVDataObject* clone(quint32 instID);

    static GPSSatellites* GetInstance(UAVObjectManager* objMngr, quint32 instID = 0);
	
private:
    DataFields data;

    void setDefaultFieldValues();

};

#endif // GPSSATELLITES_H

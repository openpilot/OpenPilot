/**
 ******************************************************************************
 *
 * @file       pipxtrememodemstatus.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @see        The GNU Public License (GPL) Version 3
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup UAVObjectsPlugin UAVObjects Plugin
 * @{
 *   
 * @note       Object definition file: pipxtrememodemstatus.xml. 
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
#ifndef PIPXTREMEMODEMSTATUS_H
#define PIPXTREMEMODEMSTATUS_H

#include "uavdataobject.h"
#include "uavobjectmanager.h"

class UAVOBJECTS_EXPORT PipXtremeModemStatus: public UAVDataObject
{
    Q_OBJECT

public:
    // Field structure
    typedef struct {
        quint8 Firmware_Version_Major;
        quint8 Firmware_Version_Minor;
        quint32 Serial_Number;
        quint32 Up_Time;
        quint32 Frequency;
        quint32 RF_Bandwidth;
        qint8 Tx_Power;
        quint8 State;
        quint16 Tx_Retry;
        quint32 Tx_Data_Rate;
        quint32 Rx_Data_Rate;

    } __attribute__((packed)) DataFields;

    // Field information
    // Field Firmware_Version_Major information
    // Field Firmware_Version_Minor information
    // Field Serial_Number information
    // Field Up_Time information
    // Field Frequency information
    // Field RF_Bandwidth information
    // Field Tx_Power information
    // Field State information
    /* Enumeration options for field State */
    typedef enum { STATE_DISCONNECTED=0, STATE_CONNECTING=1, STATE_CONNECTED=2, STATE_NOTREADY=3 } StateOptions;
    // Field Tx_Retry information
    // Field Tx_Data_Rate information
    // Field Rx_Data_Rate information

  
    // Constants
    static const quint32 OBJID = 2490854928U;
    static const QString NAME;
    static const QString DESCRIPTION;
    static const bool ISSINGLEINST = 1;
    static const bool ISSETTINGS = 0;
    static const quint32 NUMBYTES = sizeof(DataFields);

    // Functions
    PipXtremeModemStatus();

    DataFields getData();
    void setData(const DataFields& data);
    Metadata getDefaultMetadata();
    UAVDataObject* clone(quint32 instID);

    static PipXtremeModemStatus* GetInstance(UAVObjectManager* objMngr, quint32 instID = 0);
	
private:
    DataFields data;

    void setDefaultFieldValues();

};

#endif // PIPXTREMEMODEMSTATUS_H

/**
 ******************************************************************************
 *
 * @file       i2cstats.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @see        The GNU Public License (GPL) Version 3
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup UAVObjectsPlugin UAVObjects Plugin
 * @{
 *   
 * @note       Object definition file: i2cstats.xml. 
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
#ifndef I2CSTATS_H
#define I2CSTATS_H

#include "uavdataobject.h"
#include "uavobjectmanager.h"

class UAVOBJECTS_EXPORT I2CStats: public UAVDataObject
{
    Q_OBJECT

public:
    // Field structure
    typedef struct {
        quint16 event_errors;
        quint16 fsm_errors;
        quint16 irq_errors;
        quint8 last_error_type;
        quint32 event_log[5];
        quint32 state_log[5];

    } __attribute__((packed)) DataFields;

    // Field information
    // Field event_errors information
    // Field fsm_errors information
    // Field irq_errors information
    // Field last_error_type information
    /* Enumeration options for field last_error_type */
    typedef enum { LAST_ERROR_TYPE_EVENT=0, LAST_ERROR_TYPE_FSM=1, LAST_ERROR_TYPE_INTERRUPT=2 } last_error_typeOptions;
    // Field event_log information
    /* Number of elements for field event_log */
    static const quint32 EVENT_LOG_NUMELEM = 5;
    // Field state_log information
    /* Number of elements for field state_log */
    static const quint32 STATE_LOG_NUMELEM = 5;

  
    // Constants
    static const quint32 OBJID = 1063893720U;
    static const QString NAME;
    static const QString DESCRIPTION;
    static const bool ISSINGLEINST = 1;
    static const bool ISSETTINGS = 0;
    static const quint32 NUMBYTES = sizeof(DataFields);

    // Functions
    I2CStats();

    DataFields getData();
    void setData(const DataFields& data);
    Metadata getDefaultMetadata();
    UAVDataObject* clone(quint32 instID);

    static I2CStats* GetInstance(UAVObjectManager* objMngr, quint32 instID = 0);
	
private:
    DataFields data;

    void setDefaultFieldValues();

};

#endif // I2CSTATS_H

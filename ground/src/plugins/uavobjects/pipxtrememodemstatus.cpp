/**
 ******************************************************************************
 *
 * @file       pipxtrememodemstatus.cpp
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
#include "pipxtrememodemstatus.h"
#include "uavobjectfield.h"

const QString PipXtremeModemStatus::NAME = QString("PipXtremeModemStatus");
const QString PipXtremeModemStatus::DESCRIPTION = QString("Status for the @ref PipXtremeModem");

/**
 * Constructor
 */
PipXtremeModemStatus::PipXtremeModemStatus(): UAVDataObject(OBJID, ISSINGLEINST, ISSETTINGS, NAME)
{
    // Create fields
    QList<UAVObjectField*> fields;
    QStringList Firmware_Version_MajorElemNames;
    Firmware_Version_MajorElemNames.append("0");
    fields.append( new UAVObjectField(QString("Firmware_Version_Major"), QString(""), UAVObjectField::UINT8, Firmware_Version_MajorElemNames, QStringList()) );
    QStringList Firmware_Version_MinorElemNames;
    Firmware_Version_MinorElemNames.append("0");
    fields.append( new UAVObjectField(QString("Firmware_Version_Minor"), QString(""), UAVObjectField::UINT8, Firmware_Version_MinorElemNames, QStringList()) );
    QStringList Serial_NumberElemNames;
    Serial_NumberElemNames.append("0");
    fields.append( new UAVObjectField(QString("Serial_Number"), QString(""), UAVObjectField::UINT32, Serial_NumberElemNames, QStringList()) );
    QStringList Up_TimeElemNames;
    Up_TimeElemNames.append("0");
    fields.append( new UAVObjectField(QString("Up_Time"), QString("ms"), UAVObjectField::UINT32, Up_TimeElemNames, QStringList()) );
    QStringList FrequencyElemNames;
    FrequencyElemNames.append("0");
    fields.append( new UAVObjectField(QString("Frequency"), QString("Hz"), UAVObjectField::UINT32, FrequencyElemNames, QStringList()) );
    QStringList RF_BandwidthElemNames;
    RF_BandwidthElemNames.append("0");
    fields.append( new UAVObjectField(QString("RF_Bandwidth"), QString("bits/sec"), UAVObjectField::UINT32, RF_BandwidthElemNames, QStringList()) );
    QStringList Tx_PowerElemNames;
    Tx_PowerElemNames.append("0");
    fields.append( new UAVObjectField(QString("Tx_Power"), QString("dBm"), UAVObjectField::INT8, Tx_PowerElemNames, QStringList()) );
    QStringList StateElemNames;
    StateElemNames.append("0");
    QStringList StateEnumOptions;
    StateEnumOptions.append("Disconnected");
    StateEnumOptions.append("Connecting");
    StateEnumOptions.append("Connected");
    StateEnumOptions.append("NotReady");
    fields.append( new UAVObjectField(QString("State"), QString(""), UAVObjectField::ENUM, StateElemNames, StateEnumOptions) );
    QStringList Tx_RetryElemNames;
    Tx_RetryElemNames.append("0");
    fields.append( new UAVObjectField(QString("Tx_Retry"), QString(""), UAVObjectField::UINT16, Tx_RetryElemNames, QStringList()) );
    QStringList Tx_Data_RateElemNames;
    Tx_Data_RateElemNames.append("0");
    fields.append( new UAVObjectField(QString("Tx_Data_Rate"), QString("bits/sec"), UAVObjectField::UINT32, Tx_Data_RateElemNames, QStringList()) );
    QStringList Rx_Data_RateElemNames;
    Rx_Data_RateElemNames.append("0");
    fields.append( new UAVObjectField(QString("Rx_Data_Rate"), QString("bits/sec"), UAVObjectField::UINT32, Rx_Data_RateElemNames, QStringList()) );

    // Initialize object
    initializeFields(fields, (quint8*)&data, NUMBYTES);
    // Set the default field values
    setDefaultFieldValues();
    // Set the object description
    setDescription(DESCRIPTION);
}

/**
 * Get the default metadata for this object
 */
UAVObject::Metadata PipXtremeModemStatus::getDefaultMetadata()
{
    UAVObject::Metadata metadata;
    metadata.flightAccess = ACCESS_READWRITE;
    metadata.gcsAccess = ACCESS_READWRITE;
    metadata.gcsTelemetryAcked = 0;
    metadata.gcsTelemetryUpdateMode = UAVObject::UPDATEMODE_MANUAL;
    metadata.gcsTelemetryUpdatePeriod = 0;
    metadata.flightTelemetryAcked = 0;
    metadata.flightTelemetryUpdateMode = UAVObject::UPDATEMODE_PERIODIC;
    metadata.flightTelemetryUpdatePeriod = 5000;
    metadata.loggingUpdateMode = UAVObject::UPDATEMODE_NEVER;
    metadata.loggingUpdatePeriod = 0;
    return metadata;
}

/**
 * Initialize object fields with the default values.
 * If a default value is not specified the object fields
 * will be initialized to zero.
 */
void PipXtremeModemStatus::setDefaultFieldValues()
{
    data.Firmware_Version_Major = 0;
    data.Firmware_Version_Minor = 0;
    data.Serial_Number = 0;

}

/**
 * Get the object data fields
 */
PipXtremeModemStatus::DataFields PipXtremeModemStatus::getData()
{
    QMutexLocker locker(mutex);
    return data;
}

/**
 * Set the object data fields
 */
void PipXtremeModemStatus::setData(const DataFields& data)
{
    QMutexLocker locker(mutex);
    // Get metadata
    Metadata mdata = getMetadata();
    // Update object if the access mode permits
    if ( mdata.gcsAccess == ACCESS_READWRITE )
    {
        this->data = data;
        emit objectUpdatedAuto(this); // trigger object updated event
        emit objectUpdated(this);
    }
}

/**
 * Create a clone of this object, a new instance ID must be specified.
 * Do not use this function directly to create new instances, the
 * UAVObjectManager should be used instead.
 */
UAVDataObject* PipXtremeModemStatus::clone(quint32 instID)
{
    PipXtremeModemStatus* obj = new PipXtremeModemStatus();
    obj->initialize(instID, this->getMetaObject());
    return obj;
}

/**
 * Static function to retrieve an instance of the object.
 */
PipXtremeModemStatus* PipXtremeModemStatus::GetInstance(UAVObjectManager* objMngr, quint32 instID)
{
    return dynamic_cast<PipXtremeModemStatus*>(objMngr->getObject(PipXtremeModemStatus::OBJID, instID));
}

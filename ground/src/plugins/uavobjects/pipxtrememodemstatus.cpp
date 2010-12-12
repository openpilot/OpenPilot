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

/**
 * Constructor
 */
PipXtremeModemStatus::PipXtremeModemStatus(): UAVDataObject(OBJID, ISSINGLEINST, ISSETTINGS, NAME)
{
    // Create fields
    QList<UAVObjectField*> fields;
    QStringList Serial_NumberElemNames;
    Serial_NumberElemNames.append("0");
    Serial_NumberElemNames.append("1");
    Serial_NumberElemNames.append("2");
    Serial_NumberElemNames.append("3");
    Serial_NumberElemNames.append("4");
    Serial_NumberElemNames.append("5");
    Serial_NumberElemNames.append("6");
    Serial_NumberElemNames.append("7");
    Serial_NumberElemNames.append("8");
    Serial_NumberElemNames.append("9");
    Serial_NumberElemNames.append("10");
    Serial_NumberElemNames.append("11");
    Serial_NumberElemNames.append("12");
    Serial_NumberElemNames.append("13");
    Serial_NumberElemNames.append("14");
    Serial_NumberElemNames.append("15");
    Serial_NumberElemNames.append("16");
    Serial_NumberElemNames.append("17");
    Serial_NumberElemNames.append("18");
    Serial_NumberElemNames.append("19");
    Serial_NumberElemNames.append("20");
    Serial_NumberElemNames.append("21");
    Serial_NumberElemNames.append("22");
    Serial_NumberElemNames.append("23");
    fields.append( new UAVObjectField(QString("Serial_Number"), QString(""), UAVObjectField::UINT8, Serial_NumberElemNames, QStringList()) );
    QStringList Serial_Number_CRCElemNames;
    Serial_Number_CRCElemNames.append("0");
    fields.append( new UAVObjectField(QString("Serial_Number_CRC"), QString(""), UAVObjectField::UINT32, Serial_Number_CRCElemNames, QStringList()) );
    QStringList Up_TimeElemNames;
    Up_TimeElemNames.append("0");
    fields.append( new UAVObjectField(QString("Up_Time"), QString("ms"), UAVObjectField::UINT32, Up_TimeElemNames, QStringList()) );
    QStringList FrequencyElemNames;
    FrequencyElemNames.append("0");
    fields.append( new UAVObjectField(QString("Frequency"), QString("Hz"), UAVObjectField::UINT32, FrequencyElemNames, QStringList()) );
    QStringList RF_BandwidthElemNames;
    RF_BandwidthElemNames.append("0");
    fields.append( new UAVObjectField(QString("RF_Bandwidth"), QString("Hz"), UAVObjectField::UINT32, RF_BandwidthElemNames, QStringList()) );
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
    fields.append( new UAVObjectField(QString("Tx_Data_Rate"), QString("bytes/sec"), UAVObjectField::UINT32, Tx_Data_RateElemNames, QStringList()) );
    QStringList Rx_Data_RateElemNames;
    Rx_Data_RateElemNames.append("0");
    fields.append( new UAVObjectField(QString("Rx_Data_Rate"), QString("bytes/sec"), UAVObjectField::UINT32, Rx_Data_RateElemNames, QStringList()) );

    // Initialize object
    initializeFields(fields, (quint8*)&data, NUMBYTES);
    // Set the default field values
    setDefaultFieldValues();
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
    data.Serial_Number[0] = 0;
    data.Serial_Number[1] = 0;
    data.Serial_Number[2] = 0;
    data.Serial_Number[3] = 0;
    data.Serial_Number[4] = 0;
    data.Serial_Number[5] = 0;
    data.Serial_Number[6] = 0;
    data.Serial_Number[7] = 0;
    data.Serial_Number[8] = 0;
    data.Serial_Number[9] = 0;
    data.Serial_Number[10] = 0;
    data.Serial_Number[11] = 0;
    data.Serial_Number[12] = 0;
    data.Serial_Number[13] = 0;
    data.Serial_Number[14] = 0;
    data.Serial_Number[15] = 0;
    data.Serial_Number[16] = 0;
    data.Serial_Number[17] = 0;
    data.Serial_Number[18] = 0;
    data.Serial_Number[19] = 0;
    data.Serial_Number[20] = 0;
    data.Serial_Number[21] = 0;
    data.Serial_Number[22] = 0;
    data.Serial_Number[23] = 0;
    data.Serial_Number_CRC = 0;

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
